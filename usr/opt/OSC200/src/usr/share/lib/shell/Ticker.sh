#
#	Ticker.sh
#		working messages for command procedures
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
# @(#)$RCSfile: Ticker.sh,v $ $Revision: 4.1.5.2 $ (DEC) $Date: 1993/06/11 15:33:55 $ 
# 
#	000	20-apr-1991	ccb
#		culled from setld(8)
#
#	001	30-may-1991	ccb
#		moved the named pipe to /var/tmp
#		remove pipe when finished with it

_Ticker_PIPE=/var/tmp/pipe$$
_Ticker_PID=

:	-TickWhile
#		Provide ticking while a named process is running
#
#	given:	$* a command line to run
#	does:	establish a Ticker(), run the named process, turn the
#		Ticker() off.
#	return:	the exit status of the named process
#
#


TickWhile()
{	_Ticker_PROC="$@"

	Ticker on
	eval $_Ticker_PROC
	_Ticker_PROCRET=$?
	Ticker off
	return $_Ticker_PROCRET
}


:	-Ticker
#		present time stamps on stdout
#
#	given:	$1 - "on" or "off"
#	does:	turn output time stamping on or off
#	return:	nothing
#	side effect:	$_Ticker_PID updated

Ticker()
{
	case "$1" in
	on)	# make sure there isn't one already
		[ -p $_Ticker_PIPE ] ||
			mknod $_Ticker_PIPE p

		[ "$_Ticker_PID" ] && return
		(	
			# ticking is a background subshell that
			#  traps on sighup
			trap 'exit 0' 1
			echo > $_Ticker_PIPE
			sleep 15	# wait a bit before starting
			while :
			do
				echo "	Working....`date`"
				# ticker wakes up faster taking
				#  short naps...
				for X in 0 1 2 3 4 5 6 7 8 9
				{
					sleep 6
					sleep 6
				}
			done
		) &
		_Ticker_PID=$!
		(read X) < $_Ticker_PIPE
		rm -f $_Ticker_PIPE
		;;

	off)	# make sure there's one running
		[ "$_Ticker_PID" ] &&
		{
			# kill it
			kill -1 $_Ticker_PID
			Wait _Ticker_PID
		}
	esac
}
