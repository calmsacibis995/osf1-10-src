:	-Kitcap
#		kitcap manipulation routines.
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
# @(#)$RCSfile: Kitcap.sh,v $ $Revision: 4.1.3.3 $ (DEC) $Date: 1992/10/02 07:42:41 $ 
# 
#
# Modification History
# ~~~~~~~~~~~~~~~~~~~~
# 11	May-91, ech
#	Ported to OSF/1.
#
# 10	Sep-90, Greg Tarsa
#	Added code to Interpret Kitcap, based on an ORDS_FLAG, which will
#	inhibit the printing of the list of valid kit codes when an invalid
#	kit code is specified.
#
# 09	May-90, Greg Tarsa
#	added a -f option to an rm command so that it would not prompt
#	if the temp file had no write access.
#
# 08	Feb-90, Greg Tarsa
#	Added more options to INSTCTRL unpacking error.
#	Added another message check to first copy failure.
#
# 07	Jan-90, Greg Tarsa
#	Changed call to dir_exists to reflect new calling sequence.
#	Removed warning message that occurred when .comp files were not
#	found.  It turns out that this is a legitimate situation.
#	Added extra error messages to failed kitcap copy in order to clarify
#	confusion over what the basic error messages mean.
#
# 06	Oct-89, Greg Tarsa
#	Changed messages Unpack Instctrls function so that they make sense
#	whether we are building or verifying.
#	Added code to ask to operator whether or not to abort a build if
#	a .ctrl file is found to be missing.
#	Added DISPHOST code to keep passwords from being printed in logs.
#	Fixed problem with MT to TK coercion.
#	Changed code to allow any alphanumeric as part of the 1st 3
#	characters of the kitcode.
#	
# 05	Sep-89, Greg Tarsa
#	Changed Interpret Kitcode function to generate the kitcode from
#	the kittype.
#	Added code to replace an old gentapes function that got lost
#	in the translation effort: if an MT is being built and a *MT
#	entry does not exist in the kitcap file, then use the *TK line
#	instead.
#	Expanded code to allow dot relative paths for directories in
#	kitcap, as well as absolute rooted paths; this is necessary for
#	the on-line distribution code to work over a network where the
#	actual root to the kits directory is not known.
#	Fixed bug where KITNAME was being forced to null whenever it was
#	not specified in the kitcap, rather than remaining the value it
#	was in the original ctrl file.
#	Fixed bug where KITROOT substitution was being ignored when
#	the prettying code was added.
#	Changed the method by which the ctrl file gets re-written from
#	executing the old ctrl file and writing the new one to editing
#	the old one into the new one.  This paves the way for Tresvik's
#	group to add backquote substitutions to the ctrl files.
#
# 04	Aug-89, Greg Tarsa
#	Fixed bug in the kind of quotation marks used in the re-written
#	control files.
#	Fixed bug in merge code that missed reseting the LOCNUM
#	to its initial value.
#	Added code to support user-readable kitcap files.
#	Added code to support new K-file names.
#	Added new image merge algorithm from Jon Wallace's code.
#
# 03	Jul-89, Greg Tarsa
#	Changed "test" to "[".
#	Added special TK50 processing to ignore volume change info.
#	Simplified printing of info to stderr.
#	Improved handling of subset names with appended blocking info.
#	Removed $0. from start of error messages.
#	Added an environment variable check for KITCAP_KITROOT_TOKEN
#	    to allow the KITROOT_TOKEN default to be overridden by caller.
#
# 02	Jun-89, Greg Tarsa
#	Merged in new capabilities from Jon Wallace (multiple
#	    directories in kitcap entries, building .ctrl files
#	    on the fly, etc.
#	Merged in old GenTapes.sh5 routines.
#	Added variable initialization to Interpret_Kitcap
#	Added code to keep from including twice
#	Overhaul and extend; add Get_Local_Kitcap.
#
# 01	15-Jun-1989, Greg Tarsa
#	Wrote Original Routines
#####
#   What follows are a number of Utility routines for
#   scripts using the Kitcap file.
#
#	Environment variables of interest:
#	    UTIL_DEBUG	when non-null, it causes the arglists of each
#	    function to be placed on stderr prior to function execution.
#
#	    KITCAP_KITROOT_TOKEN when non-null is used as the
#	    kitroot token in the kitcap copying functions.
#
#    Notes:
#    These routines have READONLY access to the caller's symbol table.
#
#    Calling any of these routines will destroy any "set" arguments
#    in the caller.
#
#    _K_Interpret_Kitcap(kittype, kitcap)
#	Searches the kitcap file for the kitcode and extracts
#	and prints assignments for K_NAME, NEWIMAGE, KITDESC, PRODLIST,
#	KITDIRS, SUBSETLIST and NVOLS.  See function for more info.
#		
#    _K_Get_Local_Kitcap(host, rem_kitcap, lcl_kitcap, kitroot)
#	Copy the remote kitcap (from the host, if necessary)
#	and substitute the KITROOT_TOKEN with the kitroot
#	directory, if it is specified.
#
#    _K_Unpack_Instctrls(host, kitcode, kitcap, prodlst)
#	For each product comprising the specified kitcode in the
#	specifiec kitcap, verify the existence of the kit directories
#	and copy all INSTCTRL files from them.
#		
#    _K_Image_Merge(newimagename, subsetlist)
#	Create a merged image file from the component image files.
#
#    _K_Product_Merge(imagename, subsetlist)
#	Merge multiple products together by creating a new INSTCTRL
#	from the component subset information.
#####

