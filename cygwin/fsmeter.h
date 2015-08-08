//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#ifndef FSMETER_H
#define FSMETER_H

#include "fieldmetergraph.h"

class FSMeter : public FieldMeterGraph {
public:
    FSMeter(XOSView *parent, const std::string &path);
    virtual ~FSMeter(void);

    virtual std::string name( void ) const { return "FSMeter"; }
    virtual void checkevent( void );
    virtual void checkResources( void );

    static std::vector<std::string> mounts(XOSView *xosv);

private:
    unsigned long _bgColor;
    unsigned long _umountColor;
    std::string _path;

    void setBGColor(unsigned long c);
    static std::vector<std::string> getAuto(void);
    static bool isMount(const std::string &path);
};


#endif // end FSMETER_H
