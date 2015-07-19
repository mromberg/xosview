//
//  Copyright (c) 1999, 2015 by Brian Grayson (bgrayson@netbsd.org)
//
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#ifndef IRQRATEMETER_H
#define IRQRATEMETER_H

#include "fieldmetergraph.h"


class IrqRateMeter : public FieldMeterGraph {
public:
    IrqRateMeter( XOSView *parent );
    ~IrqRateMeter( void );

    virtual std::string name( void ) const { return "IrqRateMeter"; }
    void checkevent( void );
    void checkResources( void );

protected:
    void getinfo( void );

private:
    unsigned int irqcount_;
    std::vector<uint64_t> irqs_, lastirqs_;
};

#endif
