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
#include <algorithm>


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

extern std::string join(const std::vector<std::string> &v,
  const std::string &sep);

template <class X>
inline static bool fstr(const std::string &s, X &x) {
    std::stringstream os(s);
    os >> x;
    return !os.fail();
}


template <class K, class V>
const V &get(const std::map<K, V> &m, const K &k,
  const V &defVal=V()) {
    typename std::map<K, V>::const_iterator it = m.find(k);
    if (it == m.end())
        return defVal;
    return it->second;
}


template <class X>
inline bool find(const std::vector<X> &v, const X &x) {
    return std::find(v.begin(), v.end(), x) != v.end();
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
std::string strerror(void);


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


class sink {
public:
    sink(const std::string &pattern="", bool greedy=false);

    // read and discard characters from the stream that match
    // this glob pattern.   Stop when the pattern is matched.
    // If a character is read that causes the match to fail set failbit.
    // If the pattern is found to be invalid then set badbit
    // If this sink is set to greedy, then keep discarding characters
    // as long as the match continues.  This (greedy) behavior is
    // done only once.
    std::istream &consume(std::istream &is) const;

    const std::string &pattern(void) const { return _pattern; }

private:
    std::string _pattern;
    bool _greedy;
};


inline std::istream &operator>>(std::istream &is, const sink &s) {
    return s.consume(is);
}


template <class T>
inline void concat(std::vector<T> &a, const std::vector<T> &b) {
    a.reserve(a.size() + b.size());
    a.insert(a.end(), b.begin(), b.end());
}


} // namespace util


namespace std {

// print std::pair in the form: (first,second)
template<typename S, typename T>
inline ostream &operator<<(ostream &os, const pair<S,T> &pp) {
    os << "(" << pp.first << "," << pp.second << ")";
    return os;
}

// print vectors in the form: [a,b,c,d]
template<typename X>
ostream &operator<<(ostream &os, const vector<X> &x) {
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
template<typename X,typename Y>
ostream &operator<<(ostream &os, const map<X,Y> &m) {
    os << "{";
    typename map<X,Y>::const_iterator it;
    for (it = m.begin(); it != m.end(); ) {
        os << it->first << ":" << it->second;
        ++it;
        if (it != m.end())
            os << ",";
    }
    os << "}";

    return os;
}

} // end namespace std

#endif
