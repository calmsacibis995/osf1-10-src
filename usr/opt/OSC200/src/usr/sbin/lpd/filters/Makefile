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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.7.2 $ (DEC) $Date: 1993/12/21 20:37:44 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

PROGRAMS		= lpf lqf ln03rof lj250of ln03of
SCRIPTS			= xf 

ILIST			= ${PROGRAMS} ${SCRIPTS}
IDIR			= /usr/lbin/

INCFLAGS		= -I../filterlib
LIBFLAGS		= -L../filterlib
LIBS			= -lfilter

ln03of_OFILES		= decuniversal_of.o
decuniversal_of.o_CFLAGS = -DLN03OF="\"ln03of\"" -DLG31OF="\"lg31of\"" \
			   -DLG02OF="\"lg02of\"" -DLG06OF="\"lg06of\"" \
			   -DLA75OF="\"la75of\"" \
			   ${CFLAGS}
 

ln03of_ILINKS		= ${IDIR}lg31of ${IDIR}lg02of ${IDIR}lg06of \
			  ${IDIR}la75of ${IDIR}ln05of ${IDIR}ln06of \
			  ${IDIR}ln07of ${IDIR}ln08of \
			  ${IDIR}ansi_3of ${IDIR}ansi_2of

ln03rof.o_CFLAGS   	= -DLN03ROF_DECMCS="\"ln03rof_decmcs\"" \
			  -DLN03ROF_ISOLATIN1="\"ln03rof_isolatin1\"" \
			  ${CFLAGS}

ln03rof_ILINKS		= ${IDIR}ln03rof_decmcs ${IDIR}ln03rof_isolatin1 \
			${IDIR}ln05rof ${IDIR}ln05rof_decmcs ${IDIR}ln05rof_isolatin1 \
			${IDIR}ln06rof ${IDIR}ln06rof_decmcs ${IDIR}ln06rof_isolatin1 \
			${IDIR}ln07rof ${IDIR}ln07rof_decmcs ${IDIR}ln07rof_isolatin1 \
			${IDIR}ln08rof ${IDIR}ln08rof_decmcs ${IDIR}ln08rof_isolatin1 \
			${IDIR}ps_level1of ${IDIR}ps_level1of_decmcs \
			${IDIR}ps_level1of_isolatin1 ${IDIR}ps_level2of \
			${IDIR}ps_level2of_decmcs ${IDIR}ps_level2of_isolatin1 

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
