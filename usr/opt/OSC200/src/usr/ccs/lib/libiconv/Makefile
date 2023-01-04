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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 00:38:33 $
#
#
LIBRARIES	= libiconv.a
EXPLIB_TARGETS	= export_libiconv.a

ILIST		= libiconv.a
IDIR		= /usr/ccs/lib/
EXPDIR		= /usr/ccs/lib/
IMODE		= 644
NOSTRIP		=

#
# The folowing is included by the ./usr/shlib/libiconv/Makefile
# This keeps things consistant.
#
OFILES		= iconv.o ascii.o fcs.o 
HFILES		= iconvTable.h iconv932.h iconvP.h iconv_local.h

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk

fcs.o: fcs.h
