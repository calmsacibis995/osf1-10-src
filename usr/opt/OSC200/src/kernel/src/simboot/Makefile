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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.10.5 $ (DEC) $Date: 1993/07/07 19:43:42 $
#

PROGRAMS	= sboot fboot
_BLD		= 

OTHERS		= build_inst

SOBJS		= libasm.o
COMMON_OFILES	= map.o prom.o prf.o ${SOBJS}
sboot_OFILES	= sboot.o ${COMMON_OFILES}
fboot_OFILES	= hello.o ${COMMON_OFILES}
# Don't list crt0.o in _OFILES, since the -tr -h. -B flags tell cc to
# tell the loader to get it automatically from the build directory.
# But define CRT0, so we can make sboot and fboot depend on it.
CRT0            = crt0.o
DDIR		= ../../bin
IDIR            = /isl/sas/
ILIST           = sboot

prom.o_CFLAGS	= -DSECONDARY ${CFLAGS}

CFLAGS		= -no_excpt -Wco,-nofloat

INCFLAGS	= -I. -I../../include

AFLAGS		= `genpath -I.` \
                 `genpath -I../../include` 

sboot_LDFLAGS	= -N -T 20000000 -tr -h. -B
fboot_LDFLAGS	= -N -T fffffc000004e000 -tr -h. -B

USE_STATIC_LIBRARIES = 

MIPS_LIBS		=
ALPHA_LIBS		= ${LIBDIRS} \
			-L../libc_sa -L/usr/lib -lc_sa
LIBS			= ${${TARGET_MACHINE}_LIBS}

# We don't want the standard crt0.  Don't permit default libdirs before
# crt0.o on ld line,
_BLD		=

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

build_inst: 
	@-if [ -d ${DDIR} ]; \
	then \
		cmp -s sboot ${DDIR}/sboot  || { \
			rm -f ${DDIR}/sboot; \
			echo "cp sboot ${DDIR}/sboot";\
			cp sboot ${DDIR}/sboot;\
		} ;\
	else \
		mkdir ${DDIR}; \
		echo "cp sboot ${DDIR}/sboot";\
		cp sboot ${DDIR}/sboot; \
	fi

# Create a test boot image
boot: sboot fboot
	../pmerge/pmerge sboot fboot boot

${SOBJS} crt0.o: $${@:.o=.s}
	${_CC_} ${_CCFLAGS_} -g0 -O -c $*.s

sboot: ${CRT0}
fboot: ${CRT0}

