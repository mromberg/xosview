//
//  Original FieldMeter class is Copyright (c) 1994, 2006, 2015
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

#ifndef FIELDMETERDECAY_H
#define FIELDMETERDECAY_H

#include "meter.h"
#include "fieldmeter.h"

#include <vector>



class FieldMeterDecay : public FieldMeter {
public:
    FieldMeterDecay(XOSView *parent, size_t numfields,
      const std::string &title = "", const std::string &legend = "",
      bool docaptions=false, bool dolegends=false,
      bool dousedlegends=false);
    virtual ~FieldMeterDecay( void );

    virtual void checkResources(const ResDB &rdb);

protected:
    virtual void drawfields(X11Graphics &g, bool mandatory=false);

private:
    bool dodecay_;
    bool firsttime_;  //Used to set up decaying fields right the first time.
    std::vector<float> decay_;
    std::vector<float> lastDecayval_;
};


#endif
