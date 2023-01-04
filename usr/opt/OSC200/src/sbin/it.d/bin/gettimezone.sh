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
# @(#)$RCSfile: gettimezone.sh,v $ $Revision: 4.2.3.8 $ (DEC) $Date: 1992/11/25 11:31:27 $ 
#
############################################
# gettimezone - timezone information program
#
# %W%	(DEC OSF/1)	%G%
#
############################################

[ "$ISL_ADVFLAG" ] ||
{
	case `whoami` in
	root ) ;;
	* )
		echo "
You must have root privileges to change the system time zone."
		exit 1
		;;
	esac
}


trap 'Cleanup; exit' 0 1 2 3 15
set -h
ZIC=				# Zone Info Choice (default null)
SERVER=				# Name of installation server (default null)
NUL=/dev/null			# bit bucket
BMDF=/tmp/zinfo			# Base Menu Data File
BZID=/etc/zoneinfo		# Base Zone Info Directory
PWD=`/bin/pwd`			# Present Working Directory
RCMGR=/usr/sbin/rcmgr		# rcmgr program

TZ_NOCHANGE_MSG="System time zone not modified."

TZ_NOSELECT_MSG="
You did not choose a time zone from the menu.  The default
time zone in the absence of a selection is GMT0 (Greenwich Mean Time)."

HEAD_TZ_BANNER="*** TIME ZONE SPECIFICATION ***"

MAIN_TZ_BANNER="***** Main Timezone Menu *****"

SUB_TZ_BANNER='\***** $ZIC Timezone Menu \*****'

MNUOPT_QUERY="Select the number above that best describes your location:"

MNUOPT_MSG='You selected $ZIC as your time zone.'

CONFIRM_MSG="Is this correct? (y/n) [y]: \c"

MNU_ERR_MSG="
*** Invalid menu choice ***
*** Make another selection, or press <RETURN> to review the menu again. ***" 

DMZ="--------------------------------------------------------------------------"


: Cleanup - clean up after the program
#
#	Given:	Nothing
#
#	Does :	Cleans up temporary menu and data files created during
#		execution of this script.
#
#	Returns:Nothing
#
Cleanup()
{
	rm -f /tmp/zinfo*
}


: CollectGMT - Special case for collecting GMT time.
#
#	Given:	Nothing
#
#	Does :	Collects GMT data and builds a data file for it.  This
#		is a special case for GMT data because the files are
#		located in the top level directory $BZID, but
#		we want to display them to the user as if they came from
#		a lower level directory such as Australia or US.  It
#		just makes the display prettier.
#
#	Returns:Nothing
#
CollectGMT()
{
	IN=0
	>/tmp/zinfo.GMT
	for K in `cd $BZID; ls GMT* | sort +0.3n`
	do
		IN=`expr $IN + 1`
		if [ $IN -lt 10 ]
		then 
			echo " ${IN}) $K" >> /tmp/zinfo.GMT
		else
			echo "${IN}) $K" >> /tmp/zinfo.GMT
		fi
	done
}


: Collectinfo - Get valid time zones from $BZID directory
#
#	given:	directory to search in ($1)
#		filename to deposit zoneinfo data ($2)
#
#	does :	cd's to the search path and collects all valid filenames
#		and directories that may be time zone options for
#		$BZID.  "sources" is not a valid option, and 
#		excluded from the list.  Other future non-valid options
#		should be added to the case statement to exclude them
#		also.
#
#	returns:Nothing
#
#	Variables:
#		IN	- counts 1 for each valid time zone option
#		IFI	- InFIle, name of file that data will be written to
#		OFI	- OutFIle, 
#		K	- 'for' loop variable
#
Collectinfo()
{
	IN=0				# item number
	[ -s $2 ] && return		# check to see if data exists
	[ "$2" = "/tmp/zinfo.GMT" ] &&	# special case for GMT data
	{
		CollectGMT
		return
	}
	FI=$2
	cd $1
	>$FI
	for K in *
	do
		if [ -f $K ] || [ -d $K ]
		then
			case $K in
			sources | localtime ) continue ;;
			GMT ) ;;
			GMT* ) continue
			esac
			
			IN=`expr $IN + 1`
			if [ $IN -lt 10 ]
			then
				echo " ${IN}) $K" >> $FI
			else
				echo "${IN}) $K" >> $FI
			fi
		fi
	done
}


: ChooseTZ - Display time zone menu and get response from user
#
#	Given:	Nothing
#
#	Does :	Displays appropriate header banner, and then a numbered
#		menu of time zone options for selection by the user.
#		Also gets the response from the user and then searches
#		the database file (from Collectinfo) for the correct
#		time zone.
#
#	Returns:Nothing
#
#	Variables:
#		ZIC	- Zone Info Choice.  Used for user interface
#			  header banner for menu.
#		MDF	- Menu Data File.  Contains menu items.
#		MNUOPT	- The time zone selected by the user.
#
ChooseTZ()
{
    while :
    do
	[ -s $MDF.mnu ] || MakeMenu
	cat $MDF.mnu
	echo "$MNUOPT_QUERY \c"
	ANS=`Read`
	case $ANS in
	0 )
		MNUOPT=None
		break
		;;
	"" )
		continue
	esac

	while :
	do
		MNUOPT=`cat $MDF | while read LINE
		do
			set -- \`Parse ')' $LINE\`
			[ "$ANS" = $1 ] &&
			{
				echo $2
				break
			}
		done`

		if [ "$MNUOPT" ]
		then 
			break 2
		else
			echo "$MNU_ERR_MSG\n"
			echo "$MNUOPT_QUERY \c"
			ANS=`Read`
			case $ANS in
			0 )
				MNUOPT=None
				break 2
				;;
			"" )
				continue 2
			esac
		fi
	done
    done
}


