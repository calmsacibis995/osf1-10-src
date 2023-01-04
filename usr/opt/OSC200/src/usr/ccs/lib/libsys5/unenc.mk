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
# @(#)$RCSfile: unenc.mk,v $ $Revision: 1.1.2.10 $ (DEC) $Date: 1993/11/22 22:03:38 $
#

VPATH			= ${target_machine}:${LIBSYS5}:${LIBSYS5}/${target_machine}

UNENC_STDOBJS		= setpgrp.o mount.o umount.o crt1.o mknod.o \
			  mktemp.o getcwd.o \
			  sys5init.o ttyname.o pipe.o mkfifo.o \
			  siginfo_sig.o sigpause.o sigset.o truncate.o

# These will be delivered in a future checkpoint.
#UNENC_STDOBJS		= getdate.o fmtmsg.o

UNENC_OFILES		= ${UNENC_STDOBJS} \
			  ${${target_machine}UNENC_SOBJS} \
			  ${${target_machine}UNENC_COBJS} 

CFLAGS			= ${CFLAGS} -DSVR4_LIB_SYS5
doprnt.o_CFLAGS		= ${CFLAGS} -DSVR4_PRINTF
doprnt.o_INCFLAGS	= ${INCFLAGS} -I${MAKETOP}/usr/ccs/lib/libc
getopt.o_INCFLAGS	= ${INCFLAGS} -I${MAKETOP}/usr/ccs/lib/libc
