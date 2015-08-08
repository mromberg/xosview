//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#ifndef RDB_H
#define RDB_H

#include "log.h"

#include <string>
#include <utility>
#include <iostream>


class ResDB {
public:
    virtual std::string className(void) const = 0;
    virtual std::string instanceName(void) const = 0;

    typedef std::pair<bool, std::string> opt;
    virtual opt getOptResource(const std::string &rname) const = 0;

    // Fatal error if not found.
    virtual std::string getResource(const std::string &name) const;

    virtual std::string getResourceOrUseDefault(const std::string &name,
      const std::string &defaultVal) const;

    virtual bool isResourceTrue(const std::string &name) const;

    virtual std::ostream &dump(std::ostream &os) const = 0;
};


inline std::string ResDB::getResource(const std::string &name) const {
    opt o = getOptResource(name);

    if (!o.first)
        logFatal << "resource: " << name << " not found." << std::endl;

    return o.second;
}


inline std::string ResDB::getResourceOrUseDefault(const std::string &name,
  const std::string &defaultVal) const {

    opt o = getOptResource(name);

    if (!o.first)
        return defaultVal;

    return o.second;
}


inline bool ResDB::isResourceTrue(const std::string &name) const {
    opt o = getOptResource(name);

    if (!o.first)
        return false;

    return o.second == "True";
}


#endif
