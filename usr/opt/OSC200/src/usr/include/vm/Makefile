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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 17:51:43 $
#
# OSF/1 Release 1.0

VPATH			= ${MAKETOP}/kernel/vm

DATAFILES		= vmparam.h
INCLUDES		= ${DATAFILES}

# See src/setup/postinstall.sh (search for HEADERS_SYMLINKS:)
#ILIST			= ${INCLUDES}
IDIR			= /usr/include/vm/
IMODE			= 644

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