#
# Set the "already included" flag.
# this flag can be tested to see if this module has been loaded already.
#
_Kitcap_included="yes"

#
# _K_Interpret_Kitcap(kittype, kitcap)
#    Search the specified local kitcap file and extract information for
#    the kit image file name, kit description, list of subsets comprising
#    the product and either the directory containing the subsets, or the
#    list of component products comprising the kit.  These are returned
#    by printing assignments for K_NAME, NEWIMAGE, KITDESC, PRODLIST, KITDIRS,
#    SUBSETLIST and NVOLS (number of volumes in kit) onto standard output.
#
#    kittype	the designation for the kit to use
#    kitcap	the path to the local copy of the kitcap file
#
_K_Interpret_Kitcap()
{(
FUNC=_K_Interpret_Kitcap NUMARGS=2
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

#
# Reject attempt if we do not have the right number of arguments
case "$#" in
    ${NUMARGS})	# right number of args, perform search
        KITTYPE="$1"
	KITCAP="$2"
	;;

    *)		# wrong number of args, error
	echo 1>&2 "$FUNC($# args): Must have ${NUMARGS} arguments"
	exit 1
	;;
esac

K_NAME= NEWIMAGE= KITDESC= PRODLIST= KITDIRS= SUBSETLIST= NVOLS=0

#
# Separate the kitcode from the device type.
#
KITCODE=`expr $KITTYPE : '\(.*\)..'`
DEVTYPE=`expr $KITTYPE : '.*\(..\)'`

#
# Get the kitcode out of the kitcap file.
# The kitcap name field consists of a kitcode followed by an optional
# vertical bar and .K file name field.
#
KITDESC=`egrep "^$KITTYPE[:|]" $KITCAP ` ||
    {
    #
    # Tape hack: if we are an MT9 and there is no MT kitcap
    # entry, then look for a TK50 entry and use that.  This is because
    # different entries are only necessary for bootable tapes.
    #
    # NOTE: changes to this code need to be reflected in the
    # Unpack Instctrls function.
    #
    ERRFLG=0
    if [ "$DEVTYPE" = "MT" ]
    then
	KITDESC=`egrep "^${KITCODE}TK[:|]" $KITCAP ` ||
	    {
	    echo 1>&2 "\07$FUNC: can't find either $KITTYPE or" \
	    	      "${KITCODE}TK kit descriptors in kitcap."
	    ERRFLG=1
	    }
    else
	echo 1>&2 "\07$FUNC: can't find $KITTYPE kit descriptor in kitcap."
	ERRFLG=1
    fi

    #
    # We had a bad kitcode, print the list of good kitcodes
    # and then exit.
    #
    if [ $ERRFLG -ne 0 ]
    then
	if [ "$ORDS_FLAG" = "Y" ]
	then
	    #
	    # ORDS hack: too many customers are poking around trying to
	    # build products for which they are not registered.
	    # To help combat this, we are not telling them what
	    # is available at this time.
	    #
	    echo 1>&2 "\n\tPlease consult your registration packet for the"
	    echo 1>&2 "\tlist of valid kit codes."
	else
	    (
	    echo "\nRecognized kit codes are as follows:"
	    #
	    # Print a three column of the kitcodes.  The
	    # list is made up of the first entry of every
	    # non-comment line.  We know assume that $KITCAP
	    # has already been processed to remove prettifying
	    # characters so we have a simpler time processing it.
	    #
	    egrep '^[A-Z0-9]' $LCL_KITCAP |
		    sed -e '/^ULTTEST/d' \
			-e 's/:.*$//' \
			-e 's/[MT][TK][:|].*//' |
		    sort -u |
			pr -3 -t -l20 -o4
	    ) | more 1>&2
	fi
	echo 1>&2
	exit 1
    fi
    }

