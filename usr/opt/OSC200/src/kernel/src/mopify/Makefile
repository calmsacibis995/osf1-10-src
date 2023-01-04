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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.13.4 $ (DEC) $Date: 1993/07/07 19:43:37 $
#
# Makefile to build test programs
#

CCTYPE		= host
MIPS_INCFLAGS	= -I../../include -I../../include/sysV
ALPHA_INCFLAGS	= -I../../include
INCFLAGS	= ${${TARGET_MACHINE}_INCFLAGS}
USE_STATIC_LIBRARIES	=

PROGRAMS	= mopify

OTHERS		= build_inst

DDIR    	= ../../bin
IDIR    	= /isl/sas/

ILIST   	= mopify

MIPS_LIBS = -lmld -lc
ALPHA_LIBS = -lmld

LIBS	= ${${TARGET_MACHINE}_LIBS}


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

build_inst: 
	@-if [ -d ${DDIR} ]; \
	then \
		cmp -s mopify ${DDIR}/mopify  || { \
			rm -f ${DDIR}/mopify; \
			echo "cp mopify ${DDIR}/mopify";\
			cp mopify ${DDIR}/mopify;\
		} ;\
	else \
		mkdir ${DDIR}; \
		echo "cp mopify ${DDIR}/mopify";\
		cp mopify ${DDIR}/mopify; \
	fi
