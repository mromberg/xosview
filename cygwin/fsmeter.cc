//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "fsmeter.h"


std::vector<std::string> FSMeterFactory::getAuto(void) {
    //  Just return /

    std::vector<std::string> rval;

    rval.push_back("/");

    return rval;
}
