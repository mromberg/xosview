//
//  Copyright (c) 2000, 2006, 2015
//  by Leopold Toetsch <lt@toetsch.at>
//
//  File based on btrymeter.* by
//  Copyright (c) 1997 by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef LMSTEMP_H
#define LMSTEMP_H


#include "fieldmeter.h"


class LmsTemp : public FieldMeter {
public:
    LmsTemp( XOSView *parent, const std::string &filename,
      const std::string &label, const std::string &caption);

    ~LmsTemp( void );

    std::string name( void ) const { return "LmsTemp"; }
    void checkevent( void );

    void checkResources(const ResDB &rdb);

protected:
    void getlmstemp( void );
    int  checksensors(int isproc, const std::string &dir,
      const std::string &filename);
private:
    std::string _filename;
    int _highest;
    int _isproc;
    unsigned long _highColor, _actColor;
};


#endif
