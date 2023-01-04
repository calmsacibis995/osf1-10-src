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
#	@(#)$RCSfile: manpages.mk,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:33:21 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
# default nroff program to run
#
NROFF=nroff

#
# default flags to nroff
#
DEF_NROFFFLAGS=${DEF_NROFFFLAGS?${DEF_NROFFFLAGS}:-man -h}

#
# all flags for nroff
#
_NROFFFLAGS_=${${@}_DEF_NROFFFLAGS?${${@}_DEF_NROFFFLAGS}:${DEF_NROFFFLAGS}} ${${@}_NROFFENV?${${@}_NROFFENV}:${NROFFENV}} ${${@}_NROFFFLAGS?${${@}_NROFFFLAGS}:${NROFFFLAGS}} ${${@}_NROFFARGS?${${@}_NROFFARGS}:${NROFFARGS}}

#
#  Default single suffix compilation rules
#
${MANSECTION?.SUFFIXES\: .0 .${MANSECTION}:}

#
#  Default double suffix compilation rules
#
${MANSECTION?.${MANSECTION}.0:}:
	${NROFF} ${_NROFFFLAGS_} $*.${MANSECTION} >$*.0.X
	${MV} -f $*.0.X $*.0
