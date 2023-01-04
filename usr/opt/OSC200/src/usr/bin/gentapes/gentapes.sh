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
# @(#)$RCSfile: gentapes.sh,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/01/29 17:45:19 $ 
# 
#    gentapes.sh - sh script for generic remote tape production
#
GENTAPES_USAGE="Usage: gentapes [-verify] [-write] [remhost:[:]]kitcode device
	-check_only
		Check that each subset that needs to be written to a
		kit exists in on of the kit directories.  Implies
		-write.  In other words, this option inhibits anything
		being written out to tape.

	-verify	verify only

	-start x
		Create/Verify a kit starting at volume x.  A specific volume
		of a multi-volume set can be built/verified by starting
		with the first needed volume and then skipping over later
		volumes that you do not want.

	-write	write only

	remhost	TCP or DECnet hostname where kit files reside
	device	magtape output device

	DECnet remhost (not yet implemented) should contain two colons, network
	access information can be included, example:
		remhost/username/password::ULT-4.0-V-BW
	"
#
#   TCP hosts should be set up to allow rsh access without a password.
#   hostname.username forms are acceptable.
#
#   If the environment variable GENTAPES_NO_EXECUTE is defined, then
#   this file is not executed, but is loadable as a set of routines
#   suitable for calling from another script.
#
#   The mainline routine is Gentapes_Mainline.
#
#   Kitcap entries are coded as follows:
#	first word (ends in RA, TK or MT) - product code
#	word starts with '/'	- directory path
#	other words are subsets
#
#   Environment variables that affect this program if they are non-null:
#	GENTAPE_MERGEDIR	used as the local merge directory.
#	GENTAPE_WORKDIR		used as the local instctrl work dir.
#	GENTAPE_DEBUG		enables printing of debug msgs.
#
#	KITCAP_LOCATION		used as the remote kitcap file path.  No
#				hostname, this comes from cmd line.
#	KITCAP_KITROOT		causes %%KITROOT%% tokens in the remote
#				kitcap to be replaced with its value
#				in the local copy of the kitcap file.
#
# Functions in this module (starred entries affect caller's symbols):
#
#    GT_Get_Arguments(args)
#	Extract values for WRITE, VFY, HOST, KITCODE, DEV and DEVTYPE from
#	the command line and return assignments for them.
#
#    GT_Change_Volume (kitdev, access, process, label)
#	Prompt the user to perform the desired PROCESS on the
#	KITDEV with the indicated LABEL, change the name of
#	the KITDEV, skipt the device or exit the script.
#
#    *GT_Create_Kit(volume, subset, subset. . .)
#	Create a kit from information in the MERGEDIR and kit
#	data from the component kit directories.
#
#    *GT_Verify_Tape(volume, subset, subset. . .)
#	Verify that a tape created from information in the MERGEDIR
#	and kit	data from the component kit directories matches
#	the actual data.  Uses checksums to accomplish this wherever
#	possible.  Verifies that the contents of VERIFYLIST match
#	the entries on the current tape volume.
#
#####
#
# Modification History
# ~~~~~~~~~~~~~~~~~~~~
# 13	May-91, ech
#	Ported to OSF/1
#
# 12	Oct-90, Greg Tarsa
#	Removed restriction that user had to be ROOT to build kits.
#
# 11	Jul-90, Greg Tarsa
#	Incorporated changes supplied by Shuichi Shogaki for the support
#	of kit building to the TZ30 tape drive.
#
# 10	Feb-90, Greg Tarsa
#	Changed code so that there is now no default for a volume change
#	prompt.  Seems that ORDS people would sometimes get a stray CR
#	in their input stream and after 15 or so hours, it would get
#	processed as the default action of a volume change and blow away
#	their volume two.
#
# 09	Jan-90, Greg Tarsa
#	Fixed bug that occurs when using "file /dev/rmt0h | grep TK50"
#	in a pipe.  Problem occurred when grep terminated the pipe before
#	"file" had a chance to completely close the tape.  The script continued
#	and later mt commands failed.
#
#	Fixed checks to WRITE/VFY that caused verify-only passes to require
#	writing the tape.
#	Added code to check integrity of the INSTCTRL file in the wake of the
#	V4.0FT1 tar bug.  Check only works when the kit is being built and
#	verified in the same pass.
#
# 08	Nov-89, Greg Tarsa
#	Added /dev/nullmt code.
#	Changed PATH so that /usr/ucb is before /bin (to get better grep).
#	Changed test to determine whether a device is a magtape to check
#	the device name rather than assume it is an MT if it is not /dev/null.
#	Added support for volume skipping.
#	Added calls to _Utility version of Check Volume.
#	Tapes skip back 2 files if data was written in a network failure
#	case.
#	Added code to calculate and display the volume size when
#	verifying a tape.
#	Added code to prompt for DECnet access control strings.
#	Cleaned up retry code, added code to keep from retrying local errors.
#	Fixed retry checksumming.
#	Added retry to checksumming.
#	Added retry to tape building, in case network is flaky.
#	Added check to see that user is ROOT.
#
# 07	Oct-89, Greg Tarsa
#	Fixed ibs/obs/bs problems in dd.  bs= is the appropriate
#	option for our case, according to the man pages.
#	Changed method used for BOOT image comparisons to use sum and not
#	cmp.  It is more reliable, given the uncertain state of cmp.
#	Added definition for DISPHOST so that when access information is
#	specified for the HOST, it is not scattered throughout all the
#	script output.
#	Help and errors are now piped through more(1).
#	Changed code to allow any alphanumeric as part of the 1st 3
#	characters of the kitcode.
#	
# 06	Aug-89, Greg Tarsa
#	Added explicit set of umask.
#	Changed Interpret Kitcode function to use only two arguments.
#	Fixed bug where script was going over the wire for INSTCTRL
#	instead of picking it up locally.
#	Added code to support getting the text name of the kit from the
#	kitcap file.
#	Fixed bug where -writeonly path through Create Kit code was
#	not requesting a volume change.
#	Changed Tape requests to Device requests.
#	Added -check_only flag.
#	Fixed unpacking problem.
#
# 05	Jul-89, Greg Tarsa
#	Added -help flag.
#	Added code to test that the first volume is available prior
#	to all the pre-processing.
#	Changed calls to Request Tape to specify access type.
#	Removed special TK50 processing, added more general DEVTYPE
#	processing.
#	Added special TK50 processing to ignore volume change info.
#	Simplified writing to stderr.
#	Changed into a convertible script/library.
#	Fixed some bugs.
#
# 04	Jun-89, Greg Tarsa
#	Merged in new capabilities from Jon Wallace (multiple
#	    directories in kitcap entries, building .ctrl files
#	    on the fly, etc.
#	Merged in applicable old GenTapes functions.
#	Added functions, particularly echo2.
#	Isolated all existence testing that must be done over a
#	remote link and replaced with method independent code.
#	Remote kit merges should now work properly.
#	Started implementation of DECnet.
#	Added better error handling to dd-copying code.
#
# 01	7-Jun-89, Greg Tarsa
#	Derived original code from code originally converted to sh5
#	by Jon Wallace.
#	Added comments; adjusted indentation.
#	Added environment variable processing.
#	Added root-token replacement code.
#
###################################################################
###################################################################
# Set Variables #
#################

