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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.12.2 $ (DEC) $Date: 1993/04/01 20:28:59 $
#
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
# Modification History
# --------------------
# 001	hsu		91/03/21
#     - add subdir shell
# 002	gws		91/06/06
#     - Added subdir term for nroff device drivers
#     - Added this Modification History for this and previous changes
# 003	jon		91/06/26
#     - Added timezone directories
#

SEC_SUBDIRS		= sechelp
SUBDIRS			= ${SEC_SUBDIRS} \
			  kernel_options sccshelp shell term terminfo timezone

IDIR			= /usr/share/lib/

SYMLINKS		= font lib.b me more.help ms \
			  term terminfo termcap tmac man tabset

SYMLINKDIR1		= ../share/lib
SYMLINKDIR2		= /usr/lib
termcap_SYMLINKDIR1	= ../usr/share/lib
termcap_SYMLINKDIR2	= /etc
refer_SYMLINKDIR1	= ../lbin
man_SYMLINKDIR1		= share
man_SYMLINKDIR2		= /usr

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/datafiles.mk
