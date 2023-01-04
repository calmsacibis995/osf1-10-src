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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/30 09:00:29 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
# Modification History
# --------------------
# 8/14/91 - Tom Peterson
#	- added ILINKS for s.acc, s.cov, s.eqn, s.ref, s.tbl, s.ths, s.toc
#
DATAFILES		= acc.ms cov.ms eqn.ms ref.ms tbl.ms ths.ms toc.ms

ILIST			= ${DATAFILES}
IDIR			= /usr/share/lib/ms/
IMODE			= 644

# hard links section
acc.ms_ILINKS		= ${IDIR}s.acc
cov.ms_ILINKS		= ${IDIR}s.cov
eqn.ms_ILINKS		= ${IDIR}s.eqn
ref.ms_ILINKS		= ${IDIR}s.ref
tbl.ms_ILINKS		= ${IDIR}s.tbl
ths.ms_ILINKS		= ${IDIR}s.ths
toc.ms_ILINKS		= ${IDIR}s.toc

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
