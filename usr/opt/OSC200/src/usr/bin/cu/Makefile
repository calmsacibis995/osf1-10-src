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

VPATH			= ../uucp

PROGRAMS		= cu
MSGHDRS                 = uucp_msg.h

ILIST			= cu
IDIR			= /usr/bin/
IOWNER			= uucp
IGROUP			= uucp
IMODE			= 2755

CCTYPE                  = writable_strings
INCFLAGS		= -I../uucp
callers.o_CFLAGS	= -DSTANDALONE
conn.o_CFLAGS		= -DSTANDALONE
culine.o_CFLAGS		= -DSTANDALONE
uucpdefs.o_CFLAGS	= -DSTANDALONE
cu.o_CFLAGS		= -DSTANDALONE
HFILES                  = pathnames.h uucp_msg.h
OFILES			= altconn.o callers.o conn.o cu.o getargs.o \
                          culine.o ttylock.o uucpdefs.o



include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
