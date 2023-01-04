
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
# HISTORY
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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

LIBC                    = ${MAKETOP}usr/ccs/lib/libc
LIBMACH			= ${MAKETOP}usr/ccs/lib/libmach
INCDIR			= ${MAKETOP}/usr/include
LIBDL                   = ${MAKETOP}usr/shlib/libdl
VPATH                   = ${LIBMACH}:${LIBMACH}/${target_machine}:${target_machine}:${MAKETOP}kernel/mach:${MAKETOP}kernel/mach_debug:${MAKETOP}usr/include/servers:${LIBDL}

OTHERS			= libmach.so
#SHARED_LIBRARIES	= libmach.so
MSGHDRS			= 
EXPSHLIB_TARGETS	= export_libmach.so
LDFLAGS			= -warning_unresolved

ILIST			= libmach.so
IDIR			= /usr/shlib/
IMODE			= 644

INCFLAGS                = -I${target_machine}
mach_user.c_MIGFLAGS	= -DMIG_USER
mach_debug.c_MIGFLAGS	= -DMIG_USER
mach_user.o_CFLAGS	= -DTypeCheck=0 ${CFLAGS}
MIG_DEFS		= exc.defs mach.defs \
			  mach_debug.defs env_mgr.defs ipcx.defs \
			  netmemory.defs netmemstat.defs \
			  netname.defs service.defs
OFILES			= mach_init.o error_codes.o mach_error.o \
			  slot_name.o mig_support.o mig_strncpy.o msg.o \
			  nmach_user.o env_mgrUser.o ipcxUser.o netnameUser.o \
			  serviceUser.o excUser.o excServer.o netmemoryUser.o \
			  netmemstat.o mach_debug.o Ovfork.o \
			 ${${target_machine}_COBJS}	\
			  ${${target_machine}_SOBJS}
GARBAGE			= env_mgr.h env_mgrUser.c \
			  exc.h excServer.c excUser.c \
			  ipcx.h ipcxUser.c \
			  mach_debug.h mach_debug.c \
			  mach.h mach_user.c \
			  netmemory.h netmemoryUser.c \
			  netmemstat.h netmemstat.c \
			  netname.h netnameUser.c \
			  service.h serviceUser.c

#OFILE_OBSOLETE           = XXX_new_vawp.o XXX_pdp.o XXX_pds.o XXX_pdu.o \
#			  XXX_plr.o 

include ${target_machine}/machdep.mk
include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk

# Sources for raw traps are taken from kernel include files.
# Routines that shadow libc versions are ignored.
# .h files are used for mig generated interfaces instead of the .c files
# message.h defines msg_send, msg_receive and msg_rpc so msg.c is not
# included

nmach_user.o: mach_user.c

mach_user.c: mach.defs
	${MIG} ${_MIGFLAGS_} $> -user $@ -server /dev/null

mach_debug.c: mach_debug.defs
	${MIG} ${_MIGFLAGS_} $> -user $@ -server /dev/null

netmemstat.c: netmemstat.defs
	${MIG} ${_MIGFLAGS_} $> -user $@ -server /dev/null

${STDOBJS}: $${@:.o=.c}
	${_CC_} ${_CCFLAGS_} -c $*.c
 
.c.o:
	${_CC_} -c ${_CCFLAGS_} $*.c
 
libmach.so: ${_OFILES_}
	${_LD_} ${_SHLDFLAGS_} -o $@.X ${_OFILES_} ${_LIBS_}
	${MV} $@.X $@
