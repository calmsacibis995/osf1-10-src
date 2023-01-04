#!/usr/bin/ksh -p
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
# @(#)$RCSfile: secauthmigrate.ksh,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 20:17:48 $
# 

# Script to convert ULTRIX-format auth data as per auth(5) to the "tcb"
# format required on our OSF-based product.

# Environment:
# If ROOTDIR is set, the files will be created in ${ROOTDIR}/tcb/auth/files.
# If SHELL_DEBUG is set, the functions will be defined, but PATH will not
# be altered, Init() will not be called to create the arrays, and Main() will
# not be called to do the work.


# Some integer global variables used in parsing the ULTRIX data and in
# generating the TCB data.

typeset -i10 UID LCHG MINCHG MAXCHG FCNT NUMAUDS ACNTL AFLAGS

:	-Aparse
# Given:	an ULTRIX auth(5) format record
# Does:		stores the relevant values in global variables for use
#		by later phases

function Aparse
{
	typeset inrec="$1"	# remember input value
	set -- $(Split : "$1")
	case "$#" in
	 (10|11)	: true
			;;
	 (*)		print -u2 "${PROG}: invalid input record '${inrec}'"
			return 1
			;;
	esac
	UID="$1"		# user's UID
	PSW="$2"		# crypt16(3) password
	LCHG="$3"		# time_t of last successful psw change
	MINCHG="$4"		# time_t of min. psw change interval
	MAXCHG="$5"		# time_t of (soft) psw change lifetime
	AFLAGS="8#$6"		# auth flags for psw changes and login access
	FCNT="$7"		# failure count (since last good validation)
	# AUID="$8"		# ULTRIX audit-ID (not used here)
	ACNTL="8#$9"		# audit control type (and,or,usr,none)
	shift 9			# (keep positional parameters sane)
	AMASK="$1"		# get the proc_amask hexadecimal bignum
	# TMASK="$2"	# due to confusion in ULTRIX, it's all really in AMASK
}

:	-Audfix
# Given:	global variables from Aparse()
# Does:		sets up values in AUDMASK array for eventual output

function AudFix
{
# local variables
	typeset -i10 i j c m x
	typeset f s n

# Start off clean.
	unset AUDMASK

	f="${AMASK}"
	i=0
	NUMAUDS=0
# Loop over ULTRIX data one nybble at a time, just in case it was not properly
# formatted to a byte boundary.
	while [ -n "$f" ] && [ $i -lt 576 ]
	do
# Extract next nybble.
		n="${f%?}"
		s="${f#$n}"
		f="$n"
		eval m="16#$s"
# Two events per nybble.
		c=0
		while [ $c -lt 2 ]
		do
			x=i+c
# If a remapped entry, get the alternate value.
			[ -n "${AUDMAP[x]}" ] && x=${AUDMAP[x]}
			j='m&3'
			if [ -n "${AUDNAMES[x]}" ] && [ $j -ne 0 ]
			then
# Audited by ULTRIX and we care--remember it.
				AUDMASK[x]=$j
				NUMAUDS=NUMAUDS+1
			fi
			c=c+1
			m='m>>2'
		done
		i=i+2
	done
}

:	-Init
# Given:	nothing
# Does:		sets up the AUDNAMES, AUDMAP, and AUDBITS arrays.

