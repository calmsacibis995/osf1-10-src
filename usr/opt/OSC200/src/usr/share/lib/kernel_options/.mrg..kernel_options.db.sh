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

EditKdebug()
{
	KDEBUG="Kernel Breakpoint Debugger (KDEBUG):O:options		KDEBUG"

	STAT=0

	grep -q 'Kernel Breakpoint Debugger' $_FILE ||
	{
	  if grep -q 'Reserve Kernel Debug Space' $_FILE 
	  then

		if grep -v 'Reserve Kernel Debug Space' $_FILE > $_TMPFILE
		then
			MRG_Echo "removing Reserve Kernel Debug Space option"
			mv $_TMPFILE $_FILE
		else
			MRG_Echo "\tfailed to remove Reserve Kernel Debug Space option"
			STAT=1
		fi
	  fi

	  MRG_Echo "adding Kernel Breakpoint Debugger (KDEBUG) option"
  	  echo "$KDEBUG" >> $_FILE
	  [ $? = 0 ] ||
	  {
		MRG_Echo "\tfailed to add KDEBUG option"
		STAT=1
	  }
	}

	return $STAT
}


AddAdvancedFS()
{
	MSFS="Advanced File System (ADVFS):Z:options		MSFS"

	grep -q "Advanced File System" $_FILE ||
	{
		MRG_Echo "adding Advanced File System (ADVFS) option"
		echo "$MSFS" >> $_FILE
		[ $? = 0 ] ||
		{
			MRG_Echo "\tfailed to add ADVFS option"
			return 1
		}
	}

	return 0
}

EditPacketfilter()
{
	PF="Packetfilter driver (PACKETFILTER):O:options		PACKETFILTER"

	grep -q "Packetfilter driver" $_FILE ||
	{
		MRG_Echo "adding Packetfilter driver (PACKETFILTER) option"
		echo "$PF" >> $_FILE
		[ $? = 0 ] ||
		{
			MRG_Echo "\tfailed to add PACKETFILTER option"
			return 1
		}
	}

	return 0
}

EditDECAUDIT()
{
	grep -q "DEC_AUDIT" $_FILE ||
	{
		MRG_Echo "adding Audit Subsystem option"

		echo "Audit Subsystem:O:options		DEC_AUDIT" >> $_FILE
		[ $? = 0 ] ||
		{
			MRG_Echo "\tfailed to add Audit Subsystem option"
			return 1
		}
	}

	return 0
}


EditFFM()
{
	FFM="File on File File System (FFM):O:options		FFM_FS"

	grep -q "FFM_FS" $_FILE ||
	{
		MRG_Echo "adding File on File File System (FFM) option"
		echo "$FFM" >> $_FILE
		[ $? = 0 ] ||
		{
			MRG_Echo "\tfailed to add FFM option"
			return 1
		}
	}

	return 0
}


