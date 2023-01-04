#! /bin/sh
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
# @(#)cmplrs.boot.sh	3.4	(ULTRIX/OSF)	6/1/91
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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0


if [ $# != 1 ]
then
    echo "cmplrs.boot.sh <stage1|stage2|stage3>"
    exit 1
fi

#
# Constrain search paths
#
srcdir=`pwd`
base=`expr "${srcdir}" : "\(.*\)/.*"`
if [ `basename "${srcdir}"` != "src" ]
then
    echo "Please move source tree into a \"src\" subdirectory"
    exit 1
fi

STAGE=$1
TOOLSDIR=$base/tools/${target_machine}/bin
libdir=$base/tools/${target_machine}/lib

expdir=$base/hostobj/${target_machine}

LIBDIRS="-L${expdir}/usr/ccs/lib"
SHLIBDIRS="-L${expdir}/usr/shlib"
MIGCOM="${libdir}/migcom"
LEXER="${libdir}/ncform"
YACCPAR="${libdir}/yaccpar"
NO_SHARED_LIBRARIES=1

export LIBDIRS SHLIBDIRS MIGCOM LEXER YACCPAR NO_SHARED_LIBRARIES

OBJECT_FORMAT="${CMD_OBJECT_FORMAT}"
export OBJECT_FORMAT
if [ "${OBJECT_FORMAT}" = "MACHO" ]
then
    MACHO_GCC_EXEC_PREFIX="${gccdir}"
    export MACHO_GCC_EXEC_PREFIX
else
    gccdir=$COMP_HOST_ROOT/usr/bin
    COFF_GCC_EXEC_PREFIX="${gccdir}/"
    export COFF_GCC_EXEC_PREFIX
fi

#
# New build environment definitions
#
EXPORTBASE=${expdir}
SOURCEDIR=${SOURCEBASE}/usr/ccs/
CMPLRS_SUBDIR=usr/lib/cmplrs
LINTDIR=usr/lib/lint

export EXPORTBASE SOURCEDIR CMPLRS_SUBDIR LINTDIR


BOOTSTRAP=${BOOTBASE}/${STAGE}
OBJECTDIR=$BOOTSTRAP/obj/
STAGE1_ROOT=$BOOTSTRAP
CHECKDIR=${BOOTBASE}/stage2/${CMPLRS_SUBDIR}/cc${REL}
CMPO=${OBJECTDIR}/usr/ccs/bin/cmpo/cmpo
CMPLIB=${OBJECTDIR}/usr/ccs/bin/cmpo/cmplib
CMP=${OBJECTDIR}/usr/ccs/bin/cmpo/cmpos

export OBJECTDIR STAGE1_ROOT CMPLRS_SUBDIR CHECKDIR CMPO CMPLIB CMP

# For the timebeing do not remove this, 
#rm -rf $OBJECTDIR
mkdir -p $OBJECTDIR

# Create the target directory structure, this is the place where all the
# commands and libraries will be installed. 
# 
here=$PWD
if [ -d ${STAGE1_ROOT}/bin ]
then 
	true
else
	mkdir -p ${STAGE1_ROOT}/bin
fi

if [ -d ${STAGE1_ROOT}/usr/lib ]
then
	true
else
	mkdir -p ${STAGE1_ROOT}/usr/lib
	ln -s ${STAGE1_ROOT}/usr/lib ${STAGE1_ROOT}/lib
fi
if [ -d ${STAGE1_ROOT}/usr/bin ]
then
	true
else
	mkdir -p ${STAGE1_ROOT}/usr/bin
fi
if [ -d ${STAGE1_ROOT}/usr/lib ] 
then
	true
else
	mkdir -p ${STAGE1_ROOT}/usr/lib
fi
if [ -d ${STAGE1_ROOT}/usr/ccs/lib ]
then
	true
else
	mkdir -p ${STAGE1_ROOT}/usr/ccs/lib
fi
if [ -d ${STAGE1_ROOT}/usr/ccs/bin ]
then
	true
else
	mkdir -p ${STAGE1_ROOT}/usr/ccs/bin
fi

if [ -d ${STAGE1_ROOT}/${CMPLRS_SUBDIR}/cc${REL} ]
then
	true
else
	mkdir -p ${STAGE1_ROOT}/${CMPLRS_SUBDIR}/cc${REL}
fi
if [ -d ${STAGE1_ROOT}/${LINTDIR}/ ]
then
	true
else
	mkdir -p ${STAGE1_ROOT}/${LINTDIR}/
fi

#
# First build the libraries needed to build the compiler
# and install them in $TOSTAGE
#

if [ -f ${srcdir}/setup/osf1_idlist ]
then
    RELEASE_OPTIONS="-idfile ${srcdir}/setup/osf1_idlist"
    export RELEASE_OPTIONS
fi
TOSTAGE=$BOOTSTRAP/
export TOSTAGE

LDFLAGS="-L$BOOTSTRAP/usr/lib/cmplrs/cc${REL}"

export LDFLAGS

#
# Build the necessary librarires
#
(cd ${SOURCEDIR}/lib; make -f bootstrap.mk)
RC=$?
if [ ! $RC -eq 0 ] ; then exit $RC ; fi

#
# Install the libraries in $BOOTSTRAP/usr/lib/cmplrs/
#
if [ -d  $BOOTSTRAP/usr/lib/cmplrs/cc${REL} ]
then
	true ;
else
	mkdir -p $BOOTSTRAP/usr/lib/cmplrs/cc${REL} ;
fi
if [ -d  $BOOTSTRAP/usr/lib/cmplrs/cc${REL}/oldc ]
then
	true ;
else
	mkdir -p $BOOTSTRAP/usr/lib/cmplrs/cc${REL}/oldc ;
fi

echo
echo "Installing the $STAGE version of libraries ..."
echo
#
BASEDIR=$BOOTSTRAP/usr/lib/cmplrs/cc${REL}
TOBASE=$BASEDIR
IBINDIR=$BOOTSTRAP/usr/bin/
RM=/bin/rm
LINK="/bin/ln -s"
GROUP=`id | sed -e 's/.*(\(.*\))/\1/'`

if [ "`uname`" = "ULTRIX" ]
then
    LIBINSTALL="/usr/bin/install -c"
else
    LIBINSTALL="/bin/cp"
fi
OLDC=oldc
DESTROOT=$BOOTSTRAP
LANGDIR=$BASEDIR

export BASEDIR TOBASE IBINDIR RM LINK LIBINSTALL INSTALL OLDC DESTROOT

(cd ${SOURCEDIR}/lib; make -f ${SOURCEDIR}/lib/bootstrap.mk copy copybin link)
RC=$?
if [ ! $RC -eq 0 ] ; then exit $RC ; fi

#
# The bootstrap process looks for libc.a in the BOOTSTRAP directory
# so we need to make a link to it.
#
ln -s /usr/lib/libc.a $BOOTSTRAP/lib/libc.a

#
# Now build the compiler, and all the commands and install them too.
#
echo 
echo "Building the compilers and compiler related commands."
echo

(cd ${SOURCEDIR}/bin; make -f bootstrap.mk -cEF)
RC=$?
if [ ! $RC -eq 0 ] ; then exit $RC ; fi

echo
echo "Installing the $STAGE version of the compilers ..."
echo
(cd ${SOURCEDIR}/bin; make -f ${SOURCEDIR}/bin/bootstrap.mk copy copybin link)
RC=$?
if [ ! $RC -eq 0 ] ; then exit $RC ; fi

#
# After STAGE3 compare the binaries for state2 and stage 3.
#

if [ ${STAGE} = "stage3" ]
then
    echo
    echo "Comparing the stage2 and stage3 versions of the compiler."
    echo
    (cd ${SOURCEDIR}/lib; make -f ${SOURCEDIR}/lib/bootstrap.mk check)
    RC=$?
    if [ ! $RC -eq 0 ] ; then exit $RC ; fi
    (cd ${SOURCEDIR}/bin; make -f ${SOURCEDIR}/bin/bootstrap.mk check)
    RC=$?
    if [ ! $RC -eq 0 ] ; then exit $RC ; fi
fi

#
echo "`date`: Bootstrap of $STAGE is complete"
exit 0
