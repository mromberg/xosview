//
//  Copyright (c) 1994, 1995, 2015, 2016 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  NetBSD port:
//  Copyright (c) 1995, 1996, 1997-2002 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was originally written by Brian Grayson for the NetBSD and
//    xosview projects.
//
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#include "pagemeter.h"
#include "sctl.h"

#include <unistd.h>

#if defined(XOSVIEW_DFBSD)
#include <sys/vmmeter.h>
#endif



PageMeter::PageMeter( void )
    : ComPageMeter(), _psize(sysconf(_SC_PAGESIZE)), _previnfo(2, 0) {

    getPageStats(_previnfo);
    IntervalTimerStart();
}


std::pair<float, float> PageMeter::getPageRate(void) {

    IntervalTimerStop();
    double t = IntervalTimeInSecs();
    std::vector<uint64_t> info;
    getPageStats(info);
    IntervalTimerStart();

    std::pair<float, float> rval(((info[0] - _previnfo[0]) * _psize) / t ,
      ((info[1] - _previnfo[1]) * _psize) / t);

    _previnfo[0] = info[0];
    _previnfo[1] = info[1];

    return rval;
}


#if defined(XOSVIEW_DFBSD)
void PageMeter::getPageStats(std::vector<uint64_t> &pageinfo) {

    static SysCtl vmmeter_sc("vm.vmmeter");

    struct vmmeter vm;
    if (!vmmeter_sc.get(vm))
        logFatal << "sysctl(" << vmmeter_sc.id() << "failed." << std::endl;

    pageinfo.resize(2);
    pageinfo[0] = (uint64_t)vm.v_vnodepgsin + (uint64_t)vm.v_swappgsin;
    pageinfo[1] = (uint64_t)vm.v_vnodepgsout + (uint64_t)vm.v_swappgsout;
}
#endif


#if defined(XOSVIEW_FREEBSD)
void PageMeter::getPageStats(std::vector<uint64_t> &pageinfo) {

    struct vmmeter vm;

    GET_VM_STAT(vm, v_vnodepgsin);
    GET_VM_STAT(vm, v_vnodepgsout);
    GET_VM_STAT(vm, v_swappgsin);
    GET_VM_STAT(vm, v_swappgsout);

    pageinfo.resize(2);
    pageinfo[0] = (uint64_t)vm.v_vnodepgsin + (uint64_t)vm.v_swappgsin;
    pageinfo[1] = (uint64_t)vm.v_vnodepgsout + (uint64_t)vm.v_swappgsout;
}
#endif


#if defined(XOSVIEW_NETBSD)
void PageMeter::getPageStats(std::vector<uint64_t> &pageinfo) {
    static SysCtl uvmexp2_sc("vm.uvmexp2");

    struct uvmexp_sysctl uvm;
    if (!uvmexp2_sc.get(uvm))
        logFatal << "sysctl(" << uvmexp2_sc.id() << ") failed." << std::endl;

    pageinfo.resize(2);
    pageinfo[0] = (uint64_t)uvm.pgswapin;
    pageinfo[1] = (uint64_t)uvm.pgswapout;
}
#endif


#if defined(XOSVIEW_OPENBSD)
void PageMeter::getPageStats(std::vector<uint64_t> &pageinfo) {
    const int mib_uvm[] = { CTL_VM, VM_UVMEXP };
    static SysCtl uvmexp_sc(mib_uvm, 2);

    struct uvmexp uvm;
    if (!uvmexp_sc.get(uvm))
        logFatal << "sysctl(" << uvmexp_sc.id() << ") failed." << std::endl;

    pageinfo.resize(2);
    pageinfo[0] = (uint64_t)uvm.pgswapin;
    pageinfo[1] = (uint64_t)uvm.pgswapout;
}
#endif
