//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "meter.h"
#include "log.h"

#include <cmath>
#include <array>
#include <tuple>
#include <algorithm>



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


double Meter::scaleValue(double value, unsigned char &scale) const {
    // defaults if value is not found in scales table.
    scale = ' ';
    double scaled = std::abs(value);

    //  * Unfortunately, we have to do our comparisons by 1000s (otherwise
    //    a value of 1020, which is smaller than 1K, could end up
    //    being printed as 1020, which is wider than what can fit)
    //    However, we do divide by 1024, so a K really is a K, and not
    //    1000.
    //  * In addition, we need to compare against 999.5*1000, because
    //    999.5, if not rounded up to 1.0 K, will be rounded by the
    //    %.0f to be 1000, which is too wide.  So anything at or above
    //    999.5 needs to be bumped up.

    //label, SI scale, IEC (1024 = 1k) scale.
    static const std::array<std::tuple<unsigned char, double, double>, 11>
        scales = {
        {
            {'E', 0.9995 * 1e18, 1ULL << 60},
            {'P', 0.9995 * 1e15, 1ULL << 50},
            {'T', 0.9995 * 1e12, 1ULL << 40},
            {'G', 0.9995 * 1e9, 1ULL << 30},
            {'M', 0.9995 * 1e6, 1ULL << 20},
            {'K', 0.9995 * 1e3, 1ULL << 10},
            {' ', 0.9995 * 1.0, 1.0},
            {'m', 0.9995 * 1e-3, 1e-3},
            {'u', 0.9995 * 1e-6, 1e-6},
            {'n', 0.9995 * 1e-9, 1e-9},
            {'p', 0.9995 * 1e-12, 1e-12}
            // add more if needed.
        }};

    // Look for value in the scales table and set the scale.
    auto sit = std::lower_bound(scales.begin(), scales.end(), value,
      [](const auto &a, const auto &b) { return b < std::get<1>(a); });
    if (sit != scales.end()) {
        scale = std::get<0>(*sit);
        scaled = value / (metric() ? std::get<1>(*sit) : std::get<2>(*sit));
    }

    return scaled;
}


bool Meter::requestevent(void) {
    logAssert(_priority != 0) << "meter " << name() << " invalid priority\n";
    _counter = (_counter + 1) % _priority;
    return !_counter;
}
