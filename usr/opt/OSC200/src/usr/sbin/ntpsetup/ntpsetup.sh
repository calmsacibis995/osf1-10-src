#! /bin/sh
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
#       @(#)ntpsetup.sh	3.5     (ULTRIX/OSF)        8/20/91
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
# Purpose:	Set up Network Time Protocol (NTP) service
# Usage:	ntpsetup
# Environment:	Bourne shell script
# 

#
# Set up interrupt handlers:
#
QUIT='
    if [ -r "$NTPTMP" ]
    then
    	rm "$NTPTMP"
    fi
    if [ -r "$NTPTMP2" ]
    then
    	rm "$NTPTMP2"
    fi
    $ECHO "

Ntpsetup terminated without setting up the Network Time Protocol service."
    exit 1
'

#
# Trap ^c signal, etc.
#
trap 'eval "$QUIT"' 1 2 3 15

# files
NTPTMP=/tmp/ntpsetup.conf
NTPTMP2=/tmp/ntpsetup2.conf
NTPCONF=/etc/ntp.conf
RCCONF=/etc/rc.config
HOSTS=/etc/hosts
XNTPD=/sbin/init.d/xntpd
SETTIME=/sbin/init.d/settime

#commands
ECHO=/bin/echo
RCMGR=/usr/sbin/rcmgr
PING=/usr/sbin/ping 

# defines
PATH=$PATH:/usr/sbin:/usr/bin
export PATH
xntpd_opts=""
first_time="y"
xntp_serv1=""
xntp_serv2=""
xntp_serv3=""

 $ECHO "#
#  XNTPD Configuration File (template)
#
#
# Specify a filename for the "driftfile" created by xntpd.
# /etc/ntp.drift is the default.
# 
driftfile /etc/ntp.drift
#
#
#
#
# Specify several NTP servers as peers (See the xntpd documentation
# for recommendations on selecting peers).
# NOTE: Be sure to specify "version 1" for servers running the ntpd
#       daemon.  For example, if server1 runs ntpd and server2 runs
#       xntpd, the two corresponding entries would be:
#
#		peer server1 version 1      # ntpd server
#		peer server2		    # xntpd server
#
#
#
#
#
#
# For further information on configuration options, see the xntpd
# documentation.  If you have a local accurate clock (radio clock, etc),
# you will need to specify further configuration options.
#" > $NTPTMP

 $ECHO "
********************************************************************
*								   *
*		Network Time Protocol (NTP) Setup		   *
*								   *
********************************************************************

"
#
# PHASE ONE: Gather data!
#

    #
    # Require it to be run by root
    #

    if [ \! -w "$RCCONF" ]
    then
    	$ECHO "Please su to root."
  	eval "$QUIT"
    fi

    #
    # Be sure the network has already been set up, 	
    # and this host has a name!
    #

    hname=`hostname`
    if [ $? -ne 0 ]
    then
    	$ECHO "
Please bring the system to multi-user mode before running ntpsetup."
	eval "$QUIT"
    fi

    #
    #
    # See if this is a re-install
    #
    
    prev_conf=`$RCMGR get XNTPD_CONF`
    if [ "$prev_conf" = "YES" ]
    then
    	$ECHO "
NTP has already been installed on this host."
	query="y"
        while [ -n "$query" ]
        do
       	   query=""
	   $ECHO -n "
Would you like to change the current NTP configuration (y/n) [no default]? "
	   read answer
	   case "$answer" in
	   [yY]*)
   	       first_time=""
	       ;;
	   [nN]*)
	       eval "$QUIT"
	       ;;
	   *)
	       query="y"
	       ;;
	   esac
       done
    fi

    if [ -n "$first_time" ]
    then
	$ECHO "

Ntpsetup configures and runs the Network Time Protocol service (NTP)
for your system.  NTP is a distributed time service which provides 
accurate, synchronized time to each host in a hierarchical network 
of systems.  Ntpsetup runs the University of Toronto's xntpd daemon.

Each system in an NTP hierarchy must either have a local reference clock
or be served by other NTP hosts (local or remote).  Ntpsetup sets up
your host to be served by other NTP hosts.  If you have a local 
reference clock, please see the documentation for the setup procedure.

NOTE:  NTP will never jump your system's time backward; rather, if
       necessary, NTP will gradually slew your system's time backward 
       to prevent time-related system errors.


[ Press the RETURN key to continue ] : "
		read junk
   fi
   $ECHO "
Default answers are shown in square brackets ([]).  To use a default 
answer, press RETURN."

    redo_servers="y"
    while [ -n "$redo_servers" ]
    do
	if [ -r "$NTPTMP" ]
	then
	    rm "$NTPTMP"
	    $ECHO "#
