//
//  Copyright (c) 1999, 2015, 2016, 2018
//  by Brian Grayson (bgrayson@netbsd.org)
//
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#ifndef intratemeter_h
#define intratemeter_h

#include "cintratemeter.h"
#include "intrstats.h"



class IrqRateMeter : public ComIrqRateMeter {
public:
    IrqRateMeter(void);

protected:
    virtual float getIrqRate(void) override;

private:
    std::map<size_t, uint64_t> _lastirqs;
    IntrStats _istats;
};


#endif
