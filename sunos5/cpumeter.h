//  
// $Id: cpumeter.h,v 1.1 1998/06/22 14:26:07 bgrayson Exp $
//
#ifndef _CPUMETER_H_
#define _CPUMETER_H_

#include "fieldmeterdecay.h"

#include <kstat.h>
#include <sys/sysinfo.h>

class CPUMeter : public FieldMeterDecay {
 public:
	CPUMeter(XOSView *parent, kstat_ctl_t *kcp, const int cpuid = 0);
	~CPUMeter(void);

	const char *name(void) const { return "CPUMeter"; }
	void checkevent(void);

	void checkResources(void);

	static int countCPUs(kstat_ctl_t *kc);
	static const char *cpuStr(int num);

 protected:
	float cputime_[2][CPU_STATES];
	int cpuindex_;

	void getcputime(void);
	const char *toUpper(const char *str);

 private:
	kstat_ctl_t *kc;
	kstat_t *ksp;
};

#endif
