//  
//  Copyright (c) 1994, 1995 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: timeval.h,v 1.3 1996/11/19 02:13:41 bgrayson Exp $
//
#ifndef _TIMEVAL_H_
#define _TIMEVAL_H_

#define TIMEVAL_H_CVSID "$Id: timeval.h,v 1.3 1996/11/19 02:13:41 bgrayson Exp $"

#include <sys/time.h>
#include <iostream.h>

class TimeVal {
public:
  TimeVal(unsigned long sec = 0, unsigned long usec = 0) {
    _val.tv_sec = sec;
    _val.tv_usec = usec;
  }
  TimeVal(const struct timeval &val) { _val = val; }

  unsigned long sec(void) const { return _val.tv_sec; }
  unsigned long usec(void) const { return _val.tv_usec; }
  void sec(unsigned long s) { _val.tv_sec = s; }
  void usec(unsigned long us) { _val.tv_usec = us; }

  operator struct timeval(void) const { return _val; }

  ostream &printOn(ostream &os) const {
    return os <<"(" <<sec() <<" sec, " <<usec() <<" usec)";
  }

private:
  struct timeval _val;
};

inline ostream &operator<<(ostream &os, const TimeVal &tv){
  return tv.printOn(os);
}

#endif