#
# Split off pieces of variables that we will need to work with later.
#	NEWIMAGE is first three capital alphabetics of kitcode
#
NEWIMAGE=`expr $KITCODE : '\([A-Z0-9][A-Z0-9][A-Z0-9]\)'`

#
# Seperate kitcap fields, then place them into corresponding components 
# 
OFS="$IFS"
IFS=:
set $KITDESC
RAW_NAME="$1"	# save raw name field
shift		# shift past the kitname
IFS="$OFS"

for K in $*
do
    case $K in
    *TK | *MT)		# entries ending in "TK" or "MT" are product codes
			# make a list
	PRODCODE=`expr $K : '\(.*\)[TM][KT]'`
	PRODLIST="$PRODLIST $PRODCODE"
	;;

    /* | ./* )		# entry starting with slash is the dir path, save it.
	KITDIRS="$KITDIRS $K"
	;;

    %%*)		# entries starting with %% are volume numbers.
	#
	# Save the volume change number info and keep track
	# of the total number of volumes.
	#
	SUBSETLIST="$SUBSETLIST $K"

	# get the volume number
	VOL=`expr $K : '%%\(.*\)'`

	# keep track of the highest volume number seen
	if [ $VOL -gt $NVOLS ]
	then
	    NVOLS=$VOL
	fi
	;;

    * )			# All others are regular subsets, add them to the list.

	SUBSETLIST="$SUBSETLIST $K"
	;;
    esac
done

#
# Get the K_NAME out of the raw name field.
#
OFS="$IFS"
IFS='|'
set $RAW_NAME
IFS="$OFS"

case $# in
    1)			# no K_NAME if no '|'
        K_NAME=
	;;
    2)			# K_NAME is second sub-field
	K_NAME=$2
	;;
    *)
	K_NAME=$2
	echo 1>&2 "?$FUNC: WARNING: multiple synonyms in name field: $*"
	echo 1>&2 " Using $K_NAME"
	;;
esac

#
# Return the return values
#
echo K_NAME='"'$K_NAME'"'
echo NEWIMAGE='"'$NEWIMAGE'"'
echo KITDESC='"'$KITDESC'"'
echo PRODLIST='"'$PRODLIST'"'
echo KITDIRS='"'$KITDIRS'"'
echo SUBSETLIST='"'$SUBSETLIST'"'
echo NVOLS='"'$NVOLS'"'
)}
##### End _K_Interpret_Kitcap
#
# _K_Get_Local_Kitcap(host, rem_kitcap, lcl_kitcap, kitroot)
#    Copy the remote kitcap (from the host, if necessary),
#    filter the pretty characters out of it, and substitute the
#    KITROOT_TOKEN with the kitroot directory, if it is found.
#
#    host	the name of the host machine, ending with ":" if DECnet
#    rem_kitcap	the path to the copy of the kitcap file on host
#    lcl_kitcap	the path for the local copy of the kitcap file
#    kitroot	the path to replace the KITROOT_TOKEN with in the local
#		kitcap file
#
_K_Get_Local_Kitcap()
{(
NUMARGS=4 FUNC=_K_Get_Local_Kitcap
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

#
# Reject attempt if we do not have 3 or 4 arguments
case "$#" in
    3|${NUMARGS})	# right number of args, perform search
        HOST="$1"
	REMKITCAP="$2"
	LCLKITCAP="$3"
	KIT_ROOT="$4"	# will be null if only 3 args
	;;

    *)		# wrong number of args, error
	echo 1>&2 "$FUNC($# args): Must have ${NUMARGS} arguments"
	exit 1
	;;
esac

ERRS=/usr/tmp/URCerr$$

# token for kitcap root replacement
KITROOT_TOKEN=${KITCAP_KITROOT_TOKEN-"%%KITROOT%%"}

#
# Copy the kitcap file
#
# Note that this should be the first network access in any kit building
# program, hence we pay careful attention to the error messages that we
# receive so that we can give extended help to the poor user who doesn't
# know TCP from DECnet from local.
#

_U_Remote_Copy $HOST $REMKITCAP /usr/tmp/tmpkitcap$$ 2> $ERRS ||
    {
    DISPHOST=`echo $HOST | sed 's=\(.*/.*\)/.*=\1/password:='`

    uniq 1>&2 $ERRS

    echo 1>&2 "\07$FUNC: Could not copy ${DISPHOST}:${REMKITCAP}"

    if grep -s "Login incorrect." $ERRS || grep -s "unknown host:" $ERRS
    then
	echo 1>&2 "? Cannot establish TCP access to ${DISPHOST}. If you"
	echo 1>&2 "  wish DECnet access, specify the SW host with two" \
		  "colons (::)"
    else
    if grep -s "Permission denied" $ERRS
    then
	echo 1>&2 "? Cannot establish TCP access to ${DISPHOST}.  If you"
	echo 1>&2 "  wish DECnet access, specify the SW host with two" \
		  "colons (::)"
	echo 1>&2 "  or else add '`hostname` ${USER}' to your .rhosts" \
	          "file on ${DISPHOST}."
    else
    if grep -s "Connect failed, no response from object" $ERRS
    then
        echo 1>&2 "? Cannot establish DECnet access.  Possible problem:"
	echo 1>&2 "  Attempt to access kit data from an account that is not"
	echo 1>&2 "  properly registered."
    fi
    fi
    fi

    rm -f $ERRS
    return 1
    }

