//
//  Copyright (c) 2016, 2017, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef sctl_h
#define sctl_h

#include <string>
#include <vector>

// included here so clients don't have to.
#include <sys/types.h>
#include <sys/sysctl.h>



class SysCtl {
public:
    SysCtl(void);
    SysCtl(const std::vector<int> &mib);
    SysCtl(std::initializer_list<int> l);
    SysCtl(const int *mib, size_t mibsize);
#if !defined(XOSVIEW_OPENBSD)
    SysCtl(const std::string &id);
#endif

    const std::string &id(void) const { return _id; }

    bool get(void *dst, size_t dsize) const;
    bool getsize(size_t &size) const;

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

#if !defined(XOSVIEW_OPENBSD)
    void init(void);
#endif
};


#if defined(XOSVIEW_FREEBSD)
// For whatever reason the idea of one sysctl to return the
// whole vmmeter struct was not appealing.  So...
#define GET_VM_STAT(vmmeter, name)                                            \
    static SysCtl stats_vm_ ## name ## _sc("vm.stats.vm." #name);             \
    if (!stats_vm_ ## name ## _sc.get(vmmeter.name))                          \
        logFatal << "sysctl(" << stats_vm_ ## name ## _sc.id() << ") failed." \
                 << std::endl
#endif

#endif