#
# Only set these globals if we are executing as a script.
#
case "$GENTAPES_NO_EXECUTE" in
    "")
	PROG=`basename $0`
	echo "$PROG: $GENTAPES_SCCSID"
	PATH=/usr/ucb:/bin:/etc:/usr/hosts
	export PATH
esac

###################################################################
##################################################################
# Subroutines #
###############

set -h	# hash subroutine references

#
## START FUNCTIONS: 
#
# Load general utility routines (_U) and kitcap manipulation routines (_K).
#

SHELL_LIB=${SHELL_LIB:-/usr/share/lib/shell}

for library in GenUtil Kitcap
do
    #
    # Before loading, check to see if we have it already.
    #
    eval already_loaded=\$_${library}_included
    case $already_loaded in
	yes) continue ;;
    esac
    
    echo "Loading ${library}. . .\c"
    . ${SHELL_LIB}/${library}
    echo "done."
done

## END FUNCTIONS##

#####################################################################
#####################################################################
# gentapes functions
#################
#
# GT_Get_Arguments(args)
#    Extract values for WRITE, VFY, HOST, KITCODE, DEV, DEVTYPE and
#    VOL_START from the command line (and/or the user) and return
#    assignments for them.
#
#    See the GENTAPES_USAGE assignment at the head of this file for the
#    command line format.
#
GT_Get_Arguments()
{(
FUNC=GT_Get_Arguments
test $DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

WRITE=0 VFY=0 CHECK_ONLY=0 HOST= KITCODE= DEV= DEVTYPE= STAT= VOL_START=

while : true
do
    case "$1" in
	-verify|-v*)	# verify only.
	    VFY=1
	    shift
	    ;;

	-start|-s*)
	    # If, after removing all digits and whitespace we something left
	    # then the argument is not a volume number
	    if [ "`echo $2 | sed 's/[0-9 	]*//'`" ]
	    then
		(echo "?$PROG: Missing or invalid starting volume" \
			  "number specified:\n    $*\n"
		  echo "${GENTAPES_USAGE}") | more 1>&2
		return 1
	    else
		VOL_START="$2"
		shift
	    fi
	    shift
	    ;;

	# write only
	-write|-w*)
	    WRITE=1
	    shift
	    ;;

	# check only; implies write.
	-check_only|-c*)
	    CHECK_ONLY=1
	    shift
	    ;;

	# help flag
	-help|-h*)
	    echo "${GENTAPES_USAGE}" | more 1>&2
	    return 1
	    ;;

	# invalid flag
	-*)
	    (echo "$PROG.$FUNC: Invalid flag-- '$1'\n"
	     echo "${GENTAPES_USAGE}") | more 1>&2
	    return 1
	    ;;

	# 1st argument, end loop
	*)
	    break
	    ;;
    esac
done

#
# no changes from initial value,
# set both write and verify
#
case "$WRITE$VFY" in
    00)
	WRITE=1 VFY=1
	;;
esac

#
# -check_only implies -write and will not work
# properly if -verify is set, so coerce the flags
# accordingly.
#
if [ $CHECK_ONLY -ne 0 ]
then
	WRITE=1 VFY=0
fi

