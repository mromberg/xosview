//  
// $Id: swapmeter.h,v 1.3 1999/01/31 20:26:38 bgrayson Exp $
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
#ifndef _SWAPMETER_H_
#define _SWAPMETER_H_

#include "fieldmetergraph.h"
#include <kstat.h>

class SwapMeter : public FieldMeterGraph {
 public:
	SwapMeter(XOSView *parent, kstat_ctl_t *kcp);
	~SwapMeter(void);

	const char *name(void) const { return "SwapMeter"; }  
	void checkevent(void);
	void checkResources(void);

 protected:
	void getswapinfo(void);

 private:
};


#endif
