//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "ploadmeter.h"

#include <fstream>


static const char * const LOADFILENAME = "/proc/loadavg";
static const char * const SPEEDFILENAME = "/proc/cpuinfo";



float PrcLoadMeter::getLoad(void) {
    std::ifstream loadinfo( LOADFILENAME );

    if ( !loadinfo )
        logFatal << "Can not open file : " << LOADFILENAME << std::endl;

    float rval;
    loadinfo >> rval;

    if (!loadinfo)
        logFatal << "could not parse: " << LOADFILENAME << std::endl;

    return rval;
}

uint64_t PrcLoadMeter::getCPUSpeed(void) {
    std::string filename;
    std::string inp_line;

    std::string argname;
    std::string argval;

    std::ifstream speedinfo(SPEEDFILENAME);
    while ( speedinfo.good() ) {
        argname.clear();
        std::getline(speedinfo,argname,':');
        argval.clear();
        std::getline(speedinfo,argval);
        // XOSDEBUG("speed: a=\"%s\" v=\"%s\"\n",
        //     argname.c_str(),argval.c_str() );

        if ( argname.substr(0,7) == "cpu MHz" ) {
            //XOSDEBUG("SPEED: %s\n",argval.c_str() );
            return util::stof(argval) * 1000000;
        }
    }

    return 0;
}
