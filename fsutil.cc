//
//  Copyright (c) 2015, 2016, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "fsutil.h"
#include "strutil.h"
#include "log.h"

#include <fstream>

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>


namespace util {

std::vector<std::string> fs::listdir(const std::string &path) {
    std::vector<std::string> rval;

    DIR *d = opendir(path.c_str());
    if (!d) {
        logBug << "opendir(" << path << ") failed: "
               << util::strerror(errno) << std::endl;
    }
    else {
        struct dirent *de = nullptr;
        errno = 0; // EOF readdir() does not change and returns NULL too
        while ((de = readdir(d)) != nullptr) {
            const std::string name(de->d_name);
            if (name != "." && name != "..")
                rval.push_back(name);
            errno = 0;
        }
        if (errno != 0) {
            logBug << "readdir(" << path << ") failed: "
                   << util::strerror() << std::endl;
        }
        if (closedir(d) != 0)
            logBug << "closedir(" << path << ") failed: "
                   << util::strerror() << std::endl;
    }

    return rval;
}


bool fs::isdir(const std::string &path) {
    struct stat sb;
    if (stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
        return true;

    return false;
}


bool fs::isfile(const std::string &path) {
    struct stat sb;
    if (stat(path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode))
        return true;

    return false;
}


bool fs::isexec(const std::string &path) {
    struct stat sb;
    if (stat(path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode)) {
        // Just check if any of the execute bits are set 'cause
        // I don't wanna go down the whoami and who owns it rabbit hole
        if (sb.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
            return true;
    }

    return false;
}


bool fs::readAll(const std::string &file, std::string &str) {
    str.clear();
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


std::string fs::readAll(const std::string &file, bool fatal) {
    std::string rval;

    if (!readAll(file, rval)) {
        if (fatal) {
            logFatal << "failed to read: " << file << std::endl;
        }
        else {
            logBug << "failed to read: " << file << std::endl;
        }
    }

    return rval;
}


std::string fs::cwd(void) {
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


std::string fs::normpath(const std::string &path) {
    auto comps = util::split(path, "/");

    // remember if this is an absolute path for later
    // when we collapase ".."
    const bool isabspath = (!comps.empty() && comps[0].empty()) ? true : false;

    // First, remove all "" entries (which result from //)
    // and any "." entries.
    std::vector<std::string> filtered;
    std::copy_if(comps.cbegin(), comps.cend(), std::back_inserter(filtered),
      [](const auto &comp) { return !comp.empty() && comp != "."; });

    // Now collapse the ".." items.
    comps.clear();
    for (const auto &f : filtered) {
        if (f != "..")
            comps.push_back(f);
        else {
            // maybe collapse it
            if (comps.empty()) {
                // the root dir has a .. that points at /
                // If we don't know for sure we are at / (isabspath)
                // then keep the "..".  We don't use cwd() here.
                if (!isabspath)
                    comps.push_back(f);

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


std::string fs::abspath(const std::string &path) {
    if (path.size()) {
        if (path[0] == '/')
            return path;

        size_t fpos = path.find('/');
        if (fpos != std::string::npos)
            return cwd() + '/' + path;
    }

    return path;
}


std::string fs::findCommand(const std::string &command, bool log) {
    // Atempt to guess the prefix from argv0, CWD and PATH

    // If command has a path component it is already
    // "found".
    if (command.find('/') != std::string::npos)
        return canonpath(command);

    // Check on PATH
    if (!command.empty()) {
        const char *penv = getenv("PATH");
        if (penv) {
            for (const auto &p : util::split(penv, ":")) {
                const std::string fname(p + '/' + command);
                if (fs::isexec(fname))
                    return canonpath(fname);
            }
        }
    }

    if (log)
        logProblem << command << " not found." << std::endl;

    return command;
}


size_t fs::fnameMax(const std::string &path) {
    struct statvfs stat;

    if (statvfs(path.c_str(), &stat) != 0) {
        logProblem << "statvfs(" << path << ") failed." << std::endl;
        const long rval = pathconf(path.c_str(),_PC_PATH_MAX);
        if (rval == -1) {
            logProblem << "pathconf() failed too.  returning SWAG...\n";
            return 4096;
        }
        return rval;

    }

    return stat.f_namemax;
}


std::pair<uint64_t, uint64_t> fs::getSpace(
    const std::string &path, bool privileged) {

    struct statvfs stat;

    if (statvfs(path.c_str(), &stat) != 0) {
        logProblem << "statvfs(" << path << ") failed." << std::endl;
        return std::make_pair(0, 0);
    }

    // copy them into big vars before math for 32 bit os.
    uint64_t total = stat.f_blocks;
    uint64_t avail = stat.f_bavail;
    uint64_t free = stat.f_bfree;
    uint64_t frsize = stat.f_frsize;

    if (!privileged)
        return std::make_pair(avail * frsize, total * frsize);

    return std::make_pair(free * frsize, total * frsize);
}


} // end namespace util
