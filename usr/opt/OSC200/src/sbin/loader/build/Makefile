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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1992/09/29 12:10:24 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ${TARGET_MACHINE}

OTHERS			= libloader.so
PROGRAMS		= lib_admin loader
MSGHDRS			= lib_admin_msg.h
CATFILES		= lib_admin.cat

# Note: loader and libloader.so are no longer built
#       from sources in this directory.  Instead, see:
#
#       src/usr/ccs/bin/rld       for the System V loader.
#       src/usr/shlib/libloader   for the libloader entry points.
#
ILIST                 = lib_admin
IDIR			= /sbin/
IMODE			= 555
libloader.so_IDIR	= /usr/shlib/
libloader.so_IMODE	= 644

EXPLOADER_TARGETS	= export_libloader.so
EXPDIR			= /usr/shlib/

USE_STATIC_LIBRARIES	=

INCFLAGS		= -I../include -I../ldr_include

CFLAGS			= ${${TARGET_MACHINE}CFLAGS}

loader_LINKSHARED=1
loader_COMMON_OFILES	= ../loader/ldr_bootstrap.o \
			  ../loader/ldr_context.o \
			  ../loader/ldr_global_file.o \
			  ../loader/ldr_known_pkg.o \
			  ../loader/ldr_main.o \
			  ../loader/ldr_preload.o \
			  ../loader/ldr_symres.o \
			  ../loader/region_alloc.o \
			  ../loader/ldr_dyn_mgr.o \
			  ../loader/ldr_errno.o \
			  ../loader/ldr_lock.o \
			  ../loader/ldr_brk.o

# WARNING: the following list of loader exports must be kept in sync with
# the ldr_bootstrap_exports.exp file.

LOADER_CONTEXT_EXPORTS		= ldr_context_load,ldr_context_get_entry_pt,ldr_context_unload,ldr_context_lookup,ldr_context_lookup_package,ldr_context_next_module,ldr_context_inq_module,ldr_context_inq_region,ldr_context_install,ldr_context_remove,ldr_context_global_file_init,ldr_context_global_file_remove,ldr_context_create,ldr_context_bootstrap,ldr_context_atexit,preload_alloc_abs,preload_alloc_rel,preload_dealloc,ldr_switch_ins_head,ldr_switch_ins_tail,ldr_switch_rem_head,ldr_switch_rem_tail,ldr_process_context,ldr_global_data_file,ldr_brk,ldr_sbrk

LOADER_UTILITIES_EXPORTS	= ldr_open,ldr_close,ldr_read,ldr_write,ldr_stat,ldr_fstat,ldr_lseek,ldr_ftruncate,ldr_getaddressconf,ldr_mmap,ldr_munmap,ldr_mprotect,ldr_strdup,hash_string,ldr_heap_malloc,ldr_heap_free,ldr_init_window,ldr_file_window,dec_map_refcount,ldr_regions_create,ldr_regions_free,ldr_packages_create,ldr_packages_free,ldr_symbols_create,ldr_symbols_free,open_hash_create_heap,open_hash_search,open_hash_destroy_heap,open_hash_elements,chain_hash_create_heap,chain_hash_search,chain_hash_destroy_heap,chain_hash_elements,ldr_process_heap,ldr_vm_pagesize

loader_LIBS		= ../utils/libutils.a ${${TARGET_MACHINE}_LOADER_LIBS}

lib_admin_COMMON_OFILES	= lib_admin.o

libloader.so_OFILES = ../loader/loader_utilities_stub.o \
		   ../loader/loader_context_stub.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${TARGET_MACHINE}/machdep.mk

lib_admin.o:	lib_admin_msg.h

libloader.so:	${libloader.so_OFILES}
	${_LD_} ${_SHLDFLAGS_} -o $@.X \
		-package loader_context ../loader/loader_context_stub.o \
		-package loader_utilities ../loader/loader_utilities_stub.o 
	${MV} $@.X $@

