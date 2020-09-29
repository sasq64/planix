#pragma once

#include "mqtt.hpp"
#include <string>

class Server
{
    MQTT mqtt;
    std::string fifo_name;
    std::string key;
    bool doQuit = false;

    void handle_incoming();

public:
    explicit Server(std::string const& fifo);
    void run_command(std::string const& cmd);
    void run(std::string const& command);
};

