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
#
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:14:33 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# Mach Operating System
# Copyright (c) 1990 Carnegie-Mellon University
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#
#
# OSF/1 Release 1.0

#
#			INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied under the terms of a license  agreement or 
#	nondisclosure agreement with Intel Corporation and may not be copied 
#	nor disclosed except in accordance with the terms of that agreement.
#
#	Copyright 1988 Intel Corporation
# Copyright 1988, 1989 by Intel Corporation

CFLAGS	= -O $(MORECPP)
INC = -I. -I../..
MAKE = make
AS = as
LD = ld
LIBS= -lc

# The ordering is important; start.o should be the first
# and table.o be the second.
OBJS = start.o table.o boot2.o boot.o prf.o asm.o bios.o console.o \
	disk.o sys.o misc.o

boots: fdboot hdboot

all: allfloppy hdboot

# floppy boot loader
fdboot:
# 	fboot can boot a.out
	$(MAKE) CONFIG=fd boot

# hard disk boot loader, capable of loading anything
hdboot:
	$(MAKE) CONFIG=hd boot

# floppy boot loader
allfloppy:
# 	fboot can boot a.out
	$(MAKE) CONFIG=fd boot
# 	fboot.coff can boot coff
	$(MAKE) CONFIG=fcoff boot
# 	fboot .all can boot anything, but is bigger than 8k
	$(MAKE) CONFIG=fall boot

config.h: $(CONFIG)


boot: $(CONFIG) rmaouthdr boot.sed $(OBJS)
	@ if [ X$(CONFIG) = X ] ; \
		then echo No CONFIG specified; exit 1; \
		else exit 0; \
	fi
	$(LD) -N -T 0 -o boot $(OBJS) $(LIBS)
	cp boot boot.$(CONFIG).x
	-strip boot
	./rmaouthdr boot boot.tmp
	mv -f boot.tmp boot.$(CONFIG)
	ls -l boot.$(CONFIG)


# configurations

bios.s boot.c disk.c prf.c sys.c: small.h
boot.c: boot.h
start.s: floppy.h

CHECKBOOT = \
	if cmp boot.tmp boot.h >/dev/null;  \
		then rm -f boot.tmp; \
		else mv -f boot.tmp boot.h; \
	fi

CHECKSMALL = \
	if cmp small.tmp small.h >/dev/null;  \
		then rm -f small.tmp; \
		else mv -f small.tmp small.h; \
	fi

CHECKFLOPPY = \
	if cmp floppy.tmp floppy.h >/dev/null;  \
		then rm -f floppy.tmp; \
		else mv -f floppy.tmp floppy.h; \
	fi

hd:
	@ echo configuring for Hard Disk
	@ echo "#define	AOUT 1" > boot.tmp; \
	echo "#define	COFF 1" >> boot.tmp; \
	$(CHECKBOOT); \
	echo "" >small.tmp; \
	$(CHECKSMALL); \
	echo "" > floppy.tmp; \
	$(CHECKFLOPPY)

f.all:
	@ echo configuring for Floppy
	@ echo "#define	SMALL" >small.tmp; \
	$(CHECKSMALL); \
	echo "#define	FLOPPY" >floppy.tmp; \
	$(CHECKFLOPPY)

fd: f.all
	@ echo configuring for a.out
	@ echo "#define	AOUT 1" > boot.tmp; \
	echo "#define	COFF 0" >> boot.tmp; \
	$(CHECKBOOT)

fcoff: f.all
	@ echo configuring for coff
	@ echo "#define	AOUT 0" > boot.tmp; \
	echo "#define	COFF 1" >> boot.tmp; \
	$(CHECKBOOT)	

fall:
	@ echo configuring for Floppy
	@ echo "" >small.tmp; \
	$(CHECKSMALL); \
	echo "#define	FLOPPY" >floppy.tmp; \
	$(CHECKFLOPPY)
	@ echo configuring for either a.out or coff
	@ echo "#define	AOUT 1" > boot.tmp; \
	echo "#define	COFF 1" >> boot.tmp; \
	$(CHECKBOOT)


clean:
	/bin/rm -f *.o boot.hd* boot.fd* boot.fall* boot.fcoff*

rmcoffhdr: rmcoffhdr.c
	$(CC) $? -o $@

rmaouthdr: rmaouthdr.csh
	cp rmaouthdr.csh rmaouthdr
	chmod +x rmaouthdr

.SUFFIXES: .s .i .c .o

.c.o:
	@echo $(CC) -c $(CFLAGS) $(INC) $*.c $<
	-@trap "/bin/rm -f $*.i $*.s; exit 0" 0 1 2 3 10 15; \
	$(CC) -MD $(CFLAGS) $(INC) -S $<;	\
	if [ $$? != 0 ]; then :; else \
		sed -f boot.sed $*.s > $*.i;	\
		$(AS) $*.i -o $@; \
	fi

.s.o:
	@echo $(AS) -o $*.o $<
	-@trap "/bin/rm -f $*.i X$*.c; exit 0" 0 1 2 3 10 15; \
	/bin/rm -f X$*.c;	\
	ln $*.s X$*.c;	\
	$(CC) -E $(CFLAGS) X$*.c > $*.i;	\
	if [ $$? != 0 ]; then :;	\
	else \
		$(AS) $*.i -o $@;	\
	fi

#.s.i:
#	/bin/rm -f $*.c
#	ln $*.s X$*.c
#	$(CC) -E $(CFLAGS) X$*.c > $*.i
#	/bin/rm -f X$*.c

# Dependencies for File: Xasm.o:
Xasm.o: DEFS.h Xasm.c
# Dependencies for File: Xbios.o:
Xbios.o: DEFS.h Xbios.c small.h
# Dependencies for File: Xboot2.o:
Xboot2.o: DEFS.h Xboot2.c
# Dependencies for File: Xstart.o:
Xstart.o: DEFS.h Xstart.c floppy.h
# Dependencies for File: boot.o:
boot.o: ../../mach/machine/vm_types.h ../../machine/machparam.h
boot.o: ../../sys/exec.h ../../sys/inode.h ../../sys/param.h ../../sys/reboot.h
boot.o: ../../sys/signal.h ../../sys/types.h /usr/include/a.out.h boot.c boot.h
boot.o: coff.h saio.h small.h
# Dependencies for File: console.o:
console.o: console.c
# Dependencies for File: disk.o:
disk.o: ../../i386at/disk.h ../../mach/machine/vm_types.h
disk.o: ../../machine/machparam.h ../../sys/inode.h ../../sys/param.h
disk.o: ../../sys/signal.h ../../sys/types.h disk.c saio.h small.h
# Dependencies for File: misc.o:
misc.o: misc.c
# Dependencies for File: prf.o:
prf.o: ../../mach/machine/vm_types.h ../../machine/machparam.h
prf.o: ../../sys/param.h ../../sys/signal.h ../../sys/types.h prf.c small.h
# Dependencies for File: sys.o:
sys.o: ../../mach/machine/vm_types.h ../../machine/machparam.h ../../sys/dir.h
sys.o: ../../sys/fs.h ../../sys/inode.h ../../sys/param.h ../../sys/reboot.h
sys.o: ../../sys/signal.h ../../sys/types.h saio.h small.h sys.c
# Dependencies for File: table.o:
table.o: table.c
