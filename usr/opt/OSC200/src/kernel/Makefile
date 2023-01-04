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
# @(#)$RCSfile: Makefile,v $ $Revision: 4.6.31.11 $ (DEC) $Date: 1993/11/09 04:34:45 $
# 

IDFILE=\`genloc /src/setup/osf1_idlist\`
OWNER=`grep 'uid	bin' ${IDFILE} | awk '{ print $$3 }'`
GROUP=`grep 'gid	bin' ${IDFILE} | awk '{ print $$3 }'`

KERNEL_TOOLS =  KCC=${${OBJECT_FORMAT}_${CC_SUITE}_EXEC_PREFIX}cc \
		LD=${${OBJECT_FORMAT}_${CC_SUITE}_EXEC_PREFIX}ld \
		CPP=cpp \
		AWK=awk \
		AR=${${OBJECT_FORMAT}_${CC_SUITE}_EXEC_PREFIX}ar \
		RANLIB='${${OBJECT_FORMAT}_${CC_SUITE}_EXEC_PREFIX}ar ts' \
		SHELL=sh \
		SED=sed \
		SORT=sort \
		ECHO=echo \
		KSH=ksh \
		EGREP=egrep \
		GREP=grep \
		LINT=lint

CONFIG=${KERNEL_${TARGET_MACHINE}_CONFIG?${KERNEL_${TARGET_MACHINE}_CONFIG}:${KERNEL_CONFIG?${KERNEL_CONFIG}:}}

#
# Set RT in the environment to build REAL TIME kernel's
#

ALPHA_KERNELS   = RUBY COBRA FLAMINGO PELICAN JENSEN
MIPS_KERNELS    =
OTHERS		= ${RT?BINARY.rt GENERIC.rt:BINARY GENERIC SAS} ${CONFIG} \
		  ${${TARGET_MACHINE}_KERNELS} 


SUBDIRS 	= include arch bsd io data vfs kern net sec streams streamsm \
		  src conf dli dec/sas/${target_machine}

KIDIR_LIST	= ./BINARY ./BINARY.rt ./SAS ./streamsm
KILIST		= GENERIC GENERIC.rt SAS ${${TARGET_MACHINE}_KERNELS}

IDIR		= /
SYMLINKS	= sys
SYMLINKDIR1	= usr
SYMLINKDIR2	= /

install:	install_all kernel_install

BINARY BINARY.rt: $$@_config $$@_scalls $$@_depend \
	 	  ${MAKE_ARGS?:$$@_libs} $$@_vmunix

GENERIC GENERIC.rt SAS ${${TARGET_MACHINE}_KERNELS} ${CONFIG}: \
	$$@_config $$@_depend $$@_vmunix

vmunix: ${CONFIG}_vmunix
depend:	${CONFIG}_depend
scalls:	${CONFIG}_scalls
relink:	${CONFIG}_relink
clean:  ${CONFIG}_clean
config: ${CONFIG}_config
libs: ${RT?BINARY.rt:BINARY}_libs

std:
	exec ${MAKE} ${MFLAGS} ${MAKE_ARGS} SUBDIRS="" build_all

rt:   
	exec ${MAKE} ${MFLAGS} ${MAKE_ARGS} RT=1 SUBDIRS="" build_all

setup:	
	exec ${MAKE} ${MFLAGS} ${MAKE_ARGS} OTHERS="" build_all

%_%2: ${ALWAYS}
	@if [ "%" = "" ] ;\
	then echo 'error: KERNEL_CONFIG not set by default.'; exit 1; \
	else \
	echo "cd % && ${MAKE} ${MFLAGS} ${MAKE_ARGS} ${KERNEL_TOOLS} CONFIG=% %2" ;\
	cd % && ${MAKE} ${MFLAGS} ${MAKE_ARGS} ${KERNEL_TOOLS} CONFIG=% %2 ;\
	fi

%_config:
	@if [ "%" = "" ] ;\
	then echo 'error: KERNEL_CONFIG not set by default.'; exit 1; \
	else \
	echo "cd conf && ${MAKE} ${MFLAGS} CONFIG_FLAGS="${CONFIG_FLAGS}" %_config" ;\
	cd conf && ${MAKE} ${MFLAGS} CONFIG_FLAGS="${CONFIG_FLAGS}" %_config ;\
	fi


%_libs:
	@echo "[ performing kernel library operations ]"
	@KTOOLS="${KERNEL_TOOLS}"; export KTOOLS;\
	KERNEL_CONFIG=%; export KERNEL_CONFIG; \
	if [ -d %/lib ]; \
	then \
		true; \
	else \
		mkdir -p %/lib; \
	fi;  \
	cd %/lib && ${MAKE} -f ../../lib/Makefile \
	${MFLAGS} ${MAKE_ARGS?${MAKE_ARGS}:libs} ${KERNEL_TOOLS}


%_install:
	cp GENERIC/vmunix ${TOSTAGE}/vmunix.OSF1.GENERIC
	if [ -f ${TOSTAGE}/vmunix.OSF1.% ];\
	then \
		true; \
	else \
	    ln -s ../obj/alpha/kernel/%/vmunix ${TOSTAGE}/vmunix.OSF1.% ;\
	fi


kernel_install: ${KILIST/*/&_install}
	ln -f ${TOSTAGE}/vmunix.OSF1.GENERIC ${TOSTAGE}/vmunix
	cp ${TOSTAGE}/vmunix ${TOSTAGE}/genvmunix
	tar -cf - ${KIDIR_LIST} | (cd ${TOSTAGE}/usr/sys; tar xpf - )
	(cd ${TOSTAGE}/usr/sys; ${CHOWN} -fR ${OWNER} ${KIDIR_LIST})
	(cd ${TOSTAGE}/usr/sys; ${CHGRP} -fR ${GROUP} ${KIDIR_LIST})


include ${MAKEFILEPATH}/standard.mk
include ${MAKEFILEPATH}/subdirs.mk
include ${MAKEFILEPATH}/others.mk
