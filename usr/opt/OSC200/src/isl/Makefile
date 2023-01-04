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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.18.13 $ (DEC) $Date: 1993/11/23 14:17:24 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
#	000	20-apr-1991	ccb
#	001	30-may-1991	ccb
#		added "data" to SUBDIRS
#		added mfpfu
#		changed /ils/ in IDIR to /isl/
#	002	03-jun-1991	ccb
#		add subdirectories for static linked version of
#		programs required for installation
#	003	05-jun-1991	map
#		add finder subdirectory
#
#	004	20-jun-1991	ccb
#		add depord, fitset, fverify, sas, tclear, tic to SUBDIRS
#		remove install.1 from DATAFILES
#
#	005	02-jul-1991
#		add sizer to subdirs


SUBDIRS			= awk cdl chgrp compress data dd depord diff\
			  disklabel egrep finder fitset fverify \
			  getopt halt hostname ifconfig init \
			  log.d merge mknod mount mt newfs pickapart pwd rcp \
			  routed rsh sas sed sizer sort stlmenu tclear tic \
			  udetect ufs_fsck umount updmore whoami date kill \
			  route

DATAFILES		= TIMEZONE .profile buildroot.osf inittab \
			  install.osf lpblocker mfpfu mrg_template RisFiles \
			  root.inv sifsync updeng updpblock.dat \
			  updpinfo updpnowarn.dat updrepack.dat

IDIR			= /isl/

ILIST			= ${DATAFILES}

updrepack.dat_SED_OPTIONS = -e '/^[ 	]*[\#:]/d'

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/subdirs.mk
