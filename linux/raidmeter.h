//
//  Copyright (c) 1999, 2006, 2015
//  by Thomas Waldmann ( ThomasWaldmann@gmx.de )
//  based on work of Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef _RAIDMETER_H_
#define _RAIDMETER_H_

#include "bitfieldmeter.h"

#define MAX_MD 8

class RAIDMeter : public BitFieldMeter {
public:
    RAIDMeter( XOSView *parent, int raiddev = 0);
    ~RAIDMeter( void );

    void checkevent( void );

    void checkResources( void );

    static int countRAIDs( void );

protected:

    int _raiddev;
    static int mdnum;

    std::string state,
        type,
        working_map,
        resync_state;
    int  disknum;

    unsigned long doneColor_, todoColor_, completeColor_;

    int find1(const std::string &key, const std::string &findwhat, int num1);
    int find2(const std::string &key, const std::string &findwhat, int num1,
      int num2);

    int raidparse(const std::string &cp);

    void getRAIDstate( void );
};

#endif
