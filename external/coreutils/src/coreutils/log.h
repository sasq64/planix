#pragma once

#include <fmt/format.h>

#include <cstring>
#include <string>

namespace logging {

enum Level
{
    Unset = 0,
    Verbose,
    Debug,
    Info,
    Warning,
    Error,
    Off = 100
};

#ifdef COREUTILS_LOGGING_SIMPLE

inline void log2(const char* fn, int line, const Level level,
                 const std::string& text)
{
    if (level >= Info) printf("[%s:%d] %s\n", fn, line, text.c_str());
}

#else

void log(const std::string& text);
void log(Level level, const std::string& text);
void log2(const char* fn, int line, Level level, const std::string& text);

#endif

template <class... A>
void log(const std::string& fmt, const A&... args)
{
    log(fmt::format(fmt, args...));
}

template <class... A>
void log(Level level, const std::string& fmt, const A&... args)
{
    log(level, fmt::format(fmt, args...));
}

template <class... A>
void log2(const char* fn, int line, Level level, const std::string& fmt,
          const A&... args)
{
    log2(fn, line, level, fmt::format(fmt, args...));
}

void setLevel(Level level);
void setOutputFile(const std::string& fileName);

inline constexpr const char* base(const char* x)
{
    const char* slash = x;
    while (*x != 0) {
        if (*x++ == '/') {
            slash = x;
        }
    }
    return slash;
}

#ifdef COREUTILS_LOGGING_DISABLE

#    define LOGV(...)
#    define LOGD(...)
#    define LOGI(...)
#    define LOGW(...)
#    define LOGE(...)

#else

#    define LOGV(...)                                                          \
        logging::log2(logging::base(__FILE__), __LINE__, logging::Verbose,     \
                      __VA_ARGS__)
#    define LOGD(...)                                                          \
        logging::log2(logging::base(__FILE__), __LINE__, logging::Debug,       \
                      __VA_ARGS__)
#    define LOGI(...)                                                          \
        logging::log2(logging::base(__FILE__), __LINE__, logging::Info,        \
                      __VA_ARGS__)
#    define LOGW(...)                                                          \
        logging::log2(logging::base(__FILE__), __LINE__, logging::Warning,     \
                      __VA_ARGS__)
#    define LOGE(...)                                                          \
        logging::log2(logging::base(__FILE__), __LINE__, logging::Error,       \
                      __VA_ARGS__)
#endif
} // namespace logging
