//
//  Copyright (c) 1996, 2015
//  by Massimiliano Ghilardi ( ghilardi@cibs.sns.it )
//  2007 by Samuel Thibault ( samuel.thibault@ens-lyon.org )
//
//  This file may be distributed under terms of the GPL
//
#ifndef PAGEMETER_H
#define PAGEMETER_H


#include "ppagemeter.h"


class PageMeter : public PrcPageMeter {
public:
    PageMeter( void ) : PrcPageMeter(false) {}
};


#endif
