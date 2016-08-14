//
//  Copyright (c) 1994, 1995, 2015, 2016
//  by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  NetBSD port:
//  Copyright (c) 1995, 1996, 1997-2002 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file was originally written by Brian Grayson for the NetBSD and
//    xosview projects.
//  The NetBSD memmeter was improved by Tom Pavel (pavel@slac.stanford.edu)
//    to provide active and inactive values, rather than just "used."
//
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#include "memmeter.h"
#include "sctl.h"

#if defined(HAVE_UVM)
#include <sys/device.h>
#include <uvm/uvm_extern.h>
#else
#include <sys/vmmeter.h>
#endif



#if defined(HAVE_UVM)
static const char * const LEGEND = "ACT/INACT/WRD/FREE";
static size_t NFIELDS = 4;
#else
static const char * const LEGEND = "ACT/INACT/WRD/CA/FREE";
static size_t NFIELDS = 5;
#endif



MemMeter::MemMeter( void )
    : FieldMeterGraph( NFIELDS, "MEM", LEGEND ),
      _meminfo(5, 0) {
}


MemMeter::~MemMeter( void ) {
}


void MemMeter::checkResources(const ResDB &rdb) {

    FieldMeterGraph::checkResources(rdb);

    setfieldcolor( 0, rdb.getColor("memActiveColor") );
    setfieldcolor( 1, rdb.getColor("memInactiveColor") );
    setfieldcolor( 2, rdb.getColor("memWiredColor") );
#if defined(HAVE_UVM)
    setfieldcolor( 3, rdb.getColor("memFreeColor") );
#else
    setfieldcolor( 3, rdb.getColor("memCacheColor") );
    setfieldcolor( 4, rdb.getColor("memFreeColor") );
#endif
}


void MemMeter::checkevent( void ) {
    getmeminfo();
}


void MemMeter::getmeminfo( void ) {
    getMemStats(_meminfo);
    fields_[0] = (double)_meminfo[0];
    fields_[1] = (double)_meminfo[1];
    fields_[2] = (double)_meminfo[2];
#if defined(HAVE_UVM)
    fields_[3] = (double)_meminfo[4];
#else
    fields_[3] = (double)_meminfo[3];
    fields_[4] = (double)_meminfo[4];
#endif
    total_ = (double)(_meminfo[0] + _meminfo[1] + _meminfo[2] + _meminfo[3]
      + _meminfo[4]);
    setUsed(total_ - (double)_meminfo[4], total_);
}


#if defined(XOSVIEW_DFBSD)
void MemMeter::getMemStats(std::vector<uint64_t> &meminfo) {

    static SysCtl vmstats_sc("vm.vmstats");

    struct vmstats vms;
    if (!vmstats_sc.get(vms))
        logFatal << "sysctl(" << vmstats_sc.id() << ") failed." << std::endl;

    meminfo.resize(5);
    meminfo[0] = (uint64_t)vms.v_active_count * vms.v_page_size;
    meminfo[1] = (uint64_t)vms.v_inactive_count * vms.v_page_size;
    meminfo[2] = (uint64_t)vms.v_wire_count * vms.v_page_size;
    meminfo[3] = (uint64_t)vms.v_cache_count * vms.v_page_size;
    meminfo[4] = (uint64_t)vms.v_free_count * vms.v_page_size;
}
#endif


#if defined(XOSVIEW_FREEBSD)
void MemMeter::getMemStats(std::vector<uint64_t> &meminfo) {

    struct vmmeter vm;

    GET_VM_STAT(vm, v_active_count);
    GET_VM_STAT(vm, v_inactive_count);
    GET_VM_STAT(vm, v_wire_count);
    GET_VM_STAT(vm, v_cache_count);
    GET_VM_STAT(vm, v_free_count);
    GET_VM_STAT(vm, v_page_size);

    meminfo.resize(5);
    meminfo[0] = (uint64_t)vm.v_active_count * vm.v_page_size;
    meminfo[1] = (uint64_t)vm.v_inactive_count * vm.v_page_size;
    meminfo[2] = (uint64_t)vm.v_wire_count * vm.v_page_size;
    meminfo[3] = (uint64_t)vm.v_cache_count * vm.v_page_size;
    meminfo[4] = (uint64_t)vm.v_free_count * vm.v_page_size;
}
#endif


#if defined(XOSVIEW_NETBSD)
void MemMeter::getMemStats(std::vector<uint64_t> &meminfo) {
    static SysCtl uvmexp2_sc("vm.uvmexp2");

    struct uvmexp_sysctl uvm;
    if (!uvmexp2_sc.get(uvm))
        logFatal << "sysctl(" << uvmexp2_sc.id() << ") failed." << std::endl;

    meminfo.resize(5);
    // UVM excludes kernel memory -> assume it is active mem
    meminfo[0] = (uint64_t)(uvm.npages - uvm.inactive - uvm.wired
      - uvm.free) * uvm.pagesize;
    meminfo[1] = (uint64_t)uvm.inactive * uvm.pagesize;
    meminfo[2] = (uint64_t)uvm.wired * uvm.pagesize;

    // cache is already included in active and inactive memory and
    // there's no way to know how much is in which -> disable cache
    meminfo[3] = 0;
    meminfo[4] = (uint64_t)uvm.free * uvm.pagesize;
}
#endif


#if defined(XOSVIEW_OPENBSD)
void MemMeter::getMemStats(std::vector<uint64_t> &meminfo) {
    const int mib_uvm[] = { CTL_VM, VM_UVMEXP };
    static SysCtl uvmexp_sc(mib_uvm, 2);

    struct uvmexp uvm;
    if (!uvmexp_sc.get(uvm))
        logFatal << "sysctl(" << uvmexp_sc.id() << ") failed." << std::endl;

    meminfo.resize(5);
    // UVM excludes kernel memory -> assume it is active mem
    meminfo[0] = (uint64_t)(uvm.npages - uvm.inactive - uvm.wired
      - uvm.free) * uvm.pagesize;
    meminfo[1] = (uint64_t)uvm.inactive * uvm.pagesize;
    meminfo[2] = (uint64_t)uvm.wired * uvm.pagesize;

    // cache is already included in active and inactive memory and
    // there's no way to know how much is in which -> disable cache
    meminfo[3] = 0;
    meminfo[4] = (uint64_t)uvm.free * uvm.pagesize;
}
#endif
