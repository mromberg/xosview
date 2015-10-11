//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//

#ifndef WLINKMETER_H
#define WLINKMETER_H

#include "fieldmetergraph.h"


class WLinkMeter : public FieldMeterGraph {
public:
    WLinkMeter(XOSView *parent);
    virtual ~WLinkMeter(void);

    virtual std::string resName( void ) const { return "wlink"; }
    virtual void checkevent( void );
    virtual void checkResources(const ResDB &rdb);

private:
    unsigned long _goodColor;
    unsigned long _poorColor;
    int _poorValue;

    int getLink(void) const;
};

#endif
