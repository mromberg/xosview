//
//  Copyright (c) 1994, 1995, 2006, 2007, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "xosview.h"

int main(int argc, char **argv) {

    try {

        XOSView xosview;
        xosview.run(argc, argv);

    } catch (...) {
        logBug << "Unhandled exception." << std::endl;
        return 1;
    }

    return 0;
}
