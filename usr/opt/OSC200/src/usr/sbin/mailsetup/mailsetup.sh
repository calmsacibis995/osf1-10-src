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
# @(#)$RCSfile: mailsetup.sh,v $ $Revision: 1.1.10.4 $ (DEC) $Date: 1993/12/16 19:38:35 $ 
#
# system settings
#
umask 002
export PATH=/sbin:/usr/sbin:/usr/bin:/usr/etc:/etc:/bin:/usr/ucb
ECHO=print
NULL="/dev/null"
PROG=mailsetup
USAGE="usage: ${PROG} [-f cf_outfile] [-j journal]"
UNAME=`uname`

#
# Temporary files
#
CFTMP=${TMPDIR:=/tmp}/mlcf$$	# the scratch sendmail.cf file
M4TMP=${TMPDIR:=/tmp}/mlm4t$$	# the scratch parameter file
USERM4=${M4TMP}			# the user's parameter file
SM4TMP=${TMPDIR:=/tmp}/mlsm4t$$ # used to create sendmail.m4
DESTMP=${TMPDIR:=/tmp}/mldesset$$ # DEbug/Setup tmp file; Logs options.

#
# Misc files and directories
#
case "${UNAME}" in
"ULTRIX")
	CFDIR=/etc
	RCFILE=/etc/rc.local
	BINDSTART_KEY="# %BINDSTART% - BIND daemon added by \"bindsetup\""
	MAIL11FILE=/usr/bin/mail11v3
	NMLFILE=/usr/etc/nml
	NCPFILE=/usr/bin/ncp
	;;
*)
	CFDIR=/var/adm/sendmail
	RCMGR=/usr/sbin/rcmgr
	MAIL11FILE=/usr/sbin/mail11v3
	NMLFILE=/usr/sbin/nml
	NCPFILE=/usr/sbin/ncp
	X25FILE=/usr/sbin/x25mail
	;;
esac
UMCFILE=/usr/lib/umc/mr_mailer
RESOLVC=/etc/resolv.conf

CFTEMPLATE=${CFDIR}/sendmail.cf.pd	# Original Master M4 Template file.
SENDMAIL_M4=${CFDIR}/sendmail.m4	# User's (possibly modified) version.
CFFILE=${CFDIR}/sendmail.cf


# Trap/Exit routines
RETURNVALUE=
QUITERR='
	RETURNVALUE=${RETURNVALUE:-$?};
	${ECHO} "mailsetup terminated with no installations made. " 1>&2;
	eval ${QUIT}
	'

QUIT='
	rm -f ${CFTMP} ${M4TMP} ${SM4TMP} ${DESTMP};
	exit ${RETURNVALUE}
	'

trap 'eval "${QUITERR}"' 1 2 3 15
trap 'eval "${QUIT}"' 0




integer tagarrcnt=0

###########################################
#
#  Function section
#
###########################################

###########################################
# Abort the mailsetup installation
###########################################
function abort
{
    RETURNVALUE=$?
    if [[ $# != 0 ]]; then
	${ECHO} $* 1>&2
    fi
    eval ${QUITERR}
}

###########################################
#
#  ask for a yes or no answer
###########################################
function affirmative # question default
{
typeset answer
typeset question=$1
typeset default=$2

	case "${default}" in
	[yY]*) question="${question} ([y]/n)" ;;
	[nN]*) question="${question} (y/[n])" ;;
	*) question="${question} (y/n)"
	esac

	while :
	do

		${ECHO}
		read -r "answer?${question} ? " || return 1
		case ${answer} in
		[yY]*) 
			if [ -n "${JURNFILE}" ]; then
				${ECHO} "y" >> ${JURNFILE}
			fi
			return 0 
			;;
		[nN]*) 
			if [ -n "${JURNFILE}" ]; then
				${ECHO} "n" >> ${JURNFILE}
			fi
			return 1 
			;;
		"") 
			case "${default}" in
        			[yY]*) 
					if [ -n "${JURNFILE}" ]; then
						${ECHO} "y" >> ${JURNFILE}
					fi
					return 0 
					;;
        			[nN]*) 
					if [ -n "${JURNFILE}" ]; then
						${ECHO} "n" >> ${JURNFILE}
					fi
					return 1 
					;;
        		esac
			;;
		esac
		${ECHO} '\a     Enter "y" or "n" : '
	done
}

###########################################
#
#  Do you want to go on?
###########################################
function go_on
{
typeset answer
	while :
	do
		${ECHO} -n 'Enter "e" to EXIT or "c" to CONTINUE [no default]: '
		read -r answer
		case ${answer} in
		[cC]*) return 0 ;;
		[eE]*) return 1 ;;
		esac
	done
}

###########################################
#
#  get an answer from user 
#      pass it using ${PassedVar}
###########################################
function get_answer # description
{
    typeset description="$1"
    typeset answer

    while :
    do
	${ECHO} -n "\nEnter the ${description} [ ${PassedVar} ]: "
	read answer
	if [ -n "${JURNFILE}" ]; then
		${ECHO} ${answer} >> ${JURNFILE}
	fi
	case ${answer} in
	"")
		if [ -n "${PassedVar}" ]; then
			if ( affirmative "[ ${PassedVar} ] is this correct " "y" )
			then
				break
			else
				${ECHO} "Do you wish to leave this entry blank?"
				if ( affirmative "   Blank entry" "n" )
				then
					PassedVar=""
					break
				else
					continue
				fi
			fi
		fi
		;;
	*)
		PassedVar=${answer}
		;;
	esac
	if [ -n "${PassedVar}" ]; then
		if ( affirmative "[ ${PassedVar} ] is this correct " "y" ); then
			break
		fi
	else
		${ECHO} "No ${description} entered"
		if ( affirmative "   Leave entry blank" "n" )
		then
			break
		else
			continue
		fi
	fi
    done
}


###########################################
#  Get the Class array from the scratch m4 file.
#  Return it in Passed Array
#  Aborts if an error occurs
###########################################
function get_class_value	# Class_name, Optional_file
{
    typeset Class=$1
    typeset Target
    typeset File

    File=${2:-${M4TMP}}
	
    Target="^define(${Class},"
    if grep "${Target}" ${File} >> ${NULL} 2>&1
    then
	set -s -A PassedArray `sed -n "/${Target}/{
		s/.*{//
		s/}.*//p
		}" ${File}`
    else
	abort "Class ${Class} not found in m4 file ${File}"
    fi
}


###########################################
#  Get the value for Macro from the scratch m4 file.
#  Return it in REPLY
#  Aborts if an error occurs
#  Sendmail Macros ($x) are considered to be NULL
###########################################
function get_macro_value	# macro, Optional_file
{
    typeset Macro=$1
    typeset Target
    typeset File

    File=${2:-${M4TMP}}

    Target="^define(${Macro},"

    if grep "${Target}" ${File} >> ${NULL} 2>&1
    then
	REPLY=`sed -n "/${Target}/{
		s/.*{//
		s/^\$[^}]*//
		s/}.*//p
		}" ${File}`
    else
	abort "Macro ${Macro} not found in m4 file ${File}"
    fi
}

###########################################
#
#  Add a macro to the the temp cf file
###########################################
function add_macro  # Macro Value
{
    typeset Macro=$1
    typeset Value=$2
    typeset macroline Target

    #
    # Make sure the file has the initial value
    #
    get_macro_value ${Macro}

    #
    # find search string for macro
    #
    Target='^define('${Macro}','
    macroline='define('${Macro}',	{'${Value}'})dnl'

    #
    # save macro
    #
    ed - ${M4TMP} <<-END >> ${NULL} 2>&1
	/${Target}
	d
	-
	a
	${macroline}
	.
	w
	q
	END
    if [[ $? -ne 0 ]]
    then
	abort "Couldn't write Macro ${Macro} to ${M4TMP}"
    fi

}

###########################################
#
#  input a macro
#
#  Queries the user for input, and updates the parameter
#  file M4TMP.  A suggested default may be presented.
#  The default values are in a hierarchy (descending priority)
#	Latest value in M4TMP (if any)
#	Original value in USERM4
#	implicit default passed in via PassedVar
###########################################
function input_macro # macro description
{
    typeset macro=$1
    typeset description="$2"
    typeset default orig new

    # Try to get a default value
    default=${PassedVar}
    get_macro_value ${macro} ${USERM4}
    PassedVar=${REPLY:-${default}}
    get_macro_value ${macro}
    PassedVar=${REPLY:-${PassedVar}}

    # Query the user (default implicitly passed via PassedVar)
    get_answer "${description}"
    add_macro ${macro} "${PassedVar}"
}

