#include "FetchURL.h"
#include <cpprest/http_client.h>

using namespace std;
using namespace web::http;
using namespace web::http::client;

tuple<bool, string> FetchURL(string url, uri_builder builder) {
    string data;
    try {
        http_client_config config;
        //Leading research shows my friends are unable to wait longer than 5 seconds
        config.set_timeout(utility::seconds(5));
        http_client client(url, config);
        auto res = client.request(methods::GET, builder.to_string()).get();
        //fmt::print("Received response status code: {}\n", res.status_code());
        data = res.extract_string().get();
    } catch (const std::exception &e) {
        return make_tuple(true, e.what());
    }
    return make_tuple(false, data);
}
