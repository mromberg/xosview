//
//  Copyright (c) 1994, 1995, 2002, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "cpumeter.h"
#include "xosview.h"
#include <fstream>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <ctype.h>

static const char STATFILENAME[] = "/proc/stat";
static const size_t MAX_PROCSTAT_LENGTH = 4096;

CPUMeter::CPUMeter(XOSView *parent, unsigned int cpu)
    : FieldMeterGraph( parent, 8, util::toupper(CPUMeter::cpuStr(cpu)),
      "USR/NICE/SYS/SI/HI/WIO/FREE/ST" ), cpuindex_(0), _cpu(cpu) {

    for ( unsigned int i = 0 ; i < 2 ; i++ )
        for ( unsigned int j = 0 ; j < 8 ; j++ )
            cputime_[i][j] = 0;

    _lineNum = findLine();
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
    priority_ = util::stoi (parent_->getResource( "cpuPriority" ));
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
        logFatal << "Can not open file : " << STATFILENAME << std::endl;
    }

    // read until we are at the right line.
    for (unsigned int i = 0 ; i < _lineNum ; i++) {
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

    static int cputime_to_field[8] = { 0, 1, 2, 6, 5, 4, 3, 7 };
    int oldindex = (cpuindex_+1)%2;
    for ( int i = 0 ; i < 8 ; i++ ){
        int field = cputime_to_field[i];
        fields_[field] = cputime_[cpuindex_][i] - cputime_[oldindex][i];
        total_ += fields_[field];
    }

    // funny things happen after sleep/hibernate
    if (total_ < 0)
        for (int i = 0 ; i < 8 ; i++)
            total_ = fields_[i] = 0;

    if (total_){
        setUsed (total_ - (fields_[5] + fields_[6] + fields_[7]), total_);
        cpuindex_ = (cpuindex_ + 1) % 2;
    }
}

size_t CPUMeter::findLine(void) {
    std::ifstream stats(STATFILENAME);

    if ( !stats )
        logFatal << "Can not open file : " << STATFILENAME << std::endl;

    std::string cpuID(CPUMeter::cpuStr(_cpu));
    // make the ws part of the id to tell cpu1 from cpu11
    cpuID += " ";
    size_t line = 0;
    std::string buf;
    while (!stats.eof()){
        getline(stats, buf);
        if (!stats.eof()){
            if ((cpuID == buf.substr(0, cpuID.size()))) {
                return line;
            }
        }
        line++;
    }
    logFatal << "Failed to find " << cpuID
             << " in " << STATFILENAME << std::endl;
    return 0;
}

// Checks for the SMP kernel patch by forissier@isia.cma.fr.
// http://www-isia.cma.fr/~forissie/smp_kernel_patch/
// If it finds that this patch has been applied to the current kernel
// then returns the number of cpus that are on this machine.
size_t CPUMeter::countCPUs(void){
    static size_t cpuCount = 0;
    static bool first = true;

    if (first) {
        first = false;
        std::ifstream stats( STATFILENAME );

        if ( !stats )
            logFatal << "Can not open file : " << STATFILENAME << std::endl;

        std::string buf;
        while (getline(stats, buf))
            if ((buf.substr(0, 3) == "cpu") && buf[3] != ' ')
                cpuCount++;
    }

    return cpuCount;
}

std::string CPUMeter::cpuStr(size_t num){
    if (num == 0)  // The cumulative meter
        return "cpu";
    return std::string("cpu") + util::repr(num-1);
}
