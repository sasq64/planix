#include "log.h"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <mutex>
#include <unistd.h>

static std::string logSource = "ESDK-Native";

#if __APPLE__
#    include "TargetConditionals.h"
#    if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
#        define IOS_TARGET
#    endif
#endif

#ifdef ANDROID

#    include <android/log.h>
#    define LOG_PUTS(x)                                                        \
        ((void)__android_log_print(ANDROID_LOG_INFO, logSource.c_str(), "%s",  \
                                   x))
#elif defined(IOS_TARGET)
void nsLogMessage(const char* msg);
#    define LOG_PUTS(x) nsLogMessage(x);

#else

#    ifdef LOG_INCLUDE
#        include LOG_INCLUDE
#    endif

#    ifndef LOG_PUTS // const char *
#        define LOG_PUTS(x) (puts(x), fflush(stdout))
#    endif

#endif
namespace logging {

std::atomic<Level> defaultLevel{Unset};
static std::atomic<FILE*> logFile{nullptr};

void log(const std::string& text)
{
    log(defaultLevel, text);
}

static std::mutex logm;


#ifdef __APPLE__
static std::atomic<int> threadId{-1};
#else
static thread_local int threadId = -1;
#endif
static std::atomic<int> threadCount{0};

void log(Level level, const std::string& text)
{
    static const char* levelChar = "VDIWE";
    if (defaultLevel == Level::Unset) {
        defaultLevel = Level::Info;
        const char* l = std::getenv("COREUTILS_LOG_LEVEL");
        if (l != nullptr) {
            auto c = std::toupper(*l);
            switch (c) {
            case 'V':
                defaultLevel = Level::Verbose;
                break;
            case 'D':
                defaultLevel = Level::Debug;
                break;
            case 'I':
                defaultLevel = Level::Info;
                break;
            case 'W':
                defaultLevel = Level::Warning;
                break;
            case 'E':
                defaultLevel = Level::Error;
                break;
            default:
                fprintf(stderr, "Warn: Unknown log level '%s'", l);
                break;
            }
        }
    }

    if (level >= defaultLevel) {
        const char* cptr = text.c_str();
        std::lock_guard<std::mutex> lock(logm);
        LOG_PUTS(cptr);
    }
    if (logFile != nullptr) {

        if (threadId == -1) {
            threadId = threadCount++;
        }

        std::lock_guard<std::mutex> lock(logm);

        time_t now = time(nullptr);
        struct tm* t = localtime(&now);
        std::string ts;
        if (threadCount > 1) {
            ts =
                fmt::format("%c: {:02d}:{:02d}.{:02d} -#{}- ", levelChar[level],
                            t->tm_hour, t->tm_min, t->tm_sec, threadId);
        } else {
            ts = fmt::format("%c: {:02d}:{:02d}.{:02d} - ", levelChar[level],
                             t->tm_hour, t->tm_min, t->tm_sec);
        }

        fwrite(ts.c_str(), 1, ts.length(), logFile);
        fwrite(text.c_str(), 1, text.length(), logFile);
        char c = text[text.length() - 1];
        if (c != '\n' && c != '\r') {
            putc('\n', logFile);
        }
        fflush(logFile);
    }
}

void log2(const char* fn, int line, Level level, const std::string& text)
{
    enum class TermType
    {
        UnSet,
        Plain,
        Color
    };
    static std::atomic<TermType> termType{TermType::UnSet};

    if (termType == TermType::UnSet) {
        // Check if we should print in color
        termType = TermType::Plain;
        if (std::getenv("COREUTILS_NO_COLOR") == nullptr &&
            isatty(fileno(stdout)) == 1) {
            const char* tt = getenv("TERM");
            if (tt != nullptr && strncmp(tt, "xterm", 5) == 0) {
                termType = TermType::Color;
            }
        }
    }

    std::string temp;
    if (termType == TermType::Color) {
        // Simple hash to generate color from filename
        int cs = 0;
        for (size_t i = 0; i < strlen(fn); i++) {
            cs = cs ^ fn[i];
        }
        cs = (cs % 6) + 1;

        temp = fmt::format("\x1b[{}m[{}:{}]\x1b[{}m ", cs + 30, fn, line, 39);
    } else {
        temp = fmt::format("[{}:{}] ", fn, line);
    }
    log(level, temp.append(text));
}

void setLevel(Level level)
{
    defaultLevel = level;
}

void setOutputFile(const std::string& fileName)
{
    std::lock_guard<std::mutex> lock(logm);
    if (logFile != nullptr) {
        fclose(logFile);
    }
    logFile = fopen(fileName.c_str(), "wbe");
}

} // namespace logging
