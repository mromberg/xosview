//
//  Copyright (c) 2015, 2016, 2017, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "label.h"
#include "x11graphics.h"
#include "log.h"
#include "strutil.h"



Label::Label(int x, int y, Anchor anchor)
    : _color(0), _x(x), _y(y), _anchor(anchor) {
}


Label::Label(int x, int y, const std::string &text, Anchor anchor)
    : _color(0), _x(x), _y(y), _next(text), _anchor(anchor) {
}


void Label::clearOld(X11Graphics &g) const {
    // Default SW anchor
    int x = _x;
    int y = _y - g.textHeight();
    const int width = g.textWidth(_current);
    const int height = g.textHeight();

    switch (_anchor) {
    case BLSE:
        y += g.textDescent();
        FALLTHROUGH;
    case SE:
        x -= width;
        break;
    case BLSW:
        y += g.textDescent();
        FALLTHROUGH;
    case SW:
        break;
    default:
        logBug << "Unknown anchor: " << _anchor << std::endl;
    };

    g.clear(std::max(x - 1, 0), y, width, std::max(height - 1, 1));
}


void Label::drawText(X11Graphics &g, int x, int y,
  const std::string &txt) const {

    g.setFG(_color);
    g.drawString(x, y, txt);
}


void Label::draw(X11Graphics &g) {
    clearOld(g);
    _current = _next;

    int x = _x;
    int y = _y - g.textDescent();
    switch(_anchor) {
    case BLSE:
        y += g.textDescent();
        FALLTHROUGH;
    case SE:
        x -= g.textWidth(_current);
        break;
    case BLSW:
        y += g.textDescent();
        FALLTHROUGH;
    case SW:
        break;
    default:
        logBug << "Unknown anchor: " << _anchor << std::endl;
    };

    drawText(g, x, y, _current);
}


MCLabel::MCLabel(int x, int y, Anchor anchor)
    : Label(x, y, anchor), _delim(" ") {
}


MCLabel::MCLabel(int x, int y, const std::string &text,
  const std::string &delim, Anchor anchor)
    : Label(x, y, text, anchor), _delim(delim) {
}


void MCLabel::drawText(X11Graphics &g, int x, int y,
  const std::string &txt) const {

    auto labelv = util::split(txt, _delim);

    for (size_t i = 0 ; i < labelv.size() ; i++) {
        if (i != 0) {
            g.setFG(_color);
            g.drawString(x, y, _delim);
            x += g.textWidth(_delim);
        }
        if (i < _colors.size())
            g.setFG(_colors[i]);
        else
            g.setFG(_color);
        g.drawString(x, y, labelv[i]);
        x += g.textWidth(labelv[i]);
    }
}


void MCLabel::setColor(size_t index, unsigned long pixVal) {
    if (_colors.size() <= index)
        _colors.resize(index + 1, _color);
    _colors[index] = pixVal;
}
