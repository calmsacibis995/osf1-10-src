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
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
# ALL RIGHTS RESERVED 
#
# OSF/1 1.2

PROGRAMS		= patch
MSGHDRS 		= patch_msg.h
CATFILES 		= patch.cat

ILIST			= patch
IDIR			= /usr/bin/

HFILES			= patch_msg.h
OFILES 			= patch.o pch.o inp.o util.o version.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk

patch.o:   config.h common.h patch.c inp.h pch.h util.h version.h
pch.o:	   config.h common.h pch.c pch.h util.h pathnames.h
inp.o:	   config.h common.h inp.c inp.h util.h
util.o:	   config.h common.h util.c util.h pathnames.h
version.o: config.h common.h version.c version.h patchlevel.h util.h
