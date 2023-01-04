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
#	@(#)$RCSfile: Makefile,v $ $Revision: 4.3.28.11 $ (DEC) $Date: 1993/07/09 21:14:53 $
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
# OSF/1 Release 1.0

CONFIG=${KERNEL_${TARGET_MACHINE}_CONFIG?${KERNEL_${TARGET_MACHINE}_CONFIG}:${KERNEL_CONFIG?${KERNEL_CONFIG}:}}

VPATH 		= ${target_machine}
alpha_KERNELS	= FLAMINGO COBRA RUBY PELICAN JENSEN
mips_KERNELS	=

CONFIGFILES	= BINARY GENERIC BINARY.rt GENERIC.rt SAS \
		  ${${target_machine}_KERNELS} ${CONFIG}

DATAFILES	= files template.std.mk param.c filesystems newvers.sh \
		  version.banner version.build version.major version.minor \
		  version.patch version.variant copyright template.mk \
		  version.type ${CONFIGFILES} .mrg..files.dat

SCRIPTS		= .mrg..files .mrg..param.c
.mrg..param.c_SED_OPTIONS = -e 's;^#;&;' -e t

ILIST		= ${DATAFILES} ${SCRIPTS}
IDIR		= /usr/sys/conf/
IMODE		= 444
IOWNER		= bin
IGROUP		= bin
SYMLINKS	= config
SYMLINKDIR1	= ../bin
SYMLINKDIR2	= /usr/sys/conf
${target_machine}_SYMLINK = ${MACHINE}
config_SYMLINKDIR1 = ../bin
.mrg..config_IMODE	= 744
.mrg..files_IMODE	= 744
.mrg..param.c_IMODE	= 744

SUBDIRS		= ${target_machine}
OTHERS		= ${CONFIGFILES/*/&_config}

${MACHINE}:
	@-if [ -r ./${MACHINE} ]; then true; \
        else ln -s ./${target_machine} ${MACHINE}; fi

config:	${CONFIG}_config
%_config: ../%/Makefile 

../%/Makefile: % ../bin/config template.mk template.std.mk files\
	       ${target_machine}/template.mk ${target_machine}/files \
	       ../io/common/conf.c
	@-if [ -d ../% ]; then \
		true;\
	else \
		echo "[ setting up conf for % ]" ;\
	        mkdir ../%;\
	fi
	@echo "[ configuring % ]"
	@-if [ "%" = "BINARY" -o "%" = "BINARY.rt" ]; then \
		../bin/config -s -c . ${CONFIG_FLAGS} % ;\
	else \
		../bin/config ${CONFIG_FLAGS} % ;\
	fi
	@echo % >../%/vers.config 

include ${MAKEFILEPATH}/rules.mk
