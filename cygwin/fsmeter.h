//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#ifndef FSMETER_H
#define FSMETER_H

#include "cfsmeter.h"



class FSMeterFactory : public ComFSMeterFactory {
public:
    virtual std::vector<std::string> getAuto(void);
};


#endif // end FSMETER_H
