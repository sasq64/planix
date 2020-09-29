#include "locking_queue.h"

#include <cstring>
#include <memory>
#include <thread>
#include <vector>

struct mosquitto;

class mqtt_exception : public std::exception
{
public:
    explicit mqtt_exception(std::string m = "IO Exception") : msg(std::move(m))
    {}
    const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};

class MQTT
{
    struct Msg
    {
        Msg() = default;
        Msg(const char* t, uint8_t* d, size_t len) : topic{t}, data{d, d + len}
        {}
        explicit operator bool() const { return !topic.empty(); }
        std::string topic;
        std::vector<uint8_t> data;
    };

    int lib_rc;
    LockingQueue<Msg> messages;
    std::unique_ptr<struct mosquitto, void (*)(struct mosquitto*)> mp;
    std::thread mqtt_thread;
    bool connected{false};

    void connect_callback(int rc);
    void message_callback(struct mosquitto_message const& message);
    void loop();

public:
    explicit MQTT(std::string const& id = "");
    ~MQTT();

    template <typename T>
    void publish(std::string const& topic, T const& t)
    {
        publish(topic, static_cast<void*>(&t), sizeof(T));
    }

    void publish(std::string const& topic, std::string const& s)
    {
        publish(topic, static_cast<const void*>(s.data()), s.length());
    }

    void publish(std::string const& topic, const char* data)
    {
        publish(topic, static_cast<const void*>(data), strlen(data));
    }

    bool is_connected() const { return connected; }

    Msg pop_message();

    void subscribe(std::string const& topic);
    void connect(std::string const& host, int port = 1883);
    void publish(const std::string& topic, const void* data, size_t len);
    void start();
};