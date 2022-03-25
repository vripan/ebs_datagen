#include "datagen.h"

#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <random>
#include <mutex>

datagen::datagen(settings_t &settings, nlohmann::json &&schema)
    : settings(settings)
    , schema(std::move(schema))
    , kill_consumers({0})
    , fout("data.json")
{}

void datagen::worker_default_action(unsigned int id)
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
                spdlog::debug("thread exit /{}", id);
                return;
            }
            catch (std::exception &e)
            {
                spdlog::error("worker /{} exception: {}", id, e.what());
            }
            tasks_executed[id] += 1;
        }
    }
    spdlog::error("no attempts left, thread /{} exit", id);
}

void datagen::worker_publication_action()
{
    auto output = R"JSON({"publication" : "this"})JSON"_json;

    std::lock_guard g(fout_mtx);
    fout << output.dump(2) << ",\n";

    // std::lock_guard g(q_out_mtx);
    // q_out.push(output);
}

void datagen::worker_subscription_action()
{
    auto output = R"JSON({"subscription" : "this"})JSON"_json;

    std::lock_guard g(fout_mtx);
    fout << output.dump(2) << ",\n";

    // std::lock_guard g(q_out_mtx);
    // q_out.push(output);
}

void datagen::consumer_default_action()
{
    std::ofstream fout("data.json");
    fout << "[\n";

    // std::stringstream stream;

    while (true)
    {
        q_out_mtx.lock();

        if (q_out.empty())
        {
            if (kill_consumers)
            {
                fout << "{}\n]\n";
                spdlog::info("consumer exited");
                q_out_mtx.unlock();
                return;
            }
            q_out_mtx.unlock();
            std::this_thread::sleep_for(TASK_WAIT_TIMEOUT);
        }
        else
        {
            nlohmann::json *json = q_out.front();
            q_out.pop();
            q_out_mtx.unlock();

            // fout << json->dump(2) << ",\n";
            delete json;
        }
    }
}

void datagen::run()
{
    fout << "[\n";

    tasks_executed.insert(tasks_executed.begin(), settings.threads_count, 0);

    auto worker_action = [this](unsigned int id) -> void { this->worker_default_action(id); };
    auto consumer_action = [this](unsigned int id) -> void { this->consumer_default_action(); };

    for (auto idx = 0u; idx < settings.threads_count; idx++)
    {
        std::thread thread(worker_action, idx);
        workers.emplace_back(std::move(thread));
    }

    for (auto idx = 0u; idx < CONSUMERS_COUNT; idx++)
    {
        std::thread thread(consumer_action, idx);
        consumers.emplace_back(std::move(thread));
    }

    auto publications_count = schema["publications_count"].get<std::uint32_t>();
    spdlog::debug("adding publication tasks");

    for (auto idx = 0u; idx < publications_count; idx++)
    {
        std::lock_guard g(q_in_mtx);
        q_in.push([this]() { this->worker_publication_action(); });
    }

    spdlog::debug("successfully added plublication tasks");

    auto subscriptions_count = schema["subscriptions_count"].get<std::uint32_t>();
    spdlog::debug("adding subscriptions tasks");

    for (auto idx = 0u; idx < publications_count; idx++)
    {
        std::lock_guard g(q_in_mtx);
        q_in.push([this]() { this->worker_subscription_action(); });
    }

    spdlog::debug("successfully added subscriptions tasks");

    auto exit_action = []() { throw thread_exit(); };

    for (auto &thread : workers)
    {
        std::lock_guard g(q_in_mtx);
        q_in.push(exit_action);
    }

    kill_consumers = 1;

    for (auto &thread : workers)
    {
        thread.join();
    }

    for (auto &thread : consumers)
    {
        thread.join();
    }

    for (auto idx = 0u; idx < tasks_executed.size(); idx++)
    {
        spdlog::info("tid {} executed {} tasks", idx, tasks_executed[idx]);
    }

    fout << "{}\n]\n";
}