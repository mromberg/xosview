//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef meter_h
#define meter_h

#include "label.h"
#include "rdb.h"   // So every meter does not have to include this.



class Meter {
public:
    Meter(const std::string &title="", const std::string &legend="");

    virtual ~Meter(void);

    // prefix name for resouces such as cpu in cpuPriority
    virtual std::string resName(void) const = 0;
    virtual std::string name(void) const { return resName() + "Meter"; }

    virtual void checkResources(const ResDB &rdb);
    virtual void checkevent(void) = 0;
    virtual void draw(X11Graphics &g) = 0; // Draw everything cleared
    virtual void drawIfNeeded(X11Graphics &g); // Draw if needed
    virtual void resize(int x, int y, int width, int height);

    bool requestevent(void);  // if true wants to sample at this tick
    void docaptions(bool val) { _docaptions = val; }
    void dolegends(bool val) { _dolegends = val; }
    void dousedlegends(bool val) { _dousedlegends = val; }

protected:
    unsigned long fgColor(void) const { return _fgColor; }
    unsigned long bgColor(void) const { return _bgColor; }
    void title(const std::string &title) { _title.text(title); }
    void legend(const std::string &legend, const std::string &delimiter="/");
    bool docaptions(void) const { return _docaptions; }
    bool dolegends(void) const { return _dolegends; }
    bool dousedlegends(void) const { return _dousedlegends; }
    bool metric(void) const { return _metric; }
    void setMetric(bool val) { _metric = val; }
    double scaleValue(double value, std::string &scale) const;
    void setLegendColor(size_t index, unsigned long color)
        { _legend.setColor(index, color); }
    virtual void drawLabels(X11Graphics &g);

    int x(void) const { return _x; }
    int y(void) const { return _y; }
    void x(int v) { _x = v; }
    void y(int v) { _y = v; }
    int width(void) const { return _width; }
    int height(void) const { return _height; }

private:
    int _x, _y, _width, _height;
    size_t _priority, _counter;
    bool _docaptions, _dolegends, _dousedlegends, _metric;
    Label _title;
    MCLabel _legend;
    unsigned long _fgColor, _bgColor;
};



inline void Meter::legend(const std::string &legend,
  const std::string &delimiter) {
    _legend.text(legend);
    _legend.delimiter(delimiter);
}


#endif
