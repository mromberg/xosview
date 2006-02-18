//
//  Copyright (c) 1994, 1995, 2004, 2006 by Mike Romberg ( mike.romberg@noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: cpumeter.h,v 1.7 2006/02/18 06:03:46 romberg Exp $
//
#ifndef _CPUMETER_H_
#define _CPUMETER_H_

#include "fieldmetergraph.h"

class CPUMeter : public FieldMeterGraph {
public:
  CPUMeter(XOSView *parent, const char *cpuID = "cpu");
  ~CPUMeter(void);

  const char *name(void) const { return "CPUMeter"; }
  void checkevent(void);

  void checkResources(void);

  static int countCPUs(void);
  static const char *cpuStr(int num);
protected:
  int _lineNum;
  long cputime_[2][7];
  int cpuindex_;

  void getcputime(void);
  int findLine(const char *cpuID);
  const char *toUpper(const char *str);
private:
};

#endif
