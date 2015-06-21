//
//  Copyright (c) 1994, 1995, 2006, 2015
//  by Mike Romberg ( mike-romberg@comcast.net )
//
//  This file may be distributed under terms of the GPL
//

#include "intmeter.h"

#include <fstream>
#include <sstream>
#include <limits>


static const char *INTFILE     = "/proc/interrupts";
static 	int realintnum[1024]; // FIXME: global var

std::map<size_t,unsigned int> IntMeter::_irqmap; // linenum -> irq

IntMeter::IntMeter( XOSView *parent, unsigned int cpu, unsigned int cpuTot)
    : BitMeter( parent, "INT", "", 1, 0, 0 ), _cpu(cpu), _cpuTot(cpuTot) {
    // if cpu == 0:  act as cummulative
    // if cpu >= 1:  meter for cpu - 1 (in /proc/interrupts)

    logDebug << "IntMeter::IntMeter(): " << _cpu << " of " << _cpuTot << "\n";
    _maxIRQ = irqcount();
    logDebug << "irqCount: " << _maxIRQ << std::endl;
    logDebug << "IRQ map: " << _irqmap << std::endl;

    initUI();

    _last = readCounts();
    logDebug << "_last: " << _last << std::endl;
}

IntMeter::~IntMeter( void ){
}

void IntMeter::checkevent( void ){
    std::vector<unsigned long long> newc = readCounts();
    for (size_t i = 0 ; i < numBits() ; i++) {
        bits_[i] = newc[i] - _last[i];
    }
    _last = newc;

    BitMeter::checkevent();
}

void IntMeter::checkResources( void ){
    BitMeter::checkResources();
    onColor_  = parent_->allocColor( parent_->getResource( "intOnColor" ) );
    offColor_ = parent_->allocColor( parent_->getResource( "intOffColor" ) );
    priority_ = util::stoi(parent_->getResource("intPriority"));
}

void IntMeter::getirqs( void ){
    std::ifstream intfile( INTFILE );
    unsigned int intno, count;
    int	idx;

    if ( !intfile ){
        logFatal << "Can not open file : " <<INTFILE << std::endl;
    }

    while ( !intfile.eof() ){
        intfile >> idx;
        intno = realintnum[idx];
        if(intno>=numBits())
            updateirqcount(intno,false);
        if (!intfile) break;
        intfile.ignore(1);
        if ( !intfile.eof() ){
            for (unsigned int i = 0 ; i <= _cpu ; i++)
                intfile >>count;
            intfile.ignore(1024, '\n');

            irqs_[intno] = count;
        }
    }
}

/* The highest numbered interrupts, the number of interrupts
 * is going to be at least +1 (for int 0) and probably higher
 * if interrupts numbered more than this one just aren't active.
 * Must call with init = true the first time.
 */
void IntMeter::updateirqcount( int n, bool init ){
    int old_bits=numBits();
    setNumBits(n+1);
    std::ostringstream os;

    os << "INTs (0-16" ;
    for (int i=16; i<1024; i++) {
	if (realintnum[i])
            os << ", " << (i) ;
    }
    os << ")" << std::ends;

    legend(os.str().c_str());
    std::vector<unsigned long> old_irqs_(irqs_);
    std::vector<unsigned long> old_lastirqs_(lastirqs_);
    irqs_.resize(n+1);
    lastirqs_.resize(n+1);
    /* If we are in init, set it to zero,
     * otherwise copy over the old set */
    if( init ) {
        for( int i=0; i < n; i++)
            irqs_[i]=lastirqs_[i]=0;
    }
    else {
   	for( int i=0; i < old_bits; i++) {
            irqs_[i]=old_irqs_[i];
            lastirqs_[i]=old_lastirqs_[i];
	}
	// zero to the end the irq's that haven't been seen before
	for( unsigned int i=old_bits; i< numBits(); i++) {
            irqs_[i]=lastirqs_[i]=0;
        }
    }
}

