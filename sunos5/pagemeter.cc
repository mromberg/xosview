//  
// $Id: pagemeter.cc,v 1.2 1998/06/22 15:57:29 bgrayson Exp $
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
#include "pagemeter.h"
#include "xosview.h"
#include <stdlib.h>
#include <kstat.h>
#include <sys/sysinfo.h>

PageMeter::PageMeter(XOSView *parent, kstat_ctl_t *_kc, float max)
	: FieldMeterDecay( parent, 3, "PAGE", "IN/OUT/IDLE")
{
	kc = _kc;

	for (int i = 0 ; i < 2 ; i++)
		for (int j = 0 ; j < 2 ; j++)
			pageinfo_[j][i] = 0;

	maxspeed_ = max;
	pageindex_ = 0;

	ncpus = 0;
	for (kstat_t *ksp = kc->kc_chain; ksp != NULL; ksp = ksp->ks_next) {
		if (strncmp(ksp->ks_name, "cpu_stat", 8) == 0 &&
		    kstat_read(kc, ksp, NULL) != -1) {
			ksps[ncpus++] = ksp;
		}
	}
}

PageMeter::~PageMeter(void)
{
}

void PageMeter::checkResources(void)
{
	FieldMeterDecay::checkResources();

	setfieldcolor(0, parent_->getResource("pageInColor"));
	setfieldcolor(1, parent_->getResource("pageOutColor"));
	setfieldcolor(2, parent_->getResource("pageIdleColor"));
	priority_ = atoi (parent_->getResource("pagePriority"));
	maxspeed_ *= priority_ / 10.0;
	dodecay_ = !strcmp (parent_->getResource("pageDecay"), "True");
	SetUsedFormat(parent_->getResource("pageUsedFormat"));
}

void PageMeter::checkevent(void)
{
	getpageinfo();
	drawfields();
}

void PageMeter::getpageinfo(void)
{
	cpu_stat_t cs;

	total_ = 0;
	pageinfo_[pageindex_][0] = 0; // pgin
	pageinfo_[pageindex_][1] = 0; // pgout
	for (int i = 0; i < ncpus; i++) {
		if (kstat_read(kc, ksps[i], &cs) == -1)
			continue;
		pageinfo_[pageindex_][0] += cs.cpu_vminfo.pgpgin;
		pageinfo_[pageindex_][1] += cs.cpu_vminfo.pgpgout;
	}

	int oldindex = (pageindex_ + 1) % 2;
  
	for (int i = 0; i < 2; i++) {
		if (pageinfo_[oldindex][i] == 0)
			pageinfo_[oldindex][i] = pageinfo_[pageindex_][i];

		fields_[i] = pageinfo_[pageindex_][i] - pageinfo_[oldindex][i];
		total_ += fields_[i];
	}

	if (total_ > maxspeed_)
		fields_[2] = 0.0;
	else {
		fields_[2] = maxspeed_ - total_;
		total_ = maxspeed_;
	}

	setUsed (total_ - fields_[2], maxspeed_);
	pageindex_ = (pageindex_ + 1) % 2;
}
