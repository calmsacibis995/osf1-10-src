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
# @(#)$RCSfile: sifsync.sh,v $ $Revision: 1.1.2.10 $ (DEC) $Date: 1993/12/21 21:42:05 $
# 
##########################################################################
# sifsync.sh - System Inventory File SYNCronization
#
#	Finds orphans and customized system files for update
#	installations, using ritkit tools.
#
##########################################################################

: DetectFileTypeErrors -
#
#	Given:	Nothing
#
#	Does:	Uses udetect to search for files, directories, etc that
#		no longer have the same type as is shipped on the kit.
#		Without this, tclear will complain and the system will
#		be left in an unusable state.
#
#	Return: Nothing
#
DetectFileTypeErrors()
{
	> /tmp/typerrs

	for K in $SSLST
	do
		( cd /; $UDETECT -t <${PSMDB}/${K}.inv  >> /tmp/typerrs )
	done

	[ -s /tmp/typerrs ] &&
	{
		echo "
----------------------------------------------------------------------
The following directories on this system conflict with assigned file
types originally shipped in the DEC OSF/1 operating system.  This can
be caused, for example, if a symbolic link is replaced with a real
directory or vice versa.

These conflicts must be resolved before an update installation can be
performed on this system.   Additional file status information can be
found in subset inventory files located in the /usr/.smdb. directory.

For later review, this message is also logged in

	/var/adm/smlogs/update.log

The update procedure will exit and return the system to its original
state.\n" > /tmp/siferrmsg

		cat /tmp/typerrs >> /tmp/siferrmsg
		echo "
----------------------------------------------------------------------" \
		>> /tmp/siferrmsg

		return 1
	}

	return 0
}


: DisplayFTerrs -
#
#	Given:	Nothing
#
#	Does:	Displays file type errors detected by udetect -t
#
#	Return: Nothing
#
DisplayFTerrs()
{
	while :
	do
		$VCMD /tmp/siferrmsg
		echo "Press <RETURN> to review message again."
		echo "Enter any character and press <RETURN>, \c"
		echo "to exit: \c"
		read ans
		ans=`echo $ans`
		case "$ans" in
		"" )		;;
		*  )	break	;;
		esac
	done
}


: Exit -
#
#	Given:	$1 - An exit status
#
#	Does:	Generic exit handler
#
#	Return: Nothing
#
Exit()
{
	ESTAT=$1
	Ticker off
	rm -rf $TMPDIR /tmp/typerrs /tmp/siferrmsg
	exit $ESTAT
}


: SetGlobalConstants -
#
#	Given:	Nothing
#
#	Does:	Sets various constants that are used globally through-
#		out this program.
#
#	Return: Nothing
#
SetGlobalConstants()
{
	#
	# if this is not an installation, make sure that the shell
	# library area is in our path so that Error and other libraries
	# can be read in.
	#
	[ "$UPDFLAG" ] ||
	{
		echo $PATH | grep -q /usr/share/lib/shell ||
		{
			PATH=${PATH}:/usr/share/lib/shell
			export PATH
		}
	}

	#
	# Read in shell library routines.
	#
	. Error
	. Ticker
	. Wait

	#
	# Read in variables set up by installupdate and updeng.
	# ORPHAN_FILE and CUSTOM_FILE are defined by this.
	#
	if [ -f /var/adm/update/updinfo ]
	then
		. /var/adm/update/updinfo
	else
		Error "Cannot locate /var/adm/update/updinfo."
		Exit 1
	fi

	CSMDB=/usr/.smdb.
	TMPDIR=/tmp/updsync
	TSMDB=$TMPDIR
	TMPFILE=$TMPDIR/xxx
	DONELIST=$TMPDIR/donelist
	UDELTA=udelta
	UDETECT=udetect
	USYNC=usync
	UPDMORE=$LOCMNT/isl/updmore

	[ -d $TMPDIR ] || mkdir -p $TMPDIR || Exit 1

	#
	# Set message viewer
	#
	if [ -f $UPDMORE ]
	then
		VCMD=$UPDMORE
	else
		VCMD=cat
	fi

	#
	# set trap
	#
	trap 'Exit 1' 1 2 3 15

	> $ORPHAN_FILE
	> $CUSTOM_FILE
	> $DONELIST
}


: SetCommandArgs -
#
#	Given:	A list of command arguments.
#
#	Does:	Checks to see if the user is root.  If not, an error
#		message is displayed and this program is terminated.
#
#		Next, there must be at least two command arguments.
#		The first, is the path of the subset inventory files,
#		(PSMDB), and the second, is the list of subsets to be
#		checked (SSLST).  These variables should be global to
#		the program.
#
#	Return: Nothing
#
SetCommandArgs()
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
	# Make sure there are at least 2 command arguments.
	#
	[ "$#" -gt 1 ] ||
	{
		Error "Usage: $0 <subset_inventory_path> <subsets>"
		exit 1
	}

	#
	# Now set global variables
	#
	PSMDB=$1/instctrl
	shift
	SSLST=$*
}


SetTrap()
{
	trap 'Exit 1' 1 2 3 15
}