###########################################
#
#  add to a class
###########################################
function add_to_class # class class_entries
{
    typeset Class Class_entries

    Class=$1
    shift
    Class_entries="$*"

    #
    #  Get the Class into PassedArray, add the
    #  new ones, and then get rid of dups
    #
    get_class_value ${Class}
    set -s -A PassedArray ${PassedArray[*]} ${Class_entries}
    if [ ${#PassedArray[*]} -gt 1 ]; then
	set -s -A PassedArray `print ${PassedArray[*]}|tr ' 	' '\12\12'|sort -u`
    fi

    #
    # fill class file
    #
    ed - ${M4TMP} <<-END >> ${NULL} 2>&1
	/^define(${Class},
	d
	-
	a
	define(${Class},	{${PassedArray[*]}})dnl
	.
	w
	q
	END
}


###########################################
#
#  display a class
#  class is passed in via PassedArray
###########################################
function display_class_array 
{
	integer lbreak=0 i=0

	${ECHO} -n "   "
	while ((i < ${#PassedArray[*]}))
	do
		if ((lbreak > 5)); then
			${ECHO} -n "\n   "
			lbreak=0
		fi
		${ECHO} -n " ${PassedArray[i]}"
		i=i+1
		lbreak=lbreak+1
	done
	${ECHO}
}

###########################################
#
#  interactive add to a class
#
#  Query user for an entry, and adds the value
#  to PassedArray.
###########################################
function I_add_to_class # class description
{
    ${ECHO} "\n\nEnter additions to class (space or <cr> separated) - end list with a <cr>\n"

    while :
    do
	read -r "answer? ? "
	if [ -n "${JURNFILE}" ]; then
		${ECHO} ${answer} >> ${JURNFILE}
	fi
	if [ -z "${answer}" ]; then
		break
	fi
	set -s -A PassedArray ${PassedArray[*]} ${answer}
    done
    ${ECHO}
}

###########################################
#
#  interactive remove from a class
#
#  Query user for an entry, and removes the value
#  to PassedArray.
###########################################
function I_remove_from_class # class description
{
    integer i=0
    typeset answer

    ${ECHO} '\n\nEnter deletions to class one at a time - end list with a <cr>\n'

    while :
    do
	read -r "answer? ? "
	if [ -n "${JURNFILE}" ]; then
	    ${ECHO} ${answer} >> ${JURNFILE}
	fi
	if [ -z "${answer}" ]; then
	    break
	fi
	let i=0
	while ((i < ${#PassedArray[*]}))
	do
	    if [ ${PassedArray[i]} = ${answer} ]; then
		    break
	    fi
	    let i=i+1
	done
	if [ $i -ge ${#PassedArray[*]} ]; then
	    ${ECHO} "\a   ${answer} not member of this class"
	else
	    PassedArray[$i]=""
	    set -s -A PassedArray ${PassedArray[*]}
	fi
    done
    ${ECHO}
}


###########################################
#
#  uncomment file class
###########################################
function uncomment_file_class # class file yes/no
{
Class=$1
ClassFile=$2
if [ "$3" = "no" ]; then
	comment="#"
else
	comment=""
fi
#
#  check for class file
#
grep "^F${Class}[ 	]*${ClassFile}" ${CFTMP} >> ${NULL} 2>&1
if [ $? -eq 0 ]; then
	Target=`echo "^F${Class}[ 	]*${ClassFile}"|sed 's!/!\\\\/!g'`
else
	grep "^[# 	]*F${Class}[ 	]*${ClassFile}" ${CFTMP} >> ${NULL} 2>&1
	if [ $? -eq 0 ]; then
		Target=`echo "^[# 	]*F${Class}[ 	]*${ClassFile}"|sed 's!/!\\\\/!g'`
	else
		return 1
	fi
fi
ed - ${CFTMP} <<END >> ${NULL} 2>&1
/${Target}
s/^[# ]*/${comment}/
w
q
END
return 0
}

#
###########################################
#
#  input a class
###########################################
function input_class # class description
{
    Class=$1
    description="$2"

    typeset answer Target classline

    get_class_value ${Class}

    while :
    do
	if ((${#PassedArray[*]} > 0)); then
	    # Print header.  It may be exceptionaly long, so run it
	    # through fmt.
	    ${ECHO} "The following have been defined for the ${description} class:\n" | fmt -66
	    display_class_array 
	fi

	${ECHO}
	read -r "answer?add to list, delete from list or continue on (a/d/c)? " || return 1
	if [ -n "${JURNFILE}" ]; then
		${ECHO} ${answer} >> ${JURNFILE}
	fi
	case ${answer} in
	[aA]*) I_add_to_class ;;
	[dD]*) I_remove_from_class ;;
	[eEqQcC]*) break ;;
	*) ${ECHO} '\aEnter "d" for delete, "a" for add or "c" to continue [no default]: '
	esac
    done

    #
    #  get rid of dups
    #
    if [ ${#PassedArray[*]} -gt 1 ]; then
	set -s -A PassedArray `print ${PassedArray[*]}|tr ' \11' '\12\12'|sort -u`
    fi

    #
    # fill class file
    #
    Target='^define('${Class}','
    classline='define('${Class}',	{'${PassedArray[*]}'})dnl'
    ed - ${M4TMP} <<-END >> ${NULL} 2>&1
	/${Target}
	d
	-
	a
	${classline}
	.
	w
	q
	END

}


###########################################
#
#  canonhostname(hostname)
#
#  Adds $BINDDOMAIN to unqualified hostnames
#  Returns:
#	name in REPLY
###########################################
function canonhostname	# domain
{
    typeset domain=$1

    if [[ ${domain} = ${domain%.*} ]]
    then
	if [ "${BINDDOMAIN}" != "LOCAL" ]; then
	    domain=${domain}.${BINDDOMAIN}
	fi
    fi

    REPLY=${domain}
    return
}

###########################################
# Canonize Protocol Names
###########################################
function canonproto	# protocol name
{
    # Canonizes name (also forces lower case)
    # By ksh convention, the name is returned in REPLY

    typeset -l answer

    answer=$1
    case "${answer}" in
    tcp*|smtp*)
	answer="smtp" ;;
    dec*|mail*)
	answer="mail11" ;;
    uucp*)
	answer="uucp" ;;
    umc|mts|mr)
	answer="umc" ;;
    esac

    REPLY=${answer}
}


######################################################
#  chk_token(token)
#
#  Checks the token to see if it is valid.  As per
#  RFC 952, it is limited to alphanumerics and '-'
#
######################################################
function chk_token	# token
{
    if [[ -z "$1" || $1 != ${1%[!-a-zA-Z0-9]*} || $# != 1 ]]; then
	return 1
    fi
    return 0
}


######################################################
#  chk_phasevname(name)
#
#  Checks the name to see if it is a valid name.
#
######################################################
function chk_phasevname	#name
{
    typeset ns name token

    if [[ -z "$1" ]]; then
	return 1
    fi

    ns=${1%%:*}
    node=${1#*:}

    if [[ ${ns} != $1 ]]; then
	if chk_token ${ns}
	then :;
	else
	    return 1
	fi
    fi

    for i in `echo ${node} | tr '.' ' '`; do
	if chk_token $i
	then :;
	else
	    return 1
	fi
    done
    return 0
}


######################################################
# get_mailenviron()
#
# Gets the user's mail environment, e.g. DECnet configured, phase IV/V
#
# This may be a good place to expand the functionality 
# to allow the user to set up mail for another host,
# and embed the other host's enviroment here.
######################################################
function get_mailenviron
{
    if [ -x "${MAIL11FILE}" ]; then
	    DECNETCONF=yes
	    #
	    # determine if DECnet is phase IV or V
	    if [ -x "${NCPFILE}" -a -x "${NMLFILE}" ]; then
		    DECNET_PHASE=IV
	    else
		    DECNET_PHASE=V
	    fi
    else
	    DECNETCONF=no
    fi


    if [ -x "${UMCFILE}" ]; then
	    UMCCONF=yes
    else
	    UMCCONF=no
    fi

    if [[ -s /usr/lib/uucp/Systems ]]; then
	UUCPCONF=yes
    else
	UUCPCONF=no
    fi

    if [[ -x "$X25FILE" ]]; then
	X25CONF=yes
    else
	X25CONF=no
    fi

    # Other transports could be added here.
}


######################################################
# get_hostdnsinfo()
#
# get domain name and unqualified hostname
#
# If BIND/DNS is not yet setup, query and possibly
# invoke BINDsetup
######################################################
function get_hostdnsinfo
{
    case "${UNAME}" in
    "ULTRIX")
	HOSTNAME=`hostname`
	;;
    *)
	HOSTNAME=`$RCMGR get HOSTNAME`
	;;
    esac

    #
    # Check to see if BIND has been setup yet.  If BIND is to be setup
    # it would be best if they did it first.
    #
    BINDDOMAIN=""
    if [ -f "${RESOLVC}" ]; then
	BINDDOMAIN=`awk '/^domain/ {print $2}' ${RESOLVC}`
    fi

    if [ -z "${BINDDOMAIN}" ]; then
	${ECHO} "\a\n\n                        SETUP BIND"
	${ECHO}
	${ECHO} "BIND (the Internet Naming Service) has not been setup for this"
	${ECHO} "machine.  If your site is connected with the Internet, it is"
	${ECHO} "strongly advised that you setup BIND first.\n"

	if affirmative 'Do you want to do a bindsetup first ' "y"; then
	    bindsetup
	    if [ -f "${RESOLVC}" ]; then
		BINDDOMAIN=`awk '/^domain/ {print $2}' ${RESOLVC}`
	    fi
	fi
    fi

    if [ -z "${BINDDOMAIN}" ]; then
	    BIND_CONF="NO"
	    BINDDOMAIN="${HOSTNAME#*.}"
    else
	    BIND_CONF="YES"
    fi

    if [ -z "${BINDDOMAIN}" ]; then
	    BINDDOMAIN="LOCAL"
    fi

    HOSTNAME="${HOSTNAME%.${BINDDOMAIN}}"
}


###########################################
# probemx(domain, optional-hostname)
#
# Probe DNS for the best MX record for domain.
# The records are sorted in REPLY, the best in REPLY[0].
#
# If optional-host is given, it returns the hostname
# if the host has an MX record for the domain.
#
# Returns: answer in REPLY (null if none found)
# 	0 if successful
###########################################
function probemx	# domain optional-host
{
    typeset domain=$1
    typeset host=$2


    set -A REPLY

    if [[ ( -z "$domain" || ${BIND_CONF} = "NO" ) || ${domain} = LOCAL ]] ; then
	REPLY="";
	return 1
    fi

    #
    # Add on a trailing "." to the domain name to speed the search
    #
    domain=${domain%%*(.)}'.'
    host=${host%%*(.)}

    if [[ -z ${host} ]]; then
	set -A REPLY `print "set type=MX\n${domain}" | nslookup - 2>/dev/null |\
		awk '/preference =/ { n=substr($4,0,length($4)-1) +0;
				      printf("%08d %s\n", n, $8)
				    }' | sort | sed 's/.* //'`
    else
        if print "set type=MX\n${domain}" | nslookup - 2> /dev/null | \
            fgrep -si "mail exchanger = ${host}"
        then REPLY=${host}
        else REPLY=
        fi
    fi

    # Set return status
    [[ -n ${REPLY} ]]
}

######################################################
#  probe_loop(hostname, pseudodomain)
#  
#  Probes MX records to see if a routing loop occurs,
#  i.e. BIND MX records for the pseudodomain points to
#  this host.  In this case, this host *must* do something
#  beyond simply routing with MX records
#  
#  Returns: true (0) if a loop occurs
######################################################
function probe_loop	# hostname pseudodomain
{
    if [[ -z "$1" || -z "$2" ]]; then
	return 1
    fi

    probemx $2 $1
    return
}


###########################################
# probe_pseudodomain(topdomain, domain-list...)
#
# Probe DNS to find valid pseudodomains
# Probes each `guess' in turn.
# Returns the first found name in REPLY
###########################################
function probe_pseudodomain	# topdomain domain-list...
{
    typeset top=$1

    shift
    for i in $*
    do
	probemx $i.${top}

	if [[ -n ${REPLY} ]]
	then
	    REPLY=$i
	    return 0
	fi
    done
    return 1
}


######################################################
#  probe_ip_dflt_relay(hostname)
#  
#  Obtains the best guess default for the IP relay for this host.
#  The assumption is made that the MX server for the domain
#  is probably a good choice for the site's IP relay.
#  If this host is an MX server, iterate up towards the root.
#  
#  Returns: 0 (and the name in REPLY)
#	or 1 (and NULL) if no good guess
#
######################################################
function probe_ip_dflt_relay	# hostname
{
    typeset host dom
    typeset ans parent

    host=${1%%.*}
    dom=${1#*.}
    # Strip off any trailing periods from the domain name
    dom=${dom%%*(.)}
    parent=${dom#*.}

    #  Check to see if null parms, or if we are in the root domain (e.g. .com)
    if [[ ( -z "$host" || -z "$dom" ) || ( ${dom} = LOCAL || ${dom} = ${parent} ) ]]; then
	REPLY=""
	return 1;
    fi

    #  Iterate until Host is not an MX server for the domain.
    while probemx $dom $host
    do
	dom=$parent
	parent=${dom#*.}
	if [[ $dom = $parent ]]; then
	    REPLY=""
	    return 1
	fi
    done

    if probemx $dom; then
	REPLY=$dom
    else
	REPLY=""
    fi
    return
}


######################################################
#  probe_mx_policy(qual_hostname, protocol, qual_pseudodomain)
#  
#  Returns default forwarding policy for the desired protocol.
#  This helps to implement an organization's policy, since
#  it sets the defaults that a user sees.
#  The choices for a protocol are:
#      Send it directly (if the mailer is present)
#      Send it via BIND/MX records, or
#      Send it to the General Relay (and let it worry about it)
#  
#  In general, BIND is the preferred mechanism, but this is
#  a matter of *policy*.  Since it is policy (and not a hard rule),
#  the logic is encapsulated within this routine.
#  
#  The case of choosing to send it directly is handled earlier by
#  the code for each protocol; this handles the case of how
#  to forward the mail to a relay.
#  
#  The tricky case is where both a General Relay has been
#  set by the user, and BIND/MX records exist.  In this case,
#  the tie breaker has to be an organization's policy.  This
#  is inferred by the domain name.
#  
#  Returns:
#	True (0) if MX routing is desirable
#	$REPLY= y if MX is available & reasonable (no loop)
#		(null) otherwise
#
######################################################
function probe_mx_policy	# qual_hostname, protocol, qual_pseudodomain
{
    typeset host=$1
    typeset proto=$2
    typeset pseudo=$3
    typeset mx

    if [[ ( -z "$1" || -z "$2" ) || -z "$3" ]]; then
	abort "Missing parameters in probe_mx_policy"
    fi

    if probemx ${pseudo} || probe_loop $host $pseudo
    then :;
    else
	REPLY=
	return 1
    fi

    if is_gateway; then
	#  Assume Well-managed and well-configured.  Always use MX records.
	mx=0
    else
	#  Not a gateway (at least not from MX records), so
	#  institute local policies.
	#  The following section can be modified to
	#  have organization-specific policies embedded within
	#  one common source-code base.
	#
	if [[ ${host} != ${host%.dec.com} ]]; then
	    # Digital Specific routing policies
	    case "${proto}" in
	    umc|mail11)	mx=0;;
	    phaseIV)	mx=0;;
	    phaseV)	mx=0;;
	    *)		mx=1;;
	    esac
	else
	    #  Be conservative.  Send it all to the gateway, so when
	    #  things fail, we have less systems to worry about.
	    mx=1
	fi
    fi

    REPLY=y
    return ${mx}
}


######################################################
#  is_gateway
#
#  Returns: T/F if this host is a gateway
#  Implicit Input:
#	$HOSTNAME & $BINDDOMAIN
#
######################################################
function is_gateway	# hostname
{
    #
    #  Static variables aren't possible in shell, so we suffix this
    #  global w. a pretty obvious (and ugly) name
    #
    if [[ -n ${IS_GATEWAY__STATIC} ]]; then
	return ${IS_GATEWAY__STATIC}
    fi

    if [[ ( -z ${HOSTNAME} || -z ${BINDDOMAIN} ) || ${BINDDOMAIN} = LOCAL ]]; then
	return 1
    fi
    if probemx ${BINDDOMAIN} ${HOSTNAME}.${BINDDOMAIN}; then
	IS_GATEWAY__STATIC=0
    else
	IS_GATEWAY__STATIC=1
    fi

    return ${IS_GATEWAY__STATIC}
}


######################################################
#  must_config(protocol)
#
#  Returns true (0) if we must configure this protocol.
#  The check is simplistic - it checks to see if this
#  protocol is needed to reach one of the protocols.
#
#######################################################
function must_config	# protocol
{
    typeset proto ans

    if [[ -z "$1" ]]; then
	return 1
    fi

    canonproto $1
    proto=${REPLY}

    ans=1
    if [[ $proto = smtp ]]; then
	#  We're biased.  We want our users to always configure
	#  SMTP and the Internet protocols.
	ans=0
    fi
    if [[ $proto != uucp ]]; then
	if [[ "${UUCP_RELAY_TRANS}" = $proto ]]; then
	    ans=0
	fi
    fi
    if [[ "${UMCTRANS}" = $proto || "${GENERAL_RELAY_TRANS}" = $proto ]]; then
	ans=0
    fi
    #
    #  TBD: May have to revisit this if we allow one DECnet to be relayed
    #       by the other protocol, e.g. relay Phase V messages via
    #       Phase IV to a host that can handle both.
    #
    if [[ "${PHASEIVTRANS}" = $proto || "${PHASEVTRANS}" = $proto ]]; then
	ans=0
    fi

    return ${ans}
}


###########################################
#  setup the general relay
#
#  Sets: GENERAL_RELAY (used internally only)
#	 ALL_MAIL2RELAY (used internally only)
#	_RelayAll
#	_GateDomain (if the following are unset, then)
#	_GateParent (the corresponding _Trans variable)
#	_GateINET   (doesn't matter)
###########################################
function setup_tcp_relay
{
    typeset temp
    typeset ORDER
    integer num enum i start
    typeset mx_relay mx_dom

    #
    #  Get the General Relay's Name
    #
    ${ECHO} "\n\n                      ADDING A GENERAL RELAY\n"

    if [[ "${MAIL_TYPE}" = "quick" ]]; then
	${ECHO} "Every configuration should define a general relay.  If your"
	${ECHO} "host cannot resolve how to deliver a piece of mail, the mail is"
	${ECHO} "forwarded to this machine for processing.  Since you have chosen"
	${ECHO} "the \`Quick Setup' option, you must define a relay.\n"
    else
	# Advanced setup
	#
	${ECHO} "Most sites should define a general relay.  If your host cannot"
	${ECHO} "resolve how to deliver a mail message, the mail is forwarded to"
	${ECHO} "this general relay for processing.\n"

	if affirmative "Do you wish to add a general purpose relay" "y"
	then :;
	else
	    GENERAL_RELAY=
	    return 1
	fi
	${ECHO}
    fi

    #
    #  Find a reasonable default for the relay
    #
    probe_ip_dflt_relay ${HOSTNAME}.${BINDDOMAIN}
    mx_dom=${REPLY}
    if [[ -n ${mx_dom} ]]; then
	probemx ${mx_dom}
	mx_host=${REPLY}
    fi

    while :
    do
	if [[ -n ${mx_dom} ]]; then
	    ${ECHO} "Mailsetup has queried BIND and found that TCP/IP mail for the"
	    ${ECHO} "domain \`${mx_dom}' is handled by the relay ${mx_host}."
	    ${ECHO} "If you wish to use TCP/IP and ${mx_host%%.*} as your general relay,"
	    ${ECHO} "it is suggested that you enter the phrase \`${mx_dom}', and"
	    ${ECHO} "let BIND forward it to ${mx_host%%.*}.\n"

	else
	    ${ECHO} "Use the fully qualified name if the relay is a TCP/IP node.\n"
	fi


	PassedVar=${mx_dom}
	input_macro "_GateINET" "general purpose relay"
	GENERAL_RELAY=${PassedVar}
	if [ -n "${PassedVar}" ]; then
	    # Check for MX routing loop
	    if probe_loop ${HOSTNAME}.${BINDDOMAIN} ${GENERAL_RELAY}
	    then :;
	    else
		break
	    fi

	    ${ECHO} "\a\n    Using \`${GENERAL_RELAY}' may cause a loop.\n"
	    if affirmative "Use it as your general relay" "n"; then
		break
	    else
		continue
	    fi
	fi

	case ${MAIL_TYPE} in
	quick)
	    ${ECHO} "\a\nYou must have a relay defined.  Please enter one.\n"
	    ;;

	*) 
	    ${ECHO} "\a\nYou have not entered a general purpose relay.  This means that any"
	    ${ECHO} "mail this machine cannot resolve locally will fail (returned to"
	    ${ECHO} "sender)."

	    if affirmative "you do not need a relay" "n"
	    then
		break 
	    fi
	    ;;
	esac
    done

    case "${MAIL_TYPE}" in
    complete)
	# RW add_macro "_RelayAll" ""
	if [ -z "${GENERAL_RELAY}" ]; then
	    #  No Relay specified.  Clear the Gateway variables
	    #  The Trans variables are left untouched, since they
	    #  are not used unless the Gateway variables are set
	    add_macro "_GateDomain" ""
	    add_macro "_GateParent" ""
	    add_macro "_GateINET" ""
	    break
	fi

	#  Else, we do have a General Relay.  Get the Transport used
	#  to reach it, then set the Gateway for the local Domain,
	#  Parent's domain, and Internet appropriately.
	if [[ ${GENERAL_RELAY} = ${mx_dom} ]]; then
	    GENERAL_RELAY_TRANS="smtp"
	else
	    while :
	    do
		${ECHO} "\n\n                   TRANSPORT FOR GENERAL RELAY"
		${ECHO}
		${ECHO} "What protocol do you wish to use when you forward mail to your"
		${ECHO} "general relay?  If it is a TCP/IP node, enter \`tcp'; if you"
		${ECHO} "send mail to the relay via UUCP, enter \`uucp'."
		${ECHO}
		${ECHO} "If you use any other protocol besides TCP/IP to send mail to"
		${ECHO} "the general relay, you must configure and setup that protocol.\n"

		if [ "${DECNETCONF}" = "no" ]; then
		    ${ECHO} 'Enter the transport used to send to relay (tcp or uucp)\n'
		else
		    ${ECHO} 'Enter the transport used to send to relay (tcp, uucp or DECnet)\n'
		fi

		get_macro_value "_TransINET" ${USERM4}
		PassedVar=${REPLY}

		case ${PassedVar} in
		smtp|smtpl|smtpr)
			PassedVar="tcp"
			;;
		mail11)
			PassedVar="decnet"
			;;
		esac
		get_answer "transport"
		canonproto ${PassedVar}
		PassedVar=${REPLY}
		GENERAL_RELAY_TRANS="${REPLY}"

		case ${PassedVar} in
		mail11)
			if [ "${DECNETCONF}" = "no" ]; then
				${ECHO} '\a   Enter tcp or uucp '
				continue
			fi
			break
			;;
		smtp)
			# IP based relay.  Qualify hostname
			canonhostname ${GENERAL_RELAY}
			GENERAL_RELAY=${REPLY}
			break
			;;
		uucp)
			break
			;;
		*)
			if [ "${DECNETCONF}" = "no" ]; then
				${ECHO} '\a   Enter tcp or uucp '
			else
				${ECHO} '\a   Enter tcp, uucp or DECnet '
			fi
			;;
		esac
	    done
	fi

	${ECHO} "#        ${GENERAL_RELAY} is the general relay (DR)" >> ${DESTMP}
	#
	#  Have the Transport.  Set up the gateways.
	#  If LOCAL, all of them get set here, otherwise
	#  we need to find out from the user how they
	#  want to set up the relays.  The Relays are structured
	#  in a Fall-through mode, i.e.
	#     if domain == mydomain, then forward to GateParent
	#     else if domain == my parent's domain, then forward to GateParent
	#     else if not local, then forward to GateInet
	#     else must be local
	#
	add_macro "_GateINET" "${GENERAL_RELAY}"
	if [[ ${GENERAL_RELAY_TRANS} != smtp ]]; then
	    # SMTP is the default; change iff not smtp
	    add_macro "_TransINET" "${GENERAL_RELAY_TRANS}"
	fi

	if [ "${BINDDOMAIN}" = "LOCAL" ]; then
	    add_macro "_GateDomain" "${GENERAL_RELAY}"
	    add_macro "_TransDomain" "${GENERAL_RELAY_TRANS}"
	    add_macro "_GateParent" "${GENERAL_RELAY}"
	    add_macro "_TransParent" "${GENERAL_RELAY_TRANS}"
	    TCP_STYLE="machdom"
	    TCPLOCAL_STYLE="machdom"
	    ALL_MAIL2RELAY=no
	    # add_macro "_CanReachParent" ""
	    return
	fi

	#
	#  Get the Forwarding policy
	#
	if [ "${BINDDOMAIN}" = "${TOPDOMAIN}" ]; then
		start=1
		ORDER="1 2"
	else
		start=0
		ORDER="0 1 2"
	fi
	set -A relplcy_desc \
	    "Send all mail outside of top domain (${TOPDOMAIN}) to relay." \
	    "Send all mail outside of local domain (${BINDDOMAIN}) to relay." \
	    "Send all mail not local to this machine (${HOSTNAME}) to relay."

	${ECHO} "\n\n             SETUP OF SMTP RELAY FORWARDING POLICY"
	${ECHO}
	${ECHO} "Many sites cannot send mail directly throughout the Internet."
	${ECHO} "Typically, these sites can directly reach any host within their"
	${ECHO} "top-level-domain (e.g. any mail address ending in \`${TOPDOMAIN}'),"
	${ECHO} "but to send mail outside of their local area, they use a relay"
	${ECHO} "to forward their mail."
	${ECHO}
	${ECHO} "This section allows you to select which messages to forward to"
	${ECHO} "your general relay.  It is suggested that you send as much mail"
	${ECHO} "directly to its final destination as you can, and send all the"
	${ECHO} "rest to your relay (${GENERAL_RELAY})."

	while :
	do
	    ${ECHO} "\nYou can setup your SMTP mail relay forwarding in one of the"
	    ${ECHO} "following ways:\n"
	    i=$start
	    num=1
	    while (($i < ${#relplcy_desc[*]}))
	    do
		    ${ECHO} "   $num) ${relplcy_desc[i]}"
		    num=num+1
		    i=i+1
	    done

	    PassedVar="1"
	    get_answer "relay policy"
	    enum=${PassedVar}
	    if [ "${enum}" -lt 1 -o "${enum}" -ge "${num}" ]; then
		${ECHO} "\a  *** Enter in range of 1 to ${num} ***"
	    else
		break
	    fi
	done

	#
	#  Got the policy.  Now set up the relays.
	#  N.B. GateINET was already set above.
	#
	enum=enum+start

	if ((enum >= 2)); then
	    add_macro "_GateParent" "${GENERAL_RELAY}"
	    if [[ ${GENERAL_RELAY_TRANS} != smtp ]]; then
		# SMTP is the default; change iff not smtp
		add_macro "_TransParent" "${GENERAL_RELAY_TRANS}"
	    fi
	else
	    add_macro "_GateParent" ""
	fi

	if ((enum >= 3)); then
	    add_macro "_GateDomain" "${GENERAL_RELAY}"
	    if [[ ${GENERAL_RELAY_TRANS} != smtp ]]; then
		# SMTP is the default; change iff not smtp
		add_macro "_TransDomain" "${GENERAL_RELAY_TRANS}"
	    fi
	    ALL_MAIL2RELAY=yes
	else
	    add_macro "_GateDomain" ""
	fi

	;;

    quick)
	canonhostname ${GENERAL_RELAY}
	temp=${REPLY}
	add_macro "_GateINET" "${temp}"
	add_macro "_GateParent" "${temp}"
	add_macro "_GateDomain" "${temp}"
	add_macro "_RelayAll" "T"
	GENERAL_RELAY=${temp}

	${ECHO} "#        ${GENERAL_RELAY} is the general relay (DR)" >> ${DESTMP}
	;;
    esac
}


######################################################
#  setup_decnet_relay(phase)
#
#  Setups the relay for DECnet Phase XX.
#  "Phase" is either IV or V.
#  Modifies:
#	_GateXX, _TransXX
#	PHASExxTRANS
#	_PhaseVns, if DECnet Phase V
#
#  Supports MX records.  It probes to see if MX records
#  exist, and offers the user a reasonable default.
#  The current sendmail.m4 will simply encapsulate
#  and send out over IP (using BIND) if no Gateway is defined.
#
######################################################
function setup_decnet_relay	# phase
{
    typeset -u phase=$1
    typeset mx pseudo relay
    typeset -u dflt
    typeset ns
    typeset PHASEVNS

    if [[ -z $1 || ( $phase != "IV" && $phase != "V" ) ]]; then
	abort "Error calling setup_decnet_relay"
    fi

    #  Set pseudo to be the correct pseudo-domain, e.g. d5net or enet
    eval pseudo="$"{PHASE${phase}DOMAIN}

    if [[ $phase = V ]]; then
	#
	#  Need to get the default Phase V namespace
	#

	#  Get an example namespace for the text, e.g. "dec" if "dec.com"
	ns=${BINDDOMAIN%.*}
	ns=${ns##*.}

	#  Try to get a reasonable default via MX records
	probe_pseudodomain ${pseudo}.${TOPDOMAIN} ${ns} LOCAL
	dflt=$REPLY

	while :
	do
	    ${ECHO} "\n\n                     DECnet/OSI NAMESPACE"
	    ${ECHO}
	    ${ECHO} "What is your default DECnet/OSI (Phase V) name space?  A"
	    ${ECHO} "DECnet/OSI  name space is the total collection of names that one"
	    ${ECHO} "or more DECdns servers know about, look up, manage and share."
	    ${ECHO} "The namespace for your site is the token before the \`:' in your"
	    ${ECHO} "Phase V node name.  For example, \`${ns}' is the name space for"
	    ${ECHO} "the address \`${ns}:.foo.bar'."

	    PassedVar=${dflt}
	    input_macro "_PhaseVns" "DECnet/OSI (phase V) namespace"
	    PHASEVNS="${PassedVar%%:*}"

	    if [ -z "${PHASEVNS}" ]; then
		${ECHO} "\a\nYou must have a namespace."
	    else if chk_token ${PHASEVNS}; then
		break
	    else
		${ECHO} "\a\nThe namespace must be only alphanumerics or a dash."
	    fi
	    fi
	done
	add_macro "_PhaseVns" "${PHASEVNS}"
    fi


    #
    #  The common guts.  Get the relay.  As with the other relays,
    #  probe the MX records to find out if IP/BIND is a reasonable
    #  alternative, as well as the policy for routing, i.e.
    #  use BIND if available, or funnel to the relay.
    #
    if probe_mx_policy ${HOSTNAME}.${BINDDOMAIN} phase${phase} ${pseudo}.${TOPDOMAIN}; then
	dflt="n"
    else
	dflt="y"
    fi
    mx=${REPLY}


    ${ECHO} "\n\n                  ADDING A DECnet Phase ${phase} RELAY"
    if [ ${phase} = "V" ]; then
	${ECHO}
	${ECHO} "The DECnet OSI relay handles your DECnet OSI (Phase V) mail.  All"
	${ECHO} "of your DECnet OSI mail will be forwarded to this relay for"
	${ECHO} -n "delivery."
    else
	${ECHO}
	${ECHO} "The DECnet Phase IV relay handles your DECnet Phase IV mail.  All"
	${ECHO} "of your Phase IV mail will be forwarded to this relay for"
	${ECHO} -n "delivery."
    fi

    if [[ -n $mx ]]; then
	${ECHO} "\n\nMailsetup has found BIND/MX records for DECnet Phase ${phase}.  If you"
	${ECHO} "wish to use TCP/IP and BIND to forward your Phase ${phase} mail, answer"
	${ECHO} "\`no' to the following question."
    else
	${ECHO} "  Without a relay, your Phase ${phase} mail will fail."
    fi

    if affirmative "Do you wish to add a Phase ${phase} relay" $dflt
    then :;
    else
	if [[ -n $mx ]]; then
	    return
	fi

	${ECHO} "\a\nYou have chosen not to setup a DECnet Phase ${phase} relay.  This means"
	${ECHO} "that any mail sent to DECnet will fail (returned to sender)."

	if affirmative "You are not adding a DECnet Phase ${phase} relay" "n"
	then
	    return
	fi
    fi

    ${ECHO} "\nIf using a TCP/IP node, please enter the fully qualified name"
    ${ECHO} "when entering the relay.\n"
    while :
    do
	PassedVar="${GENERAL_RELAY}"
	input_macro "_Gate${phase}" "DECnet Phase ${phase} relay"
	relay=${PassedVar}
	if [ -n "${PassedVar}" ]; then
	    break
	fi

	${ECHO} "\a\nYou have not defined a DECnet Phase ${phase} relay for your machine."
	${ECHO} "This means all mail to DECnet will fail (returned to sender).\n"
	if affirmative "Is this correct " "y"; then
	    return
	fi
    done

    while :
    do
	${ECHO} "\n\n                TRANSPORT FOR DECnet Phase ${phase} RELAY"
	${ECHO}
	${ECHO} "This question refers to the transport protocol which will be used"
	${ECHO} "to send mail to your DECnet Phase ${phase} relay.  If using a TCP/IP"
	${ECHO} "node enter \`tcp'.  If you get mail to the DECnet transport via"
	${ECHO} "UUCP enter \`uucp'."

	${ECHO} 
	${ECHO} "Enter the transport used to send to relay (tcp or uucp)\n"

	eval get_macro_value _Trans${phase} ${USERM4}
	PassedVar=${REPLY}

	case ${PassedVar} in
	smtp|smtpl|smtpr)
		PassedVar="tcp"
		;;
	mail11)
		PassedVar="decnet"
		;;
	esac

	get_answer "transport"
	canonproto ${PassedVar}
	PassedVar=${REPLY}
	eval PHASE${phase}TRANS=${REPLY}
	case ${PassedVar} in
	smtp)
		canonhostname $relay
		relay=${REPLY}
		break
		;;
	uucp)
		break
		;;
	*)
		${ECHO} '\aEnter tcp or uucp'
		;;
	esac
    done

    ${ECHO} "#        ${relay} is the DECnet Phase ${phase} relay (DE)" >> ${DESTMP}

    eval add_macro "_Trans${phase}" "$"{PHASE${phase}TRANS}
    eval add_macro "_Gate${phase}" $relay
}


