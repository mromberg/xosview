#
# $Id: Makefile,v 1.2 1996/08/14 06:20:23 mromberg Exp $
#

all: ../configure

../configure: configure.in aclocal.m4
	autoconf configure.in > ../configure
	chmod 755 ../configure

clean:
	rm -f *~
