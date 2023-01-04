#!/bin/sh
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
# @(#)$RCSfile: bindsetup.sh,v $ $Revision: 4.2.3.9 $ (DEC) $Date: 1992/11/19 15:44:11 $ 
# 
# Purpose:	Set up Berkeley Internet Name Domain (BIND)
# Usage:	bindsetup
# Environment:	Bourne shell script
# 
# Remarks:
#    Sets up files:
#	/etc/rc.config
#	/etc/hosts
#	/var/adm/sendmail/sendmail.cf
#	/etc/resolv.conf
#	/etc/namedb/named.boot
#	/etc/namedb/named.ca
#	/etc/namedb/named.local
#	/etc/namedb/named.hosts
#	/etc/namedb/named.rev
#
#
# Modification History:
#
# 17-Feb-88	logcher
#    This script sets up a BIND Client interactively
#    and silently (diskless setup) modifying /etc/svcorder,
#    /etc/resolv.conf, and /etc/rc.local with the complete 
#    (hostname+binddomain).  For a BIND Primary or Secondary Server
#    setup, it sets up the /etc/svcorder, /etc/rc.local with the new
#    hostname plus the named, and /etc/named.boot file after interactive
#    queries and then asks the user to set up the remaining BIND data
#    files and points to the Network Manager's Guide.
#
# 08-Mar-88	logcher
#    Modifications after Documentation looked at script output
#	plus a bug (: => ;) fix and other cleanup.
#
# 16-Mar-88	logcher
#    Changed a few comments on the RESOLVC and NAMEDBO files to remain
#    consistent with the comments in the default /etc/namedb files on
#    kit.
#
# 31-Mar-88	logcher
#    Removed code that asks for the reverse network number for servers
#    and instead use some dms code and call a c program, revnetnum,
#    so that the reverse network number is extracted from the system
#    per smu-02198.
#
# 14-Apr-88	logcher
#    Changed references of Remote Server to Client.  Added local to
#    $SVCORDER file as well as adding full hostname to $HOSTS per
#    smu-02244.  Fixed bug in grep for "bind" or "BIND" to include
#    a "^" character.  Allow for multiple servers on client setup,
#    both interactively and silently.  Put in missing setup for
#    caching only and slave servers.
#
# 26-Apr-88	logcher
#    Changes needed to use bindsetup -c ... for diskless client /
#    BIND client setup.  Needed to add an argument that contained
#    the directory path of the root filesystem, CLIENTROOT.
#
# 08-Jun-88	logcher
#    Moved the editing of /etc/svcorder to the end of the script so
#    that in a server configuration the local service is the only one
#    defined for the "arp" command and thus avoids the time-out period
#    for a non-active bind entry.
#
# 13-Jun-88	logcher
#    Put the "verbose" check on the /etc/svcorder message for diskless
#    addition.
#
# 01-Dec-88	logcher
#    Added Donnie Cherng's fix that uses netmasks to get the network
#    number.
#
# 24-Feb-89	logcher
#    Bug fixes from u32_qar 640, v24_qar 781, pmax_bar 379.
#    Check if NAMEDDIR is a file and loop.  If not a directory,
#    then mkdir it.  Change ed to cat because of 64 char limit
#    on ed pathnames.  Changed other ed's to cd to the directory.
#    Added code to cleanup old files if changing configuration
#    If server to client, remove BIND stuff from RCFILE and remove
#    NAMEDPID file.  If client to server, mv RESOLVC to RESOLVC.old.  
#
# 28-Feb-89	logcher
#    This is the real bug fix for u32_qar 640.  When configuring a
#    client, check if arg is a directory.  If not, check if first
#    character in arg is a "/".  If so, print a message that not a
#    directory and exit.  Else, it is assumed that there is no client
#    root directory and arg is assumed to be the domain name.
#
# 08-Mar-89	logcher
#    Added umask 022 to make sure a user can read necessary files,
#    like resolv.conf.
#
# 09-May-89	logcher
#    Removed all code that modifies the hostname.  A RESOLVC file is
#    now required for all configurations with at least the "domain"
#    entry.  This change is in conjuction with the gethostent change
#    that strips of the local domain, if it exists.  Added code to make
#    the NAMEDLO and NAMEDCA files and start secondary and slave servers
#    up automatically.  Removed caching only server from list since it's
#    meaning is very little.  Removed code that calls revnetnum().  Only
#    need network number, not network number plus subnets.
#
# 24-Jul-89	sue
#    Added code to automatically setup all configurations.
#
# 03-Aug-89	sue
#	Set auto to y for automatic named startup and only ask if auto
#	setup is desired if auto is y.  Check if
#	/var/dss/namedb/src/hosts is a file before trying to manipulate
#	it.  Change name of sri-nic.arpa. to nic.ddn.mil. per mail from
#	Win Treese.
#
# 16-Aug-89	sue
#	Brought back caching server setup.  It does have meaning.
#	Removed forwarding server from primary and secondary.
#	Changed the names of the hosts databases to hosts.db and
#	hosts.rev.  Removed setup for these files.  It is now performed 
#	in /var/dss/namedb/bin/make_hosts which is run when setting up a
#	primary server.  Added Hesiod setup to primary and secondary.
#
# 22-Aug-89	sue
#	Added Kerberos questions to setup.  Can only be used in
#	conjuction with Kerberos setup which is manually right now.
#
# 19-Sep-89	sue
#	Added code to the primary server setup to check if a passwd
#	file exists in the src directory and then add code to the 
#	RCFILE and start the hesupd automatically.
#
# 04-Oct-89	sue
#	Added cache files to the secondary server named.boot file.
#	Fixed incorrect names in named.ca.
#
# 13-Nov-89	sue
#	Added code to modify the $SENDMAILCF file with the domain name.
#
# 12-Dec-89	sue
#	Modified the silent client command line input.  Now conforms to
#	that of svcsetup.  Do checking for the $CLIENTROOT$HOSTS file in
#	silent client mode.  Check and see if diskless client was a 
#	server once before and remove lines from $CLIENTROOT$RCFILE.
#	Add example domain name in prompt.  Added more specifics to
#	greps.  Really brought back caching server.
#
# 07-Feb-90	sue
#	Check to see if user added a dot at the end of the domain name.
#	If so, remove it.  Loop for more than one Kerberos server, and
#	add to /etc/hosts if not there already.  Bring back the long
#	hostname setup.  Setting /bin/hostname, and changing
#	/etc/rc.local.
#
# 28-Feb-90	sue
#	Updated the root name servers in named.ca; re mail from Win.
#
# 17-Jul-90	sue
#	Updated the root name servers in named.ca; re mail from Win.
#
# 15-Oct-90	sue
#	Added a "cd /" before changing the .$SENDMAILCF and .$RCFILE
#	files if $CLIENTROOT is null.
#
# Aug-91	bbrown
#	Ported to OSF.
#
# 5-Nov-91	bbrown
#	Added full mail support, added call to /usr/sbin/svcsetup, changed
#	named.boot and resolv.conf file header, added option to add hosts to
#	/etc/hosts, added check for the existence of the inet subset.
#

