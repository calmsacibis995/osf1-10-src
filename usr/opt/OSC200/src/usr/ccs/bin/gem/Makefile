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
# --------------------------------------------------------- 
# | Copyright (c) 1986, 1989 MIPS Computer Systems, Inc.  | 
# | All Rights Reserved.                                  | 
# --------------------------------------------------------- 
# $Header: /usr/sde/osf1/rcs/os/src/usr/ccs/bin/gem/Makefile,v 1.1.2.2 1993/04/23 20:50:37 Brett_Sampson Exp $
VPATH                   = ${target_machine}
OTHERS                  = COPY_gemc_cc

ILIST                   = gemc_cc
IDIR                    = /usr/ccs/lib/cmplrs/cc$(REL)/
IMODE                   = 555

COPY_%:%.Z
	zcat %.Z > ./%.X; ${MV} ./%.X ./%; chmod u+w ./%

include ${MAKEFILEPATH}/rules.mk
