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
# @(#)$RCSfile: secsetup.sh,v $ $Revision: 1.1.8.14 $ (DEC) $Date: 1993/11/11 20:43:18 $
#
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

PATH="/usr/bin:/usr/sbin"
typeset -u _HOSTNAME
_HOSTNAME=$(hostname | sed 's/\..*//')
CONFIG="/sys/conf/${_HOSTNAME}"
ROOTDIR=""
RCMGR=/usr/sbin/rcmgr
RC_CONFIG=${ROOTDIR}/etc/rc.config
ENHANCED_ALTERNATE=BASE
BASE_ALTERNATE=ENHANCED

#-------------------------------------------------------------------------------#
#
# Prompt(prompt, item, options, default, helpmsg)
#    Prints $PROMPT followed by a colon standard error and returns
#    an assignment of that input to $ITEM on standard output.  If
#    the input is a single question mark (?), then $HELPMSG is
#    printed and the $PROMPT is again printed.  If $OPTIONS is
#    not null, then it is printed in parentheses after the prompt.
#    If $DEFAULT is non-null, then it is displayed in square brackets
#    as a suffix to the prompt and is returned as the value of ITEM
#    if no text is typed at the prompt.
#
#    The response must be one of OPTIONS, if it is non-null, or an
#    error is printed prompt is made again.
#
#    prompt	the text of the prompt to print on stdout
#    item	the symbol to contain the user's input
#    options	the list of valid options.
#    default	the default, (or "") to be used if CR is typed
#    helpmsg	the text to be printed if the user types a single ?
#
function Prompt
{
	FUNC=Prompt
	NUMARGS=5
	typeset ${PROMPT_INPUT_TYPE} INPUT=""
	RESULTS=""

	case "$#" in
	${NUMARGS})	# right number of args, perform search
		PROMPT="$1"
		ITEM="$2"
		OPTIONS="$3"
		DEFAULT="$4"
		HELPMSG="$5"
		;;
	*)		# wrong number of args, error
		print -u2 -r - "$FUNC($# args): Needs ${NUMARGS} arguments"
		exit 1
		;;
	esac

	DEFAULT_TEXT=${DEFAULT+"[${DEFAULT}]"}
	OPTION_TEXT=${OPTIONS+"(${OPTIONS} ?)"}

	while : true
	do
		print -n "${PROMPT}${OPTION_TEXT}${DEFAULT_TEXT}: " 1>&2
		read INPUT
	
		case "${INPUT}" in
		\?)
			print "\n${HELPMSG}\n" | ${PAGER:-more} 1>&2 ;;
		"")
			if [ "$DEFAULT" ]
			then
				eval $ITEM'="'$DEFAULT'"'
				break
			else
				print -u2 "Type '?' for HELP"
			fi ;;
		*)
			PATTERN="$(print $(Loop ${#INPUT} "\077"))"
			for OPTION in ${OPTIONS}
			do	
	    		    TSTSTR=${OPTION%${OPTION#${PATTERN}}}
	    		    if [ "${TSTSTR}" = "${INPUT}" ]
			    then
				RESULTS="${OPTION}"
			    fi
			done
			if [ "${RESULTS}" ]
			then
				eval $ITEM'="'$RESULTS'"'
				break
			else
				print -u2 "\n\007Invalid option."
				print -u2 "Must be one of: $OPTIONS or ? for help\n"
			fi ;;
		esac
	done
}
#-------------------------------------------------------------------------------#
# This function will disable all segmentation and avoid any
# unauthorized segmant sharing, the following will be entered into
# the configuration file:
#	segmentation 0
function DisableSegmentation
{
	print "Updating configuration file to prevent segmentation..."
	ed - ${CONFIG} <<- END > /dev/null
	?^machine
	a
	segmentation	0
	.
	w
	q
END
	print "Configuration file '$CONFIG' updated."
	while true
	do
	    read ANS?"Would you like to have a kernel built now (y/[n])  "
	    case $ANS in
	    [yY]*)  print -n "Running 'doconfig -c ${_HOSTNAME}'...."
		doconfig -c ${_HOSTNAME}
		print "Configuration complete.  You may move /sys/${_HOSTNAME}/vmunix into place and reboot."
		break   ;;
	    [nN]*|"")
		print "Your config file has been updated.  You may 'doconfig' when ready."
		break   ;;
	    esac
	done
}

function Loop
{
	typeset -i10 _COUNT_=$1
	_VALUE_=$2
	_RETURN_=
	while [ ${_COUNT_} -gt 0 ]
	do
		_RETURN_="${_RETURN_}${_VALUE_}"
		_COUNT_=_COUNT_-1
	done
	print -r - "${_RETURN_}"
}
		
#
# letters(count, word)
#
#	count		- numbers of letters to return.
#	word		- word which contains letters.
#
function letters
{
	COUNT=$1
	WORD=$2
	if [ ${#WORD} -gt ${COUNT} ]
	then
		print -r - "$(expr ${WORD} : '\('$(Loop ${COUNT} .)'\)')"
	else
		print -r - "$WORD"
	fi
}

function EvalPWrec
{
	[[ "${PWREC}" = *:*:*:*:*:* ]] || return 1	# check format
	USR="${PWREC%%:*}"
	LETTER="${USR%${USR#?}}"
	AUTHDIR="${ROOTDIR}/tcb/files/auth/${LETTER}"
	Trec="${PWREC#*:}"
	BPSW="${Trec%%:*}"
	Trec="${Trec#*:}"
	UID="${Trec%%:*}"
	[[ "${UID}" = +([0-9]) ]] || return 1	# must be numeric
	return 0
}

function Migrate_to_new_security_level 
{
	FUNC=Migrate_to_new_security_level
	NUMARGS=1

	case "$#" in
		${NUMARGS})	# right number of args, perform search
			STATUS="$1"
		;;

		*)		# wrong number of args, error
		print -u2 -r - "$FUNC($# args): Needs ${NUMARGS} arguments" 
		exit 1
		;;
	esac

	trap : CHLD
	egrep '^[^@#:+--]' ${ROOTDIR}/etc/passwd |&
	while read -p PWREC
	do
		EvalPWrec || continue	# check format, break out values
		${STATUS}_migration
	done
}

function ENHANCED_migration
{
	[ -d "${AUTHDIR}" ] || mkdir "${AUTHDIR}" || return 1
	if [[ "${BPSW}" != ????????????? ]] && [[ -n "${BPSW}" ]]
	then
		PSW=Nologin
	else
		PSW="${BPSW}:u_succhg#0"
	fi
	if [ ! -f "${AUTHDIR}/${USR}" ]
	then
		print "created auth database for ${USR}"
		sed -e 's+^X++' > "${AUTHDIR}/${USR}" <<- EOF
			X${USR}:u_name=${USR}:u_id#${UID}:\\
			X	:u_pwd=${PSW}:\\
			X	:u_lock@:chkent:
		EOF
	elif grep -q ':u_pwd=:' "${AUTHDIR}/${USR}"
	then
		ed - "${AUTHDIR}/${USR}" <<- EOF
		/:u_pwd=:/s,:u_pwd=:,:u_pwd=${PSW}:,
		w
		q
		EOF
	elif grep -q ':u_pwd=' "${AUTHDIR}/${USR}"
	then :
	else
		print "\t:u_pwd=${PSW}:\\" >> "${AUTHDIR}/${USR}"
		ed - "${AUTHDIR}/${USR}" <<- \EOF
		$,$m1
		w
		q
		EOF
	fi
	chown auth:auth "${AUTHDIR}/${USR}"
	chmod 660 "${AUTHDIR}/${USR}"
}

function BASE_migration
{
	TPW=$(grep -s ':u_pwd=' "${AUTHDIR}/${USR}")
	TPW="${TPW#*:u_pwd=}"
	TPW="${TPW%%:*}"
	if [ "${#TPW}" -gt 12 ] && [[ "${TPW}" = +([./0-9A-Za-z]) ]]
	then
		print "Updating password field in /etc/passwd for ${USR}"
		TPW=$(letters 13 "${TPW}")
		ed - ${ROOTDIR}/etc/passwd > /dev/null 2>&1 <<- _END_OF_EDIT_
			/^${USR}:
			d
			i
			${USR}:${TPW}:${Trec}
			.
			w
			q
		_END_OF_EDIT_
	fi
}

case "$(whoami)" in
	"root")	: ;;
	*)	print -u2 "$0:  you must be root to run secsetup!"
		exit 1
esac 

# sanity check for existence of /etc/rc.config

if [ -f ${RC_CONFIG} ]
then
	. ${RC_CONFIG}
	CURRENT_STATUS=${SECURITY:=BASE}
else
	print -u2 "error: ${RC_CONFIG} file is missing"
	exit 1
fi

if [ ! -d ${ROOTDIR}/tcb ]
then 
	exit 0
fi

umask 007	# must allow group access to the directories & files.
eval DEFAULT_STATUS=\$\{${SECURITY}_ALTERNATE\}
typeset -x PROMPT_INPUT_TYPE="-u"		# upper case.
Prompt "Enter system security level" STATUS "BASE ENHANCED" "${DEFAULT_STATUS}" "
	The default option is to switch the current security level.

	BASE - Discretionary Security Protection:

	    Default OSF/1 style of security features.

	ENHANCED - Controlled Access Protection:

	    A system in this class enforces a more finely grained
	    discretionary access control than (BASE) systems.
    	    Users are individually accountable for their actions
	    through login procedures, auditing of security-relevant
	    events and resource isolation.

	The audit subsystem can be configured for either of the system 
	security levels.
 "

if [ "${CURRENT_STATUS}" != "${STATUS}" ]
then
	print "${STATUS} security level will take effect on the next system reboot."

	${RCMGR} set SECURITY ${STATUS}

	Migrate_to_new_security_level ${STATUS}

	[ "${STATUS}" = "BASE" -a -f ${ROOTDIR}/etc/passwd.pag ] && \
		mkpasswd ${ROOTDIR}/etc/passwd
else
	print "Security level already at ${STATUS} level."
fi

typeset -x PROMPT_INPUT_TYPE="-l"		# lower case.

print ""
grep -q "segmentation" ${CONFIG} && {
	print "Segment sharing has been disabled."
} || {
	Prompt "Do you wish to disable segment sharing" SEG_REPLY "yes no" "no" "
	
	Because of page table the sharing mechanism used for shared
	libraries, the normal file system permissions are not
	adequate to protect against unauthorized reading.
	For example, suppose user joe has the following
	shared library:

	-rw------- 2 joe  staff  100000 Sep 18 1992  /usr/shlib/foo.so

	When the shared library is used in a program, the
	text part of foo.so may in fact be visible to other
	running processes even though they are not running as
	user joe.  Note that only the text part of the
	library, not the data segment, is shared in this way.
	To prevent this unwanted sharing, any libraries that
	need to be protected can be linked with the -T and -D
	options to put the data section in the same 8-
	megabyte segment as the text section.  The following
	link options should work for any such library:

	% ld -shared -o libfoo.so -T 30000000000 -D 30000400000 object_files...

	In fact, this segment sharing can occur with any file
	that is mmapp'ed without the PROT_WRITE option as long
	as the mapped address falls in the same memory
	segment as other mmapp'ed files.  Any program that
	uses mmap() to examine files that may be highly
	protected can ensure that no segment sharing takes
	place by introducing a writable page into the
	segment before or during the mmap().  The easiest way
	to accomplish this is to mmap() the file with
	PROT_WRITE enabled in the protection, and then use
	mprotect() to make the mapped memory read only.

	Alternatively, to disable all segmentation and avoid
	any unauthorized sharing, answer 'yes' to the question."

	if [ ${SEG_REPLY} != "no" ]
	then
		DisableSegmentation
	fi
}

print ""
Prompt "Do you wish to run the audit setup utility at this time" REPLY "yes no" "no" "
	The audit subsystem works with either BASE or ENHANCED security, 
	recording whatever information is available.  There is no audit
	re-configuration between security levels. 
"
if [ ${REPLY} != "no" ]
then
	/usr/sbin/audit_setup
fi

print ""
read ACK?"Press return to continue:  "
