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
#	@(#)$RCSfile: programs.mk,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/05 16:15:18 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
# definitions for build
#
_PROGRAMS_=${PROGRAMS?${PROGRAMS}:_PROGRAMS_}
_PROG_OFILES_=${PROGRAMS/*/${&_OFILES?${&_OFILES}:&.o}}

#
#  Definitions for default .c.o rules used by programs or libraries
#
_PROGRAM_C_O_=${PROGRAMS?.c.o:${OBJECTS?.c.o:_PROGRAM_C_O_}}
_PROGRAM_P_O_=${PROGRAMS?.p.o:${POBJECTS?.p.o:_PROGRAM_P_O_}}
_PROGRAM_S_O_=${PROGRAMS?.s.o:${ASSEMBLY_OBJECTS?.s.o:_PROGRAM_S_O_}}

#
#  Default single suffix compilation rules
#
.c:
	${_CC_} ${_LDFLAGS_} ${_CCFLAGS_} -o $*.X $*.c ${_LIBS_}
	${MV} $*.X $@

.p:
	${_PC_} ${_LDFLAGS_} ${_PCFLAGS_} -o $*.X $*.p ${_LIBS_}
	${MV} $*.X $@

.y:
	${YACC} ${_YFLAGS_} $<
	${_CC_} ${_LDFLAGS_} ${_CCFLAGS_} -o $*.X y.tab.c
	-${RM} -f y.tab.c
	${MV} -f $*.X $@

.l:
	${LEX} ${_YFLAGS_} $<
	${_CC_} ${_LDFLAGS_} ${_CCFLAGS_} -o $*.X lex.yy.c
	-${RM} -f lex.yy.c
	${MV} -f $*.X $@

#
#  Default double suffix compilation rules
#
${_PROGRAM_C_O_}:
	${_CC_} -c ${_CCFLAGS_} $*.c

${_PROGRAM_S_O_}:
	${_AS_} -o $*.o ${_ASFLAGS_} $*.s

${_PROGRAM_P_O_}:
	${_PC_} -c ${_PCFLAGS_} $*.p

#
#  Build rules
#
${_PROGRAMS_}: ${DEPEND_LIST} $${_OFILES_}
	${_CC_} ${_LDFLAGS_} -o $@.X ${_OFILES_} ${_LIBS_}
	${MV} $@.X $@
