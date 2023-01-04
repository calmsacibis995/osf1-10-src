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
#	@(#)$RCSfile: Makefile,v $ $Revision: 1.1.10.3 $ (DEC) $Date: 1993/07/30 18:29:50 $
#

PROGRAMS 	= 
OTHERS		= mach_boot osf_boot bootblks xxboot bootxx netload hupdate
VPATH		= ..
USE_STATIC_LIBRARIES=

bootblock_CCTYPE	= host
hdrsize_CCTYPE	= host

STD_FLAG	= -std0

CFLAGS		= -O1 ${SEC_LEVEL?-DSEC_MAC_OB -DSEC_BASE:} \
		  -no_excpt -Wco,-nofloat -Wb,-static

AFLAGS		= -DLOCORE -DSTANDALONE -no_excpt -DASSEMBLER
ALPHA_RELOC	= 20010000
ALPHA_PRIMARY	= 20000000
INCFLAGS	= -I.. -I${MAKETOP}usr/sbin

ILIST		= bootblks bootra bootrz netload raboot rzboot osf_boot hupdate
IDIR		= /mdec/
osf_boot_IDIR	= /
hupdate_IDIR	= /
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

bootxx_OFILES	= csu.alpha.o prom.o libasm.o alphaboot.o io.o sys.o \
		  loadimage.o libc.o copy.o divide.o 

mach_boot_OFILES= csu.alpha.o prom.SECONDARY.o libasm.SECONDARY.o mach_boot.o \
		  sys.SECONDARY.o io.LABELS.o loadmappedimage.SECONDARY.o \
		  map.o copy.SECONDARY.o get.o prf.o libc.SECONDARY.o \
		  divide.SECONDARY.o loadimage.SECONDARY.o version.o

osf_boot_OFILES	= csu.alpha.SECONDARY.o prom.SECONDARY.o libasm.SECONDARY.o \
		  osf_boot.o sys.LSECONDARY.o io.LABELS.o \
		  loadmappedimage.SECONDARY.o map.o copy.SECONDARY.o get.o \
		  prf.o libc.SECONDARY.o divide.SECONDARY.o \
		  loadimage.SECONDARY.o version.o

netload_OFILES	= csu.alpha.SECONDARY.o prom.SECONDARY.o tftpboot.o \
		  loadmappedimage.NETLOAD.o \
		  map.o copy.SECONDARY.o libasm.SECONDARY.o libc.SECONDARY.o \
		  sys.o divide.SECONDARY.o netload.o io.o prf.o version.o 

hupdate_OFILES	 = csu.hupdate.o hupdate.o prom.o libasm.o prf.o \
		   libc.o divide.SECONDARY.o


prom.o_CFLAGS			= ${CFLAGS}
prom.SECONDARY.o_CFLAGS		= ${CFLAGS} -DSECONDARY

copy.o_CFLAGS			= ${CFLAGS}
copy.SECONDARY.o_CFLAGS		= ${CFLAGS} -DSECONDARY

divide.o_AFLAGS			= ${CFLAGS} -DOSF_BOOT
divide.SECONDARY.o_AFLAGS	= ${CFLAGS} -D${_O_F_} -DSECONDARY

libc.o_CFLAGS			= ${CFLAGS}
libc.SECONDARY.o_CFLAGS		= ${CFLAGS} -DSECONDARY

sys.o_CFLAGS			= ${CFLAGS} 
sys.LABELS.o_CFLAGS		= ${CFLAGS} -DLABELS
sys.SECONDARY.o_CFLAGS		= ${CFLAGS} -D${_O_F_} -DSECONDARY
sys.LSECONDARY.o_CFLAGS		= ${CFLAGS} -D${_O_F_} -DSECONDARY -DLABELS

loadimage.o_CFLAGS		   = ${CFLAGS} -D${_O_F_}
loadimage.SECONDARY.o_CFLAGS	= ${CFLAGS} -D${_O_F_} -DSECONDARY

loadmappedimage.o_CFLAGS	   = ${CFLAGS} -D${_O_F_}
loadmappedimage.SECONDARY.o_CFLAGS = ${CFLAGS} -D${_O_F_} \
					-DSECONDARY -DLOADSYMBOLS
loadmappedimage.NETLOAD.o_CFLAGS   = ${CFLAGS} -D${_O_F_} -DNETLOAD -DLOADSYMBOLS

osf_boot.o_CFLAGS		= ${CFLAGS} -DLABELS
mach_boot.o_CFLAGS		= ${CFLAGS}  

alphaboot.LABELS.o_CFLAGS	= ${CFLAGS} -DLABELS
alphaboot.o_CFLAGS		= ${CFLAGS}  

