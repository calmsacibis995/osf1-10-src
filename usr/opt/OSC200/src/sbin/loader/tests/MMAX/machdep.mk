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
#	@(#)$RCSfile: machdep.mk,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:41:59 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

MMAXCFLAGS		= -mnosb

# Uncomment the following lines to build tload2 statically linked against
# the loader, and comment out the lines for dynamic linking below.

# tload2.o_CFLAGS	= -DLDR_STATIC_LINK
# tload2_OFILES		= ${tload2_COMMON_OFILES} \
#			  ${LOADER_COMMON_OFILES} \
#			  ../coff/ldr_abs.o \
#			  ../macho/mo_ldr.o \
#			  ../macho/relocate.o \
#			  ../macho/standard.o
# tload2_LIBS		= -lld

# The following lines build tload2 linked dynamically against the loader

tload2_OFILES		= ${tload2_COMMON_OFILES}
tload2_LDFLAGS		= -%ld,"-export tload2_package:printf,sscanf,perror,gets" -L/usr/shlib
tload2_LIBS		= -lld ../build/loader

texport_OFILES		= ${texport_COMMON_OFILES} \
			  ${LOADER_COMMON_OFILES} \
			  ../coff/ldr_abs.o \
			  ../macho/mo_ldr.o \
			  ../macho/relocate.o \
			  ../macho/standard.o
texport_LIBS		= -lld

TLOAD2PROGS		= ltest1 ltest2

# Support for test programs

LD			= ${MACHO_GCC_EXEC_PREFIX}ld

ltest1_LDFLAGS		= -R -e _foo_entry

ltest2_LDFLAGS		= -R -e _foo_entry

ltest3_LDFLAGS		= -R -e _foo_entry

kmod1.o_CFLAGS		= -mregparm -mnosb -pic-names ${VMUNIX_PICNAMES}
kmod1_LDFLAGS		= -R -e _kmod1
kmod1_LIBS		= ${LIBVMUNIX_SO}

${TLOAD2PROGS}:
	${LD} ${_LDFLAGS_} -o $@ $@.o tload2

${KTLOAD2PROGS}:
	${LD} ${_LDFLAGS_} -o $@.X $@.o ${_LIBS_}
	${MV} $@.X $@
