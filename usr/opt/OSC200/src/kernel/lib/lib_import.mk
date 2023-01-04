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
# @(#)$RCSfile: lib_import.mk,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/09/29 12:00:36 $
#
# This file handles the specification/importation of language
# translator flags for kernel library makefiles. 

PRFX			= 	3

# Importation of compiler/assembler flags.

CPU_FILTER		=  ${SED} -n '/IDENT/p'  \
			   | ${AWK} 'BEGIN { \
					split("${CPULIST}",arr," "); \
				} { \
					for(i = 1; i <= NF; i++)   { \
						found = 0; \
						for(j in arr) { \
							if(length($$i) > 2 &&  \
								substr($$i, "${PRFX}") == arr[j]) { \
								found = 1; \
								break; \
							}\
						}\
						if(found == 0) \
							print $$i; \
					} \
			  	} ' 

CPU_ALL			= `${ECHO} ${CPULIST} | ${AWK} '{ for (i = 1; i <= NF; i++)\
			   printf(" -D%s\n ", $$i);}' `


IMFL			= `cd ${_REL_}/${CONFIG}; ${MAKE} SRCFLAGS | ${CPU_FILTER}`
IMPT_SFLAGS		= $(IMFL:SRCFLAGS=asflags)
IMPT_CFLAGS		= $(IMFL:SRCFLAGS=ccnflags)



