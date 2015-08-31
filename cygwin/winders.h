//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef WINDERS_H
#define WINDERS_H

#include <vector>
#include <string>


class WinHardware {
public:
    static std::vector<std::string> getAdapters(void);
};


#endif
