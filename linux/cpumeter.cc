//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015 by Mike Romberg ( mike.romberg@noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: cpumeter.cc,v 1.18 2008/02/28 23:37:24 romberg Exp $
//
#include "cpumeter.h"
#include "xosview.h"
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sstream>
#include <ctype.h>

static const char STATFILENAME[] = "/proc/stat";
#define MAX_PROCSTAT_LENGTH 4096

CPUMeter::CPUMeter(XOSView *parent, const char *cpuID)
: FieldMeterGraph( parent, 8, toUpper(cpuID), "USR/NICE/SYS/SI/HI/WIO/FREE/ST" ) {
  _lineNum = findLine(cpuID);
  for ( int i = 0 ; i < 2 ; i++ )
    for ( int j = 0 ; j < 8 ; j++ )
      cputime_[i][j] = 0;
  cpuindex_ = 0;

}

CPUMeter::~CPUMeter( void ){
}

void CPUMeter::checkResources( void ){
  FieldMeterGraph::checkResources();

  setfieldcolor( 0, parent_->getResource( "cpuUserColor" ) );
  setfieldcolor( 1, parent_->getResource( "cpuNiceColor" ) );
  setfieldcolor( 2, parent_->getResource( "cpuSystemColor" ) );
  setfieldcolor( 3, parent_->getResource( "cpuSInterruptColor" ) );
  setfieldcolor( 4, parent_->getResource( "cpuInterruptColor" ) );
  setfieldcolor( 5, parent_->getResource( "cpuWaitColor" ) );
  setfieldcolor( 6, parent_->getResource( "cpuFreeColor" ) );
  setfieldcolor( 7, parent_->getResource( "cpuStolenColor" ) );
  priority_ = atoi (parent_->getResource( "cpuPriority" ) );
  dodecay_ = parent_->isResourceTrue( "cpuDecay" );
  useGraph_ = parent_->isResourceTrue( "cpuGraph" );
  setUsedFormat (parent_->getResource("cpuUsedFormat"));
}

void CPUMeter::checkevent( void ){
  getcputime();
  drawfields();
}

void CPUMeter::getcputime( void ){
  total_ = 0;
  std::string tmp;
  std::ifstream stats( STATFILENAME );

  if ( !stats ){
    std::cerr <<"Can not open file : " <<STATFILENAME << std::endl;
    exit( 1 );
  }

  // read until we are at the right line.
  for (int i = 0 ; i < _lineNum ; i++) {
    if (stats.eof())
      break;
    getline(stats, tmp);
  }

  stats >>tmp >>cputime_[cpuindex_][0]
	      >>cputime_[cpuindex_][1]
	      >>cputime_[cpuindex_][2]
	      >>cputime_[cpuindex_][3]
	      >>cputime_[cpuindex_][4]
	      >>cputime_[cpuindex_][5]
	      >>cputime_[cpuindex_][6]
	      >>cputime_[cpuindex_][7];

  int oldindex = (cpuindex_+1)%2;
  for ( int i = 0 ; i < 8 ; i++ ){
    static int cputime_to_field[8] = { 0, 1, 2, 6, 5, 4, 3, 7 };
    int field = cputime_to_field[i];
    fields_[field] = cputime_[cpuindex_][i] - cputime_[oldindex][i];
    total_ += fields_[field];
  }

  if (total_){
    setUsed (total_ - (fields_[5] + fields_[6] + fields_[7]), total_);
    cpuindex_ = (cpuindex_ + 1) % 2;
  }
}

int CPUMeter::findLine(const char *cpuID){
  std::ifstream stats( STATFILENAME );

  if ( !stats ){
    std::cerr <<"Can not open file : " <<STATFILENAME << std::endl;
    exit( 1 );
  }

  int line = -1;
  std::string buf;
  while (!stats.eof()){
    getline(stats, buf);
    if (!stats.eof()){
      line++;
      if (!strncmp(cpuID, buf.data(), strlen(cpuID))
        && buf[strlen(cpuID)] == ' ')
        return line;
    }
  }
  return -1;
}

// Checks for the SMP kernel patch by forissier@isia.cma.fr.
// http://www-isia.cma.fr/~forissie/smp_kernel_patch/
// If it finds that this patch has been applied to the current kernel
// then returns the number of cpus that are on this machine.
int CPUMeter::countCPUs(void){
  std::ifstream stats( STATFILENAME );

  if ( !stats ){
    std::cerr <<"Can not open file : " <<STATFILENAME << std::endl;
    exit( 1 );
  }

  int cpuCount = 0;
  std::string buf;
  while (getline(stats, buf))
      if (!strncmp(buf.data(), "cpu", 3) && buf[3] != ' ')
          cpuCount++;

  return cpuCount;
}

const char *CPUMeter::cpuStr(int num){
  static char buffer[32];
  std::ostringstream str;
  std::ifstream stats( STATFILENAME );

  if ( !stats ){
    std::cerr <<"Can not open file : " <<STATFILENAME << std::endl;
    exit( 1 );
  }

  int cpuCount = 0;
  std::string buf;
  while ( cpuCount<num && getline(stats, buf) )
      if (!strncmp(buf.data(), "cpu", 3) && buf[3] != ' ')
          cpuCount++;

  if( cpuCount != num ){
    return "";
  }

  int n = buf.find (" ");
  if( n > 31 )
    n=31;

  strncpy(buffer, buf.data(), n);
  buffer[n] = '\0';

  return buffer;
}

const char *CPUMeter::toUpper(const char *str){
  static char buffer[MAX_PROCSTAT_LENGTH];
  strncpy(buffer, str, MAX_PROCSTAT_LENGTH);
  for (char *tmp = buffer ; *tmp != '\0' ; tmp++)
    *tmp = toupper(*tmp);

  return buffer;
}
