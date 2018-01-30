//
//  Copyright (c) 2012, 2015, 2018 by Tomi Tapper <tomi.o.tapper@jyu.fi>
//
//  File based on linux/lmstemp.* by
//  Copyright (c) 2000, 2006 by Leopold Toetsch <lt@toetsch.at>
//
//  This file may be distributed under terms of the GPL
//
//
//
#ifndef sensor_h
#define sensor_h

#include "sensorfieldmeter.h"



class BSDSensor : public SensorFieldMeter {
public:
    BSDSensor( const std::string &name, const std::string &high,
      const std::string &low, const std::string &label,
      const std::string &caption, int nbr );
    ~BSDSensor(void);

    virtual std::string resName(void) const override { return "bsdsensor"; }
    virtual void checkevent(void) override;
    virtual void checkResources(const ResDB &rdb) override;

private:
    std::string _name, _val, _highName, _lowName, _highVal, _lowVal;
    int _nbr;

    void getsensor(void);
};


#endif
