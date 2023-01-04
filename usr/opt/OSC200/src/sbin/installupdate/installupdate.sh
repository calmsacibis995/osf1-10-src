#!/sbin/sh
#
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
# @(#)$RCSfile: installupdate.sh,v $ $Revision: 1.1.4.5 $ (DEC) $Date: 1993/12/21 21:43:00 $
# 

#
# Read in external functions
#
. /usr/share/lib/shell/Error
. /usr/share/lib/shell/Strings


: BakeMagicCookie - 
#
#	Given:	Nothing
#
#	Does:	Writes variables out to UPDINFO.  The UPDINFO file is
#		later sourced by updeng.  This method is used in lieu
#		of exporting the variables, or using them as command
#		line arguments.
#
#	Return: Nothing
#
BakeMagicCookie()
{
	echo "TLOAD=\"$TLOAD\"" >  $UPDINFO
	echo "NLOAD=\"$NLOAD\"" >> $UPDINFO
	echo "LOCMNT=\"$LOCMNT\"" >> $UPDINFO
	echo "SERVER=\"$SERVER\"" >> $UPDINFO
	echo "REMMNT=\"$REMMNT\"" >> $UPDINFO
	echo "MNTFLAG=\"$MNTFLAG\"" >> $UPDINFO
	echo "PRODPATH=\"$PRODPATH\"" >> $UPDINFO
	echo "PRODUCTS=\"$PRODUCTS\"" >> $UPDINFO
	echo "ISL_PATH=\"$ISL_PATH\"" >> $UPDINFO
}


: Cleanup -
#
#	Given:	Nothing
#
#	Does:	Cleans up the system by removing temporary files and
#		mount points used by these scripts.  Also calls UnmountFS
#		to release any mount points that might have been mounted
#		by these scripts.
#
#	Return: Nothing
#
Cleanup()
{
	cd /

	#
	# Cleanup any temporary files still laying around.
	#
	rm -rf $UPDINFO

	#
	# Try to unmount our update installation  mount points
	#
	UnmountFS $LOCMNT
}


: Exit -
#
#	Given:	$1 - An exit status
#
#	Does:	Calls 'Cleanup', then exits with the status passed
#		in.
#
#	Return: Nothing
#
Exit()
{
	Cleanup
	exit $1
}


: FindMntInfo -
#
#	Given:	$1 - a local mount point
#
#	Does:	Uses the mount command to find the entry mounted on $1.
#		If the mount device is a local device, we echo back the
#		local device and the mount point.  If the mount device
#		is an NFS server, we parse the information to get the
#		server and exported file system, and then echo back the
#		local mount point, the server, and the exported file
#		system.
#
#	Return: Nothing
#
FindMntInfo()
{(
	#
	# Grep through the mount command for $1
	#
	set -- `mount | grep $1` &&
	{
		case $1 in
		*:* )
			#
			# $1 is the mount device.  If it's got a ":"
			# in it, it's a NFS mount device.  Parse out
			# the ":", set the remaining fields and echo
			# them back to the calling function.
			#
			MNTPNT=$3
			set -- `Parse : $1`
			echo "$MNTPNT $*"
			;;
		* )
			#
			# Ok, it's a local device.  Just echo back the
			# mount point and the device name.
			#
			echo "$3 $1"
			;;
		esac
	}
)}


