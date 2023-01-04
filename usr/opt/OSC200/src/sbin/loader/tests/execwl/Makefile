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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:42:09 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

LOADERS		= print_hex_flags_loader print_dec_flags_loader \
		  print_file_loader print_loader_loader \
		  print_argc_loader print_argv_loader print_env_loader \
		  user_loader echo_loader failure_loader

RANDOMS		= dec_random hex_random

CPROGS		= execve exec_with_loader env \
		  getuid geteuid getsetuid getsetgid \
		  setuidprogram setgidprogram \
		  test04 test05 test06 test11 test12 test13 test18 test19 \
		  cprog25

PROGRAMS	= $(LOADERS) $(RANDOMS) $(CPROGS)

SCRIPTS		= prog loop test01 test02 test03 test07 test08 test09 \
		  test14 test15 test16 test17 test20 test21 test22 test23 \
		  test24 test25 test26 test27 test28 test29 test30 \
		  test31 test32

dec_random_OFILES		= dec_random.o
dec_random_LDFLAGS		= -non_shared
hex_random_OFILES		= hex_random.o
hex_random_LDFLAGS		= -non_shared
hex_random.o_CFLAGS		= -DHEX

print_dec_flags_loader_OFILES	= print_dec_flags_loader.o auxv.o
print_dec_flags_loader_LDFLAGS	= -non_shared
print_hex_flags_loader_OFILES	= print_hex_flags_loader.o auxv.o
print_hex_flags_loader.o_CFLAGS	= -DHEX
print_hex_flags_loader_LDFLAGS	= -non_shared

print_loader_loader_OFILES	= print_loader_loader.o auxv.o
print_loader_loader_LDFLAGS	= -non_shared
print_file_loader_OFILES	= print_file_loader.o auxv.o
print_file_loader_LDFLAGS	= -non_shared
echo_loader_OFILES		= echo_loader.o auxv.o
echo_loader_LDFLAGS		= -non_shared

print_argc_loader_LDFLAGS	= -non_shared
print_argv_loader_LDFLAGS	= -non_shared
print_env_loader_LDFLAGS	= -non_shared
user_loader_LDFLAGS		= -non_shared
failure_loader_LDFLAGS		= -non_shared

getsetuid_OFILES		= getsetuid.o
getsetuid_LDFLAGS		= -non_shared
getsetuid.o_CFLAGS		= -DSETUID
getsetgid_OFILES		= getsetgid.o
getsetgid_LDFLAGS		= -non_shared

execve_LDFLAGS			= -non_shared
exec_with_loader_LDFLAGS	= -non_shared
env_LDFLAGS			= -non_shared
getuid_LDFLAGS			= -non_shared
geteuid_LDFLAGS			= -non_shared
setuidprogram_LDFLAGS		= -non_shared
setgidprogram_LDFLAGS		= -non_shared

LIBLT_DIR	= ../liblt

test04_LIBS		= $(LIBLT_DIR)/liblt.a
test04_LDFLAGS		= -non_shared
test05_LIBS		= $(LIBLT_DIR)/liblt.a
test05_LDFLAGS		= -non_shared
test06_LIBS		= $(LIBLT_DIR)/liblt.a
test06_LDFLAGS		= -non_shared
test11_LIBS		= $(LIBLT_DIR)/liblt.a
test11_LDFLAGS		= -non_shared
test12_LIBS		= $(LIBLT_DIR)/liblt.a
test12_LDFLAGS		= -non_shared
test13_LIBS		= $(LIBLT_DIR)/liblt.a
test13_LDFLAGS		= -non_shared
test18_LIBS		= $(LIBLT_DIR)/liblt.a
test18_LDFLAGS		= -non_shared
test19_LIBS		= $(LIBLT_DIR)/liblt.a
test19_LDFLAGS		= -non_shared
cprog25_LIBS		= $(LIBLT_DIR)/liblt.a
cprog25_LDFLAGS		= -non_shared

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/scripts.mk

dec_random.c: random.c
	cp random.c dec_random.c
hex_random.c: random.c
	cp random.c hex_random.c

print_dec_flags_loader.c: print_flags_loader.c
	cp print_flags_loader.c print_dec_flags_loader.c
print_hex_flags_loader.c: print_flags_loader.c
	cp print_flags_loader.c print_hex_flags_loader.c

getsetuid.c: getsetid.c
	cp getsetid.c getsetuid.c

getsetgid.c: getsetid.c
	cp getsetid.c getsetgid.c
