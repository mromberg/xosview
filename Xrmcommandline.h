//
// $Id: Xrmcommandline.h,v 1.15 1998/05/17 21:26:19 bgrayson Exp $
//

#ifndef _Xrmcommandline_h
#define _Xrmcommandline_h

#define XRMCOMMANDLINE_H_CVSID "$Id: Xrmcommandline.h,v 1.15 1998/05/17 21:26:19 bgrayson Exp $"

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
{ "-usedlabels", "*usedlabels", XrmoptionNoArg, "False" },
{ "+usedlabels", "*usedlabels", XrmoptionNoArg, "True" },
//  This next one is not yet supported.
#if 0
{ "-samplesPerSec", "*samplesPerSec", XrmoptionSepArg, (caddr_t) NULL },
#endif
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
// Batterymeter resources
{ "-battery", "*battery", XrmoptionNoArg, "False" },
{ "+battery", "*battery", XrmoptionNoArg, "True" },

// Networkmeter resources
{ "-net", "*net", XrmoptionNoArg, "False" },
{ "+net", "*net", XrmoptionNoArg, "True" },
//  Previously, network was overloaded to be the bandwidth and the
//  on/off flag.  Now, we have -net for on/off, and networkBandwidth
//  for bandwidth, with the alias networkBW, and network for backwards
//  compatibility.
{ "-network", "*netBandwidth", XrmoptionSepArg, (caddr_t) NULL },
{ "-networkBW", "*netBandwidth", XrmoptionSepArg, (caddr_t) NULL },
{ "-networkBandwidth", "*netBandwidth", XrmoptionSepArg, (caddr_t) NULL },

// Page Meter
{ "-page", "*page", XrmoptionNoArg, "False" },
{ "+page", "*page", XrmoptionNoArg, "True" },
{ "-pagespeed", "*pageBandWidth", XrmoptionSepArg, (caddr_t) NULL },

#if defined(XOSVIEW_NETBSD) || defined(XOSVIEW_FREEBSD) || defined(XOSVIEW_OPENBSD)
//  Disk Meter Options
{ "-disk", "*disk", XrmoptionNoArg, "False" },
{ "+disk", "*disk", XrmoptionNoArg, "True" },
#endif
// Interrupt meter resources  --  all sorts of aliases.
{ "-int", "*interrupts", XrmoptionNoArg, "False" },
{ "+int", "*interrupts", XrmoptionNoArg, "True" },
{ "-ints", "*interrupts", XrmoptionNoArg, "False" },
{ "+ints", "*interrupts", XrmoptionNoArg, "True" },
{ "-interrupts", "*interrupts", XrmoptionNoArg, "False" },
{ "+interrupts", "*interrupts", XrmoptionNoArg, "True" },
//  Special, catch-all option here --
//    xosview -xrm "*memFreeColor: purple" should work, for example.
{ "-xrm", "*xrm", XrmoptionResArg, (caddr_t) NULL },
};
//  This auto-detects changes in the number of options.
static const int NUM_OPTIONS = sizeof(options) / sizeof(options[0]);

#endif
