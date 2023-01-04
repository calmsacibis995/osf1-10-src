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
#     @(#)$RCSfile: Makefile,v $ $Revision: 4.3.10.2 $ (DEC) $Date: 1993/03/15 20:58:44 $
#
#
#


MIPS_OTHERS	= ex.ultrix.uu fsck.ultrix.uu u_vmunix.sas.uu u_netload.uu
ALPHA_OTHERS	=

OTHERS		= ${${TARGET_MACHINE}_OTHERS/*/&_decode}

ILIST		= ${${TARGET_MACHINE}_OTHERS}
IDIR		= /usr/mdec/

u_vmunix.sas_NOSTRIP	=

include ${MAKEFILEPATH}/standard.mk

%_decode: %
	uudecode %
