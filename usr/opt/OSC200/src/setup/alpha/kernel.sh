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
# OSF/1 Release 1.0

# This is the script for building the operating system component of the
# snapshot.  This script should be run after the "setup.sh" script has
# completed successfully.  The environment used within this script has
# been created by "understanding" all of the interdependencies between
# components and their environment.  When porting the sources to a
# "unknown" machine, this script is the place to start making changes.
# Good Luck!
#

#
# Read the configuration information for this host
#
if [ -r setup/alpha/host.sh ]
then
    . setup/alpha/host.sh
else
        case `/bin/uname` in
                ULTRIX) . setup/alpha/ultrix.sh;;
                *) . setup/alpha/osf1.sh;;
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
#afdfix: set for alpha compiler installed on host
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

#
# Constrain search paths
#
if [ ${target_os_type} = osf ]; then
    PATH="/usr/ccs/bin:/usr/sde/tools/alpha_osf/bin:${gccdir}:/sbin:/usr/bin"
else
    PATH="${hostbindir}:${bindir}:${gccdir}:/usr/ucb:/bin:/usr/bin"
fi
INCDIRS=""
#LIBDIRS=""
LIBDIRS="-L${expdir}/usr/ccs/lib"
SHLIBDIRS=""
MIGCOM="${libdir}/migcom"
LEXER="${libdir}/ncform"
YACCPAR="${libdir}/yaccpar"
NO_SHARED_LIBRARIES=

export PATH INCDIRS LIBDIRS SHLIBDIRS MIGCOM LEXER YACCPAR NO_SHARED_LIBRARIES

# Enable cc warnings when building config (can't disable kernel warnings).
WARN_FLAG=
export WARN_FLAG

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

# So template.std.mk knows if kernel building under kernel.sh or not.
KERNEL_SH=1
export KERNEL_SH

#
# New build environment definitions
#
MAKEFILEPATH='${MAKETOP}usr/lib/makefiles'
SOURCEBASE=${srcdir}
SOURCEDIR=""
OBJECTDIR=../obj/${target_machine}

export MAKEFILEPATH SOURCEBASE SOURCEDIR OBJECTDIR

#
# Build BINARY and the GENERIC kernel.
#
# Usage: kernel.sh, kernel.sh -option, kernel.sh CONFIG option1 ... optionN
#
# Note: need to remove NO_GCC
#

USAGE1="Usage: kernel.sh, kernel.sh [-setup] [-std] [-rt], or"
USAGE2="kernel.sh <CONFIG [setup][config][clean][depend][vmunix][relink][scalls][libs]>"

case $# in

     # Behaves like the old kernel.sh (i.e., kernel.sh with no arguments).
     # Same as kernel.sh -setup, then kernel.sh -std.
     0)	(cd kernel; make -ckEF NO_GCC=1 build_all);
	exit 0;;

     # Allows build standard and realtime kernels. Run kernel.sh -setup,
     # then kernel.sh -std or kernel.sh -rt.
     # Note: -std and -rt cannot be run concurrently.
     1)	if [ "$1" = "-setup" ]
	then
	    (cd kernel; make -ckEF NO_GCC=1 OTHERS="" all)
	    exit 0
	fi
     	if [ "$1" = "-std" ]
	then
	    (cd kernel; make -ckEF NO_GCC=1 SUBDIRS="" all)
	    exit 0
	fi
     	if [ "$1" = "-rt" ]
	then
	    RT=1
	    export RT
	    (cd kernel; make -ckEF NO_GCC=1 SUBDIRS="" all)
	    exit 0
	fi
     # If we get here, assume kernel.sh CONFIG [options], not kernel.sh -option.
     # Note: if invalid -option, we will take it as a CONFIG name (OPPS!)
     # Usage: modify kernel source file, kernel.sh BINARY (to update BINARY),
     # then kernel.sh CONFIG (to relink kernel). Normally, CONFIG = GENERIC.
     	KERNEL_CONFIG=$1;
	export KERNEL_CONFIG;
	if [ "$1" = "BINARY" -o "$1" = "BINARY.rt" ]
	then
	    (cd kernel; make -ckEF NO_GCC=1 OTHERS="" all)
  	    (cd kernel; make -ckEF NO_GCC=1 vmunix)
	else
    	    (cd kernel; make -ckEF NO_GCC=1 relink)
	fi
	exit 0;;

     *)	KERNEL_CONFIG=$1;
	export KERNEL_CONFIG;
	shift;
esac

# If we get here, assume kernel.sh CONFIG option1 option1 ... optionN.
# This allows execution of individual parts of the kernel build.
# Strictly "Caveat Emptor", would go on the unsupport kit (if we had one).
# Hint, refer to src/kernel/Makefile and src/kernel/conf/Makefile.
# Hint, options should be run in the order listed below.
while [ "$#" -gt 0 ]
do
    arg=$1
    shift
    case "$arg" in
	setup)	(cd kernel; make -ckEF NO_GCC=1 OTHERS="" all);;
 	config| clean| depend| vmunix| relink| scalls| libs)
 		 (cd kernel; make -ckEF NO_GCC=1 $arg);;
	*)	echo ${USAGE1};
		echo ${USAGE2};
		exit 1;;
    esac
done

exit 0
