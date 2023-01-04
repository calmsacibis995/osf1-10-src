:	-GenUtil
#		general utility routines.
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
# @(#)$RCSfile: GenUtil.sh,v $ $Revision: 4.1.3.2 $ (DEC) $Date: 1992/01/29 14:39:41 $ 
# 
# Modification History
# ~~~~~~~~~~~~~~~~~~~~
# 10	May-91, ech
#	Ported to OSF/1.
#
# 09	Sep-90, Greg Tarsa
#	Finally got a test site for hidden node testing.  The feature should
#	be fixed.
#
# 08	Jun-90, Greg Tarsa
#	Finally got a fix that should allow hidden nodes to build DECnet
#	kits.  The file name has to be enclosed in double quotes, which
#	means that the quoted file name must appear in single quotes so that
#	the shell doesn't remove the double quotes during normal command
#	processing.  Currently untested because we have no known hidden
#	node sites at this time.
#
# 07	Jan-90, Greg Tarsa
#	Fixed bug that occurs when using "file /dev/rmt0h | grep TK50"
#	in a pipe.  Problem occurred when grep terminated the pipe before
#	"file" had a chance to completely close the tape.  The script continued
#	and later mt commands failed.
#	Fixed bug in _U_Dir_Exists where local case had conditions
#	reversed.
#	Changed code in dir_exists, file_exists and is_space to be
#	more reasonable in their return semantics.
#	Added code so that TCP addresses could be specified as
#	address.account and that rsh's would be changed accordingly.
#	"Fixed" rcp to return proper status.
#	Adjusted retry count in _U_Remote_Copy to be fewer for TCP.
#	Added check to U_Check_Volume to alert the user if a regular file
#	is specified as a kit device.
#
# 06	Nov-89, Greg Tarsa
#	Added Check Volume function.
#	Added special quotations to all DECnet calls so that hidden
#	node accesses might work.
#	Added retry code to U_Remote_Copy.
#	Added proper error handling to all functions.
#
# 05	Oct-89, Greg Tarsa
#	Fixed long running problem where rsh calls were assumed to return
#	the status of the last remote command executes, but actually
#	always returned true.
#	Changed dd calls in Remote dd function to return status.
#	Fixed bug in Find Dir function with TCP code.
#	Added Get Dnet Password code.
#	Added some sleep time for the retries for dls.
#	Added code for more reliable dls(1).
#
# 04	Aug-89, Greg Tarsa
#	Added code to allow Remote dd function to put output to stdout
#	if required.
#	Added code to handle "hidden area" addresses for DECnet.
#	This code should be removed when DECnet Phase 5 is fully
#	operational at DEC.
#	Added Remote ls function.
#	Added a count to the read check so that an entire device
#	is not read.
#	Added access checking function and generalized tape request
#	function.
#	Fixed major bug in remote_dd that was causing options and
#	input files to be ignored.
#
# 03	Jul-89, Greg Tarsa
#	Changed "test" to "[".
#	Added machine typing function.
#	Added OPTIONS processing to Prompt function.
#	Added new diskspace function.
#	Simplified printing of info to stderr.
#	Added -n to all invocations of rsh.
#	Removed $0. from preface of error messages.
#	Fixed bugs in DECnet processing.
#
# 02	Jun-89, Greg Tarsa
#	Added code to keep from including file twice
#	Changed all "exit"s to "return"; 
#	Added _U_Get_Hostname routine
#	Standardized the preface to every function
#	Added _U_Find_Dir routine
#	Fixed *_Exists to return "false" when not found.  Fixed bugs,
#	enhanced tracing
#
# 01	9-Jun-1989, Greg Tarsa
#	Wrote Original Routines
#####
#	What follows are a number of Utility routines for
#	systems with multiple networking systems.
#
#	When present in these routines, the contents of the "host" 
#	argument determines the method of access as follows:
#
#	    host is NULL, "local" or `hostname`	local access
#	    host ends with colon (:)		DECnet access
#	    all other cases			rsh access
#
#	All script-generated error messages will contain the calling
#	script name followed by the function name followed by the text
#	of the error message, all written to standard output, unless
#	otherwise indicated.
#
#	Environment variables of interest:
#	    UTIL_DEBUG	when non-null, it causes the arglists of each
#	    function to be placed on stderr prior to function execution.
#
#	Notes:
#	DECnet is not yet implemented
#
#	These routines have READONLY access to the caller's symbol table.
#
#	Calling any of these routines will destroy
#	any "set" arguments in the caller.
#
#	mt(args)
#		Implements mt command but sleeps 10 seconds before executing
#		the operation so that SCSI tapes can settle down.
#
#	dls(args)
#		Implements a more reliable dls(1decnet).  Will perform
#		the specified dls and, upon failure, examine the error
#		output for "Connect failed, no response from object", and
#		if it sees it, the command will be reexecuted a fixed
#		number of times before giving up.
#
#       _U_Host_for_rsh(rawhost)
#		Take a rawhost, possibly of the form "host.username" and
#		return the host and username as "host -l username", if a
#		username if found. The return is an assignment to
#		RSH_HOST, that is written out to stdout.
#
#	_U_Get_Hostname(arg)
#	        extract a TCP or DECnet host from an argument
#		returning the host portion as a HOST assignment and
#		the remainder as a REMAINDER assignment.
#
#	_U_Get_Dnet_Acc(host, defuser, defpwd)
#		check the HOST specification for username and password and if
#		there is none, then prompt for it using DEFUSER and DEFPWD
#		as defaults.  Returns an assignment for HOST which
#		is the DECnet hostname with valid access control appended.
#
#		host must be in internal format, that is, single colon at
#		end for DECnet, else assumed TCP/IP.
#
#	_U_Dnet_to_Hidden(host)
#		Takes a full hidden DECnet host specification
#		(router::host/acct/pwd::) and converts it into a valid
#		DECnet sequence so that it will work properly with
# 		DECnet commands.  Writes the transformation to standard
#		output.
#
#	_U_Machine_Type
#		echos back "mips", "vax", or "unknown" depending
#		upon the machine being run on.
#
#	_U_Dir_Exists(host, dirpath)
#		returns a success status if the $dirpath directory exists
#		on the specified $host, and a failure status if it does not
#		or there are any other errors.
#
#	_U_File_Exists(host, path)
#		returns a success status "true" if the $path file
#		exists on the specified $host and a failure status
#		if it does not or there are errors in the call.
#		Error messages will be printed if it is called with invalid
#		arguments (incl, invalid hostname).
#
#	_U_Is_Space(directory, needed)
#		returns a success status if the DIRECTORY has NEEDED blocks of
#		diskspace available to it, a failure status if it does not or
#		it fails for system reasons.  Error messages are printed for
#		abnormal failures.
#
#	_U_Remote_Tar_xpf(host, tarfile)
#		Perform a "tar xpf" on $tarfile, even if $host
#		is a remote node.
#
#	_U_Remote_Copy(host, infile, outfile)
#		copies the $infile to the $outfile, 
#		even if $host is a remote node.
#
#	_U_Remote_dd(host, infile, outfile, options)
#		perform the specified dd(1) command using the
#		specified options on the infile residing on host,
#		putting the results to outfile, even if host is
#		a remote node.
#
#	_U_Remote_ls(host, filepat)
#		perform a simple ls(1) command on HOST for FILEPAT.
#		
#	_U_Find_Dir(host, file, dirlist)
#		Searches the dirlist for the specified file, returning
#		the file as an assignment to FILEDIR=.
#
#	_U_Prompt(prompt, item, options, default, helpmsg)
#		Prompts for input into ITEM and implements a standard
#		defaulting and help mechanism.
#
#	_U_Request_Dev_Load(dev, type, prompt, access)
#	        Prompt for the mounting of volume VOL and rewind
#		the volume if DEV is tape to insure BOT then verify ACCESS
#		which should be one of "read" or "write".
#
#	_U_Check_Device(dev, access)
#	        Check that the specified DEV can be accessed with
#		with specified ACCESS, which should be one of "read"
#		or "write".
#
#	_U_Check_Volume(dev, access[, voltype])
#		Check the specified DEVICE to see if we have the specified
#		access and, if it is a tape, that it is the non-rewinding
#		variety and is rewound to BOT.  The VOLTYPE is used
#		in the prompt "Please mount a valie VOLTYPE volume. . ."
#
#####

