#include <spdlog/spdlog.h>
#include <lyra/lyra.hpp>
#include <nlohmann/json.hpp>

#include <string>
#include <string_view>
#include <functional>
#include <fstream>
#include <vector>

#include "settings.h"
#include "datagen.h"

settings_t parse_args(int argc, char **argv) noexcept
{
    settings_t settings;

    bool show_help = false;
    std::string logging_level;

    auto cli = lyra::cli() | lyra::help(show_help) | lyra::opt(settings.threads_count, "threads")["--threads"]("number of threads") |
               lyra::opt(settings.schema_path, "schema")["--schema"]("path to schema file") |
               lyra::opt(logging_level, "trace|debug|info|warn|err|critical|off")["--log"]("log level")
                   .choices("trace", "debug", "info", "warn", "err", "critical", "off");

    auto result = cli.parse({argc, argv});
    if (!result)
    {
        spdlog::error("failed to parse cmdline args");
        spdlog::error(result.message());
        exit(1);
    }

    if (show_help)
    {
        std::cout << cli << std::endl;
        exit(0);
    }

    if (settings.schema_path.empty())
    {
        spdlog::error("no schema file provided");
        std::cout << std::endl << cli << std::endl;
        exit(1);
    }

    settings.set_log_level(logging_level);

    return settings;
}

nlohmann::json load_schema(settings_t &settings)
{
    std::ifstream raw_schema(settings.schema_path, std::fstream::binary | std::fstream::ate);

    if (!raw_schema.is_open())
        throw std::runtime_error("falied to open schema file");

    auto file_size = static_cast<std::int32_t>(raw_schema.tellg());

    if (file_size <= 0)
        throw std::runtime_error("get schema size failed");

    raw_schema.seekg(0, std::fstream::beg);

    std::vector<char> data(file_size + 1);
    raw_schema.read(data.data(), file_size);
    data[file_size] = '\0';

    if (!raw_schema)
    {
        throw std::runtime_error("failed to read schema");
    }

    auto schema = nlohmann::json::parse(data.data());
    spdlog::info("schema loaded, {} bytes", data.size());
    return schema;
}

int main(int argc, char **argv)
{
    spdlog::set_pattern("%H:%M:%S.%e - %^%5l%$ - thread %t - %v");
    spdlog::info("running ebs data generator");

    auto settings = parse_args(argc, argv);

    spdlog::set_level(settings.log_level);
    spdlog::info("schema file \"{}\"", settings.schema_path);
    spdlog::info("running on {} threads", settings.threads_count);

    try
    {
        auto schema = load_schema(settings);

        datagen generator(settings, std::move(schema));

        auto t1 = std::chrono::high_resolution_clock::now();
        generator.run();
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        spdlog::info("data generation duration {} ms", duration);
    }
    catch (std::exception &e)
    {
        spdlog::error("exception: {}", e.what());
        exit(1);
    }

    return 0;
}