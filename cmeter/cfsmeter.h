//
//  Copyright (c) 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#ifndef CFSMETER_H
#define CFSMETER_H

#include "fieldmetergraph.h"

#include <fstream>


#define MOUNT_FNAME "/proc/mounts"


class ComFSMeter : public FieldMeterGraph {
public:
    ComFSMeter(const std::string &path);
    virtual ~ComFSMeter(void);

    virtual std::string name( void ) const { return "FSMeter"; }
    virtual void checkevent( void );

    virtual std::string resName(void) const { return "filesys"; }
    virtual void checkResources(const ResDB &rdb);

protected:
    virtual bool isMount(const std::string &path);

private:
    unsigned long _bgColor;
    unsigned long _umountColor;
    std::string _path;

    void setBGColor(unsigned long c);
};


template <class X>
class FSMFactory {
public:
    virtual std::vector<std::unique_ptr<Meter>> make(const ResDB &rdb);
    virtual std::vector<std::string> mounts(const ResDB &rdb);
    virtual std::vector<std::string> getAuto(void);
};


using ComFSMeterFactory = FSMFactory<ComFSMeter>;


template <class X>
std::vector<std::unique_ptr<Meter>> FSMFactory<X>::make(const ResDB &rdb) {

    std::vector<std::unique_ptr<Meter>> rval;

    for (const auto &fs : mounts(rdb))
        rval.push_back(std::make_unique<X>(fs));

    return rval;
}


template <class X>
std::vector<std::string> FSMFactory<X>::mounts(const ResDB &rdb) {
    std::string mounts = rdb.getResource("filesysMounts");

    logDebug << "MOUNTS: " << mounts << std::endl;

    if (mounts == "auto")
        return getAuto();

    // Then filesysMounts is a list of directories
    std::vector<std::string> rval;
    rval = util::split(mounts, " ");

    logDebug << "MOUNTS: " << rval << std::endl;

    return rval;
}


template <class X>
std::vector<std::string> FSMFactory<X>::getAuto(void) {
    //  Create a list of entries in mounts where the device
    //  and mount point are absolute paths.

    std::vector<std::string> rval;

    std::ifstream ifs(MOUNT_FNAME);
    if (!ifs) {
        logProblem << "Could not open: " << MOUNT_FNAME << std::endl;
        return rval;
    }

    while (!ifs.eof()) {
        std::string dev, path, type, line;
        ifs >> dev >> path >> type;
        std::getline(ifs, line);
        if (ifs) {
            if (dev[0] == '/' && path[0] == '/')
                rval.push_back(path);
        }
    }

    return rval;
}


#endif // end CFSMETER_H
