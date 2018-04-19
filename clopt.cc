//
//  Copyright (c) 2015, 2016, 2017, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "clopt.h"

#include <sstream>
#include <iostream>
#include <cstdlib>
#include <algorithm>



namespace util {

CLOpts::CLOpts(int argc, const char * const *argv)
    : _argv(&argv[0], &argv[argc]) {
}


void CLOpts::add(const std::string &name, const std::string &shortOpt,
  const std::string &longOpt, const std::string &desc) {
    _opts.emplace_back(name, shortOpt, longOpt, desc);
}


void CLOpts::add(const std::string &name, const std::string &shortOpt,
  const std::string &longOpt, const std::string &value,
  const std::string &desc) {
    _opts.emplace_back(name, shortOpt, longOpt, desc, true, value);
}


std::string CLOpts::pname(void) const {
    std::string rval(_argv[0]);

    size_t fpos = rval.rfind("/");
    if (fpos != std::string::npos)
        rval = rval.substr(fpos+1);

    return rval;
}


std::string CLOpts::useage(void) const {
    std::ostringstream os;

    os << "Usage: " << pname() << " [options]\n\n";

    os << "Options:\n";
    for (const auto &opt : _opts)
        os << opt.usage() << "\n";

    return os.str();
}


void CLOpts::parse(void) {
    if (_argv.empty())
        return;

    auto argp = _argv.cbegin() + 1;
    while (argp < _argv.cend()) {
        bool skipTwo = false;
        std::string arg(*argp);
        bool found = false;
        for (auto &opt : _opts) {
            if (opt.match(arg)) {
                found = true;
                if (!opt.isValue())
                    opt.setValue();
                else {
                    if (argp + 1 >= _argv.cend()) {
                        fail("missing value for option: " + arg);
                    }
                    else {
                        std::string next(*(argp + 1));
                        for (const auto &o : _opts) {
                            if (o.match(next))
                                fail("missing value for option: " + arg);
                        }
                        opt.setValue(next);
                        // after checking the rest skip over the value
                        skipTwo = true;
                    }
                }
            }
        }
        if (!found) {
            if (arg.substr(0, 1) == "-")
                fail("unsupported opton: " + arg);
            _args.push_back(arg);
        }
        argp++;
        if (skipTwo)
            argp++;
    }
}


void CLOpts::fail(const std::string &msg) const {
    std::cerr << "ERROR: " << msg << "\n\n" << useage() << std::endl;
    exit(1);
}


CLOpt::CLOpt(const std::string &name, const std::string &shortOpt,
  const std::string &longOpt, const std::string &desc,
  bool isValue, const std::string &valDesc)
    : _name(name), _shortOpt(shortOpt), _longOpt(longOpt), _desc(desc),
      _isValue(isValue), _valDesc(valDesc) {
}


std::string CLOpt::usage(void) const {
    std::ostringstream os;
    if (_isValue)
        os << _longOpt << ", " << _shortOpt << " " << "<" << _valDesc << ">";
    else
        os << _longOpt << ", " << _shortOpt;

    std::string rval(os.str());
    if (rval.size() < 30)
        rval.resize(30, ' ');

    return rval + formatedDesc(30);
}


std::string CLOpt::formatedDesc(size_t offset) const {
    // Format the description so that it starts on column offset
    // and does not extend (most of the time) past column 80.

    std::string rval;
    size_t pos = 0;
    size_t colnum = offset;
    while (pos < _desc.size()) {
        size_t fpos = _desc.find(" ", pos);
        if (fpos == std::string::npos) {
            rval += _desc.substr(pos);
            break;
        }
        size_t wlen = fpos - pos + 1;
        std::string sstr = _desc.substr(pos, wlen);
        if ((colnum + wlen) < 80) {
            rval += sstr;
            colnum += wlen;
        }
        else {
            rval += "\n" + std::string(offset, ' ') + sstr;
            colnum = offset + wlen;
        }

        pos += wlen;

    }

    return rval;
}


void CLOpt::eraseFrom(const std::string &opt,
  std::vector<std::string> &argv) const {

    if (missing())
        return;

    auto it = std::find(argv.begin(), argv.end(), opt);

    if (it == argv.end())
        return;

    if (!isValue())
        argv.erase(it, it + 1);
    else {
        if (it + 2 <= argv.end())
            argv.erase(it, it + 2);
    }
}


void CLOpt::eraseFrom(std::vector<std::string> &argv) const {
    eraseFrom(_shortOpt, argv);
    eraseFrom(_longOpt, argv);
}


std::ostream &CLOpt::printOn(std::ostream &os) const {
    os << "[" << name() << ": " << _shortOpt << " [";
    for (auto it = _values.cbegin() ; it != _values.cend() ; ++it) {
        os << *it;
        if ((it + 1) != _values.cend())
            os << ",";
    }
    os << "]]";

    return os;
}


std::ostream &CLOpts::printOn(std::ostream &os) const {
    os << "opts: [\n";
    for (auto it = _opts.cbegin() ; it != _opts.cend() ; ++it) {
        os << "\t" << *it;
        if ((it + 1) != _opts.cend())
            os << ",\n";
    }
    os << "\nargs: [";
    for (auto it = _args.cbegin() ; it != _args.cend() ; ++it) {
        os << *it;
        if ((it + 1) != _args.cend())
            os << ",";
    }
    os << "]";

    return os;
}


const std::vector<std::string> &CLOpts::values(const std::string &name) const {
    static std::vector<std::string> rval;
    for (const auto &opt : _opts)
        if (name == opt.name())
            return opt.values();

    return rval;
}


const std::string &CLOpts::value(const std::string &name) const {
    static const std::string rval;
    for (const auto &opt : _opts)
        if (name == opt.name())
            return opt.value();

    return rval;
}


const std::string &CLOpts::value(const std::string &name,
  const std::string &defaultVal) const {
    for (const auto &opt : _opts)
        if (opt.name() == name) {
            if (opt.missing())
                return defaultVal;
            return opt.value();
        }

    return defaultVal;
}


bool CLOpts::isTrue(const std::string &name) const {
    for (const auto &opt : _opts)
        if (opt.name() == name)
            return opt.isTrue();

    return false;
}


bool CLOpts::missing(const std::string &name) const {
    for (const auto &opt : _opts)
        if (opt.name() == name)
            return opt.missing();

    return true;
}


const std::string &CLOpt::value(void) const {
    static const std::string rval;
    if (!_values.empty())
        return _values.back();  // last one wins
    if (!isValue()) { // boolean opts default
        static const std::string False("False");
        return False;
    }

    return rval;
}


bool CLOpt::isTrue(void) const {
    if (isValue()) // make something up
        return !missing();

    return !(value() == "False");
}


} // end namespace util
