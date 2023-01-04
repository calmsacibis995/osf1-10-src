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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.3.3.3 $ (DEC) $Date: 1992/04/17 10:34:46 $
#
# (c) Copyright 1991, DIGITAL EQUIPMENT CORPORATION
# ALL RIGHTS RESERVED
#
#


OTHERS			= defstyle.ddif
DATAFILES		= cda_file_type.dat

INCLUDES		= ${OTHERS} ${DATAFILES}
ILIST			= ${OTHERS} ${DATAFILES}

EXPLIB_TARGETS		= export_defstyle.ddif export_cda_file_type.dat
EXPDIR			= /usr/lib/cda/
IDIR			= /usr/lib/cda/
IMODE			= 644

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/others.mk

