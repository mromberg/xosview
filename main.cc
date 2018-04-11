//
//  Copyright (c) 1994, 1995, 2006, 2007, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "xosview.h"
#include "log.h"


int main(int argc, char **argv) {

    std::set_terminate(util::ExcLog::terminate_cb);

    auto xosv = [&]() { XOSView().run(argc, argv); };

    return util::ExcLog(xosv).threw();
//    xosv();  return 0;  // use instead of above to dump core.
}
