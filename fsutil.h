//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

//
// Utilities for dealing with files, direcoties, paths
// The idea is to make these similar to the nice os and os.path
// modules found in python.
//
#ifndef FSUTIL_H
#define FSUTIL_H

#include <string>
#include <vector>
#include <fstream>

namespace util {

class FS {
public:
    // returns a list of names in the specified path (. and .. not included)
    static std::vector<std::string> listdir(const std::string &path);

    // returns true if path exists and is a directory
    static bool isdir(const std::string &path);

    // returns true if path exists and is a regular file
    static bool isfile(const std::string &path);

    // reads the entire file into str.  Returns false on failure.
    static bool readAll(const std::string &file, std::string &str);

    // reads the first X from file.  returns false on failure.
    template<class X>
    static bool readFirst(const std::string &file, X &x);
};

template<class X>
bool FS::readFirst(const std::string &file, X &x) {
    x = X();
    std::ifstream ifs(file.c_str());
    if (!ifs)
        return false;

    ifs >> x;

    if (!ifs) {
        x = X();
        return false;
    }

    return true;
}

} // end namespace util


#endif
