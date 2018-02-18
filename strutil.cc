#include "strutil.h"
#include "glob.h"
#include "log.h"

#include <cerrno>
#include <cstring>
#include <locale>


namespace util {


const char *WHITE_SPACE = " \f\n\r\t\v";



std::string join(const std::vector<std::string> &v,
  const std::string &sep) {
    std::string rval;
    for (auto it = v.cbegin() ; it != v.cend() ; ++it) {
        if (it != v.cbegin())
            rval += sep;
        rval += *it;
    }

    return rval;
}


std::vector<std::string> split(const std::string& s,
  const std::string& delim, size_t maxsplit) {

    std::vector<std::string> result;
    if (delim.empty()) {
        const std::string wsdelim(WHITE_SPACE);
        size_t pos = 0;
        while (pos < s.size() && result.size() < maxsplit) {
            if (wsdelim.find(s[pos]) != std::string::npos) {
                pos = s.find_first_not_of(wsdelim, pos);
            }
            else {
                const size_t epos = s.find_first_of(wsdelim, pos);
                if (epos == std::string::npos)
                    result.push_back(s.substr(pos));
                else
                    result.push_back(s.substr(pos, epos - pos));
                pos = epos;
            }
        }
        if (pos < s.size())
            result.push_back(lstrip(s.substr(pos)));
    }
    else {
        size_t pos = 0;
        while (pos < s.size() && result.size() < maxsplit) {
            const size_t epos = s.find(delim, pos);
            if (epos != std::string::npos) {
                result.push_back(s.substr(pos, epos - pos));
                pos = epos + delim.size();
            }
            else {
                result.push_back(s.substr(pos));
                pos = epos;
            }
        }
        if (pos < s.size())
            result.push_back(s.substr(pos));
    }
    return result;
}


static void setLocale(void) {
    static bool first = true;
    if (first) {
        std::setlocale(LC_CTYPE, "");  // for s2ws() and ws2s()
        first = false;
    }
}


std::wstring s2ws(const std::string& s) {
    setLocale();
    const char * cs = s.c_str();
    const size_t wn = std::mbsrtowcs(nullptr, &cs, 0, nullptr);

    if (wn == size_t(-1)) {
        logBug << "Error in mbsrtowcs(): errno=" << errno
               << ", str=" << s << std::endl;
        return L"";
    }

    std::vector<wchar_t> buf(wn + 1);
    const size_t wn_again = std::mbsrtowcs(buf.data(), &cs, wn + 1, nullptr);

    if (wn_again == size_t(-1)) {
        logBug << "Error in mbsrtowcs(): errno=" << errno
               << ", str=" << s << std::endl;
        return L"";
    }

    logAssert(cs == nullptr) << "unsuccessful conversion." << std::endl;

    return std::wstring(buf.data(), wn);
}


std::string ws2s(const std::wstring &s) {
    setLocale();
    const wchar_t * cs = s.c_str();
    const size_t wn = std::wcsrtombs(nullptr, &cs, 0, nullptr);

    if (wn == size_t(-1)) {
        logBug << "Error in wcsrtombs(): errno=" << errno << std::endl;
        return "";
    }

    std::vector<char> buf(wn + 1);
    const size_t wn_again = std::wcsrtombs(buf.data(), &cs, wn + 1, nullptr);

    if (wn_again == size_t(-1)) {
        logBug << "Error in wcsrtombs(): errno=" << errno << std::endl;
        return "";
    }

    logAssert(cs == nullptr) << "unsuccessful conversion." << std::endl;

    return std::string(buf.data(), wn);
}


std::string strerror(int error) {
    return std::strerror(error);
}


std::string strerror(void) {
    return strerror(errno);
}


util::sink::sink(const std::string &pattern, bool greedy)
    : _pattern(pattern), _greedy(greedy) {
}


std::istream &util::sink::consume(std::istream &is) const {
    if (_pattern.empty())
        return is;

    std::string buf;
    bool matchedOnce = false;

    char c = is.peek();
    while (is.good()) {

        buf += c;

        switch (util::glob(_pattern, buf)) {
        case util::Glob::M_ERROR:
            is.setstate(std::ios::badbit);
            FALLTHROUGH;
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


std::vector<std::string> vargv(int argc, const char * const *argv) {
    std::vector<std::string> rval;
    rval.reserve(argc);
    for (int i = 0 ; i < argc ; i++)
        rval.push_back(argv[i]);
    return rval;
}


} // namespace util
