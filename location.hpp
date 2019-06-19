#ifndef LOCATION_H
#define LOCATION_H

#include <string>

class Location
{
public:
    std::string error;
    std::string lat;
    std::string lon;
    long double dlat;
    long double dlon;
    std::string name;
    std::string key;

    Location (std::string key);
    void Lookup (std::string query);
};

#endif // LOCATION_H
