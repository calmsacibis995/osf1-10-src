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
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/10/14 04:17:08 $
#
#
# HISTORY
#
# OSF/1 Release 1.0
#

VPATH		= ../troff
PROGRAMS	= nroff

ILIST		= nroff
IDIR		= /usr/bin/

INCFLAGS	= -I../troff
ANSIFLAG	= -std0
CFLAGS		= -DNROFF -DVMUNIX 
HFILES		= d.h tw.h v.h tdef.h sdef.h
OFILES		= ni.o nii.o n1.o n2.o n3.o n4.o n5.o n6.o n7.o \
		  n8.o n9.o n10.o hytab.o suftab.o ntab.o na.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
