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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.1.3.3 $ (DEC) $Date: 1992/04/15 16:17:09 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# Modification History
# --------------------
# 10/08/91 - Tom Peterson
#	- first version of this Makefile
#	- added /usr/skel directory and .login .profile .cshrc which
#	are used by adduser.
#	- modified Makefile in /usr to include /usr/skel as well.
#
DATAFILES		= .login .profile .cshrc

INSTDIRS		= bin
ILIST			= ${DATAFILES}
IDIR			= /usr/skel/
IMODE			= 755

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk


