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
#	@(#)$RCSfile: Makefile,v $ $Revision: 1.2.9.3 $ (DEC) $Date: 1993/10/19 21:54:21 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#

VPATH			= ${MAKETOP}/kernel/io/dec/ws

DATAFILES               = bt459.h bt431.h bt463.h pmagro.h \
                          i2c.h vsxxx.h tablet.h \
                          sfbparams.h sfbregs.h pmagbba.h \
			  stamp.h pa.h pq.h px.h pv.h vsxxx.h \
			  lk201.h pcxal.h pcxas.h vga.h vgareg.h qvision.h

ILIST   = ${DATAFILES}
IDIR    = /usr/sys/include/io/dec/ws/
IMODE   = 444
IGROUP  = bin
IOWNER  = bin

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk

