#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "../src/mqtt.hpp"
#include <chrono>
#include <fmt/format.h>
#include <string>

using namespace std::string_literals;
using namespace std::chrono_literals;

SCENARIO("datastore")
{
    GIVEN("Two mqtt sessions")
    {
        std::array<MQTT, 2> mqtt;
        WHEN("we connect them to the broker")
        {
            mqtt[0].start();
            mqtt[1].start();
            mqtt[0].connect("localhost");
            mqtt[1].connect("localhost");

            while ((!mqtt[0].is_connected()) || (!mqtt[1].is_connected())) {
                std::this_thread::sleep_for(100ms);
                fmt::print("Waiting\n");
            }

            THEN("we can publish and subscribe to a topic")
            {
                mqtt[0].subscribe("test/#");
                std::this_thread::sleep_for(100ms);
                mqtt[1].publish("other/topic", "some data");
                mqtt[1].publish("test/topic", "some more data");

                while (true) {
                    auto msg = mqtt[0].pop_message();
                    if (msg) {
                        std::string s{(const char*)msg.data.data(),
                                      msg.data.size()};
                        fmt::print("Got message:{}\n", s);
                        REQUIRE(s == "some more data");
                        break;
                    }
                    std::this_thread::sleep_for(100ms);
                }
            }
        }
    }
}
