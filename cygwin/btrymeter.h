//
//  Copyright (c) 2018
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//
#ifndef btrymeter_h
#define btrymeter_h

#include "fieldmeter.h"



class BtryMeter : public FieldMeter {
public:
    BtryMeter(void);

    virtual std::string resName(void) const override { return "battery"; }
    virtual void checkevent(void) override;

    virtual void checkResources(const ResDB &rdb) override;

private:
    unsigned long _critColor, _lowColor, _leftColor, _chargeColor;
    unsigned long _fullColor, _noneColor;

    void setLegend(bool crit, bool low, bool acstatus, bool charging,
      int batlifetime);
};


#endif
