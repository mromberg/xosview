//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef LABEL_H
#define LABEL_H

#include <string>
#include <vector>


class X11Graphics;


class Label {
public:
    enum Anchor { SW, SE };

    Label(int x, int y, Anchor anchor=SW);
    Label(int x, int y, const std::string &text, Anchor anchor=SW);

    void text(const std::string &str) { _next = str; }
    void color(unsigned long c) { _color = c; }
    void move(int x, int y) { _x = x;  _y = y; }

    void drawIfNeeded(X11Graphics &g) { if (_next != _current) draw(g); }
    void draw(X11Graphics &g);

protected:
    unsigned long _color;

    virtual void drawText(X11Graphics &g, int x, int y,
      const std::string &txt) const;

private:
    int _x;
    int _y;
    std::string _current;
    std::string _next;
    Anchor _anchor;

    void clearOld(X11Graphics &g) const;
};


class MCLabel : public Label {
public:
    MCLabel(int x, int y, Anchor anchor=SW);
    MCLabel(int x, int y, const std::string &text="",
      const std::string &delim=" ", Anchor anchor=SW);

    void delimiter(const std::string &delim) { _delim = delim; }
    void colors(const std::vector<unsigned long> &clst) { _colors = clst; }

protected:
    virtual void drawText(X11Graphics &g, int x, int y,
      const std::string &txt) const;

private:
    std::string _delim;
    std::vector<unsigned long> _colors;
};


#endif
