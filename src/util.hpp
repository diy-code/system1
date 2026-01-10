#pragma once
#include <string>
#include <chrono>
#include <sstream>
#include <algorithm>

inline std::string now_timestamp() {
    using namespace std::chrono;
    auto t = system_clock::now();
    auto tt = system_clock::to_time_t(t);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buf);
}

inline std::string short_id(const std::string &s) {
    return s.substr(0, std::min<size_t>(8, s.size()));
}
