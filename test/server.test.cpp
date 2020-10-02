
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "../src/server.hpp"

#include <chrono>
#include <fmt/format.h>
#include <string>

using namespace std::string_literals;
using namespace std::chrono_literals;

SCENARIO("Server")
{
    GIVEN("Two planix servers")
    {
        std::array<Server, 2> servers{Server{"pipe0"s, "123"s},
                                      Server{"pipe1"s, "123"s}};
        WHEN("we connect them to the broker")
        {
            std::thread s0{[&] { servers[0].run(""); }};
            std::thread s1{[&] { servers[1].run(""); }};

            std::this_thread::sleep_for(1s);

            Server::write_pipe("pipe0", "say Hello");
            std::this_thread::sleep_for(300ms);
            Server::write_pipe("pipe1", "incoming");

            fmt::print("{} {}\n", s0.joinable(), s1.joinable());

            s0.join();
            fmt::print("{} {}\n", s0.joinable(), s1.joinable());
            s1.join();
        }
    }
}