#
# Find remote host (if any) and then get Kitcode and Kittype
#
case $# in
    2)	
        #
	# save raw device parameter and
	# set our device type.
	#
        DEV=$2

	#
	# Device type is currently either TK for TK50, TZ for TZ30
	# or MT for MT9.  The T[KZ] drives are known as "square" magtapes
	# and the MT9 is a "round" magtape.
	#
	info=`file $DEV`
	if echo "$info" | grep -s "T[KZ]" > /dev/null || test "$DEV" = "/dev/null"
	then
	    DEVTYPE="TK"
	else
	     DEVTYPE="MT"
	fi

	#
	# allow /dev/nullm[t] to be specified as a /dev/null masquerading
	# as an MT9.  This is for testing.
	#
	case $DEV in
	    /dev/nullm|/dev/nullmt)
		DEV="/dev/null"
		;;
	esac
	test "$DEBUG" && echo 1>&2 "DEBUG: DEVTYPE=$DEVTYPE"

	#
	# Get the HOST and the REMAINDER from the
	# kitcode parameter
	#
	eval `_U_Get_Hostname "$1"` || return 1

	#
	# Get the KITCODE
	KITCODE="$REMAINDER"

	#
	# Check for proper DECnet access control.  If it is missing, prompt
	# for it now.
	#
	# Rewrites HOST, if necessary, with full DECnet specification.
	#
	# Note: defaults of $CLIENT_NAME and $CLIENT_PWD may need to
	# change with each field test.
	#
	eval `_U_Get_Dnet_Acc "$HOST" "$CLIENT_NAME" "$CLIENT_PWD"`
	;;

    *)  (echo "?Invalid number of arguments ($#): $*\n"
	 echo "${GENTAPES_USAGE}") | more 1>&2
	return 1
	;;
esac

echo WRITE='"'$WRITE'"' VFY='"'$VFY'"' CHECK_ONLY='"'$CHECK_ONLY'"' \
	HOST='"'$HOST'"' KITCODE='"'$KITCODE'"'\
	DEV='"'$DEV'"' DEVTYPE='"'$DEVTYPE'"'\
	VOL_START='"'$VOL_START'"'
return 0
)}
##### End GT_Get_Arguments
#
# GT_Change_Volume (kitdev, access, process, label)
#    Prompt the user to perform the desired PROCESS on the
#    KITDEV with the indicated LABEL, change the name of
#    the KITDEV, skipt the device or exit the script.
#
#    It returns an assignment for KITDEV and ANS if successful
#    and a failure status (with a message to stderr) otherwise.
#
#    ANS is one of $PROCESS, 'skip' or 'quit'.
#
#    device	the system name of the device (i.e., /dev/rmt0l)
#    access	one of "read" or "write".
#    process	the name of the process to be performed.  The abbreviation
#		of this will be the first letter and cannot be
#		'q', 's', or 'c'.
#    label	the volume label; for prompting purposes
#
#
GT_Change_Volume()
{(
FUNC=GT_Change_Volume NUMARGS=4
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

#
# Reject attempt if we do not have the right number of arguments
case "$#" in
    ${NUMARGS})	# right number of args, perform search
        KITDEV=$1
	ACCESS=$2
        PROCESS=$3
	LABEL="$4"
	;;

    *)		# wrong number of args, error
	echo 1>&2 "$FUNC($# args): Needs ${NUMARGS} arguments"
	return 1
	;;
esac

ABBREV=`expr $PROCESS : '\(.\)'`

echo 1>&2 "Please load ${LABEL} on ${KITDEV} (or another device)." \
          "\nWhen you are done, then"

while : true
do
    eval `_U_Prompt "indicate what you wish to do to $KITDEV " ANS \
	 "$ABBREV $PROCESS c change-device-name \n s skip q quit" \
	 "" "
	- Type '$PROCESS' if the next kit volume to be processed
	  is loaded and ready.

	- Type 'skip' if you wish to skip the processing of this volume.

	- Type 'change' if you wish to change the Kit Device name
	  that is to be used.

	- Type 'quit' to exit the procedure now.

	Any of these responses can be abbreviated to a single letter."`

    case "$ANS" in
	${ABBREV}*)	ANS="process"; break;;
	q*)		ANS="quit"; break ;;	
	s*)		ANS="skip"; break ;;
	c*)		# change: get name and check it.
	    eval `_U_Prompt "New Kit Device name" \
		    NEWDEV "" "$KITDEV" "
	The Kit Device is the name of the magnetic tape device
	where the software kit will be build.  The Kit Device
	should be the non-rewinding version of the device, if one
	exists; however, the device will be converted to the
	non-rewinding form even if a rewinding version is specified.


	The default is the current Kit Device, $KITDEV"`

	    #
	    # Check: is the device a special device?
	    #
	    info=`file $NEWDEV`
	    if echo "$info" | grep -s "special"
	    then
		#
		# Check out the volume, return its best name
		# in DEVICE=
		#
		DEVICE=
		eval `_U_Check_Volume $NEWDEV $ACCESS` || continue
		KITDEV="$DEVICE"
		echo 1>&2 "\nKit Device set to $KITDEV\n"
	    else
		cat 1>&2 <<- -EOF
		    The file name you specified, $NEWDEV, is not a special
		    device file.

		    If you are debugging this procedure, then a regular
		    file name can be appropriate here, otherwise, you really
		    must use a file name of the form:

		        /dev/xxxxx

		    where xxxxx is the name of the device where the kit
		    media is loaded.  For example, /dev/nrmt1h.

		-EOF

		eval `_U_Prompt "*Really* use $NEWDEV as the Kit Device " \
			RESP "y yes n no" "no" "
		    You should reply 'no', unless you are *sure* you want to
		    $PROCESS the output to ${NEWDEV} and not a special device
		    file."`

		case $RESP in
		    n*) continue ;;
		    y*)
			#
			# Check out the volume, return its best name
			# in DEVICE=
			#
			DEVICE=
			eval `_U_Check_Volume $NEWDEV $ACCESS` || continue
			KITDEV="$DEVICE"
			echo 1>&2 "\nKit Device set to $KITDEV\n"
			;;
		    *)
			echo 1>&2 "?Invalid return from _U_Prompt"
			return 1
			;;
		esac
	    fi
	    ;;

	*)				# other: error
	    echo 1>&2 "?$PROG: Invalid return from _U_Prompt"
	    exit 1
	    ;;
    esac
