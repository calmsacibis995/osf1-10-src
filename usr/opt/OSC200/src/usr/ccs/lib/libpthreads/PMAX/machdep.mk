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
#	@(#)$RCSfile: machdep.mk,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:12:15 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

SFILES				= async.o lock.o
${TARGET_MACHINE}_OFILES	= ${SFILES} machdep.o

async.o_CCTYPE			= traditional
lock.o_CCTYPE			= traditional

MACHO_CFLAGS			= -DOSF_ROSE
machdep.o_CFLAGS		= ${${OBJECT_FORMAT}_CFLAGS}

#.SUFFIXES: .S .s

#.s.S:
#	rm -f $@
#	cp $< $@

${SFILES}: $${@:.o=.s}
	${_CC_} -g0 ${_CCFLAGS_} -c ${@:.o=.s}