#  XNTPD Configuration File (template)
#
#
# Specify a filename for the "driftfile" created by xntpd.
# /etc/ntp.drift is the default.
# 
driftfile /etc/ntp.drift
#
#
#
#
# Specify several NTP servers as peers (See the xntpd documentation
# for recommendations on selecting peers).
# NOTE: Be sure to specify "version 1" for servers running the ntpd
#       daemon.  For example, if server1 runs ntpd and server2 runs
#       xntpd, the two corresponding entries would be:
#
#		peer server1 version 1      # ntpd server
#		peer server2		    # xntpd server
#
#
#
# For further information on configuration options, see the xntpd
# documentation.  If you have a local accurate clock (radio clock, etc),
# you will need to specify further configuration options.
#" > "$NTPTMP"

	fi
	if [ -r "$NTPTMP2" ]
	then
	    rm "$NTPTMP2"
	fi
	$ECHO "
NTP Server Selection
********************
Enter the names of the NTP servers for this system.
Press RETURN to terminate the list."
	query="y"
	count=0
	while [ -n "$query" ]
	do
	    count=`expr $count + 1`
	    $ECHO -n "
Hostname of NTP server [no default]: "
	    read answer

	    case "$count" in
	    1)
		if [ -n "$answer" ]
		then
		   xntp_serv1="$answer"
		fi
		;;
	    2)
		if [ -n "$answer" ]
		then
		   xntp_serv2="$answer"
		fi
		;;
	    3)
		if [ -n "$answer" ]
		then
		   xntp_serv3="$answer"
		fi
		;;
	    *)
		;;
	    esac

	    if [ -n "$answer" ]
	    then
		$ECHO -n "        Looking up host "$answer" ..."
		#
		# Use the 'ping' command to determine whether the address of this
	        # host can be determined by "gethostbyname".  If the host address
		# can not be determined, ask the user whether s/he wants to add
	        # an entry to /etc/hosts for this host. 
		#
		good=`$PING -c 1 $answer 2>&1 | grep "unknown host"`
			
		if [ -z "$good" ]
		then
		    short=`echo $hname | sed 's/\..*//'`
		    if [ "$answer" != "$hname" ] && [ "$answer" != "$short" ]
		    then
			vers="y"
			while [ -n "$vers" ]
		    	do
			    $ECHO "found."
			    $ECHO -n "        Is "$answer" running ntpd or xntpd (n/x) [x] ? "		
			    read daem
			    case "$daem" in
				[nN]*)
				    vers=""
				    $ECHO "peer $answer version 1" >> $NTPTMP
				    $ECHO $answer \(ntpd\) >> $NTPTMP2
				    ;;
				[xX]*|"")		
				    vers=""
				    $ECHO peer "$answer" >> $NTPTMP	
				    $ECHO "$answer" \(xntpd\) >> $NTPTMP2
				    ;;
				*)
				    ;;
			      esac
    	    	    	   done
		        else
    	    	    	   $ECHO "        $answer can not serve itself.
"
			   case "$count" in
			   1)
				xntp_serv1=""
				;;
			   2)
				xntp_serv2=""
				;;
			   3)
				xntp_serv3=""
				;;
			   *)
				;;
			   esac
			   count=`expr $count - 1`
			    
		        fi
		     else
		        $ECHO "
             Cannot find an address for \"$answer\"."
		 	queryhosts="yes"
			$ECHO "             To add \"$answer\" to the $HOSTS file, you must know 
             \"$answer\"'s internet (IP) address."
			while [ -n "$queryhosts" ]
			do
			 	queryhosts=""
			      	$ECHO -n "             
             Would you like to add \"$answer\" to the $HOSTS file (y/n) [y] ? "
			      	read answerhosts
			      	case "$answerhosts" in
			      	[yY]*|"")
				  	ip="yes"
				   	while [ -n "$ip" ]
				   	do
						ip=""
					     	$ECHO -n "             What is $answer's internet (IP) address [no default] ? "
					     	read ipaddress
					     	case "$ipaddress" in
					     	"")
						     	ip="yes"
						       	;;
					     	*)
						        chk="yes"
						        while [ -n "$chk" ]
						        do
						       	    chk=""
						       	    $ECHO -n "             Is $ipaddress correct (y/n) [no default] ? "
						       	    read ipchk
						       	    case "$ipchk" in
					 	       	    [yY]*)
							    	 $ECHO "$ipaddress \t $answer " >> $HOSTS
								vers="y"
								while [ -n "$vers" ]
		    						do
			    						$ECHO -n "
       Is "$answer" running ntpd or xntpd (n/x) [x] ? "		
			    						read daem
			    						case "$daem" in
									[nN]*)
				    						vers=""
				    						$ECHO "peer $answer version 1" >> $NTPTMP
				    						$ECHO $answer \(ntpd\) >> $NTPTMP2		
				    						;;
									[xX]*|"")		
				    						vers=""
				    						$ECHO peer "$answer" >> $NTPTMP	
				    						$ECHO "$answer" \(xntpd\) >> $NTPTMP2
				    						;;
									*)
				    						;;
			      						esac
    	    	    	   					done 
							    	 ;;
						       	    [nN]*)
							    	 ip="yes"
							    	 ;;
						       	    *)
							    	 chk="yes"
							    	 ;;
				  		       	    esac
						       done
						       ;;
					     	  esac
				   	     done
				   	     ;;
			      	   	[nN]*)
						case "$count" in
			   			1)
							xntp_serv1=""
							;;
			   			2)
							xntp_serv2=""
							;;
			   			3)
							xntp_serv3=""
							;;
			   			*)
							;;
						esac
			   			count=`expr $count - 1`
				   	     	;;
			      	   	*)
						queryhosts="yes"
				   	     	;;
			      	   	esac
			      	   done
			       fi
	         else
    	    	     if [ "$count" -eq 1 ]
		     then 
			$ECHO "
