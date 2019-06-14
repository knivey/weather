#ifndef LOCATION_H
#define LOCATION_H

#include <string>

class Location
{
public:
    std::string error;
    std::string lat;
    std::string lon;
    double dlat;
    double dlon;
    std::string name;

    Location ( std::string query );
    Location() {;}
    void Lookup ( std::string query );
};

#endif // LOCATION_H
