//  
// $Id: cpumeter.cc,v 1.2 2001/10/17 14:03:55 eile Exp $
//  Initial port performed by Stefan Eilemann (eile@sgi.com)
//
#include "cpumeter.h"
#include "xosview.h"
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <strstream.h>

CPUMeter::CPUMeter(XOSView *parent, int cpuid)
	: FieldMeterGraph(parent, USED_CPU_STATES, toUpper(cpuStr(cpuid)),
	    "USER/SYS/INTR/WAIT/IDLE")
{
	for (int i = 0 ; i < 2 ; i++)
		for (int j = 0 ; j < USED_CPU_STATES ; j++)
			cputime_[i][j] = 0;

	if ((sinfosz = sysmp(MP_SASZ, MPSA_SINFO)) < 0) {
		cerr << "sysinfo scall interface not supported" << endl;
		parent_->done(1);
		return;
	}
    cpuid_ = cpuid;
	cpuindex_ = 0;
}

CPUMeter::~CPUMeter(void)
{
}

void CPUMeter::checkResources(void)
{
	FieldMeterGraph::checkResources();

	setfieldcolor(0, parent_->getResource("cpuUserColor"));
	setfieldcolor(1, parent_->getResource("cpuSystemColor"));
	setfieldcolor(2, parent_->getResource("cpuInterruptColor"));
	setfieldcolor(3, parent_->getResource("cpuWaitColor"));
	setfieldcolor(4, parent_->getResource("cpuFreeColor"));
	priority_ = atoi(parent_->getResource("cpuPriority"));
	dodecay_ = parent_->isResourceTrue("cpuDecay");
	useGraph_ = parent_->isResourceTrue("cpuGraph");
	SetUsedFormat(parent_->getResource("cpuUsedFormat"));
}

void CPUMeter::checkevent(void)
{
	getcputime();
	drawfields();
}

void CPUMeter::getcputime(void)
{
	total_ = 0;

    if(cpuid_==-1)
        sysmp(MP_SAGET, MPSA_SINFO, (char *) &tsp, sinfosz);
    else
        sysmp(MP_SAGET1, MPSA_SINFO, (char *) &tsp, sinfosz, cpuid_);

    tsp.cpu[CPU_WAIT] -= (tsp.wait[W_GFXF] + tsp.wait[W_GFXC]);

	cputime_[cpuindex_][0] = tsp.cpu[CPU_USER];  
	cputime_[cpuindex_][1] = tsp.cpu[CPU_KERNEL];
	cputime_[cpuindex_][2] = tsp.cpu[CPU_INTR];  
	cputime_[cpuindex_][3] = tsp.cpu[CPU_WAIT];  
	cputime_[cpuindex_][4] = tsp.cpu[CPU_IDLE] + tsp.cpu[CPU_SXBRK];

	int oldindex = (cpuindex_ + 1) % 2;
	for (int i = 0 ; i < USED_CPU_STATES ; i++) {
		fields_[i] = cputime_[cpuindex_][i] - cputime_[oldindex][i];
		total_ += fields_[i];
	}
	cpuindex_ = (cpuindex_ + 1) % 2;

	if (total_)
		setUsed(total_ - fields_[3], total_);
}

const char *CPUMeter::toUpper(const char *str)
{
	static char buffer[256];
	strncpy(buffer, str, 256);
	for (char *tmp = buffer ; *tmp != '\0' ; tmp++)
		*tmp = toupper(*tmp);

	return buffer;
}

const char *CPUMeter::cpuStr(int num)
{
	static char buffer[32];

    if( CPUMeter::countCPUs()==1 || num==-1 )
    {
        sprintf(buffer, "CPU");
    }
    else if( CPUMeter::countCPUs()<=10 )
    {
        sprintf(buffer, "#%d", num);
    }
    else if ( CPUMeter::countCPUs()<=100 )
    {
        sprintf(buffer, "#%02d", num);
    }
    else if ( CPUMeter::countCPUs()<=1000 )
    {
        sprintf(buffer, "#%03d", num);
    }
    else
    {
        sprintf(buffer, "%4.1d", num);
    }
	return buffer;
}

int CPUMeter::countCPUs()
{
	return sysmp(MP_NPROCS); // FIXME: use NAPROCS + identify 'unused procs'
}
