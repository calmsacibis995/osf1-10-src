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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/05/26 17:46:27 $
#

PROGRAMS                = rpc.pcnfsd

ILIST                   = rpc.pcnfsd
IDIR                    = /usr/sbin/

CFLAGS			= -DSIA -DOSVER_BSD386
OFILES			= pcnfsd_svc.o pcnfsd_xdr.o pcnfsd_v1.o \
			pcnfsd_v2.o pcnfsd_misc.o pcnfsd_cache.o \
			pcnfsd_print.o
HFILES			= pcnfsd.h common.h
LIBS                    = -lrpcsvc

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