###########################################
#  setup_decnetV(optional-force-flag)
#
#  Configures this machine to handle DECnet/OSI (Phase V) locally
#  input: optional argument, 'force' if must config decnet
#  Modifies:
#	DECNETSETUP - 'yes' if configured
#	_DollarY - DECnet nodename
#	_MyVname - if Phase V is installed
#	_PhaseVNS - default PhV namespace
#	_GateV - to this host ==> use mail-11
#	_MyNicknames - the inverted IP form of the PhV name
#  Returns:
#	0 - if user configures DECnet locally
###########################################
function setup_decnetV	# optional-force-flag
{
    typeset action="$1"
    typeset hostname name
    typeset temparray
    typeset REVERSEVNAME PHASEVNS


    #
    #  Query user.  Do they want to send DECnet mail directly?
    #
    if [ "${action}" != "force" ]; then
	${ECHO} "\n\n                    DECnet/OSI (Phase V) SETUP"
	${ECHO}
	${ECHO} "Are you setting up this machine to process DECnet/OSI (Phase V)"
	${ECHO} "mail?  If you answer \`yes' to this question, this machine will"
	${ECHO} "process all Phase V mail.  If you wish to relay this mail to"
	${ECHO} "another machine, answer \`no'.\n"

	if affirmative "DECnet/OSI (Phase V) setup" "y"
	then :;
	else
	    return 1
	fi
    fi

    DECNETSETUP=yes

    ${ECHO} "\n\n                    DECnet/OSI NAME DEFINITION"
    ${ECHO}
    ${ECHO} "Enter the DECnet OSI (phase V) node name for this machine.  This"
    ${ECHO} "should be the complete name, including the namespace, but do"
    ${ECHO} "*not* include any .dnet extension (e.g. DEC:.zko.foo).\n"

    while :
    do
	PassedVar=`nodename 2> ${NULL}`
	input_macro "_MyVname" "DECnet mail name of this machine"
	name=${PassedVar%.dnet}
	name=${name%.DNET}
	if [ -z "${name}" ]; then
	    ${ECHO} "\nYou must enter a DECnet node name for this machine."
	    ${ECHO} "Please enter one.\n"
	else if [[ ${name} = ${name%:*} ]]; then
	    ${ECHO} "\a\nYou must include the namespace followed by a \`:' (e.g. DEC:.zko.foo)."
	    ${ECHO} "Please enter the full DECnet OSI name.\n"
	else if chk_phasevname ${name}; then
	    break
	else
	    ${ECHO} "\a\nThe name must be alphanumerics or a dash."
	fi
	fi
	fi
    done

    PHASEVNAME=${name}
    add_macro "_DollarY" "${PHASEVNAME}"

    PHASEVNS="${PHASEVNAME%%:*}"
    add_macro "_PhaseVns" "${PHASEVNS}"

    #
    # Set the Gate variables.  If GateX == this host, then send directly
    # via mail-11
    #
    canonhostname ${HOSTNAME}
    hostname=${REPLY}
    add_macro "_GateV" "${hostname}"

    #
    #  Convert the Phase V name into our reverse mapping
    #  dec:.zko.wasted ==> wasted.zko.dec.pseudo-domain.dec.com
    #
    REVERSEVNAME=""
    set -A temparray `echo "${PHASEVNAME}"|sed "s/://g;s/\./ /g"`
    let i=${#temparray[*]}
    while [ $i -gt 0 ]
    do
	    let i=i-1
	    REVERSEVNAME=${REVERSEVNAME}'.'${temparray[$i]}
    done
    REVERSEVNAME=`echo ${REVERSEVNAME} | sed 's/^\.//'`
    if [ -n "${REVERSEVNAME}" ]; then
	    add_to_class "_MyNicknames" "${REVERSEVNAME}.${PHASEVDOMAIN}.${TOPDOMAIN}"
    fi

    return 0

}


###########################################
#  setup_decnetIV(optional-force-flag)
#
#  Configures this machine to handle DECnet Phase IV locally
#  input: optional argument, 'force' if must config decnet
#  Modifies:
#	DECNETSETUP - 'yes' if configured
#	_DollarY - DECnet nodename
#	_GateIV - to this host ==> use mail-11
#	_MyIVname - DECnet nodename
#	_MyNickNames - add nodname & numeric form of the name
#  Returns:
#	0 - if user configures DECnet locally
###########################################:
function setup_decnetIV	# optional-force-flag
{
    typeset action="$1"
    integer decnetpre decnetpost
    typeset DECNET_NUM DECNET_NUM2 PHASEIVNAME


    #
    #  Query user.  Do they want to send DECnet mail directly?
    #
    if [[ "${action}" != "force" && "${DECNET_PHASE}" = IV ]]; then
	${ECHO} "\n\n                       DECnet Phase IV SETUP"
	${ECHO}
	${ECHO} "Are you setting up this machine to process DECnet Phase IV mail?"
	${ECHO} "If you answer \`yes' to this question, this machine will process"
	${ECHO} "all Phase IV mail.  If you wish to relay this mail to another"
	${ECHO} "machine, answer \`no'.\n"

	if affirmative "DECnet setup" "y"
	then :;
	else
	    return 1
	fi
    fi


    #
    #  Get the Phase IV Node Name (or synonym if this is a Phase V machine)
    #
    if [[ "${DECNET_PHASE}" = V ]]; then
	#
	#  Now get the Phase IV information
	#
	${ECHO} "\n\n          DECNET PHASE IV COMPATIBLE SYNONYM DEFINITION"
	${ECHO}
	${ECHO} "The Phase IV compatible synonym is the Phase IV-style name for"
	${ECHO} "your machine which is stored at the namespace.  The synonym can"
	${ECHO} "be between 1 and 6 characters long and must be unique within"
	${ECHO} "your namespace.\n"

	if affirmative "Do you have a Phase IV compatible synonym" "y"
	then :;
	else
	    return 1
	fi
    else
	DECNETSETUP=yes

	#
	# This is a Phase IV machine.
	# Print a header to get the phase IV info.
	#
	${ECHO} "\n\n             DECNET PHASE IV NODE NAME DEFINITION"
	${ECHO}
	${ECHO} "This section sets the DECnet Phase IV node name of this machine"
	${ECHO} "for DECnet communications."
	${ECHO}
	${ECHO} "Enter the DECnet Phase IV node name for this machine.\n"
    fi
    while :
    do
	#  Get a reasonable default name.  Workaround for Phase V:
	#  make sure the leading DECdns kruft ("DEC:.zko.") is stripped off
	PassedVar=`nodename -s 2> ${NULL}`
	PassedVar=${PassedVar##*.}

	input_macro "_MyIVname" "Phase IV name of this machine"
	if [[ -z "${PassedVar}" ]]; then
	    ${ECHO} "\a\nYou must enter a Phase IV node name for this machine."
	    ${ECHO} "Please enter one.\n"
	else if chk_token ${PassedVar}; then
	    break
	else
	    ${ECHO} "\a\nPlease use only alphanumerics or a dash.\n"
	fi
	fi
    done
    PHASEIVNAME=${PassedVar}
    add_to_class "_MyNicknames" "${PHASEIVNAME}.${PHASEIVDOMAIN}.${TOPDOMAIN}"

    #  Set the GateIV to be this host.  The current sendmail.m4
    #  takes this to mean "use mail11 directly"
    canonhostname ${HOSTNAME}
    add_macro "_GateIV" "${REPLY}"

    #
    #  Get the Phase IV Node Number (area.node).  This is needed since
    #  some mail may arrive addressed to "nodenumber::user".  If we
    #  don't know our number, we'll refuse this mail.
    #
    if [ "${DECNET_PHASE}" = "V" ]; then
	DECNET_NUM=""
	${ECHO} "\nEnter the DECnet Phase IV compatible node number for this machine."
	${ECHO} "The format of the Phase IV number must be area.hostpart or aa.hhh"
	${ECHO} "(i.e. 99.999).\n"

    else
	#  Phase IV.  Set the DECnet node name ($y)
	#  TBD: Should we add different ones for Phase IV & V?
	add_macro "_DollarY" "${PHASEIVNAME}"

	DECNET_NUM=`getnode ${PHASEIVNAME} 2> ${NULL} | tail -1 |awk '{print $3}' `
	${ECHO} "\nEnter the DECnet Phase IV node number for this machine."
	${ECHO} "format of the Phase IV number must be area.hostpart or aa.hhh"
	${ECHO} "(i.e. 99.999).\n"
    fi
    while :
    do
	PassedVar="${DECNET_NUM}"
	get_answer "DECnet node number"
	if [ -z "${PassedVar}" ]; then
	    break;
	fi

	DECNET_NUM=${PassedVar}
	case "${DECNET_NUM}" in
	+([0-9.])) :
		;;
	*)
		DECNET_NUM=""
		;;
	esac
	if [[ ${DECNET_NUM} = ${DECNET_NUM%%.*} ]]; then
	    ${ECHO} "\a\n${PassedVar} is not in the correct format - aa.hhh"
	    DECNET_NUM=""
	else
	    decnetpre=${DECNET_NUM%%.*}
	    decnetpost=${DECNET_NUM##*.}
	    if [ "${decnetpre}" -eq 0 -o "${decnetpost}" -eq 0 \
		  -o "${DECNET_NUM}" != "${decnetpre}.${decnetpost}" ]; then
		${ECHO} "\a\n${PassedVar} is not in the correct format - aa.hhh"
		DECNET_NUM=""
	    else
		DECNET_NUM2=`expr \( ${decnetpre} \* 1024 \) + ${decnetpost}`
		add_to_class "_MyNicknames" "${DECNET_NUM}.${PHASEIVDOMAIN}.${TOPDOMAIN}"
		add_to_class "_MyNicknames" "${DECNET_NUM2}.${PHASEIVDOMAIN}.${TOPDOMAIN}"
		break
	    fi
	fi
    done

    return 0

}

###########################################
#  setup the umc relay
###########################################
function setup_umc_relay
{
    typeset mx dflt

    if probe_mx_policy ${HOSTNAME}.${BINDDOMAIN} "umc" ${UMCDOMAIN}.${TOPDOMAIN}; then
	dflt="n"
    else
	dflt="y"
    fi
    mx=${REPLY}

    ${ECHO} "\n\n                      ADDING A UMC RELAY"
    ${ECHO}
    ${ECHO} "The UMC relay handles your UMC mail.  All of your UMC mail will"
    ${ECHO} -n "be forwarded to this relay for delivery."
    if [[ -n ${mx} ]]; then
	${ECHO} "\n\nMailsetup has found BIND/MX records for UMC.  If you wish to use"
	${ECHO} "TCP/IP and BIND to forward your UMC mail, answer \`no' to the"
	${ECHO} "following question."
    else
	${ECHO} "  Without a relay, your"
	${ECHO} "UMC mail will fail."
    fi

    if affirmative "Do you wish to add a UMC relay " "${dflt}"
    then :;
    else
	if [[ -n ${mx} ]]; then
	    return 1
	fi

	${ECHO} "\a\nYou have chosen not to setup a UMC relay.  This means that any"
	${ECHO} "mail sent to UMC will fail (returned to sender)\n"
	if affirmative "You are not adding a UMC relay" "n"
	then
	    return 1
	fi
    fi

    ${ECHO} "\nIf using a TCP/IP node, please enter the fully qualified name when"
    ${ECHO} "entering the relay.\n"

    while :
    do
	PassedVar="${GENERAL_RELAY}"
	input_macro "_GateMR" "UMC relay"
	UMCRELAY=${PassedVar}
	if [ -n "${PassedVar}" ]; then
	    break
	fi

	${ECHO} "\a\nYou have not defined a UMC relay for your machine.  This means all"
	${ECHO} "mail to UMC will fail (returned to sender).\n"
	if affirmative "Is this correct" "y"; then
	    return 1
	fi
    done

    if [ -z "${UUCPNAME}" -a "${DECNETSETUP}" != "yes" ] ; then
	canonhostname ${UMCRELAY}
	UMCRELAY=${REPLY}
	add_macro "_GateMR" "${UMCRELAY}"
	add_macro "_TransMR" "smtp"
	UMCTRANS=smtp
	return 0
    fi

    while :
    do
	${ECHO} "\n\n                     TRANSPORT FOR UMC RELAY"
	${ECHO}
	${ECHO} "This question refers to the transport protocol which will be used to"
	${ECHO} "send mail to your UMC relay.  If using a TCP/IP node enter tcp."

	if [ "${DECNETSETUP}" = "yes" ] ; then
	    ${ECHO} "If you get mail to the UMC transport via DECnet enter decnet."
	fi
	if [ -n "${UUCPNAME}" ] ; then
	    ${ECHO} "If you get mail to the UMC transport via UUCP enter uucp."
	fi

	${ECHO} "\nEnter the transport used to send to relay"
	get_macro_value _TransMR ${USERM4}
	PassedVar=${REPLY}

	case ${PassedVar} in
	smtp|smtpl|smtpr)
		PassedVar="tcp"
		;;
	mail11)
		PassedVar="decnet"
		;;
	esac
	get_answer "transport"
	canonproto ${PassedVar}
	PassedVar=${REPLY}
	UMCTRANS=${REPLY}
	case ${PassedVar} in
	smtp)
		canonhostname ${UMCRELAY}
		UMCRELAY=${REPLY}
		break
		;;
	uucp)
		if [ -n "${UUCPNAME}" ] ; then
			break
		else
			${ECHO} "\a   UUCP is not configured."
		fi
		;;

	mail11)
		if [ "${DECNETSETUP}" = "yes" ] ; then
			break
		else
			${ECHO} "\a   DECnet is not configured."
		fi
		;;
	*)
		${ECHO} '\a   Enter UMC transport '
		;;
	esac
    done

    add_macro "_TransMR" "${UMCTRANS}"
    add_macro "_GateMR" "${UMCRELAY}"

}


