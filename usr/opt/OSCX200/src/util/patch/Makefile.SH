#
# *****************************************************************
# *                                                               *
# *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
# *                                                               *
# *   All Rights Reserved.  Unpublished rights  reserved  under   *
# *   the copyright laws of the United States.                    *
# *                                                               *
# *   The software contained on this media  is  proprietary  to   *
# *   and  embodies  the  confidential  technology  of  Digital   *
# *   Equipment Corporation.  Possession, use,  duplication  or   *
# *   dissemination of the software and media is authorized only  *
# *   pursuant to a valid written license from Digital Equipment  *
# *   Corporation.                                                *
# *                                                               *
# *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
# *   by the U.S. Government is subject to restrictions  as  set  *
# *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
# *   or  in  FAR 52.227-19, as applicable.                       *
# *                                                               *
# *****************************************************************
#
# HISTORY
#
# @(#)$RCSfile: Makefile.SH,v $ $Revision: 1.2.5.2 $ (DEC) $Date: 1993/08/03 00:06:47 $
#
case $CONFIG in
    '') . config.sh ;;
esac
echo "Extracting Makefile (with variable substitutions)"
cat >Makefile <<!GROK!THIS!
#
# 

CC = $cc
bin = $bin
mansrc = $mansrc
manext = $manext
CFLAGS = $ccflags -O
LDFLAGS = $ldflags
SMALL = $small
LARGE = $large $split

!GROK!THIS!
cat >>Makefile <<'!NO!SUBS!'

public = patch
private = 
manpages = patch.man
util = Makefile

c = patch.c pch.c inp.c version.c util.c

obj = patch.o pch.o inp.o util.o version.o

lintflags = -phbvxac

addedbyconf = Makefile.old bsd config.h config.sh eunice loc pdp11 usg v7

# grrr
SHELL = /bin/sh

.c.o:
	$(CC) -c $(CFLAGS) $(LARGE) $*.c

all: $(public) $(private) $(util)
	touch all

patch: $(obj)
	$(CC) $(LDFLAGS) $(obj) $(libs) -o patch

config.h: config.h.SH
	sh config.h.SH

# won't work with csh
install: patch
	export PATH || exit 1
	- mv $(bin)/patch $(bin)/patch.old
	- if test `pwd` != $(bin); then cp $(public) $(bin); fi
	cd $(bin); chmod 755 $(public)
	- if test `pwd` != $(mansrc); then \
for page in $(manpages); do \
rm -f $(mansrc)/../cat$(manext)/`basename $$page .man`.$(manext); \
cp $$page $(mansrc)/`basename $$page .man`.$(manext); \
done; \
fi

clean:
	rm -f *.o *.orig core

realclean:
	rm -f patch *.o *.orig core $(addedbyconf)

# The following lint has practically everything turned on.  Unfortunately,
# you have to wade through a lot of mumbo jumbo that can't be suppressed.
# If the source file has a /*NOSTRICT*/ somewhere, ignore the lint message
# for that spot.

lint:
	lint $(lintflags) $(defs) $(c) > patch.fuzz

patch.o: config.h common.h patch.c inp.h pch.h util.h version.h
pch.o: config.h common.h pch.c pch.h util.h
inp.o: config.h common.h inp.c inp.h util.h
util.o: config.h common.h util.c util.h
version.o: config.h common.h version.c version.h patchlevel.h util.h

!NO!SUBS!
$eunicefix Makefile
