//
//  Copyright (c) 1997, 2005 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: btrymeter.h,v 1.3 2005/01/23 17:55:20 romberg Exp $
//
#ifndef _BTRYMETER_H_
#define _BTRYMETER_H_


#include "fieldmeter.h"
#include <string>


class BtryMeter : public FieldMeter {
public:
  BtryMeter( XOSView *parent );
  ~BtryMeter( void );

  const char *name( void ) const { return "BtryMeter"; }
  void checkevent( void );

  void checkResources( void );
protected:

  void getpwrinfo( void );
private:
  bool getapminfo( void );
  bool getacpiinfo( void );
  bool getacpiinfofield(const std::string& filename,
			const std::string& fieldname,
			float& value);

  int alarmThreshold;
};


#endif
