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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 16:14:04 $
#
# /usr/ccs/lib/libmoss/Makefile
# Makefile for producing and exporting libmoss.a
# Issues:
#	i18n
#	-std1 produces errors
# /usr/ccs/lib/libmoss/Makefile

LIBRARIES	= libmoss.a
EXPLIB_TARGETS	= export_libmoss.a
EXPDIR		= /usr/ccs/lib/

USE_STATIC_LIBRARIES=
CFLAGS		= -DRPCV2
ca_lmf.o_CFLAGS	= -DNO_PAK_REQ
INCFLAGS	= -I. -I${MAKETOP}../../export/${target_machine}/usr/include/eca

CLEANFILES	= *.a *.o
HFILES		= ${MSGHDRS}

CLIENT_STUBS	= sck_evd_cstb.o   sck_moi_cstb.o   sck_mold_cstb.o  sck_pei_cstb.o 

OFILES = \
	agent.o		agent_authentication.o agent_inter.o \
	ca_lmf.o	man_rpc.o	moss_avl.o	moss_cmp.o \
	moss_d_table.o	moss_filter.o	moss_handle.o \
	moss_misc.o	moss_mobind.o	moss_oid.o \
	moss_pebind.o	moss_reg.o	moss_send.o \
	moss_server.o	moss_uid.o	sck_ca_debug.o \
	sck_msg_text.o	socket.o 	utc_dummy.o ${CLIENT_STUBS}

include ${MAKEFILEPATH}/rules.mk


