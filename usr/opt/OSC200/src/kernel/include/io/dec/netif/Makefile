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
#	@(#)$RCSfile: Makefile,v $ $Revision: 1.2.17.5 $ (DEC) $Date: 1993/07/30 18:30:57 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#

VPATH			= ${MAKETOP}/kernel/io/dec/netif

ALPHA_SUBDIRS		= alpha
MIPS_SUBDIRS 		=
SUBDIRS			= ${${TARGET_MACHINE}_SUBDIRS}

IMODE			= 644

DATAFILES		= if_lnreg.h if_fzareg.h if_xnareg.h if_nereg.h	\
			  if_mfareg.h if_qereg.h if_uba.h if_pdqreg.h 	\
			  if_tereg.h if_trareg.h

ILIST   = ${DATAFILES}
IDIR    = /usr/sys/include/io/dec/netif/
IOWNER  = bin
IGROUP  = bin

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/datafiles.mk