function Init
{
# Note that some of the ULTRIX values conflict between VAX & MIPS syscall
# numbers.  The best match for security-relevant events were taken.  All
# others are ignored.

	set -A AUDNAMES '' exit fork read write open close old\\040creat link \
		unlink exec chdir mknod chmod chown obreak '' lseek getpid \
		mount unmount '' '' '' ptrace '' '' '' '' '' '' '' '' '' '' \
		kill '' '' '' dup pipe '' '' '' '' '' '' '' '' acct '' '' \
		ioctl reboot '' symlink '' exec umask chroot '' '' '' \
		mremap vfork '' '' '' sbrk sstk mmap vadvise munmap mprotect \
		madvise revoke '' mincore '' setgroups '' setpgrp '' '' \
		swapon '' '' sethostname '' dup2 '' fcntl '' setdopt '' \
		setpriority socket connect accept '' send recv '' bind \
		setsockopt listen '' '' '' '' '' '' recvmsg sendmsg '' \
		'' '' '' '' readv writev settimeofday fchown fchmod recvfrom \
		setreuid setregid rename truncate ftruncate flock '' sendto \
		shutdown socketpair mkdir rmdir utimes '' adjtime '' '' \
		sethostid '' setrlimit killpg setquota
	AUDNAMES[158]=nfs_svc
	AUDMAP[167]=158
	AUDNAMES[163]=nfs_biod
	AUDMAP[165]=163
#	AUDNAMES[164]=nfs_getfh
#	AUDMAP[166]=164
	AUDNAMES[166]=setdomainname
	AUDMAP[170]=166
	AUDMAP[169]=168
	AUDNAMES[168]=exportfs
	AUDNAMES[151]=msgctl
	AUDMAP[172]=151
	AUDNAMES[152]=msgget
	AUDMAP[173]=152
	AUDNAMES[153]=msgrcv
	AUDMAP[174]=153
	AUDNAMES[154]=msgsnd
	AUDMAP[175]=154
	AUDNAMES[155]=semctl
	AUDMAP[176]=155
	AUDNAMES[156]=semget
	AUDMAP[177]=156
	AUDNAMES[157]=semop
	AUDMAP[178]=157
	AUDNAMES[159]=shmsys
	AUDMAP[180]=159
	AUDNAMES[160]=plock
	AUDMAP[181]=160
	AUDNAMES[161]=lockf
	AUDMAP[182]=161
	AUDNAMES[188]=setsid
	AUDNAMES[233]=utc_gettime
	AUDNAMES[234]=utc_adjtime
	AUDNAMES[252]=audcntl
	AUDNAMES[253]=audgen
	AUDNAMES[254]=startcpu
	AUDNAMES[255]=stopcpu
	AUDNAMES[257]=setsysinfo
	AUDNAMES[512]=audit_suspend
	AUDNAMES[513]=audit_log_change
	AUDNAMES[514]=audit_shutdown
	AUDNAMES[515]=audit_log_creat
	AUDNAMES[516]=audit_xmit_fail
	AUDNAMES[517]=audit_reboot
	AUDNAMES[518]=audit_log_overwrite
	AUDNAMES[519]=audit_daemon_exit
	AUDNAMES[520]=audgen8
	AUDNAMES[521]=audit_setup
	AUDNAMES[522]=login
	AUDNAMES[535]=auth_event
	AUDNAMES[536]=audit_start
#	AUDNAMES[523]=XServerStartup
#	AUDNAMES[524]=XServerShutdown
#	AUDNAMES[525]=XServerDac
#	AUDNAMES[526]=XClientStartup
#	AUDNAMES[527]=XClientShutdown
#	AUDNAMES[528]=XClientIPC
#	AUDNAMES[529]=XObjectCreate
#	AUDNAMES[530]=XObjectRename
#	AUDNAMES[531]=XObjectDestroy
#	AUDNAMES[532]=XObjectDac
#	AUDNAMES[533]=XObjectRead
#	AUDNAMES[534]=XObjectWrite

	set -A AUDBITS '' ':0:1' ':1:0' ''
}

:	-Options
# Given:	Initial ARGV
# Does:		Parses options.  Gives usage message if appropriate.

function Options
{
# local variables
	typeset opt status=0

	while getopts ":vU" opt
	do
		case "${opt}" in
		 (+*)	print -u2 "${PROG} -- unrecognized option '${opt}'"
			Usage
			status=1
			;;
		 (v)	Verbose=1
			;;
		 (U)	UsageL
			trap 'return 0' 0	# exit script after we return
			return 0		# bug out now
			;;
		 ("?")	print -u2 "${PROG} -- unrecognized option '${OPTARG}'"
			Usage
			status=1
			;;
		 (*)	print -u2 "${PROG} -- unrecognized option '${opt}'"
			Usage
			status=1
			;;
		esac
	done

# get rid of options when done
	trap '[ $# -gt 0 ] && shift '"$((OPTIND-1))" 0

	return $status
}

