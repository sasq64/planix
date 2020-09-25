
#include <mosquitto.h>

#include <memory>
#include <string>
#include <stdexcept>

template <typename T> auto make_unique_ptr(T *p, void (*f)(T *)) {
	return std::unique_ptr<T, decltype(f)>(p, f);
}


class mqtt_exception : public std::exception
{
public:
    explicit mqtt_exception(std::string m = "IO Exception") : msg(std::move(m)) {}
    const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};


int main() {
	mosquitto_lib_init();

	auto *mqtt = mosquitto_new("myid", true, nullptr);
	auto mp = make_unique_ptr(mqtt, &mosquitto_destroy);

	auto rc = mosquitto_connect(mp.get(), "localhost", 1883, 10);
	if(rc != MOSQ_ERR_SUCCESS) {
        throw mqtt_exception("connect");
	}

	struct Raw {
		int x;
	};

	Raw raw;

	rc = mosquitto_publish(mp.get(), nullptr, "test/topic", sizeof(Raw), &raw, 1, false);

	if(rc != MOSQ_ERR_SUCCESS) {
        throw mqtt_exception("publish");
	}

	mosquitto_loop_forever(mp.get(), 10, 10);
	mosquitto_disconnect(mp.get());
}
