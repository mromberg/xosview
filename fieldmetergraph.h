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

#ifndef FIELDMETERGRAPH_H
#define FIELDMETERGRAPH_H

#include "meter.h"
#include "fieldmeterdecay.h"

#include <vector>

class X11Pixmap;


class FieldMeterGraph : public FieldMeterDecay {
public:
    FieldMeterGraph( XOSView *parent, size_t numfields,
      const std::string &title = "", const std::string &legend = "",
      bool docaptions=false, bool dolegends=false,
      bool dousedlegends=false);
    virtual ~FieldMeterGraph( void );

    // virtual from Meter
    virtual void checkResources(const ResDB &rdb);

protected:
    bool useGraph_;

    virtual void drawfields(X11Graphics &g, bool mandatory=false);

private:
    int graphNumCols_;
    int graphpos_;
    std::vector<float> heightfield_;
    X11Pixmap *_pmap;  // backbuffer

    void setNumCols( int n );
    void drawBars(X11Graphics &g);
    void drawBar(X11Graphics &g, int i) const;
    void checkBackBuffer(void);
};


#endif
