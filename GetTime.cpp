#include "GetTime.hpp"
#include <iosfwd>
#include <stdlib.h>
#include <string.h>
#include <time.h>

using namespace std;

// TODO add locks to make thread safe, until then YOLO!
string GetTime(string format, string input, string timezone) {
    char buf[256];
    struct tm t;
    memset(&t, 0, sizeof(struct tm));
    setenv("TZ", timezone.c_str(), 1);
    tzset();
    strptime(input.c_str(), "%s", &t);
    strftime(buf, 255, format.c_str(), &t);
    return string(buf);
}

string GetTime(string format, long input, string timezone) {
    char buf[256];
    struct tm t;
    memset(&t, 0, sizeof(struct tm));
    setenv("TZ", timezone.c_str(), 1);
    tzset();
    localtime_r(&input, &t);
    strftime(buf, 255, format.c_str(), &t);
    return string(buf);
}
