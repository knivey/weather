#include "FetchURL.hpp"
#include "location.hpp"
#include "nlohmann/json.hpp"

#include <cpprest/uri_builder.h>
#include <fmt/core.h>
#include <iostream>
#include <tuple>

static const char *BINGLOC_URL = "http://dev.virtualearth.net/REST/v1/Locations/";

void Location::Lookup(std::string query) {
    web::uri_builder b;
    b.append_query("key", key);
    b.append_query("query", query);
    bool err;
    std::string body;
    std::tie(err, body) = FetchURL(BINGLOC_URL, b);
    if (err) {
        this->error = fmt::format("Weather location lookup HTTP Error: {}", body);
        return;
    }

    try {
        auto j = nlohmann::json::parse(body);
        if (j["statusCode"] != 200) {
            this->error = fmt::format("Weather location service recieved an error");
            std::cout << j.dump(4) << std::endl;
            return;
        }
        auto rs = j["resourceSets"][0];
        if (rs["resources"].size() < 1) {
            this->error = fmt::format("Weather location not found");
            return;
        }
        auto r = rs["resources"][0];
        r["address"]["formattedAddress"].get_to(this->name);
        auto c = r["point"]["coordinates"];
        this->dlat = c[0].get<long double>();
        this->dlon = c[1].get<long double>();
        this->lat = fmt::format("{:.18f}", this->dlat);
        this->lon = fmt::format("{:.18f}", this->dlon);
    } catch (const std::exception &e) {
        this->error = fmt::format("Weather JSON Error exception: {}", e.what());
        return;
    }
}

Location::Location(std::string key) : key(key) {
    ;
}
