//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "rdb.h"
#include "log.h"
#include "x11graphics.h"



ResDB::ResDB(void)
    : _g(nullptr) {
}


ResDB::~ResDB(void) {
}


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


std::string ResDB::getResource(const std::string &name) const {
    opt o = getOptResource(name);

    if (!o.first)
        logFatal << "resource: " << name << " not found." << std::endl;

    return o.second;
}


std::string ResDB::getResourceOrUseDefault(const std::string &name,
  const std::string &defaultVal) const {

    opt o = getOptResource(name);

    if (!o.first)
        return defaultVal;

    return o.second;
}


bool ResDB::isResourceTrue(const std::string &name) const {
    opt o = getOptResource(name);

    if (!o.first)
        return false;

    return o.second == "True";
}