# files

ECHO=/bin/echo

case $1 in
DEBUG)
	shift
	DEBUG=1
	RCCONF=/tmp/rc.config
	HOSTS=/tmp/hosts
	SENDMAILCF=/tmp/sendmail.cf
	RESOLVC=/tmp/resolv.conf
	SVC=/tmp/svc.conf
	SVCORDER=/tmp/svcorder
	NAMEDBO=/tmp/named.boot
	NAMEDPID=/tmp/named.pid
$ECHO "Running in DEBUG mode ...
"
	;;
*)
	RCCONF=/etc/rc.config
	HOSTS=/etc/hosts
	SENDMAILCF=/var/adm/sendmail/sendmail.cf
	RESOLVC=/etc/resolv.conf
	SVC=/etc/svc.conf
	SVCORDER=/etc/svcorder
	NAMEDBO=/etc/namedb/named.boot
	NAMEDPID=/etc/named.pid
	;;
esac
#
# Default named file locations
#
NAMEDDIR=/etc/namedb
NAMEDSRC=$NAMEDDIR/src
NAMEDBIN=$NAMEDDIR/bin
NAMEDLO=named.local
NAMEDCA=named.ca
NAMEDHO=hosts.db
NAMEDRE=hosts.rev
HO=hosts
DATABASES="hosts"
DOM=domain
SVCSETUP=/usr/sbin/svcsetup
RCMGR=/usr/sbin/rcmgr
SENDMAIL=/sbin/init.d/sendmail
#
# Other declarations
#
umask 022
CLIENTROOT=""
RESTMP=/tmp/resolv.tmp.$$
HOSTSTMP=/tmp/hosts.tmp.$$
BINDTMP=/tmp/bindsetup.tmp.$$
RCTMP=/tmp/bindsetup.rc.$$
HOSTNAME=/bin/hostname
NULL=/dev/null
SHELL_LIB=
SHELL_LIB=${SHELL_LIB:-/usr/share/lib/shell}
INETSUBSET=OSFINET

# defines
PATH=$PATH:/sbin::$NAMEDDIR
export PATH

typeserver=""
server=""
slash=""
RESTARTMAIL=""
PRINTRC="y"
PRINTHO="y"
PRINTSENDM="y"
verbose=y
first_time=y
scratch_maps=y
auto=y
new_binddomain=0
#
# Set up interrupt handlers:
#
QUIT='
	if [ -r $RESTMP ]
	then
		rm $RESTMP
	fi
	if [ -r $HOSTSTMP ]
	then
		rm $HOSTSTMP
	fi
	if [ -r $BINDTMP ]
	then
		rm $BINDTMP
	fi
	$ECHO "bindsetup terminated with no installations made."
	exit 1
'

#
# Trap ^c signal, etc.
#
trap 'eval "$QUIT"' 1 2 3 15

# source and initialize the Dep code
. $SHELL_LIB/libscp
STL_DepInit

#
# PHASE ONE: Gather data!!
#
if [ -n "$1" ]
then
	#
	# Require it to be run by root
	#
	if [ \! -w $RCCONF ]
	then
		exit 1
	fi
	#
	# Run fast and silent for DMS client setup.
	#
	case $1 in
	-c)
		verbose=""
		typeserver=c
		shift
		;;
	*)
		$ECHO "usage: bindsetup [ -c [ -d directory ] -b binddomain name1,ip1 name2,ip2 ... ]"
		eval "$QUIT"
		;;
	esac
	while [ -n "$1" ]
	do
		case $1 in
		-d)
			shift
			if [ -d $1 ]
			then
				CLIENTROOT=$1
				shift
			else
				$ECHO "$1 is not a directory."
				eval "$QUIT"
			fi
			;;
		-b)
			shift
			if [ -n "$1" ] 
			then
				binddomain=$1
				$ECHO $binddomain | egrep -s "\.$"
				if [ $? -eq 0 ]
				then
					binddomain=`$ECHO $binddomain | sed s/\.$//`
				fi
				shift
			fi
			;;
		*)
			name=`$ECHO $1 | awk -F, '{print $1}'`
			number=`$ECHO $1 | awk -F, '{print $2}'`
			if [ -z "$name" -o -z "$number" ]
			then
				$ECHO "name,ip argument is improperly formatted"
				$ECHO "usage: bindsetup [ -c [ -d directory ] -b binddomain name1,ip1 name2,ip2 ... ]"
				eval "$QUIT"
			else
				$ECHO "nameserver	$number" >> $RESTMP
				if [ \! -f $CLIENTROOT$HOSTS ]
				then
#					egrep -s "[ 	]$name" $CLIENTROOT$HOSTS
# 					if [ $? -ne 0 ]
#					then
#						name=`$ECHO $name | sed s/.$binddomain//`
#						$ECHO "$number $name.$binddomain $name		# BIND Server" >> $HOSTSTMP
#					fi
#				else
					$ECHO "$CLIENTROOT$HOSTS is not a file."
					eval "$QUIT"
				fi
				shift
			fi
			;;
		esac
	done

	TMP=`$RCMGR get BIND_CONF`

	if [ \! -z "$TMP" ]
	then
		first_time=""
	fi
fi

if [ $verbose ]
then
	#
	# Require it to be run by root
	#
	if [ \! -w $RCCONF ]
	then
		$ECHO "Su to root first."
		eval "$QUIT"
	fi

#???	#
	# Be sure network has already been set up, and this system has
	# a name!!
	#

	hname=`$HOSTNAME`
	if [ $? -ne 0 ]
	then
		$ECHO "
