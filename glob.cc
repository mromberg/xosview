//
//  Copyright (c) 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "glob.h"

#include <set>
#include <iostream>



namespace /* {anonymous} */ {

class GlobRange {
public:
    GlobRange(const std::string &str);

    bool match(char c) const {
        bool rval = _classtbl.count(c);
        return _inverse ? !rval : rval;
    }

    bool good(void) const { return _good; }
    const std::string &pattern(void) const { return _pattern; }

    std::ostream &printOn(std::ostream &os) const;

private:
    std::string _pattern;
    std::set<char> _classtbl;
    bool _inverse;
    bool _good;
};



inline std::ostream &operator<<(std::ostream &os, const GlobRange &gr) {
    return gr.printOn(os);
}


template <class X>
std::ostream &operator<<(std::ostream &os, const std::set<X> &s) {
    os << "(";
    for (auto it = s.cbegin() ; it != s.cend() ; ++it) {
        if (it != s.begin())
            os << ", ";
        os << *it;
    }
    os << ")";
    return os;
}


} // namespace {anonymous}



GlobRange::GlobRange(const std::string &str)
    : _inverse(false), _good(false) {

    if (str.size() >= 3 && str[0] == '[') {
        size_t findEnd = 2;  // look for ']' starting at 2 (empty not valid)
        for (size_t i = 0 ; i < str.size() ; i++) {
            _pattern += str[i];
            if (i == 1 && str[i] == '!') {
                _inverse = true;
                findEnd = 3;  // '!' does not count as first char
                continue;
            }
            if (i > 1 && i < str.size() - 2 && str[i] == '-') {
                const unsigned char from = std::min((unsigned char)str[i-1],
                  (unsigned char)str[i+1]);
                const unsigned char to = std::max((unsigned char)str[i-1],
                  (unsigned char)str[i+1]);
                for (unsigned char c = from ; c <= to ; c++)
                    _classtbl.insert(c);
                i++;
                _pattern += str[i];
                continue;
            }
            if (i >= findEnd && str[i] == ']') {
                _good = true;
                break;
            }
            if (i >= 1)
                _classtbl.insert(str[i]);
        }
    }
}


std::ostream &GlobRange::printOn(std::ostream &os) const {
    os << "GlobRange(" << pattern() << "): "
       << "inv=" << _inverse << ", "
       << "tbl=" << _classtbl;
    return os;
}


util::Glob::MatchResult util::Glob::pglob(const char *p, const char *s) {

    switch (*p) {
    case '*': {
        MatchResult rval = pglob(p + 1, s);
        if (rval == FAIL && *s)
            return pglob(p, s + 1);
        return rval;
    }
    case '?': {
        if (!*s)
            return PARTIAL;
        return pglob(p + 1, s + 1);
    }
    case '\\': {
        if (!(*++p))
            return M_ERROR; // '\\' missing character to escape (end of str)
        if (*p && !*s)
            return PARTIAL;
        if (*s != *p)
            return FAIL;
        return pglob(p + 1, s + 1);
    }
    case '[': {
        GlobRange range(p);
        if (!range.good())
            return M_ERROR;
        if (!*s)
            return PARTIAL;
        if (!range.match(*s))
            return FAIL;
        return pglob(p + range.pattern().size(), s + 1);
    }
    default:
        if (*p && !*s)
            return PARTIAL;
        if (*s != *p)
            return FAIL;
        if (*s)
            return pglob(p + 1, s + 1);
        return MATCH;
    };

    return MATCH; // In case some compilers complain...
}


util::Glob::MatchResult util::Glob::glob(const std::string &pat,
  const std::string &str) {

    return pglob(pat.c_str(), str.c_str());
}
