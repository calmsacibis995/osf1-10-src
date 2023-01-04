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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.13.6 $ (DEC) $Date: 1994/01/11 19:57:57 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

MACHO_SUBDIRS		= ar nm odump size strings strip
COFF_SUBDIRS		= ar as btou cfe cord dbx dis driver \
			  file ld nm odump pixie ppu prof rld size stdump \
			  strings strip ugen ujoin umerge uopt \
			  upas usplit utob
SUBDIRS			= cb cflow ctags cxref dbx error flex gprof indent \
			  lex lint lorder m4 make mig mkstr \
			  ranlib tsort xstr yacc gem \
			  ${${OBJECT_FORMAT}_SUBDIRS}

SYMLINKS	= file cmplrs cfe ld
cfe_SYMLINKDIR1 = .
cfe_SYMLINKDIR2 = /usr/ccs/lib/cmplrs/cc
cfe_SYMLINK = cpp
ld_SYMLINKDIR1 = .
ld_SYMLINKDIR2 = /usr/ccs/lib/cmplrs/cc
ld_SYMLINK = uld
cmplrs_SYMLINKDIR1 = ../ccs/lib
cmplrs_SYMLINKDIR2 = /usr/lib
SYMLINKDIR1	= ../ccs/bin
SYMLINKDIR2	= /usr/bin


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
