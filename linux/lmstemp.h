//
//  Copyright (c) 2000, 2006, 2015, 2018
//  by Leopold Toetsch <lt@toetsch.at>
//
//  File based on btrymeter.* by
//  Copyright (c) 1997 by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef lmstemp_h
#define lmstemp_h


#include "fieldmeter.h"



class LmsTemp : public FieldMeter {
public:
    LmsTemp(const std::string &filename, const std::string &label,
      const std::string &caption);

    virtual std::string resName(void) const override { return "lmstemp"; }
    virtual void checkevent(void) override;
    virtual void checkResources(const ResDB &rdb) override;

private:
    std::string _filename;
    int _highest;
    bool _isproc;
    unsigned long _highColor, _actColor;

    void getlmstemp(void);
    bool checksensors(bool isproc, const std::string &dir,
      const std::string &filename);
};


#endif