io.LABELS.o_CFLAGS		= ${CFLAGS} -DLABELS
io.o_CFLAGS			= ${CFLAGS}  

libasm.o_AFLAGS			= ${AFLAGS}
libasm.SECONDARY.o_AFLAGS	= ${AFLAGS} -DSECONDARY

csu.alpha.o_AFLAGS		= ${CFLAGS} ${AFLAGS}
csu.alpha.SECONDARY.o_AFLAGS	= ${CFLAGS} ${AFLAGS} -DSECONDARY

hupdate.o_CFLAGS		= ${CFLAGS}

csu.hupdate.o_AFLAGS		= ${CFLAGS} ${AFLAGS}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

bootblks: xxboot bootxx
	cat xxboot bootxx > $@
	ls -l $@

xxboot: bootblock
	./bootblock
	cp xxboot raboot
	cp xxboot rzboot	

bootxx: ${bootxx_OFILES} hdrsize
	${LD} -non_shared -T ${ALPHA_PRIMARY} -N -x -s ${bootxx_OFILES}
	@echo "Calculating a.out header size.. `./hdrsize a.out`"
	dd if=a.out of=$@ ibs=`./hdrsize a.out` skip=1 2> /dev/null
	@echo "Testing that bootxx fits in 15 blocks..."
	@test `ls -lg bootxx | sed -e 's/  */	/g' | cut -f5` -le 7680
	@echo "bootxx bsize OK"
	rm a.out
	cp bootxx bootra
	cp bootxx bootrz


mach_boot: ${mach_boot_OFILES}
	${LD} -non_shared -N -T ${ALPHA_RELOC} -o $@ ${mach_boot_OFILES}

osf_boot: ${osf_boot_OFILES}
	${LD} -non_shared -s -N -T ${ALPHA_RELOC} -o $@ ${osf_boot_OFILES}

netload: ${netload_OFILES} hdrsize
	${LD} -non_shared -N -T ${ALPHA_PRIMARY} -x -s ${netload_OFILES}
	@echo "Calculating a.out header size.. `./hdrsize a.out`"
	dd if=a.out of=$@ ibs=`./hdrsize a.out` skip=1 2> /dev/null
	rm a.out

version.c: ${ALWAYS}
	echo "char *version=\""`date`"\";" > version.c

#
# hupdate
#
# Standalone program that determines if hvmunix should be the
# default installation path
#
# The program is loaded into the primary bootstrap address
#
# Note that divide.SECONDARY.o gets included instead of divide.o
# because prf.o depends on the unsigned versions not included
# in divide.o
#
hupdate: ${hupdate_OFILES}
	${_LD_} -non_shared -N -T ${ALPHA_PRIMARY} -e start -o $@ \
		${hupdate_OFILES}

#
# I could not find a better means of getting the following 
# done, if you can think of a better way just do it!
#

libasm.SECONDARY.s: libasm.s
	${RM} -f $@
	cp libasm.s ./$@

csu.alpha.SECONDARY.s: csu.alpha.s
	${RM} -f $@
	cp csu.alpha.s ./$@

prom.SECONDARY.c: prom.c
	${RM} -f $@
	cp prom.c ./$@

copy.SECONDARY.c: copy.c
	${RM} -f $@
	cp copy.c ./$@

divide.SECONDARY.s: divide.s
	${RM} -f $@
	cp divide.s ./$@

libc.SECONDARY.c: libc.c
	${RM} -f $@
	cp libc.c ./$@

sys.LABELS.c sys.SECONDARY.c sys.LSECONDARY.c: sys.c
	${RM} -f $@
	cp sys.c ./$@

loadimage.SECONDARY.c loadimage.NETLOAD.c: loadimage.c
	${RM} -f $@
	cp loadimage.c ./$@

loadmappedimage.SECONDARY.c loadmappedimage.NETLOAD.c: loadmappedimage.c
	${RM} -f $@
	cp loadmappedimage.c ./$@

osf_boot.c: mach_boot.c
	${RM} -f $@
	cp mach_boot.c ./$@

alphaboot.LABELS.c: alphaboot.c
	${RM} -f $@
	cp alphaboot.c ./$@

io.LABELS.c: io.c
	${RM} -f $@
	cp io.c ./$@

netload: hdrsize

hdrsize: hdrsize.c
	${_CC_} ${_CCFLAGS_} -P hdrsize.c
	mv hdrsize.i nhdrsize.c
	${_CC_} -o hdrsize nhdrsize.c -lmld

bootblock: bootblock.c
	${_CC_} -o bootblock -non_shared bootblock.c


