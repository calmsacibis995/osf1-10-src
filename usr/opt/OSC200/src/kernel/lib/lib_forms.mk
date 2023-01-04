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
# @(#)$RCSfile: lib_forms.mk,v $ $Revision: 1.1.3.6 $ (DEC) $Date: 1992/09/29 14:10:25 $
#

# Contains forms common to all kernel library makefiles. 

MAKEFILE_CONFIG		= ${_REL_}/${CONFIG}/Makefile
CONFIG_DIR		=  ${MAKEFILE_CONFIG:H}
IDIRS			=  -I${CONFIG_DIR}  -I${_REL_}/include

SFILES			= ${ASSEMBLY_OBJECTS:.o=.s}
CFILES			= ${COBJECTS:.o=.c}

include ./lib_import.mk

LIBM_OFILES		= ${COBJECTS} ${ASSEMBLY_OBJECTS}

CPU_DEF                 = ${CPU_DEF?${CPU_DEF}:${CPU_ALL}
COPTS			= ${IMPT_CFLAGS} ${IDIRS}  ${CPU_DEF} -DBINARY
SOPTS			= ${IMPT_SFLAGS} ${IDIRS}  ${CPU_DEF} -DBINARY

CSTR			= ${CCOPTS?${CCOPTS}:${COPTS}} ${XFLAGS}
SSTR			= ${SSOPTS?${SSOPTS}:${SOPTS}} ${XFLAGS}

MACHINE_DEPEND          = ${MAKETOP:../../=}${OBJECTDIR}${MAKEDIR}
PLATFORM_DEPEND         = ${MAKETOP:../=}${OBJECTDIR}${MAKEDIR}

# Create a seperate dependency file for each library on the first
# invocation of make(1). Include the dependency file on the second
# invocation. Update the appropriate dependency file when a change
# has occurred to a source file whose object is a member of a
# library.

${LIBR}.depend:	 ${CFILES} ${SFILES} ${MAKEFILE_CONFIG}
	@${ECHO} "[ checking for dependancy information ]"
	-@if test -f ./${LIBR}.depend; \
		then \
			 ${RM} ./${LIBR}.depend; \
		else \
			 exit 0; fi
	@${KCC} -M ${COPTS} ${XFLAGS} ${CFILES} ${SFILES} | \
	${SORT} -u |  \
	${AWK} ' { if ($$1 != prev) { \
 			print rec; rec = $$0; prev = $$1; \
 		} else { \
 			if (length(rec $$2) > 78) { \
 				print rec; rec = $$0; \
 			} else \
 				rec = rec " " $$2 \
 		} } \
	      END { print rec } ' >> ${LIBR}.depend
	@${ECHO} "[ creating ${LIBR} library macros ]"
	@${ECHO} "CCOPTS = " ${COPTS} >> ${LIBR}.depend
	@${ECHO} "SSOPTS = " ${SOPTS} >> ${LIBR}.depend


# Dependency on ${MAKEFILE_CONFIG} for flag derivation.
# Output piped through ${SHELL} to expand flags.

..s.o:   ${MAKEFILE_CONFIG} 
	${SSOPTS?:@${ECHO}} ${KCC} ${SSTR} $< ${SSOPTS?: | ${SHELL} -v} 

.c.o:   ${MAKEFILE_CONFIG} 
	${CCOPTS?:@${ECHO}} ${KCC} ${CSTR} $< ${CCOPTS?: | ${SHELL} -v} 

# Archive the target library. When an object module is newer than
# the contained module, replace it.

lib_${LIBR}.a: ${LIBM_OFILES}
	@TMPDIR=.; export TMPDIR; \
        ${AR} cruv $@ $? ; \
        ${RANLIB} $@

lib_${LIBR}.O: ${LIBM_OFILES}
#	${LD} -r -o $@ ${LIBM_OFILES}
	${AR} cruv $@ ${LIBM_OFILES}
	${RANLIB} $@

# Targets to support manual translation of c and assembly targets
# with subsequent relink or rearchiving without dependency on 
# other members of a given library.


lib_${LIBR}.a.X: 
	@${ECHO} "[ creating ${@:.X=} ]"
	@for file in ${LIBM_OFILES} ; \
        do \
		if  test -f ./$$file ; \
			then true; \
		else \
			${ECHO} "$$file not found" ; \
			${ECHO} "library membership incomplete" ; \
			exit 1; \
		fi \
        done 
	@TMPDIR=.; export TMPDIR; \
        ${AR} cruv ${@:.X=} ${LIBM_OFILES}; \
        ${RANLIB} ${@:.X=}

lib_${LIBR}.O.X:  
	@${ECHO} "[ creating ${@:.X=} ]"
	@for file in ${LIBM_OFILES} ; \
        do \
		if  test -f ./$$file ; \
			 then true; \
		else \
			${ECHO} "$$file not found" ; \
			${ECHO} "library membership incomplete" ; \
			exit 1; \
		fi \
        done 
	${LD} -r -o ${@:.X=} ${LIBM_OFILES}

