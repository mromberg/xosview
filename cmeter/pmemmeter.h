//
//  Copyright (c) 1994, 1995, 2006, 2015, 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef pmemmeter_h
#define pmemmeter_h

#include "fieldmetergraph.h"



class PrcMemMeter : public FieldMeterGraph {
public:
    PrcMemMeter(size_t numfields, const std::string &legend);

    virtual std::string resName(void) const override { return "mem"; }
    virtual void checkevent(void) override;

protected:
    class LineInfo {
    public:
        LineInfo(const std::string &id, float *val)
            : _line(-1), _id(id), _val(val) {}
        LineInfo(void) : _line(-1), _val(nullptr) {};

        size_t line(void) const { return _line; }
        void line(size_t l) { _line = l; }
        const std::string &id(void) const { return _id; }

        void setVal(float val) { *_val = val; }
        float val(void) const { return *_val; }

    private:
        size_t _line;
        std::string _id;
        float *_val;  // pointer into _fields array
    };

    // create a list of LineInfo objects linking to _fields.
    virtual std::vector<LineInfo> newLineInfos(void) = 0;

    // Called each "event" after fields tied to LineInfo objects are read.
    virtual void setFields(void) = 0;

private:
    std::vector<LineInfo> _lineInfos;

    std::vector<LineInfo> findLines(const std::vector<LineInfo> &tmplate,
      const std::string &fname) const;
    void getmemstat(const std::string &fname,
      std::vector<LineInfo> &infos) const;
};


#endif
