#include "datagen.h"

#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <random>
#include <mutex>

datagen::datagen(settings_t &settings, nlohmann::json &&schema)
    : settings(settings)
    , schema(std::move(schema))
    , tasks_count({0})
    , fout("data.json")
{
    publications_count = this->schema["publications_count"].get<std::uint32_t>();
    subscriptions_count = this->schema["subscriptions_count"].get<std::uint32_t>();

    spdlog::info("input size: {} publications and {} subscriptions", publications_count, subscriptions_count);
}

datagen::task_id datagen::get_task()
{
    auto task = tasks_count.fetch_sub(1) - 1;

    if (task < 0)
        return task_id::stop;

    if (task < (int)publications_count)
        return task_id::gen_pub;
    return task_id::gen_sub;
}

void datagen::worker_default_action(unsigned int id)
{
    auto &task_counter = tasks_executed[id];

    std::string filename = "data_" + std::to_string(id) + ".json";
    std::ofstream fout(filename);
    fout << "[\n";

    while (true)
    {
        auto task = get_task();
        switch (task)
        {
        case task_id::stop: {
            fout << "{}\n]\n";

            double total_tasks = publications_count + subscriptions_count;
            spdlog::info("tid /{} executed {} tasks ({:.3}%)", id, task_counter, ((double)task_counter / total_tasks * 100.0));

            spdlog::debug("thread exit /{}", id);
            return;
        }
        case task_id::gen_sub:
            worker_subscription_action(id, fout);
            break;
        case task_id::gen_pub:
            worker_publication_action(id, fout);
            break;
        }
        task_counter += 1;
    }
}

void datagen::worker_publication_action(unsigned int id, std::ofstream &fout)
{
    auto output = R"JSON({"publication" : "this"})JSON"_json;

    fout << output.dump(2) << ",\n";
}

void datagen::worker_subscription_action(unsigned int id, std::ofstream &fout)
{
    auto output = R"JSON({"subscription" : "this"})JSON"_json;

    fout << output.dump(2) << ",\n";
}

void datagen::spawn_threads()
{
    tasks_executed.insert(tasks_executed.begin(), settings.threads_count, 0);

    auto worker_default_action = [this](unsigned int id) -> void { this->worker_default_action(id); };

    for (auto idx = 0u; idx < settings.threads_count; idx++)
    {
        std::thread thread(worker_default_action, idx);
        workers.emplace_back(std::move(thread));
    }
}

void datagen::end_threads()
{
    for (auto &thread : workers)
    {
        thread.join();
    }
}

void datagen::run()
{
    tasks_count = publications_count + subscriptions_count;

    spawn_threads();
    end_threads();
}