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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

#
#
#  This script is going to bootstrap the compilers.
#
#
#  Build stage1 of the bootstrap, compile the sources by using /bin/cc
#  If you decide to use any other cc, then set COMP_HOST_ROOT accordingly.
# 
#

DBG=$1

_BLD=true

export _BLD

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

srcdir=`pwd`
base=`expr "${srcdir}" : "\(.*\)/.*"`
if [ `basename "${srcdir}"` != "src" ]
then
    echo "Please move source tree into a \"src\" subdirectory"
    exit 1
fi

#
# Build Host version of the compilers
#
toolsdir=$base/tools/${target_machine}
TOOLSBIN=$base/tools/${target_machine}/bin
hostbindir=$base/tools/${target_machine}/hostbin
if [ "`uname`" = "ULTRIX" ]
then
    BOOTBASE=$base/hostobj/${target_machine}/bootstrap
else
    BOOTBASE=$base/hostobj/${target_machine}/bootstrap.OSF
fi

PATH=$TOOLSBIN:$hostbindir:/usr/bin:/bin:/usr/ucb:/usr/lib
BASIC_PATH=$PATH

export PATH BOOTBASE BASIC_PATH


#
# Site/Environment stuff
#
CENV="-UMSG -D_BLD ${ULTRIX_BLD+-DULTRIX_BLD}"
SITE="OSF"
OWNER="bin"
GROUP="bin"

export CENV SITE OWNER GROUP

#
# New build environment definitions
#
CCTYPE=host
MAKEFILEPATH='${MAKETOP}usr/lib/makefiles'
SOURCEBASE=${srcdir}
SOURCEDIR=""
OBJECTDIR=$BOOTBASE/obj
BOOT_INCDIR=$BOOTBASE/obj/usr/include
EXPORTBASE=$base/hostexp/$target_machine
MIGCOM=${toolsdir}/lib/migcom
MIGARGS=-I${EXPORTBASE}/usr/include

export OBJECTDIR EXPORTBASE MIGCOM MIGARGS CCTYPE
export MAKEFILEPATH SOURCEBASE SOURCEDIR BOOT_INCDIR

if [ -d ${OBJECTDIR} ]
then 
	true
else
	mkdir -p ${OBJECTDIR}
fi

#
# Make sure that the base compiler directory exists.
#
if [ -d ${toolsdir}/mcc ]
then
	rm -rf ${toolsdir}/mcc.last
	mv ${toolsdir}/mcc ${toolsdir}/mcc.last
else
if [ ! -d ${toolsdir}/mcc.last/usr ]
then
	mkdir -p ${toolsdir}/mcc.last
	cd ${toolsdir}/U4V2.20+; tar cf - . | \
		(cd ${toolsdir}/mcc.last; tar xfBp -)
	cd $srcdir
	
fi
fi

COMP_HOST_ROOT=$base/tools/${target_machine}/mcc.last
COMP_TARGET_ROOT=$BOOTBASE/stage1

# jpm: Create mcc/bin dir if it doesnt exist:
if [ ! -d ${toolsdir}/mcc.last/bin ] 
then
    mkdir -p $toolsdir/mcc.last/bin
    (cd $toolsdir/mcc.last/bin; for f in ar as cc pc cpp ld nm strip ranlib; \
                           do ln -s ../$f; done )
fi

#
# Install the include headers before we start with stage1.
#
echo
echo "`date`; Installing header files."
echo
if [ "`uname`" = "ULTRIX" ]
then
    (cd usr/include/mach; make -cEF)
    RC=$?
    if [ ! $RC -eq 0 ] ; then exit $RC ; fi

    (cd usr/include; make -cEF cmplrs_hdrs)
    RC=$?
    if [ ! $RC -eq 0 ] ; then exit $RC ; fi

    (cd usr/include/$MACHINE; make -cEF cmplrs_hdrs)
    RC=$?
    if [ ! $RC -eq 0 ] ; then exit $RC ; fi

    (cd usr/include; make -cEF machine)
    RC=$?
    if [ ! $RC -eq 0 ] ; then exit $RC ; fi

    (cd usr/include/cmplrs; make -cEF)
    RC=$?
    if [ ! $RC -eq 0 ] ; then exit $RC ; fi
else
    (cd usr/include; make -cEF all)
    RC=$?
    if [ ! $RC -eq 0 ] ; then exit $RC ; fi
fi

# Define the release of this system
. setup/PMAX/mipsc_rel.sh

if [ "`uname`" = "ULTRIX" ]
then
    true
else
    OSF_BUILD=1
fi

CCTYPE=ansi

PATH=$COMP_HOST_ROOT/bin:$COMP_HOST_ROOT/usr/bin:$PATH
export PATH COMP_HOST_ROOT COMP_TARGET_ROOT OSF_BUILD

echo
echo "`date`;  Building Stage1 "
echo
sh ${DBG} setup/PMAX/cmplrs.boot.sh stage1
RC=$?
if [ ! $RC -eq 0 ] ; then exit $RC ; fi

#
#  Build boot version 
#
echo
echo "`date`; Building Stage2 "
echo
COMP_HOST_ROOT=$BOOTBASE/stage1
COMP_TARGET_ROOT=$BOOTBASE/stage2
PATH=$COMP_HOST_ROOT/bin:$COMP_HOST_ROOT/usr/bin:$BASIC_PATH

sh setup/PMAX/cmplrs.boot.sh stage2
RC=$?
if [ ! $RC -eq 0 ] ; then exit $RC ; fi

#
# Build Reboot version 
#
echo
echo "`date`; Building Stage3 "
echo

COMP_HOST_ROOT=$BOOTBASE/stage2
COMP_TARGET_ROOT=$BOOTBASE/stage3
PATH=$COMP_HOST_ROOT/bin:$COMP_HOST_ROOT/usr/bin:$BASIC_PATH

sh setup/PMAX/cmplrs.boot.sh stage3
RC=$?
if [ ! $RC -eq 0 ] ; then exit $RC ; fi

echo
echo " All the stages have been completed."
echo

#
# Move the bootstrapped compiler to tools/pmax/mcc and create the
# expected links.
#
echo
echo " Installing the bootstrapped compiler. "
echo

mkdir -p $toolsdir/mcc/bin
(cd $BOOTBASE/stage3;tar cf - usr | (cd $toolsdir/mcc;tar xBf -))
ln -s usr/lib/cmplrs/cc/ar $toolsdir/mcc
ln -s usr/lib/cmplrs/cc/driver $toolsdir/mcc/as
ln -s usr/lib/cmplrs/cc/driver $toolsdir/mcc/cc
ln -s usr/lib/cmplrs/pc/driver $toolsdir/mcc/pc
ln -s usr/lib/cmplrs/cc/cpp $toolsdir/mcc
ln -s usr/lib/cmplrs/cc/ld $toolsdir/mcc
ln -s usr/lib/cmplrs/cc/nm $toolsdir/mcc
ln -s usr/lib/cmplrs/cc/strip $toolsdir/mcc
cp usr/ccs/bin/ranlib/ranlib.sh $toolsdir/mcc/ranlib
chmod 755 $toolsdir/mcc/ranlib
(cd $toolsdir/mcc/bin; for f in ar as cc pc cpp ld nm strip ranlib; \
     do ln -s ../$f; done )
rm -fr $BOOTBASE

echo
echo " The bootstrapped compiler is now installed. "
echo

exit 0
