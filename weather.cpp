#include "FetchURL.hpp"
#include "inja.hpp"
#include "location.hpp"
#include "weather.hpp"
#include <cpprest/http_client.h>
#include <cpprest/uri_builder.h>
#include <fmt/core.h>
#include <iostream>
#include <map>
#include <tuple>

#include "GetTime.hpp"

/*
 * TODO catch specific exception types for better error output
 */


#define CONDFMT "{{cond}} {{temp}}{% if exists(\"fltemp\") %} (Feels Like {{fltemp}}){% endif %}, Cloud Cover: {{cloudCover}}, Humidity: {{humidity}}, Wind: {{windDir}} @ {{windSpeed}}{% if exists(\"windGust\") %} ({{windGust}} Gusts){% endif %}"

const std::string Weather::URL = "https://api.darksky.net/forecast/";

Weather::Weather(std::string key) : key(key) {
    ;
}

static const char *DIRS[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};

static std::map<std::string, std::map<std::string, std::string>> UNITS{
    {"us", {{"temp", "\u00b0F"}, {"windSpeed", "mph"}}},
    {"ca", {{"temp", "\u00b0C"}, {"windSpeed", "kph"}}},
    {"si", {{"temp", "\u00b0C"}, {"windSpeed", "m/s"}}},
    {"uk2", {{"temp", "\u00b0C"}, {"windSpeed", "mph"}}}};

void Weather::CurCond() {
    std::string units = w["flags"]["units"].get<std::string>();
    nlohmann::json cur = w["currently"];
    //cond
    out["cond"] = cur["summary"];
    //windDir
    if (cur["windSpeed"] == 0) {
        out["windDir"] = "*";
    } else {
        out["windDir"] = DIRS[(int)(((cur["windBearing"].get<int>() % 360) - 22.5) / 45)];
    }
    //windSpeed
    out["windSpeed"] = fmt::format("{:.1f} {}", cur["windSpeed"].get<double>(), UNITS[units]["windSpeed"]);
    //windGust
    if (!cur["windGust"].is_null()) {
        out["windGust"] = fmt::format("{:.1f} {}", cur["windGust"].get<double>(), UNITS[units]["windSpeed"]);
    }
    //temp
    out["temp"] = fmt::format("{:.1f}{}", cur["temperature"].get<double>(), UNITS[units]["temp"]);
    //fltemp
    out["has_fltemp"] = false;
    if (!cur["apparentTemperature"].is_null() && cur["apparentTemperature"] != cur["temperature"]) {
        out["fltemp"] = fmt::format("{:.1f}{}", cur["apparentTemperature"].get<double>(), UNITS[units]["temp"]);
        out["has_fltemp"] = true;
    }
    // Percent type is fucked, plus I dont need floating percents
    //humidity
    out["humidity"] = fmt::format("{}%", int(cur["humidity"].get<double>() * 100));
    //cloudcover
    out["cloudCover"] = fmt::format("{}%", int(cur["cloudCover"].get<double>() * 100));
}

void Weather::Lookup(std::shared_ptr<Location> loc) {
    Lookup(loc, "auto");
}

bool Weather::ValidUnits(const std::string_view units) {
    return (units == "auto" || UNITS.count(units.data()) > 0);
}

void Weather::Lookup(std::shared_ptr<Location> loc, std::string units) {
    if (!ValidUnits(units)) {
        error = ("Invalid units");
        return;
    };
    out["name"] = loc->name;

    web::uri_builder b;
    b.append_path(key);
    b.append_path(fmt::format("{},{}", loc->lat, loc->lon));
    b.append_query("exclude", "minutely,hourly");
    b.append_query("units", units);
    bool err;
    std::string body;
    tie(err, body) = FetchURL(URL, b);
    if (err) {
        error = fmt::format("Weather Darksky HTTP Error: {}", body);
        return;
    }
    try {
        w = nlohmann::json::parse(body);
        w["timezone"].get_to(timezone);
        if (!w["error"].is_null()) {
            error = fmt::format("Weather Darksky Error: {}", w["error"].get<std::string>());
            return;
        }
        CurCond();
    } catch (const std::exception &e) {
        error = fmt::format("Weather Error exception: {}", e.what());
        return;
    }
}

std::string Weather::GetIRC() {
    return inja::render("({{name}}) Currently " CONDFMT, out);
}

std::string Weather::Render(const std::string_view tmpl) {
    return inja::render(tmpl, out);
}
