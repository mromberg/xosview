//  
// $Id: swapmeter.h,v 1.1 1998/06/22 14:26:07 bgrayson Exp $
//
#ifndef _SWAPMETER_H_
#define _SWAPMETER_H_

#include "fieldmeterdecay.h"
#include <kstat.h>

class SwapMeter : public FieldMeterDecay {
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
