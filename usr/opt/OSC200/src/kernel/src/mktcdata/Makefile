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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.13.3 $ (DEC) $Date: 1993/09/22 22:16:13 $
#

PROGRAMS		= mktcdata
OTHERS			= build_inst

CCTYPE			= host

DDIR			= ../../bin

USE_STATIC_LIBRARIES	=

CFLAGS			= -DCMU -D_NO_PROTO -Dconst= 
INCFLAGS		= -I../../include -I/usr/include

HOST_LDFLAGS		= -tr -h/usr/lib/cmplrs/cc -B
LIBS			= -lln

VPATH			= ${MAKETOP}:${MAKETOP}/usr/ccs/lib/libAF
I18N_FLAG		=


include ${MAKEFILEPATH}/rules.mk

build_inst:
	@-if [ -d ${DDIR} ]; \
	then \
		cmp -s mktcdata ${DDIR}/mktcdata  || { \
			rm -f ${DDIR}/mktcdata; \
			echo "cp mktcdata ${DDIR}/mktcdata";\
			cp mktcdata ${DDIR}/mktcdata;\
		} ;\
	else \
		mkdir ${DDIR}; \
		echo "cp mktcdata ${DDIR}/mktcdata";\
		cp mktcdata ${DDIR}/mktcdata; \
	fi
