#include "../location.hpp"
#include "../weather.hpp"
#include "gtest/gtest.h"
#include <cpprest/http_client.h>
#include <cpprest/uri_builder.h>
#include <fstream>
#include <memory>

#include "../FetchURL.hpp"
#include <nlohmann/json.hpp>
#include <inja.hpp>

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
    std::shared_ptr<Location> loc;
    nlohmann::json j;
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
        loc = std::make_shared<Location>("key");
        loc->lat = "1";
        loc->lon = "2";
        loc->name = "LocName";
        j = nlohmann::json::parse(wdata);
    }
};


TEST_F(weatherTest, TestCond) {
    weather->Lookup(loc);
    ASSERT_EQ(weather->Render("{{cond}}"), "Clear");
    
    j["currently"]["summary"] = "Horrible Sharknadoes";
    wdata = j.dump();
    weather->Lookup(loc);
    ASSERT_EQ(weather->Render("{{cond}}"), "Horrible Sharknadoes");
}

TEST_F(weatherTest, TestBadVar) {
    weather->Lookup(loc);
    ASSERT_THROW(weather->Render("{{hsssss}}"), std::runtime_error);
}

TEST_F(weatherTest, TestLocName) {
    weather->Lookup(loc);
    ASSERT_EQ(weather->Render("{{name}}"), "LocName");
    loc->name = "Snakes Ville";
    weather->Lookup(loc);
    ASSERT_EQ(weather->Render("{{name}}"), "Snakes Ville");
}

TEST_F(weatherTest, TestTemp) {
    weather->Lookup(loc);
    ASSERT_EQ(weather->Render("{{temp}}"), "34.6\u00b0F");
    
    j["currently"]["temperature"] = 48;
    wdata = j.dump();
    weather->Lookup(loc);
    ASSERT_EQ(weather->Render("{{temp}}"), "48.0\u00b0F");
    
    j["flags"]["units"] = "si";
    wdata = j.dump();
    weather->Lookup(loc);
    ASSERT_EQ(weather->Render("{{temp}}"), "48.0\u00b0C");
    j["flags"]["units"] = "ca";
    wdata = j.dump();
    weather->Lookup(loc);
    ASSERT_EQ(weather->Render("{{temp}}"), "48.0\u00b0C");
    j["flags"]["units"] = "uk2";
    wdata = j.dump();
    weather->Lookup(loc);
    ASSERT_EQ(weather->Render("{{temp}}"), "48.0\u00b0C");
}

// "{{cond}} {{temp}}{% if exists(\"fltemp\") %} (Feels Like {{fltemp}}){% endif %}, Cloud Cover: {{cloudCover}}, Humidity: {{humidity}}, Wind: {{windDir}} @ {{windSpeed}}{% if exists(\"windGust\") %} ({{windGust}} Gusts){% endif %}"

TEST_F(weatherTest, TestFeelsTemp) {
    weather->Lookup(loc);
    ASSERT_EQ(weather->Render("{{fltemp}}"), "31.2\u00b0F");
    
    j["flags"]["units"] = "si";
    wdata = j.dump();
    weather->Lookup(loc);
    ASSERT_EQ(weather->Render("{{fltemp}}"), "31.2\u00b0C");
    j["flags"]["units"] = "ca";
    wdata = j.dump();
    weather->Lookup(loc);
    ASSERT_EQ(weather->Render("{{fltemp}}"), "31.2\u00b0C");
    j["flags"]["units"] = "uk2";
    wdata = j.dump();
    weather->Lookup(loc);
    ASSERT_EQ(weather->Render("{{fltemp}}"), "31.2\u00b0C");
    ASSERT_EQ(weather->Render("{% if has_fltemp %} (Feels Like {{fltemp}}){% endif %}"), " (Feels Like 31.2\u00b0C)");
    j["currently"]["apparentTemperature"] = 38;
    wdata = j.dump();
    weather->Lookup(loc);
    ASSERT_EQ(weather->Render("{% if has_fltemp %} (Feels Like {{fltemp}}){% endif %}"), " (Feels Like 38.0\u00b0C)");
}

TEST_F(weatherTest, TestFeelsTempEqualRealTemp) {
    j["currently"]["apparentTemperature"] = j["currently"]["temperature"];
    wdata = j.dump();
    weather->Lookup(loc);
    ASSERT_EQ(weather->Render("{% if has_fltemp %} (Feels Like {{fltemp}}){% endif %}"), "");
}

TEST_F(weatherTest, TestFeelsTempMissing) {
    j["currently"].erase("apparentTemperature");
    wdata = j.dump();
    weather->Lookup(loc);
    ASSERT_EQ(weather->Render("{% if has_fltemp %} (Feels Like {{fltemp}}){% endif %}"), "");
}

TEST_F(weatherTest, TestSuninfo) {
    weather->Lookup(loc);
    ASSERT_EQ(weather->Render("{{sunrise}}"), "11:35 am");
    ASSERT_EQ(weather->Render("{{sunset}}"), "11:56 pm");
}



TEST_F(weatherTest, TestGetIRC) {
    weather->Lookup(loc);
    ASSERT_EQ(weather->GetIRC(), "(LocName) Currently Clear 34.6\u00b0" "F (Feels Like 31.2\u00b0" "F), Cloud Cover: 0%, Humidity: 53%, Wind: N @ 3.9 mph (5.7 mph Gusts) Sunrise: 11:35 am Sunset: 11:56 pm");
}
