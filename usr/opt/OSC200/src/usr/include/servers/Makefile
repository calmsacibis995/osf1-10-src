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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/28 23:06:21 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

MIG_DEFS		= env_mgr.defs ipcx.defs netmemory.defs \
			  netmemstat.defs netname.defs service.defs
MIG_HDRS		= ${MIG_DEFS:.defs=.h}
DATAFILES		= emdefs.h errorlib.h ipcx_types.h \
			  netmemory_defs.h netname_defs.h \
			  ${MIG_HDRS} ${MIG_DEFS}
INCLUDES		= ${DATAFILES}

ILIST			= ${INCLUDES}
IDIR			= /usr/include/servers/
IMODE			= 444

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/objects.mk

${MIG_HDRS}: $${@:.h=.defs}
