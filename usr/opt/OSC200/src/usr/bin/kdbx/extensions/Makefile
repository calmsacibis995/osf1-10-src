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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.11.7 $ (DEC) $Date: 1993/12/15 22:12:21 $
#
#
#	@(#)Makefile	3.1	(ULTRIX/OSF)	2/26/91
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

STANDALONES		= array_action arp buf callout cast config convert \
			  ctime default dis ds echo export fields file filename \
                          ilock inpcb list_action list_cells mbuf memwatch mount \
                          namecache ofile paddr pcb plist printf proc procaddr \
                          slot socket sum swap task thread tty u ucred \
			  unaliasall vlock vnode walk
#NEED_KERNEL_POOL	= scsi
NEED_KERNEL_POOL	= 
PROGRAMS		= ${STANDALONES} ${NEED_KERNEL_POOL}
DATAFILES		= site.kdbxrc system.kdbxrc
KRASH_OBJS		= krashlib.o array.o
array_action_OFILES	= ${KRASH_OBJS} array_action.o
arp_OFILES       	= ${KRASH_OBJS} arp.o
buf_OFILES       	= ${KRASH_OBJS} buf.o
callout_OFILES       	= ${KRASH_OBJS} callout.o
cast_OFILES		= ${KRASH_OBJS} cast.o
config_OFILES		= ${KRASH_OBJS} config.o
convert_OFILES		= ${KRASH_OBJS} convert.o
#crcheck_OFILES		= ${KRASH_OBJS} crcheck.o
ctime_OFILES		= ${KRASH_OBJS} ctime.o
default_OFILES       	= ${KRASH_OBJS} default.o
dis_OFILES       	= ${KRASH_OBJS} dis.o
ds_OFILES               = ${KRASH_OBJS} ds.o
echo_OFILES		= ${KRASH_OBJS} echo.o
export_OFILES           = ${KRASH_OBJS} export.o  
fields_OFILES		= ${KRASH_OBJS} fields.o

file_OFILES       	= ${KRASH_OBJS} file.o
filename_OFILES		= ${KRASH_OBJS} filename.o
ilock_OFILES            = ${KRASH_OBJS} ilock.o 
inpcb_OFILES       	= ${KRASH_OBJS} inpcb.o
list_action_OFILES	= ${KRASH_OBJS} list_action.o
list_cells_OFILES	= ${KRASH_OBJS} list_cells.o
mbuf_OFILES             = ${KRASH_OBJS} mbuf.o 
memwatch_OFILES       	= ${KRASH_OBJS} memwatch.o
mount_OFILES       	= ${KRASH_OBJS} mount.o
namecache_OFILES        = ${KRASH_OBJS} namecache.o
ofile_OFILES            = ${KRASH_OBJS} ofile.o  
paddr_OFILES		= ${KRASH_OBJS} paddr.o
pcb_OFILES              = ${KRASH_OBJS} pcb.o 
plist_OFILES		= ${KRASH_OBJS} plist.o
proc_OFILES		= ${KRASH_OBJS} proc.o
printf_OFILES		= ${KRASH_OBJS} printf.o
print_data_OFILES	= ${KRASH_OBJS} print_data.o
procaddr_OFILES		= ${KRASH_OBJS} procaddr.o
#scsi_OFILES		= ${KRASH_OBJS} scsi.o
slot_OFILES		= ${KRASH_OBJS} slot.o
socket_OFILES       	= ${KRASH_OBJS} socket.o
sum_OFILES		= ${KRASH_OBJS} sum.o
swap_OFILES		= ${KRASH_OBJS} swap.o
task_OFILES             = ${KRASH_OBJS} task.o
thread_OFILES           = ${KRASH_OBJS} thread.o
tty_OFILES       	= ${KRASH_OBJS} tty.o
u_OFILES		= ${KRASH_OBJS} u.o
ucred_OFILES		= ${KRASH_OBJS} ucred.o
unaliasall_OFILES	= ${KRASH_OBJS} unaliasall.o
vlock_OFILES            = ${KRASH_OBJS} vlock.o 
vnode_OFILES		= ${KRASH_OBJS} vnode.o
walk_OFILES		= ${KRASH_OBJS} walk.o
#vmmap_OFILES		= ${KRASH_OBJS} vmmap.o

HFILES			= ../krash.h
CFLAGS			= -DDEBUGGER
INCFLAGS		= -I.. -I../../../../kernel \
			  -I../../../../../obj/alpha/kernel/BINARY
#			  -I/usr/include

INSTDIRS		= kdbx
kdbx_IDIR		= /var/
ILIST			= ${PROGRAMS} ${DATAFILES}
IDIR			= /var/kdbx/

OPT_LEVEL		= -g

EXPORT_DIR		= /osf/usr/krash
EXPORT_EXT_DIR		= ${EXPORT_DIR}/extensions
EXPORT_BIN_DIR		= ${EXPORT_DIR}/bin
EXPORT_DOC_DIR		= ${EXPORT_DIR}/doc

${PROGRAMS} : ${KRASH_OBJS}

#krashlib.o : ${MAKETOP}/../src/usr/bin/kdbx/krashlib.c

#array.o : ../array.c

cmd :
	${CMD}

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk
