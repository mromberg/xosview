//
//  Original FieldMeter class is Copyright (c) 1994, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//  Modifications from FieldMeter class done in Oct. 1995
//    by Brian Grayson ( bgrayson@netbsd.org )
//
//  This file was written by Brian Grayson for the NetBSD and xosview
//    projects.
//  This file may be distributed under terms of the GPL or of the BSD
//    license, whichever you choose.  The full license notices are
//    contained in the files COPYING.GPL and COPYING.BSD, which you
//    should have received.  If not, contact one of the xosview
//    authors for a copy.
//

#ifndef fieldmeterdecay_h
#define fieldmeterdecay_h

#include "fieldmeter.h"



class FieldMeterDecay : public FieldMeter {
public:
    FieldMeterDecay(size_t numfields, const std::string &title="",
      const std::string &legend="");

    virtual void checkResources(const ResDB &rdb) override;

protected:
    virtual void drawfields(X11Graphics &g, bool mandatory=false) override;

private:
    bool _dodecay;
    bool _firsttime;  //Used to set up decaying fields right the first time.
    std::vector<float> _decay;
    std::vector<float> _lastDecayVal;

    void firstTimeInit(void);
};


#endif
