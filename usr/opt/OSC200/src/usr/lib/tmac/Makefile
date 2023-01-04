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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.3.4 $ (DEC) $Date: 1992/09/30 14:00:43 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
# Modification History
# --------------------
# 001	gws	03-Jun-1991
#	added tmac.an.page, tmac.an.new.page, tmac.an.repro, tmac.an.new.repro
#	  macros
#
# 002   lw      06-Jun-1991
#       added rsml and sml macros
#
# 003   haslock 11-Jun-1991
#	added bookreader macro packages
#

DATAFILES		= tmac.an tmac.an.page tmac.an.repro \
			  tmac.e \
			  rsml sml \
			  tmac.s tmac.scover tmac.sdisp \
			  tmac.skeep tmac.srefs

ILIST			= ${DATAFILES}
IDIR			= /usr/share/lib/tmac/
IMODE			= 644

INOBJECTDIR_OPTIONS	= -e '/%beginstrip%/{' \
			  -e 'h' \
			  -e 's/.*/.\\" This version has had comments stripped; an unstripped version is available./p' \
			  -e 'g' \
			  -e '}' \
			  -e '/%beginstrip%/,$$s/[. 	][ 	]*\\".*//' \
			  -e '/^$$/d' \
			  -e '/\\n@/d'

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/datafiles.mk
