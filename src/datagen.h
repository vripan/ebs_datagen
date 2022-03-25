#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <functional>
#include <chrono>

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

    nlohmann::json schema;
    settings_t &settings;

    std::vector<std::thread> workers;
    std::vector<std::thread> consumers;
    std::queue<task_t> q_in;
    std::queue<task_t> q_out;
    std::mutex q_in_mtx;
    std::mutex q_out_mtx;

    void worker_default_action();
    void worker_exit_action();
    void worker_hi_action();
};