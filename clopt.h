//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef CLOPT_H
#define CLOPT_H

#include <string>
#include <vector>
#include <iostream>


namespace util {

class CLOpt {
public:
    CLOpt(const std::string &name, const std::string &shortOpt,
      const std::string &longOpt, const std::string &desc,
      bool isValue=false, const std::string &valDesc="");

    const std::string &name(void) const { return _name; }
    const std::vector<std::string> &values(void) const { return _values; }
    const std::string &value(void) const;
    bool isTrue(void) const;
    bool missing(void) const { return !_values.size(); }
    bool isValue(void) const { return _isValue; }

    bool match(const std::string &arg) const
        { return arg == _shortOpt || arg == _longOpt; }
    void setValue(const std::string &val="True") { _values.push_back(val); }

    std::string usage(void) const; // return a usage string

    void printOn(std::ostream &os) const;

private:
    std::string _name;
    std::string _shortOpt;
    std::string _longOpt;
    std::string _desc;
    bool _isValue;
    std::string _valDesc;
    std::vector<std::string> _values;

    std::string formatedDesc(size_t offset) const;
};

class CLOpts {
public:
    CLOpts(int argc, char **argv): _argc(argc), _argv(argv) {}

    // Add a boolean option (no value)
    void add(const std::string &name, const std::string &shortOpt,
      const std::string &longOpt, const std::string &desc);
    // Add a option which has a value
    void add(const std::string &name, const std::string &shortOpt,
      const std::string &longOpt, const std::string &value,
      const std::string &desc);

    void parse(void);

    const std::vector<std::string> &args(void) const { return _args; }
    const std::vector<std::string> &values(const std::string &name) const;
    const std::string &value(const std::string &name) const;
    const std::string &value(const std::string &name,
      const std::string &defaultVal) const;
    bool isTrue(const std::string &name) const;
    bool missing(const std::string &name) const;

    std::string useage(void) const; // return a usage string
    void printOn(std::ostream &os) const;

    const std::vector<CLOpt> &opts(void) const { return _opts; }

private:
    int _argc;
    char **_argv;
    std::vector<CLOpt> _opts;
    std::vector<std::string> _args;

    void fail(const std::string &msg) const;
    std::string pname(void) const;
};

inline std::ostream &operator<<(std::ostream &os, const CLOpt &c) {
    c.printOn(os);
    return os;
}

inline std::ostream &operator<<(std::ostream &os, const CLOpts &c) {
    c.printOn(os);
    return os;
}

} // end namespace util

#endif
