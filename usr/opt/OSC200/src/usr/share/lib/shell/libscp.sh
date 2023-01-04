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
#
#	libscp.sh - shell library for painless scp coding
#
#	NOTE:
# 		Refer to "Creating and Managing Software Product Kits" 
#		(DEC OSF/1 Programming Support Tools) for recommended
#		usage and examples.
#
#
# @(#)$RCSfile: libscp.sh,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/11/19 14:52:47 $ 

#	000	30-apr-1991	ccb
#		Drawn from scpPak in ULTRIX BSD
#
#	001	04-dec-1991	ccb
#		Fix recursive call in STL_DepEval
#		Point to correct dir at /usr/.smdb.



# The DEPENDENCY ROUTINES

:	-STL_DepInit
#		Initialize global subs and vars needed for DepEval
#
#	given:	NIL
#	does:	initialize global DEPSTACK and declares functions
#		PushD() and PopD().
#	return:	NIL

STL_DepInit()
{
	DEPSTACK=

	:	PushD
	#		Push a dependency status code on DEPSTACK
	#
	#	given:	$1 - a dependency status code
	#	does:	push it on DEPSTACK
	#	return:	the Logical value of $1

	PushD()
	{
		DEPSTACK="$1 $DEPSTACK"
		[ $1 = 0 ]
	}

	:	PopD
	#		Pop a dependency status code from DEPSTACK
	#
	#	given:	NIL
	#	does:	Pop the top element from DEPSTACK
	#	return:	the logical value of the popped element

	PopD()
	{
		set -- $DEPSTACK
		V=$1
		shift
		DEPSTACK="$*"
		[ $V = 0 ]
	}
}


:	-STL_DepEval
#		Evaluate a DepExp
#
#	given:	$* - a dependency expression
#	does:	evaluate the expression using lock file info in
#		./usr/.smdb.
#	return:	0 is the expression is satisfied, 1 otherwise.
#	

STL_DepEval()
{
	ARGS=$*

	case "$1" in
	"")	PopD
		return $?
		;;
	and)	PopD; X=$?
		PopD; Y=$?
		case "$X$Y" in
		00)	PushD 0
			;;
		*)	PushD 1
		esac
		;;
	or)	PopD; X=$?
		PopD; Y=$?
		case "$X$Y" in
		11)	PushD 1
			;;
		*)	PushD 0
		esac
		;;
	not)	PopD
		[ $? = 1 ]
		PushD $?
		;;
	*)	[ -f usr/.smdb./$1.lk ]
		PushD $?
	esac
	set -- $ARGS
	shift
	STL_DepEval $*
}


# ARCHITECTURE ROUTINE

:	-STL_ArchAssert
#		Architecture Predicate
#
#	given:	$1 - an architecture name {mips,vax}
#	does:	determine if current context matches architecture
#	return:	0 on a match, 1 otherwise

STL_ArchAssert()
{
	ARCH=$1

	MACH=vax
	[ -f bin/machine ] &&
		MACH=`bin/machine`

	[ "$ARCH" = "$MACH" ]
}


# DEPENDENCY LOCKING ROUTINES

:	-STL_LockInit
#		Initialize Locking capabilities
#
#	given:	NIL
#	does:	establish LOCKSTACH and the PushL() and PopL() routines.
#	return:	NIL

STL_LockInit()
{
	LOCKSTACK=
	NUL=/dev/null

	:	PushL
	#		Push a lock entry
	#
	#	given:	$1 - a lock entry to push
	#	does:	Push the lock entry onto LOCKSTACK
	#	return:	NIL

	PushL()
	{
		LOCKSTACK="$1 $LOCKSTACK"
	}

	:	PopL
	#		Pop a lock name
	#
	#	given:	NIL
	#	does:	pop a LOCK entry from the stack
	PopL()
	{
		set -- $LOCKSTACK
		echo $1
		[ $# != 0 ] && shift
		LOCKSTACK=$*
	}
}


:	-STL_DepLock
#		Lock required subsets to system, reducing liklihood
#	of deletion by the customer.
#
#	given:	$1 - SUBSET NAME to lock down
#		$2 - $n DepExp describing it's dependencies
#	does:	locks all subsets which match the dependency list.
#	return:	NIL

STL_DepLock()
{
	SUBSET=$1
	shift
	ARGS=$*

	case "$1" in
	"")	X=`PopL`; PopL > $NUL
		case "$X" in
		"")	return 0
		esac
		STL_SetLock $SUBSET $X
		return 0
		;;
	and|or)	# Lock both of the top subsets on the stack
		X=`PopL`; PopL > $NUL
		STL_SetLock $SUBSET $X

		X=`PopL`; PopL > $NUL
		STL_SetLock $SUBSET $X
		;;
	not)	X=`PopL`; PopL > $NUL
		PushL ~$X
		;;
	*)	# Must be a subset name
		PushL $1
	esac
	set -- $ARGS
	shift
	STL_DepLock $SUBSET $*
}



