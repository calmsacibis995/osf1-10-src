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
# HISTORY
#
#
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.16.8 $ (DEC) $Date: 1993/10/29 20:35:54 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

SEC_SUBDIRS		= arp chown clri date fddisk fsdb fsck halt hostid \
			  hostname ifconfig init kill killall mknod mount \
			  named newfs ping ps quotacheck \
			  reboot route savecore disklabel shutdown swapon tar \
			  ufs_fsck umount mount_fdfs
BENCH_SUBDIRS		= find diff cpio sort grep make

INSTDIRS		= subsys
SYMLINKS		= kloadsrv 
SYMLINKDIR1		= init.d
SYMLINKDIR2		= /sbin

MIPS_SUBDIRS		= cfgmgr loader
MMAX_SUBDIRS		= cfgmgr loader
ALPHA_SUBDIRS		= cfgmgr loader
AT386_SUBDIRS		= 
SUBDIRS			= ${SEC_SUBDIRS} upgrade kreg ${BENCH_SUBDIRS} \
			  cat chmod cp dd df dump ed expr \
			  init.d installupdate it it.d itruns ln ls lvm \
			  mkdir memlog mv rm restore \
			  s5fsck s5mkfs sh sleep stty sync tunefs update who \
			  radisk scu kmknod upgrade kreg ${${TARGET_MACHINE}_SUBDIRS} 
DATAFILES		= bcheckrc rc0 rc2 rc3 xdm.init snooze

MIPS_EXPLOADER_SUBDIRS	= loader
ALPHA_EXPLOADER_SUBDIRS	= loader

EXPLOADER_SUBDIRS	=  ${${TARGET_MACHINE}_EXPLOADER_SUBDIRS}

ILIST			= ${DATAFILES}
IDIR			= /sbin/

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/datafiles.mk






