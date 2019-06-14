#include "location.h"
#include "weather.h"
#include "gtest/gtest.h"
#include <cpprest/http_client.h>
#include <cpprest/uri_builder.h>
#include <fstream>
#include <memory>

#include "FetchURL.h"

using namespace std;
using namespace web::http;
using namespace web::http::client;
using namespace nlohmann;

//Our json test data;
string wdata;
tuple<bool, string> FetchURL(string url, uri_builder builder) {
    return {false, wdata};
}

static const char *DARKSKY_KEY = "3e0b34c68d70bef67e1e843840139d6c";

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

class weatherTest : public testing::Test {
  protected:
    unique_ptr<Weather> weather;
    weatherTest() {
        weather = make_unique<Weather>(DARKSKY_KEY);
    }

    virtual ~weatherTest() {
    }

    virtual void SetUp() override {
        wdata.clear();
        std::ifstream i("barf.json");
        std::stringstream buffer;
        buffer << i.rdbuf();
        wdata = buffer.str();
    }
};

TEST_F(weatherTest, TestSummary) {
    Location l;
    l.lat = "1";
    l.lon = "2";
    l.name = "TestName";
    weather->Lookup(l);
    //cout << weather->GetIRC() << endl;

    ASSERT_EQ(weather->GetIRC(), "(TestName) Currently Clear 34.6°F (Feels Like 31.2°F), Cloud Cover: 0%, Humidity: 53%, Wind: N @ 3.9 mph (5.7 mph Gusts)");
    //weather.w["currently"]["summary"] = "Partly Cloudy";
    //ASSERT_EQ(weather.out["cond"].get<string>(), "Clear");
    //weather.CurCond();
    //ASSERT_EQ(weather.out["cond"].get<string>(), "Partly Cloudy");
}
