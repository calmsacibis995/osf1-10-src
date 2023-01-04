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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:51:07 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

VPATH			= ${MAKETOP}/usr/sbin/ufs_fsck

SEC_PROGRAMS		= ufs_fsck
PROGRAMS		= ufs_fsck

SEC_ILIST		= ufs_fsck
ILIST			= ufs_fsck
IDIR			= /sbin/

USE_STATIC_LIBRARIES	=

INCFLAGS		= -I${MAKETOP}/usr/sbin/ufs_fsck
#
# there's a bug when pass1.c is optimized; needs investigating...
#
CC_OPT_LEVEL	= 

##
# Need to add SMART_FSCK to CFLAGS when the kernel is ready
##
CFLAGS			= ${SEC_DEFS}
dir.o_CFLAGS		= -D_BSD ${SEC_DEFS}
pass2.o_CFLAGS		= -D_BSD ${SEC_DEFS}

HFILES			= fsck.h
##
# NB: should find a way to get ufs_subr.c and ufs_tables.c directly out of
#	the kernel source.
##
OFILES			= dir.o inode.o main.o pass1.o pass1b.o pass2.o \
			  pass3.o pass4.o pass5.o setup.o utilities.o \
			  preen.o ufs_subr.o ufs_tables.o
LIBS                    = ${SEC_LIBS}
LDFLAGS			= ${LDSTRIP}
GLINE			=

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
