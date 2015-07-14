  Since the Gnu/hurd now has a /proc filesystem (with a stat file
that uses the same format as Linux (this is an awsome trend for operating
systems), I've added a copy of the Linux cpumeter.  So, Gnu/hurd now
has one.  It should be able to do SMP displays if/when the hurd supports it.

  The other hurd meters still worked using methods that poked around inside
mach.  They could be rewritten to use /proc and in doing so become a little
more isolated from changes in that code.

  The /proc/stat file on hurd does have an intr line.  So, the linux irqrate
meter should also work out of the box.  I did not copy it over because at
the moment the intr line is always 0.  If someone fixes that then it *should*
just work (after copying over and hooking into MeterMaker).


