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
# @(#)$RCSfile: ris.sh,v $ $Revision: 1.1.11.8 $ (DEC) $Date: 1993/12/15 19:51:51 $
# 

STARS="      ************************************************************"

# Make sure ris directories exist
[ -d /usr/var/adm/ris ] || 
{ 
	echo "No ris directories"
	exit 1
}

# Set RIS to be the real name of the ris home directory
RIS=`cd /usr/var/adm/ris; pwd`

chown ris $RIS
readonly RIS
PATH=.:/usr/sbin:/usr/bin:/sbin:$RIS/bin
#:/usr/lbin
SHELL_LIB=${SHELL_LIB:-/usr/share/lib/shell}
RISDB=$RIS/clients/risdb
export PATH SHELL_LIB RIS RISDB
NULL=/dev/null
LNK=n
lmfck -m OSF-SVR DEC 1.2 1-NOV-1992 || exit 1

. $SHELL_LIB/BadChoice
. $SHELL_LIB/Client
. $SHELL_LIB/Dialog
. $SHELL_LIB/Error
. $SHELL_LIB/Strings
. $SHELL_LIB/Ticker
. $SHELL_LIB/Wait

umask 022

: Addclient
# Add client processor
Addclient()
{
	[ -d $RIS/clients ] || 
	{
		mkdir $RIS/clients
		chown ris $RIS/clients
		chgrp ris $RIS/clients
	}
	#
	#	CLIENT REGISTRY INFORMATION
	#
	Dialog "
You have chosen to add a client for remote installation services.

The following conditions must be met to add a client:

	1. You must know the client processor's hostname
	2. The client's hostname must be in your system's host database(s).
	3. You must know the client's hardware Ethernet or FDDI address if
	   the client is registering to install operating system software.

Do you want to continue? (y/n)" ANS y
	case $ANS in
		""|[Yy]*)	;;
		*)	return 1
			;;
	esac

	while :
	do
		Getname new
		set -- `arp $CLIENT`; T_CLIARP=$2
		set -- `Parse '()' ${T_CLIARP}` ;  CLIARP=$1
		CNAME=`getname $CLIARP`
		[ "$CNAME" ] && break
		Error "\"$CLIENT\" is an invalid hostname."
	done

	# Check for duplicate IP addresses - a space and a tab are
	# enclosed in the brackets on both egrep lines
	DUPNUM=`egrep -c "^$CLIARP[ 	]" /etc/hosts 2>$NULL`
	[ $DUPNUM ] &&
	{
		[ $DUPNUM -gt 1 ] && 
		{
			DUPENT=`egrep "^$CLIARP[ 	]" /etc/hosts 2>$NULL`
			Error "
There are $DUPNUM entries in the /etc/hosts file with the IP address of
\"$CLIENT\".  The entries are:

$DUPENT

You must fix the /etc/hosts file before \"$CLIENT\" can be added."
			return 1
		}
	}

	[ "$CNAME" = "$CLIENT" ] || 
	{
		Dialog "
WARNING: \"$CNAME\" is the official name of \"$CLIENT\" on
this server.  Do you want this procedure to change \"$CLIENT\"
to \"$CNAME\" and continue? (y/n)" ANS y
		case $ANS in
			""|[Yy]*)	;;
			*)	return 1
				;;
		esac
	}

	CLIENT=$CNAME

	if [ -s "$RISDB" ]; then
		ENT=`grep "^$CLIENT:" $RISDB` && 
		{
			echo "
A client named \"$CLIENT\" is already registered.
Please use the \"Modify Client\" option. "
			return 1
		}
	fi

	# Start with ethernet addr of "none"
	CLINET=none

	Getenvir || return $?

	SelectProds install || return $?

	Mkris || return $?

	echo "\nClient $CLIENT has been added.\n"

	# Check that this is a server running nfs
	NFSSERVING=`/usr/sbin/rcmgr get NFSSERVING`	#get the flag
	if [ "$NFSSERVING" != 1 ]		#server running nfs?
	then							#no
	{
		echo "$STARS"
		echo "WARNING: In order to use RIS area(s), NFS MUST BE RUNNING.\n"
		echo "Run /usr/sbin/nfssetup to configure this system as an NFS server and to start"
		echo "NFS.  You need take no explicit action to export filesystems used by ris."
		echo "RIS will automatically add the appropriate entries to /etc/exports for you."
		echo "$STARS"
	}
	fi
	return 0
}

