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
# 
# @(#)$RCSfile: Makefile,v $ $Revision: 4.3.17.3 $ (DEC) $Date: 1993/04/08 19:20:48 $
# 

PROGRAMS 	= load
OTHERS		= mach_boot osf_boot bootblks xxboot bootxx netload
VPATH		= ..
USE_STATIC_LIBRARIES=

load_CCTYPE	= host
load.o_CCTYPE	= host
STD_FLAG	= -std0

CFLAGS		= -DPMAX ${SEC_LEVEL?-DSEC_MAC_OB -DSEC_BASE:}
AFLAGS		= -DLOCORE -DSTANDALONE -DPMAX -DASSEMBLER
MIPS_RELOC	= 80700000
INCFLAGS	= -I.. -I${MAKETOP}usr/sbin

ILIST		= ${PROGRAMS}
IDIR		= /usr/mdec/
IOWNER		= root
IGROUP		= system

#
# COFF  - If COFF is defined then loadimage is built
#         to understand COFF file format.
#
# MACHO - If MACHO is defined then loadimage is built
#         to understand MACHO file format.
#

OBJECT_FORMAT	= COFF
_O_F_		= ${OBJECT_FORMAT}

bootxxP_OFILES	= csu.mips.o prom_entry.o libasm.o mipsboot.o io.o sys.o \
		  loadimage.o

bootxx_OFILES	= csu.mips.o prom_entry.o libasm.o mipsboot.LABELS.o \
		  io.LABELS.o sys.LABELS.o loadimage.o

mach_boot_OFILES= csu.mips.o libasm.SECONDARY.o prom_entry.o mach_boot.o \
		  sys.SECONDARY.o io.o loadimage.SECONDARY.o version.o

osf_boot_OFILES	= csu.mips.o prom_entry.o libasm.SECONDARY.o osf_boot.o \
		  sys.LSECONDARY.o io.LABELS.o loadimage.SECONDARY.o version.o

netload_OFILES	= csu.mips.o prom_entry.o loadimage.NETLOAD.o tftpboot.o \
		  libasm.SECONDARY.o netload.o version.o

sys.o_CFLAGS			= ${CFLAGS} 
sys.LABELS.o_CFLAGS		= ${CFLAGS} -DLABELS
sys.SECONDARY.o_CFLAGS		= ${CFLAGS} -D${_O_F_} -DSECONDARY
sys.LSECONDARY.o_CFLAGS		= ${CFLAGS} -D${_O_F_} -DSECONDARY -DLABELS

loadimage.o_CFLAGS		= ${CFLAGS} -D${_O_F_}
loadimage.SECONDARY.o_CFLAGS	= ${CFLAGS} -D${_O_F_} -DSECONDARY -DLOADSYMBOLS
loadimage.NETLOAD.o_CFLAGS	= ${CFLAGS} -D${_O_F_} -DNETLOAD -DLOADSYMBOLS

osf_boot.o_CFLAGS		= ${CFLAGS} -DLABELS
mach_boot.o_CFLAGS		= ${CFLAGS}  

mipsboot.LABELS.o_CFLAGS	= ${CFLAGS} -DLABELS
mipsboot.o_CFLAGS		= ${CFLAGS}  

io.LABELS.o_CFLAGS		= ${CFLAGS} -DLABELS
io.o_CFLAGS			= ${CFLAGS}  

libasm.o_AFLAGS			= ${AFLAGS}
libasm.SECONDARY.o_AFLAGS	= ${AFLAGS} -DSECONDARY

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

xxboot: bootblk.b
	uudecode bootblk.b
	mv -f bootblk $@
	ls -l $@

bootblks: xxboot bootxxP
	cat xxboot bootxxP > $@
	ls -l $@

bootxxP: ${bootxxP_OFILES} load
	${LD} -non_shared -T 80600000 -N -m -s -x ${bootxxP_OFILES}
	./load -b 512 -N15 a.out > $@
	ls -l $@

bootxx: ${bootxx_OFILES} load
	${LD} -non_shared -T 80600000 -N -m -s -x ${bootxx_OFILES}
	./load -b 512 -N15 a.out > $@
	ls -l $@

mach_boot: ${mach_boot_OFILES}
	${LD} -non_shared -N -T ${MIPS_RELOC} -o $@ ${mach_boot_OFILES}

osf_boot: ${osf_boot_OFILES}
	${LD} -non_shared -s -N -T ${MIPS_RELOC} -o $@ ${osf_boot_OFILES}

netload: ${netload_OFILES}
	${LD} -non_shared -N -T ${MIPS_RELOC} -o $@ ${netload_OFILES}

version.c: ${ALWAYS}
	echo "char *version=\""`date`"\";" > version.c

#
# I could not find a better means of getting the following 
# done, if you can think of a better way just do it!
#

libasm.SECONDARY.s: libasm.s
	${RM} -f $@
	cp libasm.s ./$@

sys.LABELS.c sys.SECONDARY.c sys.LSECONDARY.c: sys.c
	${RM} -f $@
	cp sys.c ./$@

loadimage.SECONDARY.c loadimage.NETLOAD.c: loadimage.c
	${RM} -f $@
	cp loadimage.c ./$@

osf_boot.c: mach_boot.c
	${RM} -f $@
	cp mach_boot.c ./$@

mipsboot.LABELS.c: mipsboot.c
	${RM} -f $@
	cp mipsboot.c ./$@

io.LABELS.c: io.c
	${RM} -f $@
	cp io.c ./$@