###########################################
#  setup UMC
###########################################
function setup_umc
{

    ${ECHO} "\n\n                          UMC SETUP"
    ${ECHO}
    ${ECHO} "Are you setting up this machine to process UMC mail?  If you"
    ${ECHO} "answer \`yes' to this question, this machine will process all UMC"
    ${ECHO} "mail.  If you wish to relay this mail to another machine, answer"
    ${ECHO} "\`no'."

    if affirmative "   UMC setup " "y"
    then :;
    else
	return 1
    fi

    #
    #  Set GateMR to us.  If the gate == this host, then send directly
    #  via the UMC mailer.  If the gate != this host, then send to
    #  the gate via TransMR (by default from create_m4tmp, smtp).
    #
    canonhostname ${HOSTNAME}
    add_macro "_GateMR" "${REPLY}"

    return 0
}


###########################################
#  setup the uucp relay
#
#  Modifies:
#	_GateUUCP - the relay host <==> UUCP_RELAY
#	_TransUUCP - the transport to reach _GateUUCP
###########################################
function setup_uucp_relay
{
    ${ECHO} "\n                       ADDING A UUCP RELAY"
    ${ECHO}
    ${ECHO} "The UUCP relay will forward UUCP mail for sites not directly"
    ${ECHO} "reachable by this host. The list of reachable sites is read from"
    ${ECHO} "the UUCP /usr/lib/uucp/Systems file."
    ${ECHO}
    ${ECHO} "Without a UUCP relay, all UUCP mail to sites not known by this"
    ${ECHO} "host will fail (returned to sender).\n"

    if affirmative "Do you wish to add a UUCP relay" "y"
    then :;
    else
	return 1
    fi

    ${ECHO} "\nIf using a TCP/IP node, please enter the fully qualified name"
    ${ECHO} "when entering the relay.\n"
    while :
    do
	PassedVar="${GENERAL_RELAY}"
	input_macro "_GateUUCP" "UUCP relay"
	UUCP_RELAY=${PassedVar}
	if [ -n "${PassedVar}" ]; then
	    break
	fi

	${ECHO} "\n\aYou have not defined a UUCP relay for your machine.  This means"
	${ECHO} "all UUCP mail will be handled by this machine, and any UUCP mail"
	${ECHO} "to sites not known by this machine will fail (returned to sender).\n"
	if affirmative "Is this correct " "y"; then
	    return 1
	fi
    done

    #
    # Quick Hook.  Assume must be IP if DECnet is not installed
    #
    if [ "${DECNETCONF}" = "no" ]; then
	canonproto ${UUCP_RELAY}
	UUCP_RELAY=${REPLY}
	add_macro _GateUUCP ${UUCP_RELAY}
	add_macro "_TransUUCP" "smtp"
	UUCP_RELAY_TRANS="smtp"
	return 0
    fi

    while :
    do
	${ECHO} "\n\n                   TRANSPORT FOR UUCP RELAY"
	${ECHO}
	${ECHO} "What protocol do you wish to use when you forward mail to your"
	${ECHO} "UUCP relay?  If it is a TCP/IP node, enter \`tcp'.  If you send"
	${ECHO} "mail to the relay via DECnet, enter \`decnet'."
	${ECHO}
	${ECHO} "Enter the transport used to send to the relay (tcp or decnet)\n"

	get_macro_value "_TransUUCP" ${USERM4}
	PassedVar=${REPLY}

	case ${PassedVar} in
	smtp|smtpl|smtpr)
		PassedVar="tcp"
		;;
	mail11)
		PassedVar="decnet"
		;;
	esac

	get_answer "transport"
	canonproto ${PassedVar}
	PassedVar=${REPLY}
	UUCP_RELAY_TRANS=${REPLY}

	case ${PassedVar} in
	smtp)
	    canonhostname ${UUCP_RELAY}
	    UUCP_RELAY=${REPLY}
	    break
	    ;;
	mail11)
	    break
	    ;;
	*)
	    ${ECHO} '\a   Enter tcp or DECnet '
	    ;;
	esac
    done

    add_macro _GateUUCP ${UUCP_RELAY}
    add_macro "_TransUUCP" "${UUCP_RELAY_TRANS}"
}