Bring the system to multi-user mode before running bindsetup."
		eval "$QUIT"
	fi

	$ECHO -n "
The bindsetup command allows you to add, modify, or remove a
configuration of the Berkeley Internet Name Domain (BIND) Server on
your system.  BIND is a network naming service that enables servers
to name resources or objects and share information with other objects
on the network.
"
	$ECHO "
The bindsetup command takes you through the configuration process to
set the default BIND domain name of your system, determine the type
of server or client, and construct the BIND database files for this
BIND domain.  Default answers are shown in square brackets ([]).  To
use a default answer, press the RETURN key.

[ Press the RETURN key to continue ]: "
	read junk

	done=
	while test -z "$done"
	do
		$ECHO "	Berkeley Internet Name Domain (BIND)
	Action Menu for Configuration

	Add              => a
	Modify           => m
	Remove           => r
	Exit             => e"

		$ECHO
		$ECHO -n "Enter your choice [a]: "
		read action
		case $action in
		A|a|M|m|R|r|e|E|"") done=done;;
		esac
	done
	case $action in
	[AaMm]|"")
		#
		# See if this is a re-install
		#
		TMP=`$RCMGR get BIND_CONF`
		if [ "$TMP" = "YES" ]
		then
			$ECHO "
The BIND environment for this host has already been installed.
Would you like to change the current BIND configuration?"
			again=y
			while [ $again ]
			do
				again=""
				$ECHO -n "
Enter \"y\" or \"n\" [no default]: "
				read ans
				case $ans in
				[yY]*)
					first_time=""
					;;
				[nN]*)
					eval "$QUIT"
					;;
				*)
					again=y
					;;
				esac
			done
		fi
		#
		# Ask for the binddomain
		#
		$ECHO "
You must know the default BIND domain name for your site in order to
continue the configuration process.  The name "dec.com" is an example
BIND domain name.  If you do not know the name, contact your site
administrator."
		answer=y
		while [ $answer ]
		do
			$ECHO -n "
Enter \"e\" to EXIT or \"c\" to CONTINUE [no default] ? "
			read ans
			case $ans in
			[cC]*)
				answer=""
				;;
			[eE]*)
				eval "$QUIT"
				;;
			*)
				;;
			esac
		done
		if [ -f $RESOLVC ]
		then
			set xx `(grep "^$DOM" $RESOLVC)`
			if [ -n "$3" ]
			then
				binddomain=$3
			fi
		fi
		if [ -z "$binddomain" ]
		then
			binddomain=`expr $hname : '[^.]*\.\(.*\)'`
		fi
		again=y
		while [ $again ]
		do
			$ECHO -n "
Enter the default BIND domain name [$binddomain]: "
			read ans
			case $ans in
			"")
				if [ $binddomain ]
				then
					again=""
				fi
				;;
			*)
				again=""
				if [ "$binddomain" != "$ans" -a -n "$binddomain" ]
				then
					new_binddomain=1
					old_binddomain=$binddomain
				fi
				binddomain=$ans
				$ECHO $binddomain | egrep -s "\.$"
				if [ $? -eq 0 ]
				then
					binddomain=`$ECHO $binddomain | sed s/\.$//`
				fi
				;;
			esac
		done

		done=
		while test -z "$done"
		do
			$ECHO "
	Berkeley Internet Name Domain (BIND)
	Configuration Menu for domain \"${binddomain}\" 

	Primary Server   => p
	Secondary Server => s
	Caching Server   => a
	Slave Server     => l
        Client           => c
	Exit             => e"

			$ECHO
			$ECHO -n "Enter your choice [c]: "
			read typeserver
			case $typeserver in
			P|p|S|s|A|a|L|l) 
				(cd /; STL_DepEval $INETSUBSET???)
				STATUS=$?
				if [ $STATUS -eq 0 ]
				then
					done=done
				else
					$ECHO "
	$INETSUBSET is not installed.  In order to configure a primary,
	secondary, caching, or slave server, this subset must be installed."
				fi
				;;
			C|c|e|E|"") done=done;;
			esac
		done

		case $typeserver in
		[pP])
			
			cp $NULL $RESTMP
			/bin/ls $NAMEDSRC > $BINDTMP
			if [ -s $BINDTMP ]
			then
				for i in `cat $BINDTMP`
				do
					for j in $DATABASES
					do
						if [ "$i" = "$j" ]
						then
							$ECHO $i >> $RESTMP
						fi
					done
				done
			fi
			if [ -s $RESTMP ]
			then
				$ECHO "
Bindsetup has found the following files in $NAMEDSRC:"
				for i in `cat $RESTMP`
				do
					$ECHO "	$i"
				done
				answer=y
				while [ $answer ]
				do
					$ECHO -n "
Would you like bindsetup to convert the host file to the BIND database
format?  If you answer \"y\", the host file in $NAMEDSRC will be used to
create a bind database in $NAMEDDIR.  If you answer \"n\", the BIND
database will not be created, but all other configuration files will
be created.  (y/n) ? "
					read ans
					case $ans in
					[yY]*)
						auto=y
						answer=""
						;;
					[nN]*)
						auto=n
						answer=""
						;;
					*)
						;;
					esac
				done
			else 
				auto=n
				$ECHO "
Bindsetup has found NO host file in $NAMEDSRC and
therefore cannot create the BIND database."
				answer=y
				while [ $answer ]
				do
					$ECHO -n "
Would you like to continue bindsetup and create the BIND
database from the hosts file after the setup is complete
(y/n) ? "
					read ans
					case $ans in
					[yY]*)
						answer=""
						;;
					[nN]*)
						eval "$QUIT"
						;;
					*)
						;;
					esac
				done
			fi
			;;
		[sS])
			$ECHO "
Before configuring your system as a BIND secondary server, you
must know the host name and Internet address of the BIND primary
server for this domain."
			answer=y
			while [ $answer ]
			do
				$ECHO -n "
Enter \"e\" to EXIT or \"c\" to CONTINUE [no default] ? "
				read ans
				case $ans in
				[cC]*)
					answer=""
					;;
				[eE]*)
					eval "$QUIT"
					;;
				*)
					;;
				esac
			done
			#
			# Ask for servers ...
			#
			$ECHO -n "