rm -f $ERRS

#
# Write out the sed script that will filter into single lines.
#
cat <<- '-EOFsedfile' > /usr/tmp/tmpkitcap.sed$$
    0s** save the kitcap version line *
    /@(#)/b
    /^#/d;			0s**	delete comment lines *
    /^[ 	]*$/d;		0s**	delete blank lines *
    /:#/s/:#[^:]*//g;		0s**	delete in-line comments *

    0s**---------------------------------------------------- *
    0s** Combine all lines ending with backslash             *
    0s** When there are no more backslashes, jump to comprss *
    0s**---------------------------------------------------- *
    :combine
    /\\$/!bcomprss

    s/\\$//;			0s**	remove continuation character *
    N;				0s**	Append next line w/newline *
    s/[ 	]*\n[ 	]*//;	0s**	remove newline & assoc whitespace *

    bcombine

    0s**----------------------------------------------- *
    0s** Remove duplicates that arise out of prettyness *
    0s**----------------------------------------------- *
    :comprss

    s/[ 	]*:[ 	]/:/g;	0s**	remove spaces around colons *
    s/:::*/:/g;			0s**	change 2 or more colons into 1 colon *
    s/[ 	]*|[ 	]/|/g;	0s**	remove spaces around vertical bars *

    0s**	kitcap now consists of single lines *

    /:#/s/:#[^:]*//g;		0s**	delete in-line comments *
    s/:$//;			0s**	strip trailing colon *
-EOFsedfile
echo "    /${KITROOT_TOKEN}/s==${KIT_ROOT}=g" >> /usr/tmp/tmpkitcap.sed$$

#
# Now filter the kitcap file, replacing the KITROOT_TOKEN, if necessary.
#
sed -f /usr/tmp/tmpkitcap.sed$$ /usr/tmp/tmpkitcap$$ > ${LCLKITCAP} ||
	{
	echo 1>&2 "\07$FUNC: Internal error: cannot filter local kitcap file."
	return 1
	}

#
# Now remove the tmp kitcap and sed script
#
rm -f /usr/tmp/tmpkitcap$$ /usr/tmp/tmpkitcap.sed$$

)}
##### End _K_Get_Local_Kitcap
#
#    _K_Unpack_Instctrls(host, kitcode, kitcap, devtype, prodlst)
#	Check existence of Kitcodes and Directories, extract all
#	instctrl files for each kit in the prodlst and put them
#	into WORKDIR.  When completed it echos an assignment for
#	DIRELIST to standard output, to make directory scanning
#	for other operations easier.
#
#	host	machine where kit files reside (or "local")
#	kitcode	kit code for the kit being built
#	kitcap	location of the local kitcap file
#	prodlst list of products comprising the kit being built
#
#    Globals referenced: WORKDIR
#    Output:   DIRELIST
#
_K_Unpack_Instctrls()
{(
    NUMARGS=5 FUNC=_K_Unpack_Instctrls
    test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"
    #
    # Reject attempt if we do not have 5 arguments
    case "$#" in
	${NUMARGS})	# right number of args, set values
	    HOST="$1"
	    KITCODE="$2"
	    KITCAP="$3"
	    DEVTYPE="$4"
	    PRODLIST="$5"
	    ;;

	*)		# wrong number of args, error
	    echo 1>&2 "$FUNC($# args): Needs ${NUMARGS} arguments"
	    exit 1
	    ;;
    esac

    DIRELIST=

    echo 1>&2 "\nCreating component list for $KITCODE\c"

    set $PRODLIST
    case $# in
	1)  ;;
	*)
	    echo 1>&2 " by merging entries for $PRODLIST\c"
	    ;;
    esac

    echo 1>&2 ".\n"

    # For each product in the product list
    for PROD in $PRODLIST
    do
	# Find the kitcap entry; if none, then error and exit.

	KITDESC=`egrep "^${PROD}${DEVTYPE}[:|]" $KITCAP` ||
	    {
	    #
	    # Tape hack: if we are an MT9 and there is no MT kitcap
	    # entry, then look for a TK50 entry and use that.  This is because
	    # different entries are only necessary for bootable tapes.
	    #
	    # NOTE: changes to this code need to be reflected in the
	    # Interpret Kitcap function.
	    #
	    if [ "$DEVTYPE" = "MT" ]
	    then
		KITDESC=`egrep "^${PROD}TK[:|]" $KITCAP ` ||
		    {
		    echo 1>&2 "\07$FUNC: neither MT nor TK versions of $PROD" \
			      "exists in kitcap."
		    exit 1
		    }
	    else
		echo 1>&2 "\07$FUNC: Product $PROD does not exist in $KITCAP"
		exit 1
	    fi
	    }

	# set field separator to colon and make kitcap entry into arguments
	OFS="$IFS"
	IFS=:
	set $KITDESC
	IFS="$OFS"

	for K in $*
	do
	    case $K in
	    /* | ./* )	# only process kit data directories.
		# Check: does the proper kit data directory exist?
		_U_Dir_Exists $HOST "$K" ||
		    {
		    echo 1>&2 "$FUNC:\07Kitcap error: Directory $K does" \
		    	      "not exist."
		    exit 1
		    }

		# It exists: add it to the directory list.
		DIRELIST="$DIRELIST $K"
		;;
	    esac
	done
    done

    echo 1>&2 "Copying $PRODLIST control files into working directory \n"

    # get the instctrl files for each directory
    for DIR in $DIRELIST
    do
	while : true
	do
	  (cd $WORKDIR; _U_Remote_Tar_xpf $HOST $DIR/INSTCTRL) ||
	    {
	    DISPHOST=`echo $HOST | sed 's=\(.*/.*\)/.*=\1/password:='`
	    echo 1>&2 "$FUNC: IRREGULARITY: Unable to unpack" \
	    		"$DISPHOST:$DIR/$INSTCTRL"
	    eval `_U_Prompt "Do you wish to retry, continue, or quit" \
			RESP "r retry c continue q quit" "retry" "

		The possible actions are:

		    retry  will cause the script to attempt to unpack
		           the files again.  Assuming this or subsequent
			   retries succeeds, then the irregularity will
			   not affect the kit being built.

		    quit   will cause the script to exit.

		    continue will cause the script to ignore the error.
		             You should only do this if you are the
		             engineer supporting this script.

		Usually, this error occurs because the network has aborted
		a network file copy, a retry usually fixes the
		problem.  If this is NOT the case, then report
		this problem to the maintainer immediately."`

	    case $RESP in
		q*)
		    echo 1>&2 "Kit build halted due to irregularity."
		    exit 1
		    ;;

		r*) echo 1>&2 "Trying unpack operation again."
		    continue
		    ;;

		c*)
		    echo 1>&2 "Kit build continuing.  Expect problems."
		    break
		    ;;
	    esac
	    }

	    break	# success, exit while loop
	done # with while
    done # with for

    #
    # put the DIRELIST out to the real standard
    # output so that the caller can get the value.
    #
    echo DIRELIST='"'$DIRELIST'"'
)}
##### end _K_Unpack_Instctrls

