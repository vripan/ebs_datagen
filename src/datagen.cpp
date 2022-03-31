#include "datagen.h"

#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <string>
#include <mutex>

using namespace std;
using namespace nlohmann;

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator &g)
{
    std::uniform_int_distribution<> dis(0, static_cast<int>(std::distance(start, end)) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(std::mt19937 &gen, Iter start, Iter end)
{
    return select_randomly(start, end, gen);
}

double select_randomly(std::mt19937 &gen, double l1, double l2)
{
    const std::uniform_real_distribution<> distr(l1, l2);
    return distr(gen);
}

double round_up(double value, int decimal_places)
{
    const double multiplier = std::pow(10.0, decimal_places);
    return std::ceil(value * multiplier) / multiplier;
}

SubscriptionManager::SubscriptionManager(nlohmann::json &schema, int subscriptions_count)
{
    for (auto &[name, object] : schema["schema"].items())
    {
        auto type = object["type"].get<string>();

        sub_fields[name].occurence_left = subscriptions_count;

        if (object.contains("occurence_percentage"))
            sub_fields[name].occurence_left =
                static_cast<int>(std::ceil(object["occurence_percentage"].get<double>() * subscriptions_count));
        else
            sub_fields[name].occurence_left = subscriptions_count;

        if (object.contains("equal_frequency"))
            sub_fields[name].equal_left =
                static_cast<int>(std::ceil(object["equal_frequency"].get<double>() * sub_fields[name].occurence_left));

        if (type == "string")
        {
            sub_fields[name].type = SubFieldType::STRING;
            sub_fields[name].values = new auto(object["values"].get<std::vector<std::string>>());
        }
        else if (type == "date")
        {
            sub_fields[name].type = SubFieldType::DATE;
            sub_fields[name].values = new auto(object["values"].get<std::vector<std::string>>());
        }
        else if (type == "double")
        {
            sub_fields[name].type = SubFieldType::DOUBLE;
            sub_fields[name].interval = new auto(object["interval"].get<vector<double>>());
        }
        else
            throw std::exception("schema type not found");
    }
}

std::string SubscriptionManager::generateField(std::mt19937 &gen, SubField &field)
{
    std::string op = "";
    {
        // ----- START LOCK -----
        std::lock_guard<std::mutex> __guard(field._mutex);

        if (!field.occurence_left)
            return "";
        --field.occurence_left;

        // get operator
        if (field.equal_left)
        {
            op = "=";
            --field.equal_left;
        }
        // ----- END LOCK -----
    }

    switch (field.type)
    {
    case SubFieldType::DOUBLE: {
        if (op == "")
            op = OPERATORS::numbers_ops[std::floor(select_randomly(gen, 0, OPERATORS::numbers_ops.size()))];
        return op + ", " + std::to_string(round_up(select_randomly(gen, field.interval->at(0), field.interval->at(1)), 2));
    }
    case SubFieldType::STRING:
    case SubFieldType::DATE: {
        if (op == "")
            op = OPERATORS::strings_ops[std::floor(select_randomly(gen, 0, OPERATORS::strings_ops.size()))];
        return op + ", " + field.values->at(std::floor(select_randomly(gen, 0, field.values->size())));
    }
    default:
        throw std::exception("invalid field type!");
    }
}

nlohmann::json SubscriptionManager::generateSub(std::mt19937 &gen)
{
    json out;
    for (auto &[key, value] : sub_fields)
    {
        auto generatedField = generateField(gen, value);
        if (generatedField != "")
            out[key] = generatedField;
    }
    return out;
}

datagen::datagen(settings_t &settings, nlohmann::json &&schema)
    : settings(settings)
    , schema(std::move(schema))
    , tasks_count({0})
    , fout("data.json")
{
    publications_count = this->schema["publications_count"].get<std::uint32_t>();
    subscriptions_count = this->schema["subscriptions_count"].get<std::uint32_t>();

    this->subsManager = new SubscriptionManager(this->schema, subscriptions_count);

    spdlog::info("input size: {} publications and {} subscriptions", publications_count, subscriptions_count);
}

datagen::task_id datagen::get_task()
{
    auto task = tasks_count.fetch_sub(1) - 1;

    if (task < 0)
        return task_id::stop;

    if (task < (int)subscriptions_count)
        return task_id::gen_sub;
    return task_id::gen_pub;
}

void datagen::worker_default_action(unsigned int id)
{
    auto &task_counter = tasks_executed[id];

    std::string filename = "data_" + std::to_string(id) + ".json";
    std::ofstream fout(filename);
    fout << "[\n";

    std::random_device rd;
    std::mt19937 gen(rd());

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
            worker_subscription_action(id, fout, gen);
            break;
        case task_id::gen_pub:
            worker_publication_action(id, fout, gen);
            break;
        }
        task_counter += 1;
    }
}

json generate_type(json specification, std::mt19937 &gen)
{
    auto type = specification["type"].get_ref<string &>();
    if (type == "string" || type == "date")
    {
        auto values = specification["values"].items();
        return select_randomly(gen, values.begin(), values.end()).value();
    }
    if (type == "double")
    {
        auto interval = specification["interval"].get<vector<double>>();
        return round_up(select_randomly(gen, interval[0], interval[1]), 2);
    }
    throw std::exception("schema type not found");
}

void datagen::worker_publication_action(unsigned int /*id*/, std::ofstream &fout, std::mt19937 &gen)
{
    json output;

    for (auto &[name, object] : schema["schema"].items())
    {
        output[name] = generate_type(object, gen);
    }

    fout << output.dump(2) << ",\n";
}

void datagen::worker_subscription_action(unsigned int id, std::ofstream &fout, std::mt19937 &gen)
{
    auto output = this->subsManager->generateSub(gen);

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