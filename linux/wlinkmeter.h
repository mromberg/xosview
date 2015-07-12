//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//

#ifndef WLINKMETER_H
#define WLINKMETER_H

#include "fieldmetergraph.h"  // our parent drawing class

class WLinkMeter : public FieldMeterGraph {
public:
    WLinkMeter(XOSView *parent);
    virtual ~WLinkMeter(void);

    virtual std::string name( void ) const { return "WLinkMeter"; }
    virtual void checkevent( void );
    virtual void checkResources( void );

private:
    unsigned long _goodColor;
    unsigned long _poorColor;
    int _poorValue;

    int getLink(void) const;
};

#endif
