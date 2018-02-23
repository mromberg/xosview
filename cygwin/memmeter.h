//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef memmeter_h
#define memmeter_h

#include "fieldmetergraph.h"


class MemMeter : public FieldMeterGraph {
public:
    MemMeter(void);

    virtual std::string resName(void) const override { return "mem"; }
    virtual void checkevent(void) override;

    virtual void checkResources(const ResDB &rdb) override;

private:
    class LineInfo {
    public:
        LineInfo(const std::string &id, float *val)
            : _line(-1), _id(id), _val(val) {}
        LineInfo(void) {};

        size_t line(void) const { return _line; }
        void line(size_t l) { _line = l; }
        const std::string &id(void) const { return _id; }
        size_t idlen(void) const { return _id.size(); }

        void setVal(double val) { *_val = val; }

    private:
        size_t _line;
        std::string _id;
        float *_val;  // pointer into _fields array
    };

    std::vector<LineInfo> _MIlineInfos;
    std::vector<LineInfo> _MSlineInfos;

    void getmeminfo(void);
    void initLineInfo(void);
    std::vector<LineInfo> findLines(const std::vector<LineInfo> &tmplate,
      const std::string &fname) const;
    void getmemstat(const std::string &fname,
      std::vector<LineInfo> &infos) const;
};

#endif
