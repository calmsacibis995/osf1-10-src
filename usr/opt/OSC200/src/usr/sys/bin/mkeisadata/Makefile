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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/23 20:10:31 $
#
PROGRAMS		= mkeisadata
ILIST			= mkeisadata
IDIR			= /usr/sys/bin/
IMODE			= 555
IOWNER			= bin
IGROUP			= bin

USE_STATIC_LIBRARIES	=

OFILES			= mkeisadata.o

CFLAGS			= `genpath -I${MAKETOP}/kernel/src/mkeisadata` `genpath \
			  -I${MAKETOP}/kernel/include`

VPATH			= ${MAKETOP}:${MAKETOP}/kernel/src/mkeisadata
I18N_FLAG		=


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/others.mk
