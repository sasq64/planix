

#include <coreutils/file.h>
#include <coreutils/path.h>

#include "server.hpp"

#include <fmt/core.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>

using namespace std::string_literals;

void daemonize()
{
    auto pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Error: fork() failed\n");
        exit(1);
    }
    if (pid > 0) {
        // We are in parent, exit
        exit(0);
    }

    // This is the child process
    setsid();

    close(STDIN_FILENO);
    close(STDERR_FILENO);
    umask(027);

    // chdir(dir);

    signal(SIGCHLD, SIG_IGN); // NOLINT
    signal(SIGTSTP, SIG_IGN); // NOLINT
    signal(SIGTTOU, SIG_IGN); // NOLINT
    signal(SIGTTIN, SIG_IGN); // NOLINT

    auto signal_handler = [](int sig) {
        switch (sig) {
        case SIGHUP:
            break;
        case SIGTERM:
            exit(0);
        default:
            break;
        }
    };

    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
}


void write_pipe(const char* fifo_name, std::string const& what)
{
    auto* fp = fopen(fifo_name, "ae");
    fputs((what + "\n").c_str(), fp);
    fclose(fp);
}

static utils::path findPlanix()
{
    auto current = utils::absolute(".");

    while (!current.empty()) {
        if (utils::exists(current / ".planix")) {
            return current / ".planix";
        }
        current = current.parent_path();
    }
    return {};
}

void run_server(std::string const& fifo_name, std::string const& cmd)
{
   daemonize(); // Parent pid exits here
   Server server{fifo_name};
   server.run(cmd);
}

static void err(std::string_view const& msg)
{
    fmt::print(stderr, "Error: {}\n", msg);
    exit(1);
}

int main(int argc, const char* argv[])
{
    if (argc < 2) return -1;
    std::string home = getenv("HOME");
    auto fifo_name = home + "/" + ".planix_fifo";

    auto planix_path = findPlanix();

    if (planix_path.empty()) {
        err("Could not find .planix");
    }

    auto planix_dir = planix_path.parent_path();

    utils::File keyFile = utils::File{planix_path};
    auto key = keyFile.readLine();

    if (key.empty()) {
        err("keyFile contains no key");
    }

    chdir(planix_dir.c_str());

    struct stat ss; // NOLINT
    if (stat(fifo_name.c_str(), &ss) == -1) {
        run_server(fifo_name, argv[1]);
    } else {
        write_pipe(fifo_name.c_str(), argv[1]);
    }
}
