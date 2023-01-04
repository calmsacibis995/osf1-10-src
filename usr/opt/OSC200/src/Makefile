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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.12.5 $ (DEC) $Date: 1993/05/20 18:32:07 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0


SEC_SUBDIRS		= usr etc sbin seccmd tcb
IDIR			= /
SUBDIRS			= var usr dev etc sbin isl root seccmd tcb
INSTDIRS		= dev etc home mnt opt paging sbin \
			  tmp usr tcb proc 

proc_IMODE		= 555
IMODE			= 755
IOWNER			= root
IGROUP			= system

tmp_IMODE		= 1777


SYMLINKS		= lib bin
SYMLINKDIR1		= usr
SYMLINKDIR2		= /

EXPINC_SUBDIRS		= usr
EXPLIBC_SUBDIRS		= usr
EXPLOADER_SUBDIRS	= sbin
EXPSHLIB_SUBDIRS	= usr
EXPLIB_SUBDIRS		= usr etc

include usr/lib/makefiles/standard.mk
include usr/lib/makefiles/subdirs.mk
