#include "server.hpp"
#include <cstdio>
#include <fmt/core.h>
#include <fmt/format.h>
#include <sys/stat.h>

#include <coreutils/split.h>

void Server::run_command(const std::string& cmd)
{
    // fmt::print("Got line {}\n", ptr);
    if (cmd.find("kill") == 0) {
        doQuit = true;
        return;
    }
    if (cmd.find("say ") == 0) {
        auto line = cmd.substr(4);
        mqtt.publish(key + "/testuser/say", line);
    }
    if (cmd.find("incoming") == 0) {
        handle_incoming();
    }
    fmt::print("{}", cmd);
}

Server::~Server()
{
    fmt::print("Exit\n");
    if (fifo_fp != nullptr) {
        fclose(fifo_fp);
    }
    if (!fifo_name.empty()) {
        remove(fifo_name.c_str());
    }
    fmt::print("Done\n");
}

void Server::handle_incoming()
{
    while (true) {
        auto msg = mqtt.pop_message();
        if (!msg) {
            break;
        }

        auto parts = utils::split(msg.topic, "/");
        if (parts.size() >= 3) {
            if (parts[2] == "say") {
                fmt::print("{} : {}\n", parts[1], msg.text());
            }
        }
    }
}
void Server::run(const std::string& command)
{
    mkfifo(fifo_name.c_str(), 0666);
    mqtt.connect("localhost");
    mqtt.start();
    mqtt.subscribe(fmt::format("{}/#", key));
    if(!command.empty()) {
        run_command(command + "\n");
    }
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
    fifo_name = "";
}

void Server::write_pipe(std::string const& fifo_name, std::string const& what)
{
    auto* fp = fopen(fifo_name.c_str(), "ae");
    fputs((what + "\n").c_str(), fp);
    fclose(fp);
}

Server::Server(std::string const& fifo, std::string const& k)
    : fifo_name(fifo), key(k)
{
    fmt::print("Create {}\n", fifo);
}
