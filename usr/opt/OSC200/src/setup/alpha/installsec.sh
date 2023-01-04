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
if [ ${target_os_type} = osf ]; then
    PATH="/usr/ccs/bin:/usr/sde/tools/alpha_osf/bin:${gccdir}:/sbin:/usr/bin"
else
    PATH="${hostbindir}:${bindir}:${gccdir}:/usr/ucb:/bin:/usr/bin"
fi

export PATH

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

echo 
echo "======================================================================="
echo "                OSF/1 Release 1.0 Security Install Preperation"
echo


USAGE="installsec.sh <targetroot> <configuration_name>"

if [ $# != 2 ]
then
    echo "Incorrect number of command line options."
    echo $USAGE
    exit 1
fi

TOSTAGE=$1
CONFIG=$2
CONFIGDIR=${OBJECTDIR}/kernel/${CONFIG}
SECURITY_ONLY=
export TOSTAGE SECURITY_ONLY

if [ ! -d $TOSTAGE ]
then
    echo "Directory ${TOSTAGE} not found.  Please correct and rerun."
    echo $USAGE
    exit 1
fi

if [ ! -d ${CONFIGDIR} ]
then
    echo "Configuration ${CONFIG} not found.  Please correct and rerun."
    echo $USAGE
    exit 1
fi

if [ "X${SEC_LEVEL}" = "X" ]
then
    echo "SEC_LEVEL must be set in your environment as B1 or C2."
    exit 1
fi

if [ -f ${srcdir}/setup/osf1_idlist ]
then
    RELEASE_OPTIONS="-idfile ${srcdir}/setup/osf1_idlist"
    export RELEASE_OPTIONS
fi

echo "Working on ${TOSTAGE}."

echo "Installing security passwd and group files.  Current passwd and group"
echo "files will be saved as passwd.nonsec and group.nonsec"

cp ${TOSTAGE}/etc/passwd ${TOSTAGE}/etc/passwd.nonsec
  (cd etc;		make -cikEF install_group)

cp ${TOSTAGE}/etc/group ${TOSTAGE}/etc/group.nonsec
  (cd etc;		make -cikEF install_passwd)

echo "Making ACLDBASE and MACDBASE based on new password file."

  (cd ../obj/${target_machine}/tcb/files; rm -f ACLDBASE MACDBASE )
  (cd tcb/files; make -cF ACLDBASE MACDBASE )

echo "Installing misc. security programs to create minimal security root."

  (cd etc/policy; 	make -cikEF install_all)
  (cd etc/auth; 	make -cikEF install_all)

  (cd seccmd; 	\
          make -cikF SEC_SUBDIRS= \
            SEC_ILIST="audit_dlvr auths initcond privs" \
            install_all)

  (cd seccmd/secpolicy; \
          make -cikEF SEC_SUBDIRS= SEC_ILIST="acld macd mkdb spdinit" install_all)

for d in mount ufs_fsck init
do
  (cd sbin/$d; \
          make -cikEF ${d}_IDIR=/tcb/bin/ IDIR=/tcb/bin/ install_$d)
done

  (cd tcb/files/auth.${SEC_LEVEL};	make -cikEF install_all)

  (cd tcb/files; \
        make -cikEF SEC_SUBDIRS= \
          SEC_ILIST="inittab sp_config spdinitrc ACLDBASE MACDBASE" \
	  install_all)

echo "Making the devices for the system policy daemons in ${TOSTAGE}/dev"
for i in 0 1 2 3
do 
	mknod ${TOSTAGE}/dev/spd$i c 18 $i
done
mknod ${TOSTAGE}/dev/spdcontrol c 18 255

echo "Making the devices for the audit daemon in ${TOSTAGE}/dev"
mknod ${TOSTAGE}/dev/auditr c 17 0
mknod ${TOSTAGE}/dev/auditw c 17 1


echo "Copying ${CONFIG}/vmunix to ${TOSTAGE}/vmunix.OSF1.${CONFIG}"
cp ${CONFIGDIR}/vmunix ${TOSTAGE}/vmunix.OSF1.${CONFIG}
echo "Linking ${TOSTAGE}/vmunix.OSF1.${CONFIG} to vmunix.${SEC_LEVEL}"
ln ${TOSTAGE}/vmunix.OSF1.${CONFIG} ${TOSTAGE}/vmunix.${SEC_LEVEL}


echo ""
echo "The minimal security install is done to ${TOSTAGE}.  You will now have"
echo "to reboot from vmunix.${SEC_LEVEL} and create a tagged file system on"
echo "a empty disk.  Once the tagged files systems are created you can run"
echo "install.sh from the security build to create a full security install."
echo ""
exit 0



