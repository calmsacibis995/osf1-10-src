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
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# @(#)$RCSfile: Makefile,v $ $Revision: 4.3.10.4 $ (DEC) $Date: 1994/01/31 18:42:47 $
#
#
# HISTORY
#
VERSION			= 3.7

SEC_PROGRAMS		= expreserve
PROGRAMS		= ex exrecover expreserve
MSGHDRS			= ex_msg.h
CATFILES		= ex.cat

SEC_ILIST		= ${SEC_PROGRAMS}
ILIST			= ${PROGRAMS}
IDIR			= /usr/bin/
LIB_IDIR		= /usr/lbin/
ex_ILINKS		= ${IDIR}edit ${IDIR}e \
			  ${IDIR}vi ${IDIR}view \
			  ${IDIR}vedit
ex_IMODE		= 1755
exrecover_IOWNER	= root
exrecover_IMODE		= 4711
exrecover_IDIR		= ${LIB_IDIR}
exrecover_ILINKS	= ${LIB_IDIR}ex${VERSION}recover
expreserve_IOWNER	= root
expreserve_IMODE	= 4711
expreserve_IDIR		= ${LIB_IDIR}
expreserve_ILINKS	= ${LIB_IDIR}ex${VERSION}preserve

CFLAGS			= -DLISPCODE -DVMUNIX -UVFORK -DCHDIR -Dalloca=__builtin_alloca \
			  -DSTDIO -DUSG -DFLOCKFILE -DOSF_MMAP \
			  -DEXPRESERVE=\"${LIB_IDIR}ex${VERSION}preserve\" \
			  -DEXRECOVER=\"${LIB_IDIR}ex${VERSION}recover\" \
			  -DUNIX_SBRK -DVERSION=${VERSION} ${SEC_DEFS}
ex_cmds.o_CFLAGS	= ${CFLAGS} -Olimit 800
ex_OFILES		= ex.o ex_addr.o ex_cmds.o ex_cmds2.o \
			  ex_cmdsub.o ex_data.o ex_extern.o \
			  ex_get.o ex_io.o ex_put.o ex_re.o \
			  ex_set.o ex_subr.o \
			  ex_temp.o ex_tty.o ex_unix.o ex_v.o \
			  ex_vadj.o ex_vget.o ex_vmain.o \
			  ex_voper.o ex_vops.o ex_vops2.o \
			  ex_vops3.o ex_vput.o ex_vwind.o \
			  printf.o
ex_GARBAGE              = ex_vars.h
ex_LINTFILES		= ${ex_OFILES:.o=.c}
exrecover_OFILES	= exrecover.o
expreserve_OFILES	= expreserve.o ${SEC_DEFS?expre_sec.o:}
expreserve.o_LIBS	= ${SEC_LIBS}
expre_sec.o_LIBS	= ${SEC_LIBS}
expreserve_LIBS         = ${SEC_LIBS}
ex_HFILES		= ex.h ex_argv.h ex_re.h ex_temp.h ex_tty.h ex_tune.h \
			  ex_vars.h ex_vis.h ex_msg.h
ex_LIBS			= -lcurses

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

${ex_OFILES}: ${ex_HFILES}

expreserve.o: ex_msg.h
expre_sec.o:  ex_msg.h

ex_vars.h: makeoptions ex_data.c
	_CC_="${_CC_}" sh makeoptions ex_data.c ${_CCFLAGS_}

${ctags_OFILES}: ex_msg.h

exrecover.o: ex_msg.h ex.h ex_temp.h ex_tty.h
