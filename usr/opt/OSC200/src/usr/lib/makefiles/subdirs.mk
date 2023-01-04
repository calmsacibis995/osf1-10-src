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
#	@(#)$RCSfile: subdirs.mk,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:33:42 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
# flags passed to sub-invokations of make
#
_MAKEFLAGS_=${RELEASE_OPTIONS?'RELEASE_OPTIONS=${RELEASE_OPTIONS}':} ${TOSTAGE?'TOSTAGE=${TOSTAGE}':} ${FROMSTAGE?'FROMSTAGE=${FROMSTAGE}':}

#
#  Change into a directory and make recursively
#
#
#  __DO_<action>__PASS_<pass>__SUBDIR_<subdir>__
#
#	echo where we are
#	create path to subdir is needed
#	cd into subdir
#	exec make with pass and action
#
__DO_%__PASS_%2__SUBDIR_%3__: ${ALWAYS}
	@echo "[ ${MAKEDIR:/=}/%3 ]"
	makepath %3/. && cd %3 && \
	exec ${MAKE} ${_MAKEFLAGS_} ${%3_SUBMAKEFLAGS?${%3_SUBMAKEFLAGS}:${SUBMAKEFLAGS}} MAKEFILE_PASS=%2 ${_%_ACTION_}_all
