//
//  The original FieldMeter class is Copyright (c) 1994, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  Modifications from FieldMeter class done in Oct. 1995
//    by Brian Grayson ( bgrayson@netbsd.org )
//
//  Modifications from FieldMeterDecay class done in Oct. 1998
//    by Scott McNab ( jedi@tartarus.uwa.edu.au )
//

//---------------------------------------------------------------------------
// In order to use the FieldMeterGraph class in place of a FieldMeter class in
// a meter file (say, cpumeter.cc), make the following changes:
//   1.  Change cpumeter.h to include fieldmetergraph.h instead of
//       fieldmeter.h
//   2.  Change CPUMeter to inherit from FieldMeterGraph, rather than
//       FieldMeter.
//   3.  Change the constructor call to use FieldMeterGraph(), rather than
//       FieldMeter().
//   4.  Make the meter call FieldMeterGraph::checkResources(),
//       to pick up graphNumCols resource.
//   5.  Make the checkResources () function in the meter set the
//	 _useGraph variable according to the, e.g., xosview*cpuGraph
//       resource.
//---------------------------------------------------------------------------
#include "fieldmetergraph.h"
#include "x11pixmap.h"


static const size_t DEF_COLS = 100;


FieldMeterGraph::FieldMeterGraph(size_t numfields, const std::string &title,
  const std::string &legend)
    : FieldMeterDecay (numfields, title, legend),
      _useGraph(false),
      _graphNumCols(DEF_COLS), _graphPos(DEF_COLS - 1) {
}


FieldMeterGraph::~FieldMeterGraph(void) = default;


void FieldMeterGraph::drawfields(X11Graphics &g, bool mandatory) {

    if (!_useGraph) {
        // Call FieldMeterDecay code if this meter should not be
        // drawn as a graph
        FieldMeterDecay::drawfields(g, mandatory);
        return;
    }

    checkBackBuffer(g);
    // Repositon x_ and y_ relative to the pixmap
    const int oldx = x(), oldy = y();
    x(0);
    y(0);

    // Draw the graph in the pixmap
    drawBars(_pmap->g());

    // put x_ and y_ back to window coords
    x(oldx);
    y(oldy);

    // and finally copy the pixmap into the window
    _pmap->copyTo(g, 0, 0, width(), height(), x() + 1, y() + 1);
}


void FieldMeterGraph::checkBackBuffer(X11Graphics &g) {
    // Create a new Pixmap to match our area if needed
    if (!_pmap || _pmap->width() != width() || _pmap->height() != height()) {
        // the x_ and x_ if a FieldMeter are relative to
        // the parent window.  The width_ and height_ are for
        // the actual meter graphic display (where x_ and y_ are upper left
        _pmap = g.newX11Pixmap(width(), height());
        logDebug << "new X11Pixmap: " << width() << ", " << height()
                 << std::endl;

        // Fill is with the last "idle" color
        _pmap->g().setBG(fieldcolor(numfields() - 1));
        _pmap->g().setFG(fieldcolor(numfields() - 1));
        _pmap->g().drawFilledRectangle(0, 0, _pmap->width(), _pmap->height());
    }
}


void FieldMeterGraph::drawBars(X11Graphics &g) {

    if (_total <= 0.0)
        return;

    // allocate memory for height field graph storage
    // note: this is done here as it is not certain that both
    // numfields() and _graphNumCols are defined in the constructor
    if (_heightField.empty()) {
        _heightField.resize(numfields() * _graphNumCols);

        for(size_t i = 0 ; i < _graphNumCols ; i++) {
            for(size_t j = 0 ; j < numfields() ; j++) {
                if (j + 1 < numfields())
                    _heightField[i * numfields() + j] = 0.0;
                else
                    _heightField[i * numfields() + j] = 1.0;
            }
        }
    }

    // check current position here and slide graph if necessary
    if (_graphPos >= _graphNumCols) {
        for(size_t i = 0 ; i + 1 < _graphNumCols ; i++) {
            for(size_t j = 0 ; j < numfields() ; j++) {
                _heightField[i * numfields() + j] =
                    _heightField[(i + 1) * numfields() + j];
            }
        }
        _graphPos = _graphNumCols - 1;
    }

    // get current values to be plotted
    for(size_t i = 0 ; i < numfields() ; i++) {
        float a = _fields[i] / _total;
        a = std::max(a, 0.0f);
        a = std::min(a, 1.0f);
        _heightField[_graphPos * numfields() + i] = a;
    }

    // scroll area then draw new bar.
    const int col_width = std::max(
        width() / static_cast<int>(_graphNumCols), 1);
    const int sx = x() + col_width;
    const int swidth = width() - col_width;
    const int sheight = height();

    if (swidth > 0 && sheight > 0)
        g.copyArea(sx, y(), swidth, sheight, x(), y());

    drawBar(g, _graphNumCols - 1);

    _graphPos++;
    g.setStippleN(0);	//  Restore all-bits stipple.
}


void FieldMeterGraph::drawBar(X11Graphics &g, int i) const {
    int y = Meter::y() + height();
    const int x = Meter::x() + i * width() / _graphNumCols;
    const int barwidth = (Meter::x() + (i + 1) * width() / _graphNumCols) - x;

    if (barwidth > 0) {
        for (size_t j = 0 ; j < numfields() ; j++) {
            //  Round up, by adding 0.5 before converting to an int.
            int barheight = static_cast<int>((_heightField[i * numfields() + j]
                * height()) + 0.5);

            g.setFG(fieldcolor(j));
            g.setStippleN(j % 4);

            if (barheight > (y - Meter::y()))
                barheight = (y - Meter::y());

            // hack to ensure last field always reaches top of graph area
            if (j + 1 == numfields())
                barheight = (y - Meter::y());

            y -= barheight;
            if (barheight > 0)
                g.drawFilledRectangle(x, y, barwidth, barheight);
        }
    }
}


void FieldMeterGraph::checkResources(const ResDB &rdb) {
    FieldMeterDecay::checkResources(rdb);

    _useGraph = rdb.isResourceTrue(resName() + "Graph");

    // exit(1) if does not exist
    size_t n = std::stoul(rdb.getResource("graphNumCols"));
    if(n)
        setNumCols(n);
}


void FieldMeterGraph::setNumCols(size_t n) {
    _graphNumCols = n;
    _graphPos = _graphNumCols - 1;
    _heightField.resize(0);
}
