#include "FetchURL.hpp"
#include <cpprest/http_client.h>

std::tuple<bool, std::string> FetchURL(std::string url, web::uri_builder builder) {
    std::string data;
    try {
        web::http::client::http_client_config config;
        //Leading research shows my friends are unable to wait longer than 5 seconds
        config.set_timeout(utility::seconds(5));
        web::http::client::http_client client(url, config);
        auto res = client.request(web::http::methods::GET, builder.to_string()).get();
        //fmt::print("Received response status code: {}\n", res.status_code());
        data = res.extract_string().get();
    } catch (const std::exception &e) {
        return std::make_tuple(true, e.what());
    }
    return std::make_tuple(false, data);
}
