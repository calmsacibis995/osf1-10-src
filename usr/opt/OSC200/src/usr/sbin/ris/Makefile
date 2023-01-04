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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/05/06 10:55:31 $
#

SED_SH_STRIP	=
SCRIPTS		= ris rismenu
PROGRAMS	= getname fixit
ILIST		= ${PROGRAMS} ${SCRIPTS}
IDIR		= /usr/sbin/
rismenu_IDIR	= /usr/lbin/
getname_IDIR	= /var/adm/ris/bin/
fixit_IDIR	= /var/adm/ris/bin/

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/scripts.mk

