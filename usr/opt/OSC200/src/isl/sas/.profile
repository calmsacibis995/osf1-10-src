#!/sbin/sh
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
# @(#)$RCSfile: .profile,v $ $Revision: 1.2.7.7 $ (DEC) $Date: 1994/01/03 20:21:35 $ 
# 
#	.miniprofile: the installation only
#	"%W%	(DEC OSF/1)	%G%"
#
#	*********************************************************************
#	*                                                                   *
#	*      Copyright (c) Digital Equipment Corporation, 1991, 1994      *
#	*                                                                   *
#	*                       All Rights Reserved.                        *
#	*                                                                   *
#	*********************************************************************
#
#	000	12-dec-1991	ccb
#		ported from ULTRIX V.LA


#:	-restart
#
#		restart the installation, can be called from user
#	at the command line

restart()
{
	Main
}



:	-Error
#		write to stderr
#

Error()
{
	1>&2 echo "$*"
}



:	-GetInstMedia
#		get the device information about the distribution
#
#	given:	nil
#	does:	determines what the installation device is
#	return:	nothing
#
#	side effects:
#		sets global variables:
#			NLOAD, ULOAD, TLOAD, BLOAD, PLOAD, CLOAD
#

GetInstMedia()
{
	set xx `btd`
	shift

	BTD=$1

	# NLOAD = "name" of device, "rz", "BOOTP", "MOP", etc.
	# ULOAD = unit #

	NLOAD=`expr "$BTD" : '\([a-zA-Z][a-zA-Z]*\)'`
	ULOAD=`expr "$BTD" : '[a-zA-Z][a-zA-Z]*\([0-9][0-9]*\)'`

	# cull out illegal device types

	case "$BTD" in
	tms*|rmt*|tz*)
		Error "Tape devices are not supported for installation."
		return 1
		;;
	esac

	# set TLOAD, etc for all devices. TLOAD is the device type
	#  in {REMOTE,CDROM}.

	case $BTD in
	MOP|BOOTP|NETWORK)
		TLOAD=REMOTE

		# call gethost. this turns on the network, writes
		#  an /etc/hosts file and writes /netstart, which is
		#  used to restart the network after the reboot.
		#  gethost puts 2 variable assignments on stdout:
		#   CLIENT=<clientname> SERVER=<servername>

		eval `gethost`
		2>&1 routed -q > $NUL

		NLOAD=$SERVER
		# attempt to get real client and server names
		NetUpdate || return 1
		;;
	rz*)	TLOAD=CDROM
		PLOAD=c
		BLOAD=$NLOAD$ULOAD$PLOAD
		CLOAD=r$BLOAD
		(cd $D; MAKEDEV $BTD > $NUL 2>&1)
		;;
	*)	# unknown device
		Error "$BTD: unknown device"
		return 1
	esac
	return 0
}



:	-GetName
#		perform IPADDR to name translation via server
#
#	given:	$1 - server to query
#		$2 - address to translate
#	does:	translate the address
#		name is written to stdout
#	return:	0 if successful

GetName()
{(
	SRV=$1
	IPADDR=$2
	TMP=/tmp/ris.ent

	rsh $SRV -l ris -n bin/getname $IPADDR 2> $NUL > $TMP
	[ -s $TMP ]  || return 1
	cat $TMP
	rm -f $TMP
	return 0
)}



:	-GetRisDBEnt
#		get client entry from risdb on server
#
#	given:	$1 - name of server to query
#		$2 - name of client to look up
#	does:	looks up client at server
#		writes risdb entry on stdout
#	return:	0 if successful

GetRisDBEnt()
{(
	SRV=$1
	CLI=$2
	TMP=/tmp/ris.ent

	rsh $SRV -l ris -n grep "'^'$CLI:" clients/risdb 2> $NUL > $TMP
	[ -s $TMP ]  || return 1
	cat $TMP
	rm -f $TMP
	return 0
)}



:	-GetRisDir
#		get the name of the directory to mount from
#	the server.
#
#	given:	nothing
#	does:	looks up the ris directory in the clinets database
#			and returns the path to mount from the server
#	return:	0 if it all works
#
#	side effects:
#		updates the client and server names

GetRisDir()
{

	RISHOME=`GetRisHome $NLOAD` ||
	{
		Error "Cannot determine RIS home directory on $NLOAD"
		return 1
	}

	RISENT=`GetRisDBEnt $NLOAD $CLIENT` ||
	{
		Error "
Cannot find $CLIENT in risdb file.  Check with the system manager of
your RIS server."
		return 1
	}

	# parse ris entry:
	#  <client>:<hwaddr>:<ris0.x-path>,<prod1>,...,<prodn>

	set xx `Parse : $RISENT`
	shift

	PRODS=$3	# the <ris0.x-path>,<prod1>,...,<prodn> part

	set xx `Parse , $PRODS`
	shift

	echo $RISHOME/$1
	return 0
}



:	-GetRisHome
#		determine RIS home directory on server
#
#	given:	$1 - name of server to query
#	does:	look up home directory of ris on server
#		writes home dir on stdout
#	return:	0 if successful

