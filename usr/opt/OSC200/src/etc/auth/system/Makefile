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
# 
# @(#)$RCSfile: Makefile,v $ $Revision: 4.1.10.2 $ (DEC) $Date: 1993/04/01 19:55:32 $
# 
# 
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
# Makefile for the etc/auth/system directory
#



FILES		= authorize default devassign files lps subsystems ttys
OTHERS_		= authorize default devassign files subsystems ttys
OTHERS_BASE	= ${OTHERS_}
OTHERS_MAC	= lps
SEC_OTHERS	= ${FILES}
OTHERS		= ${OTHERS_${SEC_LEVEL}}
SEC_SLEVEL	= ${SEC_LEVEL?${SEC_LEVEL}:C2}


SEC_ILIST	= ${SEC_OTHERS}
ILIST		= ${OTHERS}
IDIR		= /etc/auth/system/
IMODE		= 664
IOWNER		= auth
IGROUP		= auth

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/others.mk

${OTHERS}: $${@:=.${SEC_SLEVEL}}
	${RM} ${_RMFLAGS_} $@
	${CP} ${@:=.${SEC_SLEVEL}} $@

