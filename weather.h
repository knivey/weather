#ifndef WEATHER_H
#define WEATHER_H

#include <string>
#include <memory>
#include "nlohmann/json.hpp"
#include "location.h"

class Weather
{
public:
    std::string error;
    std::string GetIRC();
    Weather ( std::string key );
    static bool ValidUnits ( std::string units );
    void Lookup ( std::shared_ptr<Location> loc );
    void Lookup ( std::shared_ptr<Location> loc, std::string units );

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
