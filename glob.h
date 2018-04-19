//
//  Copyright (c) 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef glob_h
#define glob_h

#include <string>

namespace util {

class Glob {
public:
    enum MatchResult {
        MATCH=1,      // string matches pattern
        FAIL=0,       // string did not and can not match
        PARTIAL=-1,   // string did not match but could with more characters
        M_ERROR=-2    // Bad pattern
    };

    static MatchResult glob(const std::string &pattern, const std::string &str);

private:
    static MatchResult pglob(const char *p, const char *s);
};



inline Glob::MatchResult glob(const std::string &pattern,
  const std::string &str) {

    return Glob::glob(pattern, str);
}


inline bool fnmatch(const std::string &pattern, const std::string &str) {
    return glob(pattern, str) == Glob::MATCH;
}

} // namespace util

#endif
