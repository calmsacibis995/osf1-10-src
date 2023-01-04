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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.3.7.2 $ (DEC) $Date: 1993/06/10 19:26:41 $
#
#
# HISTORY
#
# OSF/1 Release 1.0

# Maintenance History
#
# 001	gws
#	added WARN_FLAG= to turn ON compiler warnings (disables -w flag)
#	added CFLAGS = -Olimit 1074, because compiler warnings (after -w
#	  disabled) said tput wasn't getting optimized because a subroutine
#	  got too big; -Olimit 1074 required to permit optimization
#

PROGRAMS		= tput
MSGHDRS			= tput_msg.h
CATFILES		= tput.cat

ILIST			= tput
IDIR			= /usr/bin/

# Added to turn on compiler warnings
WARN_FLAG=

CFLAGS			= -Olimit 1074
HFILES			= ${MSGHDRS}
LIBS			= -lcurses

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