:	-STL_DepUnLock
#		unlock a subset's dependencies
#
#	given:	$1 -- subset to unlock
#		$2 - $n -- DepExp describing the subset's dependencies.
#	does:	remove all locks this subset holds in the system
#	return:	NIL

STL_DepUnLock()
{
	SUBSET=$1

	shift
	for K
	{
		case "$K" in
		and|or)	# Unlock the first two on the stack
			X=`PopL`; PopL > $NUL
			STL_UnLock $SUBSET $X

			X=`PopL`; PopL > $NUL
			STL_UnLock $SUBSET $X
			;;
		not)	X=`PopL`;PopL > $NUL
			PushL ~$X
			;;
		*)	# must be a subset name
			PushL $K
		esac
	}
	X=`PopL`; PopL > $NUL
	STL_UnLock $SUBSET $X
	STL_LockInit
	return
}


:	-STL_UnLock
#		remove dependency locks
#
#	given:	$1 -- subset name holding locks
#		$2 -- subset to be unlocked
#	does:	Unlock dependent subsets
#	return:	NIL


STL_UnLock()
{ (
	cd usr/.smdb.

	SUBSET=$1
	for K in `echo $2.lk`
	{
		[ -f $K ] &&
		{
			egrep -v "^$SUBSET\$" $K > $K.tmp
			mv $K.tmp $K
		}
	}
) }
		

:	-STL_SetLock
#		create dependency locks
#
#	given:	$1 -- name of subset requesting locks
#		$2 -- name of subsets to be locked
#	does:	place locks on subsets
#	return:	NIL

STL_SetLock()
{ (
	cd usr/.smdb.

	SUBSET=$1
	for K in `echo $2.lk`
	{
		[ -f $K ] && echo "$SUBSET" >> $K
	}

) }


:	-STL_DefaultSCP
#		This is the "have it our way" subset control program.
#
#	given:	$* - original args given to the scp
#		then there are some preset global variables

STL_DefaultSCP()
{
	case "$ACT" in
	PRE_L)	;;
	POST_L)
		;;
	C)	
		set xx $ARGS
		shift

		case "$1" in
		INSTALL)
			;;
		DELETE)
			;;
		esac
		;;
	PRE_D)
		;;
	esac
}


# BACKWARD SYMBOLIC LINK ROUTINES 
#
# used when: 
#	1. creating backward symbolic links. i.e., from layered product
#   	   name space to the traditional UNIX file hierarchy, AND 
#	2. the dot relative path is not static. 
#
# For example: 
#	when making backward links from ./var to ./etc, the fact that
# 	./var may be under / or be a symbolic link to ./usr/var will affect the
# 	relative path to the installation root. By using these routines, the 
# 	relative path will be built dynamically.  
#

:	-STL_LinkInit
#		prepare to provide product link assistance
#
#	given:	nothing
#	does:	sets GLOBAL variable "LINKROOT".
#	return:	NIL 
#

STL_LinkInit()
{
	LINKROOT=`/bin/pwd`

}


:	-STL_LinkBack
#               create a backward symbolic link. 
#
#       given:  $1 == name of file to be linked
#               $2 == dot-relative directory containing the link's 
#                     referent file (source)
#               $3 == dot-relative directory to contain the link (target)
#	does:	create a symbolic link ($3/$1) to $2/$1 (real file) 
#	return: NIL	
#

STL_LinkBack () 
{ 

  	FILE=$1
  	SRCDIR=$2
  	DESTDIR=$3

  	RELPATH=`(cd $DESTDIR; STL_LinkGetPath)`

  	(cd $DESTDIR; ln -s $RELPATH/$SRCDIR/$FILE $FILE)
}


:	-STL_LinkGetPath
#		get a relative path to the top of the installation
#		tree.
#
#	given:	GLOBAL LINKROOT
#	does:	generate a relative path from the current directory
#			to the LINKROOT directory, writes the path
#			on stdout. 
#
#			Ex. echo $LINKROOT => / , var -> usr/var
#			    (cd ./var/adm; STL_LinkGetPath) => ../../..
#	return:	NIL	

STL_LinkGetPath()
{ (
	WD=`/bin/pwd`

	SUBPATH=`STL_PathXor $LINKROOT $WD`

	RELPATH=
	SEP=

	for K in $SUBPATH
	{
		RELPATH="$RELPATH$SEP.."
		SEP=/
	}
	echo $RELPATH
) }


:	-STL_PathXor
#		prints deeper path names in $2
#
#	given:	two absolute paths: 	
#		$1 being that of LINKROOT.
#	does:	compares the path levels in $1 and $2 and prints
#		path names in $2 (seperated by space) which are
#		deeper than the levels in LINKROOT.
#		
#			Ex. STL_PathXor /etc /usr/share/lib ==> share lib
#	return: NIL 

STL_PathXor()
{ (
	SHELL_LIB=${SHELL_LIB:-/usr/share/lib/shell}
	. $SHELL_LIB/Strings

	PPATH=`Parse / $1`
	set xx `Parse / $2`
	shift
	
	for K in $PPATH
	{
		shift
	}
	echo $*
) }



