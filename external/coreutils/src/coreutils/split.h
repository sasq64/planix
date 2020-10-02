#pragma once

#include <algorithm>
#include <cstring>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace utils {

class StringSplit
{
    std::string source;
    std::vector<std::string_view> views;
    char* ptr;
    const int minSplits = -1;

    void split(const char delim)
    {
        while (true) {
            auto* next = strchr(ptr, delim);
            if (next == nullptr) {
                views.emplace_back(ptr, strlen(ptr));
                break;
            }
            views.emplace_back(ptr, next - ptr);
            ptr = next+1;
        }
    }

    void split(const char* delim)
    {
        const auto dz = strlen(delim);
        while (true) {
            auto* next = strstr(ptr, delim);
            if (next == nullptr) {
                views.emplace_back(ptr, strlen(ptr));
                break;
            }
            views.emplace_back(ptr, next - ptr);
            ptr = next + dz;

        }
    }

public:
    template <typename T>
    StringSplit(T&& text, std::string const& delim, int minSplits = -1)
        : source(std::forward<T>(text)), ptr(&source[0]), minSplits(minSplits)
    {
        split(delim.c_str());
    }

    template <typename T>
    StringSplit(T&& text, const char delim, int minSplits = -1)
        : source(std::forward<T>(text)), ptr(&source[0]), minSplits(minSplits)
    {
        split(delim);
    }

    size_t size() const { return views.size(); }
    auto begin() const { return views.begin(); }
    auto end() const { return views.end(); }

    std::string_view operator[](size_t n) const
    {
        return views[n];
    }

    std::string getString(size_t n) const
    {
        static std::string empty;
        return n < size() ? std::string(views[n]) : empty;
    }
    operator bool() const { return minSplits < 0 || (int)size() >= minSplits; }

    /* operator std::vector<std::string>() const */
    /* { */
    /*     std::vector<std::string> result; */
    /*     std::copy(views.begin(), views.end(), std::back_inserter(result)); */
    /*     return result; */
    /* } */
};

template <typename T, typename S>
inline StringSplit split(T&& s, S const& delim, int minSplits = -1)
{
    return StringSplit(std::forward<T>(s), delim, minSplits);
}

template <size_t... Is>
auto gen_tuple_impl(const StringSplit& ss, std::index_sequence<Is...>)
{
    return std::make_tuple(ss.getString(Is)...);
}

template <size_t N>
auto gen_tuple(const StringSplit& ss)
{
    return gen_tuple_impl(ss, std::make_index_sequence<N>{});
}

template <size_t N, typename T>
auto splitn(const std::string& text, const T& sep)
{
    return gen_tuple<N>(split(text, sep));
}

template <typename T, typename S>
std::pair<T, T> split2(const T& text, const S& sep)
{
    auto it = std::search(std::begin(text), std::end(text), std::begin(sep),
                          std::end(sep));
    if (it == std::end(text)) return std::make_pair(text, T());
    auto it2 = it;
    std::advance(it2, std::distance(std::begin(sep), std::end(sep)));
    return std::make_pair(T(std::begin(text), it), T(it2, std::end(text)));
}

template <typename T>
std::pair<std::string, std::string> split2(const char* text, const T& sep)
{
    return split2<std::string, T>(text, sep);
}

struct URL
{
    std::string protocol;
    std::string hostname;
    int port = -1;
    std::string path;
};

inline URL parse_url(std::string const& input)
{
    URL url;
    auto parts = split(input, "://");

    if (parts.size() != 2) throw std::exception();

    url.protocol = parts[0];

    auto slash = parts[1].find_first_of('/');
    if (slash == std::string::npos) {
        url.hostname = parts[1];
        return url;
    }
    url.path = parts[1].substr(slash + 1);
    url.hostname = parts[1].substr(0, slash);

    auto colon = url.hostname.find_last_of(':');
    if (colon != std::string::npos) {
        url.port = std::atoi(url.hostname.substr(colon + 1).c_str());
        url.hostname = url.hostname.substr(0, colon);
    }

    return url;
}

} // namespace utils
