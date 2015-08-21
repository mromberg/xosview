#ifndef defines_h
#define defines_h

#include "configxosv.h"

#include <sys/param.h>


/* swapctl appeared on NetBSD 1.3. and OpenBSD 2.6 */
#if ( defined(__NetBSD__) && __NetBSD_Version__ >= 103000000 ) || \
    ( defined(__OpenBSD__) && OpenBSD >= 199912 )
#define HAVE_SWAPCTL 1
#endif


/* kvm_getswapinfo appeared on FreeBSD 4.0 */
#if ( defined(__FreeBSD__) && __FreeBSD_version >= 400000 ) || \
      defined(__DragonFly__)
#define USE_KVM_GETSWAPINFO 1
#endif

/* Helper defines for battery meter. */
#define XOSVIEW_BATT_NONE         0
#define XOSVIEW_BATT_CHARGING     1
#define XOSVIEW_BATT_DISCHARGING  2
#define XOSVIEW_BATT_FULL         4
#define XOSVIEW_BATT_LOW          8
#define XOSVIEW_BATT_CRITICAL    16


#endif