#
# Set the "already included" flag.
# this flag can be tested to see if this module has been loaded already.
#
 _GenUtil_included="yes"	

#
#
#
# dls(args)
#
#    attempt to implement a more reliable dls(1decnet), that is, a dls
#    that will detect timeout problems due to "no response" connection
#    failures and retry the directory listing attempt a fixed number
#    of times before giving up.
#
#	args	arbitrary dls arguments
#
dls()
{(
FUNC=dls
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

TIMEOUT_MSG="Connect failed, no response from object"
ERR=/usr/tmp/dlserr$$
OUT=/usr/tmp/dlsout$$
MAXCOUNT=20
COUNT=0

while : true
do
    #
    # do the dls command, save status,
    # save stdin and stdout in files
    # a dls command with more than 9 arguments will likely fail.
    # However, the arguments must be quotes, or else hidden node names
    # will fail to work properly.
    #
    /usr/bin/dls "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9" 2> $ERR > $OUT
    STATUS=$?

    if [ $UTIL_DEBUG ]
    then
	echo "DEBUG: Status: $STATUS; Retry is: $COUNT of $MAXCOUNT" > /dev/tty
	echo "DEBUG: ERR=`cat $ERR`" > /dev/tty
	echo "DEBUG: OUT=`cat $OUT`" > /dev/tty
    fi

    #
    # If successful, then we are done
    #
    if [ $STATUS -eq 0 ]
    then
	break
    fi

    #
    # If it is a failure, then we check to see if it might
    # have been a timeout on the part of the remote (local)FAL.
    # If it was then we iterate MAXCOUNT times before giving up.
    #
    if grep -s "$TIMEOUT_MSG" $ERR
    then
	COUNT=`expr $COUNT \+ 1`

	# 1st: rest a bit 
	SLEEPTIME=`expr  $COUNT \* 5`
	sleep $SLEEPTIME

	#
	# If we meet or exceed MAXCOUNT, then give up.
	#
	if [ $COUNT -ge $MAXCOUNT ]
	then
	    break
	fi
    else
	break
    fi
done

#
# Now put the error and/or output onto stdin/stdout, where it
# belongs.
#
cat $OUT
cat 1>&2 $ERR
rm -f $ERR $OUT

return $STATUS
)}
###### End dls

#
# _U_Host_for_rsh(rawhost)
#    Take a rawhost, possibly of the form "host.username" and return
#    the host and username as "host -l username", if a username if found.
#    The return is an assignment to RSH_HOST, that is written out to stdout.
#
_U_Host_for_rsh()
{(
FUNC=_U_Host_for_rsh NUMARGS=1
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

#
# Reject attempt if we do not have a single argument
case "$#" in
    ${NUMARGS})	# right number of args, continue
	RAWHOST="$1"
	;;

    *)		# wrong number of args, error
	echo 1>&2 "$FUNC($# args): Needs ${NUMARGS} argument"
	return 1
	;;
esac

case $RAWHOST in
    *.*)
	OFS="$IFS"
	IFS="."
	set xxx $RAWHOST; shift
	RSH_HOST="$1 -l $2"
	IFS="$OFS"
	;;    
    *)
	RSH_HOST=$RAWHOST
	;;
esac

echo RSH_HOST='"'"$RSH_HOST"'"'
return 0
)}
# End _U_Host_for_rsh

#
# _U_Get_Hostname(arg)
#
#    Extract a TCP or DECnet hostname from an argument returning
#    the hostname in a HOST assignment and the remaining text in
#    a REMAINDER assignment.  A DECnet host is designated with a trailing
#    colon in the HOST name.  "local" is returned if no hostname is
#    found.
#
#	arg	arbitrary text
#
_U_Get_Hostname()
{(
FUNC=_U_Get_Hostname NUMARGS=1
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

#
# Reject attempt if we do not have a single argument
case "$#" in
    ${NUMARGS})	# right number of args, continue
	ARG="$1"
	;;

    *)		# wrong number of args, error
	echo 1>&2 "$FUNC($# args): Needs ${NUMARGS} argument"
	return 1
	;;
esac

HOST="local"	#default HOST is "local"

#
# if :: is in the name we're DECnet--append a colon
# to the final result.
#
DNET_SUFFIX=""
case "$ARG" in
    *::*)
	DNET_SUFFIX=":"
	;;
esac

# save the device code; extract hostname, if there is one
OFS="$IFS"
IFS=:
set xxx $ARG
shift
IFS="$OFS"

