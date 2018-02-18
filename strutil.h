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
    if (s.empty())
        return s;
    return s.substr(0, s.find_last_not_of(delimiters) + 1);
}


inline std::string lstrip(const std::string& s,
  const std::string& delimiters=WHITE_SPACE) {
    if (s.empty())
        return s;
    return s.substr(s.find_first_not_of(delimiters));
}


inline std::string strip(const std::string& s,
  const std::string& delimiters=WHITE_SPACE) {
    return lstrip(rstrip(s, delimiters), delimiters);
}


inline std::string toupper(const std::string &str) {
    std::string rval;
    std::transform(str.cbegin(), str.cend(), std::back_inserter(rval),
      [](auto c){ return std::toupper(c); });
    return rval;
}


inline std::string tolower(const std::string &str) {
    std::string rval;
    std::transform(str.cbegin(), str.cend(), std::back_inserter(rval),
      [](auto c){ return std::tolower(c); });
    return rval;
}


template <class X>
inline std::string repr(const X &x) {
    std::stringstream os;
    os << x;
    return os.str();
}


template <class X>
inline static bool fstr(const std::string &s, X &x) {
    std::stringstream os(s);
    os >> x;
    return !os.fail();
}


// Deal with those goofy forign characters
extern std::wstring s2ws(const std::string& s);
extern std::string ws2s(const std::wstring &s);

std::string strerror(int error);
std::string strerror(void);


extern std::vector<std::string> vargv(int argc, const char * const *argv);

extern std::vector<std::string> split(const std::string& s,
  const std::string& delim="", size_t maxsplit=std::string::npos);

extern std::string join(const std::vector<std::string> &v,
  const std::string &sep);


template <class K, class V>
V get(const std::map<K, V> &m, const K &k, V defVal=V()) {
    auto it = m.find(k);
    if (it == m.end())
        return defVal;
    return it->second;
}


template <class X>
inline bool find(const std::vector<X> &v, const X &x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}


template <class T>
inline void concat(std::vector<T> &a, const std::vector<T> &b) {
    a.reserve(a.size() + b.size());
    a.insert(a.end(), b.begin(), b.end());
}


template <class T>
inline void concat(std::vector<T> &a, std::vector<T> &&b) {
    for (auto &bi : b)
        a.push_back(std::move(bi));
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


} // namespace util



//---------------------------------------
// ostream operators for std containers.
//---------------------------------------
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
    for (auto it = x.cbegin() ; it != x.cend() ; ++it) {
        if (it != x.cbegin())
            os << ",";
        os << *it;
    }
    os << "]";

    return os;
}


// print maps in the form: {key1:value1,key2:value2}
template<typename X,typename Y>
ostream &operator<<(ostream &os, const map<X,Y> &m) {
    os << "{";
    for (auto it = m.cbegin() ; it != m.cend() ; ++it) {
        if (it != m.cbegin())
            os << ",";
        os << it->first << ":" << it->second;
    }
    os << "}";

    return os;
}


} // end namespace std

#endif