GetRisHome()
{(
	SRV=$1
	RISINFO=/tmp/ris.info

	rsh $SRV -l ris "/bin/pwd" 2> $NUL > $RISINFO
	[ -s "$RISINFO" ] || return 1

	cat $RISINFO
	rm -f $RISINFO
	return 0
)}



:	-Main
#		main function, no arguments
#
#	given:	nil
#	does:	set up the SAS environment so that install.osf can
#		run. Runs install.osf with logging.
#	return:	If install.1 exits with 0 status, the system is
#		halted. If install.1 exits with !0 status, Main
#		returns 1 and the user is given a prompt.

Main()
{
	ISLPATH=/kit/isl
	[ -f /hinstall ] &&
	{
		HINSTALL=1
		ISLPATH="/kit/hisl:$ISLPATH"
	}
	PATH=.:$ISLPATH:/sbin:/etc
	ISLINFO=/.islinfo

	export HINSTALL ISLINFO ISLPATH PATH

	D=/dev; NUL=$D/null

	stty dec new prterase
	mount -u /

	# figure out what we booted from
	GetInstMedia || return 1

	# mount the /kit directory
	MountMedia || return 1

	WriteIslInfo	# save state for install.1 and later scripts...

	# call install.1 - this uses the install.1 most readily
	#  available in the PATH. This must remain this way for
	#  hardware installations to work.

	INSTALL1=/kit/isl/install.1
	[ "$HINSTALL" ] &&
	{
		[ -f /kit/hisl/install.1 ] && INSTALL1=/kit/hisl/install.1
	}

	log /install.log /sbin/sh $INSTALL1 &&
	{
		sync; sync; sync
		halt
	}
	return 1
}



:	-MountMedia
#		mount the distribution media on /kit
#
#	given:	nil - uses global TLOAD, NLOAD, BLOAD
#	does:	mount the /kit directory
#	return:	0 if successful, 1 otherwise

MountMedia()
{

	if [ -d /kit ]; then
	{
		umount /kit > $NUL 2>&1
	}
	else
	{
		mkdir /kit

	}; fi

	case $TLOAD in
	REMOTE)
		RISDIR=`GetRisDir` || return 1

		mount $NLOAD:$RISDIR/kit /kit ||
		{
			Error "Cannot mount $NLOAD:$RISDIR/kit on /kit."
			return 1
		}
		;;
	CDROM)
		mount -dr $D/$BLOAD /kit ||
		{
			Error "Cannot mount $TLOAD $BLOAD on /kit."
			return 1
		}
		;;
	esac
	return 0
}



:	-NetUpdate
#		update the networking info
#
#	given:	nil
#	does:	look up the client and server names at the server
#		and update the hosts file and netstart scripts
#	return:	0 if successful
#
#	side effects:
#		will modify NLOAD and CLIENT

NetUpdate()
{
	# For BOOTP, get the real client and server hostname
	# We know $CLIENT is "CLIENT" if bootp was used.
	# NLOAD is really the remote server name ($SERVER)

	[ "$CLIENT" = CLIENT ] &&
	{
		# first, translate client IP address to a name
		CLIENT=`GetName $NLOAD $CLIENTIP` ||
		{
			Error "
Cannot find the name for $CLIENTIP using bin/getname.  Check with
the system manager of your RIS server."
			return 1
		}
		
		NLOAD=`GetName $NLOAD $SERVERIP` ||
		{
			# continue with server name = server
			NLOAD=server
		}

		# reset our hostname to the real name
		2>&1 /sbin/hostname $CLIENT > $NUL

		# fill in /etc/hosts with real names
		echo "
1,\$s/CLIENT/$CLIENT/
1,\$s/SERVER/$NLOAD/
w
q" | 1> $NUL ed - /etc/hosts

# put real client name in /netstart
		echo "
1,\$s/CLIENT/$CLIENT/
w
q" | 1> $NUL ed - /netstart

	}
	return 0
}



:	-Parse
#		break up a string into tokens using a separator
#
#	given:	$1 - separator character
#		$n - string to break up
#	does:	break $n into tokens, writes the tokens on stdout
#	return:	0

Parse()
{(
	IFS=$1
	shift
	echo $*
)}



:	-WriteIslInfo
#		write out the info needed to know what this .profile knows
#
#	given:	nil - uses global $ISLINFO
#	does:	writes a bunch of state variables to $ISLINFO
#	return:	0

WriteIslInfo()
{(
	echo "
BLOAD=$BLOAD		# block-special name, load device eg. rz2c
CLIENT=$CLIENT		# client (me) hostname eg. jetco
CLOAD=$CLOAD		# char-special name, load device eg. rrz2c
NLOAD=$NLOAD		# name fragment, load device eg. rz
PLOAD=$PLOAD		# partition, load device eg. c
RISDIR=$RISDIR		# ris kit dir eg. /var/adm/ris/ris0.alpha
RISENT=$RISENT		# risdb entry eg. jetco:00-00-00-00-00-00:ris0.alpha...
RISHOME=$RISHOME	# ~ris on server eg. /var/adm/ris
SERVER=$SERVER		# ris server hostname eg. earwig
TLOAD=$TLOAD		# type, load device eg. CDROM
ULOAD=$ULOAD		# unit # load device eg. 2

"		 > $ISLINFO

	return 0
)}

[ "$CHECK_SYNTAX" ] || Main "$@"


