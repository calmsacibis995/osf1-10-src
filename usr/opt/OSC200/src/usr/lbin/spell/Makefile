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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:24:23 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
# 
# 
# OSF/1 Release 1.0

#

#

PROGRAMS		= spellprog hashcheck hashmake spellinprg spellout
SCRIPTS			= compress
DATAFILES		= american british extra list local \
			  spellhist stop Mkf.native
MSGHDRS			= spell_msg.h
CATFILES		= spell.cat

ILIST			= ${PROGRAMS} ${SCRIPTS} ${DATAFILES}
IDIR			= /usr/lbin/spell/
BIN_IDIR		= /usr/bin/
spellout_IDIR		= ${BIN_IDIR}

HFILES			= ${MSGHDRS}
spellprog_OFILES	= spellprog.o hashlook.o hash.o huff.o
spellinprg_OFILES	= spellinprg.o huff.o
spellout_OFILES		= spellout.o hashlook.o hash.o huff.o
hashcheck_OFILES	= hashcheck.o hash.o huff.o
hashmake_OFILES		= hashmake.o hash.o

include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/programs.mk
include ${MAKEFILEPATH}/objects.mk
include ${MAKEFILEPATH}/scripts.mk
include ${MAKEFILEPATH}/datafiles.mk

hash.o: hash.h
huff.o: huff.h
spellout.o: spell_msg.h
spellinprg.o: spell_msg.h
