//  
// $Id: memmeter.cc,v 1.3 1998/09/18 19:53:49 bgrayson Exp $
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
#include "memmeter.h"
#include "xosview.h"
#include <unistd.h>
#include <stdlib.h>

MemMeter::MemMeter(XOSView *parent, kstat_ctl_t *_kc)
	: FieldMeterDecay(parent, 2, "MEM", "USED/FREE")
{
	kc = _kc;

	_pageSize = sysconf(_SC_PAGESIZE);
	total_ = sysconf(_SC_PHYS_PAGES);

	ksp = kstat_lookup(kc, "unix", 0, "system_pages");
	if (ksp == NULL) {
		parent_->done(1);
		return;
	}
}

void MemMeter::checkResources(void)
{
	FieldMeterDecay::checkResources();

	setfieldcolor(0, parent_->getResource("memUsedColor"));
	setfieldcolor(1, parent_->getResource("memFreeColor"));
	priority_ = atoi (parent_->getResource("memPriority"));
	dodecay_ = !strncasecmp (parent_->getResource("memDecay"), "True", 5);
	SetUsedFormat(parent_->getResource("memUsedFormat"));
}

MemMeter::~MemMeter(void)
{
}

void MemMeter::checkevent(void)
{
	getmeminfo();
	drawfields();
}

void MemMeter::getmeminfo(void)
{
	kstat_named_t *k;

	if (kstat_read(kc, ksp, NULL) == -1) {
		parent_->done(1);
		return;
	}
	k = (kstat_named_t *)kstat_data_lookup(ksp, "freemem");
	if (k == NULL) {
		parent_->done(1);
		return;
	}
	fields_[0] = total_ - k->value.l;
	fields_[1] = k->value.l;

	FieldMeterDecay::setUsed(fields_[0] * _pageSize, total_ * _pageSize);
}
