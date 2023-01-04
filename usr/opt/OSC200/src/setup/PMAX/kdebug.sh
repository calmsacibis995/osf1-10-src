#!/bin/sh
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

# This is the script for building kdebug for breakpoint kernel debugging.
# This script should be run after the "setup.sh" script has
# completed successfully.  The environment used within this script has
# been created by "understanding" all of the interdependencies between
# components and their environment.

if [ -r setup/PMAX/host.sh ]
then
    . setup/PMAX/host.sh
else
	case `/bin/uname` in 
		ULTRIX)	. setup/PMAX/ultrix.sh;; 
		*)    . setup/PMAX/osf1.sh;;
	esac
fi

#
# Setup immediate directory heirarchy
#
srcdir=`pwd`
base=`expr "${srcdir}" : "\(.*\)/.*"`
if [ `basename "${srcdir}"` != "src" ]
then
    echo "Please move source tree into a \"src\" subdirectory"
    exit 1
fi
obj=${base}/obj
if [ -d ${obj} ]
then
    true
else
    echo "Please create the path for ${obj}"
    exit 1
fi
objdir=${obj}/${target_machine}
if [ -d ${objdir} ]
then
    true
else
    echo "Please create the path for ${objdir}"
    exit 1
fi
exp=${base}/export
if [ -d ${exp} ]
then
    true
else
    echo "Please create the path for ${exp}"
    exit 1
fi
expdir=${exp}/${target_machine}
if [ -d ${expdir} ]
then
    true
else
    echo "Please create the path for ${expdir}"
    exit 1
fi
tools=${base}/tools
if [ -d ${tools} ]
then
    true
else
    echo "Please create the path for ${tools}"
    exit 1
fi
toolsdir=${tools}/${target_machine}
if [ -d ${toolsdir} ]
then
    true
else
    echo "Please create the path for ${toolsdir}"
    exit 1
fi
bindir=${toolsdir}/bin
if [ -d ${bindir} ]
then
    true
else
    echo "Please create the path for ${bindir}"
    exit 1
fi
hostbindir=${toolsdir}/hostbin
if [ -d ${hostbindir} ]
then
    true
else
    echo "Please create the path for ${hostbindir}"
    exit 1
fi
libdir=${toolsdir}/lib
if [ -d ${libdir} ]
then
    true
else
    echo "Please create the path for ${libdir}"
    exit 1
fi
# Note: why do they do this, then overwrite it?
#if [ "${CMD_OBJECT_FORMAT}" = "MACHO" ]
#then
#    gccdir=${toolsdir}/macho
#else
#    gccdir=${toolsdir}/gcc
#fi
#if [ -d ${gccdir} ]
#then
#    true
#else
#    echo "Please create the path for ${gccdir}"
#    exit 1
#fi
if [ "${KERNEL_OBJECT_FORMAT}" = "MACHO" ]
then
    gccdir=${toolsdir}/macho
else
    if [ "${C_COMPILER}" = "mips_cc" ]
	then
	    gccdir=${toolsdir}/mcc
	    COMP_HOST_ROOT=${gccdir}/
	    COMP_TARGET_ROOT=${gccdir}/
	    export COMP_HOST_ROOT COMP_TARGET_ROOT
	else
	    gccdir=${toolsdir}/gcc
	fi
fi
if [ -d ${gccdir} ]
then
    true
else
    echo "Please create the path for ${gccdir}"
    exit 1
fi

OBJECT_FORMAT="${KERNEL_OBJECT_FORMAT}"

export OBJECT_FORMAT

if [ "${OBJECT_FORMAT}" = "MACHO" ]
then
    MACHO_GCC_EXEC_PREFIX="${gccdir}/"
    export MACHO_GCC_EXEC_PREFIX
else
    COFF_GCC_EXEC_PREFIX="${gccdir}/"
    export COFF_GCC_EXEC_PREFIX
    if [ "${CMD_OBJECT_FORMAT}" = "MACHO" ]
    then
	MACHO_GCC_EXEC_PREFIX="${toolsdir}/macho/"
	export MACHO_GCC_EXEC_PREFIX
    fi
fi

#
# Site/Environment stuff
#
SITE="OSF"
OWNER="bin"
GROUP="bin"

export SITE OWNER GROUP


#
# Constrain search paths
#
PATH="${hostbindir}:${bindir}:${gccdir}:/usr/ucb:/bin:/usr/bin"
INCDIRS="-I${expdir}/usr/include"
LIBDIRS="-L${expdir}/usr/ccs/lib"
SHLIBDIRS="-L${expdir}/usr/shlib"
MIGCOM="${libdir}/migcom"
LEXER="${libdir}/ncform"
YACCPAR="${libdir}/yaccpar"
NO_SHARED_LIBRARIES=1

export PATH INCDIRS LIBDIRS SHLIBDIRS MIGCOM LEXER YACCPAR NO_SHARED_LIBRARIES

OBJECT_FORMAT="${CMD_OBJECT_FORMAT}"

export OBJECT_FORMAT

#
# New build environment definitions
#
MAKEFILEPATH='${MAKETOP}usr/lib/makefiles'
EXPORTBASE=${expdir}
SOURCEBASE=${srcdir}
SOURCEDIR=""
OBJECTDIR=../obj/${target_machine}

export MAKEFILEPATH EXPORTBASE SOURCEBASE SOURCEDIR OBJECTDIR

(cd kernel/dec/debug/kdebug; make -ckEF)

