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
# @(#)$RCSfile: it.sh,v $ $Revision: 4.3.9.3 $ (DEC) $Date: 1993/11/13 00:08:45 $
#
#########################################################################

[ -d "/usr/sbin" ] || exit

stty dec
term=pmconsole
PATH=/sbin:/usr/sbin:/usr/bin:/usr/lbin
export PATH
PWD=`pwd`
TMP_PATH="/tmp"
ITD_PATH="/sbin/it.d"
LOG_PATH="/var/adm/smlogs"
BIN_PATH="${ITD_PATH}/bin"
DATA_PATH="${ITD_PATH}/data"
LOG_TOOL="/usr/sbin/log"
MOTD_FILE="/etc/motd"
LOG_FILE="${LOG_PATH}/it.log"
ISL_FILE="${DATA_PATH}/inst.cnfg"
ITEM_FILE="${TMP_PATH}/it.items"
SORT_FILE="${TMP_PATH}/it.sort"
TEMP_FILE="${TMP_PATH}/it.temp"
TODO_FILE="${TMP_PATH}/it.todo"
MASK_FILE="${DATA_PATH}/options.mask"
REBOOT_FILE="${DATA_PATH}/reboot"
IT_FILE_LIST="$ITEM_FILE $SORT_FILE $TEMP_FILE $TODO_FILE"
DMZ_START="
-------------------------------------------------------------------------
 %IT START%	`date`
-------------------------------------------------------------------------
"
DMZ_STOP="
-------------------------------------------------------------------------
 %IT STOP%	`date`
------------------------------------------------------------------------- "


###############
# Subroutines #
###############
set -h

: Cleanup -
#
#	Given:	Nothing
#
#	Does:	Cleans up all temporary files or unneeded log files,
#		created by execution of this script.
#
#	Returns:Nothing
#
Cleanup()
{
	for K in $*
	do
		rm -f $K
	done
}


: Demarcate_Logfile -
#
#	Given:	$1
#
#	Does:	Writes either a start or stop demarcation line to it.log,
#		to indicate where an instance of 'it' started and stopped.
#
#	Returns:Nothing
#
Demarcate_Logfile()
{
	case $1 in
	START )
		echo "$DMZ_START" >> $LOG_FILE
		;;
	STOP )
		echo "$DMZ_STOP" >> $LOG_FILE
		;;
	esac
}


: Execute_Items -
#
#	Given:	Nothing
#
#	Does:	Executes each item in the todo list that was correlated
#		by the Sort_Routine.  One exception is to make certain
#		that the password is not logged.  The items are
#		removed from their respective directories after execution
#		and items are only executed once, even if the same item
#		is found in multiple directories.
#
#	Returns:Nothing
#
Execute_Items()
{
	for I in `cat $TODO_FILE`
	do
		case $I in
		*getpasswd )
			/sbin/sh $I
			;;
		*gitout )
			$LOG_TOOL $LOG_FILE /sbin/sh $I
			[ "$?" = 1 ] &&
			{
				GITOUT=1
				return 1
			}
			;;
		* )
			$LOG_TOOL $LOG_FILE /sbin/sh $I
			;;
		esac
		rm $I
	done
}


: Find_Items -
#
#	Given:	nothing
#
#	Does:	Determines which run state the system is operating under,
#		then collects all files within any directory that contains
#		in whole or part, the run state in it's name.
#
#	Returns:nothing
#
Find_Items()
{
	set -- `who -r`
	STATE=$7

	>$ITEM_FILE

	for F in ${ITD_PATH}/*${STATE}*.d/*
	do
		[ -f $F ] && echo $F >> $ITEM_FILE
	done
}


: Init_Env -
#
#	Given:	Nothing
#
#	Does:	Performs general setup of environment variables, and
#		prepares and checks for required files, in preparation
#		for further 'it' execution.
#
#	Returns:Nothing
#
Init_Env()
{
	[ -f $ISL_FILE ] && 
	{
		. $ISL_FILE
		export ISL_ADVFLAG
		grep -q $LOG_FILE $MOTD_FILE ||
		{
ed $MOTD_FILE <<xxEOFxx 1>/dev/null
/^[	/]
a
	$LOG_FILE		- log for it(8) utility
.
w
q
xxEOFxx
		}
	}

	if [ -s $MASK_FILE ]
	then
		MASK_LIST=`cat ${DATA_PATH}/options.mask`
	else
		echo "it: ERROR - ${DATA_PATH}\c" | tee -a $LOG_FILE
		echo "/options.mask does not exist." | tee -a $LOG_FILE
		exit 1
	fi

	case `/bin/machine` in
	mips ) /sbin/init.d/loader 2>&1 >> $LOG_FILE ;;
	esac
	swapon -a 2>&1 | tee -a $LOG_FILE
}


: SetTrap -
#
#	Given:	Nothing
#
#	Does:	Sets trap for interupts.
#
#	Returns:Nothing
#
SetTrap()
{
	trap '
	while : true
	do
		echo "\nDo you want to quit (y/n) []: \c"
		read ans
		ans=`echo $ans`
		case $ans in
		[yY] )
			trap '' 1 2 3
			Demarcate_Logfile STOP
			Cleanup $IT_FILE_LIST
			exit 1
			;;
		[Nn] )
			break
			;;
		esac
	done
	' 1 2 3
}


: Sort_Items -
#
#	Given:	Nothing
#
#	Does:	Sorts items found by Find_Items and
#		creates a 'todo' list (TODO_FILE) by checking each item
#		against a priority list (MASK_LIST) to ensure items are
#		executed in the correct order.  Any item not defined in
#		the priority list are then added to the bottom of the
#		'todo' list, randomly.
#
#	Returns:Nothing
#
Sort_Items()
{
	>$SORT_FILE
	>$TEMP_FILE
	>$TODO_FILE
	P_FN=XXXXXX
	for F in `sort -t/ +4 $ITEM_FILE`
	do
		FN=`expr $F : '.*/\(.*\)'`
		if [ "$FN" = "$P_FN" ]
		then
			rm $F
		else
			echo $F >> $SORT_FILE
			P_FN=$FN
		fi
	done

	set -- `(IFS=:; echo $MASK_LIST)`
	while [ $# -gt 0 ]
	do
		F=`grep $1 $SORT_FILE` &&
		{
			echo $F >> $TODO_FILE
			grep -v $F $SORT_FILE > $TEMP_FILE
			mv $TEMP_FILE $SORT_FILE
		}
		shift
	done

	[ -s $SORT_FILE ] && cat $SORT_FILE >> $TODO_FILE
}


########################################################################
# Procedure Division #
######################
Find_Items
[ -s $ITEM_FILE  ] ||
{
	Cleanup $IT_FILE_LIST
	exit 0
}

SetTrap
Demarcate_Logfile START
Init_Env
Sort_Items
Execute_Items
Demarcate_Logfile STOP

if [ "$GITOUT" ]
then
	Cleanup $IT_FILE_LIST
	halt
else
	Cleanup $IT_FILE_LIST $ISL_FILE
fi

[ -f $REBOOT_FILE ] &&
{
	rm -f $REBOOT_FILE
	reboot
}

exit 0
