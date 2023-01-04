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
#       @(#)$RCSfile: makefile,v $ $Revision: 4.3.5.3 $ (DEC) $Date: 1993/11/18 14:18:35 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

SCRIPTS			= ypxfr_1perday \
			  ypxfr_1perhour ypxfr_2perday 

SUBDIRS			= revnetgroup yppoll yppush \
			  ypxfr makedbm stdhosts src

ILIST			= ${DATAFILES} ${SCRIPTS}
 
IDIR			= /var/yp/
MRG_FILES		= Makefile
DATAFILES		= Makefile ${MRG_FILES/*/.mrg..&} 

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/scripts.mk


