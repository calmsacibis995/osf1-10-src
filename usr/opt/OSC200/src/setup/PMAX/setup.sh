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
#
# @(#)setup.sh	3.2	(ULTRIX/OSF)	3/12/91
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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

# This is the setup script for building the source tree from scratch
# using as little as possible from the environment already installed on
# the current machine.  The basic process is to create the "environment"
# as we go along, which requires that this script "understand" all of the
# interdependencies between components and their environment.  When porting
# the sources to a "unknown" machine, this script is the place to start
# making changes.  Good Luck!
#

#
# Read the configuration information for this host
#
if [ -r setup/PMAX/host.sh ]
then
    . setup/PMAX/host.sh
else
        case `/bin/uname` in
                ULTRIX) . setup/PMAX/ultrix.sh;;
                *) . setup/PMAX/osf1.sh;;
        esac
fi
HOST_CC="cc -D__mips__ -D__MIPSEL__"
export HOST_CC
HOST_FLAGS="-D__mips__ -D__MIPSEL__"
export HOST_FLAGS
# If building on Digital's OSF/1, tell MIPS ld to build non-shared.
if [ `/bin/uname` != ULTRIX ]
then
	HOST_LDFLAGS="-non_shared"
	export HOST_LDFLAGS
fi
ULT_INCDIRS=
ULT_LIBDIRS=
export ULT_INCDIRS ULT_LIBDIRS

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
    mkdir ${obj}
fi
objdir=${obj}/${target_machine}
if [ -d ${objdir} ]
then
    true
else
    mkdir ${objdir}
fi
obj=${base}/hostobj
if [ -d ${obj} ]
then
    true
else
    mkdir ${obj}
fi
objdir=${obj}/${target_machine}
if [ -d ${objdir} ]
then
    true
else
    mkdir ${objdir}
fi
exp=${base}/export
if [ -d ${exp} ]
then
    true
else
    mkdir ${exp}
fi
expdir=${exp}/${target_machine}
if [ -d ${expdir} ]
then
    true
else
    mkdir ${expdir}
fi
exp=${base}/hostexp
if [ -d ${exp} ]
then
    true
else
    mkdir ${exp}
fi
expdir=${exp}/${target_machine}
if [ -d ${expdir} ]
then
    true
else
    mkdir ${expdir}
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
    mkdir ${bindir}
fi
hostbindir=${toolsdir}/hostbin
if [ -d ${hostbindir} ]
then
    true
else
    mkdir ${hostbindir}
fi
libdir=${toolsdir}/lib
if [ -d ${libdir} ]
then
    true
else
    mkdir ${libdir}
fi
if [ "${KERNEL_OBJECT_FORMAT}" = "MACHO" ]
then
    gccdir=${toolsdir}/macho
else
    gccdir=${toolsdir}/gcc
fi
if [ -d ${gccdir} ]
then
    true
else
    echo "Please create the path for ${gccdir}"
    exit 1
fi
if [ "${CMD_OBJECT_FORMAT}" = "MACHO" ]
then
    gccdir=${toolsdir}/macho
else
    gccdir=${toolsdir}/gcc
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
PATH="${hostbindir}:/usr/ucb:/bin:/usr/bin"

export PATH

OBJECT_FORMAT="${HOST_OBJECT_FORMAT}"

export OBJECT_FORMAT

CCTYPE=host

export CCTYPE

#
# Site/Environment stuff
#
CENV="-UMSG -D_BLD"
SITE="OSF"
OWNER="bin"
GROUP="bin"

export CENV SITE OWNER GROUP

#
# New build environment definitions
#
MAKEFILEPATH='${MAKETOP}usr/lib/makefiles'
SOURCEBASE=${srcdir}
SOURCEDIR=""
OBJECTDIR=../hostobj/${target_machine}

export MAKEFILEPATH SOURCEBASE SOURCEDIR OBJECTDIR

#
# bootstrap make program in current environment
#
for d in ${objdir}/usr ${objdir}/usr/ccs ${objdir}/usr/ccs/bin \
	   ${objdir}/usr/ccs/bin/make
do
    if [ -d ${d} ]
    then
        true
    else
        mkdir ${d}
    fi
    if [ -d ${d} ]
    then
        true
    else
	echo "Please create the path for ${d}"
	exit 1
    fi
