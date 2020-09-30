
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "../src/server.hpp"

#include <fmt/format.h>
#include <chrono>
#include <string>

using namespace std::string_literals;
using namespace std::chrono_literals;

void write_pipe(const char* fifo_name, std::string const& what)
{
    auto* fp = fopen(fifo_name, "ae");
    fputs((what + "\n").c_str(), fp);
    fclose(fp);
}

SCENARIO("Server")
{
    GIVEN("Two planix servers")
    {
        std::array<Server, 2> servers{ Server{"pipe0"s}, Server{"pipe1"s} };
        WHEN("we connect them to the broker")
        {
            std::thread s0 { [&] { servers[0].run("say hello"); } };
            std::thread s1 { [&] { servers[1].run("say hello"); } };


            std::this_thread::sleep_for(1s);

            write_pipe("pipe0", "kill");
            write_pipe("pipe1", "kill");
            
            fmt::print("{} {}\n", s0.joinable(), s1.joinable());

            s0.join();
            fmt::print("{} {}\n", s0.joinable(), s1.joinable());
            s1.join();

        }
    }
}