: GetRisInfo -
#
#	Given:	Nothing
#
#	Does:	Determines the client name, and then searches for the
#		client in the risdb file.  If it finds an entry for the
#		client, it parses out the ris area (PRODPATH) and the
#		product areas (PRODUCTS) that the client is registered
#		for, and echos those back.
#
#	Return: 0 - complete success
#		1 - any failure
#
GetRisInfo()
{(
	#
	# Determine the system name of the client.
	#
	CLIENT=`/bin/hostname`
	[ "$CLIENT" ] ||
	{
		Error "\nCannot determine the name of this system."
		return 1
	}

	#
	# Now that we have the system name, we'll grep for it in the
	# ris server clients database file.  If it doesn't exist in the
	# database, we'll assume the system name has a bind extension.
	# We'll strip off the bind extension, and search for just the
	# system name.  If that search is successful, we'll extract the
	# product area information, which will give us the paths for the
	# subsets and instctrl information.  
	#
	ENT=`SearchRisdb $CLIENT` ||
	{
		set -- `Parse . $CLIENT`
		ENT=`SearchRisdb $1` || 
		{
			Error "Cannot find $CLIENT in risdb file."
			Error "Check with the system manager of $SERVER"
			return 1
		}
	}

	set -- `Parse : $ENT`
	set -- `Parse , $3`
	echo $*
	return 0
)}


