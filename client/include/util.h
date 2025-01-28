// util.h
#ifndef UTIL_H
#define UTIL_H

#include <string>

// Add `inline` to avoid multiple definitions
inline bool startsWith(const std::string& str, const std::string& prefix) {
    return str.compare(0, prefix.size(), prefix) == 0;
}

#endif