###########################################
#  setup uucp(optional-force-flag)
#
#  Configures this host to process UUCP mail.
#  Modifies:
#	_MyUUCPname <==> UUCPNAME
#	_MyNickNames - add on MyUUCPname
#	_GateUUCP - empty ==> process locally
###########################################
function setup_uucp	# optional-force-flag
{
    if [ "$1" != "force" ]; then
	${ECHO} "\n\n                            UUCP SETUP"
	${ECHO}
	${ECHO} "Are you setting up this machine to process UUCP mail?  This means"
	${ECHO} "that this machine can process UUCP mail and does not need a UUCP"
	${ECHO} "relay.  If you have a UUCP relay answer \`no' to this question.\n"
	if affirmative "Do a uucp setup" "$UUCPCONF"
	then :;
	else
	    return 1
	fi
    fi

    ${ECHO} "\n\n                       UUCP NAME DEFINITION"
    ${ECHO}
    ${ECHO} "What is the name of this machine when using UUCP?  It is usually"
    ${ECHO} "the same as the normal host name, but may need to be different"
    ${ECHO} "for various site-specific reasons.\n"
    while :
    do
	PassedVar=${HOSTNAME}
	input_macro "_MyUUCPname" "name of this machine for UUCP communications"
	if [ -n "${PassedVar}" ]; then
	    break
	fi
	${ECHO} "\n\aYou must enter a UUCP node name for this machine."
	${ECHO} "Please enter one."
    done

    UUCPNAME=${PassedVar}
    ${ECHO} "#        ${UUCPNAME} is the UUCP name for this machine (DU)" >> ${DESTMP}
    add_to_class "_MyNicknames" "${UUCPNAME}.UUCP"
    add_macro "_GateUUCP" ""

    return 0
}


###########################################
#  setup Mail Hub/Cluster
#
#  Modifies
#	_MailHub
#	_MailCluster
###########################################
function setup_mailhub
{
    typeset default

    if [[ ${BINDDOMAIN} = "LOCAL" ]]; then
	add_macro "_MailHub" ""
	add_macro "_MailCluster" ""
	return
    fi

    ${ECHO} "\n\n                   MAIL HUB/CENTRAL MAIL SERVER"
    ${ECHO}
    ${ECHO} "A Mail Hub is a machine that can handle mail sent to \`user@domain'."
    ${ECHO} "It has aliases for everyone in your local domain.  When it receives"
    ${ECHO} "mail addressed to \`user@${BINDDOMAIN}', it looks up \`user' in the"
    ${ECHO} "alias file and forwards the mail to its final destination."
    ${ECHO}
    ${ECHO} "Note: if you are planning mail for your site, it is recommended"
    ${ECHO} "    that you configure at least one host as a Mail Hub and set up"
    ${ECHO} "    BIND MX records.  The MX records will direct mail sent to"
    ${ECHO} "    \`@${BINDDOMAIN}' to your Mail Hub."

    if probemx ${BINDDOMAIN} ${HOSTNAME}.${BINDDOMAIN}; then
	default="y"
    else
	default="n"
    fi

    if affirmative "Is ${HOSTNAME} a Mail Hub" ${default}
    then
	add_macro "_MailHub" "T"
    else
	add_macro "_MailHub" ""
	add_macro "_MailCluster" ""
	return
    fi

    ${ECHO} "\n\n                           MAIL CLUSTER"
    ${ECHO}
    ${ECHO} "A Mail Cluster creates the illusion that everyone in your local"
    ${ECHO} "domain exists on a single machine.  This means that instead of"
    ${ECHO} "sending mail to \`user@host.${BINDDOMAIN}', you can send it to \`user',"
    ${ECHO} "regardless of which machine that their mailbox resides on."
    ${ECHO}
    ${ECHO} "To create a Mail Cluster requires *all* machines in your local"
    ${ECHO} "domain (${BINDDOMAIN}) share a common alias file identifying"
    ${ECHO} "every user in your domain.  This is usually done by YP, BIND, or"
    ${ECHO} "by copying an alias file to all the hosts in your domain.\n"

    if affirmative "Is ${HOSTNAME} part of a Mail Cluster" "n"
    then
	add_macro "_MailCluster" ""
    else
	add_macro "_MailCluster" "T"
    fi
}


###########################################
#  setup local users and aliases
###########################################
function setup_local_users_aliases
{
    ${ECHO} "\n\n                     LOCAL USERS AND ALIASES"
    ${ECHO}
    ${ECHO} "Some user names, such as \`root', \`daemon', and \`system', are"
    ${ECHO} "common to all machines.  To avoid confusion any outgoing mail"
    ${ECHO} "sent by these local users are always qualified with your"
    ${ECHO} "machine's name, e.g. \"From: root@${HOSTNAME}\".\n"

    if [[ ${MAIL_TYPE} = quick ]]; then
	${ECHO} "Since you are performing a \`Quick Setup', incoming mail will be"
	${ECHO} "forwarded to the relay, except for these local users.  Mail sent"
	${ECHO} "to these local users will not be forwarded to the relay, but will"
	${ECHO} "be delivered locally on this machine."
	${ECHO}
	${ECHO} "NOTE: if you plan to add any aliases into the system aliases file"
	${ECHO} "    (${CFDIR}/aliases), they must be added to this list.\n"
    fi

    ${ECHO} "The following is the list of local users:\n"

    get_class_value "_NonHiddenUsers"
    display_class_array

    if affirmative "Do you wish to modify this local users/aliases list" "n"
    then
	${ECHO}
	input_class "_NonHiddenUsers" "local users and aliases"
    fi


    if [[ ${MAIL_TYPE} != quick ]]; then
	${ECHO} "\nDo you want aliases in ${CFDIR}/aliases to be considered local?"
	if (affirmative 'Aliases considered local' "y"); then
	    add_macro "_AliasesLocal" "T"
	else
	    add_macro "_AliasesLocal" ""
	fi
    fi
}


