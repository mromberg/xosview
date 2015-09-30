//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#ifndef CFSMETER_H
#define CFSMETER_H

#include "fieldmetergraph.h"



class ComFSMeter : public FieldMeterGraph {
public:
    ComFSMeter(XOSView *parent, const std::string &path);
    virtual ~ComFSMeter(void);

    virtual std::string name( void ) const { return "FSMeter"; }
    virtual void checkevent( void );
    virtual void checkResources(const ResDB &rdb);

private:
    unsigned long _bgColor;
    unsigned long _umountColor;
    std::string _path;

    void setBGColor(unsigned long c);
    static bool isMount(const std::string &path);
};


class ComFSMeterFactory {
public:
    virtual std::vector<Meter *> make(const ResDB &rdb, XOSView *parent);
    virtual std::vector<std::string> mounts(const ResDB &rdb);
    virtual std::vector<std::string> getAuto(void);
};


#endif // end CFSMETER_H