: Ckdevice
# Check whether device or mount point is valid.
Ckdevice()
{
	while :
	do
		Dialog "
Enter the device special file name or the path of the directory where
the software is located (for example, /dev/rmt0h)" DEV
		case $DEV in
		"" )	;;
		/dev/* )
			UNIT=`expr $DEV : '.*mt\([0-9][0-9]*\).*'`
			DEV=/dev/nrmt${UNIT}h
			mt -f $DEV rew || return 1
			MEDIA=tape
			LNK=n
			break
			;;
		/* )
			MEDIA=disk
			PPP=`ls -d $DEV/instctrl 2>$NULL`
			[ "$PPP" ] || PPP=`ls -d $DEV/*/instctrl 2>$NULL`
			[ "$PPP" ] || 
			{
				echo "
Cannot find a valid product directory under $DEV."
				continue
			}

			PDIRS=
			for I in $PPP
			do
				PDIRS="`dirname $I` $PDIRS"
			done
			while :
			do
				Dialog "
Choose one of the following options:

    1)  Extract software from $DEV
    2)  Create symbolic link to $DEV

Enter your choice" ANS
				case $ANS in
				1 )	LNK=n
					break	;;
				2 )	LNK=y
					break	;;
				esac
			done
			break
			;;
		esac
		echo "Invalid input."
	done
	return 0
}

: Cklock_ris
# Check the ris lock
Cklock_ris()
{
	[ -p /tmp/rislock ] && 
	{
		[ -f /tmp/ris.tty.lock ] || 
		{ 
			rm -f /tmp/rislock
			return 0
		}
		TTY=`sed -e "s/\/dev\///" /tmp/ris.tty.lock`
		[ "$TTY" ] || 
		{
			rm -f /tmp/rislock /tmp/ris.tty.lock
			return 0
		}
		set -- `who | grep $TTY`
		PERSON=$1; TTY=$2
		[ "$PERSON" -a "$TTY" ] || 
		{
			rm -f /tmp/rislock /tmp/ris.tty.lock
			return 0
		}
		echo "
The ris utility is currently locked while $PERSON on /dev/$TTY
is installing software.  Try again later."
		return 1
	}
	return 0
}

: Ckprodir
# If the image file exists under one of products and is identical to
# the new one, the product is being reinstalled for whatever reason.
# Otherwise, it is a new product.
# $1: ris?.{vax,mips,...}
# $2: the directory of the *.image file
# Sets PDDIR to the existing product directory if the image file is
# the same or calls Namepd to create a new PDDIR if this is a new product.

Ckprodir()
{
	CKROOT=$1
	cd $2
	IMAGE=`ls *.image`
	SIMAGE=`find $CKROOT -name $IMAGE -print`
	case $SIMAGE in
	"" )
		Namepd $CKROOT
		;;
	* )
		FINDFLG=n
		for I in $SIMAGE
		do
			cmp $I $IMAGE >$NULL 2>&1 && 
			{
				FINDFLG=y
				A=`dirname $I`
				PDDIR=`dirname $A`
				break
			}
		done
		case $FINDFLG in
		n )
			Namepd $CKROOT
			;;
		esac
	esac
}

: Deleteprod
# Delete products
Deleteprod()
{
	Getenvir || return
	SelectProds delete || return
	set `Parse , $PRODS`
	ENV=$1
	shift
	SUBPRODS=$*

	if [ "$ALLPRODS" ]; then
		echo "\nAfter this deletion, the area $RISROOT will be empty."
		J=`basename $RISROOT`
		grep $J $RISDB > /tmp/risdb
		if [ -s /tmp/risdb ]; then
			echo "The following clients are registered for $RISROOT:"
			cut -d: -f1 /tmp/risdb | sort | fmt
			Dialog "
This procedure will remove $RISROOT altogether and remove
all clients registered to it.  Do you wish to continue? (y/n)" ANS n
		else
			echo "There are no clients registered for $RISROOT."
			Dialog "
This procedure will remove $RISROOT altogether.
Do you wish to continue? (y/n)" ANS n
		fi
		case $ANS in
		[yY] )	;;
		* )
			echo "\nThe products were not deleted."
			return ;;
		esac

		grep -v "^$RISROOT" /etc/exports > /tmp/RISexports$$ 2>$NULL
		mv /tmp/RISexports$$ /etc/exports
		rm -rf $RISROOT

		RCLIENTS=`egrep "$ENV" $RISDB | cut -d: -f1`
		for I in $RCLIENTS
		do
			RemoveClient $I
		done
		# Get rid of the $ENV part of the bootptab entry also
		grep -v "^$ENV" /etc/bootptab > /tmp/RBP 2>$NULL
		mv /tmp/RBP /etc/bootptab
	else
		N=0; DEL=
		for PR in $SUBPRODS
		do
			RCLIENTS=`egrep "$ENV.*$PR" $RISDB | cut -d: -f1`
			[ "$RCLIENTS" ] && 
			{
				set $PRCHOICE
				shift $N
				JJ=`LeadingBlanks $1`
				P=`egrep "^ $JJ" $RISROOT/ProdNames |
				  sed -e "s/^........//" 2>$NULL`
				echo "
The following clients are registered for $P:"
				echo $RCLIENTS | sort | fmt
				Dialog "
Do you wish to delete this product and unregister the clients? (y/n)" ANS n
				case $ANS in
				[yY] )	;;
				* )
					echo "\nProduct $P was not deleted."
					N=`expr $N + 1`
					continue ;;
				esac
			}
			# If ROOT is in the product we are deleting, setup
			# ROOTF.
			[ -f $RIS/$ENV/$PR/ROOT ] && 
			{
				ROOTF=$RIS/$ENV/$PR/ROOT
			}

			# If hROOT is in the product we are deleting, setup
			# ROOTF.
			[ -f $RIS/$ENV/$PR/hROOT ] && 
			{
				ROOTF=$RIS/$ENV/$PR/hROOT
			}
			# If ROOTF is defined, need to call RisFiles to 
			# delete ROOT files.  Also remove clients registered
			# for that ROOT, as well as bootptab entries.
			if [ "$ROOTF" ]
			then {
				if [ ! -f RisFiles.$PR ]; then {
					ExtractFile $ROOTF RisFiles
					mv RisFiles RisFiles.$PR
				}
				fi
				BOOTFILE=`RisFiles.$PR Delete $ROOTF`
				rm -f RisFiles.$PR

				RCLIENTS=`egrep "$ENV.*$PR" $RISDB | cut -d: -f1`
				for I in $RCLIENTS
				do
					grep -v "^$I:" /etc/bootptab > /tmp/RBP 2>$NULL
					mv /tmp/RBP /etc/bootptab
				done
				# Get rid of the $ENV part of the bootptab entry also
				if [ `basename $ROOTF` = "hROOT" ]; then 
					grep -v "^$ENV.hw:" /etc/bootptab > /tmp/RBP 2>$NULL
				else
					grep -v "^$ENV:" /etc/bootptab > /tmp/RBP 2>$NULL
				fi
				mv /tmp/RBP /etc/bootptab
			}
			fi

			# Now remove the product from entries in risdb
			sed -e "/$ENV.*$PR/s/,$PR//" $RISDB > /tmp/risdbDP
			mv /tmp/risdbDP $RISDB
			DEL=yes
			rm -rf $RIS/$ENV/$PR
			N=`expr $N + 1`
			ROOTF=
		done
		[ $DEL ] && Prodnames $RISROOT
	fi
}

: Exit
Exit()
{
	Error "Usage:
	$CMD -a client_name -p prod_name -h hardware_address
	$CMD -r client_name
	$CMD -m client_name [ -p prod_name ] [ -h hardware_address ]
	$CMD -s
"
	exit 1
}

: ExtractFile
# Extract file from ROOT or HROOT, $1 is path, $2 is file to extract
ExtractFile()
{
	ROOTFILE=$1
	EXFILE=$2
	touch tnc || 
	{
		Error "
The $RISROOT directory is not writable
and no \"$EXFILE\" file exists.  therfore, $CLIENT has not
been registered in the bootp database."
	return 1
	}
	rm tnc

	echo "\nExtracting $EXFILE ...\c"

	#Create an input file for restore
	restore -xf $ROOTFILE $EXFILE 2>$NULL || 
	{
		Error "There is no \"$EXFILE\" file in $ROOTFILE."
		return 1
	}
	echo "done."
}

: Getenvir
# Get RISROOT, the client's installation environment
# Getenvir is called with an argument only when modifying a client.  In
# that case, we want to give only those environment choices that agree
# with the client's architecture.  (In the case of adding a client, we
# give all choices because we have no way of knowing the new client's
# architecture.)

Getenvir()
{
	case $1 in
	"" )
		RISROOT=
		DIRS=`ls -d $RIS/ris[0-9]* 2>$NULL`
		;;
	* )
		RISROOT=$1
		ARC=`expr $RISROOT : '.*\.\(.*\)'`
		DIRS=`ls -d $RIS/*.$ARC 2>$NULL`
		;;
	esac
	set xxx $DIRS
	case $#  in
	1 )
		echo "
There are no directories containing remote installation environments."
		return 1	;;
	2 )
		RISROOT=$2
		echo "\nThe existing environment is $RISROOT."
		return 0 	;;
	esac
	echo "\nSelect the remote installation environment:"
	while :
	do
		DEF=
		INDEX=0
		for I in $DIRS
		do
			INDEX=`expr $INDEX + 1`
			echo "\n    $INDEX)  $I"
			# Remove the first six characters of each product
			# name entry (a number and some blanks)
			sed -e "s/^....../	/" $I/ProdNames 2>$NULL
			[ "$I" = "$RISROOT" ] && DEF=$INDEX
		done
		Dialog "\nEnter your choice" ANS $DEF
		case $ANS in
		[1-9] | [1-9][0-9] | [1-9][0-9][0-9])
			if [ $ANS -le $INDEX ]; then
				set xxx $DIRS
				# In case ANS > 9
				shift $ANS
				RISROOT=$1
				break
			fi
			;;
		esac
		BadChoice "$ANS"
	done
	return 0
}

: Installing
# Installing software from dev or mount point
# $1: temporary directory for extracting images
# $2: environment flag either New or Exist

Installing()
{
	TMPROOT=$1
	ENVFLAG=$2
	Ckdevice || return $?
	case $MEDIA in
	tape )
		case $ENVFLAG in
		New )
			Namepd $TMPROOT
			cd $PDDIR
			Pdsetld $DEV || 
			{
				cd ..; rm -rf $PDDIR
				return 1
			}
			;;
		Exist )
			Ticker on
			PDTMP=/tmp/PROD$$
			mt -f $DEV fsf 3  || return 1
			mkdir $PDTMP
			cd $PDTMP
			tar xf $DEV || return 1
			mt -f $DEV rew || return 1
			NMIMAGE=`ls *.image 2>$NULL`
			case $NMIMAGE in
			"" )
				Error "Cannot find image file."
				return 1
			esac
			Ckprodir $TMPROOT $PDTMP
			rm -rf $PDTMP
			cd $PDDIR
			Ticker off
			Pdsetld $DEV || return 1
			;;
		esac
		;;
	disk )
		for J in $PDIRS
		do
			case $LNK in
			y )
				cd $J
				Namepd $TMPROOT
				set -- `ls -ld $J`
				case $1 in
				dr?xr?xr?x )
					ln -s $J $PDDIR
					;;
				* )
					Error "
Permissions on the $J directory must allow read and execute by world.\n"
					return 1
				esac
				;;
			n )
				case $ENVFLAG in
				New )
					Namepd $TMPROOT
					cd $PDDIR
					Pdsetld $J || 
					{
						cd ..; rm -rf $PDDIR
						return 1
					}
					;;
				Exist )
					Ckprodir $TMPROOT $J/instctrl
					cd $PDDIR
					Pdsetld $J || return 1
					;;
				esac
			esac
		done
	esac
	return 0
}

: Instenv
# Install a new environment
Instenv()
{
	echo "
You have chosen to establish a new remote installation environment."

	TMPROOT=$RIS/TMP.ris
	[ -d $TMPROOT ] && rm -rf $TMPROOT
	mkdir $TMPROOT

	Installing $TMPROOT New || return $?

	case `ls $TMPROOT/*/instctrl/*BIN* 2>$NULL` in
	*ULTBIN* ) ARC=vax
		;;
	*UDTBIN* ) ARC=mips
		;;
	# OSFBIN is the same for alpha and mips
	#*OSFBIN* ) ARC=mips
	#	;;
	#
	# When we know the names of alpha files, add another case
	# and perhaps change the alpha choice to another mnemonic

	* )	while :
		do
			Dialog "
Choose the architecture of the clients that the environment
will serve:

    1) alpha
    2) custom
    3) mips

Enter your choice" ANS
			case $ANS in
			1) ARC=alpha ;;
			2) ARC=custom ;;
			3) ARC=mips ;;
			*) continue ;;
			esac
		break
		done
	esac

	NUM=0
	while :
	do
		if [ -d "$RIS/ris$NUM.$ARC" ]; then
			NUM=`expr $NUM + 1`
		else
			RISROOT=$RIS/ris$NUM.$ARC
		     	break
		fi
	done
	mv $TMPROOT $RISROOT
	chown ris $RISROOT; chgrp ris $RISROOT
	echo "\nThe new environment is in $RISROOT."
}

: Instprod:
# Install new product to existing environment.
Instprod()
{
	echo "\nYou have chosen to add a product to an existing environment."

	Getenvir || return $?
	Installing $RISROOT Exist
	return $?
}

: Instsoft
# Install software to a new or existing area
Instsoft()
{
	while :
	do
		Dialog "
RIS Software Installation Menu:

    1)  Install software into a new area
    2)  Add software into an existing area
    3)  Return to previous menu

Enter your choice" ANS
		case $ANS in
		1 ) Instenv	;;
		2 ) Instprod	;;
		3 ) :		;;	# Set return code
		* ) continue	;;
		esac
		return $*
	done
}

: LeadingBlanks
# Act as though doing printf("%3d", $1);
LeadingBlanks()
{ (
	A=`expr "   $1" : '.*\(...\)'`
	echo "$A"
) }

: Listclients
# List clients in each RIS environment
Listclients()
{ (
	[ -s $RISDB ] || 
	{
		echo "There are no clients registered at this time"
		return
	}

	DIRS=`ls -d $RIS/ris[0-9]* 2>$NULL`
	for I in $DIRS
	do
		J=`basename $I`
		grep $J $RISDB > /tmp/risdb
		if [ -s /tmp/risdb ]; then
			echo "\nThe following clients are registered for $I:"
			cut -d: -f1 /tmp/risdb | sort | fmt
		else
			echo "\nThere are no clients registered for $I."
		fi
	done
	rm -f /tmp/risdb
) }

: Lock_ris
# Lock ris
Lock_ris()
{
	Cklock_ris || exit 1
	mknod /tmp/rislock p
	echo `/usr/bin/tty` > /tmp/ris.tty.lock
	(
		exec < /tmp/rislock
		read X
		rm -f /tmp/rislock /tmp/ris.tty.lock
	) &
}

: Mkris
# Add client info to all necessary data bases
Mkris()
{
	set `Parse , $PRODS`
	RISROOT=$RIS/$1
	shift
	SUBPRODS=$*

	QUALIFIEDCLIENT=`getname -l $CLIENT` || UnlockAndExit
	[ "$QUALIFIEDCLIENT" ] || QUALIFIEDCLIENT=$CLIENT

	# Take client out of bootp database
	[ -f /etc/bootptab ] && {
		grep -v "^$CLIENT:" /etc/bootptab > /tmp/bootptab$$
		mv /tmp/bootptab$$ /etc/bootptab
	}	

	egrep "^$QUALIFIEDCLIENT " $RIS/.rhosts  >$NULL 2>&1 ||
		echo "$QUALIFIEDCLIENT root" >>$RIS/.rhosts
	chown ris $RIS/.rhosts; chgrp ris $RIS/.rhosts

	KITPATH=$RISROOT/kit
	grep "$KITPATH" /etc/exports >$NULL 2>&1 ||
	{
		echo "$KITPATH  -root=0  -ro " >> /etc/exports
	}

	# If client is not registered for ROOT, no need to do bootp
	# registration

	cd $RISROOT
	FOUND=
	FOUNDHROOT=
	ROOTF=
	BOOTFILE=
	BOOTFILEH=
	for I in $SUBPRODS
	do
		[ -f $I/ROOT ] && 
		{
			FOUND=yes
			ROOTF=$RISROOT/$I/ROOT
		}
		[ -f $I/hROOT ] && 
		{
			FOUNDHROOT=yes
			ROOTF=$RISROOT/$I/hROOT
		}
		[ "$ROOTF" ] &&
		{
			[ ! -f RisFiles.$I ] &&
			{
				ExtractFile $ROOTF RisFiles
				mv RisFiles RisFiles.$I
			}
			if [ "$FOUNDHROOT" ] 
			then
				BOOTFILEH=`RisFiles.$I Extract $ROOTF $FIXIT`
				FOUNDHROOT=
			else
				BOOTFILE=`RisFiles.$I Extract $ROOTF $FIXIT`
			fi
			ROOTF=
		}
	done
	[ "$FOUND" ] || 
	{
		Putrisdb $CLIENT:$CLINET:$PRODS
		return 0
	}

	[ "$CLINET" = none ] && Getether

	set -- `arp $CLIENT`; T_CLIARP=$2
	set -- `Parse '()' ${T_CLIARP}` ;  CLIARP=$1

	grep -q "^ris.dec" /etc/bootptab 2>$NULL || 
		echo "ris.dec:hn:ht=ethernet:vm=rfc1048" >> /etc/bootptab
	TC=`basename $RISROOT`
	set xx `Parse . $TC`
	ARC=$3

    if [ "$BOOTFILE" ] 
	then
		grep -q "^$TC:" /etc/bootptab ||
			echo "$TC:tc=ris.dec:bf=$RISROOT/$BOOTFILE:" >>/etc/bootptab
	fi
	if [ "$BOOTFILEH" ] 
	then
		TC="$TC.hw"
		grep -q "^$TC:" /etc/bootptab || 
			echo "$TC:tc=ris.dec:bf=$RISROOT/$BOOTFILEH:" >>/etc/bootptab
	fi
	HA=`echo $CLINET | sed 's/-//g'`
	echo "$CLIENT:tc=$TC:ha=$HA:ip=$CLIARP:" >>/etc/bootptab

	grep -q "^tftp" /etc/inetd.conf || {
		echo "Setting up tftp in inetd.conf..."
		echo "tftp   dgram   udp     wait    root     /usr/sbin/tftpd         tftpd  /tmp $RIS" >> /etc/inetd.conf
	}
	grep -q "^bootp" /etc/inetd.conf || {
		echo "Setting up bootp in inetd.conf..."
		case `egrep '^bootps?' /etc/services` in
		"" )
			echo "No bootp entry in /etc/services"
			;;
		bootps* )
			echo "bootps  dgram   udp     wait    root    /usr/sbin/bootpd        bootpd" >> /etc/inetd.conf
			;;
		* )
			echo "bootp  dgram   udp     wait    root    /usr/sbin/bootpd        bootpd" >> /etc/inetd.conf
			;;
		esac
	}
	GREPANS=`ps -e | grep inetd | grep -v grep`
	case "$GREPANS" in
	"" )
		/usr/sbin/inetd
		;;
	* )
		set $GREPANS; kill -1 $1
		;;
	esac

	Putrisdb $CLIENT:$CLINET:$PRODS
	return 0
}

: Modclient
# Modify client
Modclient()
{
	[ -s $RISDB ] || 
	{
		echo "There are no clients available to modify.\n"
		return
	}
	echo "\nThe following clients are available to modify: \n"
	cut -d: -f1 $RISDB | sort | fmt

	# Getname sets $CLIENT and $ENT
	Getname existing $RISDB

	set `Parse : $ENT`
	OLDCLINET=$2; OLDPRODS=$3
	set `Parse , $OLDPRODS`
	OLDRISROOT=$RIS/$1
	shift
	SUBPRODS=$*

	Getenvir $OLDRISROOT

	# If the client has the same environment as before and
	# is registered for products, show them.
	[ "$OLDRISROOT" = "$RISROOT" -a "$SUBPRODS" ] && {
		echo "
Client $CLIENT currently can install the following products
from $RISROOT:\n"
		for J in $SUBPRODS
		do
			(cd $RISROOT/$J/instctrl
			set xxx `ls *.ctrl 2>$NULL`
			I=$2
			S_ctrl=`expr $I : '\(.*\).ctrl'`
			echo "    `egrep "NAME=" $I |
			   sed -e "s/NAME=//" | sed -e "s/${S_ctrl}//"`"
			)
		done
	}

	SelectProds install || return

	# In case we don't ask for a new ethernet addr
	CLINET=$OLDCLINET

	set `Parse , $PRODS`
	shift
	SUBPRODS=$*
	FOUND=
	for I in $SUBPRODS
	do
		[ -f $I/ROOT ] && {
			FOUND=1
			ROOTF=$I/ROOT
			break
		}
	done
	[ "$FOUND" ] && 
	{
#		[ "$OLDCLINET" = none ] && {
#			Getether
#		} || {
#			Getether $OLDCLINET
#		}

		if [ "$OLDCLINET" = none ]; then
			Getether
		else
			Getether $OLDCLINET
		fi
	}

	if [ "$OLDPRODS" = "$PRODS" -a "$CLINET" = "$OLDCLINET" ]; then
		echo "\nClient $CLIENT was not modified."
		return
	else
		Mkris || return
		echo "\nClient $CLIENT has been modified."
	fi
}

: Namepd
# Make a proper product directory, unless it will be a link.
# If directory is going to be a link then don't mkdir.
Namepd()
{
	TMPROOT=$1
	cd $TMPROOT
	NUM=1
	while :
	do
		NUM0=`expr "000$NUM" : '.*\(...\)'`
		if [ -d "product_$NUM0" ]; then
			NUM=`expr $NUM + 1`
		else
			PDDIR=$TMPROOT/product_$NUM0
			break
		fi
	done
	case $LNK in
		y )
			;;
		n )
			mkdir $PDDIR
			chown ris $PDDIR; chgrp ris $PDDIR
			;;
	esac
}

: Pdsetld
# Extract image using setld
Pdsetld()
{
	DEV=$1
	setld -x $DEV || {
		 Error "setld failed."
		 return 1
	}
	rm -f all mandatory
	return 0
}

: Putrisdb
# Add client info ($1) to risdb
Putrisdb()
{
	[ -f $RISDB ] && {
		grep -v "^$CLIENT:" $RISDB > /tmp/risdb$$
		mv /tmp/risdb$$ $RISDB
	}
	echo "$1" >> $RISDB
}

: Prodnames
# Grep product name string from subsets ctrl files to a file, "ProdNames".
# $1 is the name of the ris#.arc directory.
Prodnames()
{
	PNDIR=$1
	PNFILE=$PNDIR/ProdNames
	cd $PNDIR
	CTRLDIR=`ls -d */instctrl 2>$NULL`
	case $CTRLDIR in
	"" )
		rm $PNFILE 2>$NULL
		return	;;
	esac

	trap '' 1 2 3
	> $PNFILE
	J=0
	for K in $CTRLDIR
	do
		J=`expr $J + 1`
		(cd $K
		# This code picks out the name string out of the
		# first .ctrl file
		C=`ls *.ctrl 2>$NULL`
		set xxx $C
		# do nothing if no .ctrl files
		[ $# -lt 2 ] && continue
		I=$2
		S_CTRL=`expr $I : '\(.*\).ctrl'`
		D_S=`egrep "NAME=" $I | sed -e "s/NAME=//" |
			sed -e "s/${S_CTRL}//"`

		JJ=`LeadingBlanks $J`

		echo " $JJ    $D_S" >>$PNFILE
		)
	done
	# Check for empty file
	[ -s $PNFILE ] || rm $PNFILE
	trap 1 2 3
}

: RemoveClient
# Remove a client
RemoveClient()
{
	CLIENT=$1
	cd $RIS/clients

	grep -v "^$CLIENT:" $RISDB >/tmp/risdb
	# Make sure we still have clients or no risdb file at all
	grep : /tmp/risdb > $RISDB
	rm -f /tmp/risdb

	QUALIFIEDCLIENT=`getname -l $CLIENT` || UnlockAndExit
	[ "$QUALIFIEDCLIENT" ] || QUALIFIEDCLIENT=$CLIENT
	grep -v "^$QUALIFIEDCLIENT " $RIS/.rhosts >/tmp/.rhosts 2>$NULL
	mv /tmp/.rhosts $RIS/.rhosts

	grep -v "^$CLIENT:" /etc/bootptab > /tmp/bootptab$$ 2>$NULL
	mv /tmp/bootptab$$ /etc/bootptab
}

: Rismenu
# Display the ris main menu
Rismenu()
{
# Check and set menu-control variables:
# 	SwEx	At least one RIS area contains installed software
# 	ClEx	At least one RIS client is registered

[ -d /$RIS/ris*.*/product_* ] && SwEx=y
[ -s $RISDB ] && ClEx=y

#
# Execution starts here
#
while :
do
	MMs=" " MMd=" " MMa=" " MMr=" " MMm=" " MMl=" "
	[ $SwEx ] && MMs=s && MMd=d && MMa=a
	[ $ClEx ] && MMr=r && MMm=m && MMl=l

	echo "\n*** RIS Utility Main Menu ***"
	[ "$SwEx$ClEx" = yy ] || echo "
Choices without key letters are not available."
	Dialog "
    $MMa) ADD a client
    $MMd) DELETE software products
    i) INSTALL software products
    $MMl) LIST registered clients
    $MMm) MODIFY a client
    $MMr) REMOVE a client
    $MMs) SHOW software products in remote installation environments
    x) EXIT

Enter your choice" ANSWER
	ANSWER=`ToLower $ANSWER`
	case $ANSWER in
	[iarmlds] )
		Cklock_ris || continue
	esac
	case $ANSWER in
	[i] )	trap 'UnlockAndExit' 1 2 3
		Lock_ris
		Instsoft && SwEx=y
		Unlock
		Prodnames $RISROOT
		;;

	[a] )	if [ $MMa ]
		then
			Addclient && ClEx=y
		else
			BadChoice $ANSWER
		fi
		;;

	[r] )	if [ $MMr ]
		then
			Rmclient
			ClEx=
			[ -s $RISDB ] && ClEx=y
		else
			BadChoice $ANSWER
		fi
		;;

	[m] )	if [ $MMm ]
		then
			Modclient
		else
			BadChoice $ANSWER
		fi
		;;

	[l] )	if [ $MMl ]
		then
			Listclients
		else
			BadChoice $ANSWER
		fi
		;;

	[s] )	if [ $MMs ]
		then
			Showprod
		else
			BadChoice $ANSWER
		fi
		;;

	[d] )	if [ $MMd ]
		then
			trap 'UnlockAndExit' 1 2 3
			Lock_ris
			Deleteprod
			Unlock
			ClEx= ; SwEx=
			[ -s $RISDB ] && ClEx=y
			[ -d /$RIS/ris*.*/product_* ] && SwEx=y
		else
			BadChoice $ANSWER
		fi
		;;

	[x] )	exit 0 	;;

	* )	BadChoice $ANSWER ;;
	esac
done
}

: Rmclient
# First layer of remove client
Rmclient()
{
	echo "
You have chosen to remove a client from the remote installation
services."

	Getname existing $RISDB
	Dialog "\nRemove ${CLIENT}? (y/n)" ANS n
	case $ANS in
	[yY] )	;;
	* )
		echo "$CLIENT was not removed."
		return ;;
	esac

	RemoveClient $CLIENT
}

: SelectProds
# Select products from RISROOT and set environment variable ALLPRODS
# if all products are chosen.  Also, variable PRCHOICE is left set to the
# product choice answer that the user entered.
SelectProds()
{
	case $1 in
	delete )
		SELECTMSG="
Select one or more products to delete from
$RISROOT:"
		DefaultProd=
		;;
	install )
		SELECTMSG="
Select one or more products for the client to install
from $RISROOT:"
		DefaultProd=all
		;;
	esac

	[ -s $RISROOT/ProdNames ] || Prodnames $RISROOT

	cd $RISROOT
	INSTDIRS=`ls -d */instctrl 2>$NULL`
	case $INSTDIRS in
	"" )
		Error "There are no products in $RISROOT."
		return 1 ;;
	esac
	DIRS=
	for I in $INSTDIRS
	do
		DIRS="$DIRS `dirname $I`"
	done
	set $DIRS
	NUMDIRS=$#

	while :
	do
		ALLPRODS=

		# $PRODS is initialized to the basename of the $ROOTDIR,
		# product names are appended to it.
		PRODS=`expr $RISROOT : '.*\/\(.*\)'`

		echo "$SELECTMSG\n\nProduct    Description"
		cat $RISROOT/ProdNames 2>$NULL
		Dialog "
Enter one or more choices as a space-separated list
(for example, 1 2 3) or \"all\" for all products" PRCHOICE $DefaultProd
		[ "$PRCHOICE" ] || continue
		VAL=yes
		set $PRCHOICE

		# Validate choices 

		[ $# -eq 1 ] && [ $PRCHOICE -eq "all" ] &&
		{
			J=1
			LIM=`expr $NUMDIRS - 1`
			PRCHOICE=1
			while [ $J -le $LIM ]
			do
				J=`expr $J + 1`
				PRCHOICE="$PRCHOICE $J"
			done
		}
		[ "$VAL" ] && 
		{
			PASS=yes
			for J in $PRCHOICE
			do
				case $J in
				[1-9] | [1-9][0-9] | [1-9][0-9][0-9] )
					[ $J -le $NUMDIRS ] && continue
					;;
				esac
				BadChoice $J
				PASS=
			done

			# Continue while loop, show choices again if any
			# invalid choices
			[ "$PASS" ] || continue

			# Sort the list and get rid of duplicates
			PRCHOICE=`echo $PRCHOICE | tr '\040' '\012' | sort -un`

			set $PRCHOICE
			[ $# -eq $NUMDIRS ] && ALLPRODS=yes

			echo "\nYou chose the following products: \n"
			for J in $PRCHOICE
			do
				set xxx $DIRS
				JJ=`LeadingBlanks $J`
				egrep "^ $JJ" $RISROOT/ProdNames
				# In case J > 9
				shift $J
				PRODS="$PRODS,$1"
			done
		}
		while :
		do
			Dialog "\nIs that correct? (y/n)" ANS y
			case $ANS in
			"" | [Yy]*)	break	;;
			*) 	continue 2 ;;
			esac
		done
		break
	done
	return 0
}

: Showprod
# Show products in remote installation environment
Showprod()
{
	DIRS=`ls -d $RIS/ris[0-9]* 2>$NULL`
	case $DIRS in
	"" )
		echo "\nNo remote installation environment exists.\n"
		return
		;;
	esac
	N=0
	for I in $DIRS
	do
		N=`expr $N + 1`
		echo "\n$N  $I"
		[ -s $I/ProdNames ] || Prodnames $I
		if [ -s $I/ProdNames ]; then
			sed -e "s/^....../    /" $I/ProdNames 2>$NULL
		else
			echo "There are no products installed in $I"
		fi
	done
}

: Unlock
# Unlock ris
Unlock()
{
	[ -p /tmp/rislock ] && echo > /tmp/rislock
	trap 1 2 3
}

: UnlockAndExit
# Unlock ris and exit
UnlockAndExit()
{
	Unlock
	exit 1
}

#
# Main program starts here
#
CMD=$0
case $# in
0 )
	Rismenu
	;;
esac

# Flag is set upon seeing a -a (Add client) or a -m (Modify client)
FLAG=
while [ $# -gt 0 ]
do
	case $1 in
	-a )
		# We need more arguments for add, and we shouldn't have
		# seen a -a or a -m before.
		[ "$FLAG" ] && Exit
		case "$3" in
		-* )
			FLAG=Addflg
			CLIENT=$2
			shift
			;;
		* )
			Exit
		esac
		;;
	-h )	CLINET=$2; shift
		;;
	-p)
		PRODS=$2; shift
		;;
	-r)
		[ "$FLAG" ] && Exit
		[ "$2" ] || Exit
		shift
		for I in $*
		do
			RemoveClient $I
		done
		exit 0
		;;
	-m )
		[ "$FLAG" ] && Exit
		FLAG=Modflg
		CLIENT=$2; shift
		;;
	-s )
		Showprod
		exit 0
		;;
	* )
		Exit
	esac
	shift
done
Ckname $CLIENT || Exit

case $FLAG in
Addflg )
	[ "$CLINET" ] || CLINET=none
	[ "$PRODS" ] || Exit
	;;
Modflg )
	ENT=`grep "^$CLIENT:" $RISDB 2>$NULL` || {
		Error "\
There is no entry for \"$CLIENT\" in the risdb file .\n"
		Exit
	}
	set `Parse : $ENT`
	OLDCLINET=$2; OLDPRODS=$3
	[ "$CLINET" ] || CLINET=$OLDCLINET
	[ "$PRODS" ] || PRODS=$OLDPRODS
	;;
* )
	Exit
esac

[ "$CLINET" = none ] || 
{
	CLINET=`Ckether $CLINET` || Exit
}

Mkris
exit $?
