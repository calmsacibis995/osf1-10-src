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
# 	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/28 23:06:02 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

DATAFILES		= yp_prot.h ypclnt.h yppasswd.h ypupdate_prot.h \
			  ypv1_prot.h mount.h ypsym.h ypdefs.h ypxfrd.h \
			  bootparam.h ether.h ipalloc.h pnprpc.h pwdnm.h \
			  rex.h rquota.h rstat.h rusers.h rwall.h showfh.h \
			  sm_inter.h spray.h

INCLUDES		= ${DATAFILES}

ILIST			= ${INCLUDES}
IDIR			= /usr/include/rpcsvc/
IMODE			= 444

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
