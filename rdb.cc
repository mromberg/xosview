//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "rdb.h"
#include "x11graphics.h"


unsigned long ResDB::getColor(const std::string &name) const {
    if (_g)
        return _g->allocColor(getResource(name));

    logBug << "Graphics not set in getColor()" << std::endl;
    return 0;
}


unsigned long ResDB::getColor(const std::string &name,
  const std::string &deflt) const {

    if (_g)
        return _g->allocColor(getResourceOrUseDefault(name, deflt));

    logBug << "Graphics not set in getColor()" << std::endl;
    return 0;
}
