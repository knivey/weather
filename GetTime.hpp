#ifndef GETTIME_H
#define GETTIME_H

#include <string>

std::string GetTime(std::string format, std::string input, std::string timezone);
std::string GetTime(std::string format, long input, std::string timezone);

#endif
