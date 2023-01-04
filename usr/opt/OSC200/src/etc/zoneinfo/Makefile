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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1992/12/14 17:08:12 $
#

PROGRAMS      =	zdump zic

DATAFILES     =	africa asia australasia etcetera europe factory northamerica \
       		southamerica systemv

SCRIPTS	      = yearistype

ILIST         =	zdump zic ${DATAFILES} yearistype 
IDIR          =	/etc/zoneinfo/sources/
IMODE         =	644

zdump_IDIR    =	/usr/bin/
zdump_IMODE   =	755
zdump_OFILES  =	zdump.o ialloc.o

zic_IDIR      =	/usr/bin/
zic_IMODE     =	755
zic_OFILES    =	zic.o ialloc.o scheck.o

yearistype_IDIR	= /usr/bin/
yearistype_IMODE= 755

install_all: install_zic

install_zic:
	(if [ "${machine}" = "alpha" ] ; then /usr/bin/zic -d ${TOSTAGE}/etc/zoneinfo ${DATAFILES}; else /etc/zic -d ${TOSTAGE}/etc/zoneinfo ${DATAFILES} ;fi )
	mkdir ${TOSTAGE}/etc/zoneinfo/sources 

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk
