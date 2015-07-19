#include "strutil.h"
#include "log.h"

#include <algorithm>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <cassert>
#include <stdexcept>
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

}