# 2 fields indicates that $1 is host
# 3 fields indicates that $1 is DNET gateway to machine $2 in hidden area
case "$#" in
    2)
	# get host name and shift to make remainder arg 1
	HOST=$1$DNET_SUFFIX
	shift
	;;

    # Remove this code when DECnet phase IV is fully operational
    # and hidden areas are no longer needed.
    3)	# better be a DECnet node with a gateway
	if [ "$DNET_SUFFIX" ]
	then
	    # get router *and* host name and shift to make remainder arg 1
	    HOST=$1::$2$DNET_SUFFIX
	    shift
	    shift
	else
	    echo 1>&2 "?Only DECnet node names can contain a gateway-- $ARG"
	    return 1
	fi
	;;
esac

echo HOST='"'"$HOST"'"'
echo REMAINDER='"'"$1"'"'
)}
###### End _U_Get_Hostname
# _U_Get_Dnet_Acc(host, defuser, defpwd)
#	check the HOST specification for username and password and if
#	there is none, then prompt for it using DEFUSER and DEFPWD
#	as defaults.  Returns an assignment for HOST which
#	is the DECnet hostname with valid access control appended.
#
#	Note: the input dnet_host is assumed to be TCP/IP if there is
#	no trailing colon.  For DECnet, there should be only one trailing
#	colon.
#
#	arg	DECnet host name, perhaps with some access control
#		information.
#
_U_Get_Dnet_Acc()
{(
FUNC=_U_Get_Dnet_Acc NUMARGS=3
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

#
# Reject attempt if we do not have a single argument
case "$#" in
    ${NUMARGS})	# right number of args, continue
	HOST="$1"
	DEFUSER="$2"
	DEFPWD="$3"
	;;

    *)		# wrong number of args, error
	echo 1>&2 "$FUNC($# args): Needs ${NUMARGS} argument"
	return 1
	;;
esac

#
# if :: is in the name, then remove it and continue.
# Otherwise, we are done; return it and exit successfully.
#
if echo "$HOST" | grep -s ':$'
then
    HOST=`expr $HOST : '\(.*\):$'`
else
    echo HOST='"'$HOST'"'
    return 0
fi

#
# Now, check for access control information
#

OFS="$IFS"
IFS="/"
set xxx $HOST
shift
IFS="$OFS"

# $1 should be host, $2 should be username $3 should be password
case "$#" in
    0)
	echo 1>&2 "?${FUNC}: null DECnet name found."
	return 1
	;;

    *)
	RAWHOST="$1"
	USERNAME="$2"
	PASSWORD="$3"
	;;
esac

if [ ! "$USERNAME" ]
then
    eval `_U_Prompt "Username to use on Software Host $RAWHOST" USERNAME \
    		"" "$DEFUSER" "
	The Software Host Username is the name of the login account on
	the remote software host where the kit data is located.

	Usually the default is the correct value for this account.  In any
	case your registration document should contain the proper user name.
	"`
fi

if [ ! "$PASSWORD" ]
then
    #
    # Technically we should use _U_Prompt for this, but it does not
    # have the capability of input without echoing.
    #
    if [ "$DEFPWD" ]
    then
	DEFAULT="$DEFPWD"
    else
	DEFAULT="no default"
    fi
    while : true
    do
	stty -echo > /dev/tty
	echo "Password for $RAWHOST/${USERNAME}:: [$DEFAULT] ?\c" > /dev/tty
	read password < /dev/tty; echo > /dev/tty
	stty echo > /dev/tty

	if [ "$password" ]
	then
	    if [ "$password" = "NONE" ]
	    then
		echo 1>&2 "Setting null password by request."
		password=
	    fi

	    PASSWORD="$password"
	    break
        else
	    if [ "$DEFPWD" ]
	    then
		PASSWORD="$DEFPWD"
		break
	    else
		echo 1>&2 "\n?There is no default password. " \
			  "Please type 'NONE'"
		echo 1>&2 " if you wish to specify a null password.\n"
	    fi
	fi
    done
fi

#
# Now, build the full decnet specification in internal format (i.e., one
# trailing colon.
#
HOST="${RAWHOST}/${USERNAME}/${PASSWORD}:"

echo HOST='"'"$HOST"'"'
)}
###### End _U_Get_Dnet_Acc
# _U_Dnet_to_Hidden(host)
#	Takes a full hidden DECnet host specification
#	(router::host/acct/pwd::) and converts it into a valid
#	DECnet sequence so that it will work properly with
# 	DECnet commands.  Writes the transformation to standard
#	output.
#
#	If no router is found, then the name passed is the name returned.
#
#	host	full DECnet node name, with router.
#
_U_Dnet_to_Hidden()
{(
FUNC=_U_Dnet_to_Hidden NUMARGS=1
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

#
# Reject attempt if we do not have a single argument
case "$#" in
    ${NUMARGS})	# right number of args, continue
	HOST="$1"
	;;

    *)		# wrong number of args, error
	echo 1>&2 "$FUNC($# args): Needs ${NUMARGS} argument"
	return 1
	;;
esac

#
# If we are not a hidden node, then return the name as passed.
#
case "$HOST" in
    *::*::)
	;;
    *)
	echo "$HOST"
	return 0
	;;
esac

#
# At this point we will assume that we have a name of the form:
#	ROUTER::NODE/acct/password::
# and work to convert it to:
#	ROUTER::'NODE"acct password"::'
#
HOST=`echo "$HOST" |
    sed 's=\([^:]*\)::\([^/]*\)/\([^/]*\)/\([^:]*\):.*=\1::\2"\3 \4"::='`

echo "$HOST"
)}
###### End _U_Dnet_to_Hidden

#
# _U_Machine_Type
#
#    echos back "mips", "vax", or "unknown" depending
#    upon the machine being run on.
#
_U_Machine_Type()
{(
FUNC=_U_Machine_Type NUMARGS=0
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

#
# Reject attempt if we do not have a single argument
case "$#" in
    ${NUMARGS})	# right number of args, continue
	;;

    *)		# wrong number of args, error
	echo 1>&2 "$FUNC($# args): Needs ${NUMARGS} arguments"
	return 1
	;;
esac

if [ -f /bin/machine ]
then
    /bin/machine
else
    #
    # strange as it may seem, the TSC machine has a /bin/vax
    # that returns a false status when called and a *real* vax
    # has no means of identifying it.  Hence, if /bin/vax
    # exists, then we are *not* a VAX.
    #
    if [ -f /bin/vax ]
    then
	echo "unknown"
    else
	echo "vax"
    fi
fi
)}
###### End _U_Machine_Type

#
# _U_Dir_Exists(host, dirpath)
#    Retuens a success status if the directory exists, and a failure
#    status if the directory does not exist, is inaccessible, or is
#    not a directory, or if any other failures occur.
#
#	host	is the remote host, as previously defined.
#	dirpath	is the path to the directory on the remote host.	
#
_U_Dir_Exists()
{(
FUNC=_U_Dir_Exists NUMARGS=2
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

HOST="$1"
DIRPATH="$2"
#
# Only attempt the check if we have 2 arguments
case "$#" in
    ${NUMARGS})
        case $HOST in
	    #
	    # local node, just use test
	    local|`hostname`)
		if [ -d $DIRPATH ]
		then
		    return 0
		else
		    return 1
		fi
		;;

	    #
	    # DECnet nodes, search for the "." file
	    # in the alleged directory.  use status from dls
	    # to indicate success.
	    *:)
	    # set name according to whether it is a hidden file or not
	    case $HOST: in
	        *::*::*)		# hidden node
		    FNAME=`_U_Dnet_to_Hidden $HOST:`\"$DIRPATH/.\"
		    ;;
		*)			# ordinary node
		    FNAME=$HOST:$DIRPATH/.
		    ;;
	    esac		    
	    dls "$FNAME" > /dev/null 2>&1
	    #
	    # dls is strange in that it returns an error code if
	    # any file it tries to stat cannot be accessed, particularly
	    # if symbolic links cannot be followed.  66 is definitely
	    # "file or directory not found" so we will assume that any
	    # other status message indicates a problem within the directory
	    # so that we can assume that we *are* a directory in those cases.
	    case $? in
		66)
		    return 1
		    ;;
		*)
		    return 0
		    ;;
	    esac
	    ;;

	    #
	    # TCP nodes, use remote test.  Note that rsh(1) does not
	    # return a proper status.
	    #
	    *)
		RSH_HOST=
	        eval `_U_Host_for_rsh $HOST`

	        isdir=`rsh $RSH_HOST -n "test -d $DIRPATH || echo 'no'"`dir
		if [ "$isdir" = "dir" ]
		then
		    return 0
		else
		    return 1
		fi
		;;
	esac
	;;
    *)
	echo 1>&2 "$FUNC($# args): Must have exactly ${NUMARGS} arguments"
	return 1
	;;
esac
)}
##### End _U_Dir_Exists

#
# _U_File_Exists(host, path)
#    returns success if the PATH file exists and failure
#    if the file does not exist or is inaccessible.
#    A failure status is returned and a error message is put
#    to standard error if calling arguments are invalid.
#
#	host	is the name of the remote host, as defined above.
#	path	is the path to the file on the remote host.	
#
#
_U_File_Exists()
{(
FUNC=_U_File_Exists NUMARGS=2
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"
#
# Only attempt the check if we have 2 arguments
HOST="$1"
FPATH="$2"
case "$#" in
    ${NUMARGS})
	case $HOST in
	    #
	    # local node, just use test
	    local|`hostname`)
		if [ -f $FPATH -a -r $FPATH ]
		then
		    return 0
		else
		    return 1
		fi
		;;

	    #
	    # DECnet nodes, use status from dls to determine
	    # existence/non-existence.  Check output for leading
	    # 'd' to determine directory/file-ness.
	    #
	    # no check is currently made for readability.
	    *:)
		# set name according to whether it is a hidden file or not
		case $HOST: in
		    *::*::*)		# hidden node
			FNAME=`_U_Dnet_to_Hidden $HOST:`\"$FPATH\"
			;;
		    *)			# ordinary node
			FNAME=$HOST:$FPATH
			;;
		esac		    

	        if dls -l "$FNAME" > /tmp/FEdls$$ 2> /dev/null
		then
		    case `cat /tmp/FEdls$$` in
		        d) RETURN=1 ;;
			*) RETURN=0 ;;
		    esac
		    rm /tmp/FEdls$$
		    return $RETURN
		else
		    return 1
		fi
		;;

	    #
	    # TCP nodes, remote execution of test; note that rsh(1) does
	    # not return proper exit status.
	    #
	    *)  # TCP nodes
		RSH_HOST=
	        eval `_U_Host_for_rsh $HOST`
	        is_rfile=`rsh $RSH_HOST -n \
			"test -f $FPATH -a -r $FPATH || echo 'no'"`good

		if [ "$is_rfile" = "good" ]
		then
		    return 0
		else
		    return 1
		fi
		;;
	esac
	;;
    *)
	echo 1>&2 "$FUNC($# args): Must have exactly ${NUMARGS} arguments"
	return 1
	;;
esac
)}
##### End _U_File_Exists

# _U_Is_Space(directory, needed)
#    returns success status if the DIRECTORY has NEEDED blocks of
#    diskspace available to it, a failure status if it does not and
#    both failure and error messages if it is called with invalid arguments
#    or any commands in the function fail.
#
#    The text this routine prints is *always* to standard output,
#    even when they are error messages.
#
#	directory	is the directory that needs the space
#	needed		is the amount of space needed in blocks
#
_U_Is_Space()
{(
FUNC=_U_Is_Space NUMARGS=2
test $UTIL_DEBUG && echo 1>&2 "$DEBUG: $FUNC($*)"

#
# Only attempt the check if we have 2 arguments
case "$#" in
    ${NUMARGS})
	DIRECTORY="$1"
	NEEDED="$2"
	;;
    *)
	echo 1>&2 "$FUNC($# args): Must have exactly ${NUMARGS} arguments"
	return 1
	;;
esac

#
# The last line of a df call contains the diskfree data for the file system
# The fourth field is the number of blocks free
#
set xxx `df $DIRECTORY | tail -1`; shift
if [ "$#" -ge 4 ]
then
    if [ "$4" -ge "$NEEDED" ]
    then
	return 0
    else
	return 1
    fi
else
    echo 1>&2 "$FUNC: strange number of arguments from df call: $*"
    return 0
fi    
)}
##### End _U_Is_Space

#
# _U_Remote_Tar_xpf(host, tarfile)
#    extracts files from the specified tar file, possibly remote,
#    and puts them in the current directory taking the file modes
#    from the tape.
#
#	host	is the name of the remote host, as defined above.
#	tarfile	is a tar archive, see tar(1).
#
_U_Remote_Tar_xpf()
{(
FUNC=_U_Remote_Tar_xpf NUMARGS=2
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

ERRFILE=RTXerr$$
#
# Only attempt the extraction if we have 2 arguments
HOST="$1"
TARFILE="$2"
case "$#" in
    ${NUMARGS})	case $HOST in
		#
		# local node, just use local tar
		local|`hostname`)
		    tar x${TARVERIFY}pf $TARFILE
		    return $?
		    ;;

		#
		# DECnet nodes
		*:)
		    # set name according to whether it is a hidden file or not
		    case $HOST: in
			*::*::*)
			    FNAME=`_U_Dnet_to_Hidden $HOST:`\"$TARFILE\"
			    ;;
			*)
			    FNAME=$HOST:$TARFILE
			    ;;
		    esac		    

		    dcp -i "$FNAME" - 2> $ERRFILE | tar x${TARVERIFY}pf -
		    #
		    # In sh, the status of a pipe is the status of
		    # the last command, in this case a dd(1) command
		    # that always succeeds.  So to determine the
		    # results of the copy, we examine the error file. 
		    # If there is anything in it, we have an error.
		    #
		    if [ -s $ERRFILE ]
		    then
			cat 1>&2 $ERRFILE
			rm $ERRFILE
			return 1
		    else
			rm $ERRFILE
			return 0
		    fi
		    ;;

		#
		# TCP nodes
		*)
		    RSH_HOST=
		    eval `_U_Host_for_rsh $HOST`

		    rsh $RSH_HOST -n "cat $TARFILE" 2> $ERRFILE | \
					    tar x${TARVERIFY}pf -
		    if [ -s $ERRFILE ]
		    then
			cat 1>&2 $ERRFILE
			rm $ERRFILE
			return 1
		    else
			rm $ERRFILE
			return 0
		    fi
		    ;;
	esac
	;;
    *)
	echo 1>&2 "$FUNC($# args): Must have exactly ${NUMARGS} arguments"
	return 1
	;;
esac
)}
##### End _U_Remote_Tar_xpf

#
# _U_Remote_Copy(host, infile, outfile)
#    Copies infile from $host to outfile, using the network
#    if necessary.
#    
#	host	is the name of the remote host, as defined above.
#	infile	is the path to the input file on $host
#	outfile	is the path to the output file on $host
#
_U_Remote_Copy()
{(
FUNC=_U_Remote_Copy NUMARGS=3
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"
COUNT=0
ERRFILE=/tmp/REMCPYerr_$$
#
# Only attempt the copy if we have 3 arguments
case "$#" in
    ${NUMARGS})
        case $1 in
	    #
	    # local node, just use cp, no retry
	    local|`hostname`)
		HOST="$1"
		INFILE="$2"
		OUTFILE="$3"
		cp $INFILE $OUTFILE
		return $?
		;;

	    *)	# dcp and rcp use same retry code.
		# try the copy a few times before giving up
		HOST="$1"
		INFILE="$2"
		OUTFILE="$3"
		while : true
		do
		    case $HOST in
			#
			# DECnet nodes
			*:)
			    MAXCOUNT=5	# large count for DECnet

			    # set name according to whether it is
			    # a hidden file or not
			    case $HOST: in
				*::*::*)
				    FNAME=`_U_Dnet_to_Hidden $HOST:`\"$INFILE\"
				    ;;
				*)
				    FNAME=$HOST:$INFILE
				    ;;
			    esac		    

			    dcp -i "$FNAME" $OUTFILE 2> $ERRFILE
			    STATUS=$?
			    #
			    # Special case: if we have a password failure,
			    # then don't bother retrying.
			    # Doubtless there are other obvious cases, but
			    # this is the most common.
			    if grep -s \
			    	    'failed, access control rejected' $ERRFILE
			    then
				COUNT=$MAXCOUNT
			    fi
			    ;;
			#
			# TCP nodes
			*)
			    HOST="$1"
			    INFILE="$2"
			    OUTFILE="$3"

			    MAXCOUNT=2	# small count for TCP
			    rcp $HOST:$INFILE $OUTFILE 2> $ERRFILE 1>&2

			    #
			    # rcp has the brain-damage that it does not
			    # return valid status and writes some, if not
			    # all of its error messages to stdout.
			    # So, we write all output of rcp to a file
			    # if the file is not empty, we declare an
			    # error.
			    if [ -s $ERRFILE ]
			    then
			        STATUS=1
			    else
			    	STATUS=0
			    fi
			    ;;
		    esac

		    if [ $STATUS -eq 0 ]
		    then
			rm -f $ERRFILE
			return 0	# Success, return status
		    fi

		    #
		    # Failures get here.
		    #

		    # increment count
		    COUNT=`expr $COUNT \+ 1`

		    # If we are at MAX, return current status
		    if [ $COUNT -ge $MAXCOUNT ]
		    then
		    	# print only one of each consecutive failure
			/usr/bin/uniq 1>&2 $ERRFILE
			rm -f $ERRFILE
			return $STATUS
		    fi

		    # rest a bit before trying again
		    sleep `expr $COUNT \* 5`
		done
		;;
	esac
	;;
    *)
	 echo 1>&2 "$FUNC($# args): Must have exactly ${NUMARGS} arguments"
	 return 1
	;;
esac
)}
##### End _U_Remote_Copy

# _U_Remote_dd(host, infile, outfile, options)
#    perform the specified dd(1) command using the
#    specified options on the infile residing on host,
#    putting the results to outfile, even if host is
#    a remote node.
#
#	host	is the name of the remote host, as defined above.
#	infile	is the path to the input file on $host
#	outfile	is the path to the output file on $host
#	options	is the list of dd options to use for xfer.
#
#	If OUTFILE is specified as a null string (""), then output
#	will be directed to standard output.
#
_U_Remote_dd()
{(
FUNC=_U_Remote_dd NUMARGS=4
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"
#
# Only attempt the copy if we have 4 arguments
case "$#" in
    ${NUMARGS})
	HOST="$1"
	INFILE="$2"
	OUTFILE="$3"
	OPTIONS="$4"
	;;

    *)
	 echo 1>&2 "$FUNC($# args): Must have exactly ${NUMARGS} arguments"
	 return 1
	;;
esac

case $OUTFILE in
    "")			# let output go to default stdin
	OUTPARAM=""
	;;
    *)			# put output to output parameter
	OUTPARAM="of=$OUTFILE"
	;;
esac

ERRFILE=/tmp/error$$

case $HOST in
    #
    # local node, just use cp
    local|`hostname`)
	dd if=$INFILE $OUTPARAM $OPTIONS
	return $?
	;;

    #
    # DECnet nodes
    *:)
	# set name according to whether it is a hidden file or not
	case $HOST: in
	    *::*::*)
		FNAME=`_U_Dnet_to_Hidden $HOST:`\"$INFILE\"
		;;
	    *)
		FNAME=$HOST:$INFILE
		;;
	esac		    

	dcp -i "$FNAME" - 2> $ERRFILE | dd $OUTPARAM $OPTIONS
	#
	# In sh, the status of a pipe is the status of the last command,
	# in this case a dd(1) command that always succeeds.  So to determine
	# the results of the copy, we examine the error file.  If there is
	# anything in it, we have an error.
	#
	if [ -s $ERRFILE ]
	then
	    cat 1>&2 $ERRFILE
	    rm $ERRFILE
	    return 1
	else
	    rm $ERRFILE
	    return 0
	fi
	;;

    #
    # TCP nodes, note that ibs is ignored in OPTIONS
    *)
	RSH_HOST=
	eval `_U_Host_for_rsh $HOST`

	rsh $RSH_HOST -n dd if=$INFILE ibs=10k obs=2k 2> $ERRFILE |
		dd $OUTPARAM $OPTIONS
	#
	# If the dd error output has a colon in it, then we have an
	# error message.  It is a kludge, but since rsh returns no
	# status we are stuck with it.
	#
	if grep -s ":" $ERRFILE
	then
	    cat 1>&2 $ERRFILE
	    rm $ERRFILE
	    return 1
	else
	    rm $ERRFILE
	    return 0
	fi

	;;
esac
)}
##### End _U_Remote_dd
#
# _U_Remote_ls(host, filepat)
#	perform a simple ls(1) command on HOST for FILEPAT.
#
#	host	is the name of the remote host, as defined above.
#	filepat	is the directory path/file pattern to use as ls arg.
#
_U_Remote_ls()
{(
FUNC=_U_Remote_ls
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"
#
# Only attempt the copy if we have the right number of arguments
case "$#" in
    1|2)
	HOST="$1"
	FILEPAT="$2"
	;;

    *)
	 echo 1>&2 "$FUNC($# args): Must have 1 or 2 arguments" \
	 	' (2nd can be specified as "", if null desired)'
	 return 1
	;;
esac

case $HOST in
    #
    # local node, just use cp
    local|`hostname`)
	ls "$FILEPAT"
	;;

    #
    # dls always prefaces the file names with the directory
    # name, so we edit it out.
    *:)
	# set name according to whether it is a hidden file or not
	case $HOST: in
	    *::*::*)
		FNAME=`_U_Dnet_to_Hidden $HOST:`\""$FILEPAT"\"
		;;
	    *)
		FNAME=$HOST:"$FILEPAT"
		;;
	esac		    

	dls "$FNAME" | sed 's=^.*/=='
	;;

    #
    # TCP nodes, note that ibs is ignored in OPTIONS
    *)
	RSH_HOST=
	eval `_U_Host_for_rsh $HOST`

	rsh $RSH_HOST -n ls "$FILEPAT"
	;;
esac
)}
##### End _U_Remote_ls
#
# _U_Find_Dir(host, filename, dirlist)
#    Searches the dirlist for the first occurrance of the specified
#    file or directory.  The directory name is returned as an 
#    assignment to FILEDIR=, if found and will be set to
#    "" if the file is not found.  Search will occur on HOST, if not
#    the local host.
#	
#    host	name of machine where directories reside
#    filename	name of file to look for
#    dirlist	list of directories to search through
#
_U_Find_Dir()
{(
FUNC=_U_Find_Dir NUMARGS=3
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

#
# Reject attempt if we do not have 3 arguments
case "$#" in
    ${NUMARGS})	# right number of args, perform search
        HOST="$1"
	FILENAME="$2"
	DIRLIST="$3"
	;;

    *)		# wrong number of args, error
	echo 1>&2 "$FUNC($# args): Needs ${NUMARGS} arguments"
	return 1
	;;
esac

FILEFOUND=0	# assume not found

#
# check each directory
#
for FILEDIR in $DIRLIST
do
    case $HOST in
	#
	# local node, just use test for file or dir
	local|`hostname`)
	    if [ -f $FILEDIR/$FILENAME -o -d $FILEDIR/$FILENAME ]
	    then
		FILEFOUND=1
		break
	    fi
	    ;;

	#
	# DECnet nodes, use status from dls,
	# no check for readability.
	#
	# Save errors in a file to print out at the end.
	*:)
	    # set name according to whether it is a hidden file or not
	    case $HOST: in
		*::*::*)
		    FNAME=`_U_Dnet_to_Hidden $HOST:`\"$FILEDIR/$FILENAME\"
		    ;;
		*)
		    FNAME=$HOST:$FILEDIR/$FILENAME
		    ;;
	    esac		    

	    if dls -l "$FNAME" > /dev/null 2>> /tmp/fderr$$
	    then
		FILEFOUND=1
		break
	    fi
	    ;;

	#
	# TCP nodes, use rsh with test
	# result is "notfound" if the file is not found, "found" otherwise.
	#
	*)  # TCP nodes
	    RSH_HOST=
	    eval `_U_Host_for_rsh $HOST`

	    result=`rsh $RSH_HOST -n "test -f $FILEDIR/$FILENAME \
	    			-o -d $FILEDIR/$FILENAME || echo 'not'"`found
	    if [ "$result" = "found" ]
	    then
		FILEFOUND=1
		break
	    fi
	    ;;
    esac
done

#
# Return the assignment indicating success
# or failure
#
if [ $FILEFOUND = "1" ]
then
    echo FILEDIR='"'"$FILEDIR"'"'
else
    #
    # If an find directory error file exists, print it to stderr.
    # This can only happen with DECnet failures.
    #
    # We want to report an error message if the directory is not found
    # anywhere.  However, we may have searched many directories and
    # the "not found" message for each search is in the error file.
    # We, therefore use uniq(1) to minimize error output.  For the case
    # where the file was not found in any directory, all lines in the error
    # file are identical and uniq(1) will cause only one to be printed.
    # However in the case where one of the searched directories, presumably
    # the onewhere the file actually existed, timed out, uniq will produce
    # "not found" messages as well as a "Connect failed, no response. . ."
    # message.
    #
    if [ -r /tmp/fderr$$ ]
    then
	uniq 1>&2 /tmp/fderr$$
	rm /tmp/fderr$$
    fi
    echo FILEDIR='""'
fi
)}
##### End _U_Find_Dir

#
# _U_Prompt(prompt, item, options, default, helpmsg)
#    Prints $PROMPT followed by a colon standard error and returns
#    an assignment of that input to $ITEM on standard output.  If
#    the input is a single question mark (?), then $HELPMSG is
#    printed and the $PROMPT is again printed.  If $OPTIONS is
#    not null, then it is printed in parentheses after the prompt.
#    If $DEFAULT is non-null, then it is displayed in square brackets
#    as a suffix to the prompt and is returned as the value of ITEM
#    if no text is typed at the prompt.
#
#    The response must be one of OPTIONS, if it is non-null, or an
#    error is printed prompt is made again.
#
#    prompt	the text of the prompt to print on stdout
#    item	the symbol to contain the user's input
#    options	the list of valid options.
#    default	the default, (or "") to be used if CR is typed
#    helpmsg	the text to be printed if the user types a single ?
#
_U_Prompt()
{(
FUNC=_U_Prompt NUMARGS=5
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

#
# Reject attempt if we do not have 5 arguments
case "$#" in
    ${NUMARGS})	# right number of args, perform search
        PROMPT="$1"
	ITEM="$2"
	OPTIONS="$3"
	DEFAULT="$4"
	HELPMSG="$5"
	;;

    *)		# wrong number of args, error
	echo 1>&2 "$FUNC($# args): Needs ${NUMARGS} arguments"
	exit 1
	;;
esac

#
# Define a display version of the default value
if [ "$DEFAULT" ]
then
    DEFAULT_TEXT="$DEFAULT"
else
    DEFAULT_TEXT="no default"
fi

#
# Define a display version of the OPTIONS value
if [ "$OPTIONS" ]
then
    OPTION_TEXT="($OPTIONS ?)"
else
    OPTION_TEXT=""
fi

while : true
do
    #
    # Print the prompt
    echo 1>&2 "${PROMPT}${OPTION_TEXT}[${DEFAULT_TEXT}]: \c"

    #
    # read the response
    read INPUT

    if [ "$INPUT" = "?" ]
    then
    	#
	# INPUT was Question Mark, print the help message to stderr
	#
	echo "${HELPMSG}\n" > /tmp/help$$
	more 1>&2 /tmp/help$$
	rm /tmp/help$$
    else
	if [ "$INPUT" ]
	then
	    #
	    # INPUT was non-NULL, then we check against
	    # OPTIONS, if specified, or just return resp, if not.
	    #
	    if [ "$OPTIONS" ]
	    then
		for OPT in $OPTIONS
		do
		    if [ "$OPT" = "$INPUT" ]
		    then
			echo $ITEM'="'$INPUT'"'
			break 2
		    fi
		done
		echo 1>&2 "?Invalid option.  Must be one of: $OPTIONS"
		echo 1>&2 " or ? for help"
	    else
		echo $ITEM'="'$INPUT'"'
		break
	    fi
	else
	    #
	    # INPUT was NULL, if DEFAULT exist
	    # then return it, else loop
	    #
	    if [ "$DEFAULT" ]
	    then
		echo $ITEM'="'$DEFAULT'"'
		break
	    else
		echo 1>&2 "%Type '?' for HELP"
	    fi
	fi
    fi
done
)}
##### End _U_Prompt
#
# _U_Request_Device_Load(dev, type, prompt, access)
#    Prompt for the mounting of a volume on DEV with the option
#    to continue or quit.  If TYPE=tape, rewind the volume, to insure BOT;
#    Then verify ACCESS, which should be one of "read" or "write".
#	
#    A successful return status from this routine indicates that
#    the user has mounted the volume and it can be rewound.
#    A failure indicates that the user indicated that he wished
#    to quit the procedure.
#
#    dev	device to request volume for
#    type	type of device, either "tape", "disk", or "unknown"
#    prompt	text to print as prompt
#    access	the type of access to check for, either "read" or "write".
#
_U_Request_Device_Load()
{(
FUNC=_U_Request_Device_Load NUMARGS=4
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

#
# Reject attempt if we do not have the right number of arguments
case "$#" in
    ${NUMARGS})	# right number of args, perform search
        DEV="$1"
	TYPE="$2"
	PROMPT="$3"
	ACCESS="$4"
	;;

    *)		# wrong number of args, error
	echo 1>&2 "$FUNC($# args): Needs ${NUMARGS} arguments"
	return 1
	;;
esac

case $TYPE in
    tape | t* | disk | d* | unknown | u* ) ;;
    *)
	echo 1>&2 "?$FUNC: Invalid device type: '$TYPE', must be one of:"
	echo 1>&2 "        disk tape unknown"
	return 1
	;;
esac
    
case $ACCESS in
    read | r* | write | w* ) ;;
    *)
	echo 1>&2 "?$FUNC: Invalid access: '$ACCESS', must be one of:"
	echo 1>&2 "        read write"
	return 1
	;;
esac

#
# Loop until success, or a request to quit
#
while : true
do
    echo 1>&2 "$PROMPT"
    echo 1>&2 "Press RETURN when ready, 'q' to quit: \c"
    read TAPEREADY
    case $TAPEREADY in
	q)
	    return 1
	    ;;

	"")
	    case "$TYPE" in
		t*)
		    # Rewind tape to BOT, loop if we cannot
		    mt -f $DEV rew || continue

		    #
		    # Verify the proper access
		    #
		    if _U_Check_Device $DEV $ACCESS
		    then
			mt -f $DEV rew && break
		    fi
		    ;;

		d* | u* )
		    # Verify the proper access
		    if _U_Check_Device $DEV $ACCESS
		    then
		        break
		    fi
		    ;;
	    esac
	    ;;
	*)
	    echo 1>&2 "\n?Invalid response '$TAPEREADY'.\n"
	    continue
	    ;;
    esac
done
return 0
)}
##### End _U_Request_Tape_Load
# _U_Check_Device(dev, access)
#	Check that the specified DEV can be accessed with
#	with specified ACCESS, which should be one of "read"
#	or "write".
#
#    A successful return status from this routine indicates that
#    the DEVICE can be accessed as requested.
#
#    A failure indicates that the device cannot be accessed, or is
#    somehow invalid or not congifured.
#
#    dev	device to request volume for
#    access	the type of access to check for, either "read" or "write".
#
_U_Check_Device()
{(
FUNC=_U_Check_Device NUMARGS=2
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

#
# Reject attempt if we do not have 2 arguments
case "$#" in
    ${NUMARGS})	# right number of args, perform search
        DEV="$1"
	ACCESS="$2"
	;;

    *)		# wrong number of args, error
	echo 1>&2 "$FUNC($# args): Needs ${NUMARGS} arguments"
	return 1
	;;
