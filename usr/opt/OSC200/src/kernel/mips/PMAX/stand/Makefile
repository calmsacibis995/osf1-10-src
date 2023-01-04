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
#
# HISTORY
#

#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:37:46 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#
#
# OSF/1 Release 1.0
#

#
#		Copyright (c) 1985 by
#	Digital Equipment Corporation, Maynard, MA
#		All rights reserved.
#
#  This software is furnished under a license and may be used and
#  copied  only  in accordance with the terms of such license and
#  with the  inclusion  of  the  above  copyright  notice.   This
#  software  or  any  other copies thereof may not be provided or
#  otherwise made available to any other person.  No title to and
#  ownership of the software is hereby transferred.
#
#  This software is  derived  from  software  received  from  the
#  University    of   California,   Berkeley,   and   from   Bell
#  Laboratories.  Use, duplication, or disclosure is  subject  to
#  restrictions  under  license  agreements  with  University  of
#  California and with AT&T.
#
#  The information in this software is subject to change  without
#  notice  and should not be construed as a commitment by Digital
#  Equipment Corporation.
#
#  Digital assumes no responsibility for the use  or  reliability
#  of its software on equipment which is not supplied by Digital.
#

.SUFFIXES: .S .s
DESTROOT=
INCFLAGS= -I.
#
# Security note:  Uncomment the next line and comment the one following
#                 it to build for a traditional filesystem format
# CFLAGS= -DPMAX
CFLAGS= -DPMAX -DSEC_MAC_OB -DSEC_BASE
ASFLAGS= -DLOCORE -DSTANDALONE -DPMAX -DASSEMBLER
MIPS_RELOC=80700000
MIPS_INTERLUDE_RELOC=80800000
RELEASE=0.0

#
# General maintenance rules
#
BOOTS = mach_boot osf_boot bootblks xxboot bootxx
LOADERS = netload
ALL = ${BOOTS} ${LOADERS}

USE_STATIC_LIBRARIES=
OBJECT_FORMAT	= COFF

PROGRAMS	= load
OTHERS		= ${ALL}
GARBAGE		= *.out core *.o errs makefile.bak version.c xxboot

load_CCTYPE	= host
load.o_CCTYPE	= host


install: all
	-if [ ! -d ${DESTROOT}/usr/mdec ] ; then \
		mkdir ${DESTROOT}/usr/mdec; \
		chmod 755 ${DESTROOT}/usr/mdec; \
		/etc/chown root ${DESTROOT}/usr/mdec; \
		chgrp system ${DESTROOT}/usr/mdec; \
		else true; \
	fi
	install -m 444 -c bootblks ${DESTROOT}/usr/mdec/bootblks
	install -m 444 -c xxboot ${DESTROOT}/usr/mdec/rzboot
	install -m 444 -c bootxx ${DESTROOT}/usr/mdec/bootrz
	install -m 444 -c netload ${DESTROOT}/usr/mdec/netload
	install -m 444 -c mach_boot ${DESTROOT}/mach_boot
	install -m 444 -c osf_boot ${DESTROOT}/osf_boot

oldclean:
	rm -f *.out core ${BOOTS} ${LOADERS} *.o errs \
		makefile.bak version.c xxboot

clobber:
	for i in `echo [a-z]*`; do \
		if test -f $$i; \
		then \
		rm -f $$i; \
		fi; \
	done
	rm -f Makefile

sccsget:
	sccs get ${DATE} -s SCCS

sccsinfo:
	sccs info

#
# Specific rules
#

version.c: FRC
	echo "char *version=\""`date`"\";" > version.c

version.o: version.c
	-rm -f version.o
	${_CC_} ${_CCFLAGS_} -c version.c

sys.o sys.SECONDARY.o sys.LABELS.o sys.LSECONDARY.o: sys.c
	${_CC_} ${_CCFLAGS_} -DCOFF -DSECONDARY -c sys.c
	mv -f sys.o sys.SECONDARY.o
	${_CC_} ${_CCFLAGS_} -DCOFF -DSECONDARY -DLABELS -c sys.c
	mv -f sys.o sys.LSECONDARY.o
	${_CC_} ${_CCFLAGS_} -DLABELS -c sys.c
	mv -f sys.o sys.LABELS.o
	${_CC_} ${_CCFLAGS_} -c sys.c

#
#
#
# loadimage understands the following macros
#
#           LOADSYMBOLS - load the symbol table after the bss
#                       section. The symbol table is used by kdb
#
#           SECONDARY - The secondary bootstrap can be reasonably
#                       large. The primary must fit in the first 15
#                       sectors on the disk.
#
#		At least one of the following options must be selected:
#
#           COFF      - If COFF is defined then loadimage is built
#                       to understand COFF file format.
#
#           MACHO     - If MACHO is defined then loadimage is built
#                       to understand MACHO file format.
#

loadimage.o loadimage.SECONDARY.o: loadimage.c
	${_CC_} ${_CCFLAGS_} -DCOFF -DSECONDARY -DLOADSYMBOLS -c loadimage.c
	mv -f loadimage.o loadimage.SECONDARY.o
	${_CC_} ${_CCFLAGS_} -DCOFF  -c loadimage.c

mach_boot.o osf_boot.o: mach_boot.c
	${_CC_} ${_CCFLAGS_} -DLABELS -c mach_boot.c
	mv -f mach_boot.o osf_boot.o
	${_CC_} ${_CCFLAGS_} -c mach_boot.c


