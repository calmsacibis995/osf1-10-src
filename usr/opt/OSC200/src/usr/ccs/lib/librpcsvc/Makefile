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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:59:49 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

LIBRARIES		= librpcsvc.a

EXPLIB_TARGETS		= export_librpcsvc.a

ILIST			= librpcsvc.a
IDIR			= /usr/ccs/lib/
IMODE			= 644

OFILES			= bootparam_xdr.o etherxdr.o ipalloc.o ipalloc_clnt.o \
			  mountxdr.o pnprpc.o pnprpc_clnt.o sm_inter.o \
			  rex_xdr.o rquotaxdr.o rstatxdr.o rusersxdr.o \
			  rwallxdr.o showfh_xdr.o sprayxdr.o yppasswdxdr.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
