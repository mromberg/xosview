//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#ifndef fsmeter_h
#define fsmeter_h

#include "cfsmeter.h"



class FSMeterFactory : public ComFSMeterFactory {
public:
    virtual std::vector<std::string> getAuto(void) override;
};


#endif // end FSMETER_H