: Udelta -
#
#	Given: 	$1 - A usync'd subset inventory file from the current smdb
#		     directory on the system.
#
#		$2 - A subset inventory file to compare $1 against.
#
#	Does:	Compares inventory entries between $1 and $2, and awks out
#		the file name of an entry that is no longer being shipped.
#
#		Configured file names that are shipped with the .new..
#		prefix will show up as deltas.  These need to be weeded
#		out to get a true representation of those files that are
#		truly no longer being shipped.
#
#		NOTE: Log files that are not shipped, but created by 
#		various programs and processes, will appear in the list
#		also.
#
#	Return: Nothing
#
Udelta()
{(
	cd /

	> $TMPFILE

	for K in `$UDELTA $1 $2 | awk '{print $10}'`
	do
		d=`dirname $K`
		b=`basename $K`
		fgrep -s "$d/.new..$b" $2 ||
		{
			fgrep -s "$d/.upd..$b" $2 ||
			{
				[ -f $K ] &&
					echo $K >> $TMPFILE
			}
		}
	done

	[ -s $TMPFILE ] &&
	{
		SS=`basename $2`
		SS=`expr "$SS" : '\(.*\)\.'`
		echo "
============================================
= Files no longer shipped in $SS
============================================" >> $ORPHAN_FILE
		cat $TMPFILE | sort -r >> $ORPHAN_FILE
	}
	rm -f $TMPFILE
)}


: Udetect -
#
#	Given: 	$1 - A usync'd subset inventory file from the current smdb
#		     directory on the system.
#
#	Does:	Detects system files that have been modified since they
#		were originally shipped in the product.
#
#	Return: Nothing.
#
Udetect()
{(
	> $TMPFILE

	cd /
	for K in `$UDETECT < $1 | awk '{print $10}'`
	do
		fgrep -s "$K" $ORPHAN_FILE ||
		{
			d=`dirname $K`
			b=`basename $K`
			fgrep -s "$d/.new..$b" $2 || echo $K >> $TMPFILE
		}
	done

	[ -s $TMPFILE ] &&
	{
		SS=`basename $1`
		SS=`expr "$SS" : '\(.*\)\.'`
		echo "
============================================
= Unprotected Customized $SS Files
============================================" >> $CUSTOM_FILE
		cat $TMPFILE | sort -r >> $CUSTOM_FILE
	}
	rm -f $TMPFILE
)}


: Usync -
#
#	Given:	$1 - A subset name. (OSFBASE140, etc)
#
#	Does:	Performs an inventory syncronization for each subset
#		installed on the system with the same base subset name.
#
#	Return:	0 - Got something.  Success.
#		1 - No subset with SSNAM currently installed
#
Usync()
{(
	# drop off the version number of the subset
	SSNAM=`expr $1 : '\(.*\)...'`

	#
	# Because we are using only part of the subset name,
	# subsets such as OSFBINCOM140 and OSFBINCOM141 will
	# have the same data show up twice.  So for each SS,
	# we'll grep for the name in the DONELIST file.  If
	# a subset with the same base name has already been
	# done, we'll skip the present one.
	#
	fgrep -s "$SSNAM" $DONELIST && return 1

	for DIR in $PSMDB $CSMDB
	do
		# Get a list of all subsets with the same name as
		# SSNAM.  Because we are wildcarding on only part
		# of the subset name, it's possible that subsets
		# such as OSFBINCOM, will get returned with a search
		# for OSFBIN subsets.  We'll weed out the ones that
		# don't belong in the list.
		cd $DIR
		RSSLIS=
		TSSLIS=`ls ${SSNAM}*.inv 2>/dev/null` || return 1
		for K in $TSSLIS
		do
			J=`expr "$K" : '\(.*\).......'`
			case $J in
			$SSNAM) RSSLIS="$RSSLIS $K" ;;
			esac
		done

		#
		# If this is the incoming product, we need to write
		# to /tmp because it might be a CDROM.
		#
		[ "$DIR" = "$PSMDB" ] && cd $TSMDB

		[ "$RSSLIS" ] &&
		{
			set -- $RSSLIS
			cp $DIR/$1 ${SSNAM}.sync
			shift
			while [ $# -gt 0 ]
			do
				mv ${SSNAM}.sync ${SSNAM}.psync
				$USYNC ${SSNAM}.psync $DIR/$1 > ${SSNAM}.sync
				shift
			done
			rm -f ${SSNAM}.psync 
		}
	done

	if [ -f $CSMDB/${SSNAM}.sync ]
	then
		echo $SSNAM >> $DONELIST
		echo "${SSNAM}.sync"
		return 0
	else
		return 1
	fi
)}


: Main -
#
#	Given:	At set of command arguments.
#
#	Does:	Runs the rest of the program.
#
#	Return: Nothing.
#
Main()
{
	SetCommandArgs "$@"
	SetGlobalConstants

	Ticker on

	DetectFileTypeErrors ||
	{
		Ticker off
		DisplayFTerrs
		Exit 1
	}

	for K in $SSLST
	do
		SYNCSS=`Usync $K` &&
		{
			Udelta $CSMDB/$SYNCSS $TSMDB/$SYNCSS 
			Udetect $CSMDB/$SYNCSS $TSMDB/$SYNCSS
		}
		[ -f $TSMDB/$SYNCSS ] && rm -f $TSMDB/$SYNCSS
		[ -f $CSMDB/$SYNCSS ] && rm -f $CSMDB/$SYNCSS
	done

	Exit 0
}


[ "$CHECK_SYNTAX" ] || Main "$@"
