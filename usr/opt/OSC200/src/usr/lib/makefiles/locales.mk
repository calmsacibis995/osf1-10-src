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
#	@(#)$RCSfile: locales.mk,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:33:19 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

# The following modules contain locale information.  Archiving this
# information into libc.a involves several steps:
#
# 1) The locale's collating table information (ctab) is converted into
# binary form via the ctab utility.
# The new locale and datafile can be installed on a running system so
# that the new locale can be used.
#
# 2) The binary form of the ctab information and non-binary environment
# information are used by the libloc utility to create a C-language source
# module.
#
# 3) The C-language source module is compiled and archived into the
# library.
# 
# 4) Locinfo.c's string and structure pointer arrays for the
# locales are hand modified as locales are added/removed from the
# library  module usr/ccs/lib/libc/locinfo.c (see comments in locinfo.c
# for more comments).
# After you have completed 1,2,3, and 4 you need to rebuild usr/ccs/lib/libc
# and relink your application or command with the new libc.  The newly
# linked application does not need an installed locale or locale datafile
# in order to be found, it is built in.

#
#  Build rules
#

${CTABFILES}: $${@:=.ctab}
	${CTAB} -i ${@:=.ctab} -o $@.X
	${MV} $@.X $@
