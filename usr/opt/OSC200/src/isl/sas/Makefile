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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.4.11.3 $ (DEC) $Date: 1993/06/02 20:01:57 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
#	000	21-jun-1991	ccb
#	001	02-jul-1991	ccb
#		Fix typo. MAKEPSD said MAKESPD
#	002	18-jul-1991	ccb
#		Add OTHERS to ILIST
#	003	11-Dec-1991	pwl
#		Add PROGRAMS to ILIST
#

DATAFILES		= .miniprofile .profile buildmini.osf fstab inittab \
			   install.1 machine mini.inv updmini
PROGRAMS		= btd gethost
USE_STATIC_LIBRARIES	=

IDIR			= /isl/sas/

INCFLAGS		= -I../../kernel

ILIST			= ${DATAFILES} ${PROGRAMS}

MAKEPSD			= ${SOURCEBASE}/isl/sas

btd_OFILES		= btd.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/others.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk
