
#include <coreutils/file.h>
#include <coreutils/path.h>

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include <mosquitto.h>

#include <csignal>
#include <fmt/core.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <vector>

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

    signal(SIGCHLD, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

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

template <typename T>
auto make_unique(T* p, void (*f)(T*))
{
    return std::unique_ptr<T, decltype(f)>(p, f);
}

class mqtt_exception : public std::exception
{
public:
    explicit mqtt_exception(std::string m = "IO Exception") : msg(std::move(m))
    {}
    const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};

struct Raw
{
    int x{};
};

std::vector<std::string> lines;
bool doQuit = false;

void run_command(std::string const& cmd)
{
    // fmt::print("Got line {}\n", ptr);
    if (cmd.find("kill") == 0) {
        doQuit = true;
        return;
    }
    lines.push_back(cmd);
    fmt::print("{}", cmd);
}

void run_server(const char* fifo_name, std::string const& command)
{
    mkfifo(fifo_name, 0666);
    daemonize();
    run_command(command + "\n");
    std::array<char, 256> line{};
    while (!doQuit) {
        auto* fp = fopen(fifo_name, "re");
        if (fgets(line.data(), line.size(), fp) != nullptr) {
            run_command(line.data());
        }
        fclose(fp);
    }
    remove(fifo_name);
    exit(0);
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
            return current;
        }
        current = current.parent_path();
    }
    return {};
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
        run_server(fifo_name.c_str(), argv[1]);
    } else {
        write_pipe(fifo_name.c_str(), argv[1]);
    }
}

void mqtt_setup()
{
    mosquitto_lib_init();

    auto* mqtt = mosquitto_new("myid", true, nullptr);
    auto mp = make_unique(mqtt, &mosquitto_destroy);

    mosquitto_connect_callback_set(mp.get(), [](auto* msq, void*, int) {
        fmt::print("We have connected\n");
        fmt::print("Publish\n");
        Raw raw{};
        auto rc = mosquitto_publish(msq, nullptr, "test/topic", sizeof(Raw),
                                    &raw, 1, false);

        if (rc != MOSQ_ERR_SUCCESS) {
            throw mqtt_exception("publish");
        }
    });

    auto rc = mosquitto_connect(mp.get(), "localhost", 1883, 10);
    if (rc != MOSQ_ERR_SUCCESS) {
        throw mqtt_exception("connect");
    }

    fmt::print("Loop\n");
    mosquitto_loop_forever(mp.get(), 10, 10);
    mosquitto_disconnect(mp.get());
}
