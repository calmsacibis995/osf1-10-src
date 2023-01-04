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
# @(#)$RCSfile: RisFiles.sh,v $ $Revision: 1.1.9.13 $ (DEC) $Date: 1993/08/10 19:30:35 $
# 
#
#	RisFiles - Extracts files needed for ris loading
#	
#	This file is only used on OSF ris servers, ULTRIX is ignorant
#	on its use.  
#	Call:	RisFiles Extract ROOT_PATH  [FIXIT PROG]
#		RisFiles Delete 
#
#	Parameters:	ROOT_PATH - Full path name to ROOT or hROOT image
#			FIXIT	  - Full path name to run fixit (mips)
#	
#	Return:	echo out which file to boot
#

# PATH *must* contain pieces for both ULTRIX and DEC OSF/1 environments

RIS=/var/adm/ris
PATH=".:/bin:/etc:/usr/etc:/usr/etc/stl:/usr/sbin:/usr/bin:/usr/lbin:/sbin"

export PATH RIS

NULL=/dev/null
KERNEL="vmunix"

umask 022

:       -Error
#               print a message on stderr
#
#       args:   $* - message strings to be printed
#       does:   print message strings on file descriptor 1
#       return: nil
#
#	Note:	defined here (instead of sourcing from SHELL_LIB) to
#		accommodate ULTRIX servers.

Error()
{
        1>&2 echo "$*"
}


:	-Extract
#		Extract files needed for ris.
#
#	given:
#	does:
#	return:

Extract()
{
	# Check if KERNEL file is there, otherwise extract from 
	# $SUBPROD/$ROOTTYPE

        [ -f $KERNEL ] ||
	{
		# get the correct boot kernel for the default protocol
		#  of the server

		case "$UNAME" in
		ULTRIX)
                        ExtractFile $ROOT $KERNEL.sas
                        mv $KERNEL.sas $KERNEL
			;;
		OSF1)
                        ExtractFile $ROOT $KERNEL.bootp
                        mv $KERNEL.bootp $KERNEL
                	;;	
		esac
        }

	# Check if BOOTFILE is needed and if it is in current 
	# directory, extract if available

	[ $UNAME = ULTRIX ] &&
	{
		ExtractFile $ROOT $BOOTFILE
		# These were part of Dan F.'s original implementation
		#  they'll probably be removed after he responds to
		#  mail sent 6/30/93

		# echo "KERNELIMAGE:$RISROOT/vmunix" > nettext
		# echo SERVERNAME:`/sbin/hostname` >> nettext
	}

	# extract the /isl directory from the ROOT
	#! must be updated to support hardware releases

	[ -d kit/isl ] ||
	{
		[ -d kit ] || mkdir kit	
		(cd kit;restore xf - isl) < $ROOT
	}
	
	return 0
}



:	-ExtractFile
#		Extract file from ROOT or HROOT, $1 is path, $2 is
#		file to extract

ExtractFile()
{
	ROOTFILE=$1
	EXFILE=$2
	touch tnc || {
		Error "
The $RISROOT directory is not writable
and no \"$EXFILE\" file exists.  therfore, $CLIENT has not
been registered in the bootp database."
	return 1
	}
	rm tnc

	Echo "\nExtracting $EXFILE ...\c"

	# Create an input file for restore
	restore -xf $ROOTFILE $EXFILE 2>$NULL || {
		Error "There is no \"$EXFILE\" file in $ROOTFILE."
		return 1
	}
	Echo "done."
	return 0
}


:	-Delete
#		remove the files associated with a product area
#
#	given:
#	does:
#	return:

Delete()
{
	# Delete files extracted earlier, KERNEL and BOOTFILE
	rm -f ./$KERNEL
	[ "$BOOTFILE" ] &&
	{
		rm -f ./$BOOTFILE
	}
	rm -rf kit
}


:	-Echo
#		Echo to program calling

Echo()
{
	1>&2 echo $*
}



: Main
#
# Main program starts here
#

Main()
{
	CMD=$1
	ROOT=$2

	RISROOT=`pwd`
	ARC=`basename $RISROOT`
	ARC=`expr $ARC : '.*\.\(.*\)'`
	UNAME=`uname`

	[ "$UNAME" = ULTRIX ] && BOOTFILE=netload

	case "$CMD" in 
	Extract)
		Extract
		;;
	Delete)
		Delete
		;;
	esac

	[ "$BOOTFILE" ] || BOOTFILE=$KERNEL

	echo "$BOOTFILE"
	exit $?
}


:	-Parse
#		break $2... using $1 as a separator
#

Parse()
{(
	IFS=$1
	shift
	echo $*
)}


:	-ToUpper
#		convert to upper case
#

ToUpper()
{
	echo $* | dd conv=ucase 2> /dev/null
}



[ "$CHECK_SYNTAX" ] || Main "$@"

