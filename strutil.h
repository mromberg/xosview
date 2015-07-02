//
// String manipulations that probably won't be in the standard library
// before the 32 bit time clock rolls over.
//
#ifndef strutil_H
#define strutil_H

#include <string>
#include <vector>
#include <sstream>
#include <map>

namespace util {

extern const char *WHITE_SPACE; // = " \f\n\r\t\v";

inline std::string rstrip(const std::string& s,
  const std::string& delimiters=WHITE_SPACE) {
    if (!s.size())
        return s;
    return s.substr(0, s.find_last_not_of( delimiters ) + 1);
}

inline std::string lstrip(const std::string& s,
  const std::string& delimiters=WHITE_SPACE) {
    if (!s.size())
        return s;
    return s.substr(s.find_first_not_of(delimiters));
}

inline std::string strip(const std::string& s,
  const std::string& delimiters=WHITE_SPACE) {
    return lstrip(rstrip(s, delimiters), delimiters);
}

template <class X>
inline std::string repr(const X &x) {
    std::stringstream os;
    os << x;
    return os.str();
}

// The stoX functions are in the standard.  But gcc wants
// you to use -std=c++-11 to use them.  And this has made
// things explode in the past
int stoi(const std::string &str, size_t *idx = 0, int base = 10);
float stof(const std::string &str, size_t *idx = 0);


extern std::vector<std::string> split(const std::string& s,
  const std::string& delim, const bool keep_empty = true);


template <class X>
inline static bool fstr(const std::string &s, X &x) {
    std::stringstream os(s);
    os >> x;
    return os;
}

// Deal with those goofy forign characters
std::wstring s2ws(const std::string& s);
std::string ws2s(const std::wstring &s);

inline std::string toupper(const std::string &str) {
    std::string rval;
    for (size_t i = 0 ; i < str.size() ; i++)
        rval.push_back(std::toupper(str[i]));
    return rval;
}

inline std::string tolower(const std::string &str) {
    std::string rval;
    for (size_t i = 0 ; i < str.size() ; i++)
        rval.push_back(std::tolower(str[i]));
    return rval;
}

std::string strerror(int error);

} // namespace util

// print std::pair in the form: (first,second)
template<typename S, typename T>
inline std::ostream &operator<<(std::ostream &os, const std::pair<S,T> &pp) {
    os << "(" << pp.first << "," << pp.second << ")";
    return os;
}

// print vectors in the form: [a,b,c,d]
template<class X>
std::ostream &operator<<(std::ostream &os, const std::vector<X> &x) {
    os << "[";
    for (size_t i = 0 ; i < x.size() ; i++) {
        os << x[i];
        if (i < x.size() - 1)
            os << ",";
    }
    os << "]";

    return os;
}

// print maps in the form: {key1:value1,key2:value2}
template<class X,class Y>
std::ostream &operator<<(std::ostream &os, const std::map<X,Y> &m) {
    os << "{";
    typename std::map<X,Y>::const_iterator it;
    for (it = m.begin(); it != m.end(); ) {
        os << it->first << ":" << it->second;
        ++it;
        if (it != m.end())
            os << ",";
    }
    os << "}";

    return os;
}

#endif
