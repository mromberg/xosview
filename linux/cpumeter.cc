//
//  Copyright (c) 1994, 1995, 2002 by Mike Romberg ( romberg@fsl.noaa.gov )
//
//  This file may be distributed under terms of the GPL
//
//
// $Id: cpumeter.cc,v 1.15 2004/06/01 04:21:54 romberg Exp $
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
: FieldMeterGraph( parent, 4, toUpper(cpuID), "USR/NICE/SYS/FREE" ) {
  _lineNum = findLine(cpuID);
  for ( int i = 0 ; i < 2 ; i++ )
    for ( int j = 0 ; j < 4 ; j++ )
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
  setfieldcolor( 3, parent_->getResource( "cpuFreeColor" ) );
  priority_ = atoi (parent_->getResource( "cpuPriority" ) );
  dodecay_ = parent_->isResourceTrue( "cpuDecay" );
  useGraph_ = parent_->isResourceTrue( "cpuGraph" );
  SetUsedFormat (parent_->getResource("cpuUsedFormat"));
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
	      >>cputime_[cpuindex_][3];

  int oldindex = (cpuindex_+1)%2;
  for ( int i = 0 ; i < 4 ; i++ ){
    fields_[i] = cputime_[cpuindex_][i] - cputime_[oldindex][i];
    total_ += fields_[i];
  }

  if (total_){
    setUsed (total_ - fields_[3], total_);
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

  str << "cpu";
  if (num != 0)
    str << (num - 1);
  str << std::ends;

  strncpy(buffer, str.str().c_str(), 32);
  buffer[31] = '\0';

  return buffer;
}

const char *CPUMeter::toUpper(const char *str){
  static char buffer[MAX_PROCSTAT_LENGTH];
  strncpy(buffer, str, MAX_PROCSTAT_LENGTH);
  for (char *tmp = buffer ; *tmp != '\0' ; tmp++)
    *tmp = toupper(*tmp);

  return buffer;
}
