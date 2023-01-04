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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.4.4.4 $ (DEC) $Date: 1992/07/08 16:37:19 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#

VPATH			= ${MAKETOP}/kernel/vm

DATAFILES		= vm_map.h vm_kern.h pmap.h heap_kmem.h \
			u_mape_dev.h vm_anon.h vm_anonpage.h vm_control.h \
			vm_fault.h vm_kmap.h vm_lock.h vm_object.h vm_page.h \
			vm_pagelru.h vm_swap.h vm_tune.h vm_umap.h vm_vlock.h \
			vm_vp.h vm_vppage.h vpage.h vmparam.h vm_user.h \
			vm_mmap.h vm_debug.h vm_perf.h u_mape_seg.h
ILIST   = ${DATAFILES}
IDIR    = /usr/sys/include/vm/
IMODE   = 444
IGROUP  = bin
IOWNER  = bin


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk

