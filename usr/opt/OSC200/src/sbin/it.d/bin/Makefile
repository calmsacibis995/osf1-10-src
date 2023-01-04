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
# HISTORY
#
#
#	"@(#)$RCSfile: Makefile,v $ $Revision: 4.3.6.2 $ (DEC) $Date: 1993/11/13 00:08:41 $"
#
#
#	000	?		jon
#
#	001	30-may-1990	ccb
#		add VPATH to pick up a copy of doconfig - should
#			be a symbolic link
#		set ILIST to ${SCRIPTS} rather than duplicating the list
#
#	002	23-jun-1991	ccb
#		remove doconfig. this is now a symlink to /usr/sbin/docnfig
#		made in postinstall
#
#	003	30-Sep-1991	jon
#		Added getpasswd to the SCRIPTS list.
#

SCRIPTS		= gitout c_install getdate getpasswd getsysname gettimezone

ILIST		= ${SCRIPTS}
IDIR		= /sbin/it.d/bin/
SYMLINKS	= doconfig
SYMLINKDIR1	= ../../../usr/sbin
SYMLINKDIR2	= ${IDIR}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/scripts.mk

