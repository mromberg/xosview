//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//

#ifndef wlinkmeter_h
#define wlinkmeter_h

#include "fieldmetergraph.h"


class WLinkMeter : public FieldMeterGraph {
public:
    WLinkMeter(void);
    virtual ~WLinkMeter(void);

    virtual std::string resName( void ) const override { return "wlink"; }
    virtual void checkevent(void) override;
    virtual void checkResources(const ResDB &rdb) override;

private:
    unsigned long _goodColor;
    unsigned long _poorColor;
    int _poorValue;

    int getLink(void) const;
};

#endif
