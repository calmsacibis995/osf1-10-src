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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.1.7.2 $ (Digital Equipment Corporation) $Date: 1993/11/11 17:07:27 $
#
# (c) COPYRIGHT 1991, DIGITAL EQUIPMENT CORPORATION
#       ALL RIGHTS RESERVED
#
#
# Makefile for installing DAS include files
#
#       Ray Giroux      2/28/92
#
VPATH = ${MAKETOP}/usr/ccs/lib/libchf:${MAKETOP}/usr/ccs/lib/libips:${MAKETOP}/usr/ccs/lib/libimg:${MAKETOP}/usr/ccs/lib/libids_nox

DAS_EXPORT =	ChfDef.h \
		IdsImage.h \
		IdsStatusCodes.h \
		ImgDef.h \
		ImgEntry.h \
		ImgStatusCodes.h \
		ids_alloc_colors.h \
		ImgDefP.h \
		ImgMacros.h \
		IpsDef.h \
		IpsStatusCodes.h \
		ids__macros.h \
		ids__pipe.h \
		ids__widget.h \
		ids__widget_motif.h \
		imgprot_ids.h \
		ipsprot.h \
		idsprot.h


DATAFILES = 	${DAS_EXPORT}
INCLUDES	= ${DATAFILES}
EXPDIR		= /usr/include/img/

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
