#pragma once

#include "file.h"
#include "log.h"
#include "path.h"
#include "split.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <sys/stat.h>
#include <utility>
#include <vector>

namespace utils {
/*
template <class T, template <typename DUMMY> class CONTAINER>
auto find(CONTAINER<T> const& haystack, T const& needle)
{
    return std::find(begin(haystack), end(haystack), needle);
}

template <class T, class FX, template <typename> class CONTAINER>
auto find_if(CONTAINER<T> const& haystack, FX const& fn)
{
    return std::find_if(begin(haystack), end(haystack), fn);
}
*/
template <typename ITERATOR>
std::string join(ITERATOR begin, ITERATOR end,
                 const std::string& separator = ", ")
{
    std::ostringstream ss;

    if (begin != end) {
        ss << *begin++;
    }

    while (begin != end) {
        ss << separator;
        ss << *begin++;
    }
    return ss.str();
}

template <typename C>
std::string join(C& c, const std::string& separator = ", ")
{
    return join(std::cbegin(c), std::cend(c), separator);
}

inline std::string spaces(int n)
{
    return std::string(n, ' ');
}

// Indent `text` by prefixing each line with `n` spaces
inline std::string indent(std::string const& text, int n)
{
    std::vector<std::string> target;
    auto v = split(text, "\n");
    std::transform(v.begin(), v.end(), std::back_inserter(target),
                   [prefix = spaces(n)](auto const& s) { return prefix + s; });
    return join(target.begin(), target.end(), "\n");
}

inline bool endsWith(const std::string& name, const std::string& ext)
{
    if (ext.empty()) return true;

    // avoid negative surprise in compare
    if (name.size() < ext.size()) return false;

    return name.compare(name.size() - ext.size(), ext.size(), ext) == 0;
}

inline bool startsWith(const std::string& name, const std::string& pref)
{
    if (pref.empty()) return true;

    return name.compare(0, pref.size(), pref) == 0;
}

inline void quote(std::string& s)
{
    s = "\"" + s + "\"";
}

inline bool isUpper(char const& c)
{
    return std::isupper(c) != 0;
}
inline bool isLower(char const& c)
{
    return std::islower(c) != 0;
}

inline bool isUpper(std::string const& s)
{
    return std::all_of(s.begin(), s.end(),
                       static_cast<bool (*)(char const&)>(&isUpper));
}

inline bool isLower(std::string const& s)
{
    return std::all_of(s.begin(), s.end(),
                       static_cast<bool (*)(char const&)>(&isLower));
}

inline std::string getUniqueKey()
{
    std::string result;
    const char* letters =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@_";
    char key[33];
    char* out = key;
    std::mt19937 gen{std::random_device{}()};
    std::uniform_int_distribution<size_t> dist{0, strlen(letters) - 1};
    for (int i = 0; i < 32; i++) {
        *out++ = letters[dist(gen)];
    }
    *out = 0;
    result = key;
    return result;
}

inline std::string getUniqueKey(utils::path const& p)
{
    std::string result;
    if (utils::exists(p)) {
        result = utils::File{p}.readAll();
        // LOGI("Read '{}' from {}", result, p.string());
        if (result.length() >= 32) {
            return result;
        }
    }
    result = getUniqueKey();

    utils::File f{p, utils::File::Mode::Write};
    f.writeString(result);
    return result;
}

inline int64_t currentTime()
{
    auto t = std::chrono::system_clock::now();
    return static_cast<int64_t>(std::chrono::system_clock::to_time_t(t));
}

inline std::string getHomeDir()
{
    std::string homeDir;
#if _WIN32
    char* userProfile = getenv("USERPROFILE");
    if (userProfile == nullptr) {

        char* homeDrive = getenv("HOMEDRIVE");
        char* homePath = getenv("HOMEPATH");

        if (homeDrive == nullptr || homePath == nullptr) {
            fprintf(stderr, "Could not get home directory");
            return "";
        }

        homeDir = std::string(homeDrive) + homePath;
    } else
        homeDir = std::string(userProfile);
#else
    homeDir = std::string(getenv("HOME"));
#endif
    return homeDir;
}

inline bool to_num(const char* start_ptr, const char* end_ptr, double& res)
{
    using namespace std::string_literals;
    char* err_ptr;
    std::string s{start_ptr, static_cast<size_t>(end_ptr - start_ptr)};
    res = std::strtod(s.c_str(), &err_ptr);
    return (err_ptr != s.c_str());
}

inline bool to_num(const char* start_ptr, const char* end_ptr, uint16_t& res)
{
    using namespace std::string_literals;
    char* err_ptr;
    std::string s{start_ptr, static_cast<size_t>(end_ptr - start_ptr)};
    auto long_res = std::strtol(s.c_str(), &err_ptr, 0);
    if (long_res > std::numeric_limits<uint16_t>::max() ||
        long_res < std::numeric_limits<uint16_t>::min())
        return false;

    res = static_cast<uint16_t>(long_res);

    return (err_ptr != s.c_str());
}

inline bool to_num(const char* start_ptr, const char* end_ptr, int64_t& res)
{
    using namespace std::string_literals;
    char* err_ptr;
    std::string s{start_ptr, static_cast<size_t>(end_ptr - start_ptr)};
    res = std::strtoll(s.c_str(), &err_ptr, 0);
    return (err_ptr != s.c_str());
}

template <int BASE = 0>
inline bool to_num(const char* start_ptr, const char* end_ptr, int& res)
{
    using namespace std::string_literals;
    char* err_ptr;
    std::string s{start_ptr, static_cast<size_t>(end_ptr - start_ptr)};
    res = static_cast<int>(std::strtol(s.c_str(), &err_ptr, BASE));
    return (err_ptr != s.c_str());
}

template <typename Numeric = int64_t, typename Str>
inline bool to_num(Str str, Numeric* target)
{
    return to_num(str.data(), str.data() + str.size(), *target);
}

template <typename Numeric = int64_t, typename Str>
inline Numeric to_num(Str str, Numeric const& def)
{
    Numeric res;
    if (!to_num(str.data(), str.data() + str.size(), res)) {
        return def;
    }
    return res;
}

template <typename Numeric = int64_t, typename Str>
inline Numeric to_num(Str str)
{
    using namespace std::string_literals;
    Numeric res;
    if (!to_num(str.data(), str.data() + str.size(), res)) {
        throw std::invalid_argument{"Could not convert "s +
                                    std::string(str.data(), str.size())};
    }
    return res;
}
} // namespace utils
