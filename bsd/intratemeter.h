//
//  Copyright (c) 1999, 2015, 2016
//  by Brian Grayson (bgrayson@netbsd.org)
//
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#ifndef IRQRATEMETER_H
#define IRQRATEMETER_H

#include "cintratemeter.h"
#include "intrstats.h"



class IrqRateMeter : public ComIrqRateMeter {
public:
    IrqRateMeter( void );

protected:
    virtual float getIrqRate(void);

private:
    size_t _irqcount;
    std::vector<uint64_t> _irqs, _lastirqs;
    IntrStats _istats;
};


#endif
