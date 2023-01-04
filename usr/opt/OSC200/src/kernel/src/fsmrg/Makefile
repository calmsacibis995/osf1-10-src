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
# Makefile to build test programs
#
#

MIPS_INCFLAGS	= -I -I../../include -I../../include/sysV
ALPHA_INCFLAGS	= -I../../../usr/include -I../../include
INCFLAGS	= ${${TARGET_MACHINE}_INCFLAGS}

CCTYPE		= host

OTHERS	= fsmrg
DDIR	= ../../bin
IDIR	= /isl/sas/
ILIST	= fsmrg

#INCLUDES= -I -I/usr/projects/alpha/wp/lib/_alpha.d/include \
#	    -I/usr/projects/alpha/wp/lib/_alpha.d/hostmips64/usr/include
MIPS_INCLUDES = -I -I../../include -I../../include/sysV
ALPHA_INCLUDES = -I../../../usr/include -I../../include

COPTS_P	= ${CDEBUGOPTS} ${DEPENDS} ${ALLOPTS} ${GCC_OPTS}
COPTS	= ${COPTS_P} ${${@}_MIG?-Imach:}
CFLAGS	= ${COPTS} ${CPP_ONLY}

#
#  Common assembly flags
#
SFLAGS	= ${DEPENDS} ${ALLOPTS} ${GCC_OPTS} -DASSEMBLER
ASFLAGS	= -c ${INCLUDES}

MIPS_LIBS = -L -L/usr/lib/cmplrs/mcc -lmld -lc
ALPHA_LIBS = -L/usr/ccs/lib -lmld

LIBS	= ${${TARGET_MACHINE}_LIBS}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/others.mk

MIPS_CC_LINE	= ${_CC_} -o fsmrg fsmrg.c ${LIBS}
ALPHA_CC_LINE	= ${_CC_} -non_shared -o fsmrg fsmrg.c ${LIBS}

fsmrg: fsmrg.c
	${${TARGET_MACHINE}_CC_LINE}
	@-if [ -d ${DDIR} ]; \
	then \
		cmp -s fsmrg ${DDIR}/fsmrg  || { \
			rm -f ${DDIR}/fsmrg; \
			echo "cp fsmrg ${DDIR}/fsmrg";\
			cp fsmrg ${DDIR}/fsmrg;\
		} ;\
	else \
		mkdir ${DDIR}; \
		echo "cp fsmrg ${DDIR}/fsmrg";\
		cp fsmrg ${DDIR}/fsmrg; \
	fi
