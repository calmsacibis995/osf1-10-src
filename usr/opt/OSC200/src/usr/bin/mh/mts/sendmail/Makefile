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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/09/29 13:01:47 $  
#
Options = -DSYS5DIR -D_BSD -DISI -DMHRC -DWHATNOW -DMHE -DBSD42 -DBERK \
          -DBSD43 -DLOCKF -DNETWORK -DBIND -DRPATHS -DDPOP -DRPOP -DUK \
          -DFOLDPROT='"0700"' -DMSGPROT='"0600"' -DTYPESIG='void' \
          -DPOPSERVICE='"pop"' -DSPRINTF='int' -DSENDMTS -DSMTP -DPOP -DX400
INCFLAGS = -I..

IDIRS	= /usr/lib/mh
IFILES	= bboardsMH.m4 popMH.m4

CFLAGS = ${CFLAGS} ${Options}
LIBRARIES = libsendmts.a
OFILES = hosts.o smail.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/libs.mk
include ${MAKEFILEPATH}/objects.mk