EditStreams()
{
	MSG1="Could not remove 'strheap 384' from kernel_options.db."
	MSG2="Please remove manually."

	STAT=0

	#
	# Get rid of strheap
	#
	grep -q 'pseudo-device\(.*\)strheap\(.*\)384' $_FILE &&
	{
		MRG_Echo "removing strheap pseudo-device"

		if grep -v 'pseudo-device\(.*\)strheap\(.*\)384' $_FILE > $_TMPFILE
		then
			mv $_TMPFILE $_FILE
		else
			MRG_Echo "\tfailed to remove strheap pseudo-device"
			STAT=1
		fi
	}

	#
	# Get rid of strthreads
	#
	grep -q 'pseudo-device\(.*\)strthreads' $_FILE &&
	{
		MRG_Echo "removing strthreads pseudo-device"

		if grep -v 'pseudo-device\(.*\)strthreads' $_FILE > $_TMPFILE
		then
			mv $_TMPFILE $_FILE
		else
			MRG_Echo "\tfailed to remove strthreads pseudo-device"
			STAT=1
		fi
	}

	#
	# Add BSD_TTY and LDTTY and PCTK
	#
	grep -q "BSD_TTY" $_FILE ||
	{
		MRG_Echo "adding Streams Protocol options"

		ed - $_FILE << EOF 1> /dev/null
/options\(.*\)STREAMS
a
	options		BSD_TTY:		# STREAMS PTY
	options		LDTTY			# STREAMS TTY
.
w
q
EOF
		[ $? = 0 ] ||
		{
			MRG_Echo "\tfailed to add Streams Protocol options"
			STAT=1
		}
		PCKT="STREAMS pckt module (PCKT):O:options      PCKT"
		MRG_Echo "adding STREAMS pckt module option"
	
		echo "$PCKT" >> $_FILE
		[ $? = 0 ] ||
		{
			MRG_Echo "\tfailed to add PCKT option"
			STAT=1
		}
	}

	#
	# Get rid of old options
	#
	for K in STRSC: STRNULL: STRECHO: STRPASS: STRTMUX: SVTT TIMOD TIRDWR XTISO
	do
		grep -q $K $_FILE &&
		{
			OPT=`echo $K | sed 's/://'`
			MRG_Echo "removing $OPT option"
			if grep -v $K $_FILE > $_TMPFILE
			then
				mv $_TMPFILE $_FILE
			else
				MRG_Echo "\tfailed to remove $OPT option"
				STAT=1
			fi
		}
	done

	#
	# Add TIMOD and TIRDWR to xti optional question
	#
	XTI1="X/Open Transport Interface (XTISO, TIMOD, TIRDWR):O:"
	XTI2="      options      XTISO:"
	XTI3="      options      TIRDWR:"
	XTI4="      options      TIMOD"
	MRG_Echo "modifying STREAMS XTI module option"

	echo "$XTI1" >> $_FILE
	echo "$XTI2" >> $_FILE
	echo "$XTI3" >> $_FILE
	echo "$XTI4" >> $_FILE
	[ $? = 0 ] ||
	{
		MRG_Echo "\tfailed to modify XTI option"
		STAT=1
        }
	

	return $STAT
}


EditSys5()
{
	grep -q "SYSV_FS" $_FILE &&
	{
		MRG_Echo "removing SYSV_FS option"
		if grep -v "SYSV_FS" $_FILE > $_TMPFILE
		then
			mv $_TMPFILE $_FILE
		else
			MRG_Echo "\tfailed to remove SYSV_FS option"
			return 1
		fi
	}

	return 0
}


EditTerminalService()
{
	grep -q 'pseudo-device\(.*\)rpty\(.*\)255' $_FILE ||
	{
		MRG_Echo "modifying Terminal Service option"
		ed - $_FILE << EOF 1>/dev/null
		/^Terminal\(.*\)Service/s/pty/rpty/
		w
		q
EOF
		[ $? = 0 ] ||
		{
                	MRG_Echo "\tfailed to modify Terminal Service option"
			return 1
		}
	}

	return 0
}

EditHab()
{
	grep -q 'sysvid' $_FILE &&					#is it here?
	{											#yes
		MRG_Echo "modifying sysvid_three_hab declaration"
		ed - $_FILE << EOF 1>/dev/null
		/sysvid_three_hab
		s/sysvid/svid/
		w
		q
EOF
		[ $? = 0 ] ||							#edit ok?
		{										#no
			MRG_Echo "\tfailed to correct sysvid_three_hab declaration"
			return 1
		}
	}
	return 0
}

DeleteMBClusters()
{
	# Search the pseudo-device mbclusters entry and zap it
	fgrep -q mbclusters $_FILE &&
	{
		MRG_Echo "Deleting the unused \"mbclusters\" entry"
		ed - $_FILE << EOF 1>/dev/null
/pseudo-device[ 	]*mbclusters/d
w
q
EOF
		# Error message not required (will confuse otherwise).
	}
}

MERGE_ROUTINE=DRI_Merge


#	-DRI_Merge
#		merge routine provided by the DRIs.
#
#	given: 	global variable $_FILE $_TMPFILE
#	return: 0 if success
#		non-zero if failure
#
#	Note:	1) use MRG_Echo() to output additional messages.
#		2) see also /usr/share/lib/shell/libmrg for other available 
#		   global variables.

DRI_Merge()
{
	RET=0

	EditDECAUDIT || RET=1
	EditFFM || RET=1
	EditStreams || RET=1
	EditSys5 || RET=1
	EditTerminalService || RET=1
	AddAdvancedFS || RET=1
	EditPacketfilter || RET=1
	EditKdebug || RET=1
	EditHab || RET=1
	DeleteMBClusters	# If it didn't exist, why bother?

	return $RET
}


SHELL_LIB=${SHELL_LIB:-/usr/share/lib/shell}
. $SHELL_LIB/libmrg


[ "$CHECK_SYNTAX" ] || MRG_Merge "$@" 

