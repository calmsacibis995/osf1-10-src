# 
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
#	@(#)$RCSfile: Makefile,v $ $Revision: 1.2.18.4 $ (DEC) $Date: 1993/10/19 20:16:22 $

mips_SUBDIRS	= bi ci dti fbus fdi gvp lsb mbox msi \
		  netif np scs scsi sysap tc \
		  uba ws xmi

alpha_SUBDIRS	= bi ci dti fbus fdi gvp lsb mbox msi \
		  netif np scs scsi sysap tc \
		  uba ws xmi isa eisa

SUBDIRS 	= ${${target_machine}_SUBDIRS}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
