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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

# This is the script for building the kernel/dec/sas component of the
# snapshot.  This script should be run after the "setup.sh" script has
# completed successfully.  The environment used within this script has
# been created by "understanding" all of the interdependencies between
# components and their environment.  When porting the sources to a
# "unknown" machine, this script is the place to start making changes.
# Good Luck!
#

#
# REVISION HISTORY:
#

#
# Read the configuration information for this host
#
if [ -r setup/alpha/host.sh ]
then
    . setup/alpha/host.sh
else
	case `/bin/uname` in 
		ULTRIX)	. setup/alpha/ultrix.sh;; 
		*)    . setup/alpha/osf1.sh;;
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
# FARKLE: why do they do this, then overwrite it?
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
    if [ "${C_COMPILER}" = "alpha_cc" ]
	then
	    gccdir=${toolsdir}/acc
	    COMP_HOST_ROOT=/
	    COMP_TARGET_ROOT=/
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
# Paste on "a" to get "acc" for alpha, otherwise it uses "cc"
    COFF_GCC_EXEC_PREFIX="${gccdir}/a"
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
if [ ${target_os_type} = osf ]; then
    PATH="/usr/ccs/bin:/usr/sde/tools/alpha_osf/bin:${gccdir}:/sbin:/usr/bin"
else
    PATH="${hostbindir}:${bindir}:${gccdir}:/usr/ucb:/bin:/usr/bin"
fi
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

#
# build requested target
#
if [ $# != 2 ]
then
    echo "stand.sh <build rule> <destroot>"
    exit 1
fi

export MAKEFILEPATH SOURCEBASE SOURCEDIR OBJECTDIR

(cd kernel/dec/sas/alpha; make -ckEF $1 DESTROOT=$2)

