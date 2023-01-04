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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/04/01 20:15:07 $
# 
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
# Copyright (c) 1989 SecureWare, Inc.  All rights reserved.
#


USE_STATIC_LIBRARIES	=

PROGRAMS_BASE	= 
PROGRAMS_ACL	= chacl ipcacl lsacl spconfig
PROGRAMS_PRIV   = chpriv lspriv
PROGRAMS_MAC	= chlevel getlevel ipclevel lslevel setlevel
PROGRAMS_ILB	= chilevel ipcilevel lsilevel

PROGRAMS	= ${PROGRAMS_${SEC_LEVEL}}
SEC_PROGRAMS	= ${PROGRAMS}

SEC_ILIST	= ${ILIST}
ILIST		= ${PROGRAMS}
IDIR		= /var/tcb/bin/
SYMLINKS	= ${ILIST}
SYMLINKDIR1	= ../../var/tcb/bin
SYMLINKDIR2	= /tcb/bin
SEC_IMODE	= 111
SEC_IGROUP	= sec


MSGHDRS		= policy_msg.h lsacl_msg.h chacl_msg.h
CATFILES	= policy.cat lsacl.cat chacl.cat

HFILES		= ${MSGHDRS}

CFLAGS		= ${SEC_DEFS}

# Rules for individual commands
chacl_IMODE	= 2111
chacl_OFILES    = ${chacl_${ACL_POLICY}_OFILES} filetobuf.o
chacl_ACL_SWARE_OFILES  = chpolicy.o conv_acl.o
chacl_ACL_POSIX_OFILES  = chacl.o
lsacl_OFILES 	= ${lsacl_${ACL_POLICY}_OFILES}
lsacl_ACL_SWARE_OFILES  = lspolicy.o conv_acl.o print_acl.o
lsacl_ACL_POSIX_OFILES  = lsacl.o
chlevel_OFILES	= chpolicy.o conv_sl.o filetobuf.o
chilevel_OFILES = chpolicy.o conv_il.o filetobuf.o
getlevel_OFILES	= getlevel.o conv_sl.o
ipcacl_OFILES	= ipcpolicy.o conv_acl.o ipc_acl.o filetobuf.o
ipcacl_IMODE	= 2111
ipclevel_OFILES	= ipcpolicy.o conv_sl.o ipc_slabel.o filetobuf.o
ipcilevel_OFILES = ipcpolicy.o conv_il.o ipc_ilabel.o filetobuf.o
lslevel_OFILES	= lspolicy.o conv_sl.o print_sl.o
lsilevel_OFILES	= lspolicy.o conv_il.o print_il.o
setlevel_OFILES	= setlevel.o conv_sl.o
LIBS		= -lsecurity
ipclevel_LIBS	= ${LIBS} -lcurses
ipcilevel_LIBS	= ${LIBS} -lcurses
lsacl_LIBS	= ${LIBS}
chacl_LIBS	= ${LIBS}
lslevel_LIBS	= ${LIBS} -lcurses
lsilevel_LIBS	= ${LIBS} -lcurses
lspriv_LIBS	= ${LIBS} -lcurses

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
