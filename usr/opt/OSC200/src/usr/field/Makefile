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
#       @(#)Makefile    3.1     (ULTRIX/OSF)
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
# Modification History
# --------------------
# Oct 10 1991           added shmx and shmx subdirectories
#
# July 3 1991		prs
#			Added fsx and fsx subdirectories
#
# April	1991		Matthew Sacks
#			Created this module

# Please leave mtx & dskx out. They are not shipped with OSF/1 -- Fred
SUBDIRS			= diskx tapex cmx memx memxr fsx fsxr shmx shmxb rt

# Modes and ownership of executables should match Ultrix -- johng
IMODE			= 744
IOWNER			= root
IGROUP			= system

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
