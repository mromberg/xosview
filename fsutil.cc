//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "fsutil.h"
#include "strutil.h"
#include "log.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <fstream>
#include <sstream>

namespace util {

std::vector<std::string> FS::listdir(const std::string &path) {
    std::vector<std::string> rval;

    DIR *d = opendir(path.c_str());
    if (!d) {
        logBug << "opendir(" << path << ") failed: "
               << util::strerror(errno) << std::endl;
    }
    else {
        struct dirent *de = 0;
        errno = 0; // EOF readdir() does not change and returns NULL too
        while ((de = readdir(d)) != NULL) {
            std::string name(de->d_name);
            if (name != "." && name != "..")
                rval.push_back(name);
            errno = 0;
        }
        if (errno != 0) {
            logBug << "readdir(" << path << ") failed: "
                   << util::strerror(errno) << std::endl;
        }
        closedir(d);
    }

    return rval;
}

bool FS::isdir(const std::string &path) {
    struct stat sb;
    if (stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
        return true;

    return false;
}

bool FS::isfile(const std::string &path) {
    struct stat sb;
    if (stat(path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode))
        return true;

    return false;
}

bool FS::readAll(const std::string &file, std::string &str) {
    str = "";
    std::ifstream ifs(file.c_str());
    if (!ifs)
        return false;

    std::stringstream iss;
    iss << ifs.rdbuf();

    if (!ifs || !iss)
        return false;

    str = iss.str();
    return true;
}


} // end namespace util
