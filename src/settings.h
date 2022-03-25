#pragma once

#include <spdlog/spdlog.h>

#include <string>
#include <string_view>

struct settings_t
{
    std::uint32_t threads_count = 1;
    spdlog::level::level_enum log_level = spdlog::level::info;
    std::string schema_path;

    void set_log_level(std::string_view new_value);
};

inline void settings_t::set_log_level(std::string_view new_value)
{
    if (new_value.empty())
    {
        log_level = spdlog::level::debug;
        return;
    }

    char id = new_value[0];

    switch (id)
    {
    case 't':
        log_level = spdlog::level::trace;
        return;
    case 'd':
        log_level = spdlog::level::debug;
        return;
    case 'i':
        log_level = spdlog::level::info;
        return;
    case 'w':
        log_level = spdlog::level::warn;
        return;
    case 'e':
        log_level = spdlog::level::err;
        return;
    case 'c':
        log_level = spdlog::level::critical;
        return;
    case 'o':
        log_level = spdlog::level::off;
        return;
    }

    spdlog::error("invalid logging level");
    throw std::invalid_argument("invalid new log level");
}