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
# @(#)$RCSfile: lpblocker.sh,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/12/21 21:41:41 $
# 

: CreateBlockingFile -
#
#	Given:	Nothing
#
#	Does:	Checks to see if a fatal error message file has been
#		created already.  If not, it creates one with a message.
#
#	Return: Nothing
#
CreateBlockingFile()
{
	[ -s $BLOCKMSG ] ||
	{
		echo "
  This system can not be updated with the following layered products
  installed  on it.  Please remove these  products from  your system
  before attempting an update installation:
	" >> $BLOCKMSG
	}
}


: CreateWarningFile -
#
#	Given:	Nothing
#
#	Does:	Checks to see if a warning error message file has been
#		created already.  If not, it creates one with a message.
#
#	Return: Nothing
#
CreateWarningFile()
{
	[ -s $WARNMSG ] ||
	{
		echo "
  The following layered products may require re-installation after
  the update installation has completed:
	" >> $WARNMSG
	}
}


: Main -
#
#	Main function of program
#
Main()
{
	#
	# Set Variables
	#
	SetVariables $*

	#
	# Analyze layered products on the system.
	#
	ProcessProducts
}


: ProcessProducts -
#
#	Given:	Nothing
#
#	Does:	Creates a list of products that will either prevent the
#		update from occuring, or that the user should be warned
#		about in the event that they may have to reinstall the
#		layered products to get them to work after the update.
#
#	Return:	Nothing
#
ProcessProducts()
{
	#
	# Change directory to the subset information directory and pick
	# up all the subsets that are installed.
	#
	cd /usr/.smdb.
	for K in *.lk
	do
		#
		# Get the Product Code (CODE) and Subset name (SSNAME)
		#
		CODE=`expr "$K" : '\(...\)'`
		SSNAME=`echo "$K" | sed 's/.lk//'`

		#
		# If the ctrl file exists, read the NAME string 
		#
		if [ -f ${SSNAME}.ctrl ]
		then
			eval `fgrep NAME ${SSNAME}.ctrl`
			#
			# Some products tack on the Subset Name on the
			# end of the NAME string.  We want to take it
			# off, otherwise multiple entries will appear
			# on the message file.
			#
			PNAME=`echo "$NAME" | sed 's/'$SSNAME'//'`
		else
			continue
		fi

		#
		# If the CODE is in updpblock.dat, put the entry in
		# the message file.
		#
		fgrep -s "$CODE" $BLOCKDAT &&
		{
			CreateBlockingFile
			fgrep -s "$PNAME" $BLOCKMSG ||
				echo "\t$PNAME" >> $BLOCKMSG
			continue
		}

		#
		# If the CODE is NOT in updpnowarn.dat, put the entry in
		# the message file.
		#
		fgrep -s "$CODE" $WARNDAT ||
		{
			CreateWarningFile
			fgrep -s "$PNAME" $WARNMSG ||
				echo "\t$PNAME" >> $WARNMSG
		}
	done
}


: SetVariables -
#
#	Given:	Command list
#
#	Does:	Sets up various variables, temp files, and environment
#		conditions for the execution of this program.
#
#	Return: Nothing
#
SetVariables()
{
	#
	# Step through the command line arguments.
	#
	while [ $# -gt 0 ]
	do
		case "$1" in
		-m )
			MNTPNT=$2	
			shift; shift
			;;
		-w )
			WARNMSG=$2
			shift; shift
			;;
		-b )
			BLOCKMSG=$2
			shift; shift
			;;
		esac
	done

	#
	# Set stat equal to null, and make sure the shell directory is
	# part of PATH.
	#
	STAT=
	PATH=${PATH}:/usr/share/lib/shell
	export PATH
	. Error

	#
	# All three variables need to be defined, or we can't do what
	# we want to do.
	#
	if [ "$MNTPNT" -a "$WARNMSG" -a "$BLOCKMSG" ]
	then
		> $WARNMSG
		> $BLOCKMSG
	else
		Error "Usage: lpblocker -m <isl mount point>"
		Error "                 -w <filename of warning message>"
		Error "                 -b <filename of blocking message>"
		exit 1
	fi

	#
	# Check to make sure both data files are available.  We don't
	# exit right away, so that we can catch both files missing if
	# they are.
	#
	[ -f $MNTPNT/isl/updpblock.dat ] ||
	{
		Error "Cannot locate $MNTPNT/isl/updpblock.dat"
		STAT=1
	}

	[ -f $MNTPNT/isl/updpnowarn.dat ] ||
	{
		Error "Cannot locate $MNTPNT/isl/updpnowarn.dat"
		STAT=1
	}

	#
	# Now we know if both files are missing or not.  If yes, exit.
	#
	if [ "$STAT" ]
	then
		exit $STAT
	else
		BLOCKDAT=$MNTPNT/isl/updpblock.dat
		WARNDAT=$MNTPNT/isl/updpnowarn.dat
	fi
}


[ "$CHECK_SYNTAX" ] || Main $*
