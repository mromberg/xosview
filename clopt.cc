#include "clopt.h"

#include <sstream>
#include <iostream>
#include <cstdlib>

namespace util {

void CLOpts::add(const std::string &name, const std::string &shortOpt,
  const std::string &longOpt, const std::string &desc) {
    _opts.push_back(CLOpt(name, shortOpt, longOpt, desc));
}

void CLOpts::add(const std::string &name, const std::string &shortOpt,
  const std::string &longOpt, const std::string &value,
  const std::string &desc) {
    _opts.push_back(CLOpt(name, shortOpt, longOpt, desc, true, value));
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
    for (size_t i = 0 ; i < _opts.size() ; i++)
        os << _opts[i].usage() << "\n";

    return os.str();
}

void CLOpts::parse(void) {
    char **argp = &_argv[1];
    while (*argp) {
        bool skipTwo = false;
        std::string arg(*argp);
        bool found = false;
        for (size_t i = 0 ; i < _opts.size() ; i++) {
            if (_opts[i].match(arg)) {
                found = true;
                if (!_opts[i].isValue())
                    _opts[i].setValue();
                else {
                    char **nextp = argp + 1;
                    if (!*nextp) {
                        fail("missing value for option: " + arg);
                    }
                    else {
                        std::string next(*nextp);
                        for (size_t j = 0 ; j < _opts.size() ; j++) {
                            if (_opts[j].match(next)) {
                                fail("missing value for option: " + arg);
                            }
                        }
                        _opts[i].setValue(next);
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

void CLOpt::printOn(std::ostream &os) const {
    os << "[" << name() << ": " << _shortOpt << " [";
    for (size_t i = 0 ; i < _values.size() ; i++) {
        os << _values[i];
        if (i + 1 != _values.size())
            os << ",";
    }
    os << "]]";
}

void CLOpts::printOn(std::ostream &os) const {
    os << "opts: [\n";
    for (size_t i = 0 ; i < _opts.size() ; i++) {
        os << "\t" << _opts[i];
        if (i + 1 < _opts.size())
            os << ",\n";
    }
    os << "\nargs: [";
    for (size_t i = 0 ; i < _args.size() ; i++) {
        os << _args[i];
        if (i + 1 < _args.size())
            os << ",";
    }
    os << "]";
}

const std::vector<std::string> &CLOpts::values(const std::string &name) const {
    static std::vector<std::string> rval;
    for (size_t i = 0 ; i < _opts.size() ; i++)
        if (name == _opts[i].name())
            return _opts[i].values();

    return rval;
}

const std::string &CLOpts::value(const std::string &name) const {
    static const std::string rval;
    for (size_t i = 0 ; i < _opts.size() ; i++)
        if (name == _opts[i].name())
            return _opts[i].value();

    return rval;
}

const std::string &CLOpts::value(const std::string &name,
  const std::string &defaultVal) const {
    for (size_t i = 0 ; i < _opts.size() ; i++)
        if (_opts[i].name() == name) {
            if (_opts[i].missing())
                return defaultVal;
            return _opts[i].value();
        }

    return defaultVal;
}

bool CLOpts::isTrue(const std::string &name) const {
    for (size_t i = 0 ; i < _opts.size() ; i++)
        if (_opts[i].name() == name) {
            return _opts[i].isTrue();
        }

    return false;
}

bool CLOpts::missing(const std::string &name) const {
    for (size_t i = 0 ; i < _opts.size() ; i++)
        if (_opts[i].name() == name)
            return _opts[i].missing();
    return true;
}

const std::string &CLOpt::value(void) const {
    static const std::string rval;
    if (_values.size() > 0)
        return _values[_values.size()-1];  // last one wins
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
