//
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: nfsmeter.h,v 1.2 2007/09/12 22:11:08 romberg Exp $
//
#ifndef _NFSMETER_H_
#define _NFSMETER_H_

#include "fieldmetergraph.h"
#include "timer.h"

class Host;

class NFSMeter : public FieldMeterGraph {
public:
	NFSMeter(
		XOSView *parent,
		const char *name,
		int   nfields,
		const char *files,
		const char *statfile);
  ~NFSMeter( void );

  const char *name( void ) const { return _statname; }
  void checkResources( void );
  void starttimer(void) { return _timer.start(); };
  void stoptimer(void) { return _timer.stop(); };
  double usecs(void) { return _timer.report_usecs(); };

protected:
  const char *_statname;
  const char *_statfile;

private:
  Timer _timer;

};

class NFSStats : public NFSMeter {
public:
  NFSStats(XOSView *parent);
  ~NFSStats(void);

  void checkevent( void );

  void checkResources( void );
private:
	unsigned long _lastcalls, _lastretrns, _lastauthrefresh;
};

class NFSDStats : public NFSMeter {
public:
  NFSDStats(XOSView *parent);
  ~NFSDStats(void);

  void checkevent( void );

  void checkResources( void );

protected:
  float maxpackets_;

private:
  unsigned long _lastTcp, _lastUdp, _lastNetCnt, _lastBad;
};

#endif
