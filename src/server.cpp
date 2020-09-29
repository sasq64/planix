#include "server.hpp"
#include <cstdio>
#include <fmt/core.h>
#include <sys/stat.h>

void Server::run_command(const std::string& cmd)
{
    // fmt::print("Got line {}\n", ptr);
    if (cmd.find("kill") == 0) {
        doQuit = true;
        return;
    }
    if(cmd.find("say ") == 0) {
        auto line = cmd.substr(4);
        mqtt.publish(key + "/say", line);
    }
    handle_incoming();
    fmt::print("{}", cmd);
}
void Server::handle_incoming()
{
    while(true) {
        auto msg = mqtt.pop_message();
        if(!msg) {
            break;
        }

    }
}
void Server::run(const std::string& command)
{
    mkfifo(fifo_name.c_str(), 0666);
    mqtt.connect("localhost");
    run_command(command + "\n");
    std::array<char, 256> line{};
    while (!doQuit) {
        // int fd = open(fifo_name, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        // auto* fp = fdopen(fd, "re");
        auto* fp = fopen(fifo_name.c_str(), "re");
        char* rc = nullptr;
        while (rc == nullptr) {
            rc = fgets(line.data(), line.size(), fp);
            if (rc != nullptr) {
                run_command(line.data());
            }
        }
        fclose(fp);
    }
    remove(fifo_name.c_str());
    exit(0);

}
Server::Server(const std::string& fifo) : fifo_name(fifo) {
}
