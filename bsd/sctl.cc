//
//  Copyright (c) 2016, 2017, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "sctl.h"
#include "strutil.h"
#include "log.h"



SysCtl::SysCtl(void) {
}


SysCtl::SysCtl(const std::vector<int> &mib) : _mib(mib) {
}


SysCtl::SysCtl(std::initializer_list<int> l)
    : _mib(std::move(l)) {
}


SysCtl::SysCtl(const int *mib, size_t mibsize)
    : _mib(mib, mib + mibsize) {
}


#if !defined(XOSVIEW_OPENBSD)
SysCtl::SysCtl(const std::string &id)
    : _id(id) {
    init();
}
#endif


#if !defined(XOSVIEW_OPENBSD)
void SysCtl::init(void) {

    size_t sizep = 0;
    if (sysctlnametomib(_id.c_str(), nullptr, &sizep) < 0) {
        logProblem << "sysctlnametomib(" << _id << ") failed." << std::endl;
    }

    _mib.resize(sizep);
    sizep = _mib.size();
    if (sysctlnametomib(_id.c_str(), _mib.data(), &sizep) < 0) {
        logProblem << "sysctlnametomib(" << _id << ") failed." << std::endl;
    }
}
#endif


bool SysCtl::get(void *dst, size_t dsize) const {
    if (sysctl(_mib.data(), _mib.size(), dst, &dsize, nullptr, 0) < 0) {
        logDebug << "sysctl(" << id() << ") failed: "
                 << util::strerror() << std::endl;
        return false;
    }

    return true;
}


bool SysCtl::getsize(size_t &size) const {
    if (sysctl(_mib.data(), _mib.size(), nullptr, &size, nullptr, 0) < 0)
        return false;

    return true;
}
