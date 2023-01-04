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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/08/12 21:22:41 $
#
#
#	000	?		hsu
#	001	30-may-1991	hsu
#		add GenUtil, change to DATAFILES
#	002	30-may-1991	ccb
#		add Avl and libscp, change back to SCRIPTS
#	003	20-aug-1991	ech
#		add SED_SH_STRIP to avoid stripping # and :

SED_SH_STRIP	=

SCRIPTS		= Avl BadChoice BitTest Client Dialog Error GenUtil Kitcap \
		  Lists Logging Pwd Ready Strings Ticker Wait libmrg libscp

ILIST		= ${SCRIPTS}

IDIR		= /usr/share/lib/shell/

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/scripts.mk
