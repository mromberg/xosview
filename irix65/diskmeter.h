//  
// $Id: diskmeter.h,v 1.1 2002/04/24 08:23:11 eile Exp $
//  Initial port performed by Stefan Eilemann (eile@sgi.com)
//

#ifndef _DISKMETER_H_
#define _DISKMETER_H_

#include "fieldmetergraph.h"

class DiskMeter : public FieldMeterGraph 
{
public:
    DiskMeter( XOSView *parent, float max );
    ~DiskMeter( void );
    
    const char *name( void ) const { return "DiskMeter"; }
    void checkevent( void );
    
    void checkResources( void );
protected:
    
    void getdiskinfo( void );
private:
    unsigned long int read_prev_;
    unsigned long int write_prev_;
    float maxspeed_;
};

#endif
