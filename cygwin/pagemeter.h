//
//  Copyright (c) 1996, 2007, 2015
//  by Massimiliano Ghilardi ( ghilardi@cibs.sns.it )
//
//  This file may be distributed under terms of the GPL
//
#ifndef PAGEMETER_H
#define PAGEMETER_H

#include "fieldmetergraph.h"


class PageMeter : public FieldMeterGraph {
public:
    PageMeter( XOSView *parent, float max );
    ~PageMeter( void );

    std::string name( void ) const { return "PageMeter"; }
    void checkevent( void );

    void checkResources(const ResDB &rdb);

private:
    std::vector<std::vector<float> > pageinfo_;
    int pageindex_;
    float maxspeed_;
    bool _vmstat;
    std::string _statFileName;

    void getpageinfo( void );
    void getvmpageinfo( void );
    void updateinfo(void);
};


#endif