:	-OutOne
# Given:	the variables set up by Aparse() and Audfix()
# Does:		creates a new TCB auth entry, or leaves what it would
#		have created lying about (it refuses to overwrite an
#		existing TCB entry.)

function OutOne
{
# Local variables.
	typeset first= audname audbit
	typeset -i10 i m

# Find username, compute file location, and start writing an authcap record.
	UNAME=$(UidName $UID)
	if [[ "$UNAME" = +([0-9]) ]]
	then
		print -u2 "\aFailed to find name for UID # $UID -- skipped"
		return 1
	fi
	LETTER="${UNAME%${UNAME#?}}"
	ADIR="${ROOTDIR}/tcb/files/auth/${LETTER}"
	AFILE="${ADIR}/${UNAME}"
	TFILE="${AFILE}:t"
	UFILE="${AFILE}:ULT"

	if [ -d "${ADIR}" ]
	then :
	elif mkdir "${ADIR}"
	then
		chown auth:auth "${ADIR}"
		chmod 770 "${ADIR}"
	else
		return 1
	fi

	exec 3>"${UFILE}"
	print -u3 -r - "${UNAME}:u_name=${UNAME}:u_id#${UID}:\\"
	print -u3 "\t:u_pwd=${PSW}:u_oldcrypt#1:u_auditcntl#${ACNTL}:\\"
	print -u3 -n "\t:u_auditmask="

# Loop over the audit info writing that to the authcap file.
	i=0
	while [ ${NUMAUDS} -gt 0 ] && [ $i -lt 576 ]
	do
		m="0${AUDMASK[i]}"
		if [ $m -gt 0 ]
		then
			audname="${AUDNAMES[i]}"
			audbit="${AUDBITS[m]}"
# Handle special mappings from ULTRIX syscalls to OSF syscalls.
			case "${audname}" in
			 (exec)
				print -n -u3 "${first}execv${audbit}"
				print -n -u3 ",execve${audbit}"
				print -n -u3 ",exec_with_loader${audbit}"
				;;
			 (open|stat|lstat|send|recv|fstat)
				print -n -u3 "${first}${audname}${audbit}"
				print -n -u3 ",old\\040${audname}${audbit}"
				;;
			 (accept|recvmsg|sendmsg|recvfrom)
				print -n -u3 "${first}${audname}${audbit}"
				print -n -u3 ",n${audname}${audbit}"
				;;
			 (revoke)
				print -n -u3 "${first}${audname}${audbit}"
				print -n -u3 ",old\\040vhangup${audbit}"
				;;
			 (killpg)
				print -n -u3 "${first}sigsendset${audbit}"
				print -n -u3 ",old\\040${audname}${audbit}"
				;;
			 (setpgrp)
				print -n -u3 "${first}${audname}${audbit}"
				print -n -u3 ",setpgid${audbit}"
				;;
			 (chdir)
				print -n -u3 "${first}${audname}${audbit}"
				print -n -u3 ",fchdir${audbit}"
				;;
			 (chown)
				print -n -u3 "${first}${audname}${audbit}"
				print -n -u3 ",lchown${audbit}"
				;;
			 (audcntl)
				print -n -u3 "${first}${audname}${audbit}"
				print -n -u3 ",security${audbit}"
				;;
			 (msgctl|msgget|msgrcv|msgsnd|semctl|semget|semop|plock|setsid)
				print -n -u3 "${first}${audname}${audbit}"
				print -n -u3 ",alternate\\040${audname}${audbit}"
				;;
			 (*)
				print -n -u3 "${first}${audname}${audbit}"
				;;
			esac
			first=','
			NUMAUDS=NUMAUDS-1
		fi
		i=i+1
	done
	print -u3 ":\\"

# Finish the authcap data.
	print -u3 "\t:u_succhg#${LCHG}:u_minchg#${MINCHG}:u_exp#${MAXCHG}:\\"
	print -u3 -n "\t:u_pickpw"
	if [ $((AFLAGS&4)) -eq 0 ]
	then
		print -u3 -n "@"
	fi
	print -u3 -n ":u_genpwd"
	if [ $((AFLAGS&2)) -eq 0 ]
	then
		print -u3 -n "@"
	fi
	print -u3 -n ":u_lock"
	if [ $((AFLAGS&1)) -ne 0 ]
	then
		print -u3 -n "@"
	fi
	print -u3 ":\\"

	print -u3 "\t:chkent:"
	exec 3>&-
# Fix permissions on the authcap entry.
	chown auth:auth "${UFILE}"
	chmod 660 "${UFILE}"
# Use this file if it's the first time we've needed it.
	if grep -q -s ":chkent:" "${UFILE}"
	then
		if ln "${UFILE}" "${TFILE}" 2>/dev/null && \
			ln "${TFILE}" "${AFILE}" 2>/dev/null
		then
			rm -f "${TFILE}" "${UFILE}"
			[ -n "$Verbose" ] && print -r - " - ${UNAME} (${UID})"
		else
			[ "${TFILE}" -ef "${UFILE}" ] && rm -f "${TFILE}"
			print -u2 "Existing file for $UNAME -- conversion saved in ${UFILE}"
		fi
	else
		print -u2 "Incomplete record for $UNAME -- conversion saved in ${UFILE}"
	fi
}

