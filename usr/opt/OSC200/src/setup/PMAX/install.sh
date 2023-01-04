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
# @(#)$RCSfile: install.sh,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/01/28 17:27:28 $ 
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

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

#
# Setup MIPS C compiler version (used by ccs/{bin,lib}/Makefile).
#
. setup/PMAX/mipsc_rel.sh

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
gccdir=${toolsdir}/${gccsuffix}
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
PATH="${hostbindir}:${bindir}:${gccdir}:/usr/ucb:/bin:/usr/bin"

export PATH

OBJECT_FORMAT="${CMD_OBJECT_FORMAT}"

export OBJECT_FORMAT

#
# Site/Environment stuff
#
SITE="OSF"
OWNER="bin"
GROUP="bin"

export SITE OWNER GROUP

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
# install targets
#
if [ $# != 1 ]
then
    echo "install.sh <targetroot>"
    exit 1
fi

if [ -f ${srcdir}/setup/osf1_idlist ]
then
    RELEASE_OPTIONS="-idfile ${srcdir}/setup/osf1_idlist"
    export RELEASE_OPTIONS
fi

# So symbolic links have group system instead of staff.
/bin/chgrp system $1

TOSTAGE=$1
export TOSTAGE
make -ckEF install_all

echo ""
echo "File installation done.  To complete installation you need to run"
echo "$1/postinstall to finish creating a bootable directory structure."
echo ""
echo "The syntax for postinstall is:"
echo "   postinstall [-noconfig] [-nodev] [-X11] [-gcc] <targetroot>"
echo ""
echo "    -noconfig tells postinstall not to create configuation files"
echo "                etc/rc.config, etc/fstab, etc/hosts and etc/exports."
echo "    -nodev    tells postinstall not to run dev/MAKEDEV"
echo "    -X11      tells postinstall to create links to support X11"
echo "    -gcc      tells postinstall to create links to support gcc"
echo ""
exit 0



