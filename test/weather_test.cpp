#include "../location.hpp"
#include "../weather.hpp"
#include "gtest/gtest.h"
#include <cpprest/http_client.h>
#include <cpprest/uri_builder.h>
#include <fstream>
#include <memory>

#include "../FetchURL.hpp"
#include <nlohmann/json.hpp>

//Our json test data;
std::string wdata;
std::tuple<bool, std::string> FetchURL(std::string url, web::uri_builder builder) {
    (void)builder; //Silence unused parameter
    return {false, wdata};
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

class weatherTest : public testing::Test {
  protected:
    std::unique_ptr<Weather> weather;
    weatherTest() {
        weather = std::make_unique<Weather>("key");
    }

    virtual ~weatherTest() {
    }

    virtual void SetUp() override {
        wdata.clear();
        std::ifstream i("../test/input/weather.json");
        std::stringstream buffer;
        buffer << i.rdbuf();
        wdata = buffer.str();
    }
};

TEST_F(weatherTest, TestSummary) {
    std::shared_ptr<Location> l = std::make_shared<Location>("key");
    l->lat = "1";
    l->lon = "2";
    l->name = "TestName";
    weather->Lookup(l);
    ASSERT_EQ(weather->GetIRC(), "(TestName) Currently Clear 34.6°F (Feels Like 31.2°F), Cloud Cover: 0%, Humidity: 53%, Wind: N @ 3.9 mph (5.7 mph Gusts)");
    
    auto j = nlohmann::json::parse(wdata);
    j["currently"]["summary"] = "Horrible Tornadoes";
    wdata = (std::string)j.dump();
    weather->Lookup(l);
    ASSERT_EQ(weather->GetIRC(), "(TestName) Currently Horrible Tornadoes 34.6°F (Feels Like 31.2°F), Cloud Cover: 0%, Humidity: 53%, Wind: N @ 3.9 mph (5.7 mph Gusts)");
    //weather.w["currently"]["summary"] = "Partly Cloudy";
    //ASSERT_EQ(weather.out["cond"].get<string>(), "Clear");
    //weather.CurCond();
    //ASSERT_EQ(weather.out["cond"].get<string>(), "Partly Cloudy");
}