#
#
#  The LOADSYMBOLS macro tells mach_o_boot to load the symbol
#  table
#
mach_o_boot.o: mach_o_boot.c
	${_CC_} ${_CCFLAGS_} -DLOADSYMBOLS -c mach_o_boot.c

netload.o: netload.c
	${_CC_} ${_CCFLAGS_} -c netload.c

mipsboot.o mipsboot.LABELS.o: mipsboot.c
	${_CC_} ${_CCFLAGS_} -DLABELS -c mipsboot.c
	mv -f mipsboot.o mipsboot.LABELS.o
	${_CC_} ${_CCFLAGS_} -c mipsboot.c

io.o io.LABELS.o: io.c
	${_CC_} ${_CCFLAGS_} -DLABELS -c io.c
	mv -f io.o io.LABELS.o
	${_CC_} ${_CCFLAGS_} -c io.c

xxboot: bootblk.b
	uudecode bootblk.b
	mv -f bootblk $@
	ls -l $@

FRC:

#
# boot image which resides in LBNs 0-15
# xxboot is image of LBN 0, bootxx is LBNs 1-15
#
bootblks: xxboot bootxxP
	cat xxboot bootxxP > $@
	ls -l $@

bootxxP: csu.mips.o prom_entry.o libasm.o mipsboot.o io.o sys.o loadimage.o load
	${_LD_} -T 80600000 -N -m -s -x csu.mips.o prom_entry.o libasm.o \
		mipsboot.o io.o sys.o loadimage.o
	./load -b 512 -N 15 a.out > $@
	ls -l $@

bootxx: csu.mips.o prom_entry.o libasm.o mipsboot.LABELS.o io.LABELS.o \
		sys.LABELS.o loadimage.o load
	${_LD_} -T 80600000 -N -m -s -x csu.mips.o prom_entry.o libasm.o \
		mipsboot.LABELS.o io.LABELS.o sys.LABELS.o loadimage.o
	./load -b 512 -N 15 a.out > $@
	ls -l $@
#
# mach_boot rules
#
mach_boot: csu.mips.o prom_entry.o libasm.o mach_boot.o \
		sys.SECONDARY.o io.o loadimage.SECONDARY.o
	make version.o; ${_LD_} ${_LDFLAGS_} -N -T ${MIPS_RELOC} -o $@ \
		csu.mips.o libasm.o \
		prom_entry.o mach_boot.o sys.SECONDARY.o io.o \
		loadimage.SECONDARY.o version.o ; rm -f version.o

osf_boot: csu.mips.o prom_entry.o libasm.o osf_boot.o \
		sys.LSECONDARY.o io.LABELS.o loadimage.SECONDARY.o
	make version.o; ${_LD_} ${_LDFLAGS_} -N -T ${MIPS_RELOC} -o $@ \
		csu.mips.o libasm.o \
		prom_entry.o osf_boot.o sys.LSECONDARY.o io.LABELS.o \
		loadimage.SECONDARY.o version.o ; rm -f version.o

#
# mach_o_boot rules (interlude boot program)
#
mach_o_boot: csu.mips.o prom_entry.o libasm.o mach_o_boot.o \
		sys.LSECONDARY.o io.LABELS.o 
	make version.o; ${_LD_} ${_LDFLAGS_} -N -T ${MIPS_INTERLUDE_RELOC} \
		-o $@ csu.mips.o libasm.o \
		prom_entry.o mach_o_boot.o sys.LSECONDARY.o io.LABELS.o \
	 	version.o -lld -lc ; rm -f version.o


#
# netload rules
#
netload: csu.mips.o prom_entry.o \
		libasm.o netload.o
	make version.o; ${_LD_} -N -T ${MIPS_RELOC} -o $@ \
		csu.mips.o prom_entry.o \
		libasm.o netload.o version.o; rm -f version.o

# change for new build tools

csu.mips.S: csu.mips.s
	rm -f csu.mips.S
	cp csu.mips.s csu.mips.S

csu.mips.o: csu.mips.S
	${_CC_}  -g0 ${_CCFLAGS_} ${ASFLAGS} -c csu.mips.S -o csu.mips.o

prom_entry.S: prom_entry.s
	rm -f prom_entry.S
	cp prom_entry.s prom_entry.S

prom_entry.o: prom_entry.S
	${_CC_}  -g0 ${_CCFLAGS_} ${ASFLAGS} -c prom_entry.S -o prom_entry.o

libasm.S: libasm.s
	rm -f libasm.S
	cp libasm.s libasm.S

libasm.o: libasm.S
	${_CC_}  -g0 ${_CCFLAGS_} ${ASFLAGS} -DMIPS -c libasm.S -o libasm.o

depend:
	grep '^#include' ${SRCS} | grep -v '<' | \
	sed -e 's/:[^"]*"\([^"]*\)".*/: \1/' \
	    -e 's/\.c/.o/' > xtmp 
	awk ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		       else rec = rec " " $$2 } } \
	      END { print rec } ' > makedep <xtmp
	rm -f xtmp
	echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile makefile.bak
	ed - Makefile < eddep
	rm eddep makedep

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/programs.mk

# DO NOT DELETE THIS LINE -- make depend needs it
