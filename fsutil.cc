//
//  Copyright (c) 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "fsutil.h"
#include "strutil.h"
#include "log.h"

#include <fstream>
#include <sstream>
#include <cerrno>

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

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

bool FS::isexec(const std::string &path) {
    struct stat sb;
    if (stat(path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode)) {
        // Just check if any of the execute bits are set 'cause
        // I don't wanna go down the whoami and who owns it rabbit hole
        if (sb.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
            return true;
    }

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

std::string FS::cwd(void) {
    std::string result(1024,'\0');
    while( getcwd(&result[0], result.size()) == 0) {
        if( errno != ERANGE ) {
            logBug << "getcwd() failed: " << strerror(errno) << std::endl;
            return ".";
        }
        result.resize(result.size()*2);
    }
    result.resize(result.find('\0'));
    return result;
}

std::string FS::normpath(const std::string &path) {
    std::vector<std::string> comps = util::split(path, "/");

    // remember if this is an absolute path for later
    // when we collapase ".."
    bool isabspath = false;
    if (comps.size() && comps[0] == "")
        isabspath = true;

    // First, remove all "" entries (which result from //)
    // and any "." entries.
    std::vector<std::string> filtered;
    for (size_t i = 0 ; i < comps.size() ; i++)
        if (comps[i] != "" && comps[i] != ".")
            filtered.push_back(comps[i]);

    // Now collapse the ".." items.
    comps.resize(0);
    for (size_t i = 0 ; i < filtered.size() ; i++) {
        if (filtered[i] != "..")
            comps.push_back(filtered[i]);
        else {
            // maybe collapse it
            if (comps.empty()) {
                // the root dir has a .. that points at /
                // If we don't know for sure we are at / (isabspath)
                // then keep the "..".  We don't use cwd() here.
                if (!isabspath)
                    comps.push_back(filtered[i]);

            }
            else {
                // There is something before the current ".." see if it
                // can be collapsed.
                if (comps.back() != "..") { // it got there for a reason
                    comps.pop_back();
                }
            }
        }
    }

    std::string rval = util::join(comps, "/");
    if (isabspath)
        rval = "/" + rval;

    return rval;
}

std::string FS::abspath(const std::string &path) {
    if (path.size()) {
        if (path[0] == '/')
            return path;

        size_t fpos = path.find('/');
        if (fpos != std::string::npos)
            return cwd() + '/' + path;
    }

    return path;
}

std::string FS::findCommand(const std::string &command) {
    // Atempt to guess the prefix from argv0, CWD and PATH

    // If command has a path component it is already
    // "found".
    if (command.find('/') != std::string::npos)
        return command;

    // Check on PATH
    if (command.size()) {
        char *p = getenv("PATH");
        if (p) {
            std::vector<std::string> path = util::split(p, ":");
            for (size_t i = 0 ; i < path.size() ; i++) {
                std::string fname(path[i] + '/' + command);
                if (FS::isexec(fname))
                    return fname;
            }
        }
    }

    return "";
}


} // end namespace util
