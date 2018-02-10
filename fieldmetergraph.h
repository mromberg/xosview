//
//  Original FieldMeter class is Copyright (c) 1994, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  Modifications from FieldMeter class done in Oct. 1995
//    by Brian Grayson ( bgrayson@netbsd.org )
//
//  Modifications from FieldMeterDecay class done in Oct. 1998
//    by Scott McNab ( jedi@tartarus.uwa.edu.au )
//

#ifndef fieldmetergraph_h
#define fieldmetergraph_h

#include "fieldmeterdecay.h"

#include <memory>

class X11Pixmap;


class FieldMeterGraph : public FieldMeterDecay {
public:
    FieldMeterGraph(size_t numfields, const std::string &title="",
      const std::string &legend="");
    virtual ~FieldMeterGraph(void);

    virtual void checkResources(const ResDB &rdb) override;

protected:
    virtual void drawfields(X11Graphics &g, bool mandatory=false) override;

private:
    bool _useGraph;
    size_t _graphNumCols, _graphPos;
    std::vector<float> _heightField;
    std::unique_ptr<X11Pixmap> _pmap;  // backbuffer

    void setNumCols(size_t n);
    void drawBars(X11Graphics &g);
    void drawBar(X11Graphics &g, int i) const;
    void checkBackBuffer(X11Graphics &g);
};


#endif
