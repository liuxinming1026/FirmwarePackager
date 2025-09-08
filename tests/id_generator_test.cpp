#include <gtest/gtest.h>
#include "core/IdGenerator.h"

#include <regex>
#include <thread>
#include <unordered_set>
#include <mutex>

TEST(IdGeneratorTest, GeneratesExpectedFormat) {
    core::IdGenerator gen;
    std::string id = gen.generate();
    std::regex pattern(R"(\d{8}-\d{6}-[0-9a-f]{8})");
    EXPECT_TRUE(std::regex_match(id, pattern));
}

TEST(IdGeneratorTest, ProducesUniqueIds) {
    core::IdGenerator gen;
    std::unordered_set<std::string> ids;
    std::mutex m;
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < 50; ++i) {
                auto id = gen.generate();
                std::lock_guard<std::mutex> lock(m);
                ids.insert(std::move(id));
            }
        });
    }
    for (auto& th : threads) th.join();
    EXPECT_EQ(ids.size(), 200u);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

