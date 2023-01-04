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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:15:58 $
#
# /usr/ccs/lib/libmir/Makefile
# Makefile for producing libmir.a

LIBRARIES	= libmir.a
EXPLIB_TARGETS	= export_libmir.a
EXPDIR		= /usr/ccs/lib/

USE_STATIC_LIBRARIES=
CFLAGS		= -std1 -D_XOPEN_SOURCE
INCFLAGS	= -I. -I${MAKETOP}../../export/${target_machine}/usr/include/eca

CLEANFILES      = *.a *.o
HFILES		= ${MSGHDRS} mir_goi.cinc
OFILES		= mir_intercmp.o mir_interdmp.o mir_symdump.o mir_t0.o mir_t1.o

include ${MAKEFILEPATH}/rules.mk
