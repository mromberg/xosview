//
//  Initial port performed by Greg Onufer (exodus@cheers.bungi.com)
//
#include "swapmeter.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/swap.h>
#include <iostream>


SwapMeter::SwapMeter(XOSView *parent)
	: FieldMeterGraph(parent, 2, "SWAP", "USED/FREE")
{
	pagesize = sysconf(_SC_PAGESIZE);
}

SwapMeter::~SwapMeter(void)
{
}

void SwapMeter::checkResources(void)
{
	FieldMeterGraph::checkResources();

	setfieldcolor(0, parent_->getResource("swapUsedColor"));
	setfieldcolor(1, parent_->getResource("swapFreeColor"));
	priority_ = util::stoi(parent_->getResource("swapPriority"));
	dodecay_ = parent_->isResourceTrue("swapDecay");
	useGraph_ = parent_->isResourceTrue("swapGraph");
	setUsedFormat(parent_->getResource("swapUsedFormat"));
        decayUsed(parent_->isResourceTrue("swapUsedDecay"));
}

void SwapMeter::checkevent(void)
{
	getswapinfo();
	drawfields(parent_->g());
}

void SwapMeter::getswapinfo(void)
{
	swaptbl_t *swaps;
	char *names;

	total_ = fields_[0] = fields_[1] = 0;
	int numswap = swapctl(SC_GETNSWP, NULL);
	if (numswap < 0) {
		logFatal << "Can not determine number of swap spaces." << std::endl;
	}
	if (numswap > 0) {
		swaps = (swaptbl_t *)malloc(sizeof(swaptbl_t) + numswap * sizeof(swapent_t));
		names = (char *)calloc(numswap + 1, PATH_MAX);
		if (!swaps || !names) {
                    logFatal << "malloc failed." << std::endl;
		}
		swaps->swt_n = numswap;
		for (int i = 0; i <= numswap; i++)
			swaps->swt_ent[i].ste_path = names + (i * PATH_MAX);

		if (swapctl(SC_LIST, swaps) < 0) {
                    logFatal << "Can not get list of swap spaces." << std::endl;
		}
		for (int i = 0; i < numswap; i++) {
			total_ += swaps->swt_ent[i].ste_pages;
			fields_[1] += swaps->swt_ent[i].ste_free;
                        logDebug << swaps->swt_ent[i].ste_path << ": "
                                 << swaps->swt_ent[i].ste_pages * (pagesize / 1024)
                                 << " kB ("
                                 << swaps->swt_ent[i].ste_free * (pagesize / 1024)
                                 << " kB free)" << std::endl;
		}
		fields_[0] = total_ - fields_[1];
		free(swaps);
		free(names);
	}

	setUsed(fields_[0] * pagesize, total_ * pagesize);
}
