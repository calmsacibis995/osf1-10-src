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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.18.5 $ (DEC) $Date: 1993/10/29 22:00:11 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#

SEC_SUBDIRS		= bin share sbin
SUBDIRS			= include ccs ${NO_SHARED_LIBRARIES?:shlib} \
			  lib lbin share bin sbin local mdec s5 field skel \
                          examples sys ultrix 32bit opt

INSTDIRS		= include ccs ${NO_SHARED_LIBRARIES?:shlib} \
			  lib lbin share bin sbin local field skel \
                          examples adm 32bit opt

IDIR			= /usr/
IMODE			= 755
IGROUP			= system
IOWNER			= root
lib_IMODE		= 775
adm_IDIR		= /var/
adm_IOWNER		= adm
adm_IGROUP		= adm

EXPINC_SUBDIRS		= include s5 shlib 32bit
EXPLIBC_SUBDIRS		= ccs
EXPSHLIB_SUBDIRS	= shlib
EXPLIB_SUBDIRS		= ccs

SYMLINKS		= bin adm news preserve spool tmp
SYMLINKDIR1		= ../var
SYMLINKDIR2		= /usr
bin_SYMLINKDIR1		= .
bin_SYMLINK		= ucb

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