###########################################
#  setup address style
###########################################
function setup_tcp_address_format
{
    typeset EXT_FMT1 EXT_FMT2 EXT_FMT3
    typeset ORDER DFLT MXHOST
    typeset address_typ
    typeset address_desc
    typeset address_dom
    typeset tag_val
    integer i num enum

    if [ "${BINDDOMAIN}" = "LOCAL" ]; then
	    MXHOST=
	    if [ -n "${GENERAL_RELAY}" ]; then
		    EXT_FMT1=${HOSTNAME}
		    EXT_FMT2=${GENERAL_RELAY}
		    ORDER="1 2"
	    else
		    add_macro "_ExportedName" ${HOSTNAME}
		    return
	    fi
    else
	    probemx ${BINDDOMAIN}
	    MXHOST=${REPLY}
	    EXT_FMT1=${HOSTNAME}.${BINDDOMAIN}
	    EXT_FMT3=${BINDDOMAIN}
	    if [[ -n "${GENERAL_RELAY}" && "${GENERAL_RELAY}" != ${BINDDOMAIN} ]]; then
		    EXT_FMT2=${GENERAL_RELAY}
		    ORDER="1 2 3"
	    else
		    ORDER="1 3"
	    fi
    fi

    set -A address_typ user sender@${EXT_FMT1} sender@${EXT_FMT2} sender@${EXT_FMT3}
    set -A address_dom "" "${EXT_FMT1}" "${EXT_FMT2}" "${EXT_FMT3}"
    set -A tag_val local machdom relay dom
    set -A address_desc "Format address as user only." \
	    "qualify with your machine name." \
	    "qualify with your relay's name." \
	    "qualify with only your domain name."

    while :
    do
	${ECHO} "\n\n                  TCP RETURN ADDRESS FORMAT"
	${ECHO}
	${ECHO} "When mail leaves your local domain (${BINDDOMAIN}), your return"
	${ECHO} "address needs to be qualified.  You can qualify it in one of the"
	${ECHO} "following ways:\n"


	let num=0
	for i in ${ORDER}
	do
	    num=num+1
	    ${ECHO} "    ${num}) ${address_typ[i]} - ${address_desc[i]}"
	done

	if [[ ${BINDDOMAIN} != LOCAL ]]; then
	    if [[ -n ${MXHOST} ]]; then
		${ECHO} "\nOption #${num} (\`${BINDDOMAIN}') is recommended."
	    else
		${ECHO} "\nOption #${num} (\`${BINDDOMAIN}') is recommended, but requires at least one"
		${ECHO} "host in your local domain to be configured as a Mail Hub."
		${ECHO} "Mailsetup was unable to find any Mail Hubs for your domain."

	    fi
	fi
	${ECHO}

	#
	# See if MX records are set; if so then set the default
	# to be the domain format.
	#
	if [[ -z ${MXHOST} ]]; then
	    DFLT=1
	else
	    DFLT=${num}
	fi

	let enum=0
	while [ "${enum}" -lt 1 -o "${enum}" -gt "${num}" ]
	do
		PassedVar=${DFLT}
		get_answer "tcp address format"
		enum=0
		for i in ${ORDER}
		do
			let enum=enum+1
			if [ "${PassedVar}" = "${enum}" ]; then
				TCP_STYLE=${tag_val[i]}
				EXPORTED_NAME=${address_dom[i]}
				add_macro "_ExportedName" "${address_dom[i]}"
				break 2
			fi
		done
		${ECHO} "\a  *** Enter in range of 1 to ${num} ***"
		enum=0
	done
	break

    done

    #
    # Use smtpr if relay = exported name
    #
    if [ "${EXPORTED_NAME}" = "${GENERAL_RELAY}" ]; then
	    case "${GENERAL_RELAY_TRANS}" in
	    tcp|smtp)
		    add_macro "_TransINET" "smtpr"
		    if [ "${BINDDOMAIN}" = "LOCAL" -o "${ALL_MAIL2RELAY}" = "yes" ]; then
			    add_macro "_TransParent" "smtpr"
			    add_macro "_TransDomain" "smtpr"
		    fi
		    ;;
	    esac
    fi
}

