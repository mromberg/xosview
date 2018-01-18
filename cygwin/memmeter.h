//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef MEMMETER_H
#define MEMMETER_H

#include "fieldmetergraph.h"


class MemMeter : public FieldMeterGraph {
public:
    MemMeter( void );
    ~MemMeter( void );

    std::string resName( void ) const { return "mem"; }
    void checkevent( void );

    void checkResources(const ResDB &rdb);

private:
    class LineInfo {
    public:
        LineInfo(const std::string &id, float *val)
            : _line(-1), _id(id), _val(val) {}
        LineInfo(void) {};

        int line(void) const { return _line; }
        void line(int l) { _line = l; }
        const std::string &id(void) const { return _id; }
        size_t idlen(void) const { return _id.size(); }

        void setVal(double val) { *_val = val; }

    private:
        int _line;
        std::string _id;
        float *_val;  // pointer into _fields array
    };

    std::vector<LineInfo> _MIlineInfos;
    std::vector<LineInfo> _MSlineInfos;

    void getmeminfo( void );
    void initLineInfo(void);
    std::vector<LineInfo> findLines(const std::vector<LineInfo> &tmplate,
      const std::string &fname);
    void getmemstat(const std::string &fname, std::vector<LineInfo> &infos);
};

#endif
