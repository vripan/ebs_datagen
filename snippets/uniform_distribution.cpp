void test(unsigned size) {
    unsigned int data[2] = {};


    for (unsigned idx = 0; idx < size; idx++)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> distrib(0, 1);

        auto v = distrib(gen) > 0.8;
        data[v] += 1;
    }

    spdlog::info("size:{}, b[0]={}, b[1]={}", size, (double)data[0] / size, (double)data[1] / size);
}