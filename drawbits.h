#ifndef drawbits_h
#define drawbits_h

#include "x11graphics.h"
#include "log.h"

#include <vector>
#include <map>


template<class X>
class DrawBits {
public:
    DrawBits(void) : _bdColor(0) {}

    void color(const X &val, unsigned long color) { _cmap[val] = color; }
    void colorMap(const std::map<X, unsigned long> &cmap) { _cmap = cmap; }
    void borderColor(unsigned long c) { _bdColor = c; }

    void draw(const std::vector<X> &bits, X11Graphics &g,
      int x, int y, int width, int height,
      bool mandatory=false);

private:
    std::vector<X> _lastbits;
    std::map<X, unsigned long> _cmap;
    unsigned long _bdColor;

    void resize(size_t n);
    unsigned long getColor(const X &val) const;
};


template <class X>
unsigned long DrawBits<X>::getColor(const X &val) const {
    auto it = _cmap.find(val);

    if (it != _cmap.end())
        return it->second;

    logAssert(true) << "Missing color for value: " << val << std::endl;
    return 0;
}


template <class X>
void DrawBits<X>::draw(const std::vector<X> &bits, X11Graphics &g,
  int x, int y, int width, int height, bool mandatory) {

    if (_lastbits.size() != bits.size()) {
        resize(bits.size());
        mandatory = true;
    }

    // Borders.
    if (mandatory) {
        g.setFG(_bdColor);
        g.drawFilledRectangle(x - 1, y - 1, width + 2, height + 2);
    }

    int x1 = x, x2 = 0;

    for ( size_t i = 0 ; i < bits.size() ; i++ ) {
        if (i + 1 != bits.size())
            x2 = x + ((i + 1) * (width + 1)) / bits.size() - 1;
        else
            x2 = x + (width + 1) - 1;

        if ((bits[i] != _lastbits[i]) || mandatory) {
            g.setFG(getColor(bits[i]));
            g.drawFilledRectangle( x1, y, x2 - x1, height);
        }

        _lastbits[i] = bits[i];

        x1 = x2 + 2;
    }
}


template <class X>
void DrawBits<X>::resize(size_t n) {
    _lastbits.resize(n);
    std::fill(_lastbits.begin(), _lastbits.end(), X());
}

#endif
