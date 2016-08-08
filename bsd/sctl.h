//
//  Copyright (c) 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef sctl_h
#define sctl_h

#include <iostream>
#include <string>
#include <vector>

#include <sys/sysctl.h>


class SysCtl {
public:
    SysCtl(void) {}
    SysCtl(const std::vector<int> &mib) : _mib(mib) {}
#if !defined(XOSVIEW_OPENBSD)
    SysCtl(const std::string &id) : _id(id) { init(); }
#endif

    const std::string &id(void) const { return _id; }

    bool get(void *dst, size_t dsize) const {
        if (sysctl(_mib.data(), _mib.size(), dst, &dsize, NULL, 0) < 0)
            return false;

        return true;
    }

    template <class X>
    bool get(X &x) const {
        size_t size = sizeof(X);
        return get(&x, size);
    }

    template <class X>
    bool get(std::vector<X> &v) const {
        size_t size = v.size() * sizeof(X);
        return get(v.data(), size);
    }

    std::vector<int> &mib(void) { return _mib; }

private:
    std::string _id;
    std::vector<int> _mib;

    void init(void) {
#if !defined(XOSVIEW_OPENBSD)
        size_t sizep = 0;
        if (sysctlnametomib(_id.c_str(), NULL, &sizep) < 0) {
            std::cerr << "sysctlnametomib(" << _id << ") failed." << std::endl;
        }

        _mib.resize(sizep);
        sizep = _mib.size();
        if (sysctlnametomib(_id.c_str(), _mib.data(), &sizep) < 0) {
            std::cerr << "sysctlnametomib(" << _id << ") failed." << std::endl;
        }
#endif
    }
};

#endif
