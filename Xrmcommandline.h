//
// $Id: Xrmcommandline.h,v 1.7 1996/12/26 20:18:45 mromberg Exp $
//

#ifndef _Xrmcommandline_h
#define _Xrmcommandline_h

#define XRMCOMMANDLINE_H_CVSID "$Id: Xrmcommandline.h,v 1.7 1996/12/26 20:18:45 mromberg Exp $"

#include <X11/Xresource.h>

static XrmOptionDescRec options[] = {
//  For these options, try to use '+' to turn them on, and '-' to turn them
//    off, even though this is different from the usual tradition of -foo
//    turning on 'foo', which is off by default.  At least this way,
//    it is self-consistent, and self-explanatory.

//  General, X11 resources:
{ "-display", "*display", XrmoptionSepArg, (caddr_t) NULL },
{ "-font", "*font", XrmoptionSepArg, (caddr_t) NULL },
{ "-title", "*title", XrmoptionSepArg, (caddr_t) NULL },
{ "-geometry", "*geometry", XrmoptionSepArg, (caddr_t) NULL },

//  XOSView-specific resources:
{ "-labels", "*labels", XrmoptionNoArg, "False" },
{ "+labels", "*labels", XrmoptionNoArg, "True" },
//  CPU resources
{ "-cpu", "*cpu", XrmoptionNoArg, "False" },
{ "+cpu", "*cpu", XrmoptionNoArg, "True" },
// Load resources
{ "-load", "*load", XrmoptionNoArg, "False" },
{ "+load", "*load", XrmoptionNoArg, "True" },
// Memmeter resources
{ "-mem", "*mem", XrmoptionNoArg, "False" },
{ "+mem", "*mem", XrmoptionNoArg, "True" },
// Swapmeter resources
{ "-swap", "*swap", XrmoptionNoArg, "False" },
{ "+swap", "*swap", XrmoptionNoArg, "True" },

//  Previously, network was overloaded to be the bandwidth and the
//  on/off flag.  Now, we have -net for on/off, and networkBandwidth
//  for bandwidth, with the alias networkBW, and network for backwards
//  compatibility.
{ "-network", "*networkBandwidth", XrmoptionSepArg, (caddr_t) NULL },
{ "-networkBW", "*networkBandwidth", XrmoptionSepArg, (caddr_t) NULL },
{ "-networkBandwidth", "*networkBandwidth", XrmoptionSepArg, (caddr_t) NULL },
//  -net is an abbreviation for -network
{ "-net", "*net", XrmoptionNoArg, "False" },
{ "+net", "*net", XrmoptionNoArg, "True" },

// Page Meter
{ "-page", "*page", XrmoptionNoArg, "False" },
{ "+page", "*page", XrmoptionNoArg, "True" },
{ "-pagespeed", "*pageBandWidth", XrmoptionSepArg, (caddr_t) NULL },

//  Serial Meter Options
{ "+serial0", "*serial0", XrmoptionNoArg, "True" },
{ "-serial0", "*serial0", XrmoptionNoArg, "False" },
{ "+serial1", "*serial1", XrmoptionNoArg, "True" },
{ "-serial1", "*serial1", XrmoptionNoArg, "False" },
{ "+serial2", "*serial2", XrmoptionNoArg, "True" },
{ "-serial2", "*serial2", XrmoptionNoArg, "False" },
{ "+serial3", "*serial3", XrmoptionNoArg, "True" },
{ "-serial3", "*serial3", XrmoptionNoArg, "False" },
{ "+serial4", "*serial4", XrmoptionNoArg, "True" },
{ "-serial4", "*serial4", XrmoptionNoArg, "False" },
{ "+serial5", "*serial5", XrmoptionNoArg, "True" },
{ "-serial5", "*serial5", XrmoptionNoArg, "False" },
{ "+serial6", "*serial6", XrmoptionNoArg, "True" },
{ "-serial6", "*serial6", XrmoptionNoArg, "False" },
{ "+serial7", "*serial7", XrmoptionNoArg, "True" },
{ "-serial7", "*serial7", XrmoptionNoArg, "False" },
{ "+serial8", "*serial8", XrmoptionNoArg, "True" },
{ "-serial8", "*serial8", XrmoptionNoArg, "False" },
{ "+serial9", "*serial9", XrmoptionNoArg, "True" },
{ "-serial9", "*serial9", XrmoptionNoArg, "False" },

#ifdef XOSVIEW_NETBSD
//  Disk Meter Options
{ "-disk", "*disk", XrmoptionNoArg, "False" },
{ "+disk", "*disk", XrmoptionNoArg, "True" },
#endif
//  Special, catch-all option here --
//    xosview -xrm "*memFreeColor: purple" should work, for example.
{ "-xrm", "*xrm", XrmoptionResArg, (caddr_t) NULL },
};
//  This auto-detects changes in the number of options.
static const int NUM_OPTIONS = sizeof(options) / sizeof(options[0]);




#endif
