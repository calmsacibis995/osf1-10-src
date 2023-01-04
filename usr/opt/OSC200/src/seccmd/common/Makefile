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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:59:56 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
# Copyright (c) 1990 SecureWare, Inc.  All rights reserved.
#


SEC_LIBRARIES   = libcommon.a
LIBRARIES	= ${SEC_LIBRARIES}

SFILES		= AuthUtils.c Errors.c Message.c Utils.c XAuthUtils.c \
		PrUtils.c dbmalloc.c pwutils.c signals.c userutil.c \
		val_utils.c AudMaint.c EvMask.c UsrGrp.c DirLst.c \
		AudColSL.c ListSess.c BackupDel.c Restore.c Parameters.c \
		Selection.c Reduce.c Reports.c Devices.c HelpDisp.c \
		FileDisp.c

OFILES		= AuthUtils.o Errors.o Message.o Utils.o XAuthUtils.o \
		PrUtils.o dbmalloc.o pwutils.o signals.o userutil.o \
		val_utils.o AudMaint.o EvMask.o UsrGrp.o DirLst.o \
		AudColSL.o ListSess.o BackupDel.o Restore.o Parameters.o \
		Selection.o Reduce.o Reports.o Devices.o HelpDisp.o \
		FileDisp.o

INCFLAGS	= -I${MAKETOP}/seccmd

CFLAGS		= ${SEC_DEFS} -D_AIF -DLOGGING

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/scripts.mk
include ${MAKEFILEPATH}/objects.mk







