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
#	@(#)$RCSfile: Makefile,v $ $Revision: 1.2.10.2 $ (DEC) $Date: 1993/10/13 17:12:22 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#

VPATH			= ${MAKETOP}/kernel/io/dec/tc

DATAFILES		= ${ILIST}

ILIST			= dc7085cons.h dc7085reg.h gx.h ioasic.h sccreg.h \
			  scc_common.h slu.h tc.h xcons.h nvtcreg.h codecioc.h \
			  amd79c30.h amd79c30_codec.h amd79c30_dlld_drv.h \
			  amd79c30_hdlc.h amd79c30_reg.h

IDIR	= /usr/sys/include/io/dec/tc/
IMODE	= 444
IGROUP	= bin
IOWNER	= bin

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk

