//
//  Copyright (c) 2016
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef intrstats_h
#define intrstats_h

#include <vector>
#include <map>
#include <cstdint>


class IntrStats {
public:
    const std::vector<std::map<size_t, uint64_t> > &get(void);

private:
    std::vector<std::map<size_t, uint64_t> > read(void);
};

#endif
