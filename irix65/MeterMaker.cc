//  
// $Id: MeterMaker.cc,v 1.4 2002/02/20 11:15:04 eile Exp $
//  Initial port performed by Stefan Eilemann (eile@sgi.com)
//
#include "MeterMaker.h"
#include "xosview.h"

#include "loadmeter.h"

#include "cpumeter.h"
#include "memmeter.h"
#include "gfxmeter.h"
#if 0
#include "swapmeter.h"
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
        bool any = 0;
        int cpuCount = CPUMeter::countCPUs();

        if( strncmp( _xos->getResource("cpuFormat"), "single", 2) == 0 || 
            strncmp( _xos->getResource("cpuFormat"), "both", 2) == 0 )
        {
            push(new CPUMeter(_xos, -1));
            any = 1;
        }

        if( strncmp( _xos->getResource("cpuFormat"), "all", 2) == 0 || 
            strncmp( _xos->getResource("cpuFormat"), "both", 2) == 0 )
        {
            for (int i = 0 ; i < cpuCount ; i++)
                push(new CPUMeter(_xos, i));
            any = 1;
        }

        if( strncmp( _xos->getResource("cpuFormat"), "auto", 2) == 0 )
        {
            push(new CPUMeter(_xos, -1));

            if( cpuCount>1 )
                for (int i = 0 ; i < cpuCount ; i++)
                    push(new CPUMeter(_xos, i));
            any = 1;
        }

        if( !any )
        {
            int cpuCount = CPUMeter::countCPUs();
            for (int i = 0 ; i < cpuCount ; i++)
                push(new CPUMeter(_xos, i));
        }            

    }

    if (_xos->isResourceTrue("gfx"))
        push(new GfxMeter( _xos, atoi( _xos->getResource( "gfxWarnThreshold" ))));

    if (_xos->isResourceTrue("mem"))
        push(new MemMeter(_xos));

#if 0
    if (_xos->isResourceTrue("swap"))
        push(new SwapMeter(_xos, kc));

    if (_xos->isResourceTrue("page"))
        push(new PageMeter(_xos, kc,
            atof(_xos->getResource("pageBandwidth"))));
#endif
}

