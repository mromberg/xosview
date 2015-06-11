//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef _MEMMETER_H_
#define _MEMMETER_H_

#include "fieldmetergraph.h"
#include <vector>
#include <string>


class MemMeter : public FieldMeterGraph {
public:
    MemMeter( XOSView *parent );
    ~MemMeter( void );

    std::string name( void ) const { return "MemMeter"; }
    void checkevent( void );

    void checkResources( void );
protected:

    void getmeminfo( void );
private:
    int _shAdj;

    class LineInfo {
    public:
        LineInfo(const char *id, float *val)
            { _line = -1; _id = id; _val = val; _idlen = strlen(_id); }
        LineInfo(void) {};

        int line(void) const { return _line; }
        void line(int l) { _line = l; }
        const char *id(void) const { return _id; }
        int idlen(void) const { return _idlen; }

        void setVal(double val) { *_val = val; }

    private:
        int _line;
        const char *_id;
        int _idlen;
        float *_val;
    };

    std::vector<LineInfo> _MIlineInfos;
    std::vector<LineInfo> _MSlineInfos;

    void initLineInfo(void);
    std::vector<LineInfo> findLines(const std::vector<LineInfo> &tmplate,
      const std::string &fname);
    void getmemstat(const std::string &fname, std::vector<LineInfo> &infos);
};

#endif
