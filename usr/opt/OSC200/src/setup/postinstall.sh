#! /bin/sh5
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
# @(#)$RCSfile: postinstall.sh,v $ $Revision: 4.5.22.2 $ (DEC) $Date: 1993/03/15 20:47:42 $ 
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
# Modification History
#
# 082091.1 gws
#	added softlink /usr/lib/tabset -> ../share/lib/tabset
#	  to support old termcap and terminfo databases entries
#
# 082091.2 gws
#	added softlink /usr/lib/refer -> ../lbin/refer
#	  so lookbib and refer commands work
#
# 082091.3 vsp
#	added softlink ../opt/s5/usr/shlib/libsys5.so ->
#	  ./usr/shlib/libsys5.so
# 100491.4 gaudet
#	/var/spool/mail chmod 1777 and chgrp mail
#
# 100891 terry
#      added softlink ../init.d/settime -> ./sbin/rc3.d/S05settime
#
# 101091 gaudet
#	removed var/mail and usr/mail
#
# 101091.7 tap
#     removed softlink (ln -s  ../share/lib/nterm  ./usr/lib/nterm)
#
# 111291.8 tap
#	removed softlink (ln -s  ../share/lib/macros  ./usr/lib/macros)

echo 
echo "======================================================================="
echo "                  OSF/1 Rev 1.0 Postinstall Process"
echo


USAGE="usage: postinstall.sh [-noconfig] [-nodev] [-X11] [-gcc] [-mcc] <targetroot>"
DOCONFIG="yes"
DODEV="yes"
DODEBUG="no"
DOX11="no"
DOGCC="no"
DOMCC="no"
INSTALLBASE=""

if [ $# = 0 ]
then
  echo "ERROR: Incorrect number of arguments:"
  echo $USAGE
  exit 1
fi

while [ "$#" -gt 0 ] 
do
  arg=$1
  shift
  case "$arg" 
  in
    -noconfig) DOCONFIG="no";;
    -nodev) DODEV="no";;
    -X11) DOX11="yes";;
    -gcc) DOGCC="yes";;
    -mcc) DOMCC="yes";;
    -debug) DODEBUG="yes";;

    *) if [ "$INSTALLBASE" = "" ] 
       then
         INSTALLBASE=$arg
         if [ ! -d ${INSTALLBASE} ]
         then
           echo $USAGE
           echo "ERROR: Directory ${INSTALLBASE} not found."
           echo "   Please correct and rerun postinstall.sh"
           exit 1
         fi
       else
         echo $USAGE
         echo "$arg unknown argument" 
         exit 1
       fi;;
  esac
done

if [ "$INSTALLBASE" = ""  ]
then
  echo "ERROR: no <targetroot> specified"
  echo $USAGE
  exit 1
fi

#
# setup/PMAX/mipsc_rel.sh sets MIPS C version variable (REL).
#
if [ $DOMCC = "yes" ]
then
  . setup/PMAX/mipsc_rel.sh
fi

cd ${INSTALLBASE}


if [ $DOCONFIG = "yes" ]
then
#
# local variables
#
config="etc/rc.config.tmp"
real_config="etc/rc.config"
fstab="etc/fstab.tmp"
real_fstab="etc/fstab"
#
# Truncate the config files
#
> $config
> $fstab

#
# termination handler
#
trap 'echo "\nAborting..."; rm -f $config $fstab; exit 1' 1 2 3 15

#
# Setup defaults
#
DISPLAYTYPE="notset"
HOSTNAME="notset"
HOSTID="notset"
NETMASK="255.255.255.0"
NETDEV="notset"
PAGEMINSZ="notset"
PAGEMAXSZ="notset"
PAGEFILE="notset"
PAGEPARTITION="notset"
PAGEPARTITIONTYPE="notset"
ROOTPARTITION="notset"
VARPARTITION="notset"
USRPARTITION="notset"
#
#
#
if [ -f ${real_config} ]
then
  OVERWRITE="n"
  echo "An ${real_config} already exists.  Do you wish to overwrite it? (y/n) [$OVERWRITE] "
  read OVERWRITE
else
  OVERWRITE=y
fi

if [ "$OVERWRITE" = y ]
then

  if [ -f ${real_config} ]
  then
    echo "copying $real_config to ${real_config}.bak"
    cp $real_config ${real_config}.bak
  fi # -f ${real_config}

echo "#" > $config
echo "# DISPLAYTYPE={none,mono,color} type of monitor" >> $config
echo "# HOSTNAME={unknown}  the Internet name of your system" >> $config
echo "# HOSTID={130.105.255.255}  Internet address in dot format of your system" >> $config
echo "# NETMASK={network netmask} - default: 255.255.255.0" >> $config
echo "# NETDEV={se0,en0} - pmax=se0, mmax=en0" >> $config
echo "# PAGEFILE={/paging/space, other}  the name of the paging file" >> $config
echo "# PARTITION={rz[0-3]b} the paging partition if PAGEFILE=/paging/space" >> $config
echo "# PARTITIONTYPE={RZ23,RZ55,...} disk type containing the paging partition" >> $config
echo "# PAGEMINSZ={16M,32M}  low-water point for paging file - default: 32M" >> $config
echo "# PAGEMAXSZ={32M,64M}  high-water point for paging file - default: 64M" >> $config
echo "#" >> $config


#
# Query the user for the hostname
#
  thishost=`hostname`
  echo "What is the Internet host name of this system ? [$thishost]: "
  while [ "$HOSTNAME" = "notset" ]
  do
    read hostname
    case $hostname in
      "")     HOSTNAME="$thishost";;
      *)      HOSTNAME="$hostname";;
    esac
  done

#
# Query the user for the hostid
#
  thishostid=""
  echo "What is the Internet host address of \"$HOSTNAME\" ? [$thishostid]: "
  while [ "$HOSTID" = "notset" ]
  do
    read hostid
    case $hostid in
      "") echo "There is no default, you must enter a valid address. Try again: ";;
      *.*.*.*) HOSTID="$hostid";;
      *)      echo "You must specify the address in dot format. Try again: ";;
    esac    
  done

#
# Find out the root device
#
  echo "What is the device and partition for the root (rz3a, md0b, etc) ? [rz3a]: "
  while [ "$ROOTPARTITION" = "notset" ]
  do
    read rootpartition
    case $rootpartition in
      "")     ROOTPARTITION="rz3a";;
      ????)   ROOTPARTITION="$rootpartition";;
      *)      echo "You specified an incorrect device. Try again: ";;
    esac
  done

#
# Find out the usr device
#
  echo "What is the device and partition for /usr (rz3a, rz0b, "nfs") ? [nfs]: "
  while [ "$USRPARTITION" = "notset" ]
  do
    read usrpartition
    case $usrpartition in
      "" | nfs | NFS ) usrnfs=1
        USRPARTITION="nfs"
        ;;
      ????)   USRPARTITION="$usrpartition";;
      *)      echo "You specified an incorrect device. Try again: ";;
    esac
    if [ "$USRPARTITION" = "$ROOTPARTITION" ]; then
      echo "You cannot have the root and usr on the same device. Try again: "
      USRPARTITION="notset"
    fi
  done

  if [ "$usrnfs" = "1" ]; then
    releasearea="notset"
    echo "What is the network mount point for /usr ?  "
    while [ "$releasearea" = "notset" ]
    do
      read usrpartitiontype
      case $usrpartitiontype in
      "")  ;;
      *)  releasearea="$usrpartitiontype";;
      esac
    done
  fi # [ "$usrnfs" = "1" ]

#
# Find out the var partition
#
  echo "What is the device and partition for /var (rzxx, mdxx, "usr",...) ? [rz3g]: "
  while [ "$VARPARTITION" = "notset" ]
  do
    read varpartition
    case $varpartition in
      "")     VARPARTITION="rz3g";;
      usr)    if [ "$usrnfs" = "1" ]; then
          echo "You cannot have a NFS var partition."
        else # [ "$usrnfs" = "1" ]
          usrvar=1
          # Make a link on the root
          VARPARTITION="usr"
        fi # [ "$usrnfs" = "1" ]
        ;;
      ????)   VARPARTITION="$varpartition"
        ;;
      *)      echo "You specified an incorrect device. Try again: ";;
    esac
  done

#
# Query the user for the name of the paging file
#
  echo "What is the path name for the paging file (/paging/space, /usr/tmp/.paging, etc) ? [/paging/space]: "
  while [ "$PAGEFILE" = "notset" ]
  do
    read pagefile
    case $pagefile in
      "")     PAGEFILE="/paging/space";;
      /usr/*) if [ "$usrnfs" = "1" ]; then
          echo "/usr is a NFS filesystem. Your paging file should be local. Try again: "
          PAGEFILE="notset"
         fi # [ "$usrnfs" = "1" ]
         ;;

      /paging/*) if [ "$pagefile" != "/paging/space" ]; then
          echo "You should call the file /paging/space. Try again: "
          PAGEFILE="notset"
           else # [ "$pagefile" != "/paging/space" ]
          PAGEFILE="$pagefile"
           fi # [ "$pagefile" != "/paging/space" ]
           ;;

      /*)     PAGEFILE="$pagefile";;
      *)      echo "You must use an absolute path name. Try again: "
        PAGEFILE="notset"
        ;;
    esac
  done

#
# If the user answered "/paging/space" for a PAGEFILE then we
# assume this is to be a special partition - usually the b one.
# Now we need to ask for the partition device and device type
# for the newfs call in rc
#
  if [ "$PAGEFILE" = "/paging/space" ]; then
    echo "What is the device and partition for /paging ? [rz3b]: "
    while [ "$PAGEPARTITION" = "notset" ]
    do
      read pagepartition
      case $pagepartition in
      "")     PAGEPARTITION="rz3b";;
      ????)   PAGEPARTITION="$pagepartition";;
      *)      echo "You specified an incorrect device. Try again: ";;
      esac
    done

    # find out the partition type
    echo "What is the drive type for /paging (/dev/$PAGEPARTITION) (RZ23, RZ55, etc)? [$ROOTPARTITIONTYPE]: "
    while [ "$PAGEPARTITIONTYPE" = "notset" ]
    do
      read pagepartitiontype
      case $pagepartitiontype in
      "")     PAGEPARTITIONTYPE="$ROOTPARTITIONTYPE";;
      ????) PAGEPARTITIONTYPE="$pagepartitiontype";;
      *)      echo "You specified an incorrect device. Try again: ";;
      esac
    done
    # Make sure the /paging directory exists
    if [ ! -d paging ]; then
        if [ -f paging ]; then
          mv paging paging.old
        fi
      mkdir paging
    fi # [ ! -d paging ]
  else # [ "$PAGEFILE" = "/paging/space" ]
    PAGEPARTITION=""
    PAGEPARTITIONTYPE=""
    nopagepartition=1
  fi # [ "$PAGEFILE" = "/paging/space" ]

#
# Query the user for the low and high water points for the paging file
#
  valueok="0"
  while [ "$valueok" = "0" ]
  do
    echo "What is the low size limit for the paging file in Mb ? [16]: "
    read pageminsz
    case $pageminsz in
      "")     PAGEMINSZ="16";;
      [0-9]*) PAGEMINSZ="$pageminsz";;
      *)      echo "Numeric value expected for low limit. Try again: ";;
    esac

    echo "What is the high size limit for the paging file in Mb ? [64]: "
    read pagemaxsz
    case $pagemaxsz in
      "")     PAGEMAXSZ="64";;
      [0-9]*) PAGEMAXSZ="$pagemaxsz";;
      *)      echo "Numeric value expected for HIGH limit. Try again: ";;
    esac
    if [ $PAGEMINSZ -gt $PAGEMAXSZ ]; then
      echo "Low limit cannot be greater than high limit. Try again."
    else # [ $PAGEMINSZ -gt $PAGEMAXSZ ]
      valueok="1"
    fi # [ $PAGEMINSZ -gt $PAGEMAXSZ ]
    PAGEMINSZ="${PAGEMINSZ}M"
    PAGEMAXSZ="${PAGEMAXSZ}M"
  done

#
# Query the user for the network device type
#
  echo "What type of network adapter are you using (pmax=se0, mmax=en0) ? [se0]: "
  while [ "$NETDEV" = "notset" ]
  do
    read netdev
    case $netdev in
      "")     NETDEV="se0";;
      *)      NETDEV=$netdev;;
    esac
  done

#
# Query the user for the display type
#
  echo "What type of display are you using (mono,color,none,foreign) ? [none]: "
  while [ "$DISPLAYTYPE" = "notset" ]
  do
    read displaytype
    case $displaytype in
      mono)   DISPLAYTYPE="mono";;
      color)  DISPLAYTYPE="color";;
      none)   DISPLAYTYPE="none";;
          foreign) DISPLAYTYPE="foreign";;
      "")     DISPLAYTYPE="none";;
      *)      echo "Your choices are (mono,color,none,foreign) - try again: ";;
    esac
  done


#
# Write the config file
#
  echo "DISPLAYTYPE=$DISPLAYTYPE" >> $config
  echo "HOSTNAME=$HOSTNAME" >> $config
  echo "HOSTID=$HOSTID" >> $config
  echo "NETMASK=$NETMASK" >> $config
  echo "NETDEV=$NETDEV" >> $config
  echo "PAGEFILE=$PAGEFILE" >> $config
  echo "PARTITION=$PAGEPARTITION" >> $config
  echo "PARTITIONTYPE=$PAGEPARTITIONTYPE" >> $config
  echo "PAGEMINSZ=$PAGEMINSZ" >> $config
  echo "PAGEMAXSZ=$PAGEMAXSZ" >> $config
  echo "export DISPLAYTYPE HOSTNAME HOSTID NETMASK NETDEV" >> $config
  echo "export PAGEFILE PARTITION PARTITIONTYPE PAGEMINSZ PAGEMAXSZ" >> $config
#
# Copy the tmp file into place
#
  cp $config $real_config
else # [ "$OVERWRITE" = y ]
  echo "$real_config exists, reading..."
  . $real_config
fi # [ "$OVERWRITE" = y ]
#
#
echo
echo "Making etc/fstab..."
if [ -f etc/fstab ]
then
  OVERWRITE="n"
  echo "An etc/fstab already exists.  Do you wish to replace it? (y/n) [$OVERWRITE] "
  read OVERWRITE
else
  OVERWRITE="y"
fi

if [ "$OVERWRITE" = yes -o "$OVERWRITE" = y ]
then

#
# Write the fstab file
#
echo "/dev/$ROOTPARTITION		/	ufs rw 0 1" >> $fstab
if [ "X$usrvar" = "X" ]; then
        echo "/dev/$VARPARTITION		/var	ufs rw 0 2" >> $fstab
fi

if [ "$usrnfs" = "1" ]; then
        echo "$releasearea	/usr	nfs ro 0 0" >> $fstab
else
        echo "/dev/$USRPARTITION		/usr	ufs rw 0 3" >> $fstab
fi
  cp $fstab $real_fstab
fi
#
#
#
echo
echo "Making etc/hosts..."
if [ -f etc/hosts ]
then
  OVERWRITE="n"
  echo "An etc/hosts already exists.  Do you wish to overwrite it? (y/n) [$OVERWRITE] "
  read OVERWRITE
else
  OVERWRITE="y"
fi

if [ "$OVERWRITE" = yes -o "$OVERWRITE" = y ]
then
  
  echo "#  Minimal Host file...."  > etc/hosts.new
  echo "#" >> etc/hosts
  echo "127.0.0.1    localhost" >> etc/hosts.new
  echo "$HOSTID  $HOSTNAME" >> etc/hosts.new

  if [ -f etc/hosts ]
  then
    echo "Original etc/hosts moved to etc/hosts.old"
    mv etc/hosts etc/hosts.old
  fi

  mv etc/hosts.new etc/hosts

fi

if [ ! -f etc/exports ]
then
  touch etc/exports
fi

rm -f $config $fstab   # remove .tmp files...

fi  # if [ DOCONFIG ]
#
#
#
if [ $DODEV = "yes" ]
then
  echo "Making devices..."
  if [ -d dev ]
  then
    cd dev
    sh ./MAKEDEV std
    cd ..
  fi
fi

echo "Making additional directories..."

#
# If var placement not specified (-noconfig),
# then relocate /var to /usr/var.
# ONLY relocate /var if it exists and is a directory.
# On lastout (ULTRIX) /var is created. On bambam (OSF) /var
# is a symbolic link to usr/var. WHY?
#
if [ $DOCONFIG = "no" -a ! -d usr/var ]
then
	tar cf - var |(cd usr; tar xpf -)
	rm -rf var
	ln -s usr/var var
fi

mklink()
    {
    while read TARGET LINKNAME; do
	if [ ! -d ${LINKNAME} ]; then
	    ln -s ${TARGET} ${LINKNAME}
	fi
    done
    }



#
# Create links for X11
#
if [ $DOX11 = "yes" ]
then
  echo " - Links for X11"
  mkdir usr/X11
  echo  ../X11/bin		./usr/bin/X11 | mklink
  echo  ../X11/lib/X11		./usr/lib/X11 | mklink
  ln -s  ../X11/lib/libMrm.a	./usr/lib/libMrm.a
  ln -s  ../X11/lib/libX11.a	./usr/lib/libX11.a
  ln -s  ../X11/lib/libXau.a	./usr/lib/libXau.a
  ln -s  ../X11/lib/libXaw.a	./usr/lib/libXaw.a
  ln -s  ../X11/lib/libXdmcp.a	./usr/lib/libXdmcp.a
  ln -s  ../X11/lib/libXext.a	./usr/lib/libXext.a
  ln -s  ../X11/lib/libXinput.a	./usr/lib/libXinput.a
  ln -s  ../X11/lib/libXm.a	./usr/lib/libXm.a
  ln -s  ../X11/lib/libXmu.a	./usr/lib/libXmu.a
  ln -s  ../X11/lib/libXt.a	./usr/lib/libXt.a
  ln -s  ../X11/lib/liboldX.a	./usr/lib/liboldX.a
fi 
#
# Links from /usr/ccs/bin to /usr/ccs/gcc
#
if [ $DOGCC = "yes" ]
then
  echo " - Links to use gcc"
  ln -s  ../gcc/as	./usr/ccs/bin/as
  ln -s  ../gcc/cc1	./usr/ccs/lib/gcc-cc1
  ln -s  ../gcc/cpp	./usr/ccs/lib/gcc-cpp
  ln -s  ../gcc/cc1	./usr/ccs/lib/cc1
  ln -s  ../gcc/cpp	./usr/ccs/lib/cpp
  ln -s  ../gcc/cpp	./usr/ccs/bin/cpp
  ln -s  ../gcc/gcc	./usr/ccs/bin/cc
  ln -s  ../gcc/gcc	./usr/ccs/bin/gcc
  ln -s  ../gcc/gdb	./usr/ccs/bin/gdb
  ln -s  ../gcc/gnulib	./usr/ccs/lib/gnulib
  ln -s  ../gcc/ld	./usr/ccs/bin/ld
fi

#
# Links from /usr/ccs/lib/cmplrs to /usr/lib/cmplrs, /usr/lib, /usr/bin, etc.
#
if [ $DOMCC = "yes" ]
then
  echo " - Links to use mcc"
  CMPLRS_ROOT=./usr/ccs/lib/cmplrs
  
#
# Set up the ./usr/lib/cmplrs files.
#
  ln -s ../ccs/lib/cmplrs ./usr/lib/cmplrs
  ln -s cc${REL} ./usr/lib/cmplrs/as${REL}
  ln -s cc${REL} ./usr/lib/cmplrs/cc
  ln -s as${REL} ./usr/lib/cmplrs/as
  
  FILES1="ar as0 as1 btou cfe cord crt0.o dbx dbx.help dis \
         driver err.english.cc file ftoc ld libexc.a libfastm.a libfe.a \
         libm43.a libmisc.a libmld.a libp.a libprof1.a  \
         libu.a libus.a libxmalloc.a  mcrt0.o nm odump pcrt0.o pmcrt0.o pixie \
         pixstats ppu prof runcord size stdump strip \
         ugen ujoin umerge uopt usplit utob vcrt0.o vmcrt0.o"
  
  for f in ${FILES1}
  do
      mv ${CMPLRS_ROOT}/cc${REL}/${f} ${CMPLRS_ROOT}/cc${REL}/${f}${REL}
      ln -s ${f}${REL} ${CMPLRS_ROOT}/cc${REL}/${f}
  done
  
  ln -s cc${REL}/err.english.cc${REL} ./usr/lib/cmplrs/err.cc
  ln -s cfe${REL} ${CMPLRS_ROOT}/cc${REL}/cpp${REL}
  ln -s cpp${REL} ${CMPLRS_ROOT}/cc${REL}/cpp
  ln -s ld ${CMPLRS_ROOT}/cc${REL}/uld
  mv ${CMPLRS_ROOT}/cc${REL}/oldc/ccom ${CMPLRS_ROOT}/cc${REL}/oldc/ccom${REL}
  ln -s ccom${REL} ${CMPLRS_ROOT}/cc${REL}/oldc/ccom
  mv ${CMPLRS_ROOT}/cc${REL}/oldc/cpp ${CMPLRS_ROOT}/cc${REL}/oldc/cpp${REL}
  ln -s cpp${REL} ${CMPLRS_ROOT}/cc${REL}/oldc/cpp
  
  
#
# Set up the ./usr/bin & ./usr/ccs/bin files.
#
  ln -s ../lib/cmplrs/cc${REL}/driver ./usr/bin/cc${REL}
  ln -s cc${REL} ./usr/bin/cc
  ln -s ../lib/cmplrs/as${REL}/driver ./usr/bin/as${REL}
  ln -s as${REL} ./usr/bin/as
  
  ln -s ../lib/cmplrs/cc${REL}/driver ./usr/ccs/bin/cc${REL}
  ln -s cc${REL} ./usr/ccs/bin/cc
  ln -s ../lib/cmplrs/as${REL}/driver ./usr/ccs/bin/as${REL}
  ln -s as${REL} ./usr/ccs/bin/as
  
  FILES2="ar btou cord dbx dbx.help dis file ld nm odump pixie pixstats \
         ppu prof runcord size stdump strip utob"
  
  for f in ${FILES2}
  do
      ln -s ../lib/cmplrs/cc${REL}/${f} ./usr/bin/${f}${REL}
      ln -s ${f}${REL} ./usr/bin/${f}
      ln -s ../lib/cmplrs/cc${REL}/${f} ./usr/ccs/bin/${f}${REL}
      ln -s ${f}${REL} ./usr/ccs/bin/${f}
  done
  
  ln -s ld${REL} ./usr/bin/uld${REL}
  ln -s ld ./usr/bin/uld
  ln -s ld${REL} ./usr/ccs/bin/uld${REL}
  ln -s ld ./usr/ccs/bin/uld
  
  
#
# Set up the ./usr/lib & ./usr/ccs/lib files.  Note:  err.english.cc 
# ought to go wherever cpp goes.
#
  ln -s ./cmplrs/cc${REL}/cpp ./usr/lib/cpp${REL}
  ln -s cpp${REL} ./usr/lib/cpp
  ln -s ./cmplrs/cc${REL}/err.english.cc ./usr/lib/err.english.cc${REL}
  ln -s err.english.cc${REL} ./usr/lib/err.english.cc
  ln -s ./cmplrs/cc${REL}/cpp ./usr/ccs/lib/cpp${REL}
  ln -s cpp${REL} ./usr/ccs/lib/cpp
  ln -s ./cmplrs/cc${REL}/err.english.cc ./usr/ccs/lib/err.english.cc${REL}
  ln -s err.english.cc${REL} ./usr/ccs/lib/err.english.cc
fi

echo "************************************************************************"
echo "Post Install done.  Please remember to do the following before trying"
echo "to reboot."
echo
echo "  - review and edit if needed etc/rc.config and etc/fstab"
echo "  - run installk.sh to install a kernel if not already done"
echo "  - Install the boot program for your computer on the root partition"
echo ""
echo "After you boot OSF/1 into multiuser mode you will need to run"
echo "/postboot program to create the databases that can only be created when"
echo "OSF/1 is running.  The postboot program only has to be run once and"
echo "then can be deleted."
echo 
exit 0