If you do not want bindsetup to modify the hostname you enter, specify a
fully qualified server name, (ex. foo.bar.dec.com. ) and add a trailing dot.
For example:

		\"foo.bar.dec.com.\"  --> foo.bar.dec.com

Otherwise, if you do not append a trailing dot, (ex. foo.bar) bindsetup
will append the default domain name.  For example:

		\"foo.bar\" --> foo.bar.dec.com 

"
			again=y
			while [ $again ]
			do
				again=""
				$ECHO -n "
Enter the host name of the BIND primary server for the \"$binddomain\"
domain: "
				read name
				case $name in
				"")
					again=y
					;;
				esac
			done

			$ECHO $name > $BINDTMP
			egrep -s "\.$" $BINDTMP
			if [ $? -eq 1 ]
			then
				name=`$ECHO $name | sed 's/$/\.'$binddomain'/'`
			else
				name=`$ECHO $name | sed s/\.$//`
			fi

			binddomain_tmp=$name
			name=`$ECHO $name | sed 's/\..*$//'`
			binddomain_tmp=`$ECHO $binddomain_tmp | sed 's/'$name'//'`
			binddomain_tmp=`$ECHO $binddomain_tmp | sed 's/^\.//'`

			notgotit=y
			set xx `(grep "[ 	]$name" $HOSTS)`
			if [ -n "$2" ]
			then
				number=$2
				notgotit=""
			fi
			again=y
			while [ $again ]
			do
				again=""
				$ECHO -n "
Enter the Internet address for $name.$binddomain_tmp [$number]: "
				read ans
				case $ans in
				"")
					if [ -z "$number" ]
					then
						again="y"
					fi
					;;
				*)
					number=$ans
					;;
				esac
			done
			if [ -n "$notgotit" -o "$2" != "$number" ]
			then
				answer=y
				while [ $answer ]
				do
					$ECHO -n "
Would you like to add $name to the $HOSTS file (y/n) [n] ? "
					read ans
					case $ans in
					[yY]*)
						if [ $binddomain_tmp ]
						then
							$ECHO "$number $name.$binddomain_tmp $name		# BIND server" >> $HOSTSTMP
						else
							$ECHO "$number $name	# BIND server" >> $HOSTSTMP
						fi
						answer=""
						;;
					[nN]*|"")
						answer=""
						;;
					*)
						;;
					esac
				done
			fi
			IPNETPRI=$number
			;;
		[lL])
			$ECHO "
Before configuring your system as a BIND slave server, you must know
the host name and Internet address of the specified BIND server(s) for
this domain."
			answer=y
			FORWSERVER=""
			while [ $answer ]
			do
				$ECHO -n "
Enter \"e\" to EXIT or \"c\" to CONTINUE [no default] ? "
				read ans
				case $ans in
				[cC]*)
					answer=""
					;;
				[eE]*)
					eval "$QUIT"
					;;
				*)
					;;
				esac
			done
			#
			# Ask for servers ...
			#
			$ECHO -n "
If you do not want bindsetup to modify the hostname you enter, specify a
fully qualified server name, (ex. foo.bar.dec.com. ) and add a trailing dot.
For example:

                \"foo.bar.dec.com.\"  --> foo.bar.dec.com

Otherwise, if you do not append a trailing dot, (ex. foo.bar) bindsetup
will append the default domain name.  For example:

                \"foo.bar\" --> foo.bar.dec.com

"
			$ECHO "
When finished entering BIND server(s), press the RETURN key only."
			again=y
			first=y
			while [ $again ]
			do
				$ECHO -n "
Enter the host name of the BIND server: "
				read name
				case $name in
				"")
					if [ $first ]
					then
						$ECHO "
At least one BIND server must be entered."
					else
						$ECHO -n "
Finished entering BIND server(s) (y/n) [n] : "
						read ans
						case $ans in
						[yY]*)
							again=""
							;;
						[nN]*|"")
							;;
						*)
							;;
						esac
					fi
					;;
				*)
					first=""

					$ECHO $name > $BINDTMP
					egrep -s "\.$" $BINDTMP
					if [ $? -eq 1 ]
					then
						name=`$ECHO $name | sed 's/$/\.'$binddomain'/'`
					else
						name=`$ECHO $name | sed s/\.$//`
					fi

					binddomain_tmp=$name
					name=`$ECHO $name | sed 's/\..*$//'`
					binddomain_tmp=`$ECHO $binddomain_tmp | sed 's/'$name'//'`
					binddomain_tmp=`$ECHO $binddomain_tmp | sed 's/^\.//'`

					notgotit=y
					set xx `(grep "[ 	]$name" $HOSTS)`
					if [ -n "$2" ]
					then
						number=$2
						notgotit=""
					else
						number=""
					fi
					againn=y
					while [ $againn ]
					do
						againn=""
						$ECHO -n "
Enter the Internet address for $name.$binddomain_tmp [$number]: "
						read ans
						case $ans in
						"")
							if [ -z "$number" ]
							then
								againn="y"
							fi
							;;
						*)
							number=$ans
							;;
						esac
					done
					if [ -n "$notgotit" -o "$2" != "$number" ]
					then
						answer=y
						while [ $answer ]
						do
							$ECHO -n "
Would you like to add $name to the $HOSTS file (y/n) [n] ? "
							read ans
							case $ans in
							[yY]*)
								if [ $binddomain_tmp ]
								then
									$ECHO "$number $name.$binddomain_tmp $name		# BIND server" >> $HOSTSTMP
								else
									$ECHO "$number $name	# BIND server" >> $HOSTSTMP
								fi
								answer=""
								;;
							[nN]*|"")
								answer=""
								;;
							*)
								;;
							esac
						done
					fi
					FORWSERVER=`$ECHO "$FORWSERVER $number"`
					;;
				esac
			done
			;;
		[cC]|"")
			typeserver=c
			$ECHO "
Before configuring your system as a BIND client, you should first be
sure that there IS at least one system on the network configured as
either a BIND primary or secondary server for this domain.  You must
know the host name(s) and Internet address(es) of the specified BIND
server(s) for this domain.  If no server is configured, you will not
be able to access the hosts database.  If you do not know whether a
BIND server exists, contact your site administrator."
			answer=y
			while [ $answer ]
			do
				$ECHO -n "