: Main -
#
#	Given:	$1 - Command line argument
#
#	Does:	Drives the rest of the program.
#
#	Return: Nothing
#
Main()
{
	#
	# First thing, check to make sure we are root.
	#
	[ `whoami` = "root" ] ||
	{
		Error "\nYou must have super user privileges to run $0."
		exit 1
	}

	#
	# Set program name, and check for stuff on the command line.
	# We need at least one thing, the device.  If the command line
	# is null, give a usage message and exit.
	#
	PROG=$0

	if [ $# -gt 0 ]
	then
		CMDFLAGS=$*
	else
		Usage
		Exit 1
	fi

	#
	# Set Global Variables
	#
	SetGlobals

	#
	# Start swapping
	#
	swapon -a 1>/dev/null

	#
	# MIF is the Media Information Flag.  It gets set as long
	# as something on the command line resembles a media
	# installation device.  If it doesn't get set, then we exit.
	#
	MIF=

	for K in $CMDFLAGS
	do
	case $K in
	/* )
		#
		# Set MIF flag
		#
		MIF=1

		#
		# This is either a local device, or a local mount point.
		# In either case, we'll label it CDROM. 
		#
		TLOAD=CDROM
		MNTFLAG="-dr"
		PRODUCTS="ALPHA/BASE ALPHA/UPDATE ALPHA/hUPDATE"

		#
		# We need to determine if the user entered a block device,
		# a character device, or a local mount point, and take
		# appropriate action.
		#
		if [ -b $K ]
		then
			#
			# This is a block device.  We'll need to mount it
			# up for the user.
			#
			SERVER=$K
			MountFS $MNTFLAG $LOCMNT $SERVER || Exit 1
			NLOAD=$LOCMNT
		elif [ -c $K ]
		then
			#
			# This is a character device.  We can't mount it
			# so we'll ask the user to specify it as a block
			# device.
			#
			Error "Please specify a block-special device file."
			Exit 1
		else
			#
			# This must be a mount point.  If we can't find the
			# isl directory on the mount point, it's either not
			# update install media or it's mounted wrong. 
			#
			[ -d $K/isl ] ||
			{
				Error "$K is an invalid update \c"
				Error "installation mount point."
				Exit 1
			}

			#
			# Ok then, it looks legit.  But before we do
			# anything else, we need to get some information
			# about the LOCal mount point.  Like on what is it
			# mounted.  updeng will need that information later
			# on.
			#
			set -- `FindMntInfo $K`
			LOCMNT=$1; SERVER=$2; REMMNT=$3
			NLOAD=$LOCMNT
			ISL_PATH="$LOCMNT/isl:$LOCMNT/hisl"
		fi
		PRODPATH=$LOCMNT
		;;
	*: )
		#
		# Set MIF flag
		#
		MIF=1

		#
		# This is an installation from a RIS area.  We'll set some
		# variables, and then strip off the ":" from the command
		# line argument.
		#
		TLOAD=REMOTE
		NLOAD=$K
		MNTFLAG="-r"
		SERVER=`echo $K | sed 's/://'`

		#
		# Start any network processes necessary.
		#
		StartNetwork

		#
		# Now we need to find the isl directory on the RIS server.
		# We'll call GetRisInfo which will give us information
		# about which area the client is registered for on the
		# server.
		#
		RISPATH=`rsh $SERVER $RCMD "/bin/pwd" 2>$NUL` ||
		{
			Error "Could not retrieve RIS area information \c"
			Error "on $SERVER."
			Error "Exiting procedure..."
			Exit 1
		}
		set -- `GetRisInfo || Exit 1`
		PRODPATH=$RISPATH/$1
		shift
		PRODUCTS=$*
		REMMNT=$PRODPATH/kit

		#
		# Mount up the remote area from the server.
		#
		MountFS $MNTFLAG $LOCMNT $SERVER $REMMNT || Exit 1
		;;
	*:* )
		#
		# Set MIF flag
		#
		MIF=1

		#
		# The user wants to take the software off of a CDROM
		# that is mounted remotely.
		#
		TLOAD=NFS
		MNTFLAG="-r"
		PRODUCTS="ALPHA/BASE ALPHA/UPDATE ALPHA/hUPDATE"

		#
		# Start any network processes necessary.
		#
		StartNetwork

		#
		# Let's parse out the ":" so we can determine the server
		# and the exported mount point from the command line
		# argument.

		#
		set -- `Parse : $K`
		SERVER=$1
		REMMNT=$2

		#
		# Mount up the remote area from the server.
		#
		MountFS $MNTFLAG $LOCMNT $SERVER $REMMNT || Exit 1

		#
		# Just set NLOAD and PRODPATH now, so updeng knows where
		# to look for the update software, and this part is done.
		#
		NLOAD=$LOCMNT
		PRODPATH=$LOCMNT
		;;
	* )
		CMDINFO="$CMDINFO $K"
		;;
	esac
	done

	#
	# If MIF is NULL, we didn't get our mount point information and
	# don't know where to pull the software from.  Echo out an error
	# message, a usage message, and exit.
	#
	[ "$MIF" ] ||
	{
		Error "No media device or mount point on command line."
		Error "Don't know where to install new software from."
		Usage
		Exit 1
	}

	BakeMagicCookie

	if [ -f $LOCMNT/$UPDENG ]
	then
		$LOCMNT/$LOGTOOL $LOGFILE $SH $LOCMNT/$UPDENG $CMDINFO
		E_STAT=$?
	else
		Error "Cannot locate update information on $LOCMNT."
		Exit 1
	fi

	Cleanup
	[ "$E_STAT" = 0 ] && reboot
	exit "$E_STAT"
}


: MountFS -
#
#	Given:	$1 - mount command flags
#		$2 - local mount point
#		$3 - mount device
#		$4 - remote mount point
#
#	Does:	Attempts to mount a file system given the above infor-
#		mation.
#
#	Return: 0 - complete success
#		1 - any failure
#
MountFS()
{(
	MNTFLG=$1
	MNTPNT=$2
	case $TLOAD in
	CDROM )
		MNTDEV=$3
		;;
	* )
		MNTDEV=$3:$4
		;;
	esac

	#
	# See if the local mount point already exists.
	#
	if [ -d $MNTPNT ]
	then
		#
		# The local mount point already exists, so let's see
		# if there's already something mounted on it.
		#
		set -- `mount | grep $MNTPNT` &&
		{
			#
			# Yup, there's something mounted on it, so let's
			# see if we can figure out what's there.
			#
			case $1 in
			$MNTDEV )
				#
				# The device we want mounted is already
				# mounted.  We don't have to do anything
				# else, so we'll just leave now.
				# 
				return 0
				;;
			"" )
				#
				# There's nothing mounted on the mount
				# point.  The mount point was probably
				# left around somehow by a previous attempt
				# at installation.  We don't have to mkdir
				# the mount point, so we'll just continue
				# through and go directly to the mount
				# command.
				#
				;;
			* )
				#
				# Ooops.  Somethings mounted on our mount
				# point, and it's not what we expected.
				# We'll inform the user and let them try
				# the script again when they've got it
				# straightened out.
				#
				Error "Update installation mount point \c"
				Error "already mounted:"
				Error "`mount | grep $MNTPNT`"
				Error "\nPlease unmount $MNTPNT manually."
				return 1
				;;
			esac
		}
	else
		#
		# No mount point, so we'll make one.
		#
		mkdir $MNTPNT || return 1
	fi

	#
	# We'll attempt to mount the device on the mount point.
	#
	mount $MNTFLG $MNTDEV $MNTPNT ||
	{
		Error "Cannot mount $MNTDEV on $MNTPNT."
		Error "Check with the system manager of your host server."
		return 1
	}

	return 0
)}


: SearchRisdb -
#
#	Given:	$1 - Name of a client
#
#	Does:	Searches a remote RIS server for the clients database
#		entry.
#
#	Return: 0 - success
#		1 - failure
#
SearchRisdb()
{(
	K=`rsh $SERVER $RCMD grep "'^$1:'" clients/risdb 2>$NUL`
	[ "$K" ] &&
	{
		echo "$K"
		return 0
	}
	return 1
)}


: SetGlobals -
#
#	Given:	Nothing
#
#	Does:	Sets some global variables.
#
#	Return: Nothing
#
SetGlobals()
{
	PATH=/sbin:/usr/lbin:/usr/sbin:/usr/bin:/bin:/etc:.
	export PATH

	ISL_PATH=/updmnt/isl:/updmnt/hisl
	LOCMNT=/updmnt
	LOGFILE=/var/adm/smlogs/update.log
	LOGTOOL=isl/log
	NUL=/dev/null
	PROG=$0
	RCMD="-l ris -n"
	SH=/sbin/sh
	UPDENG=isl/updeng
	UPDINFO=/tmp/updinfo
}


: StartNetwork -
#
#	Given:	Nothing
#
#	Does:	Starts network and other items needed to RIS install
#
#		inet	- network
#		route	- route daemon
#		gateway	- gateway daemon
#		nis	- nis
#		named	- bind
#
#	Return:	Nothing
#
StartNetwork()
{
	for K in inet route gateway nis named
	do
		/sbin/init.d/$K start 1>$NUL ||
		{
			Error "Error starting $K." 
			Error "Cannot continue with update installation."
			Exit 1
		}
	done
}


: UnmountFS -
#
#	Given:	$1 - Name of a file system to unmount
#
#	Does:	Attempts to unmount the file system
#
#	Return: 0 - success
#		1 - failure
#
UnmountFS()
{(
	#
	# Use mount command to see if the file system is actually
	# mounted.
	#
	[ "$1" ] &&
	{
		mount | grep -q $1 &&
		{
			umount $1 ||
			{
				Error "Could not unmount:"
				Error "`mount | grep $1`"
				Error "Please unmount $1 manually."
				return 1
			}
			[ "$1" = "/updmnt" ] && rm -rf /updmnt
		}
	}
	return 0
)}


: Usage -
#
#	Given:	Nothing
#
#	Does:	Print out the usage message for this program.
#
#	Return: Nothing
#
Usage()
{
	#
	# Hmmmmm...well, the user used some combination of
	# command line argument that just doesn't make much
	# sense.  We'll type out a message that describes
	# how to use this script.
	#
	Error "Usage:"
	Error "$PROG <device>\t\t\t# From local device"
	Error "$PROG <local_mount_point>\t\t# From local mount_point"
	Error "$PROG <server:>\t\t\t# From RIS server"
	Error "$PROG <server:exported_mount_point>\c"
	Error " # From NFS remote file system"
}


[ "$CHECK_SYNTAX" ] || Main "$@"
