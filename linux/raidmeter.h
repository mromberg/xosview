//
//  Copyright (c) 1999, 2006, 2015
//  by Thomas Waldmann ( ThomasWaldmann@gmx.de )
//  based on work of Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef RAIDMETER_H
#define RAIDMETER_H

#include "bitfieldmeter.h"



class RAIDMeter : public BitFieldMeter {
public:
    RAIDMeter( XOSView *parent, int raiddev = 0);
    ~RAIDMeter( void );

    void checkevent( void );

    void checkResources(const ResDB &rdb);

    static int countRAIDs( void );

private:
    int _raiddev;
    int mdnum;

    std::string state;
    std::string type;
    std::string working_map;
    std::string resync_state;
    int  disknum;
    unsigned long doneColor_, todoColor_, completeColor_;

    int find1(const std::string &key, const std::string &findwhat, int num1);
    int find2(const std::string &key, const std::string &findwhat, int num1,
      int num2);
    int raidparse(const std::string &cp);
    void getRAIDstate( void );
};


#endif
