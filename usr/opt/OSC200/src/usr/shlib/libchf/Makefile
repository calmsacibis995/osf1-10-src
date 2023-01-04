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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.8.2 $ (DEC) $Date: 1993/11/11 17:07:36 $
#
# (c) COPYRIGHT 1991, DIGITAL EQUIPMENT CORPORATION
#	ALL RIGHTS RESERVED
#
#
# Makefile for shareable DAS CHF -- libchf.so
#
#	Ray Giroux	2/28/92
#
VPATH			= ${MAKETOP}usr/ccs/lib/libchf
LIBCHF			= ${MAKETOP}usr/ccs/lib/libchf

SHARED_LIBRARIES	= libchf.so

USE_SHARED_LIBRARIES	=

EXPSHLIB_TARGETS	= export_libchf.so
LDFLAGS			= -L
# EXPORTS			= -package libchf
LIBS			= -lc -lm

ILIST			= libchf.so
IDIR			= /usr/shlib/
IMODE			= 644
NOSTRIP			=

INCFLAGS                = -I${LIBCHF}
OFILES                  = ${OBJECTS}

CFLAGS 			= -DDAS_EXPAND_PROTO=1

OBJECTS                 = chf_osf.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/compiler.mk