unsigned int IntMeter::irqcount( void ){
    // find and return the highest NUMBERED irq
    // The unnumbered ones (such as NMI, IWI, etc) leave
    // for later.
    // populates the _irqmap
    int rval = -1;

    // Check and see if any IntMeter has done this before
    if (_irqmap.size()) {
        for (std::map<size_t, unsigned int>::const_iterator i=_irqmap.begin() ;
             i != _irqmap.end() ; ++i) {
            rval = std::max(rval, static_cast<int>(i->second));
        }
    }
    else
        rval = loadIRQMap();


    if (rval == -1)
        logProblem << "IRQ count failed." << std::endl;

    return std::max(rval, 0);
}

int IntMeter::loadIRQMap(void) {
    int rval = -1;

    std::ifstream intfile( INTFILE );
    if ( !intfile )
        logFatal << "Can not open file : " << INTFILE << std::endl;

    // Dump the first line
    intfile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (!intfile)
        logFatal << "Error reading: " << INTFILE << std::endl;

    size_t lineNum = 1;

    // just looking for the highest number interrupt that
    // is in use, ignore the rest of the data
    while ( !intfile.eof() ){
        std::string rowLabel;

        intfile >> rowLabel;
        if (!intfile.eof()) {
            if (intfile.fail())
                logFatal << "failed reading: " << INTFILE << std::endl;

            if ((rowLabel.size() == 0)
              || (rowLabel[rowLabel.size()-1] != ':')) {
                logProblem << "unexpected row label: " << rowLabel << std::endl;
            }
            else {
                int inum = 0;
                std::string istr = rowLabel.substr(0, rowLabel.size()-1);
                if (util::fstr(istr, inum)) {
                    rval = std::max(rval, inum);
                    _irqmap[lineNum] = inum;
                }
                else {
                    //logDebug << "skiping irq: " << istr << std::endl;
                }
            }

            // skip to next line
            intfile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (intfile.fail())
                logFatal << "failed reading: " << INTFILE << std::endl;
            lineNum++;
        }
    }

    return rval;
}

void IntMeter::initUI(void) {
    // parent handles the bit display
    setNumBits(_maxIRQ + 1);

    // set the legend
    std::string lgnd("IRQS: (0-");
    lgnd += util::repr(_maxIRQ) + ")";
    legend(lgnd);

    // set the CPU in the title
    std::ostringstream os;
    os << "INT";
    if (_cpu != 0) {
        os << "(";
        os << _cpu - 1 << ")";
    }
    title(os.str());
}

std::vector<unsigned long long> IntMeter::readCounts(void) const {

    std::vector<unsigned long long> rval;
    rval.resize(_maxIRQ + 1, 0);

    std::ifstream ifs(INTFILE);
    if (!ifs)
        logFatal << "Could not open: " << INTFILE << std::endl;

    // Dump the first line
    ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    size_t lineNo = 1;
    while (!ifs.eof()) {
        std::map<size_t,unsigned int>::const_iterator mi = _irqmap.find(lineNo);
        if (mi != _irqmap.end()) {
            std::string ln;
            std::getline(ifs, ln);
            if (!ifs)
                logFatal << "error reading: " << INTFILE << std::endl;
            rval[mi->second] = parseLine(ln);
        }
        else
            ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        lineNo++;
    }

    return rval;
}

unsigned long long IntMeter::parseLine(const std::string &line) const {
    unsigned long long rval = 0;

    std::istringstream is(line);
    is.ignore(std::numeric_limits<std::streamsize>::max(), ':');

    if (_cpu == 0) {
        // cummulative mode
        for (unsigned int i = 0 ; i < _cpuTot ; i++) {
            unsigned long long n;
            is >> n;
            if (is.fail())
                logFatal << "failed parsing line: " << line << std::endl;
            rval += n;
        }
    }
    else {
        // Single CPU counts
        unsigned long long n;
        for (unsigned int i = 0 ; i < _cpu ; i++)
            is >> n;

        if (is.fail())
            logFatal << "failed parsing line: " << line << std::endl;
        rval = n;
    }

    return rval;
}
