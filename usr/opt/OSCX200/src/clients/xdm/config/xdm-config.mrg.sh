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
# @(#)$RCSfile: xdm-config.mrg.sh,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/10/01 15:14:34 $
# 
# 
# =========================================================================
# For update installation merges, there are two schemes you can use:
#
#	1. provide your own merge routine.
#		a) set MERGE_ROUTINE to the name of your merge routine. 
#	
#			Example: MERGE_ROUTINE=DRI_Merge
#	
#		b) define your merge routine. 
#	
# 			- given: global variable   
#				 	$_FILE - file name  
#			- return: 0 if success
#		    		  non-zero if failure
#
# 			Note: 	1. use MRG_Echo() to output additional messages.
#				2. see /usr/share/lib/shell/libmrg for other
#				   available global variables.
#	
#			Example:
#				DRI_Merge ()
#				{
#					RET=0
#
#					# merge operations, 
#					# use 'grep -v', 'ed', 'echo >>', etc. 
#					# set $RET to non-zero on failure.
#
#					return $RET
#				}
#
#	2. use the RCS 3-way merge utility. 
#		a) use the template as is 
#		b) if necessary, check for duplicates after merge, call 
#		   MRG_SetCheck at specified location.
#
#			Note: 	1. specify one check (a regular expression or a
#			  	   plain text string) in one call
# 				2. enclose regular expressions in single quotes
#				   (' ') and plain text strings in double quotes#				   (" ") as argument of MRG_SetCheck is to be 
#				   passed to 'egrep'
#
#			Example: 
#				MRG_SetCheck "auth"    
#				MRG_SetCheck '^auditd[ 	].*/tcp'
#
# =========================================================================
#
# specify name of your merge routine here if you are not using 3-way merge. 

MERGE_ROUTINE=MergeXdmConfig

SHELL_LIB=${SHELL_LIB:-/usr/share/lib/shell}
. $SHELL_LIB/libmrg

: MergeXdmConfig - 
#
#	Given:	Nothing
#
#	Does:	Performs andy merge processing that is necessary
#		during the C INSTALL phase of the subset installation.
#
#	Returns:	Nothing
#
MergeXdmConfig()
{

MRG_DIR="/usr/lib/X11/xdm"		# Full path of file to merge
MRG_FILE="xdm-config"	# Name of file to merge
#
	[ "$OPTION" = "ToProto" ] && return 0

	(grep -q "greeterLib" $MRG_FILE) &&
	{
	    return 0
	}
#
#
#	Insert merge code here
#
	ed - $MRG_FILE <<END 1>/dev/null
/accessFile
a
DisplayManager.greeterLib:	/usr/lib/X11/xdm/libXdmDecGreet.so
.
/_0.reset
a
DisplayManager.local_0.authorize:	true
DisplayManager.local_0.setup:	/usr/lib/X11/xdm/Xsetup_0
DisplayManager.local_0.startup:	/usr/lib/X11/xdm/GiveConsole
DisplayManager.local_0.reset:	/usr/lib/X11/xdm/TakeConsole
.
w
q
END

	if [ "$?" = 0 ]
	then
		return 0
	else
		return 1
	fi
	return
}


# OPTIONAL: for 3-way merge, make MRG_SetCheck calls here.

[ "$CHECK_SYNTAX" ] || MRG_Merge "$@" 