###########################################
#  input top domain
###########################################
function input_top_domain
{
    typeset top

    TOPDOMAIN=${BINDDOMAIN}
    if [ "${BINDDOMAIN}" = "LOCAL" ]; then
	return 0
    fi

    ${ECHO} "\n\n                    TOP LEVEL DOMAIN DEFINITION"
    ${ECHO}
    ${ECHO} "A top level domain is needed if your organization uses any other"
    ${ECHO} "protocols besides TCP/IP to deliver mail (e.g. DECnet or UUCP)."
    ${ECHO} "The top domain is used to encapsulate mail addresses for these"
    ${ECHO} "non-IP protocols before sending mail out over the Internet."
    ${ECHO}
    ${ECHO} "An example of a top domain is \`DEC.COM'.  \`DEC.COM' is the top"
    ${ECHO} "domain for the \`ZK3.DEC.COM' domain.\n"

    #
    # Get a reasonable default, the last two fields of the domain
    #
    top=${BINDDOMAIN%.*.*}
    if [[ ${BINDDOMAIN} != "$top" ]]; then
	top=${BINDDOMAIN#${top}.}
    fi

    PassedVar=$top
    input_macro "_ParentDomain" "Top domain name for this machine" 
    TOPDOMAIN=${PassedVar}
    if [ -n "${TOPDOMAIN}" ]; then
	${ECHO} "#        ${TOPDOMAIN} is the top domain (DT)" >> ${DESTMP}
    fi
    return 0
}


###########################################
# Enter the unqualified hostname
###########################################
function input_hostname
{
    typeset hostname

    ${ECHO} "\n\n                        HOST DEFINITION"
    ${ECHO}
    ${ECHO} "The unqualified hostname is the name of this machine without the"
    ${ECHO} "domain extension.  For example: a machine called \`foo.dec.com'"
    ${ECHO} "would have an unqualified name of \`foo'.\n"

    while :
    do
	PassedVar=${HOSTNAME}
	input_macro "_UnqualifiedName" "unqualified hostname for this machine"
	hostname=${PassedVar%%.*}
	if chk_token ${hostname}; then
	    HOSTNAME=${hostname}
	    break
	else
	    ${ECHO} "Please enter a hostname using only alphanumerics or a dash.\n"
	fi
    done

    CLIENT_M4=${CFDIR}/${HOSTNAME}.m4
    if [ -f "${CLIENT_M4}" ]; then
	${ECHO} "\a\nThe m4 configuration file ${CLIENT_M4}"
	${ECHO} "has been found."
	if ( affirmative "Do you wish to use this file for default values " "y" ); then
	    USERM4=${CLIENT_M4}
	fi
    fi

    ${ECHO} "#        ${HOSTNAME} is the unqualified hostname (DA)" >> ${DESTMP}
}

###########################################
# add the BIND domain
###########################################
function input_primary_domain
{
    typeset -u uppercase

    ${ECHO} "\n\n                        DOMAIN DEFINITION"
    ${ECHO}
    ${ECHO} "If your machine is part of a registered domain, that name should"
    ${ECHO} "be defined here.  Some example domains are ZK3.DEC.COM, MIT.EDU,"
    ${ECHO} "CSS.GOV and CS.INDIANA.EDU."
    ${ECHO}
    ${ECHO} "If you do not have a domain, enter \`LOCAL' here.\n"

    PassedVar=${BINDDOMAIN}
    input_macro "_MyDomain" "BIND domain name for this machine"
    BINDDOMAIN=${PassedVar}
    uppercase=${PassedVar}

    if [ -z "${BINDDOMAIN}" -o "${uppercase}" = "LOCAL" ]; then
	BINDDOMAIN="LOCAL"
	add_macro "_MyDomain" "${BINDDOMAIN}"
    fi

    ${ECHO} "#        ${BINDDOMAIN} is the BIND domain (DD)" >> ${DESTMP}

    add_macro "_MyDomains" "${BINDDOMAIN%%.*}"
}

###########################################
#  input_decnet_domains()
#
#  Modifies:
#	_PhaseIVdomain, PHASEIVDOMAIN - e.g. "enet"
#	_AddMail11Cl - other PhIV domains, e.g. "dnet"
#	_PhaseVdomain, PHASEVDOMAIN - e.g. "d5net"
# 
###########################################
function input_decnet_domains
{
    ${ECHO} "\n\n                      DECnet PHASE IV DOMAIN"
    ${ECHO}
    ${ECHO} "Mail for your organization's DECnet network should be encapsulated"
    ${ECHO} "inside a pseudo-domain such as ENET.DEC.COM or IV.EASYNET.DEC.COM"
    ${ECHO} "before being sent out onto the Internet."
    ${ECHO}
    ${ECHO} "The name that you choose for your Phase IV domain (e.g. \"ENET\")"
    ${ECHO} "is not important, but it must be the same throughout your entire"
    ${ECHO} "organization.  If you are unsure, contact your system administrator."
    ${ECHO}
    ${ECHO} "When entering the Phase IV domain, enter the unqualified name,"
    ${ECHO} "i.e. if you wish to use \"IV.EASYNET.${TOPDOMAIN}\", enter \"IV.EASYNET\".\n"

    #
    # Try to find a reasonable default for the pseudo-domain
    #
    probe_pseudodomain ${TOPDOMAIN} enet dnet decnet eznet iv.eznet
    PassedVar=${REPLY:-ENET}
    input_macro "_PhaseIVdomain" "DECnet phase IV domain"

    #
    # Correct the user's input if they qualified their answer
    # with the topdomain or trailing periods.
    #
    PHASEIVDOMAIN="${PassedVar%%*(.)}"
    PHASEIVDOMAIN="${PHASEIVDOMAIN%.${TOPDOMAIN}}"
    if [ "${PHASEIVDOMAIN}" != "${PassedVar}" ]; then
	    add_macro "_PhaseIVdomain" "${PHASEIVDOMAIN}"
    fi
    add_to_class "_AddMail11Cl" "${PHASEIVDOMAIN}"


    ${ECHO} "\n\n                 OTHER DECnet PHASE IV DOMAINS"
    ${ECHO}
    ${ECHO} "If your users or your inbound mail11 listener puts any pseudo-"
    ${ECHO} "domains other than \`.${PHASEIVDOMAIN}' on incoming addresses, sendmail needs to"
    ${ECHO} "know about it.\n"

    ${ECHO} "The following is the list of DECnet Phase IV pseudodomains:\n"
    get_class_value "_AddMail11Cl"
    display_class_array
    if affirmative "Do you wish to modify this list" "n"
    then
	${ECHO}
	input_class "_AddMail11Cl" "other Phase IV pseudodomains"
    fi


    #
    #  Get the Phase V Domain.  Currently, it cannot be the same
    #  as the Phase IV Domain (although this may change in the future
    #
    PHASEVDOMAIN=${PHASEIVDOMAIN}
    while :
    do
	${ECHO} "\n\n                   DECnet OSI (PHASE V) DOMAIN"
	${ECHO}
	${ECHO} "As above for the phase IV domain, this is your phase V pseudo-"
	${ECHO} "domain such as D5NET.DEC.COM or V.EASYNET.DEC.COM."
	${ECHO} 
	${ECHO} "Do not set this to your phase IV domain (${PHASEIVDOMAIN}).  The domains need"
	${ECHO} "to be unique for proper relaying and encapsulation.\n"

	#
	# Try to find a reasonable default for the pseudo-domain
	#
	probe_pseudodomain ${TOPDOMAIN} d5net e5net dnet5 v.eznet
	PassedVar=${REPLY:-D5NET}
	input_macro "_PhaseVdomain" "DECnet OSI (phase V) domain"

	#
	# Correct the user's input if they qualified their answer
	# with the topdomain or trailing periods.
	#
    	PHASEVDOMAIN="${PassedVar%%*(.)}"
	PHASEVDOMAIN="${PHASEVDOMAIN%.TOPDOMAIN}"
	if [ "${PHASEVDOMAIN}" != "${PassedVar}" ]; then
	    add_macro "_PhaseVdomain" "${PHASEVDOMAIN}"
	fi

	if [ "${PHASEVDOMAIN}" = "${PHASEIVDOMAIN}" ]; then
	    ${ECHO} "\a   Your Phase V domain must be different than your Phase IV domain"
	else
	    break
	fi
    done
}

###########################################
# get nicknames for this machine
###########################################
function input_nicknames
{
    ${ECHO} "\n\n                   NICKNAMES FOR THIS MACHINE"
    ${ECHO}
    ${ECHO} "Are there any other names that are used to send mail to this"
    ${ECHO} "machine?  For instance, if you have changed this host's name (or"
    ${ECHO} "plan to in the near future), a nickname allows sendmail to"
    ${ECHO} "recognize both names, \"${HOSTNAME}\" and the nickname, as synonyms"
    ${ECHO} "for this machine."
    ${ECHO}
    ${ECHO} "Another good use for nicknames occurs when a host receives mail"
    ${ECHO} "from multiple different networks.  A host's name may not be the"
    ${ECHO} "same on all of the different networks.  Again, nicknames allows"
    ${ECHO} "sendmail to recognize these different names as synonyms for this"
    ${ECHO} "host.\n"

    if affirmative "Do you wish to enter nicknames for this machine" "n"; then
	input_class "_MyNicknames" "nicknames for ${HOSTNAME}"
    else
	return 1
    fi
}

###########################################
# setup_quick
###########################################
function setup_quick
{
	#
	# Set hostname
	#
	PassedVar=${HOSTNAME}
	HOSTNAME=""
	add_macro "_UnqualifiedName" "${PassedVar}"
	HOSTNAME=${PassedVar}
	${ECHO} "#        ${HOSTNAME} is the unqualified hostname (DA)" >> ${DESTMP}
	#
	# Get domain if bindsetup not done
	#
	if [ -z "${BINDDOMAIN}" -o "${BIND_CONF}" != "YES" ]; then
		input_primary_domain
	else
		add_macro "_MyDomain" "${BINDDOMAIN}"
		add_macro "_MyDomains" "${BINDDOMAIN%%.*}"
	fi
	${ECHO} "#        ${BINDDOMAIN} is the BIND domain (DD)" >> ${DESTMP}


	#
	# Get the general relay
	#
	setup_tcp_relay
	#
	# Turn off UUCP to send all UUCP to relay
	add_macro "_UUCP" ""
	#
	# Turn off DECnet to send all to relay
	add_macro "_DECNet" ""
	#
	# Turn off POP to send all to relay
	add_macro "_POP" ""
	#
	# Set the local address formatting
	#
	add_macro "_ExportedName" "${GENERAL_RELAY}"
	#
	# Set so all mail goes to relay for resolution
	#
	# add_macro "_CanReachParent" ""
	add_macro "_TransDomain" "smtpr"
	add_macro "_TransINET" "smtpr"
	${ECHO} "#        All mail goes to relay for resolution (DN)" >> ${DESTMP}
	uncomment_file_class N /etc/passwd yes
	${ECHO} "#        users in passwd file are considered local (FN)" >> ${DESTMP}

	setup_local_users_aliases
}

###########################################
# setup_tcp_info
###########################################
function setup_tcp_info
{
	input_hostname
	input_primary_domain
	input_top_domain

	ALL_MAIL2RELAY=no
	setup_tcp_relay
	setup_mailhub
	setup_tcp_address_format
	if [ ! \( "${TCP_STYLE}" = "machdom" -a \
		"${TCPLOCAL_STYLE}" = "machdom" -a \
		"${ALL_MAIL2RELAY}" = "no" \) ]; then
		setup_local_users_aliases
	fi
}

###########################################
# setup_uucp_info
#
# Entry point for UUCP.
# Modifies
#	_UUCP - to recognize UUCP style addresses
# Calls:
#	setup_uucp
#	setup_uucp_relay
###########################################
function setup_uucp_info
{
    if must_config uucp; then
	${ECHO} "\n\n                           UUCP SECTION"
	${ECHO}
	${ECHO} "This section sets up your machine for UUCP mail.  Since you have"
	${ECHO} "chosen to forward mail to one of your relays via UUCP, you must"
	${ECHO} "configure UUCP.\n"
	setup_uucp force
	add_macro "_UUCP" "T"
	return 0
    fi

    ${ECHO} "\n\n                           UUCP SECTION"
    ${ECHO}
    ${ECHO} "This section sets up your machine for UUCP mail.  If you use UUCP"
    ${ECHO} "at your site or need to setup a UUCP relay answer \`yes' to the"
    ${ECHO} "the following question.\n"

    if affirmative "Do you wish to recognize UUCP style addresses" "y"
    then
	add_macro "_UUCP" "T"
    else
	add_macro "_UUCP" ""
	return 1
    fi

    if setup_uucp
    then :;
    else
	setup_uucp_relay
    fi
}

###########################################
# setup_decnet_info
#
# Entry point to configure DECnet (both Phase IV & V)
# Modifies:
#	_DECNet - recognize DECnet style addresses
# Calls:
#	setup_decnetXX - to configure Phase XX on this host,
#	setup_decnet_relay - to configure a relay
#	input_decnet_domains - to config the pseudo domains.
###########################################
function setup_decnet_info
{
    typeset forceV forceIV

    ${ECHO} "\n\n                          DECnet SECTION"
    ${ECHO}
    ${ECHO} -n "This section sets up your machine for DECnet mail.  "

    if must_config mail11; then
	${ECHO} "Since you"
	${ECHO} "have chosen to forward mail to a relay via DECnet, you must"
	${ECHO} "configure DECnet.\n"
    else
	${ECHO} "If you use"
	${ECHO} "DECnet in your organization, or need to deal with DECnet style"
	${ECHO} "addresses answer \`yes' to the following question.\n"

	if affirmative "Do you wish to recognize DECnet style addresses" y
	then :;
	else
	    add_macro "_DECNet" ""
	    return 1
	fi
    fi
    add_macro "_DECNet" "T"

    if [[ "${PHASEIVDOMAIN}" = "${PHASEVDOMAIN}" ]]; then
	#  Get the decnet pseudo-domains.
	#  If the variables aren't the same, then he has been
	#  here before, so we won't bother to re-enter them.
	input_decnet_domains
    fi
	
    if must_config mail11; then
	eval force${DECNET_PHASE}="force"
    fi

    if [[ "$DECNETCONF" = yes && "$DECNET_PHASE" = V ]] && setup_decnetV ${forceV}
    then :;
    else
	setup_decnet_relay "V"
    fi

    if [[ "$DECNETCONF" = yes ]] && setup_decnetIV ${forceIV}
    then :;
    else
	setup_decnet_relay "IV"
    fi
}


###########################################
# setup_umc_info
###########################################
function setup_umc_info
{
    ${ECHO} "\n\n                            UMC SECTION"
    ${ECHO}
    ${ECHO} "This section sets up your machine for UMC mail.  If you use UMC"
    ${ECHO} "at your site or need to deal with UMC style addresses answer \`yes'"
    ${ECHO} "to the following question.\n"

    if affirmative "Do you wish to recognize UMC style addresses" "y"
    then
	add_macro "_UMC" "T"
    else
	add_macro "_UMC" ""
   	return 1
    fi


    ${ECHO} "\n\n                            UMC DOMAIN"
    ${ECHO}
    ${ECHO} "Your locally-reachable UMC will be inside of a pseudo-domain"
    ${ECHO} "such as MTS.DEC.COM (use your own domain here unless you are"
    ${ECHO} "directly connected to Digital Equipment Corporation's internal"
    ${ECHO} "network). The UMC domain is the non-qualified name of the"
    ${ECHO} "pseudodomain.  It is always qualified with the parent or top"
    ${ECHO} "domain (${TOPDOMAIN}) before being emitted into the Internet.\n"

    #
    # Try to find a reasonable default for the pseudo-domain
    #
    probe_pseudodomain ${TOPDOMAIN} mts umc mR
    PassedVar=${REPLY:-MTS}
    input_macro "_MsgRoutDomain" "UMC domain"

    #
    # Correct the user's input if they qualified their answer
    # with the topdomain or trailing periods.
    #
    UMCDOMAIN="${PassedVar%%*(.)}"
    UMCDOMAIN="${UMCDOMAIN%.${TOPDOMAIN}}"
    if [ "${UMCDOMAIN}" != "${PassedVar}" ]; then
	    add_macro "_MsgRoutDomain" "${UMCDOMAIN}"
    fi
    add_to_class "_GateMsgRoutCl" "${UMCDOMAIN}"


    ${ECHO} "\n\n                        OTHER UMC DOMAINS\n"
    ${ECHO}
    ${ECHO} "If your users or other inbound mail puts any pseudodomains other"
    ${ECHO} "than \`.${UMCDOMAIN}' on incoming addresses, sendmail needs to know about it.\n"

    ${ECHO} "The following is the list of UMC pseudodomains:\n"
    get_class_value "_GateMsgRoutCl"
    display_class_array
    if affirmative "Do you wish to modify this list" "n"
    then
	${ECHO}
	input_class "_GateMsgRoutCl" "other UMC pseudodomains"
    fi

    #
    #  To execute UMC directly, we need it installed on this host,
    #  and DECnet installed and configured.  Otherwise, use a relay
    #
    if [[ "${UMCCONF}" = "yes" && "${DECNETSETUP}" = "yes" ]] && setup_umc
    then :;
    else
	setup_umc_relay
    fi
}


function setup_x25_info
{
    #
    #  Limited support for the X.25 PSI mailer.
    #  Support is simply to query and activate the mailer.
    #
    if [[ "$X25CONF" = yes ]]; then
	${ECHO} "\n\n                           X.25 SECTION"
	${ECHO}
	${ECHO} "This section sets up your machine to handle X.25 (PSI) mail.  If"
	${ECHO} "you wish to enable X.25 mail, answer \`yes' to the following"
	${ECHO} "question.\n"

	if affirmative "Do you wish to configure X.25 (PSI) Mail" "y"; then
	    add_macro "_X25Mail" "T"
	fi
    fi
}


###########################################
# save_old  save old config files
###########################################
function save_old # ${ORIGFILE}
{
ORIGFILE=$1
if [ -f "${ORIGFILE}.orig" ]; then
	for i in 6 5 4 3 2 1
	do
		let j=i-1
		if [ -f "${ORIGFILE}.${j}" ]; then
			mv -f ${ORIGFILE}.${j} ${ORIGFILE}.${i}
		fi
	done
	if [ -f "${ORIGFILE}" ]; then
		mv -f ${ORIGFILE} ${ORIGFILE}.0
	fi
else
	if [ -f "${ORIGFILE}" ]; then
		mv -f ${ORIGFILE} ${ORIGFILE}.orig
	fi
fi
}

###########################################
# Creates the sendmail.m4 template file
# If the user has modified it, queries to see
# whether they wish to use it or not.
#
# Uses the temp file $SM4TMP
###########################################
function create_m4template
{
    typeset LDELIM
    typeset chksum tmp

    if [ ! -f "${CFTEMPLATE}" ]; then
	${ECHO} "$0: \aUnable to find the ${CFTEMPLATE}"
	${ECHO} "    file.  This file must exist before $0 can run."
        exit 1
    fi

    # Make sure that the file has most of the RCS cruft removed
    cp ${CFTEMPLATE} ${SM4TMP}
    LDELIM="Log"
    ed - ${SM4TMP} <<-END >> ${NULL} 2>&1
	/^# \$${LDELIM}:/,/^# \$End${LDELIM}/d
	1
	/# HISTORY
	d
	.
	w
	q
	END


    if [ -f "${SENDMAIL_M4}"  ]; then
	if cmp ${SM4TMP} ${SENDMAIL_M4} > ${NULL} 2>&1
	then :;
	else
	    #
	    #  Files are different.  User modification, or simply
	    #  our upgrade?
	    #
	    #  The checksum is empirically obtained from rev 1.1.3.9
	    #
	    sed '/^[ 	]*#/d
		/^[ 	]*$/d' $SENDMAIL_M4 | sum | read chksum tmp
	    if [[ $chksum = 47884 ]]; then
		mv -f ${SM4TMP} ${SENDMAIL_M4}
		return
	    fi
	    ${ECHO} "\aAn m4 configuration file has been found and it is different from the"
	    ${ECHO} "default produced by mailsetup.  Mailsetup does not support a"
	    ${ECHO} "${SENDMAIL_M4} file which has been modified."
	    ${ECHO} "Use this file at your own risk."

	    if affirmative "Do you wish to use this file " "n"; then
		rm -f ${SM4TMP}
	    else
		mv -f ${SM4TMP} ${SENDMAIL_M4}
	    fi
	fi
    else
	mv -f ${SM4TMP} ${SENDMAIL_M4}
    fi

}

###########################################
# Creates the user's temporary hostname.m4
# parameter file with reasonable starting values
###########################################
function create_m4tmp
{
    cat > ${M4TMP} <<-EOT
	dnl
	dnl -- My Internet Domain
	define(_MyDomain,	{\$D})dnl
	dnl
	dnl -- My parent or top Domain - I can reach everyone in this domain.
	define(_ParentDomain,	{}_MyDomain)dnl
	dnl
	dnl -- Other domains I consider local.
	dnl -- Should be at least my current, unqualified domain.
	define(_MyDomains,	{})
	dnl
	dnl -- Name tacked on exported local addresses (user@_ExportedName)
	define(_ExportedName,	{\$j})dnl
	dnl
	dnl -- Define if you are part of a mail cluster.
	define(_MailCluster,	{})dnl
	dnl
	dnl -- Define if you are a mail hub, i.e. can handle @domain addrs
	define(_MailHub,	{})dnl
	dnl
	dnl -- Local SMTP transport
	define(_TransDomain,	{smtpl})dnl
	define(_GateDomain,	{})dnl
	dnl
	dnl -- Other names for me - aliases of my machine
	define(_MyNicknames,	{})dnl
	dnl
	dnl -- Turn on for RFC976 style addresses - "user@foo.bar (my full name)"
	define(_RFC976,	{})dnl
	dnl
	define(_QueueDir,	{/var/spool/mqueue})dnl
	ifelse(_Uname,{OSF1},dnl
	{define(_Mail11path,	{/usr/sbin/mail11v3})}dnl
	,dnl
	{define(_Mail11path,	{/usr/bin/mail11v3})}dnl
	)dnl
	dnl
	dnl -- Trusted users
	define(_TrustedUsers,	{root daemon uucp news})dnl
	dnl
	dnl -- Users which always need \$j tacked on
	define(_NonHiddenUsers,	{root postmaster news uucp mailer-daemon rdist nobody daemon pop})dnl
	dnl
	dnl -- Consider system aliases to be local (T=yes, {}=no)
	define(_AliasesLocal,	{T})dnl
	dnl
	dnl -- Is \$w qualified ULTRIX 4.2 and before = T (T=yes, {}=no)
	define(_UnqualifiedW,	{})dnl
	dnl
	dnl -- Is this an IDA sendmail (T=yes, {}=no)
	define(_IDA,		{T})dnl
	dnl
	dnl -- Using unsupported mail11v3 from gatekeeper.dec.com  (T=yes, {}=no)
	define(_UTK_Mail11,	{})dnl
	dnl
	dnl -- Relay all nonlocal ("CN") to relay (T=yes, {}=no)
	dnl --   (overrides the other _Gatexxx logic)
	define(_RelayAll,	{})dnl
	dnl
	dnl -- Define if you want to be able to recognize POP customers.
	define(_POP,		{T})dnl
	dnl
	dnl  Tag pop with a .POP (T=$+.POP, {}=nope)
	define(_TagPOP,		{})dnl
	dnl
	dnl  Tag pop with a @POP (T=$+<@POP>, {}=nope)
	define(_HostPOP,	{T})dnl
	dnl
	dnl -- INET Relay and transport (if _GateINET {}, we do it ourselves)
	define(_TransINET,	{smtp})dnl
	define(_GateINET,	{})dnl
	dnl
	dnl -- Parent Relay and transport 
	define(_TransParent,	{smtp})dnl
	define(_GateParent,	{})dnl
	dnl
	dnl -- Define if you want to be able to recognize UUCP style addresses.
	define(_UUCP,		{T})dnl
	dnl
	dnl -- Added Args for your uux (UUCP mailer)
	define(_UuxArgs,	{})dnl
	dnl
	dnl -- My UUCP name
	define(_MyUUCPname,	{})dnl
	dnl
	dnl -- UUCP Relay and transport (if _GateUUCP {}, we do it ourselves)
	define(_TransUUCP,	{smtp})dnl
	define(_GateUUCP,	{}_GateINET)dnl
	dnl
	dnl -- My DECnet phaseIV and phaseV name
	define(_MyIVname,	{})dnl
	define(_MyVname,	{})dnl
	dnl
	dnl -- My unqualified Name
	define(_UnqualifiedName,	{})dnl
	dnl
	define(_GateUsenet,	{})dnl
	define(_UsenetTag,	{usenet})dnl
	dnl
	dnl -- Addional Mail11 domains
	define(_AddMail11Cl,	{})dnl
	dnl
	dnl -- Define if you recognize DECnet-style adresses
	define(_DECNet,		{T})dnl
	dnl
	dnl -- Define if you are using UMC - message router
	define(_UMC,		{})dnl
	dnl
	dnl -- Define if the y macro does not define DECnet node name
	define(_DollarY,		{})dnl
	dnl
	dnl -- DECnet Phase IV Relay and transport and domain 
	dnl    (if _GateIV {}, we use MX records & the IP logic)
	dnl    (if _GateIV {thishost}, we use mail11 directly)
	define(_TransIV,	{smtp})dnl
	define(_GateIV,		{})dnl 
	define(_PhaseIVdomain,	{})dnl
	dnl
	dnl -- Reverse phase IV nn.mmm on encapsulation (T=yes {}=no)
	define(_ReversePhaseIV,	{})dnl
	dnl
	dnl -- DECnet OSI local namespace
	define(_PhaseVns,	{})dnl
	dnl
	dnl -- DECnet OSI Relay and transport and domain
	dnl    (same as for GateIV above)
	define(_TransV,		{smtp})dnl
	define(_GateV,		{})dnl
	define(_PhaseVdomain,	{})dnl
	dnl
	define(_TransMR,	{smtp})dnl
	dnl    (if _GateMR {}, we use MX records & the IP logic)
	define(_GateMR,	{})dnl
	define(_GateMsgRoutCl,	{mts mr umc})dnl
	define(_MsgRoutDomain,	{})dnl
	dnl
	dnl -- X25 (PSI) mail
	define(_X25Mail,	{})dnl
	EOT

    if [[ $? -ne 0 ]]; then
	exit 1
    fi
}


###########################################
# Creates the user makefile
###########################################
function create_makefile
{
    if [[ -f ${HOSTMK} ]]; then
	return 0
    fi

    cat > ${HOSTMK} <<-EOT
	.SUFFIXES: .cf .m4

	.m4.cf:; m4 -D_ConfigFile=$< sendmail.m4 > \$@

	all: `basename ${HOSTCF}`

	clean:; -rm -f `basename ${HOSTCF}`

	`basename ${HOSTCF}`: `basename ${HOSTM4}` sendmail.m4
	EOT

    if [[ $? -ne 0 ]]; then
	${ECHO} "Error creating ${makefile}"
	exit 1
    fi
    return
}


###########################################
# Introductory screen
# Returns: T/F if the user wants to continue
###########################################
function do_introduction
{
    ${ECHO} "\n\n                            MAIL SETUP"
    ${ECHO}
    ${ECHO} "Mailsetup configures the sendmail(8) utility.  This allows users"
    ${ECHO} "on your local machine to send and/or receive mail in a networked"
    ${ECHO} "environment."
    ${ECHO}
    ${ECHO} "Mailsetup will prompt you for information.  Some of these questions"
    ${ECHO} "have a default answer contained within square brackets.  If you"
    ${ECHO} "simply press <Return>, the default answer is assumed.\n"

    affirmative "Continue" "y"
    return
}

###########################################
# Query user for type of setup
# Returns: T for a quick setup
###########################################
function query_quicksetup
{
    ${ECHO} "\n\n                        TYPE OF MAIL SETUP"
    ${ECHO}
    ${ECHO} "You may perform a quick setup, or an advanced setup of mail.  A"
    ${ECHO} "quick setup is ideal for workstations that wish to have all of"
    ${ECHO} "their mail handled by a centralized machine.  Quick setup:"
    ${ECHO}
    ${ECHO} "    - Forwards all mail to a relay machine.  This implies"
    ${ECHO} "      that users can create and send mail, but cannot read"
    ${ECHO} "      it on the local machine."
    ${ECHO}
    ${ECHO} "    - Formats the mail so that it looks as it was created on"
    ${ECHO} "      the relay."
    ${ECHO}
    ${ECHO} "    - Uses TCP/IP (SMTP) to send the mail to the relay."
    ${ECHO}
    ${ECHO} "    - Since no mail is delivered locally, to read your mail"
    ${ECHO} "      you must login to another host.  Usually this is"
    ${ECHO} "      a central time share machine.\n"

    affirmative "Do you wish to do a quick setup" "y"
    return
}


###########################################
#  Complete the installation?
#  If `yes', copy the file to sendmail.cf
#  and restart sendmail
###########################################
function finish_install # RESTART_flag
{
    typeset -l RESTART=$1
    integer pid
    typeset SENDMAILSTART
    typeset MSG

    ${ECHO} "\n\n                 MAILSETUP CONFIGURATION COMPLETE"
    ${ECHO}
    if [[ ${RESTART} = "yes" ]]; then
	${ECHO} "Do you wish to complete the installation?  If you answer \`yes',"
	${ECHO} "the new configuration will replace \`sendmail.cf', and sendmail"
	${ECHO} "will be restarted.\n"

	MSG="Complete the setup"
    else
	${ECHO} "Do you wish to save this new sendmail configuration?  If you"
	${ECHO} "answer \`yes', it will be saved as ${CFFILE}.\n"

	MSG="Save the configuration file"
    fi

    if affirmative "${MSG}" "y"
    then
	#
	# Save old versions
	#
	save_old ${CFFILE}
	cp ${HOSTCF} ${CFFILE}

	if [[ "${RESTART}" = "yes" ]]; then
	    ${ECHO} "\nRestarting sendmail..."
	    case "${UNAME}" in
	    "ULTRIX")
		     pid=`/bin/ps ax | grep sendmail | grep connection | sed -e 's/^  *//' -e 's/ .*//' | head -1`
		    if [ "X$pid" != "X" ]; then
			/bin/kill $pid
			sleep 3
		    else
			echo "No pid for SMTP Mail Service found"
		    fi
		    /usr/lib/sendmail -bz
		    /usr/lib/sendmail -bi
		     pid=`/bin/ps ax | grep sendmail | grep connection | sed -e 's/^  *//' -e 's/ .*//' | head -1`
		    if [ "X$pid" != "X" ]; then
			echo "Could not kill SMTP Mail Service"
			echo "  -- Kill and restart sendmail by hand "
		    else
			SENDMAILSTART=`grep "sendmail" "${RCFILE}" | grep "bd"`
			if [ -n "${SENDMAILSTART}" ]; then
			    ${SENDMAILSTART}
			fi
		    fi
		    ;;
	    *)
		    /sbin/init.d/sendmail restart
		    ${RCMGR} set MAIL_CONF YES
		    ;;
	    esac
	fi
    else
	${ECHO} "\nMoving new sendmail.cf file to ${CFFILE}.tmp"
	${ECHO} "and exiting."
	cp ${HOSTCF} ${CFFILE}.tmp
	exit 1
    fi
}


