#include "mqtt.hpp"

#include <fmt/core.h>
#include <mosquitto.h>

#include <exception>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace std::string_literals;

template <typename T>
auto make_unique(T* p, void (*f)(T*))
{
    return std::unique_ptr<T, decltype(f)>(p, f);
}

struct LibInit
{
    int rc;
    LibInit() { rc = mosquitto_lib_init(); }
    ~LibInit() { mosquitto_lib_cleanup(); }
};

int init_lib()
{
    static LibInit lib_init;
    return lib_init.rc;
}

void MQTT::connect_callback(int rc)
{
    fmt::print("Connected {}\n", rc);
    connected = true;
}

void MQTT::message_callback(const mosquitto_message& msg)
{
    fmt::print("Message {}\n", msg.topic);
    messages.enqueue(Msg{msg.topic, static_cast<byte*>(msg.payload),
                         static_cast<size_t>(msg.payloadlen)});
}

void MQTT::publish(std::string const& topic, const void* data, size_t len)
{
    int rc = mosquitto_publish(mp.get(), nullptr, topic.c_str(), len, data, 2,
                               false);
    if (rc != MOSQ_ERR_SUCCESS) {
        throw mqtt_exception(fmt::format("publish: {}", rc));
    }
    fmt::print("Published {}\n", topic);
}

MQTT::MQTT(std::string const& id)
    : lib_rc{init_lib()}, mp{make_unique(
                              mosquitto_new(id.empty() ? nullptr : id.c_str(),
                                            true, this),
                              &mosquitto_destroy)}
{
    mosquitto_connect_callback_set(mp.get(), [](auto*, void* data, int rc) {
        static_cast<MQTT*>(data)->connect_callback(rc);
    });

    mosquitto_subscribe_callback_set(
        mp.get(), [](auto* msq, void* data, int mid, int qos, const int* x) {
            fmt::print("Subscribed to {}\n", mid);
            // static_cast<MQTT*>(data)->subscribe_callback(mid, qos, x);
        });

    mosquitto_message_callback_set(
        mp.get(), [](auto*, void* data, struct mosquitto_message const* msg) {
            static_cast<MQTT*>(data)->message_callback(*msg);
        });
}

MQTT::~MQTT()
{
    fmt::print("Stoping\n");
    mosquitto_disconnect(mp.get());
    if(mqtt_thread.joinable()) {
        mqtt_thread.join();
    }
}

void MQTT::loop()
{
    fmt::print("Loop\n");
    mosquitto_loop_forever(mp.get(), 10, 10);
    fmt::print("Stopped\n");

    //mosquitto_disconnect(mp.get());
}

void MQTT::start()
{
    mqtt_thread = std::thread([this] { loop(); });
}

void MQTT::connect(const std::string& host, int port)
{
    //mosquitto_username_pw_set(mp.get(), "arne", "pass");

    mosquitto_connect(mp.get(), host.c_str(), port, 10);
}
void MQTT::subscribe(const std::string& topic)
{
    mosquitto_subscribe(mp.get(), nullptr, topic.c_str(), 2);
}
MQTT::Msg MQTT::pop_message()
{
    Msg msg;
    messages.try_dequeue(msg);
    return msg;
}
