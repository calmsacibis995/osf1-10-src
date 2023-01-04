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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2.17.4 $ (DEC) $Date: 1993/10/21 23:22:29 $
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
# Modification History
# --------------------
# 001	Fred C.		91/05/01
#     - added LDFLAGS = non-shared for Mips compiler changes
#
# 002	gws		91/05/30
#     - this came from usr/local/nosupport/term/Makefile -r3.2
#     - added this Modification history, including -r3.2 changes above
#     - changed mktab.o_INCFLAGS to point to ../../../bin/troff
#     - added `tabln03' to list of drivers to be built
#     - removed the non-Digital devices from list of those to be
#	built, so only tablpr, and tabln03 remain.
#     - install the driver sources, host machine Makefile, chartst nroff
#	  test file
#     - removed ILINKS for the non-Digital devices
#     - added tablpr_ILINKS for tablp, tabcrt, tabvt100
#     - added IMODE for drivers*.c, mktab.o, host machine Makefile, chartst
#
# 003	gws		91/10/29
#	link "vt100" to "ln03", instead of to "crt", because VT100(tm) series
#	terminals's are a lot like LN03(tm) printers instead of being a lot
#	like dumb crt's
#		VT100 and LN03 are trademarks of
#			Digital Equipment Corporation, Inc.
#
# 004	gws		91/10/29
#	link "lp" to "vt100", so that man(1) and catman(1) will generate
#	manpages for VT100 terminals
#		

OBJECTS			= tablpr \
			  tabln03

DATAFILES		= README mktab.o chartst Makefile.install \
			  ${OBJECTS:=.c}

ILIST			= ${OBJECTS} ${DATAFILES}
IDIR			= /usr/share/lib/term/

#
tablpr_ILINKS		= ${IDIR}tabcrt					# 004
tablpr.c_IMODE		= 644
tabln03.c_IMODE		= 644
#
# default man(1)/catman(1) device "lp" is linked to "vt100"		# 004
tabln03_ILINKS		= ${IDIR}tabvt100 ${IDIR}tablp			# 004
#
README_IMODE		= 644
mktab.o_IMODE		= 444
Makefile.install_IMODE	= 644
chartst_IMODE		= 644

GARBAGE			= ${OBJECTS:=.o} a.out Makefile*
CLEANFILES		= ${OBJECTS:=*} a.out ${DATAFILES} Makefile*

#CCTYPE			= host
LDFLAGS			= -non_shared
mktab.o_INCFLAGS	= -I../../../bin/troff

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/datafiles.mk

${OBJECTS}: mktab.o $${@:=.o}
	${RM} -f $@
	${_CC_} ${_LDFLAGS_} mktab.o ${@:=.o}
	./a.out > $@
	${RM} -f ./a.out