###########################################
# Main - the guts of mailsetup
#
# Mailsetup configures the user's sendmail.
# It creates several files along the way.
#	hostname.m4 - the user's parameter file
#	hostname.cf - the resultant .cf file
#	Makefile.cf.hostname - hostname.m4 ==> hostname.cf
# Optionally, the user can complete the config.
# In this case, hostname.cf is copied to sendmail.cf
# Other files:
#	sendmail.cf.pd - the original template file
#	sendmail.m4 - the (possibly user-modified) template file
###########################################

# Parse arguments

RESTART=yes
while getopts :j:f: c
do  case $c in
	j)
		JURNFILE=${OPTARG}
		rm -f ${JURNFILE}
		;;
	f)
		CFFILE=${OPTARG}
		RESTART=no
		;;
	\:)
		${ECHO} ${USAGE}
		exit 1
		;;
	\?)
		${ECHO} ${USAGE}
		exit 1
		;;
	esac
done
if [ $# -ge $OPTIND ]; then
	${ECHO} ${USAGE}
	exit 1
fi

# Display some basic info immediately for the user prior to doing
# all the generic setup.  It's better to start (and then let the
# user wait) so they at least know *something* is happening.

if do_introduction
then :;
else
    exit 0
fi
case "`whoami`" in
"root")         : ;;
*)      ${ECHO} "$0: \aYou must be root to run mailsetup."
	exit 1
esac

# Start logging into the "DEbug the Setup" file.
${ECHO} "#    The following options have been configured by mailsetup" > ${DESTMP}
${ECHO} "#" >> ${DESTMP}


# Get the user's local environment, e.g. DECnet configured,
# Bind configured, etc.  get_hostdnsinfo may result in a BINDsetup
get_mailenviron
get_hostdnsinfo


# Generic setup.  Create temporary and scratch files.
create_m4template
# This next line is bogus, but leave for now
cp ${CFTEMPLATE} ${CFTMP}
create_m4tmp

#
# OK.  The Guts.  Query the user for quick/advanced setup,
# then run with it.
#
if query_quicksetup
then
    MAIL_TYPE=quick
    setup_quick
else
    MAIL_TYPE=complete
    setup_tcp_info
    setup_uucp_info
    setup_decnet_info
    setup_umc_info
    setup_x25_info
    input_nicknames
fi

#
# Check the transport protocols.  Ensure that they are set up
# if the user is forwarding mail via the protocol.
#
if [[ "${DECNETSETUP}" != "yes" ]] && must_config mail11; then
    setup_decnet_info
fi

if [[ -z "${UUCPNAME}" ]] && must_config uucp; then
    setup_uucp_info
fi


#
# Parameter file is now complete.  Create the final files.
#
HOSTMK=${CFDIR}/Makefile.cf.${HOSTNAME}
HOSTCF=${CFDIR}/${HOSTNAME}.cf
HOSTM4=${CFDIR}/${HOSTNAME}.m4

if mv ${M4TMP} ${HOSTM4}
then :;
else
    print "Error creating ${HOSTM4}; exiting."
    exit 1
fi
if create_makefile
then :;
else
    print "Error creating ${HOSTMK}; exiting."
    exit 1
fi

cd ${CFDIR}
if make -f ${HOSTMK} > ${NULL} 2>&1
then :;
else
    print "Error creating ${HOSTCF}; exiting."
    exit 1
fi
# Remove top junk comments
if sed '/^#(begin m4 junk)/,/^#(end m4 junk)/d' ${HOSTCF}>${CFTMP} && mv ${CFTMP} ${HOSTCF}
then :;
else
    print "Error creating ${HOSTCF}; exiting."
    exit 1
fi

finish_install ${RESTART}