:	-Split
# Given:	$1 as a separator and $2-$n as fields to be split
# Does:		prints the fields after splitting on new separators.

function Split
{
	typeset IFS="$1" flags=$-
	shift
	set -o noglob
	set -- $*
	[[ "$flags" = *f* ]] || set +o noglob
	print -r - "$@"
}

:	-UidName
# Given:	$1 as a decimal UID to translate
# Does:		prints the (or a) corresponding username if it can find one

function UidName
{
	typeset tname=/tmp/authcvt.uid.$$
	if [ $1 -eq 0 ]
	then
		print 'root'
		return
	fi
	touch ${tname}
	chown "$1" ${tname}
	set -- $(ls -o ${tname})
	rm -f ${tname}
	print -r - "$3"
}

:	-Usage
# Given:	nothing
# Does:		writes a usage message to standard error.

function Usage
{
	print -u2 "Usage: ${PROG} [options] files"
	print -u2 "\t(Try ${PROG} -U for help.)"
}

:	-UsageL
# Given:	nothing
# Does:		writes a long usage message to standard error.

function UsageL
{
	print -u2 ""
	print -u2 "Usage: ${PROG} [options] files"
	print -u2 "\t\t(\"files\" can include \"-\" to imply standard input.)"
	print -u2 " options:"
	print -u2 "\t-v\tDo verbose output (list usernames converted)."
	print -u2 "\t-U\tGet this usage message."
	print -u2 ""
}

:	-Main
# Given:	a list of files or standard input
# Does:		tries to create TCB entries for each ULTRIX auth(5) record 
#		in the input files.

function Main
{
	cat ${1+"$@"} |
	while read aline
	do
		Aparse "$aline" || continue
		AudFix
		OutOne
	done
}

# Allow for debugging.

[ -n "${SHELL_DEBUG}" ] && return 0

# Get a path we trust.

PATH=/usr/sbin:/usr/bin:/usr/ccs/bin:/usr/lbin:/sbin
export PATH

# Keep track of where we find commands.

set -h

PROG="$0"
[ ! -n "${PROG}" ] && PROG=/usr/sbin/secauthmigrate
PNAME="${PROG##*/}"

# Set up the syscall name arrays.

Init

# Check our options

Options ${1+"$@"} || return $?

# Create files with restrictive permissions (we fix them later)
umask 077

# Read the input and make the /tcb/files/auth entries.

if [ $# -eq 0 ]
then
	Usage
	return 1
else
	Main ${1+"$@"}
fi

return $?
