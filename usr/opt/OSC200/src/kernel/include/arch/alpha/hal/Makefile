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

VPATH	= ${MAKETOP}/kernel/arch/${MACHINE}/hal

DATAFILES	= cpuconf.h cons_sw.h hal_sysinfo.h lmf_smm.h \
		  adudefs.h ka_adu.h kn430.h kn15aa.h kn7aa.h \
		  kn16aa.h dc21064.h iic.h kn121.h isp.h mc146818clock.h \
		  82357_pic.h

SYMLINKS	= ${DATAFILES}
SYMLINKDIR1	= hal
SYMLINKDIR2	= /usr/sys/include/arch/alpha

ILIST	= ${DATAFILES}
IDIR	= /usr/sys/include/arch/alpha/hal/
IMODE	= 444
IGROUP	= bin
IOWNER	= bin

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/subdirs.mk
