#
# $Id: Makefile,v 1.3 2002/02/18 02:09:21 bgrayson Exp $
#

all: ../configure ../config.h.in

../configure: configure.in *.m4
	autoconf -Wall configure.in > ../configure
	chmod 755 ../configure

../config.h.in: configure.in *.m4
	autoheader && mv -f config.h.in ..

clean:
	rm -f *~

#  NOTE:  To update to a new version of autoconf, the config.sub,
#  config.guess, and install-sh scripts need to be copied manually
#  from the autoconf tarball.  (At least, the autoreconf -i flag
#  didn't seem to do the trick.)
#  Updated to autoconf-2.52 on Feb 17, 2001
#  - bgrayson
