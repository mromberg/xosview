//  
// $Id: pagemeter.h,v 1.3 1999/01/31 20:26:38 bgrayson Exp $
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
#ifndef _PAGEMETER_H_
#define _PAGEMETER_H_

#include "fieldmetergraph.h"
#include <kstat.h>

class PageMeter : public FieldMeterGraph {
 public:
	PageMeter(XOSView *parent, kstat_ctl_t *kcp, float max);
	~PageMeter(void);

	const char *name(void) const { return "PageMeter"; }
	void checkevent(void);

	void checkResources(void);

 protected:
	float pageinfo_[2][2];
	int pageindex_;
	float maxspeed_;

	void getpageinfo(void);

 private:
	kstat_ctl_t *kc;
	kstat_t *ksps[64];	/* XXX */
	int ncpus;
};

#endif
