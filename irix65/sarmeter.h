//  
// $Id: sarmeter.h,v 1.1 2002/02/20 11:15:04 eile Exp $
//  Initial port performed by Stefan Eilemann (eile@sgi.com)
//

#ifndef _SARMETER_H_
#define _SARMETER_H_

#include "fieldmetergraph.h"
#include <unistd.h>

// common function for all sar based graphs
class SarMeter
{
public:
	SarMeter() {}
	~SarMeter(void) {}

protected:
    size_t readLine( int input, char *buf, size_t max );
    int    setupSar( const char *option );
};

#endif
