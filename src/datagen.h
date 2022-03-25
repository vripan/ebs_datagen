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

class thread_exit : std::exception
{
};

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
    const std::uint32_t CONSUMERS_COUNT = 0;

    nlohmann::json schema;
    settings_t &settings;

    std::vector<std::thread> workers;
    std::vector<std::thread> consumers;
    std::queue<task_t> q_in;
    std::queue<nlohmann::json*> q_out;
    std::mutex q_in_mtx;
    std::mutex q_out_mtx;
    std::atomic_int kill_consumers;

    std::vector<std::uint32_t> tasks_executed;

    std::mutex fout_mtx;
    std::ofstream fout;

    void worker_default_action(unsigned int id);
    void worker_publication_action();
    void worker_subscription_action();
    void consumer_default_action();
};