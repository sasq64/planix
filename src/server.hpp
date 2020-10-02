#pragma once

#include "mqtt.hpp"

#include <cstdio>
#include <string>

class Server
{
    MQTT mqtt;
    std::string fifo_name;
    std::string key;
    bool doQuit = false;
    FILE* fifo_fp = nullptr;

    void handle_incoming();

public:
    explicit Server(std::string const& fifo, std::string const &key);
    ~Server();
    void run_command(std::string const& cmd);
    void run(std::string const& command);

    static void write_pipe(std::string const& fifo_name, std::string const& what);
};

