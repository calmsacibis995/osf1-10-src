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
# @(#)$RCSfile: Xresources.mrg.sh,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1994/01/11 18:49:03 $
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

MERGE_ROUTINE=MergeXres

SHELL_LIB=${SHELL_LIB:-/usr/share/lib/shell}
. $SHELL_LIB/libmrg

# define your merge routine here. 

: MergeXres - 
#
#	Given:	Nothing
#
#	Does:	Performs andy merge processing that is necessary
#		during the C INSTALL phase of the subset installation.
#
#	Returns:	0 success, -1 failure
#
MergeXres()
{

MRG_FILE="Xresources"	# Name of file to merge
#
	[ "$OPTION" = "ToProto" ] && return 0

	(grep -q "LogoBackground" $MRG_FILE) &&
	{
	    return 0
	}
#
#
#	Insert merge code here
#
#	Create the V1.3 version
#
tmp_file=$$Xresources

cat - <<END > $tmp_file
!
! xdm resources
!

XConsole.text.geometry:	480x130
XConsole.verbose:	true
XConsole*iconic:	true
XConsole*font:		fixed

Chooser*geometry:		700x500+300+200
Chooser*allowShellResize:	false
Chooser*viewport.forceBars:	true
Chooser*label.font:		*-new century schoolbook-bold-i-normal-*-240-*
Chooser*label.label:		XDMCP Host Menu  from CLIENTHOST
Chooser*list.font:		-*-*-medium-r-normal-*-*-230-*-*-c-*-iso8859-1
Chooser*Command.font:		*-new century schoolbook-bold-r-normal-*-180-*

!
! dxlogin widget resources
!

#ifdef COLOR
*background:				#ca94aa469193
#endif

dxlogin.LogoY:				100
dxlogin.LogoColor:			rgb:8182/0604/2c28
dxlogin.logoBW:				Black
dxlogin.axpLogoColor:			Gold
dxlogin.axpLogoBW:			White
dxlogin.rootColor:			rgb:3030/5050/606

#ifdef COLOR
dxlogin*failColor:			red3
#endif
END
#
#	merge
# 

    PrepRealFile
    /usr/lbin/upd_merge $MRG_FILE $tmp_file .new..$MRG_FILE


	if [ "$?" = 0 ]
	then
		rm $tmp_file
		return 0
	else
		rm $tmp_file
		return 1
	fi

}


PrepRealFile()
{
    if cmp -s $MRG_FILE $tmp_file
    then
	return
    fi

    if grep -q "! A value of 0 for the LogoY" $MRG_FILE
    then
	ed - $MRG_FILE << EOF 1> /dev/null
/^! A value of 0 for the LogoY/d
i
! a value of -1 for LogoX or LogoY causes an appropriate default to be 
.
w
q
EOF
    fi

    sed -e '/^dxlogin.LogoY:[ 	]*0[ 	]*$/s/0/-1/' $MRG_FILE > \
		$MRG_FILE.edited

    mv $MRG_FILE.edited $MRG_FILE

    if grep -q dxlogin.LogoX $MRG_FILE
    then
	sed -e '/^dxlogin.LogoX:[ 	]*0[ 	]*$/s/0/-1/' \
			$MRG_FILE >  $MRG_FILE.edited
	mv $MRG_FILE.edited $MRG_FILE
    else
	ed - $MRG_FILE << EOF 1> /dev/null
/^dxlogin.LogoY/
a
dxlogin.LogoX:				-1
.
w
q
EOF
	fi

}


# OPTIONAL: for 3-way merge, make MRG_SetCheck calls here.

[ "$CHECK_SYNTAX" ] || MRG_Merge "$@" 