done

#
# return the assignments for KITDEV and ANS
#
echo KITDEV='"'$KITDEV'"' ANS='"'$ANS'"'
return 0
)}
##### End GT_Change_Volume

#
#
# GT_Create_Kit()
#    Create the kit media from the subset lists.
#
GT_Create_Kit()
    {
    (test $DEBUG && echo 1>&2 "DEBUG: GT_Create_Kit($*)")

    #
    # Clear the VERIFYLIST
    #
    VERIFYLIST=

    #
    # Give an appropriate message
    #
    OPNAME="Generating"
    if [ $CHECK_ONLY -ne 0 ]
    then
	OPNAME="Checking"
    fi	

    DISPHOST=`echo $HOST | sed 's=\(.*/.*\)/.*=\1/password::='`
    case $MAXVOL in
	"" | 1)
	    echo 1>&2 "$OPNAME $KITCODE kit from $DISPHOST on $DEV"
	    ;;
	*)
	    echo 1>&2 "$OPNAME volume $THISVOL of $KITCODE from" \
	              "$DISPHOST on $DEV"
	    ;;
    esac

    echo 1>&2 "This volume will contain the following subsets:"
    echo "$SUBSETLIST" | fmt | sed 1>&2 's/^/    /'
    echo 1>&2 ""

    if [ $CHECK_ONLY -ne 0 ]
    then
	echo 1>&2 "Note: component existence check only. No kit will be made."
    fi

    #
    # rewind tape
    #
    case $DEV in
	/dev/*rmt*)
	    mt -f $DEV rew &
	    echo 1>&2 rewind.
	    ;;
    esac

    if [ $CHECK_ONLY -eq 0 ]
    then
        echo 1>&2 "Write."
    fi

    # write each subset to the tape
    PREVSUBSET=
    for K in $SUBSETLIST
    do
	#
	# On CHECKS, skip redundant consecutive subsets (like SPACE)
	#
	if [ $CHECK_ONLY -ne 0 ]
	then
	    if [ "$PREVSUBSET" = "$K" ]
	    then
	        PREVSUBSET="$K"
		continue
	    fi

	    PREVSUBSET="$K"
	fi

	#
	# Write the subset to tape
	#
	# subset entries are of the form SUBSET|blocksize
	# extract the blocksize, if there is one.
	#
	OFS="$IFS"
	IFS='|'
	set $K
	IFS="$OFS"

	IMAGE=$1

	# 2nd arg, if any, is the blocksize
	case "$2" in
	   "")	BS=20
		;;
	    *)  BS=$2
		;;
	esac

	echo 1>&2 "Image $IMAGE...\c"
	VERIFYLIST="$VERIFYLIST $K"
	wait

	#
	# Determine which directory the subset file
	# is in.
	#
	case $PRODLIST in
	    "" )
		;;

	    * )
		case $IMAGE in
		    INSTCTRL )
			FILEDIR=$MERGEDIR
			;;
		    * )
			# find IMAGE in $DIRELIST;
			# FILEDIR is the return value
			eval `_U_Find_Dir $HOST $IMAGE "$DIRELIST"`

			case $FILEDIR in
			    "")
				echo 1>&2 "Cannot find $IMAGE in any of the" \
					  "following directories:"
				echo 1>&2 "$DIRELIST"

			    	#
				# If not found and DECnet, try again,
				# in case of timeout problems
				#
				case $HOST in
				    *:) ;;
				    *) return 1
				esac
				echo 1>&2 "\nTrying again. . ."

				eval `_U_Find_Dir $HOST $IMAGE "$DIRELIST"`

				case $FILEDIR in
				    "")
					echo 1>&2 "\07 $IMAGE not found in" \
						  "any of the following" \
						  "directories:"
					echo 1>&2 "$DIRELIST"
					return 1
					;;
				esac

				;;
			esac
			;;
		esac
		;;
	esac

	#
	# save a copy of the copy command used and any output that
	# occurs on its execution in a temp file that will be printed
	# in the event of an error return from dd.
	#
	if [ $CHECK_ONLY -eq 0 ]
	then
	    #
	    # if the file directory is the merge directory, then
	    # we are getting our copy of this file locally and not
	    # from the software host
	    #
	    if [ "$FILEDIR" = "$MERGEDIR" ]
	    then
		echo "dd if=$FILEDIR/$IMAGE of=$DEV bs=${BS}b" > $DD_ERRS
		dd if=$FILEDIR/$IMAGE of=$DEV bs=${BS}b 2>> $DD_ERRS
	    else
		# for remote files, try a few times before giving up
		# Be sure to put '0' as the *end* of the iteration list
		#
		for try in $RETRIES 0
		do
		    echo "_U_Remote_dd $DISPHOST $FILEDIR/$IMAGE" \
			    " $DEV 'obs=${BS}b'" > $DD_ERRS
		    _U_Remote_dd $HOST $FILEDIR/$IMAGE \
			    $DEV "obs=${BS}b" 2>> $DD_ERRS ||
			{
			echo 1>&2 "\nError on copy:"
			sed 1>&2 's/^/    /' $DD_ERRS

			case $DEV in
			    /dev/*rmt*)
				#
				# reposition for retry
				#
				echo 1>&2 "\nRepositioning tape to" \
					  "retry write."

				# if no data written, don't skip 2
				if grep -s "0+0 records out" $DD_ERRS
				then
				    mt -f $DEV bsf
				else
				    mt -f $DEV bsf 2
				fi

			    mt -f $DEV fsf
			esac

			# local builds only get one try, because no network
			if [ $try -eq 0 -o "$HOST" = "local" ]
			then
			   try="error"
			   echo 1>&2 "giving up."
			else
			   sleep `expr $try \* 10`
			   echo 1>&2 "Retrying $IMAGE...\c"
			fi
			continue
			}
		    break
		done
		test "$try" != "error"
	    fi ||
		{
		echo 1>&2 "\07Failed:"
		sed 1>&2 's/^/	/' $DD_ERRS
		rm $DD_ERRS
		return 1
		}

	    if [ "$DEBUG" ]
	    then
		cat $DD_ERRS
	    fi

	    rm $DD_ERRS
	    echo 1>&2 "done.\n"
	else
	    echo 1>&2 "exists.\n"
	fi
    done

    STAT=0
    }
##### End GT_Create_Kit 

#
# GT_Verify_Tape
#    Verify that the kit mounted on the kit device contains
#    the same bits as in the original directories.  This is done
#    by comparing, byte for byte, the boot images (*TK50*) and
#    by comparing checksums for the other files.
#
#    At the end of the verification, the size of the volume in
#    Mbytes will be printed.
#
GT_Verify_Tape()
    {
    (test $DEBUG && echo 1>&2 "DEBUG: GT_Verify_Tape($*)")

    #
    # Return success immediately if there is nothing to verify
    #
    if [ "$VERIFYLIST" = "" ]
    then
	return 0
    fi
    #
    # Give an appropriate message
    #
    DISPHOST=`echo $HOST | sed 's=\(.*/.*\)/.*=\1/password::='`

    case $MAXVOL in
	"" | 1)
	    echo 1>&2 "Verifying $KITCODE kit from $DISPHOST on $DEV"
	    ;;
	*)
	    echo 1>&2 "Verifying volume $THISVOL of $KITCODE from" \
	              "$DISPHOST on $DEV"
	    ;;
    esac

    echo 1>&2 "This volume should contain the following subsets:"
    echo "$SUBSETLIST" | fmt | sed 1>&2 's/^/    /'
    echo 1>&2 ""

    case $DEV in
	/dev/*rmt*)
	    mt -f $DEV rew &
	    echo 1>&2 rewind.
	    ;;
    esac

    STAT=1	# reset to failure, will set to success at end

    case $PRODLIST in
	"")
	    FILELOC=$HOST
	    ;;
	* )
	    FILEDIR=$MERGEDIR
	    FILELOC=local
	    ;;
    esac


    # use 1st 3 chars of kitcode as image name
    CODE=`expr $KITCODE : '\([A-Z0-9][A-Z0-9][A-Z0-9]\)'`

    _U_Remote_Copy $FILELOC $FILEDIR/$CODE.image \
	    $LCL_DIR/lcl_$CODE.image$$ ||
	{
	echo 1>&2 "\07$PROG: error accessing checksum file."
	return 1
	}

    VOLSIZE=0	# initialize the size of the volume to 0

    for V in $VERIFYLIST
    do
	# subset entry is of the form SUBSET|blocksize
	OFS="$IFS"
	IFS='|'
	set $V
	IFS="$OFS"

	IMAGE=$1

	# if there is a 2nd argument, it is the blocksize
	case "$2" in
	    "") BS=20
		;;
	    *)  BS=$2
		;;
	esac

	echo 1>&2 "Image $IMAGE...\c"
	wait	# for any rewinds to finish

	case $IMAGE in
	    *TK50* )
		case $PRODLIST in
		    "") ;;
		    * )
			# find IMAGE in $DIRELIST;
			# FILEDIR is the return value
			eval `_U_Find_Dir $HOST $IMAGE "$DIRELIST"`\
							 2> $LCL_DIR/finderrs$$
			;;
		esac

		case $FILEDIR in
		    "" )
			echo 1>&2 "\n$PROG: source ${IMAGE} not found. Reason:"
			sed 's/^/    /' $LCL_DIR/finderrs$$
			rm $LCL_DIR/finderrs$$
			case $DEV in
			    /dev/*rmt*)
				mt -f $DEV fsf ||
				    {
				    echo 1>&2 "\07Tape positioning error."
				    return 1
				    }
				;;
			esac
			;;

		    * )
			rm $LCL_DIR/finderrs$$

			#
			# Calculate the checksums
			#
			SUM1=$LCL_DIR/sum_a$$
			SUM2=$LCL_DIR/sum_b$$

			#
			# start the device checksum in the background
			#
			echo "dd if=$DEV bs=${BS}b" > ${DD_ERRS}-b
			dd if=$DEV bs=${BS}b 2>> ${DD_ERRS}-b |
				sum > $SUM2 &

			#
			# Now try for the checksum "from the wire"
			# Do this in the foreground, since it might
			# fail.
			#
			for try in $RETRIES 0
			do
			    echo "_U_Remote_dd $DISPHOST $FILEDIR/$IMAGE" \
				 "obs=${BS}b" > $DD_ERRS
			    _U_Remote_dd $HOST $FILEDIR/$IMAGE "" \
				    "obs=${BS}b" 2>> $DD_ERRS |
				sum > $SUM1

			    #
			    # Check, to see if dcp put any messages into
			    # the error file.  If it did, then we have
			    # an error, try the command again.
			    #
			    if grep -s "[rd]cp:" $DD_ERRS
			    then
			        echo 1>&2 "\nError in checksum calculation:"
				sed 1>&2 's/^/    /' $SUM1 $DD_ERRS

				# local builds get only one try
				if [ $try -eq 0 -o "$HOST" = "local" ]
				then
				    echo 1>&2 "giving up."
				    break # script will fail in sum comp code
				else
				    sleep `expr $try \* 10`
				    echo 1>&2 "Retrying calculation. . ."
				    continue
				fi
			    fi

			    break # if we get here, then checksum exists
			done

			wait	# for tape summation to finish, if necessary.

			#
			# No need to check "wire" checksum for errors
			# since the verify will fail a little later on.
			#

			if [ "$DEBUG" ]
			then
			    echo "IMAGE DD_ERRS:"
			    cat $DD_ERRS
			    echo "\nDEV DD_ERRS:"
			    cat $DD_ERRS-b
			fi
			    
			#
			# compare the check sums (only)
			#
			set xxx `cat $SUM1`
			shift
			ISUM=$1 ISIZE=$2

			set xxx `cat $SUM2`
			shift
			DSUM=$1 DSIZE=$2

			case "$ISUM$ISIZE" in
			    $DSUM$DSIZE)
				;;
			    *)
				echo 1>&2 "\n\07$PROG: Image $IMAGE checksum" \
					  "does not match device checksum:\n" \
					  "    $ISUM != $DSUM or\n" \
					  "    $ISIZE != $DSIZE"
				echo 1>&2 "Verify failed!!\nExtra info:" \
					  "\nImage:"
				sed 1>&2 's/^/    /' $DD_ERRS
				echo 1>&2 "\nDevice:"
				sed 1>&2 's/^/    /' ${DD_ERRS}-b

				rm $DD_ERRS ${DD_ERRS}-b $SUM1 $SUM2
				return 1
				;;
			esac

			rm $DD_ERRS ${DD_ERRS}-b $SUM1 $SUM2
			;;
		esac

		# Add the size of this image to the volume size
		VOLSIZE=`expr $VOLSIZE + $DSIZE`

		echo 1>&2 "done.\n"
		continue
		;;
	esac

	#
	# get the checksum information out of
	# the local copy of the image file.
	#
	grep $V $LCL_DIR/lcl_$CODE.image$$ > $LCL_DIR/sum$V$$ ||
	    {
	    # only subsets have entries in the image file and all
	    # "official" subsets should have them. If they don't,
	    # flag a warning.
	    case $V in
		ROOT | [A-Z0-9][A-Z0-9][A-Z0-9]*[0-9][0-9][0-9])
		    echo 1>&2 "\nWarning: No checksum entry found for" \
		    		"subset $V"
		    ;;
	    esac

	    #
	    # We continue on so that we can calculate the volume
	    # size.
	    #
	    }


	#
	# If we have the INSTCTRL checksum in a variable, then set
	# make it look as though the information came out of the image
	# file.
	#
	case $IMAGE in
	    INSTCTRL)
		if [ "$INSTCTRL_CHECKSUM" ]
		then
		    echo "$INSTCTRL_CHECKSUM" > $LCL_DIR/sum$V$$
		else
		    echo "INSTCTRL checksum not checked because information" \
			 "is not available."

		    if [ $WRITE -ne 0 ]
		    then
			echo "?This is an error, since the information" \
			     "should be available."
			exit 1
		    else
			echo "This is not an error, since the checksum is only"
			echo "available when the kit is built."
		    fi
		fi
		;;
	esac

	#
	# extract the checksum and filesize, if there is one
	#
	if [ -s $LCL_DIR/sum$V$$ ]
	then
	    set xxx  `cat $LCL_DIR/sum$V$$`
	    shift

	    SUM=$1
	    SIZE=$2
	fi

	#
	# calculate the tape checksum for this file
	#
	dd if=$DEV bs=${BS}b 2> $DD_ERRS |
	    sum > $LCL_DIR/SUM$$ ||
		{
		echo 1>&2 "\n\07$PROG: checksum error image $V."
		cat 1>&2 $DD_ERRS
		rm $DD_ERRS
		return 1
		}

        test "$DEBUG" &&
	    {
	    echo "\nDD_ERRS$$ file:"
	    cat $DD_ERRS
	    echo "\nDEBUG: kit sum file, $LCL_DIR/sum$V$$ is:"
	    cat $LCL_DIR/sum$V$$
	    echo "DEBUG: sum file, $LCL_DIR/SUM$$ is:"
	    cat $LCL_DIR/SUM$$
	    }

	rm $DD_ERRS

	set xxx `cat $LCL_DIR/SUM$$`; shift
	DEVSUM=$1 DEVSIZE=$2

	# Add the size of this image to the volume size
	VOLSIZE=`expr $VOLSIZE + $DEVSIZE`

	#
	# Only compare the checksums if we have them both.
	#
	if [ -s $LCL_DIR/sum$V$$ ]
	then
	    # now check it against the image file values
	    case "$DEVSUM$DEVSIZE" in
	    $SUM$SIZE)
		    ;;
	    *)
		    echo 1>&2 "\n\07$PROG: Image file checksum for $IMAGE"\
			      "does not match device checksum:\n" \
			      "    $SUM != $DEVSUM or\n" \
			      "    $SIZE != $DEVSIZE"
		    return 1
	    esac
	fi

	rm -f $LCL_DIR/sum$V$$
	rm -f $LCL_DIR/SUM$$
	echo 1>&2 "done.\n"
    done

    echo "Volume $THISVOL contains approximately $VOLSIZE blocks\n"

    echo 1>&2 rewind.
    case $DEV in
	/dev/*rmt*)
	    mt -f $DEV rew
	    ;;
    esac

    STAT=0	# Successful, conclusion
    }
##### End GT_Verify_Tape

######################################################################
######################################################################
# Main Line Program Start #
#################
#start:

Gentapes_Mainline()
{
PROG=`basename $0`
umask 022	# allow no group or world write access
STAT=1		# assume failure, unless explicitly set to success

TAPEDEV="mt"	# for DEC: production

#
# Initialize tape volume information
#
THISVOL=1		# current volume number
VOLUME_LIST="$THISVOL"	# list of volumes to build/verify

#
# Initialize the number of retries
#
RETRIES="1 2 3 4 5 6 7 8 9 10"

#
# Define the local locations for our copies of the
# remote kitcap file and merged instctrl information
# Also the error file for dd commands
#
# Note that all temp files contain the process number
# at the very end of the name, to facilitate cleanup.
#
LCL_DIR=/usr/tmp
LCL_KITCAP=$LCL_DIR/lcl_kitcap$$
DD_ERRS=$LCL_DIR/dd_errors$$

#
# define the kitcap location, merge and instctrl dirs
# using environment variables, if they exist.
#
MERGEDIR=${GENTAPE_MERGEDIR:-"$LCL_DIR/merge"$$}
WORKDIR=${GENTAPE_WORKDIR:-"$MERGEDIR/instctrl"}
REM_KITCAP=${KITCAP_LOCATION:-"/etc/kitcap"}
KITROOT=${KITCAP_KITROOT:-""}

DEBUG=${GENTAPE_DEBUG-""}

COMPLIST=
PRODLIST=
PRODCODE=
DIRELIST=
SUBSETLIST=

STAT=1	# assume failure

#
# Establish a trap to return proper status when finished or on error
#
trap '	exit $STAT' 0 1 2 3 15

#
# Process command line switches and arguments.
# Return WRITE, VFY, HOST, KITCODE and DEV from command line.
#
# default action is write *and* verify
#
WRITE=0 VFY=0 HOST= KITCODE= DEV= DEVTYPE= VOL_START=

eval `GT_Get_Arguments $*` || exit 1	# exit on failure

#
# Define a few words and actions depending on what we
# were called to do.
#
if [ $WRITE -ne 0 ]
then
    PROCESS="write"
    ACCESS="write"
    if [ $VFY -ne 0 ]
    then
	OP="created"
    else
	OP="created and verified"
    fi
else
    PROCESS="verify"
    ACCESS="read"
    OP="verified"
fi

#
# Now make sure that we have a volume (rewound to BOT, if a tape)
# that is accessible for reading or writing  as necessary.
#
DEVICE=
eval `_U_Check_Volume $DEV $ACCESS "kit"` || exit 1
DEV=$DEVICE

#
# Establish a trap to cleanup when finished or on error
#
trap '  echo 1>&2 "Cleaning up. . ."
	rm -rf $MERGEDIR $WORKDIR $LCL_DIR/*$$
	case $STAT in
	    0)
		echo 1>&2 "Operation successfully concluded"
		;;
	    *)
		echo 1>&2 "Error during operation"
		;;
	esac
	exit $STAT' 0 1 2 3 15
#
# Copy the remote kitcap file to the local
# kitcap area, substituting $KITCAP_KITROOT_TOKEN (usually %%KITROOT%%)
# with $KITROOT, if it appears.
#
_K_Get_Local_Kitcap $HOST $REM_KITCAP $LCL_KITCAP $KITROOT || exit 1

#
# _K_Interpret_Kitcap returns assignments for the following:
#	K_NAME, NEWIMAGE, KITDESC, PRODLIST, KITDIRS, SUBSETLIST and NVOLS
#
K_NAME= NEWIMAGE= KITDESC= PRODLIST= KITDIRS= SUBSETLIST= NVOLS=
eval `_K_Interpret_Kitcap "${KITCODE}${DEVTYPE}" "$LCL_KITCAP"` || exit 1

#
# Figure out if we're working with a single product, or merging two
# or more products into one.
#
set xxx $PRODLIST
case $# in
    1 )
	test $DEBUG && echo 1>&2 "DEBUG: we are a single product"

	PRODLIST=$KITCODE
	;;
    * )
	test $DEBUG && echo 1>&2 "DEBUG: we are a multiple products"
	;;
esac

#
# We are ready to build the image file.
# Create the temporary directories
mkdir $MERGEDIR
mkdir $WORKDIR

cd $WORKDIR

#
# Unpack instctrl files and create the image file.
# If anything fails, quit.
#
# Output is DIRELIST.
#
DIRELIST=

if eval `_K_Unpack_Instctrls $HOST $KITCODE $LCL_KITCAP $DEVTYPE "$PRODLIST"`
then
    if test "$DIRELIST"
    then
	_K_Image_Merge $NEWIMAGE "$SUBSETLIST" || exit 1
    else
	echo 1>&2 "$PROG: DIRELIST is null.  Internal error."
	exit 1
    fi
else
    exit 1
fi

#
# The SUBSETLIST is provided to us by the Interpret Kitcap
# function as a series of subset names punctuated by volume
# change tokens.
#
# We must split this into a set of subset lists for each volume
#
FIRSTVOL=$THISVOL
MAXVOL=1
for ITEM in $SUBSETLIST
do
    case $ITEM in
	%%*)		#volume change
	    # save the current subsets
	    eval VOL${THISVOL}_SUBSETS=\"$CUR_SUBSETS\"
	    CUR_SUBSETS=

	    # get new volume information
	    THISVOL=`expr $ITEM : '%%\(.*\)'`
	    VOLUME_LIST="$VOLUME_LIST $THISVOL"

	    # Keep track of highest volume number
	    if [ $MAXVOL -lt $THISVOL ]
	    then
		MAXVOL=$THISVOL
	    fi
	    ;;

	*)		# add subset to volume list
	    CUR_SUBSETS="$CUR_SUBSETS $ITEM"
	    ;;
    esac
done

# save the last bunch of current subsets
eval VOL${THISVOL}_SUBSETS=\"$CUR_SUBSETS\"
CUR_SUBSETS=

#
# If the user requested a specific volume or volumes, then take
# the first of those as the current volume.
#
if [ "$VOL_START" ]
then
    FIRSTVOL=$VOL_START

    #
    # Is the first volume too high?
    #
    if [ $FIRSTVOL -gt $MAXVOL ]
    then
	echo 1>&2 "?$PROG: Starting volume #$FIRSTVOL is greater than" \
		  "the total number of volumes ($MAXVOL)."
	exit 1
    fi
else
    if [ $MAXVOL -gt 1 ]
    then
        SUFFIX="s"
    fi

    echo "$MAXVOL volume$SUFFIX will be ${OP}."
fi


#
# Adjust the VOLUME_LIST to begin with THISVOL.
#
set xx $VOLUME_LIST; shift
for THISVOL
do
    if [ $THISVOL -eq $FIRSTVOL ]
    then
	break
    fi
    shift
done
VOLUME_LIST=$*

#
# Perform all merge operations prior to writing any volumes
#
INSTCTRL_CHECKSUM=
case $WRITE$VFY in
    01 )		# Verify only, set VERIFYLIST to all subsets
	;;

    10)		# Create only, skip verify pass
	_K_Product_Merge $NEWIMAGE "$K_NAME" "$SUBSETLIST" || exit 1

	#
	# Calculate the checksum of the INSTCTRL file
	# for verification purposes.
	#
	INSTCTRL_CHECKSUM=`sum $MERGEDIR/INSTCTRL`
	;;

    11)		# Create & Verify, verify the current VERIFYLIST only
	_K_Product_Merge $NEWIMAGE "$K_NAME" "$SUBSETLIST" || exit 1

	#
	# Calculate the checksum of the INSTCTRL file
	# for verification purposes.
	#
	INSTCTRL_CHECKSUM=`sum $MERGEDIR/INSTCTRL`
	;;

    *)
	echo 1>&2 "$PROG: Internal error. " \
		"Unexpected values for WRITE (=$WRITE) and/or VFY (=$VFY)"
	exit 1
	;;
esac

#
# Free up space in the temp directory
#
rm -rf $WORKDIR

for THISVOL in $VOLUME_LIST
do
    #
    # Request a volume change, if necessary
    #
    if [ "$THISVOL" != "$FIRSTVOL" ]
    then
	#
	# Prompt for a volume load/change/skip.
	# Return values are in ANS and KITDEV
	# ANS can be one of $PROCESS, "quit", or "skip"
	#
	ANS= KITDEV=
	eval `GT_Change_Volume $DEV $ACCESS $PROCESS \
		"volume $THISVOL of $KITCODE"` || exit 1
	DEV=$KITDEV

	case $ANS in
	    process) ;;

	    quit)
		exit 1
		;;

	    skip)
		continue
		;;
	    *)
		echo "?$PROG: Unexpected return ($ANS) from GT_Change_Volume"
		exit 1
		;;
	esac

    fi

    #
    # Load up the current subsets
    #
    VERIFYLIST=
    eval SUBSETLIST=\"\$VOL${THISVOL}_SUBSETS\"

    # Process accordingly
    case $WRITE$VFY in
	01 )		# Verify only, set VERIFYLIST to all subsets
	    VERIFYLIST=$SUBSETLIST
	    GT_Verify_Tape || exit 1
	    ;;

	10)		# Create only, skip verify pass
	    GT_Create_Kit || exit 1
	    ;;

	11)		# Create & Verify, verify the current VERIFYLIST only
	    GT_Create_Kit || exit 1
	    GT_Verify_Tape || exit 1
	    ;;
    esac
done
if [ $CHECK_ONLY -ne 0 ]
then
    echo 1>&2 "Check of \c"
fi
echo 1>&2 "\07Kit $KITCODE done."
exit $STAT
}
##### End Gentapes_Mainline
case $GENTAPES_NO_EXECUTE in
    "") Gentapes_Mainline $* ;;
esac
