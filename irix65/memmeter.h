//  
// $Id: memmeter.h,v 1.1 2001/10/10 15:58:22 eile Exp $
//  Initial port performed by Stefan Eilemann (eile@sgi.com)
//
#ifndef _MEMMETER_H_
#define _MEMMETER_H_

#include "fieldmetergraph.h"
#include <sys/types.h>
#include <sys/sysmp.h>
#include <sys/sysinfo.h> /* for SAGET and MINFO structures */

class MemMeter : public FieldMeterGraph {
 public:
	MemMeter(XOSView *parent);
	~MemMeter(void);

	const char *name(void) const { return "MemMeter"; }  
	void checkevent( void );

	void checkResources(void);

 protected:
	//  struct pst_status *stats_;
	int _pageSize;

	void getmeminfo( void );

 private:
    struct rminfo  mp;
    int            minfosz;
};


#endif
