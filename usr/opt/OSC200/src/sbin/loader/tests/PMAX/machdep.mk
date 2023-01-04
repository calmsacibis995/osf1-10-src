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
#	@(#)$RCSfile: machdep.mk,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/19 09:20:11 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# OSF/1 Release 1.0

# PMAXCFLAGS		= -G 0

tohash_LDFLAGS		= -non_shared

tchash_LDFLAGS		= -non_shared

testdq_LDFLAGS		= -non_shared

testsq1_LDFLAGS		= -non_shared

testsq2_LDFLAGS		= -non_shared

tnmmap_LDFLAGS		= -non_shared

# Uncomment the following lines to build tload2 statically linked against
# the loader, and comment out the lines for dynamic linking below.

tload2.o_CFLAGS	= -DLDR_STATIC_LINK
tload2_OFILES		= ${tload2_COMMON_OFILES} \
			  ../dummy/dummy.o ../dummy/dummy_switch.o \
			  ${LOADER_COMMON_OFILES} \
			  ../coff/coff_ldr.o \
			  ../coff/coff_relocate.o

#			  ../macho/mo_ldr.o \
#			  ../macho/relocate.o \
#			  ../macho/standard.o

# Fix for DEC OSF/1
#
#tload2_LIBS		= -lld
#
tload2_LIBS		= -lmld
tload2_LDFLAGS		= -non_shared

# The following lines build tload2 linked dynamically against the loader

# tload2_LDFLAGS		= -%ld,"-glue -export tload2_package:printf,sscanf,perror,gets"
# tload2_LIBS		= ../build/loader
#
# tload2_OFILES		= ${tload2_COMMON_OFILES}

tralloc_LDFLAGS		= -non_shared

ktload2_LDFLAGS		= -non_shared

kloadcall_LDFLAGS	= -non_shared

vm_region_LDFLAGS	= -non_shared

texport_OFILES		= ${texport_COMMON_OFILES} \
			  ${LOADER_COMMON_OFILES} \
			  ../coff/coff_ldr.o \
			  ../coff/coff_relocate.o

#			  ../macho/mo_ldr.o \
#			  ../macho/relocate.o \
#			  ../macho/standard.o

# Fix for DEC OSF/1
#
#texport_LIBS		= -lld
#
texport_LIBS		= -lmld
texport_LDFLAGS		= -non_shared

terrno_LDFLAGS		= -non_shared

TLOAD2PROGS		= ltest1 ltest2

# Support for test programs

LD			= ${MACHO_GCC_EXEC_PREFIX}ld

ltest1.o_CFLAGS		= -pic-lib tload2.so

ltest2.o_CFLAGS		= -pic-lib tload2.so

ltest1_LDFLAGS		= -R

ltest2_LDFLAGS		= -R 

kmod1.o_CFLAGS		= -pic-names ${VMUNIX_PICNAMES}
kmod1_LDFLAGS		= -R -e kmod1
kmod1_LIBS		= ${LIBVMUNIX_SO}

${TLOAD2PROGS}:
	${LD} ${_LDFLAGS_} -o $@ $@.o tload2

${KTLOAD2PROGS}:
	${LD} ${_LDFLAGS_} -o $@.X $@.o ${_LIBS_}
	${MV} $@.X $@


