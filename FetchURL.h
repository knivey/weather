#ifndef FETCHURL_H
#define FETCHURL_H
#include <cpprest/uri_builder.h>
#include <tuple>
#include <string>

std::tuple<bool, std::string> FetchURL(std::string url, web::uri_builder builder);

#endif //FETCHURL_H

