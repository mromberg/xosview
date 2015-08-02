//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//

//-------------------------------------------------------
// BASIC Data Meter Template.  See example.h for comments
//-------------------------------------------------------

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
    std::string _path;
};


#endif // end FSMETER_H
