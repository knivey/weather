#include "FetchURL.h"
#include "inja.hpp"
#include "location.h"
#include "weather.h"
#include <cpprest/http_client.h>
#include <cpprest/uri_builder.h>
#include <fmt/core.h>
#include <iostream>
#include <map>
#include <tuple>

#include "GetTime.h"

/*
 * TODO catch specific exception types for better error output
 */

using namespace std;
//using namespace utility;                    // Common utilities like string conversions
//using namespace web;                        // Common features like URIs.
using namespace web::http;         // Common HTTP functionality
using namespace web::http::client; // HTTP client features
//using namespace concurrency::streams;       // Asynchronous streams
using json = nlohmann::json;

using namespace fmt;

#define CONDFMT "{{cond}} {{temp}}{% if exists(\"fltemp\") %} (Feels Like {{fltemp}}){% endif %}, Cloud Cover: {{cloudCover}}, Humidity: {{humidity}}, Wind: {{windDir}} @ {{windSpeed}}{% if exists(\"windGust\") %} ({{windGust}} Gusts){% endif %}"

const string Weather::URL = "https://api.darksky.net/forecast/";

Weather::Weather(string key) : key(key) {
    ;
}

static const char *DIRS[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};

static map<string, map<string, string>> UNITS{
    {"us", {{"temp", "\u00b0F"}, {"windSpeed", "mph"}}},
    {"ca", {{"temp", "\u00b0C"}, {"windSpeed", "kph"}}},
    {"si", {{"temp", "\u00b0C"}, {"windSpeed", "m/s"}}},
    {"uk2", {{"temp", "\u00b0C"}, {"windSpeed", "mph"}}}};

void Weather::CurCond() {
    string units = w["flags"]["units"].get<string>();
    json cur = w["currently"];
    out["cond"] = cur["summary"];
    ;
    if (cur["windSpeed"] == 0) {
        out["windDir"] = "*";
    } else {
        out["windDir"] = DIRS[(int)(((cur["windBearing"].get<int>() % 360) - 22.5) / 45)];
    }
    out["windSpeed"] = format("{:.1f} {}", cur["windSpeed"].get<double>(), UNITS[units]["windSpeed"]);

    if (!cur["windGust"].is_null()) {
        out["windGust"] = format("{:.1f} {}", cur["windGust"].get<double>(), UNITS[units]["windSpeed"]);
    }

    out["temp"] = format("{:.1f}{}", cur["temperature"].get<double>(), UNITS[units]["temp"]);
    if (!cur["apparentTemperature"].is_null() && cur["apparentTemperature"] != cur["temperature"]) {
        out["fltemp"] = format("{:.1f}{}", cur["apparentTemperature"].get<double>(), UNITS[units]["temp"]);
    }
    // Percent type is fucked, plus I dont need floating percents
    out["humidity"] = format("{}%", int(cur["humidity"].get<double>() * 100));
    out["cloudCover"] = format("{}%", int(cur["cloudCover"].get<double>() * 100));
}

void Weather::Lookup(std::shared_ptr<Location> loc) {
    Lookup(loc, "auto");
}

bool Weather::ValidUnits(string units) {
    return (units == "auto" || UNITS.count(units) > 0);
}

void Weather::Lookup(std::shared_ptr<Location> loc, string units) {
    if (!ValidUnits(units)) {
        error = ("Invalid units");
        return;
    };
    out["name"] = loc->name;

    web::uri_builder b;
    b.append_path(key);
    b.append_path(format("{},{}", loc->lat, loc->lon));
    b.append_query("exclude", "minutely,hourly");
    b.append_query("units", units);
    bool err;
    string body;
    tie(err, body) = FetchURL(URL, b);
    if (err) {
        error = format("Weather Darksky HTTP Error: {}", body);
        return;
    }
    try {
        w = json::parse(body);
        w["timezone"].get_to(timezone);
        if (!w["error"].is_null()) {
            error = format("Weather Darksky Error: {}", w["error"].get<string>());
            return;
        }
        CurCond();
    } catch (const std::exception &e) {
        error = format("Weather Error exception: {}", e.what());
        return;
    }
}

string Weather::GetIRC() {
    return inja::render("({{name}}) Currently " CONDFMT, out);
}
