//  
// $Id: gfxmeter.h,v 1.1 2002/02/20 11:15:04 eile Exp $
//  Initial port performed by Stefan Eilemann (eile@sgi.com)
//
#ifndef _GFXMETER_H_
#define _GFXMETER_H_

#include "sarmeter.h"

#include <rpcsvc/rstat.h>
#include <sys/sysinfo.h>

class GfxMeter : public FieldMeterGraph, public SarMeter {
 public:
	GfxMeter(XOSView *parent, int max);
	~GfxMeter(void);

	const char *name(void) const { return "GfxMeter"; }  
	void checkevent(void);

	void checkResources(void);

protected:
    void getgfxinfo(void);
    
    unsigned long swapgfxcol_, warngfxcol_, critgfxcol_;
private:
    int warnThreshold, critThreshold, alarmstate, lastalarmstate;
    int nPipes;
    int input;
    struct statstime res;

    gfxinfo gi;
    unsigned int lastSwapBuf;
    off_t lastPos;
    char buf[50000];
};

#endif
