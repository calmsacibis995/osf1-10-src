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
#	@(#)$RCSfile: objects.mk,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:33:24 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
# definitions for build
#
_ALL_OFILES_=${OFILES?${OFILES}:${_PROG_OFILES_:u}}

_OFILES_=${OFILES?${OFILES}:${${@}_OFILES?${${@}_OFILES}:${BINARIES?$@.o:}}}

#
# definitions for clean
#
_CLEAN_OFILES_=${OFILES?${OFILES}:${$%_OFILES?${$%_OFILES}:${BINARIES?$%.o:}}}

#
# definitions for lint
#
_LINT_OFILES_=${OFILES?${OFILES}:${$%_OFILES?${$%_OFILES}:${PROGRAMS?$%.o:}}}

#
# definitions for tags
#
_TAGS_OFILES_=${OFILES?${OFILES}:${$%_OFILES?${$%_OFILES}:${PROGRAMS?$%.o:}}}

#
#  Definitions for using xstr
#
_XSTR_C_O_=${XSTR_OFILES?%_x.o:_XSTR_C_O_}

#
#  Default double suffix compilation rules
#
${_XSTR_C_O_}: %.c
	${_CC_} -E ${_CCFLAGS_} %.c | ${XSTR} -c -
	${_CC_} ${_CCFLAGS_} -c x.c
	${MV} -f x.o %_x.o
	${RM} -f x.c

.y.o:
	${YACC} ${_YFLAGS_} $<
	${_CC_} ${_CCFLAGS_} -c y.tab.c
	-${RM} -f y.tab.c
	${MV} -f y.tab.o $@

.y.c:
	${YACC} ${_YFLAGS_} $<
	${MV} -f y.tab.c $@

.l.o:
	${LEX} ${_LFLAGS_} $<
	${_CC_} ${_CCFLAGS_} -c lex.yy.c
	-${RM} -f lex.yy.c
	${MV} -f lex.yy.o $@

.p.o:
	${_PC_} -c ${_PCFLAGS_} $*.p

.s.o:
	${_AS_} -o $*.o ${_ASFLAGS_} $*.s

.c.pp:
	${_CC_} ${_CCFLAGS_} -E $< >$@

${_ALL_OFILES_}: ${HFILES}

${_MIG_HDRS_}: $${@:.h=.defs}
	${MIG} ${_MIGFLAGS_} ${@:.h=.defs} -server /dev/null -user /dev/null

${_MIG_USRS_}: $${@:User.c=.defs}
	${MIG} ${_MIGFLAGS_} ${@:User.c=.defs} -server /dev/null

${_MIG_SRVS_}: $${@:Server.c=.defs}
	${MIG} ${_MIGFLAGS_} ${@:Server.c=.defs} -user /dev/null
