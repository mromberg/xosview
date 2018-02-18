//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#include "log.h"
#include "glob.h"

#ifdef XOSVDEBUG
#include "strutil.h"

#include <fstream>
#include <sstream>


namespace util {

static const char *LOGCONFIGFILE = "log.conf";

bool Log::_readConfig = false;  //  Have we read (or tried to) the config file?

// The supression list.  processed in order
// fileSpec(fglob or straight match) true = supress false = log
// lines in the config file which start with + are false
// and lines which start with - are true
std::vector<std::pair<std::string, bool>> Log::_slist;


void Log::readConfig(void) {
    _readConfig = true;

    std::ifstream ifs(LOGCONFIGFILE);
    if (ifs) {
        std::string line;
        while (!ifs.eof()) {
            std::getline(ifs, line);
            if (!ifs)
                break;

            // each *valid* line is +|- fspec
            std::istringstream iss(line);
            std::string flag, fspec;
            iss >> flag >> fspec;
            if (iss) {
                if (flag == "+")
                    _slist.push_back(std::make_pair(fspec, false));
                else if (flag == "-")
                    _slist.push_back(std::make_pair(fspec, true));
                // just ignore malformed lines
            }
        }
    }
}


bool Log::suppress(const std::string &file, size_t) {
    if (!_readConfig)
        readConfig();

    // process each entry in the file
    // changing the supression status of any matches
    // as we go.  Later matches override earlier ones
    bool suppress = false;
    for (const auto &s : _slist)
        if (util::fnmatch(s.first, file))
            suppress = s.second;

    return suppress;
}

} // end namespace util
#endif  // XOSVDEBUG
