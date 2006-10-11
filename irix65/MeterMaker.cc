//  
// $Id: MeterMaker.cc,v 1.9 2006/10/11 07:30:53 eile Exp $
//  Initial port performed by Stefan Eilemann (eilemann@gmail.com)
//
#include "MeterMaker.h"
#include "xosview.h"

#include "loadmeter.h"

#include "cpumeter.h"
#include "memmeter.h"
#include "gfxmeter.h"
#include "diskmeter.h"
#if 0
#include "pagemeter.h"
#endif

#include <stdlib.h>


MeterMaker::MeterMaker(XOSView *xos)
{
    _xos = xos;
}

void MeterMaker::makeMeters(void)
{
    if (_xos->isResourceTrue("load"))
        push(new LoadMeter(_xos));

    // Standard meters (usually added, but users could turn them off)
    if (_xos->isResourceTrue("cpu")) {
        bool any = false;
        const int cpuCount = CPUMeter::nCPUs();

        if( strncmp( _xos->getResource("cpuFormat"), "single", 2) == 0 || 
            strncmp( _xos->getResource("cpuFormat"), "both", 2) == 0 )
        {
            push(new CPUMeter(_xos, -1));
            any = true;
        }

        if( strncmp( _xos->getResource("cpuFormat"), "all", 2) == 0 || 
            strncmp( _xos->getResource("cpuFormat"), "both", 2) == 0 )
        {
            for (int i = 0 ; i < cpuCount ; i++)
                push(new CPUMeter(_xos, i));
            any = true;
        }

        if( strncmp( _xos->getResource("cpuFormat"), "auto", 2) == 0 )
        {
            push(new CPUMeter(_xos, -1));

            if( cpuCount>1 )
                for (int i = 0 ; i < cpuCount ; i++)
                    push(new CPUMeter(_xos, i));
            any = true;
        }

        if( !any )
        {
            for (int i = 0 ; i < cpuCount ; i++)
                push(new CPUMeter(_xos, i));
        }            

    }


    if( _xos->isResourceTrue("gfx") && GfxMeter::nPipes() > 0 )
        push(new GfxMeter( _xos, atoi( _xos->getResource( "gfxWarnThreshold" ))));

#if 0 // eile: not yet working 
    if (_xos->isResourceTrue("disk"))
        push(new DiskMeter(_xos, atof(_xos->getResource("diskBandwidth"))));
#endif

    if (_xos->isResourceTrue("mem"))
        push(new MemMeter(_xos));

}

