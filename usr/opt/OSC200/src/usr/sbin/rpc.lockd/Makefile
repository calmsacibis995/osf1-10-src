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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/15 19:03:08 $
#
# Makefile for rpc.lockd - the network lock manager

PROGRAMS 		= rpc.lockd

ILIST			= rpc.lockd
IDIR			= /usr/sbin/

LIBS			= -lrpcsvc

OFILES			= prot_main.o prot_lock.o prot_pklm.o \
			  prot_pnlm.o prot_msg.o \
			  prot_proc.o prot_libr.o prot_alloc.o prot_priv.o \
			  tcp.o udp.o xdr_nlm.o xdr_klm.o sm_monitor.o \
			  prot_share.o prot_freeall.o hash.o svc_udp.o \
			  ufs_lockf.o

HFILES			= prot_lock.h prot_time.h sm_res.h priv_prot.h \
			  lockf.h klm_prot.h \
			  nlm_prot.h lockmgr.h sm_inter.h

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
