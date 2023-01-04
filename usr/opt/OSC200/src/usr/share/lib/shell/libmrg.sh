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
# @(#)$RCSfile: libmrg.sh,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/12 21:23:08 $
# 
# 	libmrg.sh - shell library routines for update installation merges
#

:	-MRG_3WayMerge
#		calls 3-way 'merge' to accomplish the merge
#
#	given:  nothing. Uses just the global variables.
#	does:	check if all needed files are present. If yes, calls merge
#	return:	0 - success
#		other - failure

MRG_3WayMerge()
{
	#check for the existence of the three components required by 
	#merge which uses diff3.
	[ -f "$_FILE" ] ||
	{
		MRG_Echo "$_PWD/$_FILE does not exist." 
		MRG_Echo "Can not perform 3-way merge."	
		return 1
	}

	[ -f "$_NEWFILE" ] ||
	{
		MRG_Echo "$_PWD/$_NEWFILE does not exist." 
		MRG_Echo "Can not perform 3-way merge."	
		return 1
	}

	[ -f "$_OLDNEWFILE" ] ||
	{
		MRG_Echo "$_PWD/$_OLDNEWFILE does not exist." 
		MRG_Echo "Can not perform 3-way merge."	
		return 1
	}

	# check if /usr/lbin is in PATH, which is where we put upd_merge.
	echo $PATH | 
	egrep '^\/usr\/lbin:|:\/usr\/lbin:|:\/usr\/lbin$' >/dev/null ||
		PATH="$PATH:/usr/lbin"

	upd_merge -q $_FILE $_OLDNEWFILE $_NEWFILE
	RET=$?

	[ "$RET" = 0 ] &&
	{ 
		# merge was successful, check for duplicates.
		[ -s /tmp/merge_checks ] &&
		{
			# go through the list, check each string
			cat /tmp/merge_checks | while read CHECK
			do
				MRG_DupCheck "$CHECK" 
				[ -f $_FILE.DupMRG ] &&
				{
					# prepare for next check
					rm $_FILE.DupMRG

					# the overall mark for failure
					> $_FILE.MRG
				}
			done

			# remove the data file
			rm -f /tmp/merge_checks

			[ -f $_FILE.MRG ] && RET=1
		}
	}

	[ "$RET" = 1 ] &&
	{
		# there was merge conflicts or dupliate entries
		if [ -f $_FILE.MRG ] 
		then
			# remove the two marks
			rm -f $_FILE.DupMRG $_FILE.MRG
		else
			# this is plain merge failure
			MRG_Echo "search for <<<<<<< in $_FILE.FailMRG for merge conflicts."
		fi
	}

	return $RET
} 


:	-MRG_SetCheck
#		specify a check for duplicates after a successful 3-way merge.
#
#	given:	a regular expression in single quotes (' ') or
#		a plain text string in double quotes  (" ").
#	does: 	feed the check to egrep later on in MRG_DupCheck().	
#	return: nothing	

MRG_SetCheck()
{
	echo "$1" >> /tmp/merge_checks
}


:	-MRG_DupCheck
#		check for duplicates after a successful 3-way merge.
#
#	given: 	a regular expression or a plain text string.  
#	does:	Depending upon the location of the customization, merge 
#		that add entries may result in doubly defined entries. 
#		This routine helps detect this situation by using egrep.
#	return: 0 - no duplicates
#		1 - duplicates found

MRG_DupCheck()
{
	EXP=$1

	COUNT=0
	egrep "$EXP" $_FILE | 
		while read ENTRY
		do
			COUNT=`expr $COUNT + 1`
			[ "$COUNT" -gt 1 ] &&
			{
				# drop a mark
				> $_FILE.DupMRG
				break	
			} 
		done

	[ -f $_FILE.DupMRG ] &&
	{
		MRG_Echo "'$EXP' is defined more than once after the merge." 
		return 1
	}

	return 0
}


:	-MRG_Echo
#		output a message
#
#	given:	a message
#	does:	precede the message with tabs and output it on the screen.
#	return: 0

MRG_Echo()
{
	echo "\t\t$*"
} 


:	-MRG_Merge
#		If there is a DRI-defined merge routine, call it. Otherwise, 
#		call 3-way merge to accomplish the merge task.
#	given:  $1 - merge option (ToProto/ToConfig)
#		$2 - configured filename (no path, for example: fstab)	
#	does:	sets global variables and carries out merges. 
#	return:	0 - success
#		other - failure 

MRG_Merge()
{
	OPTION=$1
	FILE=$2
	MRG_SetGlobals $OPTION $FILE

	[ "$MERGE_ROUTINE" ] &&
	{
		$MERGE_ROUTINE 
		RET=$?

		rm -f $_TMPFILE
		return $RET
	}

	MRG_3WayMerge
	return $?
}


:	-MRG_SetGlobals
#		sets global variables:
#			_PWD: 		present working directory
#			_FILE:  	file to be merged
#			_NEWFILE:	.new.. file
#			_PROTOFILE:	.proto.. file
#			_OLDNEWFILE:	.new..<file>.PreMRG
#			_TMPFILE:	.tmp.. file
#
#	given:	$1 - merge option (ToProto or ToConfig)
#		$2 - name of the configured file
#	does:	sets global variables available in the merge script.
#	return:	0

MRG_SetGlobals()
{
	OPTION=$1
	_FILE=$2		

	_PWD=`pwd`
	_NEWFILE=.new..$_FILE
	_PROTOFILE=.proto..$_FILE
	_OLDNEWFILE=$_NEWFILE.PreMRG
	_TMPFILE=.tmp..$_FILE
	
	[ "$OPTION" = "ToProto" ] &&
		_FILE=$_PROTOFILE
}


