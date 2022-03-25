#include "datagen.h"

#include <spdlog/spdlog.h>
#include <random>
#include <mutex>

datagen::datagen(settings_t &settings, nlohmann::json &&schema)
    : settings(settings)
    , schema(std::move(schema))
{}

void datagen::worker_default_action()
{
    auto attempts = TASK_TIMEOUT_ATTEMPTS;

    while (attempts)
    {
        q_in_mtx.lock();
        if (q_in.empty())
        {
            q_in_mtx.unlock();
            std::this_thread::sleep_for(TASK_WAIT_TIMEOUT);
            attempts -= 1;
        }
        else
        {
            // performance improvement: allocate tasks on heap to avoid copy
            task_t task = q_in.front();
            q_in.pop();
            q_in_mtx.unlock();

            try
            {
                task();
            }
            catch (thread_exit &)
            {
                spdlog::debug("thread exit");
                return;
            }
            catch (std::exception &e)
            {
                spdlog::error("worker exception: {}", e.what());
            }
        }
    }
}

void datagen::run()
{
    auto worker_action = [this]() -> void { this->worker_default_action(); };

    for (auto idx = 0; idx < settings.threads_count; idx++)
    {
        std::thread thread(worker_action);
        workers.emplace_back(std::move(thread));
    }

    auto exit_action = []() { throw thread_exit(); };

    auto hi_action = []() { spdlog::debug("heilo"); };

    for (auto idx = 0; idx < 100; idx++)
    {
        std::lock_guard g(q_in_mtx);
        q_in.push(hi_action);
    }

    for (auto &thread : workers)
    {
        std::lock_guard g(q_in_mtx);
        q_in.push(exit_action);
    }

    for (auto &thread : workers)
    {
        thread.join();
    }

    

}