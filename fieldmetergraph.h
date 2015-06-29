//
//  Original FieldMeter class is Copyright (c) 1994, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  Modifications from FieldMeter class done in Oct. 1995
//    by Brian Grayson ( bgrayson@netbsd.org )
//
//  Modifications from FieldMeterDecay class done in Oct. 1998
//    by Scott McNab ( jedi@tartarus.uwa.edu.au )
//

#ifndef _FIELDMETERGRAPH_H_
#define _FIELDMETERGRAPH_H_

#include "meter.h"
#include "fieldmeterdecay.h"
#include <vector>

class X11Pixmap;

class FieldMeterGraph : public FieldMeterDecay {
public:
    FieldMeterGraph( XOSView *parent, size_t numfields,
      const std::string &title = "", const std::string &legend = "",
      bool docaptions=false, bool dolegends=false, bool dousedlegends=false);
    virtual ~FieldMeterGraph( void );

    // virtual from Meter
    virtual void checkResources( void );

    // virtual from FieldMeter
    virtual void drawfields(X11Graphics &g, bool manditory=false);

protected:
    void setNumCols( int n );

    int useGraph_;
    int graphNumCols_;
    int graphpos_;
    int firstTimeDrawn_;
    /*  There's some sort of corruption going on -- we can't have
     *  variables after the heightfield_ below, otherwise they get
     *  corrupted???
     */
    std::vector<float> heightfield_;
private:
    X11Pixmap *_pmap;  // backbuffer

    void drawBars(X11Graphics &g, bool manditory);
    void drawBar(X11Graphics &g, int i);
    void checkBackBuffer(void);
};

#endif
