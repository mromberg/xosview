//
//  Copyright (c) 2008, 2015 by Tomi Tapper <tomi.o.tapper@jyu.fi>
//
//  Read coretemp reading with sysctl and display actual temperature.
//  If actual >= high, actual temp changes color to indicate alarm.
//
//  File based on linux/lmstemp.* by
//  Copyright (c) 2000, 2006 by Leopold Toetsch <lt@toetsch.at>
//
//  This file may be distributed under terms of the GPL
//
//
//

#include "coretemp.h"
#include "kernel.h"



CoreTemp::CoreTemp( XOSView *parent, const std::string &label,
  const std::string &caption, int cpu)
    : FieldMeter( parent, 3, label, caption, 1, 1, 1 ),
      cpu_(cpu), cpucount_(countCpus()),
      high_(0), temps_(cpucount_, 0),
      actcolor_(0), highcolor_(0) {

    setMetric(true);
}


CoreTemp::~CoreTemp( void ) {
}


void CoreTemp::checkResources(const ResDB &rdb) {
    FieldMeter::checkResources(rdb);

    actcolor_  = rdb.getColor("coretempActColor");
    highcolor_ = rdb.getColor("coretempHighColor");

    setfieldcolor( 0, actcolor_ );
    setfieldcolor( 1, rdb.getColor( "coretempIdleColor") );
    setfieldcolor( 2, highcolor_ );

    priority_ = util::stoi( rdb.getResource( "coretempPriority" ) );
    std::string highest = rdb.getResourceOrUseDefault(
        "coretempHighest", "100" );
    total_ = util::stoi( highest );
    std::string high = rdb.getResourceOrUseDefault("coretempHigh", "");
    setUsedFormat( rdb.getResource( "coretempUsedFormat" ) );

    // Get tjMax here and use as total.
    float total = -300.0;
    std::vector<float> tjmax(cpucount_, 0.0);
    BSDGetCPUTemperature(temps_, tjmax);
    for (int i = 0; i < cpucount_; i++) {
        if (tjmax[i] > total)
            total = tjmax[i];
    }

    if (total > 0.0)
        total_ = total;

    std::string lgnd;
    if (!high.size()) {
        high_ = total_;
        lgnd = "ACT(\260C)/HIGH/" + util::repr((int)total_);
    }
    else {
        high_ = util::stoi( high );
        lgnd = "ACT(\260C)/" + util::repr((int)high_)
            + "/" + util::repr((int)total_);
    }
    legend(lgnd);
}


unsigned int CoreTemp::countCpus( void ) {
    return BSDGetCPUTemperature();
}


void CoreTemp::checkevent( void ) {
    getcoretemp();
}


void CoreTemp::getcoretemp( void ) {
    BSDGetCPUTemperature(temps_);

    fields_[0] = 0.0;
    if ( cpu_ >= 0 && cpu_ < cpucount_ ) {  // one core
        fields_[0] = temps_[cpu_];
    }
    else if ( cpu_ == -1 ) {  // average
        float tempval = 0.0;
        for (int i = 0; i < cpucount_; i++)
            tempval += temps_[i];
        fields_[0] = tempval / (float)cpucount_;
    }
    else if ( cpu_ == -2 ) {  // maximum
        float tempval = -300.0;
        for (int i = 0; i < cpucount_; i++) {
            if (temps_[i] > tempval)
                tempval = temps_[i];
        }
        fields_[0] = tempval;
    }
    else {    // should not happen
        logFatal << "Unknown CPU core number in coretemp." << std::endl;
    }

    setUsed(fields_[0], total_);
    if (fields_[0] < 0)
        fields_[0] = 0;
    fields_[1] = high_ - fields_[0];
    fields_[2] = total_ - fields_[1] - fields_[0];
    if (fields_[0] > total_)
        fields_[0] = total_;
    if (fields_[2] < 0)
        fields_[2] = 0;

    if (fields_[1] < 0) { // alarm: T > high
        fields_[1] = 0;
        if (colors_[0] != highcolor_) {
            setfieldcolor( 0, highcolor_ );
        }
    }
    else {
        if (colors_[0] != actcolor_) {
            setfieldcolor( 0, actcolor_ );
        }
    }
}
