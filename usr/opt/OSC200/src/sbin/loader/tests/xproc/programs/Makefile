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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:45:26 $
#
# (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# OSF/1 Release 1.0.1

#
#            Loader Cross Process Interface Test Programs
#
# This makefile builds some utility programs that are generally useful
# and help in testing the cross process interfaces.  The programs are
# ldr_region and dbg.  They both call the cmd_info() function as
# stolen from the loader test program tload2.  For example, the following
# output was produced by running ldr_region.
#
# % ldr_region $$
# 2	"/sbin/loader"	(Nregion 0 Flags 0x00000021):
# 3	"/usr/bin/csh"	(Nregion 4 Flags 0x00000080):
#	0	text
#		VA 0x00400000	MA 0x00400000	SZ   155648	Prot 0x005
#	1	rdata
#		VA 0x00426000	MA 0x00426000	SZ     8192	Prot 0x001
#	2	data
#		VA 0x10000000	MA 0x10000000	SZ     8192	Prot 0x003
#	3	bss
#		VA 0x10002000	MA 0x10002000	SZ    20480	Prot 0x007
# 4	"/usr/shlib/libc.so"	(Nregion 4 Flags 0x00000000):
#	0	text
#		VA 0x73000000	MA 0x73000000	SZ   299008	Prot 0x005
#	1	rdata
#		VA 0x73049000	MA 0x73049000	SZ    20480	Prot 0x001
#	2	data
#		VA 0x7304e000	MA 0x7304e000	SZ    49152	Prot 0x003
#	3	bss
#		VA 0x7305a000	MA 0x7305a000	SZ   131072	Prot 0x007
# %
#
# The dbg test program produces the same output, but won't work with an
# arbitrary process.  The dbg test programs accepts a program name, forks
# a child to run the program, but has the child enable tracing by calling
# ptrace(0), before exec'ing the program.  Thus the dbg program is ideal
# for testing the cross process interfaces when they use ptrace().
# The pause program built in this directory is ideal for this kind of
# testing.  For example, if everything is working, the following example,
#
# % dbg pause
#
# should produce output similar to the output listed above from ldr_region,
# although a little more slowly because of the use of ptrace().

# This Makefile assumes that the cross-process calls are in the C library
# use ptrace().  dbg should pick up that version.  ldr_region wants to use
# the VM calls and that's why it builds ldr_xproc.o with -D_USE_VM_CALLS.
# It builds ldr_xcalls.o so that the ldr_xproc.o that is a part of the
# shared C library won't be used.

PROGRAMS		= ldr_region dbg pause

LIBS			= -lloader

ldr_region_OFILES	= ldr_region.o info.o
ldr_region_LIBS		= -lxproc -lloader

dbg_OFILES		= dbg.o info.o
dbg_LIBS		= -lxproc -lloader

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
