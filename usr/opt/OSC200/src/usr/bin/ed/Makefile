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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.3.9.2 $ (DEC) $Date: 1993/06/10 19:14:06 $
#
#
# HISTORY
#
# OSF/1 Release 1.0
# Modification History:
# 01 Piyanai (7/30/91)  Separation of crypt/encrypt at source level.
#
# 02 Gray (11/13/91)    Added -DPDA define to change maximum file lengths
#                       from 64 to MAXPATH+1 (1025)

PROGRAMS		= ed
MSGHDRS			= ed_msg.h
CATFILES		= ed.cat

ILIST			= ed
IDIR			= /usr/bin/
ILINKS			= ${IDIR}e ${IDIR}red

CCTYPE			= writable_strings
HFILES			= pathnames.h ${MSGHDRS}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
