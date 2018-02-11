//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "meter.h"
#include "log.h"

#include <cmath>



Meter::Meter(const std::string &title, const std::string &legend)
    : _x(0), _y(0), _width(1), _height(1),
      _priority(1), _counter(0),
      _docaptions(false), _dolegends(false),
      _dousedlegends(false), _metric(false),
      _title(_x, _y, title, Label::BLSW),
      _legend(_x, _y, legend, "/"),
      _fgColor(0), _bgColor(0) {
}


Meter::~Meter(void) = default;


void Meter::checkResources(const ResDB &rdb) {
    _priority = std::stoul(rdb.getResourceOrUseDefault(resName() + "Priority",
        "10"));
    _fgColor = rdb.getColor("foreground");
    _bgColor = rdb.getColor("background");
    auto tcolor = rdb.getColor("meterLabelColor");
    _title.color(tcolor);
    _legend.color(tcolor);
}


void Meter::resize(int x, int y, int width, int height) {
    _x = x;
    _y = y;
    _width = std::max(width, 0);
    _height = std::max(height, 0);
    _width &= ~1; // only allow even _width values
    _title.move(0, _y + _height + 2);
    _legend.move(_x, _y - 1);
}


void Meter::drawLabels(X11Graphics &g) {
    if (dolegends()) {
        _title.draw(g);

        if (docaptions())
            _legend.draw(g);
    }
}


void Meter::drawIfNeeded(X11Graphics &g) {
    if (dolegends()) {
        _title.drawIfNeeded(g);

        if (docaptions())
            _legend.drawIfNeeded(g);
    }
}


double Meter::scaleValue(double value, std::string &scale) const {
    double scaled = std::abs(value);
    scale = "";

    if (scaled >= 999.5 * 1e15) {
        scale = "E";
        scaled = value / (metric() ? 1e18 : 1ULL << 60);
    }
    else if (scaled >= 999.5 * 1e12) {
        scale = "P";
        scaled = value / (metric() ? 1e15 : 1ULL << 50);
    }
    else if (scaled >= 999.5 * 1e9) {
        scale = "T";
        scaled = value / (metric() ? 1e12 : 1ULL << 40);
    }
    else if (scaled >= 999.5 * 1e6) {
        scale = "G";
        scaled = value / (metric() ? 1e9 : 1UL << 30);
    }
    else if (scaled >= 999.5 * 1e3) {
        scale = "M";
        scaled = value / (metric() ? 1e6 : 1UL << 20);
    }
    else if (scaled >= 999.5) {
        scale = metric() ? "k" : "K";
        scaled = value / (metric() ? 1e3 : 1UL << 10);
    }
    else if (scaled < 0.9995 && metric()) {
        if (scaled >= 0.9995 / 1e3) {
            scale = "m";
            scaled = value * 1e3;
        }
        else if (scaled >= 0.9995 / 1e6) {
            scale = "\265";
            scaled = value * 1e6;
        }
        else {
            scale = "n";
            scaled = value * 1e9;
        }
        // add more if needed
    }
    else {
        scaled = value;
    }

    return scaled;
}


bool Meter::requestevent(void) {
    logAssert(_priority != 0) << "meter " << name() << " invalid priority\n";
    _counter = (_counter + 1) % _priority;
    return !_counter;
}
