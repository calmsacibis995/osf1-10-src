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
# @(#)$RCSfile: Makefile,v $ $Revision: 1.1.15.2 $ (DEC) $Date: 1993/03/15 20:44:54 $
#

# Top level kernel library makefile. 

# Default to 'BINARY' in all but one case. This variable used
# to determine which area from which to import language
# translator flags and some includes.

CONFIG  	= `if test "${KERNEL_CONFIG}" = "BINARY.rt"; \
                   then \
			 ${ECHO} "BINARY.rt"; \
		   else \
			 ${ECHO} "BINARY"; fi;`

MFLAGS		=  ${MFLAGS} "CONFIG=${CONFIG}"

# Setup paths for lower level makefile includes.

_PT_		= ../../../..

MPATHS		= _REL_=${_PT_:/..=} \
		  _FP_=.. 

PPATHS		= _REL_=${_PT_} \
		  _FP_=../..

LAYER		= hal

# Path to object side configuration file and machine
# specific files file.

CONFIG_FILE	= ${MAKETOP:../=}conf/${KERNEL_CONFIG}

MSPEC_FILES	= ${MAKETOP:../=}conf/${target_machine}/files

# Libraries representing individual workstation
# models, ex: DS5000_300, ALPHARUBY. This information
# is derived from the machine specific files file
# so as to represent what is intended to be built.

PLTFRM_LIBS 	= `@${AWK} '/^LIB\/lib/ {print $$1}'\
		   ${MSPEC_FILES} | ${SED} -e 's/LIB\///g' `

# Directories in addition to the generic architecture library 
# directory which is assumed to exist in every case. 

DIRS 		= hal `${AWK} '/^LIB\/lib/ {print $$1}'\
		  ${MSPEC_FILES} | ${SED} -e 's/LIB\/lib_//g' \
		  -e 's/\.[Oa]//`

# List of cpus from configuration file which will result in a 
# define in a given compile/assy string

CPUALL		= `${AWK} 'BEGIN {FS="\""} /^cpu/ \
		  {printf("%s ", $$2);}' ${CONFIG_FILE}`

all:  ${target_machine}_libs 
libs:  ${target_machine}_libs 

alpha_libs:
	@echo "Kernel libraries are not used in alpha."

mips_libs:  lib_${target_machine}.a 	\
	    lib_${LAYER}.a	\
	    $${PLTFRM_LIBS}		

lib_${target_machine}.a:
	@${ECHO} "[ generating ${TARGET?${TARGET}:$@} ]"
	-@if [ -d ${target_machine} ]; then true; else mkdir ${target_machine}; fi
	@CPULIST=${CPUALL}; export CPULIST;\
	cd ${target_machine} && exec ${MAKE} ${MFLAGS} -f ${_PT_:/..=}/lib/${target_machine}/Makefile \
	${TARGET?${TARGET}:${target_machine}.depend} ${MPATHS} ${KTOOLS}
	@CPULIST=${CPUALL}; export CPULIST;\
	cd ${target_machine} && exec ${MAKE} ${MFLAGS} -f ${_PT_:/..=}/lib/${target_machine}/Makefile \
	${TARGET?$@.X:} ${MPATHS} ${KTOOLS}

lib_%.a lib_%.O: 
	@${ECHO} "[ generating ${TARGET?${TARGET}:$@} ]"
	-@if [ -d ${target_machine}/% ]; then true; else mkdir -p ${target_machine}/%; fi
	@CPULIST=${CPUALL}; export CPULIST;\
	cd ${target_machine}/% && exec ${MAKE} ${MFLAGS} -f ${_PT_}/lib/${target_machine}/%/Makefile \
	${TARGET?${TARGET}:%.depend}  ${PPATHS} ${KTOOLS} 
	@CPULIST=${CPUALL}; export CPULIST;\
	cd ${target_machine}/% && exec ${MAKE} ${MFLAGS} -f ${_PT_}/lib/${target_machine}/%/Makefile \
	${TARGET?$@.X:} ${PPATHS} ${KTOOLS} 

clean_${target_machine}:
	@${ECHO} "[ cleaning ${target_machine} library ]"
	@cd ${target_machine} && rm -rf *.depend  *.[oaO] ;

clean_%:
	@${ECHO} "[ cleaning % library ]"
	@cd ${target_machine}/% && rm -rf *.depend  *.[oaO] ;

cleanall:
	@${ECHO} "[ cleaning kernel libraries ]"
	@cd ${target_machine} ; ${ECHO} "+cleaning ${target_machine}"; rm -rf *.depend  *.[oaO] ;
	-@for dir in ${DIRS} ;\
	do \
		(cd ${target_machine}/$$dir ; ${ECHO} "+cleaning $$dir"; rm -rf *.depend  *.[oaO] ;) \
	done

check:
	@${ECHO} "[ current library inventory: ]"
	@ls -l ${target_machine}/*.[aO]
	-@for dir in ${DIRS} ;\
        do \
                ls -l  ${target_machine}/$$dir/*.[aO] ;  \
        done
	
FORCE:
