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
#	setld.sh
#		manage software subset distributions
#
#	setld [-D path] -c subset message		(configure)
#	setld [-D path] -d subset [subset...]		(delete)
#	setld [-D path] -h				(help)
#	setld [-D path] -i [subset...]			(inspect)
#	setld [-D path] -l location			(load)
#	setld [-D path] -v [subset...]			(validate)
#	setld [-D path] -x location			(extract)
#
# @(#)$RCSfile: setld.sh,v $ $Revision: 4.3.10.13 $ (DEC) $Date: 1994/01/05 16:00:32 $ 
#
#	000	ccb	12-mar-1987
#		Digital Equipment Corporation
#	Many thanks to robin, rnf, and afd
#	new version for 2.2
#
#	001	ccb	02-mar-1988
#		Set UMASK to 22 so that ris can read the images.
#
#	002	06-APR-1988	ccb
#		Fix DEVICE parsing bug.
#
#	003	18-feb-1989	ccb
#		Port to sh5
#		add update (-u) support
#
#	004	07-mar-1989	ccb
#		Add support for new style ris servers.
#
#	005	12-jun-1989	ccb
#		Fix extraneous scp on failed subset problem,
#		clean up entry to -l when subsets specified on command line,
#		clean up exit returns, fix 'D is read-only' problem in trap
#		for keyboard interrupts.
#
#	006	17-jul-1990	ccb
#		Fix problem finding unique client names in risdb.
#
#	007	05-nov-1990	ech
#		Fix path used by setld to exclude current directory,
#		add subset description in the dependency-check error message,
#		change format of messages givien when subset is loaded,
#		change return value such that setld will exit if underlying
#		scp performs an exit 1.
#	   
#	008	16-nov-1990	ech
#		Fix problem in setld -i when .ctrl is empty by resetting 
#		all control variables in ReadCtrlFile(), also mark subset as 
#		incomplete if .ctrl is incomplete.
#		Fix leftover /hosts problem.
#
#	009	17-dec-1990	ech
#		call fverify -yp in Verify(), fix path of .inv fed to tclear 
#		in LoadFromMedia().
#
#       010     29-jan-1991     ech
#               add current directory back to the end of setld PATH.
#               restore /etc/hosts every time LoadFromInet is called.
#
#	011     19-feb-1991     ech
#		clean up usage (no -a, -u, -v w/o specifying subsets).
#		move "Deleting (subset)" to before C_DELETE is called.
#		bug fix in RunScps (no C_INSTALL if POST_L fails).
#
#	012	04-mar-1991	ccb
#		removed reusable code to be placed in SHELL_LIB
#		removed v4.0 setld -u code
#		recoded uses of Add and Subtract calls to use expr(1),
#			the version of expr(1) on OSF/1 correctly handles
#			unary minus.
#
#       013     12-mar-1991     ech
#               add reg expr when 'egrep' ROOT in .image file in Extract().
#               check if RIS server echoes 'hello' in InitDevice().
#
#       014     03-apr-1991     ech
#               take out fverify -yp in Verify().
#
#       014     05-apr-1991     ech
#               bug fix, pass Ferror() the return status of the routine where
#               fatal error occurred.
#
#	015	23-apr-1991 	ech
#		added support for "it" by creating ordered subset list 
#		for C_INSTALL in $_R/sbin/it.d/data/cinst.data.
#
#	016	10-may-1991 	ech
#		added -f (force load) flag to bypass dependency checking.
#		added menu caching.
#
#	017	30-may-1991	ccb
#		add *Proto() and *Config() routines.
#		integrate with TIN isl
#		move usr/opt/data to usr/.smdb.
#		create syslog directory before attempting to open the
#			log file
#		correct sequencing of scp calls for Delete()
#
#	018	21-jun-91	ccb
#		added Basename() and Dirname() replacing calls to the
#		corresponding 'C' programs - done to avoid the overhead
#		of more staically linked binaries
#		fixed an error calling CopyConfig() from Configure()
#		use full path when creating directory for logfiles
#		fix bugs in CopyProto() and CopyConfig()
#
#	019	02-jul-1991	ccb
#		move logging from /var/adm/syslog to /var/adm/smlogs
#
#	020	24-jul-1991	ccb
#		change variable D to _D in DeleteConfig() and DeleteProto()
#
#	021	27-sep-1991	ech
#		no longer direct error message from fitset to /dev/null
#		modify MASK from "OSF" to "^OSF"
#
#	022 	15-oct-1991	ech
#		use "cp -p" in CopyConfig and CopyProto
#		change menu option "None of the Above" to "Mandatory subsets 
#		only", add example in menu prompt, list subsets in basic inst.
#

SHELL_LIB=${SHELL_LIB:-/usr/share/lib/shell}

. $SHELL_LIB/Lists
. $SHELL_LIB/Logging
. $SHELL_LIB/Pwd
. $SHELL_LIB/Ready
. $SHELL_LIB/Strings
. $SHELL_LIB/Ticker
. $SHELL_LIB/Wait


#%	DECLARATIONS
#%	SUBROUTINES
#		all subroutines used in setld are defined in alphabetical
#		order here.

:	-AreInstalled
#		subset installed status predicate
#
#	given:	$* - list of subsets
#	does:	copies each subset code on the arglist representing a
#		subset which is installed to stdout
#	return:	nothing

AreInstalled()
{ (	ILIST=$*

	for ISUB in $ILIST
	{
		[ -f $SMDB/$ISUB.lk ] && echo $ISUB
	}
) }


:	-Args
#		break up the command args
#
#	given:	the command line
#	does:	break the command line up
#	return:	nothing
#	effect:	$ACT - sets to act value
#		$DEFPATH - set to 0 if root specified, else 1
#		$DEVICE - set to device name for -alux
#		$_R - set to / or new rootpath if specified
#		$ARGV - set to remaining arguments

Args()
{
	IGNORE=0	# ignore current option
	OPT=		# current option value

	# parsing the command line:
	[ $# = 0 ] &&
	{
		# no arguments
		Usage
		return 1
	}

	case "$1" in
	-*)	;;
	*)	# assume old-style optional path prepend -D option switch
		set xx -D $*; shift
	esac
	set xx `getopt "cD:dhil:x:v" $*`; shift 
	[ $# = 0 ] &&
	{
		Usage
		return 1
	}

	ACT=
	_R=/
	DEFPATH=1

	for OPT in $*
	{
		[ "$IGNORE" = 1 ] &&
		{
			# skip already used option
			IGNORE=0
			continue
		}
		#! checks may be desired to guarantee that only one
		#!  action option has been specified
		case "$OPT" in
		-[lx])
			CMDSW=$OPT
			DEVICE=$2
			IGNORE=1	# ignore OPT value on next iteration
			shift 2
			;;

		-[cdhiv])
			CMDSW=$OPT
			shift
			;;

		-D)	DEFPATH=0
			_R=$2
			IGNORE=1	# ignore OPT value on next iteration
			shift 2

			# verify that the target directory exists
			[ -d $_R ] ||
			{
				Error "$_R: $ENOENT"
				return 1
			}
			case "$_R" in
			/*)	;;
			*)	_R=`(cd $_R;Pwd)`
			esac
			;;

		--)	# end of options
			shift
			break
			;;
		*)	# bad switches
			Error "Unreached in Args()"
			return 1
		esac
	}

	CMDSW=`Parse - $CMDSW`
	(Log "root=$_R -$CMDSW")	# subshell to protect command args

	ACT=`Ucase $CMDSW`
	case "$WHOAMI,$CMDSW" in
	*,)	# no action switch specified
		Usage
		return 1
		;;
	root,[cdlvx])
		;;
	*,[cdlvx])
		Error "-$CMDSW can be used by super-user only."
		return 1
	esac


	ARGV=$*
	return 0
}



:	-Basename
#		like the program, just cheaper
#

Basename()
{ (
	[ "$1" ] || return 0
	P=$1
	set xx `Parse / $P`
	shift
	eval echo \$$#
) }


:	-Cleanup
#		cleanup before exiting
#
#	given:	uses global context
#	does:	exit processing, called from Exit

Cleanup()
{
	trap '' 0
	cd $_R

	case "$_MEDIA" in
	tape)	echo "Rewinding Tape..."
		TickWhile Wait MTPID
		TickWhile mt -f $RAW rew
		;;
	esac

	DATE=`date +19%y.%m.%d.%T`
	Ticker off
	wait
	Log "SETLD $$ $DATE $_R -$CMDSW $STAT $EMESG"
	rm -rf $_TDIR
}


:	-Configure
#		configure a subset
#
#	given:	$1 - the name of the subset to configure
#		$2 - the argument to be used in invoking the scp
#	does:	invoke the scp for the named subset with ACT=C and the
#		specified argument on the command line
#	return:	0 is successful, 1 on error

Configure()
{
	[ $# != 2 ] &&
	{
		Usage -c
		return 1
	}
	set `Ucase $*`	#! coerce subset name and option to upper case
	SUBSET=$1	# subset name to configure
	CONFARG=$2	# command arg for scp

	#% Local Code
	cd $_R

	# Make sure the subset is installed
	[ -f $SMDB/$SUBSET.lk ] ||
	{
		Error "$SUBSET $E_NOINST, cannot configure."
		return 1
	}

	# verify that the scp exists
	[ -f $SMDB/$SUBSET.scp ] ||
	{
		Error "$SUBSET: missing control program, cannot configure."
		return 1
	}

	# all clear
	ReadCtrlFile $SMDB $SUBSET 
	echo "\nConfiguring \"$SDESC\" ($SUBSET)"

	# transition .proto.. files to config state
	CopyConfig $SUBSET

	# if in update installation, execute merge files
	[ "$UPDFLAG" ] && UpdMerge $SUBSET ToConfig

	Log "$SUBSET SCP C $CONFARG \c"
	if ACT=C $SMDB/$SUBSET.scp $CONFARG; then
	{
		Log "SUCCEEDED"

		# rename update merge scripts to start with '#' and remove 
		# their inventory records in the .inv. 
		[ "$CONFARG" = INSTALL ] &&
			MoveMrg $SUBSET 

		return 0
	}
	else
	{
		Log "FAILED: scp status $?"

		# remove .lk and create a corrupt lock
		rm -f $SMDB/$SUBSET.lk
		echo "SCP C_INSTALL FAILED" > $SMDB/$SUBSET.dw

		return 1
	}; fi
}


:	-CopyConfig
#		make state transition to Config state for state files
#
#	given:	$1 - subset name
#	does:	copies all .proto.. files to full config names
#	return:	NIL

CopyConfig()
{ (
	SUBSET=$1

	awk '{print $10}' $SMDB/$SUBSET.inv | 
		egrep '/\.new\.\.' | sed 's/\/\.new\./\/.proto./' |
		while read FILE
		do
			_D=`Dirname $FILE`
			F=`Basename $FILE`

			N=`expr $F : '\.proto\.\.\(.*\)'`
			[ ! -f $_D/$N ] && cp -p $FILE $_D/$N
		done
) }


:	-CopyInstctrl
#		copy instctrl files to SMDB
#	
#	given:	$1 - a list of subsets whose instctrl files are to be copied.
#		$2 - specified only in update installation and contains
#		     subsets to be updated.
#	does:	copy related instctrl files to SMDB. 
#
#		In update installation, clean up outdated instctrl info of
#		subsets available on the media but are not to be updated.
#
#	return: 0


CopyInstctrl()
{ (
	case $# in
	1) 	ALLSUBS=$1
		;;
	2)	# this case only true with update installation.			

		SUBS=$1
		UPD=$2

		# filter through ACT=M first
		ALLSUBS=
		for _S in $SUBS
		{
			ACT=M $_TDIR/$_S.scp -$CMDSW &&
				ALLSUBS="$ALLSUBS $_S"
		}

		# remove old instctrl info of un-updated subsets now. Outdated 
		# instctrl info of updated subsets will be removed later
		# in UpdCleanup().

		NO_COPY=
		for _S in $ALLSUBS
		{
			Member $_S $UPD ||
			{
				# if subset is installed, then do not replace 
				# its smdb files.
				[ -f $SMDB/$_S.lk ] &&
				{
					NO_COPY="$NO_COPY $_S" 
					continue
				}

				NO_VER=`expr "$_S" : '\(.*\)[0-9][0-9][0-9]'`
				rm -f $SMDB/$NO_VER*
			}
		}

		# filter out subsets whose smdb files should be kept intact.
		[ "$NO_COPY" ] &&
			ALLSUBS=`ListUniq "$ALLSUBS" "$NO_COPY"`
		;;
	esac

	for _S in $ALLSUBS
	{
		cp $_TDIR/$_S.ctrl $_TDIR/$_S.inv $_TDIR/$_S.scp $SMDB
	}

	return 0

) }


:	-CopyProto
#		make state transition to Proto state for state files
#
#	given:	$1 - subset name
#	does:	for all of the .new.. files in the subset inventory,
#		copies them to .proto..
#	return:	NIL

CopyProto()
{ (
	SUBSET=$1

	awk '{print $10}' $SMDB/$SUBSET.inv | egrep '/\.new\.\.' |
		while read FILE
		do
			_D=`Dirname $FILE`
			F=`Basename $FILE`

			N=`expr $F : '\.new\.\.\(.*\)'`
			[ ! -f $_D/.proto..$N ] && cp -p $FILE $_D/.proto..$N
		done
) }



:	-Delete
#		Delete subsets
#
#	given:	$* - a list of subsets to delete
#	does:	delete the subsets
#	return:	0 on success
#		1 on failure

Delete()
{ (

	ERR=0
							
	case "$#" in
	0)	#! menuhook
		Usage -d
		Log "Delete(): argument error"
		return 1
	esac
	set xx `Ucase $*`; shift

	CTX=		# context prefix
	_S=		# current subset

	cd $_R		# assure operation in user specified hierarchy

	SUBS=$*

	# filter out subsets that can not be deleted and arrange 
	# the rest in reverse depord order.
       
	NO_DEL=
	for _S in $SUBS
	{
		CTX=$SMDB/$_S

		# is it installed? (correctly or damaged)
		[ -f $CTX.lk -o -f $CTX.dw ] ||
		{
			Error "$_S: $E_NOINST, cannot delete"
			ERR=1
			NO_DEL="$NO_DEL $_S"	
			continue
		}

		# read in the control file.
		ReadCtrlFile $SMDB $_S ||
		{
			Error "Error reading control file for $_S, cannot delete."
			ERR=1
			NO_DEL="$NO_DEL $_S"	
			continue
		}

		# check the sticky bit
		[ `FlagsAttrCheck SATTR_STICKY $FLAGS` = 1 ] &&
		{
			# sticky flag is set for this subset
			Error "Sorry, You may not delete \"$SDESC\" ($_S)"
			NO_DEL="$NO_DEL $_S"	
			ERR=1
			continue
		}
	}

	[ "$NO_DEL" ] &&
	{
		# filter out subsets that can not be deleted
		SUBS=`ListUniq "$SUBS" "$NO_DEL"`
		[ "$SUBS" ] ||
		{
			# no subsets left to delete
			return $ERR
		}
	}

	# rearrange deletable subsets in reverse depord order
	SUBS=`(cd $SMDB; depord $SUBS)`
	SUBS=`ListReverse $SUBS`

	for _S in $SUBS
	{
		CTX=$SMDB/$_S
		Log "$_S"

		# read in the control file.
		ReadCtrlFile $SMDB $_S 

		# save the description for messaging purpose 
		S_DESC=$SDESC

		[ -s $CTX.lk ] &&
		{
			#  subset is installed and has a non-zero length lock
			#  file. This means that there are subsets that depend
			#  on the one that we are trying to remove. This
			#  requires that we notify the user and verify that
			#  the ramification of a request to delete are
			#  understood.

			# take out duplicate locks.
			sort -o $CTX.lk -u $CTX.lk

			# if dependency subsets are also specified to be
			# deleted, they need to be filtered out in the msgs.
			# we do this here in case reverse depord order did not 
		        # catch the dependency, which is likely for LPs.

			LOCKS=`cat $CTX.lk`
			LOCKS=`ListUniq "$LOCKS" "$SUBS"`

			[ `Length $LOCKS` -ne 0 ] && 
			{
				echo "
The following subsets need \"$S_DESC\" ($_S) to operate correctly:\n"

				for K in $LOCKS
				{
					# read in the control file.
					ReadCtrlFile $SMDB $K 
					echo "	$SDESC ($K)"
				}

				while :
				do
					echo "
Are you sure you wish to delete \"$S_DESC\" ($_S)? (y/n): \c"
					read X
					case "$X" in
					[Yy]*)	Log " WARNING: locked"
						break
						;;
					[Nn]*)	Log " FAILED: locked"
						ERR=1
						continue 2
					esac
				done
			}
		}

		[ -f $CTX.lk ] &&
		{
			# subset is installed, run the configure delete
			Log "$_S SCP C DELETE"
			ACT=C $CTX.scp DELETE ||
			{
				Error "$_S, deletion declined by subset control program."
				ERR=1
				continue
			}
			DeleteConfig $_S

			Log "$_S SCP PRE_D"
			ACT=PRE_D $CTX.scp ||
			{
				Error "$_S, deletion declined by subset control program."
				ERR=1
				continue
			}

			DeleteProto $_S

			echo "
Deleting \"$S_DESC\" ($_S)."

		}
		

		# remove the subset.

		# run the inventory into 'frm'
		frm < $CTX.inv

		[ -f $CTX.lk ] &&
		{
			# subset was installed, clean up

			Log "$_S SCP POST_D"
			ACT=POST_D $CTX.scp ||
			{
				Error "$_S, subset control program failed."
				ERR=1
			}

			# clean out dependency info
			[ "$DEPS" != . ] &&
			{
				# remove dependency lock file info
				for K in $DEPS
				{
					[ -f $SMDB/$K.lk ] &&
					{
						egrep -v $_S $SMDB/$K.lk > $TMP1
						mv $TMP1 $SMDB/$K.lk
					}
				}
			}
		}
		# mark subset as uninstalled
		rm -f $CTX.lk $CTX.dw
	}
	return $ERR 
) }


:	-DeleteConfig
#		Delete files in Config state
#
#	given:	$1 - subset name
#	does:	deletes all configure state versions of files in subset
#	return:	NIL
#

DeleteConfig()
{ (
	S=$1

	awk '{print $10}' $SMDB/$S.inv | egrep '/\.new\.\.' |
		while read FILE
		do
			_D=`Dirname $FILE`
			F=`Basename $FILE`

			N=`expr $F : '\.new\.\.\(.*\)'`

			rm -f $_D/$N &
		done
) }
	


:	-DeleteProto
#		Delete files in Proto state
#
#	given:	$1 - subset name
#	does:	deletes all proto state versions of files in subset
#	return:	NIL
#

DeleteProto()
{ (
	S=$1

	awk '{print $10}' $SMDB/$S.inv | egrep '/\.new\.\.' |
		while read FILE
		do
			_D=`Dirname $FILE`
			F=`Basename $FILE`

			N=`expr $F : '\.new\.\.\(.*\)'`

			rm -f $_D/.proto..$N &
		done
) }


:	-DependencyOrder
#		order a bucketload of subsets
#
#	given:	$* - a list of subsets
#	does:	orders the subset in tape order, writing the ordered list
#		to stdout. It is assumed that the control files for the
#		subsets are in the current directory.
#	return:	nothing

DependencyOrder()
{
	# for disk and ris, use depord 
	[ "$_MEDIA" != tape ] &&
	{
		depord $SBS
		return 
	}

	case "$#" in
	0)	Error "DependencyOrder(): no subsets specified"
		return 
	esac
	Bu_SUBS=$*

	#% Local Variables
	Bu_ORD=			# ordered subsets
	Bu_LEFT= Bu_RIGHT=	# sort temp variables

	#% Local Code
	# get the PNV for the lead subset in the bucket
	for Bu_SUB in $Bu_SUBS
	{
		NameParse Bu_ $Bu_SUB && break
		Bu_SUBS=`(set $Bu_SUBS;shift;echo $*)`
	}

	for Bu_S in $Bu_SUBS
	{
		# get tape location
		ReadCtrlFile $_TDIR $Bu_S || continue

		set xx `Parse : $MTLOC`; shift
		Bu_VOL=$1 
		Bu_SORD=$2

		# pass tape volume, order on tape, subset name to awk.
		echo "$Bu_VOL $Bu_SORD $Bu_S"

		# arrays in awk are dynamic, so we need to keep track of
                # the bounds ourselves (vmax, lmax, vmin, lmin) */
		#
                # order and inuse are two-dimensional arrays indexed by
                # tape volume ($1) and order on tape ($2).
                # 'order' stores the subset name;
                # 'inuse' stores 'y' to mark the tape location in use. */

	} | awk 'BEGIN {
			vmax=0; vmin=0
			lmax=0; lmin=0
		}
		{       
			order[$1","$2] = $3
			inuse[$1","$2] = "y"
			if( $1 > vmax )	vmax = $1
			if( $2 > lmax )	lmax = $2
			if( $1 < vmin ) vmin = $1
			if( $2 < lmin ) lmin = $2
		}
		END {
			for( j = vmin; j <= vmax; ++j )
			{
				for( i = lmin; i <= lmax; ++i )
				{
					if( inuse[j","i] == "y" )
						print( order[j","i] )
				}
			}
		}'
}


:	-DetermineAvailable
#		determine which subsets are installable in this environment
#
#	given:	global data only
#		contents of $_TDIR
#		$ADVFLAG - specifies installation type
#	does:	uses the temp directory to derive a list of subsets which
#		are available and installable to the system. Installable
#		subsets are those which are not currently installed and
#		are not masked by the subset mask
#	return:	nothing
#	effect:	sets $SBS
#

DetermineAvailable()
{
	MASK=".*"		# default, all subsets

	# decode,
	#  bits:
	#	00 - basic base
	#	01 - adv base
	#	10 - basic ws
	#	11 - adv ws

	case "$ADVFLAG" in
	[01])	MASK="$RISC_B|$VAX_B"
		;;
	[23])	MASK="$RISC_B|$RISC_WS|$VAX_B|$VAX_WS"
		;;
	esac


	# get list of subsets from temp dir and filter thru the mask
	SBS=`(cd $_TDIR; ls *.ctrl | sed 's/.ctrl//g' | egrep "$MASK")`
	# verify that there were control files
	[ "$SBS" = '*' ] &&
		SBS=
}



:	-Dirname
#		get all but the last part of a path
#

Dirname()
{ (
	P=$1

	set xx `Parse / $P`
	shift
	OUT=
	case "$P" in
	/*)	SL=/
		;;
	*)	SL=
	esac
	while [ $# -gt 1 ]
	do
		OUT="$OUT$SL$1"
		SL=/
		shift
	done
	echo $OUT
) }


:	-Dirs
#		create all needed directories
#
#	given:	nothing
#	does:	makes sure the directories needed to perform correctly
#		are available
#	return:	0

Dirs()
{
	# check if setld is in use.
	#! a better lock mechanism is needed here
	#
	case $CMDSW in
	[lx])	[ -d $_TDIR ] &&
		{
			Error "Temp directory $_TDIR already in use"
			return 1
		}
	esac

	# make sure that all required directories exist.
	#
	(cd $_R
		rm -rf $_TDIR
		for X in $S $S/$I $SID $U $SMDB $V $V/$A $LOGDIR $V/$T $_TDIR 
		{
			[ -d $X ] || mkdir $X ||
			{
				Error "$E_MKDIR $X"
				return 1
			}
		}
	)
	return 0
}


:	-Error
#		Print an error message to stderr
#
#	given:	a message to print, $1 may be option '-n'
#	does:	prints the message to stderr, if -n option is absent,
#		also logs the message to the logfile
#	return:	nothing

Error()
{
	case "$1" in
	-n)	shift
		;;
	*)	Log "$1"
	esac
	1>&2 echo "$PROG: $1"
}


:	-Exit
#		Leave the program
#
#	given:	$1 - exit status
#	does:	leave program performing any needed cleanup
#	return:	no

Exit()
{
	STAT=$1
	Cleanup
	exit $STAT
}


:	-Extract
#		extract subsets from media
#
#	given:	$* - names of specific subsets to extract
#	does:	Extract subsets from the media for use by
#		remote installation service.
#	return: 0 if all goes well
#		1 if mandatory/ROOT extraction failure
#		2 if optional extraction failure

Extract()
{
	#% Binding
	SBS=$*		# subsets to be installed

	#% Local Variables
	EISSUB=		# extraction element is a subset {0,1}
	ELOC=		# Extraction element tape location (x:y)
	ENAME=		# Extraction element name
	EOPT=		# extraction element optionality value
	EXTL=		# EXTraction List

	#% Local Code
	[ "$DEFPATH" = 0 ] && cd $_R

	Wait MTPID ||
	{
		Error "Tape Positioning Error."
		return 1
	}

	# make sure there is an instctrl directory
	[ -d instctrl ] ||
		mkdir instctrl

	# get image and comp file
	cp $_TDIR/*.image $_TDIR/*.comp instctrl 2> /dev/null
	touch instctrl/*.image

	# establish an extraction list. This lists each object to be
	#  extracted, it's media location, whether it is optional, and
	#  whether it is an actual subset or a proprietary tape file.
	# at the same time, create an egrep mask to be used to generate
	#  a checksum comparison file
	EXTL=
	#! reference to image file
	ENAME=`egrep '[	 ]h?ROOT[	 ]*$' instctrl/*.image` &&
	{
		set xx $ENAME; shift
		ENAME=$3
		# dummy out an entry that looks like a subset would
		EXTL="$ENAME:1:-1:0:0:$1"
	}
	# build entries for the rest of the subsets
	for ENAME in $SBS
	{
		ReadCtrlFile $_TDIR $ENAME
		EOPT=`FlagsAttrCheck SATTR_OPTION $FLAGS`
		SUM=`egrep $ENAME instctrl/*.image`
		set xx $SUM; shift
		SUM=$1
		EXTL="$EXTL $ENAME:$MTLOC:$EOPT:1:$SUM"
	}
	OPTERR=0
	# create empty files
	> checksums; > mandatory; > all
	> $TMP1
	for EXTENT in $EXTL
	{
		set xx `Parse : $EXTENT`; shift
		ENAME=$1 EVOL=$2 ELOC=$3 EOPT=$4 EISSUB=$5
		Log "$ENAME \c"
		echo "Extracting $ENAME..."

		# read the media.....
		case "$_MEDIA" in
		tape)
			PositionTape $EVOL $ELOC ||
			{
				Error "Error Extracting $ENAME"
				rm -f checksums mandatory all
				return 1
			}
			TickWhile "dd if=$RAW of=$ENAME bs=20b 2> /dev/null" ||
			{
				Error "Error Extracting $ENAME"
				rm -f checksums mandatory all
				return 1
			}
			_CPOS=`expr $_CPOS + 1`
			;;
		disk)	TickWhile "cp $_SRC/$ENAME ." ||
			{
				Error "Error Extracting $ENAME"
				rm -f checksums mandatory all
				return 1
			}
		esac

		[ $EISSUB = 1 ] &&
		{
			# subset specific operations:
			#  copy .ctrl, .inv, .scp for this subset into instctrl
			#  update mandatory, all
			for X in inv scp ctrl
			{
				cp $_TDIR/$ENAME.$X instctrl ||
				{
					Error "Control Info Error on $ENAME"
					rm -f all checksums mandatory
					return 1
				}
			}
			echo "$ENAME" >> all
			[ $EOPT = 0 ] && echo "$ENAME" >> mandatory
		}

		# checksum the image
		#  wait for previous checksum
		Wait SUMPID
		(
			echo "$ENAME	\c" >> $TMP1
			sum $ENAME >> $TMP1
		) &
		SUMPID=$!

		Log "SUCCEEDED"
	}
	Wait SUMPID
	SUMLIST="$SUMLIST $SUM"

	
	[ "$_MEDIA" = tape ] &&
	{
		mt -f $RAW rew &
		MTPID=$!
		_CPOS=-$T_0
	}

	for EXTENT in $EXTL
	{
		# get subset name
		set xx `Parse : $EXTENT`; shift
		ENAME=$1
		ESUM=$6
		set xx `egrep $ENAME $TMP1`; shift
		SSUM=$2
		shift
		SUMLIST=$*
		[ "$ESUM" = "$SSUM" ] ||
			Error "$ENAME: extract checksum error"
	}

	# rm -f mandatory compare checksums csd all &

	echo "Media extraction complete."
	return 0
}


:	-Ferror
#		Fatal Error
#
#	given:	$1 - Exit status
#		$2 - Error Message
#	does:	print error message and Exit with status

Ferror()
{
	STAT=$1
	Error "$2"
	Exit $STAT
}


:	-FlagsAttrCheck
#		check flags attribute
#
#	given:	$1 - flag attribute to check
#		$2 - word to check in
#	does:	check the value of the $1 flag in $2
#	return:	0 if flag is clear, 1 if set
#

FlagsAttrCheck()
{ (
	ATTR=$1
	FLAG=$2

	case "$ATTR" in
	SATTR_STICKY)
		VAL=$FLAG
		;;
	SATTR_OPTION)
		VAL=`expr $FLAG / 2`
		;;
	*)	Error "FlagsAttrCheck: $ATTR: unknown attribute type"
		Exit 1
	esac

	expr $VAL % 2
) }


:	-GetCompAttr
#		determine compression status of a subset
#
#	given:	a subset name
#	does:	determines whether the subset is compressed
#	return:	0 for a compressed subset
#		1 for a non-compressed subset

GetCompAttr()
{ (	S=$1

	cd $_TDIR
	NameParse X_ $S
	[ -f $X_P$X_V.comp ]
) }


:	-InitDevice
#		Initialize the installation device
#
#	given:
#	does:
#	return:
#

InitDevice()
{
        _MEDIA=         # media type - tape, diskette, disk, inet
        _SRC=           # unit - server name, /dev/xxx, install path
        _LOC=           # location where the savesets are stored, can be
                        # path name, host name or tape

	[ -d $DEVICE ] &&
	{
		_MEDIA=disk
		_SRC=$DEVICE
		_LOC=$_SRC

		# stabilize the pathname
		case "$_SRC" in
		/*)	;;
		*)	_SRC=`(cd $_SRC;Pwd)`
		esac

		Log "Loading from $_LOC ($_MEDIA)"
		return 0
	}

	# get the media type.
	case "$DEVICE" in
	*::)	#
		Error "$DEVICE - DECnet installation not supported"
		return 1
		;;
	*:)	# TCP inet installation
		_MEDIA=inet
		# the 'unit-number' is the server hostname
		_SRC=`Parse : $DEVICE`
		_LOC=$_SRC

		#! error case should be handled with retries for goodies like
		#!  no inet ports, login limit reached.
		ERROR=`rsh $_SRC -l ris -n "echo hello" 2>&1` 
	        [ "$ERROR" = 'hello' ] || 
		{
			Error "Error contacting server $_SRC: $ERROR"
			return 1
		}
		;;

	*mt*[lmh])
		# some sort of tape device, get unit number and verify
		#  access to nrmt?h.
		_MEDIA=tape
		_SRC=`expr $DEVICE : '.*mt\([0-9][0-9]*\).*'`
		_LOC="/dev/nrmt${_SRC}h"
		;;
		
	*mt*)	# tape device naming obsolete - xlate to 2.0
		_MEDIA=tape
		_SRC=`expr $DEVICE : '.*mt\([0-9][0-9]*\).*'`
		_SRC=`expr $_SRC % 4`	# this gets unit plug number.
		;;

	*ra*)	# ra, diskettes
		# get unit number
		_SRC=`expr $DEVICE : '.*ra\([0-9][0-9]*\).*'`

		_MEDIA=diskette
		_LOC=$_SRC
		ALTOP=+
		ALT=`expr $_SRC $ALTOP 1`
		# validate the existence of device files for
		# both the primary and alternate diskettes
		[ -c $D/rra${_SRC}a -a -c $D/rra${ALT}a ] ||
		{
			Error "$E_NODEV: /dev/rra${U}a, /dev/rra${ALT}a"
			return 1
		}
	esac

	case "$_MEDIA" in
	"")	# bogus device.
		Error "Device $DEVICE $E_NOSUPP"
		return 1
		;;
	tape)	# do code for old and new style tapes
		RAW=$D/nrmt${_SRC}h
		[ -c $RAW ] ||
		{
			Error "Cannot access $RAW"
			_MEDIA=
			return 1
		}
		while :
		do
			echo "
Please make sure your installation tape is mounted and on-line."

			Ready
			mt -f $RAW rew && break
		done
		_CPOS=-$T_0
		;;
	esac
	Log "Loading from $_LOC ($_MEDIA)"
}


:	-InitializeConstants
#		Initialize all constants to be used in setld
#
#	given:	arglist identical to invocation arglist
#	does:	initialize the constants used thruough the program.
#		all constants should be defined here.
#	return:	nothing

InitializeConstants()
{
	# Message Strings
	E_FAIL="File copy to system disk failed."
	E_MKDIR="Cannot create directory"
	E_NODEV="Please be certain that device special file"
	E_NOINST="not currently installed"
	E_NORECOVER="Cannot recover"
	E_NOSUPP="not supported for installations."
	E_READ="Attempt to read from your distribution media failed."
	E_STARS="*** Subset"
	E_TPOS="Tape positioning error."
	E_UNKNOWN="Unknown subset"
	ENOENT="no such file or directory"
	IC="Installation Control"

	# the usage messages
	CUSAGE="Send a configuration message to an installed subset:
	setld [-D dir] -c subset message"
	DUSAGE="Delete subset(s):
	setld [-D dir] -d subset [subset ...]"

	USAGE="
Setld Usage Examples:

$CUSAGE

$DUSAGE

List all subsets:
	setld [-D dir] -i

List contents of installed subset(s):
	setld [-D dir] -i subset [subset ...]

Display this message:
	setld [-D dir] -h

Load layered product from device:
	setld [-D dir] -l device [subset...]

Verify integrity of subset(s):
	setld [-D dir] -v subset [subset ...]

Extract media images from device for network distribution:
	setld [-D dir] -x device [subset...]

"

	DATFMT="+19%y.%m.%d.%T"		# format specifier for date command
	PROG=setld			# program name
	T_0=3				# control file tape offset

	# path name constants
	#
	A=adm
	D=/dev
	E=etc
	I=it.d
	S=sbin
	SID=$S/$I/data			# location for data file used by "it"
	T=tmp
	U=usr
	SMDB=$U/.smdb.			# _s_oftware _m_anagement _d_b_
	V=var
	_TDIR=/$V/$T/stltmp$$		# default temporary directory

	# log file constants
	CINSTDATA=$SID/cinst.data	# ordered subset list for "it"
	LOGDIR=$V/$A/smlogs		# location for log file
	FVERIFYLOG=$LOGDIR/fverify.log
	FITSETLOG=$LOGDIR/fitset.log
	LOGFILE=/$LOGDIR/setld.log	# default logfile
	MENU=$_TDIR/menu		# contains user selection from menu.
	UPDMRGLOG=$LOGDIR/upd_mergefail_files # contains files that failed to
					      # merge in update installation.

	[ ! "$CHECK_SYNTAX" ] &&
	{
		# interactive debugging turned off, mark all constants
		#  as read only - please insert here in the order in which
		#  they are defined above

		readonly E_NOSUPP E_READ E_STARS E_TPOS E_UNKNOWN ENOENT
		readonly E_FAIL E_MKDIR E_NODEV E_NOINST E_NORECOVER
		readonly IC CUSAGE DUSAGE USAGE
		readonly PROG T_0 A D E T U SMDB V
	}

}



:	-InitializeGlobals
#		Initialize default values for all global variables
#
#	given:	nothing
#	does:	sets starting values for the global values used in
#		setld. All global variables should be defined here.
#	return:	nothing

InitializeGlobals()
{
	_CVOL=1				# currently mounted tape volume
	_R=/				# default install root (_R for root)

	DATE=				# current date
	DEBUG=0				# default debug flag
	DECOMP=cat			# decompression program
	DEFPATH=1			# flag - installing to default root
	MTPID=				# mag tape async op pid
	RISC_B="^OSF"			# RISC base system product definition
	RISC_WS="^OSF"			# RISC UWS product definition
	STAT=1				# default exit status
	TMP1=$_TDIR/tmp1		# scratch file	
	TMP2=$_TDIR/tmp2		# scratch file	
	VAX_B="^ULT"			# VAX base system product definition
	VAX_WS="^UWS"			# VAX UWS product definition
	VBSE=				# default tar verbose switch

}


:	-Install
#		Install subsets
#
#	given:	$* - list of subsets to be installed
#	does:	calls LoadFromMedia() and PostLoad()/Configure() to install 
#		the software
#	return:	0 if all subsets install correctly
#		1 (immediately) on LoadFromMedia mandatory subset failure
#		2n for n optional failures
#		

Install()
{
	OPTERRS=0
	SCPLIST=			# subsets that survive LoadFromMedia() 

	# load the bits
	for _S
	{
		LoadFromMedia $_S
		case "$?" in
		0)	SCPLIST="$SCPLIST $_S"	# all's OK

			# move .upd.. into real name space and update .inv
			# do it here instead of in PostLoad() to avoid
			# accumulating all the .upd.. and wasting disk space.
			MoveUpd $_S

			;;
		1)	OPTERRS=1		# MAND (fatal)
			break
			;;
		*)	OPTERRS=`expr $OPTERRS + 2`	# OPT (non-fatal)
		esac

	}

	# state transitions
	# Note: even with fatal error, we still need to complete state 
	# transitions for subsets previously successfully installed since
	# .lk was already created in LoadFromMedia() (QAR 14757).

	for _S in $SCPLIST
	{

		# further update installation merge optimization
		[ "$UPDFLAG" ] && UpdMoreOpt $_S

		# transition to proto state and call scp with POST_L
		PostLoad $_S ||
			continue

		# if installing without specifying -D, configure subset now,
		# i.e., transition to config state. 
		if [ "$DEFPATH" = 1 ] 
		then
			Configure $_S INSTALL 
		else
			# delay configuration, record subset in $CINSTDATA
			# so it can be configured later with setld -c
			echo "$_S" >> "$_R/$CINSTDATA"
		fi
	}

	return $OPTERRS
}


:	-Inventory
#		product inventory listings
#
#	given:	$* - which subsets to inventory, if empty will provide
#		a list of all known subsets and subset status
#	does:	see given
#	return:	0

Inventory()
{ (
	_S=	# current subset name
	LIST=	# list of all known subsets

	cd $_R/$SMDB

	[ "$*" ] &&
	{
		set `Ucase $*`
		ERR=0
		for _S
		{
			[ -f $_S.inv ] ||
			{
				Error "$_S: $E_UNKNOWN"
				ERR=1
				continue
			}
			awk '{print $10}' $_S.inv
		}
		return $ERR 
	}

	LIST=`echo *.ctrl | sed 's/\.ctrl//g'`

	[ "$LIST" = '*' ] && 
	{
		Error "No subset information available in $_R/$SMDB"
		return 1
	}

	# Generate report
	echo "
Subset		Status		Description
------		------		-----------"

	for _S in $LIST
	{
		STATUS=
		[ -f $_S.lk ] && STATUS="installed"
		[ -f $_S.dw ] && STATUS="corrupt" # ica-12459

		ReadCtrlFile . $_S 2> /dev/null || 
		{
			# if .ctrl is incomplete, subset is incomplete 
			STATUS="incomplete"
		} 

		DESC=`echo $DESC | sed 's/\(.*\)%\(.*\)/\1(\2)/'`
		echo "$_S'$STATUS'$DESC"

	} | awk -F"'" '{ printf "%-16s%-16s%s\n", $1, $2, $3 }'
	return 0
) }


:	-ListKnown
#		Given a version free subset name, print a list of all other
#		versions of that subset currently known to the system.
#
#	given:	a version free subset name ($P$N) parts only.
#	does:	looks up version of $P$N know the the system
#	return:	nothing
#	sides:	none
#	output:	a list of subset codes

ListKnown()
{ (	PN=$1

	LIST=`(cd $SMDB; echo $PN???.ctrl | sed 's/\.ctrl//g' )`

	case "$LIST" in
	*'*'*)	return		# No '*' match
	esac
	echo "$LIST"
) }


:	-LoadCtrlInfo
#		read the installation control information from the media
#	into $_TDIR
#
#	given:	global info
#		$_MEDIA - media type to read
#		$_SRC - unit to read from
#		$_TDIR - target directory
#	does:	read the control information required to perform the task
#		defined in $CMDSW into $_TDIR
#	return:	0 if everything goes well, 1 otherwise
#	effect:	sets SBS to list subsets installable from the media
#	note:	assumes that wd is the desired root

LoadCtrlInfo()
{
	case "$_MEDIA" in
	diskette)
		[ "$CMDSW" = "x" ] &&
		{
			Error "diskette media unsupported for -x"
			return 1
		}

		echo "
Insert the $IC diskette for the software
you wish to load on your system in $_MEDIA unit $_SRC.
"

		Ready

		(cd $_TDIR;tar xpf $D/rra${_SRC}a) > $TMP1 ||
		{
			echo "
Attempt to read from diskette drive $_SRC failed.

Remove your diskette from $_MEDIA unit $_SRC."

			# flip unit numbers
			_SRC=`expr $_SRC $ALTOP 1`
			ALTOP=`ChOp $ALTOP`

			echo "
Insert the $IC diskette for the software
you wish to load in $_MEDIA unit $_SRC.
NOTE: we are trying the SAME DISKETTE again but in $_MEDIA unit $_SRC.
"

			Ready

			(cd $_TDIR;tar xpf $D/rra${_SRC}a) ||
			{
				Error "Cannot read control information"
				return 1
			}
		}
		;;
	disk)	# mounted directory contains subsets
		[ -d $_SRC/instctrl ] ||
		{
			Error "$_SRC/instctrl: $ENOENT"
			return 1
		}
		# copy control info into $_TDIR.
		(cd $_SRC/instctrl; tar cf - *)|(cd $_TDIR; tar xpf -)
		;;
	inet)	[ $CMDSW = x ] &&
		{
			Error "network media not supported for -x"
			return 1
		}

		cd $_R

		ERROR=`rcp ris@$_SRC:clients/risdb $TMP1 2>&1` ||
		{
			Error "cannot access server database ($ERROR)"
			return 1
		}

		# we have copied the server's ris database to $TMP1
		#  this database has entries:
		#  cliname:haddr:sys-type,prod,...,prod

		# find the record for this client, check the long name first
		IAM=`hostname`
		RISRECORD=`egrep '^'$IAM: $TMP1` ||
		{		
			# get the short name and try again
			# break off bind spec, first part only used in client
			#  file naming
			set xx `Parse . $IAM`; shift
			IAM_S=$1

			RISRECORD=`egrep '^'$IAM_S: $TMP1` ||
			{
				Error "$IAM: not in server database"
				return 1
			}
		}

		# break out the fields, save the sys-type,prod,...,prod
		#  section
		set xx `Parse : $RISRECORD`; shift
		RISDATA=$3

		# break the sys-type,prod,...,prod into parts
		set xx `Parse , $RISDATA`; shift
		_SYSTEM=$1
		shift
		# Products
		PRODUCTS=$*

		# Read Control Files over one dir at a time, building rismap
		#  from each directory as it comes
	
		# turn on Ticker in case rsh takes a long time due to 
		# network traffic or too many product areas, etc.	
 		Ticker on
 
		for DIR in $PRODUCTS
		{
			mkdir $_TDIR/$DIR
			rsh $_SRC -l ris -n "cd $_SYSTEM/$DIR/instctrl;tar cf - *" |
				(cd $_TDIR/$DIR;tar xpf -)
			(	cd $_TDIR/$DIR
				echo "$DIR \c" >> ../rismap
				echo *.ctrl | sed 's/.ctrl//g' >> ../rismap
				mv * ..
			)
			rm -rf $_TDIR/$DIR &
		}

		Ticker off
		;;
	tape)	echo "Positioning Tape"
		PositionTape 1 0 ||
		{
			Error "Tape Positioning Error"
			mt -f $RAW rew &
			_CPOS=-$T_0
			return 1
		}

		(cd $_TDIR;tar xpbf 20 $RAW && mt -f $RAW fsf) ||
		{
			Error "Error reading control information"
			return 1
		}
		_CPOS=1
		echo
		;;
	*)	Error "Internal error in LoadCtrlInfo()"
		set | WriteLog
		return 1
	esac
	
	# set all modes and ownerships of incomming files
	(cd $_TDIR
		chown root *.inv *.scp *.ctrl
		chgrp system *.inv *.scp *.ctrl
		chmod 644 *.inv *.scp *.ctrl
		chmod +x *.scp
	)

	# Determine which subsets are availble from the media
	#  and are installable in the current context.
	#
	#! fix for setld -l loc subset [ subset... ]

	# have DetermineAvailable set up $SBS
	DetermineAvailable

	return 0
}


:	-LoadFromDisk
#		Load a subset from disk
#
#	given:	full GLOBAL context
#	does:	load a subset from disk
#	return:	1 on failure

LoadFromDisk()
{
	TickWhile "$DECOMP < $_SRC/$S | tar xpf -"
	return $?
}


:	-LoadFromDiskette
#		Load a subset from a diskette
#
#	given:	full GLOBAL context
#	does:	loads a subset from diskettes
#	return:	ignore

LoadFromDiskette()
{
	VOL=1
	set xx `Parse : $NVOLS`

	NVOLS=$2
	VMAX=`expr $2 + 1`
	while [ $VOL -le $NVOLS ]
	do
		echo "
Insert diskette $NAME $VOL of $NVOLS in $_MEDIA unit $_SRC.
"

		Ready

		# check volume number.
		tar tf $D/rra${_SRC}a > $TMP1 ||
		{
			_SRC=`expr $_SRC $ALTOP 1`
			ALTOP=`ChOp $ALTOP`

			echo "
$E_FAIL
Remove diskette $NAME $VOL of $NVOLS.

Insert diskette $NAME $VOL of $NVOLS in $_MEDIA unit $_SRC.
NOTE: we are trying the SAME DISKETTE AGAIN but in $_MEDIA unit $_SRC.
"

			Ready

			tar tf $D/rra${_SRC}a > $TMP1 ||
			{
				echo "
Cannot read your diskette, we will continue with the next volume."
				VOL=`expr $VOL + 1`
				continue
			}
		}
		ISVOL=`egrep "^Volume" $TMP1`
		ISVOL=`expr "$ISVOL" : '.*ume\([0-9][0-9]*\)'`
		case "$ISVOL" in
		$VOL)
			;;
		*)
			echo "
You have mistakenly mounted volume $ISVOL.

Please remove the diskette."
			sleep 1
			continue
			;;
		esac
		tar xpf $D/rra${_SRC}a ||
		{
			_SRC=`expr $_SRC $ALTOP 1`
			ALTOP=`ChOp $ALTOP`

			echo "
$E_FAIL
Remove diskette $NAME $VOL of $NVOLS.

Insert diskette $NAME $VOL of $NVOLS in $_MEDIA unit $_SRC.
NOTE: we are trying the SAME DISKETTE AGAIN but in $_MEDIA unit $_SRC.
"

			Ready
			tar xpf $D/rra${_SRC}a ||
			{
				Error "

$E_FAIL
Diskette $NAME: $VOL of $NVOLS may not have been installed
properly on your system but the rest of the installation will continue."
			}
			ERR=1
		}
		echo "
Remove diskette $NAME $VOL of $NVOLS."
		VOL=`expr $VOL + 1`
	done
}


:	-LoadFromInet
#		read a subset from internet
#
#	given:	full GLOBAL context
#	does:	read a subset from internet onto the system
#	return:	non-zero on failure

LoadFromInet()
{ (
	# read the rismap to determine which directory to search for this
	#  subset
	set xx `egrep $S $_TDIR/rismap`; shift
	[ $# = 0 ] && 
	{
		Error "LoadFromInet($S): cannot open rismap"
		set | WriteLog
		return 1
	}
	DIR=$1

	# unpack the subset from the server onto the system
	Ticker on
	rsh $_SRC -l ris -n "dd if=$_SYSTEM/$DIR/$S bs=10k" 2> /dev/null |
		$DECOMP | tar xpbf 20 -
	RETVAL=$?
	Ticker off
	return $RETVAL
) }


:	-LoadFromMedia
#		load a subset from the installation medium
#
#	given:	$1 - the name of the subset to install
#		$DEVICE - device (location) name to use
#		$_MEDIA - media type
#		$_SRC - media handle to use (named unit, hostname, etc)
#	does:	Load and verify the subset
#	return:	nothing
#	effect: may modify globals ALTOP, _CPOS, _CVOL

LoadFromMedia()
{	S=$1

	ERR=0
	CTX=$SMDB/$S

	cd $_R

	Log "$S LOAD"

	ReadCtrlFile $_TDIR $S ||
	{
		Error "Error reading control file"
		return 1
	}

	ISOPT=`FlagsAttrCheck SATTR_OPTION $FLAGS`

	# check sizing (only if not in update installation) 
	[ "$UPDFLAG" ] ||
	{
		echo "
SUBSET $S at $_R:" >> $FITSETLOG
		fitset $_R < $_TDIR/$S.inv 
		case $? in
		1|2)	# miscellaneous error conditions or no more free space 
			Error "
There is sizing or access problem installing \"$SDESC\"."
			return `expr $ISOPT + 1`
			;;
		esac
	}

	# check dependency
	# report missing dependencies all at once + provide subset description
	# wherever possible.
	DEP_STATUS=0
	for DEP in $DEPS
	{
		[ "$DEP" = '.' ] && continue
	
		[ -f $SMDB/$DEP.lk ] ||
		{
			DEP_STATUS=`expr $DEP_STATUS + 1`
			[ $DEP_STATUS -eq 1 ] &&
			{
				# save value of SDESC (subset description of $S)
				SAVE_SDESC=$SDESC

				Error "
Error installing \"$SDESC\" ($S)
This subset requires following subset(s) to operate correctly:"
			}
			ReadCtrlFile $SMDB $DEP 2>/dev/null 
			Log "
	\"$SDESC\" ($DEP)"
			1>&2 echo "
	\"$SDESC\" ($DEP)"
		}
	}

	[ $DEP_STATUS -gt 0 ] &&
	{
		# restore value of SDESC
		SDESC=$SAVE_SDESC

		Error "
Please install required subset(s) first." 

		return `expr $ISOPT + $DEP_STATUS`
	}

	Log "$S SCP PRE_$ACT"
	ACT=PRE_$ACT $_TDIR/$S.scp ||
	{
		Error "Installation declined by subset control program (PRE_$ACT).
\"$SDESC\" ($S) will not be loaded."

		return `expr $ISOPT + 1`
	}

	# drop a .dw marker
	> $CTX.dw

	echo "
$SDESC"

	echo "   Copying from $_LOC ($_MEDIA)"

	# check if it's a compressed subset.
	DECOMP=cat
	NameParse LOC_ $S

	[ -f $_TDIR/$LOC_P$LOC_V.comp ] && DECOMP="compress -d"

	# run a sweep of the disk clearing away files which cannot
	#  be tarred over.
	tclear < $_TDIR/$S.inv ||
	{
	 	Error "System contains files which cannot be overwritten"
	 	return `expr $ISOPT + 1`
	}

	case $_MEDIA in
	disk)	LoadFromDisk ||
		{	# utter failure
			Error "
$E_FAIL

Subset $S may not have been installed properly on your system
but the rest of the installation will continue."
		}
		;;

	diskette)
		LoadFromDiskette
		;;

	inet)	LoadFromInet ||
		{
			Error "Load from $_SRC failed, subset $S"
		}

		;;

	tape)	LoadFromTape ||
		{
			Error "$E_FAIL

\"$SDESC\" ($S) may not have installed properly on your system (tar error)"
		}
	esac

	sync

	# Verify the subset
	Log "VERIFY, \c"
	echo "   Verifying"
	echo "
SUBSET $S at $_R:" >> $FVERIFYLOG

	TickWhile "fverify -y < $CTX.inv 2> /dev/null" &&
	{
		Log "SUCCEEDED"

		# verify succeeded, move lock and queue SCP
		[ "$UPDFLAG" -a -s $CTX.lk ] &&
		{
			# if updating to the same version and there were 
			# layered product lock(s), transfer them over now.
			egrep -v '^OSF' $CTX.lk > $CTX.dw
		}
		mv $CTX.dw $CTX.lk

		return 0
	}
	# Verify Failed

	Error "
There were verification errors for \"$SDESC\" ($S)"

	case "$ISOPT$CMDSW" in
	0l)	return 1
		;;
	*)	return 2
	esac
}



:	-LoadFromTape
#		Load a subset from tape to the system
#
#	given:	full GLOBAL context
#	does:	read the subset from the tape to the system
#	return:	non-zero on failure

LoadFromTape()
{
	set xx `Parse : $MTLOC`; shift
	MTVOL=$1
	MTLOC=$2

	PositionTape $1 $2

	TickWhile "dd if=$RAW bs=10k 2> /dev/null | $DECOMP | tar x${VBSE}pf -"

	RET=$?
	[ $RET = 0 ] && _CPOS=`expr $_CPOS + 1`

	return $RET
}


:	-Main
#		This is the main program
#
#	given:	$* from command line
#	does:	parse arguments and all sorts of miscellany
#	return:	does not return

Main()
{

	CDPATH= ;export CDPATH		# assure no surprises.

	ISL_PATH=${ISL_PATH:-/isl}
	PATH=$ISL_PATH:/sbin:/usr/lbin:/usr/sbin:/usr/bin:.
	export PATH

	InitializeConstants
	InitializeGlobals

	trap '
		trap "" 3
		Log "Non-Standard Exit"
		Ferror 1 "Exiting"
	' 1 2 3 15

	[ -t 1 ] && stty -tabs		# permit tabs use on output if isatty
	DATE=`date $DATFMT`

	# Set tar verbose switch if necessary
	case "$-" in
	*x*)	VBSE=v
		DBG_FITSET=-d
		DEBUG=1
		Error "$PROG: debug: debug enabled"
	esac

	umask 22

	(Mkdir /$LOGDIR)		# needed for logging
	OpenLog			# initialize logging

	ARGV=$*
	Args $ARGV || Ferror $? "error in Args()"
	(Dirs) || Ferror $? "error in Dirs()"

	case $CMDSW in
	c)	cd $_R
		Configure $ARGV
		Exit $?
		;;

	d)	Delete $ARGV
		Exit $?
		;;

	h)	Usage -h
		Exit 0
		;;

	i)	Inventory $ARGV
		Exit $?
		;;

	l)	OPERATION=install

		InitDevice || Ferror $? "cannot initialize $DEVICE"

		cd $_R

		# read instctrl info from media to $_TDIR
		LoadCtrlInfo ||
			Ferror $? "cannot load control information"

		set xx $ARGV; shift

		# prepare for the installation and set global $SBS:
		#	- present menu,
		#	- copy over instctrl information, etc. 
		# 	- check system space
		#	- order $SBS by dependency 
		#	- prepare/save files for update installation.
		#
		PrepInstall $* ||
			Exit $?

		Install $SBS 
		Exit $?
		;;

	v)	Verify $ARGV
		Exit $?
		;;

	x)	OPERATION=extract
		InitDevice || Ferror $? "cannot initialize $DEVICE"

		LoadCtrlInfo ||
			Ferror $? "cannot load control information"

		set xx $ARGV; shift
		if [ $# = 0 ] 
		then
			SplitByType || Exit $?
			TickWhile PreSize extract $MAND ||
			{
				Error "
There is not enough file system space to extract the mandatory subsets"
				Exit 1
			}
			MenuSelect "$MAND" "$OPT" subset || Exit $?
		else
			SBS=$*	
			TickWhile PreSize extract $SBS ||
			{
				Error "
There is not enough file system space to extract the specified subsets"
				Exit 1
			}
		fi

		SBS=`(cd $_TDIR; DependencyOrder $SBS)`

		Extract $SBS
		Exit $?
		;;
	esac

	EMESG=
	Exit 0
}

:	-MenuListInit
#		seperate objects into two piles (one with category specified
#		in description and one without) and then sort objects 
#		in each pile.
#	        (category is the first sort key and subset description is the
#		 second sort key)  
#	given:	1) list containing subsets or packages
# 	does:	1) seperate according to existence of category specification. 
#		2) sort the objects according to their descriptions.
#	return: a list of subset ids (subsets with category precede those
#		without category.  
#

MenuListInit()
	 
{	

	# sort objects in the list according to the subset/package description
  	# in ascending alphabetic order 

	LIST=$1
        [ `Length $LIST` -gt 0 ] &&
	{
		> $TMP1
        	for X in $LIST  
        	{
			eval DESC=\$DESC$X
			echo "$DESC%$X" >> $TMP1 
        	}

		# group descriptions with category and echo back subset id 
		egrep '.+%.+%.+' $TMP1 | sort -t% +1 -2 | awk -F% '{print $3}' 	

		# group descriptions without category and echo back id  
		egrep -v '.+%.+%.+' $TMP1 | sort | awk -F% '{print $2}' 	
	}
}

:       -MenuSelect
#               select which subsets(packages) are to be installed/extracted
#
#       given:  1) list of mandatory subsets to be listed before presenting menu.
#               2) list of optional subsets from which to select
#               3) the string 'subset' (or 'package') to make the distinction.
#       does:   display the mandatory. offer selection of the optional.
#               recieve list of selected menu item and confirm selection.
#               set global SBS to reflect the selection.
#       return: 1 if "Nothing from this menu" is selected, else return 0
#       effect: global $SBS or $PKGS - change to list the mandatory prepended to
#               selected optionals.


MenuSelect()
{

        _MAND=$1
        _OPT=$2
        OBJECT=$3

        case "$OBJECT" in
        #package)        OBSW="-p"
        #                ;;
        subset)         OBSW="-s"
                        ;;
        esac

        ISLSW=
        [ "$ADVFLAG" ] &&
                ISLSW="-i"

	# export all necessary subset/package descriptions.
        for X in $_MAND $_OPT  
        {
                [ "$OBJECT" = subset ] &&
                        ReadCtrlFile $_TDIR $X
                export DESC$X
	}

	# sort subsets according to the subset description
  	# in ascending alphabetic order 
	_MAND=`MenuListInit "$_MAND"`
	_OPT=`MenuListInit "$_OPT"`
        export _MAND _OPT

	while :
	do
        	stlmenu -$CMDSW $OBSW $CONTSW $ISLSW -m $MENU select _MAND _OPT || Exit $?

        	MENULIST=`cat "$MENU"`

        	# nothing was selected from the menu
        	case "$MENULIST" in
        	"")     case "$OBJECT" in
                	subset)         SBS=
                                	;;
                	#package)        PKGS=
                	#                ;;
                	esac
                	return 1
        	esac
	
        	# something gets selected

        	# presize the total selection only if any optional
        	# has been selected. if MENULIST is contained by MAND, since
        	# we've already presized MAND, there's no need to presize
        	# again
	
        	ListContained "$MENULIST" "$MAND" || 
		{
			echo "
Checking file system space required to $OPERATION selected subsets:"

                	TickWhile PreSize $OPERATION $MENULIST 
			case $? in
			0) 	echo "
File system space checked OK."
				;;
			1)	Error "
There is sizing or access problem ${OPERATION}ing the selected subsets.
Please press RETURN to start another selection."
				read X
				continue 
				;;
			2)	Error "
There is not enough file system space to $OPERATION all the subsets 
you have selected. Please press RETURN to start another selection."
				read X
                        	continue
				;;
			3)	while :
				do
					Error "
<warning> ${OPERATION}ing all the subsets you have selected will cause file 
system(s) to be (over) 100% full. Do you really want to do this (y/n): \c"
					read X
					case "$X" in
					[Yy]*)  break
						;;
					[Nn]*)  continue 2
						;;   
					esac
				done
				;;
			esac
		}

        	case "$OBJECT" in
        	subset)         SBS="$MENULIST"
                        	;;
        	#package)        PKGS="$MENULIST"
        	#                ;;
        	esac
		break
  	
	done

        return 0
}


:	-Mkdir
#		make a directory and all dirs the lead to it.
#
#	given:	$* - pathnames
#	does:	for each pathname, create a directory and any needed
#		intermediary directories
#	return:	void
#	warning:	FIXED PATHS ONLY

Mkdir()
{
	ARGL=$*
	for DIR in $ARGL
	{
		set xx `Parse / $DIR`
		shift
		FP=
		for PCMP in $*
		{
			FP=$FP/$PCMP
			mkdir $FP 2> /dev/null
		}
	}
}


:	-MoveMrg
#		move update merge scripts .mrg..<file> to #.mrg..<file>
#		also remove the inventory entry.
#
#	given:	$1 - subset name
#	does:	for all of the .mrg.. files in the subset inventory,
#		prefix the filenames with #'s so they can be removed
#		by user/cron later. The inventory record is also removed
#		from the .inv.
#	return:	NIL

MoveMrg()
{ (

	SUBSET=$1
	egrep -s '/\.mrg\.\.' $SMDB/$SUBSET.inv && 
	{
	
		awk '{print $10}' $SMDB/$SUBSET.inv | egrep '/\.mrg\.\.' |
			while read FILE
			do
				[ -f $FILE ] &&
				{
					_D=`Dirname $FILE`
					F=`Basename $FILE`
					mv $FILE $_D/#$F	
				}
			done
 
		# update the inventory file (.inv)
		egrep -v '/\.mrg\.\.' $SMDB/$SUBSET.inv > $TMP1
		mv $TMP1 $SMDB/$SUBSET.inv
	}
) }

:	-MoveUpd
#		move .upd..<file> into the real name space and update
#		the inventory entry in .inv.	
#
#	given:	$1 - subset name
#	does:	for all of the .upd.. files in the subset inventory,
#		move them to the real name space and update the inventory.
#	return:	NIL

MoveUpd()
{ (

	SUBSET=$1
	egrep -s '/\.upd\.\.' $SMDB/$SUBSET.inv && 
	{
	
		awk '{print $10}' $SMDB/$SUBSET.inv | egrep '/\.upd\.\.' |
			while read FILE
			do
				_D=`Dirname $FILE`
				F=`Basename $FILE`

				N=`expr $F : '\.upd\.\.\(.*\)'`
				mv $FILE $_D/$N
			done

		# update the inventory file (.inv) 
		sed 's/\.upd\.\.//' $SMDB/$SUBSET.inv | sort +9 -10 -o $SMDB/$SUBSET.inv

	}
) }


:	-NameParse
#		parse a subset name into components
#
#	given:	$1 - a variable name prefix to use
#		$2 - a subset name to parse
#	does:	parse the subset name in P (xxx), N (y*), & V (zzz) components
#	return:	nothing
#	effect:	assign the P component to ${1}P
#		assign the N component to ${1}N
#		assign the V component to ${1}V

NameParse()
{
	[ $# -lt 2 ] &&
	{
		Error "NameParse(): too few arguments ($#)"
		return 1
	}
	L_PREFIX=$1	# variable prefix to use
	L_S=$2		# subset name to parse

	#% Local Variables
	L_N=		# temporary subset name
	L_P=		# temporary product code
	L_V=		# temporary version code
	L_X=		# loop control

	#% Local Code
	# check to see if parse information has been cached
	eval NPC=\$Np$L_S
	[ "$NPC" ] &&
	{
		# assign cached values and return
		eval `(
			set $NPC
			shift
			echo "${L_PREFIX}P=$1 ${L_PREFIX}N=$2 ${L_PREFIX}V=$3"
		)`
		return 0
	}

	case "$L_S" in
	.|..|...|....|.....|......)
		Error "NameParse(): $L_S: subset names must be > 7 chars long"
		return 1
	esac
	L_N=`expr $L_S : '^...\(.*\)...$'` ||	# name component
	{
		Error "NameParse($L_S): cannot parse subset name"
		return 1
	}
	L_P=`expr $L_S : '^\(...\).*$'`	||	# product component
	{
		Error "NameParse($L_S): cannot parse product code"
		return 1
	}
	L_V=`expr $L_S : '^.*\(...\)$'` ||	# version component
	{
		Error "NameParse($L_S): cannot parse version version"
		return 1
	}

	# make the assignments
	for L_X in N P V
	{
		eval $L_PREFIX$L_X=\$L_$L_X
	}
	# cache the resutls
	NPC="$L_S $L_P $L_N $L_V"
	eval Np$L_S='$NPC'
	return 0
}


:	-OpenLog
#		establish logfile
#
#	given:	$LOGFLILE - default logfile
#		$WHOAMI - user name running this process
#		$DATE - invocation time as formatted by $DATFMT
#	does:	establishes a log file for the session and writes initial
#		entry for this invocation
#	return:	0
#	effect:	$LOGFILE - may modify
#		$WHOAMI - will set from output of whoami(1) if not set

OpenLog()
{
	WHOAMI=${WHOAMI:=`whoami`}
	[ "$WHOAMI" != root ] && LOGFILE=/dev/null

	# create initial entry for this run
	(Log "SETLD $$ $WHOAMI $DATE \c")
	return 0
}


:	-PositionTape
#		position tape to a specified location
#
#	given:	$1 - volume number to position to
#		$2 - setld readable file position on the tape to go to
#		global $_CPOS - current tape position
#	does:	position the tape to be able to read the specified volume
#		and file
#	return:	0 if all's well.
#		1 on failure.
#	effect:	global $_CVOL - may change to reflect new position
#		global $_CPOS - may change to reflect new position

PositionTape()
{
	NEWVOL=$1		# desired volume
	NEWPOS=$2		# desired position

	case "$NEWPOS" in
	-[123])	;;
	-*)	Ferror 1 "Unaccessible tape position $NEWPOS"
	esac

	# make sure the tape drive isn't being used...
	Wait MTPID

	#  1. get the correct volume mounted
	[ "$NEWVOL" != "$_CVOL" ] &&
	{
		# volume change logic
		echo "
Volume change. Rewinding tape...\c"

		TickWhile "mt -f $RAW rew && mt -f $RAW offl" ||
		{
			Error "
I can't dismount your tape. You will have to take the
tape drive off line manually."
		}

		echo "
Please remove tape volume $_CVOL and replace it with volume $NEWVOL."

		while :
		do
			Ready
			mt -f $RAW rew && break
		done
		_CVOL=$NEWVOL
		_CPOS=-$T_0
	}

	#  2. get the correct position in the volume
	MOVE=`expr $_CPOS - $NEWPOS`
	_CPOS=$NEWPOS

	case "$MOVE" in
	0)	# we're at it.
		;;
	-*)	# forward X files.
		MOVE=`Parse - $MOVE`
		TickWhile "mt -f $RAW fsf $MOVE"
		;;
	*)	MOVE=`expr $MOVE + 1`
		TickWhile "mt -f $RAW bsf $MOVE && mt -f $RAW fsf"
		;;
	esac
	return $?	# returns value of last exec in case
}


:	-PostLoad 
#		transition to proto state and call scp with POST_L 
# 
#	given:	a subset id	
#	does:	1) transition files to proto state 	
#		2) if in update installation, execute merge files
#		3) run scp POST_L
#	return:	0 is successful, 1 on error
#

PostLoad()
{
	SUBSET=$1

	[ -f $SMDB/$SUBSET.scp ] ||
		return 1

	ReadCtrlFile $_TDIR $SUBSET

	# transition to proto state
	CopyProto $SUBSET

	# if in update installation, execute merge files
	[ "$UPDFLAG" ] && UpdMerge $SUBSET ToProto

	Log "$SUBSET SCP POST_L \c"
	if ACT=POST_L $SMDB/$SUBSET.scp; then 
	{
		Log "SUCCEEDED"

		# if in update installation:
		# transfer subset dependency locks and remove predecessors' 
		# smdb files
		[ "$UPDFLAG" ] &&
			UpdCleanup $SUBSET

		# create subset locks in .lk of subsets depended upon.
		for K in $DEPS
		{
			[ "$K" = '.' ] && break
			echo "$SUBSET" >> $SMDB/$K.lk
	
			# take out duplicates
			sort -o $SMDB/$K.lk -u $SMDB/$K.lk
		}	

		return 0
	}
	else
	{
		Error "
\"$SDESC\" ($SUBSET) failed in subset control program (POST_L)." 
		[ -s $SMDB/$SUBSET.lk ] &&
		{
			echo "
This failure may adversely affect the operation of the following subsets:
"
			sort -u $SMDB/$SUBSET.lk
		}

		# remove .lk and create a corrupt lock
		rm -f $SMDB/$SUBSET.lk
		echo "SCP POST_L FAILED" > $SMDB/$SUBSET.dw

		return 1
	}; fi

}


:	-PrepInstall
#		prepare for subset installation.
#	given:	subset list specified on setld -l command line (may be empty)
#	does:	1) presents setld menu (if no subsets specified)
#		2) copy instctrl info to SMDB
#		3) check disk space
#		4) dependency ordering
#		5) prepare/save files for update installation.
#	effect:	modify (order) global $SBS
#	return: 0 if success
#		non-0 if failure
# 

PrepInstall() 
{
	ARGS=$*
	NOT_DEPSORT=0	# not sorted by dependency yet

	if [ `Length $ARGS` = 0 ] 
	then
		# no subsets specified on command line

		SplitByType || return $?

		# copy over instctrl info 
		CopyInstctrl "$MAND $OPT" 

		TickWhile PreSize install $MAND 
		case $? in
		1)	Error "
There is sizing or access problem installing the mandatory subsets."  
			return 1
			;;
		2)	Error "
There is not enough file system space to install the mandatory subsets."
			return 1
			;;
		3)	Error "
<warning> Installing the mandatory subsets will cause file system(s) 
to be (over) 100% full." 
			;;
		esac

		case "$ADVFLAG" in
		0|2)	# basic installation
			SBS=$MAND

                        [ `Length $SBS` -gt 0 ] &&
                        {
                        	for X in $SBS
                                {
                                	ReadCtrlFile $_TDIR $X
                                        export DESC$X
                                }

				SBS=`(cd $_TDIR; DependencyOrder $SBS)`
				NOT_DEPSORT=1

                                export SBS

	    			echo "
The following subsets will be installed:"

				stlmenu -$CMDSW -s -i display SBS || 
					return $?
			}
			;;
		*)	# present menu	
			MenuSelect "$MAND" "$OPT" subset || return $?
		esac
	else
		# subsets specified on command line, stored in $ARGS

		ARGS=`ListNoDup $ARGS`
		ListContained "$ARGS" "$SBS" ||
			Ferror 1 "specified subset(s) not available on media"

		if [ "$UPDFLAG" ] 
		then
			# in update installation, we copy instctrl info of all 
			# available subsets on the kit to SMDB 
			CopyInstctrl "$SBS" "$ARGS"

			# in update installation, updeng calls fitset
			# directly before invoking setld. So there's no
			# need to call PreSize again here.
 
		else
			# copy instctrl info of specified subsets only.
			CopyInstctrl "$ARGS"

			echo "
Checking file system space required to install specified subsets:"

			TickWhile PreSize install $ARGS
			case $? in
			0) 	echo "
File system space checked OK." 
				;;
			1)	Error "
There is sizing or access problem installing the specified subsets." 
				return 1
				;;
			2)	Error "
There is not enough file system space to install the specified subsets."
				return 1
				;;
			3)	Error "
<warning> Installing the specified subsets will cause file system(s) 
to be (over) 100% full."
				;;
			esac
		fi

		SBS=$ARGS	

	fi

	# order $SBS according to dependency.
	[ "$NOT_DEPSORT" = 0 ] &&
		SBS=`(cd $_TDIR; DependencyOrder $SBS)`

	# if update installation, save current copies (.new../.proto../conf) 
	# of protected files that need to be updated. 
	[ "$UPDFLAG" ] &&
		UpdSave $SBS

	return 0
}


:	-PreSize
#		determine is space is available to -l/-x a subset to
#	the system.
#
#	given:	$1 - operation description in {install,extract}
#		$2... - names of objects to be operated upon
#	does:	permit space to be measured before reading software
#		onto the system
#	return:	0 is operation can be performed in the available space,
#		1 if not.
#	effect:	none, closed context
#

PreSize()
{ (
	OPCODE=$1
	shift
	SUBSETS=$*

	[ "$SUBSETS" ] || return 0	# no subsets.

	case "$OPCODE" in
	extract)
		# use the image file information and sizes of .ctrl, .inv,
		#  and .scp files to determine if subset
		#  image will fit on the system

		# for each subset being extracted, get its .image record.
		#  place the image records in a temp file. At the same
		#  time, gather size information about the ctrl, etc. files
		> $TMP1
		SIZES=0
		for S in $SUBSETS
		{
			egrep $S $_TDIR/*.image >> $TMP1
			SIZE=`ls -s $_TDIR/$S.* | awk '{print $1}'`
			SIZES="$SIZES $SIZE"
		}

		# add up the sizes of all of the images
		ISIZE=`awk '
			BEGIN	{size=0}
				{size += $2}
			END	{print size}' < $TMP1`

		# add up image size and all individual file sizes
		SIZE=0
		for X in $SIZES $ISIZE
		{
			SIZE=`expr $SIZE + $X`
		}

		# get freespace from system
		FREESPACE=`df -k . | awk 'NR == 2 {print $4}'`

		[ "$SIZE" -lt "$FREESPACE" ] && return 0
		return 1
		;;

	install)
		echo "
SUBSET $SUBSETS at $_R:" >> $FITSETLOG
		# cram all of the specified inventory files thru fitset
		> $_TDIR/allinv 
		for S in $SUBSETS
		{
			cat $_TDIR/$S.inv >> $_TDIR/allinv 
		}
		fitset $_R < $_TDIR/allinv && return 0
		return $? 
		;;
	*)	Error "PreSize(): $OPCODE: unknown opcode"
	esac
) }


:	-ReadCtrlFile
#		Initialize control file values
#
#	given:	$1 - pathname for directory to search for control file
#		$2 - name of subset
#	does:	read the file and verify it's contents
#		saves the control attributes
#	return:	1 on error
#	effect:	sets DESC, SDESC, NVOLS, MTLOC, DEPS, FLAGS
#			sets \$DESC$2....
#

ReadCtrlFile()
{
	# initialize, clean up residual values
	DESC=
	NVOLS=
	MTLOC=
	DEPS=
	FLAGS=

	# this variable stores the subset description without the category
	SDESC=

	[ $# = 2 ] ||
	{
		Error "ReadCtrlFile($*): expected 2 args, recieved $#"
		return 1
	}
	L_DIR=$1	# directory to search
	L_SUB=$2	# subset name

	# check to see if the information is already cached 
	eval SET=\$SET$L_SUB
	[ "$SET" = 1 ] &&	# retrieve the info
	{
		eval DESC=\$DESC$L_SUB
		eval SDESC=\$SDESC$L_SUB
		eval NVOLS=\$NVOLS$L_SUB
		eval MTLOC=\$MTLOC$L_SUB
		eval DEPS=\$DEPS$L_SUB
		eval FLAGS=\$FLAGS$L_SUB
		return 0
	}

	# info not cached, read and cache
	[ -f $L_DIR/$L_SUB.ctrl ] ||
	{
		Error "ReadCtrlFile(): cannot find $L_DIR/$L_SUB.ctrl"
		return 1
	}
	. $L_DIR/$L_SUB.ctrl ||
	{
		Error "ReadCtrlFile(): error reading $L_DIR/$L_SUB.ctrl"
		return 1
	}

	# did we get it all?
	case "~$DESC~$NVOLS~$MTLOC~$DEPS~$FLAGS~" in
	*~~*)	# Control File appears to be incomplete
		Error "ReadCtrlFile(): $L_DIR/$L_SUB.ctrl is incomplete"
		return 1
	esac

	# filter out category in the subset description
	SDESC=`echo $DESC | sed 's/%.*//'`

	# preserve the information
	eval DESC$L_SUB='$DESC'
	eval SDESC$L_SUB='$SDESC'
	eval NVOLS$L_SUB='$NVOLS'
	eval MTLOC$L_SUB='$MTLOC'
	eval DEPS$L_SUB='$DEPS'
	eval FLAGS$L_SUB='$FLAGS'

	# mark the subset as 'set'
	eval SET$L_SUB=1
	return 0
}



:	-SplitByType
#		split subset list into optional and mandatory
#
#	given:
#	does:
#	return:

SplitByType()
{
	MAND="" # list of mandatory subsets
	OPT=""  # list of optional subsets

	# scan control files, differentiate OPT & MAND subsets
	INSTALLED=		# smu-2290
	for _S in $SBS
	{
		case "$CMDSW" in
		l)
			# make sure subset is not installed.
			[ -f $SMDB/$_S.lk ] && 
			{
				INSTALLED=1
				continue
			}
			;;
		x)	# Make sure subset is not already on server
			[ -f $_S ] &&
			{
				INSTALLED=1
				continue
			}
		esac

		ReadCtrlFile $_TDIR $_S

		# if STL_NOACTM is not set, run scp with M action.
		#  Will fail if subset does not want to appear on the menu.

		[ "$STL_NOACTM" ] ||
		{
			ACT=M $_TDIR/$_S.scp -$CMDSW || continue
		}

		# bit 2 on flags means subset is optional
		case `FlagsAttrCheck SATTR_OPTION $FLAGS` in
		1)	OPT="$OPT $_S"
			;;
		*)	MAND="$MAND $_S"
		esac
	}
	case "$MAND$OPT$INSTALLED" in
	"")	Error "No ${OPERATION}able subsets on your kit"
		return 1
		;;
	1)	Error "All subsets on the kit are already ${OPERATION}ed"
		return 1
	esac
}


:	-UpdCleanup
#		relock dependency for layered products and remove outdated
#		smdb files (.inv/.ctrl/.scp/.lk)
#
#		Note: this is only called during an update installation.
#
#	given:	a subset id and global $SCPLIST.
#	does:	1. carry over dependency locks logged by layered products
#		   in its predecessor(s)'s .lk.
#		2. remove smdb files of its predecessor(s). 
#

UpdCleanup()
{ (
        SUBSET=$1

	# subset id without version, e.g. OSFBASE	
        NO_VER=`expr "$SUBSET" : '\(.*\)[0-9][0-9][0-9]'` 

	# get the list of subsets known to the system (with .ctrl present)
	ONSYS=`ListKnown $NO_VER`
	# filter out self

	SLIST=`ListUniq "$ONSYS" "$SUBSET"`
	# Note:	the above makes the routine a no-op for updating to the same 
	# 	version code. When updating to the same version code, lock
	#	transfer is done in LoadFromMedia() when .lk is created.

        for K in $SLIST
	{
		# if there is a non-empty lock file
		[ -s $SMDB/$K.lk ] &&
		{
			# first take out duplicates. Duplicated locks
			# are eliminated starting V1.4
			sort -o $SMDB/$K.lk -u $SMDB/$K.lk
		
			# transfer layered product locks.
			# OSF subset locks are already logged in PostLoad().
			egrep -v '^OSF' $SMDB/$K.lk >> $SMDB/$SUBSET.lk
		}

		# Presumably if a predecessor is also in the update list
		# (as in the case of ris), $SUBSET is a mup subset and 
		# we do not want to remove info of its parent subset.

		Member $K $SCPLIST ||
			rm -f $SMDB/$K.*
	}
) }

:	-UpdMerge
#		execute the merge files in the specified subset
#	given: 	1) a subset id
#		2) merge option to pass to the merge file, i.e., ToProto or
#		   ToConfig.
#	does:	For each .mrg..filename listed in the .inv (excluding the
#		.mrg..xxx.dat),, execute the merge with specified option.
#
#	return: 0 

UpdMerge()
{ (
	SUBSET=$1
	OPTION=$2

	MRGFILES=`egrep '/\.mrg\.\.' $SMDB/$SUBSET.inv | 
		  egrep -v '\.dat[ 	]' | awk '{print $10}'`

	for K in $MRGFILES
        {
		_D=`Dirname $K`
                MF=`Basename $K`     # .mrg.. file
                CF=`expr $MF :  '\.mrg\.\.\(.*\)'` # customer's copy
		NF=".new..$CF"
		PF=".proto..$CF"
 
		# no customization was made or ToProto merge failed.
		# Do explicit propagation.
		[ -f $_D/$CF.passMRG ] &&
		{
			case "$OPTION" in
			ToProto) 	[ -f $_D/$NF ] &&
						cp -p $_D/$NF $_D/$PF
					;;
			ToConfig)	[ -f $_D/$PF ] &&
						cp -p $_D/$PF $_D/$CF
					# cleanup the mark
					rm -f $_D/$CF.passMRG 	
					;;
			esac
			continue
		}

		# there was customization but the new .new..
		# is the same as the old one. Just skip the merge.
		# CopyProto/CopyConfig did/will not propagate.
		[ -f $_D/$CF.keepCUS ] &&
		{
			[ "$OPTION" = ToConfig ] &&
				rm -f $_D/$CF.keepCUS
			continue
		}

		# merge needed,  provide message banners
		case "$OPTION" in
		ToProto)	FILE=$PF
				;;
		ToConfig)	FILE=$CF
				;;
		esac

		echo "
	*** Merging new file $_D/$NF into 
		    existing $_D/$FILE
"
		# invoke merge script
                (cd $_D; ./$MF $OPTION $CF) 
		RET=$?

		# so we can use MRG_Echo
		. $SHELL_LIB/libmrg

		case $RET in
		0)	# merge successful 
			echo
			MRG_Echo "Merge completed successfully."
			[ "$OPTION" = "ToProto" ] &&
				continue
			;;
		*)	# merge failed 
			MRG_Echo "Unable to complete this merge."

			[ -f $_D/$FILE ] &&	
				mv $_D/$FILE $_D/$FILE.FailMRG

			case "$OPTION" in
			ToProto)	# copy newly delivered .new..
					# to .proto..		
					cp -p $_D/$NF $_D/$PF

					# prevent further ToConfig merge
					> $_D/$CF.passMRG

					MRG_Echo "$NF is copied directly to $PF and $CF." 
					# log the failure
					echo "$_D/$PF"  >> $UPDMRGLOG
					;;
			ToConfig)	# copy new .proto.. 		
					cp -p $_D/$PF $_D/$CF
					MRG_Echo "$PF is copied directly to $CF."
					;;
			esac

			# log the failure
			echo "$_D/$CF" >> $UPDMRGLOG

			echo
			MRG_Echo "Your old file(s) are saved to:"
			[ "$OPTION" = ToProto ] &&
				MRG_Echo "\t$_D/$PF.PreMRG" 
			MRG_Echo "\t$_D/$CF.PreMRG" 

			[ -f $_D/$FILE.FailMRG ] &&	
			{
				echo
				MRG_Echo "The file resulting from the merge attempt is saved to:"
				MRG_Echo "\t$_D/$FILE.FailMRG"
			}

			echo
			MRG_Echo "Merge your customization into following file(s) manually:"
			[ "$OPTION" = ToProto ] &&
				MRG_Echo "\t$_D/$PF"
			MRG_Echo "\t$_D/$CF"

			echo
			MRG_Echo "Files that do not merge successfully are recorded in:"
			MRG_Echo "\t./$UPDMRGLOG"

			;;
		esac

		# remove extra backup copies.
		rm -f $_D/$NF.PreMRG 

        }

	return 0
) }



:	-UpdMoreOpt
#		more optimization for update installation merges.
#
#	given:	a subset
#	does:	Goes through newly delivered merge scripts in a subset,
#		if the target file was customized (indicated by the
#		absence of .passMRG), check if the new .new.. is the
#		same as the old one (.new..file.PreMRG). If they are the
#		same, leave a mark for UpdMerge() to pick up.
#	return:	0	
#

UpdMoreOpt()
{
	SUBSET=$1

	MRGFILES=`egrep '/\.mrg\.\.' $SMDB/$SUBSET.inv | 
		  egrep -v '\.dat[ 	]' | awk '{print $10}'`

	for K in $MRGFILES
        {
		_D=`Dirname $K`
                MF=`Basename $K`     # .mrg.. file
                CF=`expr $MF :  '\.mrg\.\.\(.*\)'` # customer's copy
		NF=".new..$CF"
		PF=".proto..$CF"

		[ -f $_D/$CF.passMRG ] &&
		{
			# there was no customization (no merge necessary)
			continue
		}
 
		[ -f $_D/$NF.PreMRG ] || 
		{
			# no previous version of .new.. saved (can't optimize
			# further)
			continue
		}

		# further optimization: check if newly delivered
		# .new.. is the same as the old one or not.
                # Note: works with ASCII files only.

		# first filter out RCS strings 
		egrep -v '^# @\(#\)\$RCSfile' $_D/$NF 	     > $TMP1 
		egrep -v '^# @\(#\)\$RCSfile' $_D/$NF.PreMRG > $TMP2 

		# then diff the real contents of the files
                diff $TMP1 $TMP2 >/dev/null 2>&1
		[ $? = 0 ] && 
		{
			# drop a mark to bypass merge process and
			# keep the customization.
			> $_D/$CF.keepCUS
		
			# remove backups since $PF and $CF
			# will remain unchanged (customized).
			rm -f $_D/.*..$CF.PreMRG $_D/$CF.PreMRG
		}
	}

	return 0
}


:	-UpdSave
#		before update installation starts, for each protected file
#		to be updated, check if merge is needed. If yes, save its old 
#		.new../.proto.. and configured files.
#
#	given:	a list of subsets
#	does:	If protected files are customized, saves their .new../.proto.. 
#		and configured copies to files postfixed with .PreMRG 
#		Also reset $UPDMRGLOG.
#	return:	0	
#

UpdSave()
{ (
	SUBSETS=$*

	for K in $SUBSETS
	{
		# get list of protected files to be updated by checking
		# the existence of .mrg.. files.
		awk '{print $10}' $_TDIR/$K.inv |
			egrep '/\.mrg\.\.' | egrep -v '\.dat$' | 
			sed 's/\.mrg\.\./\.new\.\./' |
		while read FILE
		do
			_D=`Dirname $FILE`
			NF=`Basename $FILE`	# .new.. file
			CF=`expr $NF : '\.new\.\.\(.*\)'` # configured file	
			PF=".proto..$CF"	# .proto file

			# optimization: if no customization was made
			# in this file, no merge is needed later, i.e.,
			# normal state transition can occur.
			diff $_D/$NF $_D/$CF >/dev/null 2>&1
			case $? in
			0)	# if = 0, no difference found, which means
				# no customization was made.
				
				# drop a mark to bypass merge process.
				> $_D/$CF.passMRG
				;;
			*)	# if = 1, there is customization
				# save files for merge purpose.

				# if = 2, diff returns with an error, eg,
				# $NF or $CF may not exist. In this case,
				# we assume there is customization just 
				# to be safe.
 
				# save .new..
				[ -f $_D/$NF ] &&
					cp $_D/$NF $_D/$NF.PreMRG 
	
				# save .proto..
				[ -f $_D/$PF ] &&
					cp $_D/$PF $_D/$PF.PreMRG 

				# save customer's configured version
				[ -f $_D/$CF ] &&
					cp $_D/$CF $_D/$CF.PreMRG
				;;
			esac
		done
	}

	# initialize error log.
	> $UPDMRGLOG

	return 0
) }


:	-Usage
#		print usage messages
#
#	given:	$1 - switch for which message to print
#	does:	prints a particular usage message
#	return:	nothing

Usage()
{
	case "$1" in
	"")	Error -n "$USAGE"
		;;
	-c)	Error -n "
Usage:
$CUSAGE"
		;;
	-d)	Error -n "
Usage: $DUSAGE"
		;;
	-h)	echo "$USAGE"
		;;
	esac
	return 0
}


:	-Verify
#		verify the integrity of the installation subsystem
#
#	given:	$* - list of subsets to be verified 
#	does:   For each subset specified:
#		1) check if subset is installed.
#		2) read control file to get subset description 
#		3) test ivp
#	return: nothing

Verify()
{
	cd $_R
	ERR=0
	for _S in $*
	{
		[ -f $SMDB/$_S.lk ] ||
		{
			Error "$_S: $E_NOINST"
			ERR=1
			continue
		}

                # read .ctrl to get subset description ($SDESC), 
                # ReadCtrlFile() will provide proper error msg when needed.
		ReadCtrlFile $SMDB $_S  

		Log "$_S SCP V \c"
		echo "
$SDESC ($_S)"

		ACT=V $SMDB/$_S.scp ||
		{
			Error "ivp failed."
			ERR=1
			continue
		}

		Log "SUCCEEDED"
	}
	return $ERR
}



#% CODE
#	actual code begins here. This is structured this way to
#	enable interactive debugging of the independent subroutines
#	by setting CHECK_SYNTAX to something and running '. setld'

[ "$CHECK_SYNTAX" ] || Main "$@"



