//  
// $Id: memmeter.h,v 1.1 1998/06/22 14:26:07 bgrayson Exp $
//
#ifndef _MEMMETER_H_
#define _MEMMETER_H_

#include "fieldmeterdecay.h"
#include <kstat.h>

class MemMeter : public FieldMeterDecay {
 public:
	MemMeter(XOSView *parent, kstat_ctl_t *kcp);
	~MemMeter(void);

	const char *name(void) const { return "MemMeter"; }  
	void checkevent( void );

	void checkResources(void);

 protected:
	//  struct pst_status *stats_;
	int _pageSize;

	void getmeminfo( void );

 private:
	kstat_ctl_t *kc;
	kstat_t *ksp;
};


#endif
