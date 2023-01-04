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
#	@(#)$RCSfile: machdep.mk,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:35:15 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

loader_LDFLAGS		= -T 70000000 -D 70800000

# need the following for macho support:
# PMAX_LOADER_LIBS	= -lld

loader_OFILES		= main.o switch.o ${loader_COMMON_OFILES} \
			  ../coff/coff_ldr.o \
			  ../coff/coff_relocate.o

# need the following for macho support:
#			  ../macho/mo_ldr.o \
#			  ../macho/relocate.o \
#			  ../macho/standard.o

# need the following for macho support:
# switch.o_CFLAGS		= -DLDR_PRELOAD_MGR -DLDR_MACHO_MGR -DLDR_COFF_MGR
switch.o_CFLAGS		= -DLDR_PRELOAD_MGR -DLDR_COFF_MGR

# Uncomment the following lines to statically link lib_admin against the
# standalone loader, and comment out the dynamic link lines below.

lib_admin.o_CFLAGS	= -DLDR_STATIC_LINK
lib_admin_OFILES	= ${lib_admin_COMMON_OFILES} \
			  ../dummy/dummy.o ../dummy/dummy_switch.o \
			  ${loader_COMMON_OFILES} \
			  ../coff/coff_ldr.o \
			  ../coff/coff_relocate.o

# need the following for macho support:
#			  ../macho/mo_ldr.o \
#			  ../macho/relocate.o \
#			  ../macho/standard.o

# need the following for macho support:
# lib_admin_LIBS		= ../utils/libutils.a -lld -lAF
lib_admin_LIBS		= ../utils/libutils.a -lAF

# The following lines dynamically link lib_admin against the loader.

# lib_admin_OFILES	= ${lib_admin_COMMON_OFILES}
# # need the following for macho support:
# # lib_admin_LIBS		= -lld -lAF loader
# lib_admin_LIBS		= -lAF loader
