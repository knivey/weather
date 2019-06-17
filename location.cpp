#include "FetchURL.hpp"
#include "location.hpp"
#include "nlohmann/json.hpp"

#include <boost/lexical_cast.hpp>
#include <cpprest/uri_builder.h>
#include <fmt/core.h>
#include <iostream>
#include <tuple>

using namespace fmt;
using namespace std;
using json = nlohmann::json;

static const char *BINGLOC_URL = "http://dev.virtualearth.net/REST/v1/Locations/";
static const char *BINGLOC_KEY = "AkPlNsEy3tq4KHRJpm-jBoBBZdqTuHRyFFJeTiklmqqGJ5Ntvk88kTxKLapHHQd4";

void Location::Lookup(std::string query) {
    web::uri_builder b;
    b.append_query("key", BINGLOC_KEY);
    b.append_query("query", query);
    bool err;
    string body;
    tie(err, body) = FetchURL(BINGLOC_URL, b);
    if (err) {
        this->error = format("Weather location lookup HTTP Error: {}", body);
        return;
    }

    try {
        auto j = json::parse(body);
        if (j["statusCode"] != 200) {
            this->error = format("Weather location service recieved an error");
            cout << j.dump(4) << endl;
            return;
        }
        auto rs = j["resourceSets"][0];
        if (rs["resources"].size() < 1) {
            this->error = format("Weather location not found");
            return;
        }
        auto r = rs["resources"][0];
        r["address"]["formattedAddress"].get_to(this->name);
        auto c = r["point"]["coordinates"];
        this->dlat = c[0].get<double>();
        this->dlon = c[1].get<double>();
        this->lat = boost::lexical_cast<string>(this->dlat);
        this->lon = boost::lexical_cast<string>(this->dlon);
    } catch (const std::exception &e) {
        this->error = format("Weather JSON Error exception: {}", e.what());
        return;
    }
}

Location::Location(std::string key) : key(key) {
    ;
}
