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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# OSF/1 Release 1.0

# Select one of the following IPC mechanisms (must match mechanism
# used in building libc)

# IPC_CFILE		= kls_ipc_sysV_msg.c
# IPC_CFILE		= kls_ipc_sock_stream.c
# IPC_CFILE		= kls_ipc_mach.c

IPC_CFILE		= kls_ipc_sock_dgram.c

ldr_kernel_bootstrap.o_CFLAGS = -DKLS_COFF_MGR
# need this for macho support:
#			-DKLS_MACHO_MGR

# Select the kernel loader managers for obtaining symbol information
# from the kernel

kloadsrv_KERNEL_MGRS	= kls_coff.o \
			  kls_coff_machdep.o

# need this for macho support:
#			  kls_macho.o
# kls_switch.o_CFLAGS	= -DLDR_MACHO_MGR -DLDR_RCOFF_MGR
kls_switch.o_CFLAGS	= -DLDR_RCOFF_MGR

# need this directory for rcoff support
# How to do this?  Currently have a symbolic link
# in a sandbox to pick off the include file.
#
rcoff_relocate.o__CCFLAGS_ = ${_CCFLAGS_} -I../rcoff/alpha/include
../rcoff/rcoff_relocate.o_CFLAGS = ${CFLAGS} -I../rcoff/alpha/include

# Uncomment the following lines to statically link the kernel load
# server against the loader, and uncomment the dynamic link lines below.

kloadsrv.o_CFLAGS	= -DLDR_STATIC_LINK
kloadsrv_LDFLAGS	= -non_shared
kloadsrv_OFILES	= kloadsrv.o \
			  ${IPC_OFILES} \
			  ${KERNEL_LOADER_OFILES} \
			  ${LOADER_COMMON_OFILES} \
			  ${kloadsrv_KERNEL_MGRS} \
			  ../rcoff/rcoff_ldr.o \
			  ../rcoff/rcoff_relocate.o \
			  ../rcoff/rcoff_standard.o
# need this for coff support:
#			  ../coff/coff_ldr.o \
#			  ../coff/coff_relocate.o \

# need this for macho support:
#			  ../macho/mo_ldr.o \
#			  ../macho/relocate.o \
#			  ../macho/standard.o

# need this for macho support:
# kloadsrv_LIBS		= -lld

# The following lines dynamically link the kernel load server against the loader

# kloadsrv_OFILES		= kloadsrv.o \
#			  ${IPC_OFILES} \
#			  ${KERNEL_LOADER_OFILES} \
#			  ${kloadsrv_KERNEL_MGRS}
# # kloadsrv_LDFLAGS	= -%ld,-glue
# # need this for macho support:
# # kloadsrv_LIBS		= -lld ../build/loader
# kloadsrv_LIBS		= ../build/loader