Enter \"e\" to EXIT or \"c\" to CONTINUE [no default] ? "
				read ans
				case $ans in
				[cC]*)
					answer=""
					;;
				[eE]*)
					eval "$QUIT"
					;;
				*)
					;;
				esac
			done
			#
			# Ask for servers ...
			#
			$ECHO -n "
If you do not want bindsetup to modify the hostname you enter, specify a
fully qualified server name, (ex. foo.bar.dec.com. ) and add a trailing dot.
For example:

                \"foo.bar.dec.com.\"  --> foo.bar.dec.com

Otherwise, if you do not append a trailing dot, (ex. foo.bar) bindsetup
will append the default domain name.  For example:

                \"foo.bar\" --> foo.bar.dec.com

"
			$ECHO "
When finished entering BIND server(s), press the RETURN key only."
			again=y
			first=y
			while [ $again ]
			do
				$ECHO -n "
Enter the host name of a BIND server: "
				read name
				case $name in
				"")
					if [ $first ]
					then
						$ECHO "
At least one BIND server must be entered."
					else
						$ECHO -n "
Finished entering BIND server(s) (y/n) [n] : "
						read ans
						case $ans in
						[yY]*)
							again=""
							;;
						[nN]*|"")
							;;
						*)
							;;
						esac
					fi
					;;
				*)
					first=""
					$ECHO $name > $BINDTMP
					egrep -s "\.$" $BINDTMP
					if [ $? -eq 1 ]
					then
						name=`$ECHO $name | sed 's/$/\.'$binddomain'/'`
					else
						name=`$ECHO $name | sed s/\.$//`
					fi

					binddomain_tmp=$name
					name=`$ECHO $name | sed 's/\..*$//'`
					binddomain_tmp=`$ECHO $binddomain_tmp | sed 's/'$name'//'`
					binddomain_tmp=`$ECHO $binddomain_tmp | sed 's/^\.//'`

					notgotit=y
					set xx `(grep "[ 	]$name" $HOSTS)`
					if [ -n "$2" ]
					then
						number=$2
						notgotit=""
					else
						number=""
					fi
					againn=y
					while [ $againn ]
					do
						againn=""
						$ECHO -n "
Enter the Internet address for $name.$binddomain_tmp [$number]: "
						read ans
						case $ans in
						"")
							if [ -z "$number" ]
							then
								againn="y"
							fi
							;;
						*)
							number=$ans
							;;
						esac
					done
					if [ -n "$notgotit" -o "$2" != "$number" ]
					then
						answer=y
						while [ $answer ]
						do
							$ECHO -n "
Would you like to add $name to the $HOSTS file (y/n) [n] ? "
							read ans
							case $ans in
							[yY]*)
								if [ $binddomain_tmp ]
								then
									$ECHO "$number $name.$binddomain_tmp $name		# BIND server" >> $HOSTSTMP
								else
									$ECHO "$number $name	# BIND server" >> $HOSTSTMP
								fi
								answer=""
								;;
							[nN]*|"")
								answer=""
								;;
							*)
								;;
							esac
						done
					fi
					$ECHO "nameserver	$number" >> $RESTMP
					;;
				esac
			done
			;;
		[eE])
			eval "$QUIT"
			;;
		esac
		;;
	[Rr])
		#
		# Remove BIND Configuration
		#
		$ECHO "
The Remove Option eliminates bind from the rc.config file, 
moves $RESOLVC to $RESOLVC.old, and removes bind from the 
/etc/svc.conf file."
		answer=y
		while [ $answer ]
		do
			$ECHO -n "
Enter \"e\" to EXIT or \"c\" to CONTINUE [no default] ? "
			read ans
			case $ans in
			[cC]*)
				answer=""
				;;
			[eE]*)
				eval "$QUIT"
				;;
			*)
				;;
			esac
		done
		if [ $verbose ]
		then
			$ECHO ""
       			$ECHO "Updating files:"
		fi

		$RCMGR set BIND_CONF NO
		$RCMGR set BIND_SERVERTYPE NOT_CONFIGURED
		$RCMGR set BIND_SERVERARGS NOT_CONFIGURED
		if [ $verbose ]
		then
			$ECHO "  /etc/rc.config"
		fi
		if [ -f $RESOLVC ]
		then
			if [ $verbose ]
			then
				$ECHO "  $RESOLVC"
			fi
			mv $RESOLVC $RESOLVC.old
		fi
		#
		# Must edit /etc/svc.conf
		#
		grep hosts= $SVC | grep bind >> /dev/null
		if [ $? -eq 0 ]
		then
			$SVCSETUP -auto-bind-remove 
			if [ $verbose ]
			then
				$ECHO "  $SVC"
			fi 
		fi 
		exit 0
		;;
	[eE])
		eval "$QUIT"
		;;
	esac

fi

#
# PHASE TWO... Update files!!
#
trap "" 1 2 3 15
if [ $verbose ]
then
 	$ECHO ""
       	$ECHO "Updating files:"
fi

#
# If have servers, add to end of $HOSTS
#
if [ -r $HOSTSTMP ]
then
	if [ $verbose ]
	then
       		echo "  $CLIENTROOT$HOSTS"
		PRINTHO=""
	fi
	cat $HOSTSTMP >> $CLIENTROOT$HOSTS
fi

#
# First check to see if hostname has domain in it already, then
# edit $RCCONF and add $binddomain to hostname if it's not there
# already
#
hname=`$RCMGR get HOSTNAME`
$ECHO $hname > $BINDTMP
egrep -s "$binddomain" $BINDTMP
if [ $? -eq 0 ]
then
	hname=`$ECHO $hname | sed s/.$binddomain//`
else
	if [ $verbose ]
	then
		$ECHO "  $CLIENTROOT$RCCONF"
		PRINTRC=""
	fi

#

	if [ $new_binddomain -eq 1 ]
	then
		binddomain_tmp=`expr $hname : '[^.]*\.\(.*\)'`
		hname=`$ECHO $hname | sed s/.$binddomain_tmp//`
	fi
	$RCMGR set HOSTNAME $hname.$binddomain
fi
#
# Check if $hname.$binddomain is in $HOSTS
# If not, add it
#

