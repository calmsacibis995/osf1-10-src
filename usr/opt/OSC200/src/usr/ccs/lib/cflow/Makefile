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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:40:57 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

CCSBIN			= ${MAKETOP}/usr/ccs/bin
VPATH			= ${CCSBIN}/common:${CCSBIN}/cflow

SUBDIRS			= cflow1
PROGRAMS		= dag flip lpfx nmf
#
# cflow.msg comes from usr/ccs/bin/cflow
#
MSGHDRS			= cflow_msg.h ctools_msg.h
CATFILES		= cflow.cat ctools.cat

ILIST			= dag flip lpfx nmf
IDIR			= /usr/ccs/lib/

INCFLAGS		= -I${CCSBIN}/common -I${CCSBIN}/cflow
CFLAGS			= -DHOSTIEEE -DCFLOW2

lpfx_OFILES		= lpfx.o tftn.o xdefs.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/objects.mk

dag.o:	cflow_msg.h
lpfx.o: mfile1.h error.h ctools_msg.h
tftn.o:	ctools_msg.h