You must specify at least one NTP server."
		 	count=`expr $count - 1`	
		     else
			query=""
		        $ECHO  "
The list of NTP servers is:"
		        awk '
			   {
			   printf "\t%s\n", $0 
		    	   }' $NTPTMP2
		        query2="y"
		        while [ -n "$query2" ]
		        do
			   query2=""
			   $ECHO -n "
Enter \"r\" to REDO the servers list, \"e\" to EXIT the ntpsetup
procedure,  or \"c\" to CONTINUE [no default]: "
			   read answer
			   case "$answer" in
			   [cC]*)
			      redo_servers=""
			      ;;
			   [rR]*)
			      xntp_serv1=""
			      xntp_serv2=""
			      xntp_serv3=""
			      ;;
			   [eE]*)
			      eval "$QUIT"
			      ;;
		    	   *)
			      query2="y"
			      ;;
			   esac
		        done
			
    	    	     fi
    	          fi
    	       done
            done
	    $ECHO "

IMPORTANT NOTE:  If any of the NTP servers you specified are 
		 NOT IN YOUR SUBNET, you must run either "routed"
		 or "gated" in order to access them.  Please see 
		 the networking documentation to set up one of 
		 these services.

[ Press RETURN to continue ] : "
     	    read junk
            query="y"
            $ECHO "
		   Xntpd (the NTP daemon) Options
		   ******************************

NOTE: The default answers are RECOMMENDED.

		Correct large time differences, -g
		----------------------------------

    If your system time differs from the network time by more
    than 1000 seconds while xntpd is running, the daemon will 
    suspect that something is very wrong with your system which 
    may be a security threat.  It will then log a message to the 
    syslog and exit to allow the system manager to resolve the 
    problem.

    The -g option allows xntpd to correct large time differences
    without logging a message or exiting.  It should be used by
    systems which are less sensitive to security threats. "
	    while [ -n "$query" ]
            do
	       query=""
	       $ECHO -n "
    Would you like to use the -g option (y/n) [y]? "
	       read answer
	       case "$answer" in
	       [yY]*|"")	
	          xntpd_opts="-g"
	          ;;
	       [nN]*)
    	          ;;
	       *)
	          query="y"
	          ;;
	       esac
            done
	
            query="y"
            $ECHO "
		Limit syslog messages, -l
		-------------------------
    NTP uses syslog to record status messages once an hour plus
    several other informative messages.  NTP also logs error 
    messages and an initialization message."
            while [ -n "$query" ]
            do
    	       query=""
	       $ECHO -n "
    Would you like to limit NTP to log ONLY error messages 
    and the initialization message (y/n) [n]? "
	       read answer
	       case "$answer" in
	       [yY]*)	
	          xntpd_opts="$xntpd_opts -l"
	          ;;
	       [nN]*|"")
	          ;;
	       *)
	          query="y"
	          ;;
	       esac
            done

            $ECHO -n "

Configuring your system to run NTP..."
	    if [ "$prev_conf" = "YES" ]
    	    then
               "$XNTPD" stop
    	    fi

            if [ -r "$NTPCONF" ]
            then
	       mv "$NTPCONF" "$NTPCONF.sav"
            fi

            cat "$NTPTMP" >> "$NTPCONF"
            rm "$NTPTMP"
            rm "$NTPTMP2"

            $RCMGR set XNTPD_CONF "YES"
     	    $RCMGR set XNTP_SERV1 "$xntp_serv1"
     	    $RCMGR set XNTP_SERV2 "$xntp_serv2"
     	    $RCMGR set XNTP_SERV3 "$xntp_serv3"
     	    $RCMGR set XNTPD_OPTS "$xntpd_opts"
	    $ECHO "done."

    	    $ECHO "
Starting the NTP daemon (xntpd)..."

    	    "$SETTIME" 
    	    "$XNTPD" start
	   
	    $ECHO "
To monitor NTP, type \"/usr/bin/ntpq -p\".
"
    	    $ECHO "******  NTPSETUP Complete ******"
	