#
#   _K_Image_Merge(newimagename, subsetlist)
#	Create a merged image file from the component image files.
#
#	imagename	name of the new image file w/o ".image" suffix
#	subsetlist	list of subsets to be included in the image file
#
#    Globals referenced: MERGEDIR, WORKDIR
#    No values are output.
#    The file $MERGEDIR/$NEWIMAGENAME.image is created by this procedure.
#
_K_Image_Merge()
{(
    NUMARGS=2 FUNC=_K_Image_Merge
    test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

    #
    # Reject attempt if we do not have 2 arguments
    case "$#" in
	${NUMARGS})	# right number of args, set values
	    NEWIMAGENAME="$1"
	    SUBSETLIST="$2"
	    ;;

	*)		# wrong number of args, error
	    echo 1>&2 "$FUNC($# args): Needs ${NUMARGS} arguments"
	    exit 1
	    ;;
    esac

    echo 1>&2 "Creating a merged image file.\n"

    # remove any old versions that may be lying about
    rm -f $MERGEDIR/INSTCTRL $MERGEDIR/*.image $MERGEDIR/*.comp

    for K in $SUBSETLIST
    do
	case $K in
	    %%*)	# volume changes, skip these
	        continue
		;;

	    *"|"*)	# blocking information, remove.
		#
		# Some subsets have blocking information appended.
		# Strip it off.
		OFS="$IFS"
		IFS="|"
		set $K
		IFS="$OFS"
		case $# in
		    1) ;;
		    *) K=$1 ;;
		esac
		;;
	esac

	#
	# If there is no new image file, then
	# touch the new image file, to create it.
	# If we do not, then the first subset through
	# the loop will have an automatic grep failure.
	#
	test -f $MERGEDIR/$NEWIMAGE.image ||
	    touch $MERGEDIR/$NEWIMAGE.image

	#
	# The ROOT subset is a special case, that does not
	# have a ctrl or comp file, but does have an image entry.
	#
	if [ "$K" != "ROOT" ]
	then	
	    #
	    # if there is no .ctrl file, then skip the subset
	    test -f ${WORKDIR}/${K}.ctrl || continue

	    OLDIMAGE=`expr $K : '\([A-Z0-9][A-Z0-9][A-Z0-9]\)'`
	    VERSCODE=`expr $K : '.*\([0-9][0-9][0-9]\)'`

	    #
	    # if there is not .comp file, then make one
	    #
	    test -f $MERGEDIR/$OLDIMAGE$VERSCODE.comp ||
		touch $MERGEDIR/$OLDIMAGE$VERSCODE.comp
	fi

	#
	# Now, search the image files for each subset and
	# build the new one from the entries found.
	#
	FOUND=0
	for IMAGEFILE in $WORKDIR/*.image
	do
	    SUBSETINFO=`egrep $K $IMAGEFILE` && FOUND=1

	    #
	    # If we found information for $K, then
	    # check that it does not appear more than once
	    # with a different checksum, or size.  If it
	    # does we have a major error.
	    #
	    if [ "$SUBSETINFO" ]
	    then
		DUPINFO=`egrep "$K" $MERGEDIR/$NEWIMAGE.image`

		#
		# If information is already in the file (for some reason)
		# then don't put it in again.
		#
		if [ "$DUPINFO" != "$SUBSETINFO" ]
		then
		    #
		    # If the subset is already in the file (for some
		    # reason) but with *different* information then
		    # we have a major error; if it is not in the
		    # file, put it in.
		    #
		    if [ "$DUPINFO"]
		    then
			echo 1>&2 "?$FUNC: Fatal error. Cannot merge" \
				  "image files."
			echo 1>&2 " Multiple, different entries exist for $K."
			return 1
		    else
			echo "$SUBSETINFO" >> $MERGEDIR/$NEWIMAGE.image
		    fi
		fi
	    fi
	done

	if [ $FOUND -eq 0 ]
	then
	     echo 1>&2 "$FUNC: Cannot find image file entry for $K in any of"
	     echo 1>&2 " the following image files: " $WORKDIR/*.image
	     return 1
	fi
    done
)}
	
#
#   _K_Product_Merge(imagename, kitname, subsetlist)
#	Merge multiple products together by modifying the subset
#	control files to reflect the order found in the kitcap file
#	and creating a new INSTCTRL file for the merged kit.
#
#	imagename	name of the merged image file
#	kitname		name to use in K file, null if none.
#	subsetlist	list of subsets that comprise new kit
#
#    Globals referenced: MERGEDIR, WORKDIR
#    No values are output.
#    The file $MERGEDIR/INSTCTRL is created by this procedure.
#
_K_Product_Merge()
{(
    NUMARGS=3 FUNC=_K_Product_Merge
    test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

    #
    # Reject attempt if we do not have 2 arguments
    case "$#" in
	${NUMARGS})	# right number of args, set values
	    IMAGENAME="$1"
	    KITNAME="$2"
	    SUBSETLIST="$3"
	    ;;

	*)		# wrong number of args, error
	    echo 1>&2 "$FUNC($# args): Needs ${NUMARGS} arguments"
	    exit 1
	    ;;
    esac

    #
    # The following are used to calculate file positions on the
    # tape so that correct .ctrl files can be built.
    #
    SIGN="-"		# 1st LOCNUM-1 files are negative
    TAPEVOL=1
    LOCNUM=4
    TAPELOC= 

    echo 1>&2 "Merging subset control files into a new INSTCTRL.\n"

    #
    # Put the new image file into a new INSTCTRL file
    #
    (cd $MERGEDIR && tar c${TARVERIFY}pf INSTCTRL $IMAGENAME.image) || return 1
    NUM_FILES=1

    #
    #    Gather each .comp file to add to the INSTCTRL file 
    #
    MDIRFILES=""
    for K in $MERGEDIR/*.comp
    do
	{
	K=`basename $K 2> /dev/null`
	#
	# If there is no .comp file it is assumed that the
	# subset is not compressed.
	#
	if [ -f $WORKDIR/$K ]
	then
	    MDIRFILES="$MDIRFILES $K"
	fi
	}
    done

    #
    #    Add the .comp file to the INSTCTRL file 
    # 
    if [ "$MDIRFILES" ]
    then
	(cd $WORKDIR &&
	    tar r${TARVERIFY}pf $MERGEDIR/INSTCTRL $MDIRFILES) || return 1
	set xxx $MDIRFILES; shift
	NUM_FILES=`expr $NUM_FILES \+ $#`
    fi
    MDIRFILES=

    #
    # Adjust the tape location of each subset to reflect any merge.
    # Code referencing SIGN is there because expr can't handle negative
    # numbers.
    #
    for K in $SUBSETLIST
    do
	if [ "$SIGN" = "-" ]
	then
	    LOCNUM=`expr $LOCNUM - 1`
	    if [ "$LOCNUM" = "0" ]
	    then
		SIGN=
		TAPELOC="$TAPEVOL:$LOCNUM"
	    else
		TAPELOC="$TAPEVOL:$SIGN$LOCNUM"
	    fi
	else
	    LOCNUM=`expr $LOCNUM + 1`
	    TAPELOC="$TAPEVOL:$LOCNUM"
	fi
	
	#
	# Process volume changes and
	# check to see that we have proper control files
	#
	case $K in
	    %%*)		# new volume, reset position values
		TAPEVOL=`expr $K : '%%\(.*\)'`
		SIGN=-
		LOCNUM=4
		continue
		;;

	    *)			# check for existence of ctrl file
		#
		# SPACE, et. al. subsets may have blocking
		# information appended.  Strip it off.
		OFS="$IFS"
		IFS="|"
		set $K
		IFS="$OFS"
		case $# in
		    1) ;;
		    *) K=$1 ;;
		esac

		#
		# Produce a warning if an important control
		# file is missing
		#
		test -f ${WORKDIR}/${K}.ctrl ||
		    {
		    case $K in
			# ok for these to have no .ctrl file
			SPACE | INSTCTRL | ROOT | *TK50*)
			    ;;

			* )	# other missing files are irregularities.
			    echo 1>&2 "$FUNC: IRREGULARITY: No control file" \
			    	      "for $K."
			    eval `_U_Prompt "Do you wish to continue anyway?" \
			    		RESP "y yes n no" "no" "
			You should only choose to continue if you desire an
			irregular kit.

			If you are not sure what to do, then report this to
			your supervisor immediately as a kit buiding
			problem."`

			    case $RESP in
				n*)
				    echo 1>&2 "Kit build halted due to" \
				    	      "irregularity"
				    exit 1
				    ;;
				y*)
				    echo 1>&2 "Kit build continuing.  Expect" \
				    	 "to see warnings about missing" \
					 "checksum entries."
				    ;;
			    esac
			    ;;
		    esac
		    continue
		    }
		;;
	esac

	#
	# Set the Kit name into the ctrl file.
	# Use the name from the old ctrl file, unless
	# we have a KITNAME to override it.
	#
	# We edit the ctrl file because there may be backquote
	# substitutions that need to be evaluated at setld loading
	# time and not now.
	#
	if [ "$KITNAME" ]
	then
	    sed	-e "/^NAME=/s.*NAME='$KITNAME $K'" \
		    -e "/^MTLOC=/s.*MTLOC=$TAPELOC" \
			    ${WORKDIR}/$K.ctrl > ${LCL_DIR}/ctrl.$$
	else
	    sed	-e "/^MTLOC=/s.*MTLOC=$TAPELOC" \
			    ${WORKDIR}/$K.ctrl > ${LCL_DIR}/ctrl.$$
	fi && 
	    {
	    if test "$UTIL_DEBUG"
	    then
		echo "\noriginal file:" | cat - ${WORKDIR}/$K.ctrl
		echo "\nnew file:" | cat - ${LCL_DIR}/ctrl.$$
	    fi

	    #
	    # Replace the old file with the edited version
	    #
	    mv ${LCL_DIR}/ctrl.$$ ${WORKDIR}/$K.ctrl
	    }

	#
	# Add all files for this subset to INSTCTRL
	#
	(cd $WORKDIR; tar r${TARVERIFY}pf $MERGEDIR/INSTCTRL $K.*)
	set xxx $K.*; shift
	NUM_FILES=`expr $NUM_FILES \+ $#`
    done

    #
    # This code checks for a tar bug that haunted 4.0FT1 ULTRIX tar
    # on RISC machines.  Leave it here in case the problem someday returns.
    #
    echo 1>&2 "\nChecking integrity of new INSTCTRL file. . .\c"
    set xxx `tar tf $MERGEDIR/INSTCTRL`; shift
    if [ $NUM_FILES -eq $# ]
    then
	echo "OK"
    else
	cat <<- -KPM-ERROR | fmt 1>&2


	?ERROR: $NUM_FILES files were put into INSTCTRL, but only $#
	 can be extracted.  This usually indicates that the
	 tar(1) command is improperly appending files to the
	 archive.

	 If you are a FT customer using ORDS, then your software
	 host should have a replacement version of tar(1) that fixes
	 the problem.  Consult your registration package for
	 details on how to obtain it.

	 If you are not a FT customer, then contact your system
	 administrator immediately regarding how to get a valid
	 version of tar onto your system.

	 When the tar command is fixed, run this script again.

	-KPM-ERROR
	return 1
    fi

    echo 1>&2 "\nMerge process for $KITCODE complete. \n\n"
    return 0
)}
# end _K_Product_Merge()

