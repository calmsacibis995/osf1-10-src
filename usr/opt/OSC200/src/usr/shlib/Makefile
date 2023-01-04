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
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# static char rcsid[] = "@(#)$RCSfile: Makefile,v $ $Revision: 4.2.21.10 $ (DEC) $Date: 1993/10/07 20:25:50 $";
#
# HISTORY
#
#
#       @(#)$RCSfile: Makefile,v $ $Revision: 4.2.21.10 $ (DEC) $Date: 1993/10/07 20:25:50 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
#       P. Derr  25 June, 1991          Add libdvs and libcda to SUBDIRS
#
#	V. Patel 31 June, 1991		Add libsys5 to SUBDIRS

SUBDIRS			= libc libm libcda libdvs \
			libsys5 libaud libsecurity \
			libchf libips libimg libids_nox \
			libdnet libc_r libmach DECthreads libcurses \
			libiconv libcmalib libmxr libtli libxti \
			libcdrom libretired_malloc ${target_machine}

EXPINC_SUBDIRS		= ${target_machine}
EXPSHLIB_SUBDIRS	= ${SUBDIRS}

include ${MAKEFILEPATH}/rules.mk
