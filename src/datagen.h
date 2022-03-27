#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <functional>
#include <chrono>
#include <fstream>

#include <nlohmann/json.hpp>

#include "settings.h"

using namespace std::chrono_literals;

class datagen
{
public:
    datagen(settings_t &settings, nlohmann::json &&schema);

    datagen(const datagen &) = delete;
    datagen(datagen &&) = delete;

    datagen &operator=(const datagen &) = delete;
    datagen &operator=(datagen &&) = delete;

    void run();

private:
    typedef std::function<void()> task_t;

    const std::chrono::seconds TASK_WAIT_TIMEOUT = 2s;
    const std::uint32_t TASK_TIMEOUT_ATTEMPTS = 3;

    enum task_id
    {
        gen_sub = 0,
        gen_pub,
        stop
    };

    nlohmann::json schema;
    settings_t &settings;

    std::vector<std::thread> workers;
    std::vector<std::uint32_t> tasks_executed;

    std::atomic_int tasks_count;
    std::uint32_t publications_count;
    std::uint32_t subscriptions_count;

    std::mutex fout_mtx;
    std::ofstream fout;

    void worker_default_action(unsigned int id);
    void worker_publication_action(unsigned int id, std::ofstream& fout);
    void worker_subscription_action(unsigned int id, std::ofstream& fout);
    
    void spawn_threads();
    void end_threads();

    task_id get_task();
};