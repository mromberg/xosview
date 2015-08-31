#include "strutil.h"
#include "log.h"

#include <algorithm>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <cassert>
#include <stdexcept>
#include <set>

#include <locale.h>


namespace util {

const char *WHITE_SPACE = " \f\n\r\t\v";

std::string join(const std::vector<std::string> &v,
  const std::string &sep) {
    std::string rval;
    for (size_t i = 0 ; i < v.size() ; i ++) {
        rval += v[i];
        if (i + 1 < v.size())
            rval += sep;
    }

    return rval;
}

std::vector<std::string> split(const std::string& s,
  const std::string& delim, const bool keep_empty) {
    std::vector<std::string> result;
    if (delim.empty()) {
        result.push_back(s);
        return result;
    }
    std::string::const_iterator substart = s.begin(), subend;
    while (true) {
        subend = std::search(substart, s.end(), delim.begin(), delim.end());
        std::string temp(substart, subend);
        if (keep_empty || !temp.empty())
            result.push_back(temp);
        if (subend == s.end())
            break;

        substart = subend + delim.size();
    }

    return result;
}

static void setLocale(void) {
    static bool first = true;
    if (first) {
        setlocale(LC_CTYPE, "");  // for s2ws() and ws2s()
        first = false;
    }
}

std::wstring s2ws(const std::string& s) {
    setLocale();
    const char * cs = s.c_str();
    const size_t wn = std::mbsrtowcs(NULL, &cs, 0, NULL);

    if (wn == size_t(-1)) {
        logBug << "Error in mbsrtowcs(): errno=" << errno
               << ", str=" << s << std::endl;
        return L"";
    }

    std::vector<wchar_t> buf(wn + 1);
    const size_t wn_again = std::mbsrtowcs(buf.data(), &cs, wn + 1, NULL);

    if (wn_again == size_t(-1)) {
        logBug << "Error in mbsrtowcs(): errno=" << errno
               << ", str=" << s << std::endl;
        return L"";
    }

    assert(cs == NULL); // successful conversion

    return std::wstring(buf.data(), wn);
}

std::string ws2s(const std::wstring &s) {
    setLocale();
    const wchar_t * cs = s.c_str();
    const size_t wn = std::wcsrtombs(NULL, &cs, 0, NULL);

    if (wn == size_t(-1)) {
        logBug << "Error in wcsrtombs(): errno=" << errno << std::endl;
        return "";
    }

    std::vector<char> buf(wn + 1);
    const size_t wn_again = std::wcsrtombs(buf.data(), &cs, wn + 1, NULL);

    if (wn_again == size_t(-1)) {
        logBug << "Error in wcsrtombs(): errno=" << errno << std::endl;
        return "";
    }

    assert(cs == NULL); // successful conversion

    return std::string(buf.data(), wn);
}


int stoi(const std::string &str, size_t *, int) {
    int rval;
    if (!fstr(str, rval))
        throw std::invalid_argument("error converting " + str + " to int");
    return rval;
}

float stof(const std::string &str, size_t *) {
    float rval;
    if (!fstr(str, rval))
        throw std::invalid_argument("error converting " + str + " to float");
    return rval;
}

std::string strerror(int error) {
    return std::strerror(error);
}


template <class X>
inline std::ostream &operator<<(std::ostream &os, const std::set<X> &s) {
    os << "(";
    typename std::set<X>::iterator it;
    for (it = s.begin() ; it != s.end() ; ++it) {
        if (it != s.begin())
            os << ", ";
        os << *it;
    }
    os << ")";
    return os;
}


class GlobRange {
public:
    GlobRange(const std::string &str) : _inverse(false), _good(false) {
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
                    unsigned char from = std::min((unsigned char)str[i-1],
                      (unsigned char)str[i+1]);
                    unsigned char to = std::max((unsigned char)str[i-1],
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

    bool match(char c) const {
        bool rval = _classtbl.count(c);
        if (_inverse)
            return !rval;
        return rval;
    }

    bool good(void) const { return _good; }
    const std::string &pattern(void) const { return _pattern; }

    std::ostream &printOn(std::ostream &os) const {
        os << "GlobRange(" << pattern() << "): "
           << "inv=" << _inverse << ", "
           << "tbl=" << _classtbl;
        return os;
    }

private:
    std::string _pattern;
    std::set<char> _classtbl;
    bool _inverse;
    bool _good;
};


inline std::ostream &operator<<(std::ostream &os, const GlobRange &gr) {
    return gr.printOn(os);
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


util::sink::sink(const std::string &pattern, bool greedy)
    : _pattern(pattern), _greedy(greedy) {
}


std::istream &util::sink::consume(std::istream &is) const {

    if (!_pattern.size())
        return is;

    std::string buf;
    bool matchedOnce = false;

    char c = is.peek();
    while (is.good()) {

        buf += c;

        switch (util::glob(_pattern, buf)) {
        case util::Glob::M_ERROR:
            is.setstate(std::ios::badbit);
        case util::Glob::FAIL:
            if (_greedy && matchedOnce)
                return is;
            is.setstate(std::ios::failbit);
            break;
        case util::Glob::MATCH:
            matchedOnce = true;
            if (_greedy)
                break;
            is.ignore();
            return is;
        case util::Glob::PARTIAL:
            if (_greedy && matchedOnce)
                return is;
        default:
            break;
        };

        is.ignore();
        c = is.peek();
    }

    return is;
}


} // namespace util
