//
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
#ifndef _SWAPMETER_H_
#define _SWAPMETER_H_

#include "fieldmetergraph.h"
#include "xosview.h"
#include <kstat.h>


class SwapMeter : public FieldMeterGraph {
 public:
	SwapMeter(XOSView *parent);
	~SwapMeter(void);

        virtual std::string name(void) const { return "SwapMeter"; }
	void checkevent(void);
	void checkResources(void);

 protected:
	void getswapinfo(void);

 private:
	size_t pagesize;
};


#endif
