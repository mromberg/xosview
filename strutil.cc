#include "strutil.h"
#include "log.h"

#include <cerrno>
#include <cstring>
#include <set>
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
