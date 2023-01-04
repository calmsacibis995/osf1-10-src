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
# HISTORY
#
# --------------------------------------------------------- 
# | Copyright (c) 1986, 1989 MIPS Computer Systems, Inc.  | 
# | All Rights Reserved.                                  | 
# --------------------------------------------------------- 
# $Header: /usr/sde/osf1/rcs/os/src/usr/shlib/libm/Makefile,v 1.1.11.6 1993/07/23 13:52:32 Brett_Sampson Exp $

VPATH                   = ${MAKETOP}/usr/ccs/lib/libm/${target_machine}
INOBJECTDIR		= libm.a
INOBJECTDIR_OPTIONS	=
EXPSHLIB_TARGETS         = export_libm.so

ILIST                   = libm.so
IDIR                    = /usr/shlib/
IMODE                   = 644
LDFLAGS         = -S 
LIBS            = -lc 

include ${MAKEFILEPATH}/rules.mk

.INOBJECTDIR: ${_INOBJECTDIR_}
	${CP} -p $@ ./$@.X
	${MV} ./$@.X ./$@
	chmod u+w ./$@

libm.so: libm.a
	${_LD_} ${_SHLDFLAGS_} -o $@.X -all libm.a  ${_LIBS_}
	${MV} $@.X $@





