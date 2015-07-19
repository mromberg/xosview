//
//  Copyright (c) 2008, 2015 by Tomi Tapper <tomi.o.tapper@jyu.fi>
//
//  File based on linux/lmstemp.* by
//  Copyright (c) 2000, 2006 by Leopold Toetsch <lt@toetsch.at>
//
//  This file may be distributed under terms of the GPL
//
//
//
#ifndef CORETEMP_H
#define CORETEMP_H


#include "fieldmeter.h"



class CoreTemp : public FieldMeter {
public:
    CoreTemp( XOSView *parent, const std::string &label,
      const std::string &caption, int cpu);
    ~CoreTemp( void );

    virtual std::string name( void ) const { return "CoreTemp"; }
    void checkevent( void );
    void checkResources( void );

    static unsigned int countCpus( void );

protected:
    void getcoretemp( void );

private:
    int   cpu_, cpucount_;
    float high_;
    std::vector<float> temps_;
    unsigned long actcolor_, highcolor_;
};

#endif