# branch on "change-domain-name" variable to code that will change the /etc/hosts entry
egrep -s "$hname.$binddomain" $CLIENTROOT$HOSTS
if [ $? -ne 0 ]
then
	if [ -n "$verbose" -a -n "$PRINTHO" ]
	then
		$ECHO "  $CLIENTROOT$HOSTS"
	fi


	if [ $new_binddomain -eq 1 ]
	then

		ed - $CLIENTROOT$HOSTS << ENDING3 >> $NULL
/[[:space:]]$hname.$old_binddomain[[:space:]]/s/$hname.$old_binddomain/$hname.$binddomain/
w
q
ENDING3

	else

		ed - $CLIENTROOT$HOSTS << ENDING >> $NULL
/[[:space:]]$hname[[:space:]]/s/$hname/$hname.$binddomain $hname/
w
q
ENDING

		egrep -s "$hname.$binddomain" $CLIENTROOT$HOSTS
		if [ $? -ne 0 ]
		then
			ed - $CLIENTROOT$HOSTS << ENDING2 >> $NULL
/[[:space:]]$hname$/s/$hname$/$hname.$binddomain $hname/
w
q
ENDING2

		fi
	fi
fi

# Check if the domain name is in $SENDMAILCF.  If not add it and
# set a switch to ensure the restart of sendmail.

egrep -s "^DD$binddomain" $CLIENTROOT$SENDMAILCF
if [ $? -ne 0 ]
then
	if [ -n "$verbose" -a -n "$PRINTSENDM" ]
	then
		$ECHO "  $CLIENTROOT$SENDMAILCF"
		PRINTSENDM=""
	fi

	here=`pwd`
	if [ -n "$CLIENTROOT" ]
	then
		cd $CLIENTROOT
	else
		cd /
	fi


	ed - .$SENDMAILCF << END >> $NULL
/^DD
d
-
a
DD$binddomain
.
w
q
END
	cd $here
	RESTARTMAIL=y
fi

# Check if the machine's unqualified name is in $SENDMAILCF.  If not add it and
# set a switch to ensure the restart of sendmail.

# branch on "change-domain-name" variable to code that will change the DA entry

egrep -s "^DA$hname" $CLIENTROOT$SENDMAILCF
if [ $? -ne 0 ]
then
	if [ -n "$verbose" -a -n "$PRINTSENDM" ]
	then
		$ECHO "  $CLIENTROOT$SENDMAILCF"
		PRINTSENDM=""
	fi

	here=`pwd`
	if [ -n "$CLIENTROOT" ]
	then
		cd $CLIENTROOT
	else
		cd /
	fi


	ed - .$SENDMAILCF << END >> $NULL
/^DA
d
-
a
DA$hname
.
w
q
END
	cd $here
	RESTARTMAIL=y
fi
#
# make the $RESOLVC file with domain entry
#
if [ $verbose ]
then
       	$ECHO "  $CLIENTROOT$RESOLVC"
fi
$ECHO ";
; resolv.conf
;
; Description:  The resolv.conf file lists name-value pairs that provide
;		information to the BIND resolver.
;
; Syntax:	domain  <domainname>
;                      and
;		nameserver  <address>
;
; Caution:  White space entered after the domain name is not ignored; it
;           is interpreted as part of the domain name.
;
; domain <domainname>       local domain name
; nameserver <address>	    Internet address of a name server that the 
;                           resolver should query
;
domain		$binddomain" > $CLIENTROOT$RESOLVC

#
# Declare BIND configured if an error occurs in the below, declare it
# otherwise within QUIT 
#
$RCMGR set BIND_CONF YES

#
# If server type is client, add nameservers to the $RESOLVC file
#
case $typeserver in
[cC])
	cat $RESTMP >> $CLIENTROOT$RESOLVC
	#
	# first_time is null if $BINDSTART_KEY in RCFILE.
	# This means, previously was server, so remove
	# named since not needed for client setup.
	# Also cd to CLIENTROOT because of ed bug that limits
	# the pathname to 64 characters.
	#
	if [ -n "$verbose" -a -n "$PRINTRC" ]
	then
		$ECHO "  $RCCONF"
	fi

	$RCMGR set BIND_SERVERTYPE CLIENT
	$RCMGR set BIND_SERVERARGS NONE

	if [ -z "$first_time" ]
	then
		#
		# Only kill named if not running in silent client
		# mode for a diskless client.
		#
		if [ -n "$verbose" -a -z "$CLIENTROOT" ]
		then
			if [ -f $NAMEDPID ]
			then
			    /sbin/init.d/named stop silent
			fi
		fi
	fi
	;;
esac
#
# Add /usr/etc/named to RCFILE if server type is primary, secondary 
# or slave
#
case $typeserver in
[pPsSaAlL])
	#
	# Add localhost to RESOLVC
	#
	$ECHO "nameserver	127.0.0.1" >> $CLIENTROOT$RESOLVC
	#
	if [ -n "$verbose" -a -n "$PRINTRC" ]
	then
		$ECHO "  $RCCONF"
	fi

	case $typeserver in
	[pP])
		$RCMGR set BIND_SERVERTYPE PRIMARY
		;;
	[sS])
		$RCMGR set BIND_SERVERTYPE SECONDARY
		;;
	[aA])
		$RCMGR set BIND_SERVERTYPE CACHING
		;;
	[lL])
		$RCMGR set BIND_SERVERTYPE SLAVE
		;;
	esac

	$RCMGR set BIND_SERVERARGS "-b $NAMEDBO"

	case $typeserver in
	[pPsS])
		#
		# Extract the reverse network number of this host
		# by arping.
		#
		#  Get IP address of host.
		#
		2>&1 hostaddr=`/sbin/arp "$hname" | sed 's/.*(\(.*\)).*/\1/'`
		case $hostaddr in 
		"") 	$ECHO "Can't find $hname in arp tables."
			eval "$QUIT"
		esac
		set xx `(IFS=.; $ECHO $hostaddr)`
		w=$2; x=$3; y=$4; z=$5
		if [ $w -ge 1 -a $w -le 126 ]
		then
			IPNETREV=$w
		elif [ $w -ge 128 -a $w -le 191 ]
		then
			IPNETREV=$x.$w
		elif [ $w -ge 192 -a $w -le 223 ]
		then
			IPNETREV=$y.$x.$w
		fi
	
		;;
	esac
	#
	# Set up $NAMEDBO
	#
	if [ $verbose ]
	then
        	$ECHO "  $NAMEDBO"
	fi
	case $typeserver in
	[pP])
		$ECHO ";