done
cp usr/ccs/bin/make/* ${objdir}/usr/ccs/bin/make
(cd ${objdir}/usr/ccs/bin/make; sh -x ./bootstrap.sh)
cp ${objdir}/usr/ccs/bin/make/make ${hostbindir}/make
cp ${objdir}/usr/ccs/bin/make/environment.o ${libdir}/environment.o
rm -rf ${objdir}/usr/ccs

#
# create program to support object directory path searches
#
(cd usr/local/sdm; \
 make -cEF \
  SCRIPTS="genloc instdir mksymlinks"\
  genloc instdir mksymlinks)
cp ${objdir}/usr/local/sdm/genloc ${hostbindir}/genloc 
cp ${objdir}/usr/local/sdm/instdir ${hostbindir}/instdir 
cp ${objdir}/usr/local/sdm/mksymlinks ${hostbindir}/mksymlinks 

#
# create program to support object directory path searches
#
(cd usr/local/sdm/bin/genpath; \
 make -cEF CARGS="-DMACH -DCMU -D${SITE} -D_NO_PROTO" \
  _GENINC_="-I${objdir}/usr/include" _GENLIB_= \
  OFILES="genpath.o ${libdir}/environment.o" LIBS= \
  genpath)
cp ${objdir}/usr/local/sdm/bin/genpath/genpath ${hostbindir}/genpath
rm -f ${libdir}/environment.o

#
# program to create paths for files
#
(cd usr/local/sdm/bin/makepath; \
 make -cEF CARGS="-DMACH -DCMU -D${SITE}" \
  INCARGS="-I${objdir}/usr/include" LIBS= \
  makepath)
cp ${objdir}/usr/local/sdm/bin/makepath/makepath ${hostbindir}/makepath

#
# program to install in non-sandbox environment
#
(cd usr/local/sdm/bin/release; \
 make -cEF CARGS="-DMACH -DCMU -D${SITE}" \
  INCARGS="-I${objdir}/usr/include" \
  OFILES="release.o" LIBS= \
  release)
cp ${objdir}/usr/local/sdm/bin/release/release ${hostbindir}/release

#
# populate include tree with files referenced by libloc & ctab
#
(cd usr/include; make -cEF NLchar.h NLctype.h)
(cd usr/include/sys; make -cEF NLchar.h localedef.h)

#
# xmkcatdefs to create <xxx>_msg.h files from <xxx>.msg
#
(cd usr/bin/xmkcatdefs; \
 make -cEF INCARGS="-I${objdir}/usr/include" xmkcatdefs)
cp ${objdir}/usr/bin/xmkcatdefs/xmkcatdefs ${hostbindir}/xmkcatdefs

#
# gencat to create <xxx>.cat files from <xxx>.msg
#
(cd usr/bin/msg; \
 make -cEF MSGHDRS= INCARGS="-I${objdir}/usr/include" gencat)
cp ${objdir}/usr/bin/msg/gencat ${hostbindir}/gencat

#
# ctab needed by etc Makefile
#
(cd usr/bin/ctab; \
 make -cEF CARGS="-D_NO_PROTO" INCARGS="-I${objdir}/usr/include" ctab)
cp ${objdir}/usr/bin/ctab/ctab ${hostbindir}/ctab

#
# libloc needed by libc Makefile
#
(cd usr/bin/libloc; \
 make -cEF CARGS="-D_NO_PROTO" INCARGS="-I${objdir}/usr/include" libloc)
cp ${objdir}/usr/bin/libloc/libloc ${hostbindir}/libloc

#
# populate include tree with files referenced by libsb/migcom
#
(cd usr/include; make -cEF mach_error.h)
(cd usr/include/mach; make -cEF boolean.h error.h kern_return.h message.h port.h)
mkdir ${objdir}/usr/include/mach/machine
cp kernel/mach/${MACHINE}/boolean.h ${objdir}/usr/include/mach/machine
cp kernel/mach/${MACHINE}/kern_return.h ${objdir}/usr/include/mach/machine

#
# mig - Mach interface generator (cover script)
#
(cd usr/ccs/bin/mig; make -cEF CARGS="-DMACH -DCMU -D${SITE}" mig)
cp ${objdir}/usr/ccs/bin/mig/mig ${hostbindir}/mig

#
# migcom - Mach interface generator (where all the work is really done...)
#
PORTING_CARGS=
if [ -n "$NO_VFPRINTF" ]
then
    PORTING_CARGS="-DNO_VFPRINTF"
fi
(cd usr/ccs/lib/migcom; make -cEF INCARGS="-I. -I${objdir}/usr/include" \
  CARGS="-DMACH -DCMU -D${SITE} ${PORTING_CARGS}" migcom)
cp ${objdir}/usr/ccs/lib/migcom/migcom ${libdir}/migcom

#
# md - make dependency post-processor
#
PORTING_CARGS=
if [ -n "$NO_DIRENT" ]
then
    PORTING_CARGS="-DNO_DIRENT"
fi
(cd usr/local/sdm/bin/md; \
 make -cEF INCARGS="-I${objdir}/usr/include" \
  LIBS= CARGS="-DMACH -DCMU -D${SITE} -D_BSD ${PORTING_CARGS}" md)
cp ${objdir}/usr/local/sdm/bin/md/md ${hostbindir}/md

#
# uudecode for encoded binary files
#
if [ -n "$NO_UUDECODE" ]
then
  (cd usr/bin/uudecode; make -cEF INCARGS="-I${objdir}/usr/include" uudecode)
  cp ${objdir}/usr/bin/uudecode/uudecode ${hostbindir}/uudecode
fi

#
# Special build of yacc
#
(cd usr/ccs/bin/yacc; make -cEF INCARGS="-I${objdir}/usr/include" yacc)
cp ${objdir}/usr/ccs/bin/yacc/yacc ${hostbindir}/yacc
cp usr/ccs/lib/yaccpar ${libdir}/yaccpar
YACCPAR=${libdir}/yaccpar
export YACCPAR

#
# Special build of lex
#
(cd usr/ccs/bin/lex; make -cEF INCARGS="-I. -I${objdir}/usr/include" lex)
cp ${objdir}/usr/ccs/bin/lex/lex ${hostbindir}/lex
cp usr/ccs/lib/ncform ${libdir}/ncform

#
# Install ranlib script if commands are macho
#
if [ "${CMD_OBJECT_FORMAT}" = "MACHO" ]
then
    (cd usr/ccs/bin/ranlib; make -cEF ranlib)
    cp ${objdir}/usr/ccs/bin/ranlib/ranlib ${gccdir}/ranlib
fi

exit 0
