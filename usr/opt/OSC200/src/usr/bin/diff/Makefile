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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/10 18:13:17 $
#
#
# HISTORY
#
# OSF/1 Release 1.0

PROGRAMS		= diff diffh
MSGHDRS			= diff_msg.h
CATFILES		= diff.cat

ILIST			= diff diffh
IDIR			= /usr/bin/
LIB_IDIR		= /usr/lbin/
diffh_IDIR		= ${LIB_IDIR}

DIFF			= ${IDIR}diff
DIFFH			= ${LIB_IDIR}diffh
PR			= ${IDIR}pr

CFLAGS			= -D${OBJECT_FORMAT}
diff.o_CFLAGS		= -DDIFF='"${DIFF}"' -DDIFFH='"${DIFFH}"'\
			  -DPR='"${PR}"' ${CFLAGS}
HFILES			= diff_msg.h diff.h
diff_OFILES		= diff.o diffdir.o diffreg.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