; named.boot
;
; Description:  The named.boot file is required to boot a BIND name server.
;
; Syntax:   directory	<directory_name>
;	    ;[comment]
;	    primary		<domain>	<file>
;	    secondary		<domain>	[<host>	<host>...]       <file>
;	    cache		<domain>	<file>
;	    slave
;	    forwarders		<host>	[<host>	<host>...]
;
; <directory_name>   location where domain data files are stored
; ;[comment]	     text following the ';' character is ignored
; domain   	     For a secondary or primary line, the name of the BIND
;		     domain for which the server is a secondary or primary
;		     server.  For a cache line, the name of the domain for
;		     which the file, <file>, is a cache.
; host		     For a secondary line, the IP address of a primary or
;		     secondary server distributing the database for domain,
;		     <domain>.  For a forwarders line, the IP address of a host
;		     to which queries should be forwarded.
; file		     For a secondary line, the name of the file in which the
;		     data of domain, <domain>, received from one of the hosts
;		     specified can be dumped.  For a primary line, the file from
;		     which to read the master copy of the domain data.  For a
;		     cache line, the name of the file in which the cache is
;		     stored.
;
directory	$NAMEDDIR
;
primary		$binddomain		$NAMEDHO
primary		$IPNETREV.in-addr.arpa	$NAMEDRE
;" > $NAMEDBO
		if [ -s $RESTMP ]
		then
			for i in `cat $RESTMP`
			do
				if [ $i = "hosts" ]
				then
					continue
				elif [ $i = "rpc" ]
				then
					$ECHO "primary		$i.$binddomain		$i.db" >> $NAMEDBO
				     else
					$ECHO "primary		$i.$binddomain	$i.db" >> $NAMEDBO
				fi
			done
			$ECHO ";" >> $NAMEDBO
		fi
		$ECHO "primary		0.0.127.in-addr.arpa	$NAMEDLO
;
; load the cache data last
cache		.			$NAMEDCA" >> $NAMEDBO
		if [ -s $RESTMP ]
		then
			if [ "$auto" = "y" ]
			then
				#
				# Make BIND host databases from hosts file
				#
				here=`pwd`
				cd $NAMEDDIR
				for i in `cat $RESTMP`
				do
					if [ $verbose ]
					then
       						$ECHO "  $NAMEDDIR/$i.db"
						if [ $i = "hosts" ]
						then
       							$ECHO "  $NAMEDDIR/$i.rev"
						fi
					fi
					$NAMEDBIN/make_$i
				done
				cd $here
			fi
		fi
		
		;;
	[sS])
		$ECHO ";
; named.boot
;
; Description:  The named.boot file is required to boot a BIND name server.
;
; Syntax:   directory	<directory_name>
;	    ;[comment]
;	    primary		<domain>	<file>
;	    secondary		<domain>	[<host>	<host>...]       <file>
;	    cache		<domain>	<file>
;	    slave
;	    forwarders		<host>	[<host>	<host>...]
;
; <directory_name>   location where domain data files are stored
; ;[comment]	     text following the ';' character is ignored
; domain   	     For a secondary or primary line, the name of the BIND
;		     domain for which the server is a secondary or primary
;		     server.  For a cache line, the name of the domain for
;		     which the file, <file>, is a cache.
; host		     For a secondary line, the IP address of a primary or
;		     secondary server distributing the database for domain,
;		     <domain>.  For a forwarders line, the IP address of a host
;		     to which queries should be forwarded.
; file		     For a secondary line, the name of the file in which the
;		     data of domain, <domain>, received from one of the hosts
;		     specified can be dumped.  For a primary line, the file from
;		     which to read the master copy of the domain data.  For a
;		     cache line, the name of the file in which the cache is
;		     stored.
;
directory	$NAMEDDIR
;
secondary	$binddomain		$IPNETPRI	hosts.db
secondary	$IPNETREV.in-addr.arpa	$IPNETPRI	hosts.rev
;" > $NAMEDBO
		for i in $DATABASES
		do
			if [ $i = "hosts" ]
			then
				continue
			elif [ $i = "rpc" ]
			then
				$ECHO "secondary	$i.$binddomain		$IPNETPRI	$i.db" >> $NAMEDBO
			     else
				$ECHO "secondary	$i.$binddomain	$IPNETPRI	$i.db" >> $NAMEDBO
			fi
		done
		$ECHO ";
primary		0.0.127.in-addr.arpa	$NAMEDLO
;
; load the cache data last
cache		.			$NAMEDCA" >> $NAMEDBO
		;;
	[aA])
		$ECHO ";
; named.boot
;
; Description:  The named.boot file is required to boot a BIND name server.
;
; Syntax:   directory	<directory_name>
;	    ;[comment]
;	    primary		<domain>	<file>
;	    secondary		<domain>	[<host>	<host>...]       <file>
;	    cache		<domain>	<file>
;	    slave
;	    forwarders		<host>	[<host>	<host>...]
;
; <directory_name>   location where domain data files are stored
; ;[comment]	     text following the ';' character is ignored
; domain   	     For a secondary or primary line, the name of the BIND
;		     domain for which the server is a secondary or primary
;		     server.  For a cache line, the name of the domain for
;		     which the file, <file>, is a cache.
; host		     For a secondary line, the IP address of a primary or
;		     secondary server distributing the database for domain,
;		     <domain>.  For a forwarders line, the IP address of a host
;		     to which queries should be forwarded.
; file		     For a secondary line, the name of the file in which the
;		     data of domain, <domain>, received from one of the hosts
;		     specified can be dumped.  For a primary line, the file from
;		     which to read the master copy of the domain data.  For a
;		     cache line, the name of the file in which the cache is
;		     stored.
;
directory	$NAMEDDIR
;
primary		0.0.127.in-addr.arpa	$NAMEDLO
;
; load the cache data last
cache		.			$NAMEDCA" > $NAMEDBO
		;;
	[lL])
		$ECHO ";
