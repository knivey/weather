#include "GetTime.hpp"
#include <iosfwd>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// TODO add locks to make thread safe, until then YOLO!
std::string GetTime(std::string format, std::string input, std::string timezone) {
    char buf[256];
    struct tm t;
    memset(&t, 0, sizeof(struct tm));
    setenv("TZ", timezone.c_str(), 1);
    tzset();
    strptime(input.c_str(), "%s", &t);
    strftime(buf, 255, format.c_str(), &t);
    return std::string(buf);
}

std::string GetTime(std::string format, long input, std::string timezone) {
    char buf[256];
    struct tm t;
    memset(&t, 0, sizeof(struct tm));
    setenv("TZ", timezone.c_str(), 1);
    tzset();
    localtime_r(&input, &t);
    strftime(buf, 255, format.c_str(), &t);
    return std::string(buf);
}