: DecodeChoice - Perform various actions on menu selection
#
#	Given:	Nothing
#
#	Does :	If menu option is "None of the above"; determines if we
#		are at the top level $BZID directory or not.  If
#		yes, then exit.  If no, then reset variables to take us
#		up one level, and display menu again.
#
#		Next, set the ZIC variable that is used in header banners.
#		Then, determine if the menu choice is a subdirectory or
#		not.  If yes, reset variables to take us down one level
#		to the appropriate directory.  If no, confirm the choice
#		and act accordingly.
#
#	Returns:
#		0 - Good selection
#		1 - Go Back to menu
#
DecodeChoice()
{
	case $MNUOPT in
	None )
		if [ "$ZID" = "$BZID" ]
		then
			while :
			do
				echo "$TZ_NOSELECT_MSG"
				echo "Is this ok? (y/n) [y]: \c"
				ANS=`Read`
				case $ANS in
				[Yy]* | "" )
					ZIC="GMT0"
					return 0
					;;
				[Nn]* ) return 1 ;;
				esac
			done
		else
			ZIC=`expr "$ZIC" : '\(.*\)[/]'`
			ZID=`expr "$ZID" : '\(.*\)[/]'`
			MDF=`expr "$MDF" : '\(.*\)[.]'`
			return 1
		fi
	esac

	if [ "$ZIC" ]
	then
		ZIC="$ZIC/$MNUOPT"
	else
		ZIC=$MNUOPT
	fi

	if [ -d $BZID/$ZIC ] || [ "$MNUOPT" = "GMT" ]
	then
		ZID="$BZID/$ZIC"
		MDF="$MDF.$MNUOPT"
		return 1
	else
		while :
		do
			echo
			eval echo $MNUOPT_MSG
			echo $CONFIRM_MSG
			ANS=`Read`
			case $ANS in
			[Yy]* | "" )
				case $ZIC in
				GMT* ) ZIC=`expr "$ZIC" : '.*[/]\(.*\)'`
				esac
				return 0
				;;
			[Nn]* )
				ZIC=`expr "$ZIC" : '\(.*\)[/]'`
				return 1
			esac
		done
	fi
}


: GetNetTZ
#
#	Given:	Nothing.
#
#	Does:	Checks to see if this is a RIS install (INST_SERVER)
#		and then attempts to get the TZ info from the server.
#
#	Return:
#		0 - success
#		1 - failure
#
GetNetTZ()
{
	SERVER=`$RCMGR get INST_SERVER`
	[ "$SERVER" ] &&
	{
		ZIC=`rsh $SERVER -l ris "ls -l /etc/zoneinfo/localtime" 2>$NUL`
		[ "$ZIC" ] &&
		{
			ZIC=`expr "$ZIC" : '.*-> \(.*\)'` &&
			{
				ZIC=`echo $ZIC | sed 's/\.//g' | sed 's/\// /g'`
				ZIC=`echo $ZIC | sed 's/ /\//g'`
				[ -f $BZID/$ZIC ] && return 0
			}
		}
	}
	return 1
}


: Main - Program driver
#
#	Given:	Nothing
#
#	Does :	Drives the rest of the program.
#
#	Returns:Nothing
#
Main()
{
	GetNetTZ
	[ $? = 1 ] &&
	{
		ZIC=
		ZID=$BZID
		MDF=$BMDF
		echo "\n\n$HEAD_TZ_BANNER"
		while :
		do
			Collectinfo $ZID $MDF
			[ "$GMTFLG" ] && SortGMT
			ChooseTZ
			DecodeChoice 
			[ $? = 0 ] && break
		done
	}
	SetTZ
	exit 0
}


: MakeMenu - Create menu to be displayed on screen
#
#	Given:	Nothing
#
#	Does :	Creates a menu with appropriate banners using the data
#		contained within a /tmp/zinfo file.
#
#	Returns:Nothing
#
MakeMenu()
{
	if [ "$ZIC" ]
	then
		echo "" > $MDF.mnu
		eval echo $SUB_TZ_BANNER >> $MDF.mnu
	else
		echo "\n$MAIN_TZ_BANNER" > $MDF.mnu
	fi

	echo "\n$DMZ\n" >> $MDF.mnu
	cat $MDF | pr -t -o4 -l1 -4 >> $MDF.mnu
	echo "\n     0) None of the above" >> $MDF.mnu
	echo "\n$DMZ\n" >> $MDF.mnu
}


: Parse - Pull strings apart
#
#	Given:	IFS demarcation
#		String
#
#	Does :	Separates string via IFS demarcation
#
#	Returns:String separated by spaces, minus IFS demarcator
#
Parse()
{ (
	IFS=$1;shift
	echo $*
) }


: Read - Reads standard input and echos it back
#
#	Given:	Nothing
#
#	Does :	Reads from standard input and echos it out of the
#		function.  This is used to avoid spaces in the input
#		string that might otherwise screw up our tests and 
#		case statements on the appropriate variable that is
#		assigned the output of this function.
#
#	Returns:Nothing
#
Read()
{(
	read ANS
	echo $ANS
)}


: SetTZ - Set system time zone
#
#	Given:	Nothing
#
#	Does :	Sets up the system time zone by copying the proper time
#		zone file to '/usr/share/zoneinfo/localtime'
#
#	Returns:Nothing
#
SetTZ()
{
	cd $BZID
	rm -f localtime
	ln -s ./$ZIC localtime
	echo :$ZIC > /etc/svid3_tz
}


[ $CHECK_SYNTAX ] || Main
