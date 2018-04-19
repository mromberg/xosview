//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef winders_h
#define winders_h

#include <vector>
#include <string>


class WinHardware {
public:
    static std::vector<std::string> getAdapters(void);
};


#endif
