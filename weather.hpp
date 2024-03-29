#ifndef WEATHER_H
#define WEATHER_H

#include <string>
#include <string_view>
#include <memory>
#include "nlohmann/json.hpp"
#include "location.hpp"

class Weather
{
public:
    std::string error;
    std::string GetIRC();
    Weather(std::string key);
    static bool ValidUnits(const std::string_view units);
    void Lookup(std::shared_ptr<Location> loc);
    void Lookup(std::shared_ptr<Location> loc, std::string units);
    std::string Render(const std::string_view tmpl);

private:
    static const std::string URL;
    std::string key;
    std::shared_ptr<Location> loc;
    std::string timezone;
    nlohmann::json w;
    nlohmann::json out;

    //Read current conditions into out
    void CurCond();
};

#endif //WEATHER_H