esac

case $ACCESS in
    read | r* | write | w* | R* | W*) ;;
    *)
	echo 1>&2 "?$FUNC: Invalid access: '$ACCESS', must be one of:"
	echo 1>&2 "        read write READ WRITE"
	return 1
	;;
esac

#
# Now make sure that we have a volume and
# can read or write it as necessary.
#
case $ACCESS in
    w* | W* )
	#
	# Verify write access by trying to write a block to the device.
	#
	if [ "$UTIL_DEBUG" ]
	then
	    echo "echo 'Writeable?' | dd of=$DEV bs=1b" >/tmp/RTLDD$$
	else
	    echo "	\c" > /tmp/RTLDD$$
	fi

	if echo "Writeable?" | dd of=$DEV bs=1b 2>> /tmp/RTLDD$$
	then
	    rm -f /tmp/RTLDD$$
	    return 0
	else
	    echo 1>&2 "Cannot write to $DEV:"
	    cat 1>&2 /tmp/RTLDD$$; echo 1>&2 " "
	    rm -f /tmp/RTLDD$$
	    return 1
	fi
	;;

    *)
	#
	# Verify read access by trying to read a block from the device.
	#
	if [ "$UTIL_DEBUG" ]
	then
	    echo "dd if=$DEV of=/dev/null bs=1b count=1" > /tmp/RTLDD$$
	else
	    echo "	\c" > /tmp/RTLDD$$
	fi
	if dd if=$DEV of=/dev/null bs=1b count=1 2>> /tmp/RTLDD$$
	then
	    rm -f /tmp/RTLDD$$
	    return 0
	else
	    echo 1>&2 "Cannot read from $DEV:"
	    cat 1>&2 /tmp/RTLDD$$; echo 1>&2 " "
	    rm -f /tmp/RTLDD$$
	    return 1
	fi
	;;
esac

echo 1>&2 "?$FUNC: Unexpected return."
return 1
)}
##### End _U_Check_Device
#
# _U_Check_Volume (device, access)
#    Check the specified DEVICE to see if we have the
#    specified ACCESS and, if it is a tape, that it is the
#    non-rewinding variety and is rewound to BOT.
#
#    It returns an assignment for DEVICE if successful
#    and a failure status (with a message to stderr) otherwise.
#
#    device	the system name of the device (i.e., /dev/rmt0l)
#    access	one of "read" or "write".
#    voltype	optional, the type of volume, for instance, "kit", or "boot".
#		This will be used in the prompt: "Please mount a
#		valid VOLTYPE volume. . ."
#
_U_Check_Volume()
{(
FUNC=_U_Check_Volume
test $UTIL_DEBUG && echo 1>&2 "DEBUG: $FUNC($*)"

#
# Reject attempt if we do not have the right number of arguments
case "$#" in
    2|3)	# right number of args, perform search
        DEVICE="$1"
	ACCESS="$2"
	VOLTYPE="$3"
	;;

    *)		# wrong number of args, error
	echo 1>&2 "$FUNC($# args): Needs 2 or 3 arguments"
	return 1
	;;
esac

case $ACCESS in
    read | r*)
    	VOL_STATE=
	;;

    write | w* )
	VOL_STATE=" write-enabled"
	;;

    *)
	echo 1>&2 "?$FUNC: Invalid access: '$ACCESS', must be one of:"
	echo 1>&2 "        read write"
	return 1
	;;
esac

#
# Check: is the device a special file.  If not, we have an error.
#
# Note: this pipe depends on SYSV shell behavior, the pipe status
# is the status of the last command.  Also note that the "file" command
# MUST NOT be in the pipeline, since the grep will terminate before
# the file command has finished with long access devices (like magtapes)
# causeing synchronization problems.
#
info=`file $DEVICE`
echo "$info" | grep -s "special" > /dev/null ||
    {
    echo 1>&2 "WARNING: The file ${DEVICE}, is not a special device. "\
    	      "It should be of the"
    echo 1>&2 "         form:    /dev/xxxx"
    eval `_U_Prompt "Proceed with build using $DEVICE" RESP \
    		"y yes n no" "n" "
	If you are testing the procedures, then 'yes' may be an appropriate
	response to this query.

	If you are trying to produce a production kit, then you have
	requested an invalid device name and you should respone 'no'.
	"`
    case $RESP in
	y*|Y*) echo 1>&2 "\n  Proceeding, but kit build is non-standard.";;
	n*|N*) return 1 ;;
	*)     echo 1>&2 "?Invalid return from _U_PROMPT!!"; exit 1 ;;
    esac
    }

#
# Read the output device name from the terminal
# and perform a consistency check on it.
#
#
# Force any magtape specification to be of the non-rewinding variety.
#
# /dev/null is a test case.
#
case $DEVICE in
    /dev/*rmt*)
	UNIT=`expr $DEVICE : '.*'mt'\([0-9][0-9]*.*\)'`
	DEVICE=/dev/nrmt$UNIT
	test $DEBUG && echo 1>&2 "DEBUG: UNIT=$UNIT DEVICE=$DEVICE"

	echo 1>&2 "Rewinding. . ."
	mt -f $DEVICE rew ||
	    {
	    echo 1>&2 "$PROG: cannot rewind $DEVICE."
	    return 1
	    }
	;;

    /dev/null)
	test $DEBUG && echo 1>&2 "DEBUG: DEVICE=$DEVICE"
	;;

esac

#
# Check access
#
# If we don't have the necessary access, then
# ask to mount the volume.
#
_U_Check_Device $DEVICE $ACCESS ||
    {
    if [ $? -ne 0 ]
    then	    
	case $DEVICE in
	    /dev/*rmt)
		MEDIATYPE="tape"
		;;
	    *)
		MEDIATYPE="disk"
		;;
	esac
		
	_U_Request_Device_Load $DEVICE $MEDIATYPE \
		"Please mount a$VOL_STATE $VOLTYPE volume for $KITCODE on $DEVICE" $ACCESS || return 1
    fi
    }
#
# Now return the DEVICE
#
echo DEVICE='"'$DEVICE'"'
return 0
)}
##### End of _U_Check_Volume