; named.boot
;
; Description:  The named.boot file is required to boot a BIND name server.
;
; Syntax:   directory	<directory_name>
;	    ;[comment]
;	    primary		<domain>	<file>
;	    secondary		<domain>	[<host>	<host>...]       <file>
;	    cache		<domain>	<file>
;	    slave
;	    forwarders		<host>	[<host>	<host>...]
;
; <directory_name>   location where domain data files are stored
; ;[comment]	     text following the ';' character is ignored
; domain   	     For a secondary or primary line, the name of the BIND
;		     domain for which the server is a secondary or primary
;		     server.  For a cache line, the name of the domain for
;		     which the file, <file>, is a cache.
; host		     For a secondary line, the IP address of a primary or
;		     secondary server distributing the database for domain,
;		     <domain>.  For a forwarders line, the IP address of a host
;		     to which queries should be forwarded.
; file		     For a secondary line, the name of the file in which the
;		     data of domain, <domain>, received from one of the hosts
;		     specified can be dumped.  For a primary line, the file from
;		     which to read the master copy of the domain data.  For a
;		     cache line, the name of the file in which the cache is
;		     stored.
;
directory	$NAMEDDIR
;
primary		0.0.127.in-addr.arpa	$NAMEDLO
;
slave" > $NAMEDBO
		if [ -n "$FORWSERVER" ]
		then
			$ECHO "forwarders $FORWSERVER" >> $NAMEDBO
		fi
		;;
	esac
	#
	# Add NAMEDLO
	#
	if [ $verbose ]
	then
		$ECHO "  $NAMEDDIR/$NAMEDLO"
	fi
	$ECHO ";
; BIND data file for local loopback interface. 
;
@	IN	SOA	$hname.$binddomain. postmaster.$hname.$binddomain. (
			1	; Serial
			3600	; Refresh
			300	; Retry
			3600000	; Expire
			3600 )	; Minimum
	IN	NS	$hname.$binddomain.
1	IN	PTR	localhost.
localhost.	IN	A	127.0.0.1" > $NAMEDDIR/$NAMEDLO
	#
	# Add NAMEDCA
	#
	case $typeserver in
	[pPsSaA])
		if [ $verbose ]
		then
			$ECHO "  $NAMEDDIR/$NAMEDCA"
		fi
		$ECHO ";
; BIND data file for initial cache data for root domain servers.
;
.		99999999	IN	NS	    ns.nic.ddn.mil.
.		99999999	IN	NS	    ns.nasa.gov.
.		99999999	IN	NS	    terp.umd.edu.
.		99999999	IN	NS	    a.isi.edu.
.		99999999	IN	NS	    aos.brl.mil.
.		99999999	IN	NS	    gunter-adam.af.mil.
.		99999999	IN	NS	    c.nyser.net.
ns.nic.ddn.mil.	99999999	IN	A	    192.67.67.53
ns.nasa.gov.	99999999	IN	A	    128.102.16.10	; BIND
		99999999	IN	A	    192.52.195.10
a.isi.edu.	99999999	IN	A	    26.3.0.103		; Jeeves
		99999999	IN	A	    128.9.0.107
aos.brl.mil.	99999999	IN	A	    128.20.1.2		; BIND
		99999999	IN	A	    192.5.25.82
gunter-adam.af.mil. 99999999	IN	A	    26.1.0.13		; Jeeves
c.nyser.net.	99999999	IN	A	    192.33.4.12		; BIND
terp.umd.edu.	99999999	IN	A	    128.8.10.90		; BIND" > $NAMEDDIR/$NAMEDCA
		;;
	esac
esac

#
#
# Only set /bin/hostname if no CLIENTROOT set.
#
if [ -z "$CLIENTROOT" ]
then
	if [ $verbose ]
	then
		$ECHO -n "
Setting hostname to: "
		$HOSTNAME $hname.$binddomain
	else
		$HOSTNAME $hname.$binddomain >> $NULL
	fi
fi

#
# restart sendmail with either a new domain name or a new unqualified
# name. 
#
if [ $RESTARTMAIL ]
then
	if [ $verbose ]
	then
		$ECHO -n "
"
		$SENDMAIL restart
	else
		$SENDMAIL restart >> $NULL
	fi
fi

case $typeserver in
[pPsSaAlL])
	if [ "$auto" = "y" ]
	then
		$ECHO ""
		answer=y
		while [ $answer ]
		do
			$ECHO -n "Would you like bindsetup to start the BIND daemon automatically [y]? "
			answer=""
			read ans
			case $ans in
			[yY]*|"")
				/sbin/init.d/named stop silenterror
				/sbin/init.d/named start 
				;;
			[nN]*)
				;;
			*)
				answer=y
				;;
			esac
		done
	fi
esac

if [ $verbose ]
then
	$ECHO "
To resolve hostname queries, your system can use the local /etc/hosts
file, BIND or NIS.  The "hosts" line of the /etc/svc.conf file maintains 
a list of these services in the order in which they should be queried for 
host information.  

It is recommended that your system check the local /etc/hosts file first 
before querying BIND for host information.
"
	done=
        while test -z "$done"
        do

                $ECHO "	Service Order Selection Menu

	1) Query /etc/hosts before querying BIND for host information.

	2) Query BIND first for host information.
	
	3) Run svcsetup to customize service order selection."
		$ECHO
                $ECHO -n "Enter your choice [1]: "
                read selection
		case $selection in
		1|"") done=done
		      $SVCSETUP -auto-bind-2 
		      $ECHO "Updating files:
 $SVC " 
		      ;;	
		2) done=done
		   $SVCSETUP -auto-bind-1 
		   $ECHO "Updating files:
 $SVC " 
		   ;;
		3) done=done
		   $SVCSETUP
		   ;;
		esac
	done
fi
if [ ! -f $CLIENTROOT$SVCORDER ]
then
	$ECHO "#
#       The /etc/svcorder file designates the order and selection of
#       ULTRIX name services to be queried in the resolution of host
#       names and addresses.  This file is not required for /etc/hosts
#       (local) access, but is required to access host names from
#       database lookup services such as Yellow Pages and BIND.
#
#       Note that additional preparation is required to set up each
#       service.  Consult the documentation for further information.
#
#

local
bind" > $CLIENTROOT$SVCORDER
fi

#
# Clean up
#
if [ -r $RESTMP ]
then
	rm $RESTMP
fi
if [ -r $HOSTSTMP ]
then
	rm $HOSTSTMP
fi
if [ -r $BINDTMP ]
then
	rm $BINDTMP
fi
if [ $verbose ]
then
	$ECHO ""
	$ECHO "***** BINDSETUP COMPLETE *****"
fi
exit 0

