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
# @(#)$RCSfile: ypsetup.sh,v $ $Revision: 4.3.9.3 $ (DEC) $Date: 1993/09/29 13:46:01 $ 
#
# Purpose:	Set up Network Information Service (NIS, formerly YP) 
# Usage:	nissetup or ypsetup 
# Environment:	Bourne shell script
# 
# Remarks:
#    For NIS clients, sets up files:
#	/etc/rc.config
#    For NIS servers (master or slave), sets up files:
#	/etc/rc.config
#       /var/spool/cron/crontabs/root (slave only)
#	/var/yp/{domainname}/*
#
					
#
# Set up interrupt handlers:
#
      	  
QUIT='
     if [ -r "$YPTMP" ]
     then
     	  rm "$YPTMP"
     fi
     if [ -r "$CRTMP" ]
     then
     	  rm "$CRTMP"
     fi
     $ECHO "
Nissetup (ypsetup) terminated without configuring the Network 
Information Service.
"
     exit 1
'

QUIT_AFTER='
     if [ -r "$YPTMP" ]
     then
          rm "$YPTMP"
     fi
     if [ -r "$CRTMP" ]
     then
          rm "$CRTMP"
     fi
     $ECHO "
Nissetup (ypsetup) terminated after configuring the Network
Information Service.

***** NISSETUP COMPLETE *****
"
       exit 1
'

NOCHANGE='
	if [ -r "$YPTMP" ]
     then
          rm "$YPTMP"
     fi
     if [ -r "$CRTMP" ]
     then
          rm "$CRTMP"
     fi
     $ECHO "
***** NISSETUP COMPLETE *****
"
     exit 1
'

QMSG='
     rm -rf "${YPDIR}/${ypdomain}"
     if [ $? -ne 0 ]
     then
	  $ECHO "
Please clean up ${YPDIR}/${ypdomain}."
     fi
     eval "$QUIT"
'

#
# Trap ^c signal, etc.
#
trap 'eval "$QUIT"' 1 2 3 15

# files
YPTMP=/tmp/ypsetup.yp.$$
YPTMP3=/tmp/ypsetup.yp3.$$
CRTMP=/tmp/ypsetup.cr.$$
HOSTS=/etc/hosts
YPDIR=/var/yp
YPLOG=ypxfr.startup.log
RCCONF=/etc/rc.config
CRFILE=/var/spool/cron/crontabs/root
SVCSETUP=/usr/sbin/svcsetup
SVC=/etc/svc.conf

# commands
MAKE="make -f /var/yp/Makefile"
MAKEDBM=/var/yp/makedbm
YPXFR=/var/yp/ypxfr
ECHO=/bin/echo
RCMGR=/usr/sbin/rcmgr
DOMAINNAME=/usr/bin/domainname
PORTMAP=/usr/sbin/portmap
YPBIND=/usr/sbin/ypbind
YPSERV=/usr/sbin/ypserv
YPXFRD=/usr/sbin/ypxfrd
NIS=/sbin/init.d/nis

# defines
PATH=$PATH:/usr/sbin:$YPDIR
export PATH
DEFMAPS="group.bygid group.byname hosts.byaddr hosts.byname \
mail.aliases netgroup netgroup.byuser netgroup.byhost networks.byaddr \
networks.byname passwd.byname passwd.byuid protocols.byname \
protocols.bynumber services.byname ypservers"

ypdomain=""
flavor=""
first_time=y
scratch_maps=y
ypbind_args=""
useforall="" 
#
# PHASE ONE: Gather data!
#
	
#
# Set default answers from previous config.
#
prev_type=`$RCMGR get NIS_TYPE`
case "$prev_type" in
"MASTER")
     def_type=1
     ;;
"SLAVE")
     def_type=2
     ;;
*)
     def_type=3
     ;;
esac

prev_domain=`$RCMGR get NIS_DOMAIN`
prev_args=`$RCMGR get NIS_ARGS`
prev_pwd=`$RCMGR get NIS_PASSWDD`
prev_conf=`$RCMGR get NIS_CONF`

#
# Require nissetup to be run by root
#
if [ \! -w "$RCCONF" ]
then
     $ECHO "
Please su to root."
     eval "$QUIT"
fi

#
# Be sure network has already been set up, and this host has a name!
#
	
hname=`hostname`
if [ $? -ne 0 ] 
then
     $ECHO "
Please set up the network before running nissetup."
     eval "$QUIT"
fi

$ECHO "
********************************************************************
*                                                                  *
*               Network Information Service (NIS) Setup            *
*                                                                  *
********************************************************************"

$ECHO "
NOTE:  Please be sure the network is set up and running before you 
       set up NIS.  

       Also, if you are going to set up an NIS server, be sure you
       have installed the \"Additional Networking Services\" subset."

query="y"
while [ -n "$query" ]
do
      query=""
      $ECHO -n "
Enter \"e\" to EXIT or \"c\" to CONTINUE [no default]: "
      read answer
      case "$answer" in
      [eE]*)
         eval "$QUIT"
         ;;
      [cC]*)
         ;;
      *)
         query="y"
         ;;
      esac
done
#
#
# See if this is a re-install
#

if [ "$prev_conf" = "YES" ]
then
     $ECHO "

       ---------------------------
      	Current NIS Configuration 
       ---------------------------

   	domainname     : $prev_domain
   	type           : $prev_type
   	ypbind options : $prev_args"

     if [ "$prev_type" = "MASTER" ]
     then
	if [ "$prev_pwd" = "YES" ]
	then
	   $ECHO "   	yppasswdd      : yes"
        else
	   $ECHO "   	yppasswdd      : no"
	fi
     fi
      
     query="y"
     while [ -n "$query" ]
     do
      	  query=""
	  $ECHO -n "
Would you like to change the current NIS configuration (y/n) [no default]? "
	  read answer
	  case "$answer" in
	  [yY]*)
   	       first_time=""
	       ;;
	  [nN]*)
	       eval "$NOCHANGE"
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
Nissetup (formerly ypsetup) configures and runs the Network 
Information Service (NIS, formerly YP) for your system. 

NIS provides a distributed data lookup service for sharing data 
among networked systems.  NIS data is stored in database files 
called "maps".  A "domain" is a group of systems which share a 
common set of maps. 

[ Press RETURN to continue ] : "
     read junk
     $ECHO "
For each domain, there are three types of systems:

	- a MASTER SERVER maintains the master copy of the 
	  domain's maps. 
	  There should be ONLY ONE master server per domain.

	- a SLAVE SERVER periodically receives updated versions 
	  of the domain's maps from the master.  A slave server 
     	  can retrieve data from its private collection of maps, 
	  and can take over for the master server if the master
	  server fails.

	- a CLIENT retrieves NIS data by requesting service from 
	  an NIS server.  A client does not have local copies of 
	  the domain's maps.

NOTE: To set up your system as a MASTER or SLAVE SERVER, you must have
      installed the \"Additional Networking Services\" subset.

[ Press RETURN to continue ] : "
     read junk
fi
   

#
# Ask for a name...
#
query_domain="y"
while [ -n "$query_domain" ]
do
     query_domain=""
     $ECHO -n "

Default answers are shown in square brackets ([]).
Press RETURN to use a default answer.

What is $hname's NIS domain name ["$prev_domain"] ? "
     read answer
     case "$answer" in
     "")
	  if [ -n "$prev_domain" ]
	  then
	       ypdomain="$prev_domain"
	  else
	       query_domain="y"
	  fi
	  ;;
     *)
	  ypdomain="$answer"
	  ;;
     esac
       	
     if [ -n "$ypdomain" ]
     then   
       	  query="y"
       	  while [ -n "$query" ]
       	  do
	       query=""
       	       $ECHO -n "
Is "$ypdomain" the correct domain name (y/n) [no default]? "
       	       read answer
       	       case "$answer" in
       	       [yY]*)
       	       	    ;;
       	       [nN]*)
       	       	    query_domain="y"
       	       	    ;;
       	       *)
	       	    query="y"
       	       	    ;;
       	       esac
       	  done
     fi
done

$ECHO "
Will $hname be a

     	      1. MASTER server,
     	      2. SLAVE server, or
	      3. CLIENT ? "
query="y"
while [ -n "$query" ]
do
     query=""
     $ECHO -n "
1 2 or 3 ["$def_type"] ? "
     read answer
     
     if [ -z "$answer" ]
     then
	  answer="$def_type"
     fi

     if [ "$answer" != "2" ] && [ "$def_type" = "2" ]
     then
	  $ECHO "
NOTE: Remember to remove the NIS SLAVE server entries from the crontab
	file "$CRFILE" after completing nissetup. "
     fi
 	
     if [ $answer -eq 1 ] || [ $answer -eq 2 ]
     then
	  if [ \! -d "$YPDIR" ]
	  then
		$ECHO "
Please install the \"Additional Networking Services\" subset before setting up an NIS server. "
                eval "$QUIT"
          fi
     fi	

     case "$answer" in
     [1]*)
     	  $ECHO "

			MASTER Server Setup

There can be ONLY ONE master server for each domain.  If a master
NIS server is already configured for domain ${ypdomain} please exit 
nissetup now."
	  query="y"
	  while [ -n "$query" ]
	  do
	       query=""
	       $ECHO -n "
Enter \"e\" to EXIT or \"c\" to CONTINUE [no default]: "
	       read answer
	       case "$answer" in
	       [eE]*)
	       	    eval "$QUIT"
	       	    ;;
	       [cC]*)
	       	    ;;
	       *)
	       	    query="y"
	       	    ;;
	       esac
	  done
	  $ECHO "
As a MASTER NIS server, you may run the yppasswdd daemon to 
allow remote password updates to the master copy of the passwd file. "
	  query="y"
	  while [ -n "$query" ]
	  do
	       query=""
	       $ECHO -n "
Would you like to run the yppasswdd daemon [y]? "
	       read answer
	       case "$answer" in
	       [yY]*|"")
	            yppasswdd="YES"
	       	    ;;
	       [nN]*)
	       	    yppasswdd="NO"
	       	    ;;
	       *)
	       	    query="y"
	       	    ;;
	       esac
	  done
          if [ -d "$YPDIR/$ypdomain" ]
	  then
	       $ECHO "
*** The NIS maps have already been initialized for the domain
\"${ypdomain}\".  If you are changing your current NIS configuration 
from a slave server to a master server, the maps MUST be remade.  
If you choose to remake the maps, be sure that the NIS database
files are in the directory /var/yp/src."
	       query="y"
	       while [ -n "$query" ]
	       do
	       	    query=""
	       	    $ECHO -n "
Would you like to remake the NIS maps (y/n) [no default]? "
	       	    read answer
	       	    case "$answer" in
	       	    [yY]*)
	            	 scratch_maps="y"
	            	 ;;
	       	    [nN]*)
	            	 scratch_maps=""
	            	 ;;
	       	    *)
	            	 query="y"
	            	 ;;
	       	    esac
	       done
       	  fi
	
          if [ -n "$scratch_maps" ]
          then
               $ECHO "
As a MASTER NIS server, this system maintains a list of slave
NIS servers in the ${ypdomain} domain.  This list is used to 
update the slave servers whenever a map is modified.

Enter the names of the SLAVE servers in the ${ypdomain} domain.  
Press RETURN to terminate the list.
"
               not_done="y"
	       while [ -n "$not_done" ]
	       do
	       	    query="y"
	            #
	       	    # Enter this host first...
	       	    #
	       	    $ECHO "$hname" > $YPTMP
	       	    while [ -n "$query" ]
     	       	    do
	            	 $ECHO -n "	Hostname of slave server: "
	            	 read servname
	            	 if [ -n "$servname" ]
	            	 then
	             	      #
		     	      # Search /etc/hosts for hostname,
		     	      #
			      sed "s/#.*//" $HOSTS > $HOSTS.tmp
		     	      good=`egrep "[ 	]$servname([ 	\.]|$)" $HOSTS.tmp`
		     	      if [ -n "$good" ]
		     	      then
				   short=`echo $hname | sed 's/\..*//'`
		              	   if [ "$servname" = "$hname" ] || [ "$servname" = "$short" ]
		              	   then
		              	   	$ECHO "This host can not be a slave server."
			      	   	$ECHO ""
		              	   else 
			      	   	$ECHO $servname >> $YPTMP
		              	   fi
		     	      else
		       	      	   $ECHO "            Cannot find $servname in the file $HOSTS."
			      	   queryhosts="yes"
			      	   $ECHO "            To add $servname to the $HOSTS file you MUST 
                 know $servname's internet (IP) address."
				   while [ -n "$queryhosts" ]
			      	   do
			      	   	queryhosts=""
			      	   	$ECHO -n "
            Would you like to add $servname to the $HOSTS file (y/n) [y]? "
			      	   	read answerhosts
			      	   	case "$answerhosts" in
			      	   	[yY]*|"")
				   	     ip="yes"
				   	     while [ -n "$ip" ]
				   	     do
					     	  ip=""
					     	  $ECHO -n "            What is $servname's internet (IP) address [no default] ? "
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
						       	    $ECHO -n "            Is $ipaddress correct (y/n) [no default] ? "
						       	    read ipchk
						       	    case "$ipchk" in
					 	       	    [yY]*)
							    	 $ECHO "$ipaddress \t $servname " >> $HOSTS
								 $ECHO $servname >> $YPTMP 
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
				   	     ;;
			      	   	*)
				   	     queryhosts="yes"
				   	     ;;
			      	   	esac
			      	   done
			      	   $ECHO ""
		     	      fi
	            	 else
		     	      query=""
	            	 fi
     	       	    done
	
               	    #
	       	    # Ask for verification...
	       	    #
	       	    $ECHO "
The list of NIS servers (including the master) for domain ${ypdomain} is:"
	       	    awk '{ printf "\t%s\n", $0 }' $YPTMP
	       	    $ECHO "
You may now redo this list, exit nissetup, or continue.

If you choose to continue, the default set of NIS maps for your host
will now be created.  Please be sure that the files that you wish 
to distribute are in the /var/yp/src directory."
	       	    query="y"
	       	    while [ -n "$query" ]
	       	    do
	            	 query=""
	            	 $ECHO -n "
Enter \"r\" to REDO the servers list, \"e\" to EXIT nissetup,  
or \"c\" to CONTINUE [no default]: "
	            	 read answer
	            	 case "$answer" in
		    	 [cC]*)
		     	      not_done=""
		     	      ;;
		    	 [rR]*)
		     	      ;;
		    	 [eE]*)
		     	      eval "$QUIT"
		     	      ;;
		    	 *)
			      query="y"
		     	      ;;
		    	 esac
	       	    done
	       done
			
	       trap 'eval "$QMSG"' 1 2 3 15

	       #
	       # Create NIS Domain
	       #
	       if [ -n "$scratch_maps" ]
	       then
	       	    rm -rf "$YPDIR/$ypdomain"
	       fi
	       mkdir $YPDIR/$ypdomain
	       if [ $? -ne 0 ]
	       then
	       	    $ECHO "
Couldn't create the NIS directory: ${YPDIR}/${ypdomain}."
	       	    eval "$QUIT"
	       fi
	 
	       #
	       # Make default maps...
	       #
	       $ECHO "
Creating default NIS maps.  Please wait..."
	       cat $YPTMP | awk '{print $$0, $$0}' \
	       	    | $MAKEDBM - $YPDIR/$ypdomain/ypservers
	       if [ $? -ne 0 ]
	       then
	       	    $ECHO "
Couldn't build the ypservers database."
	       	    eval "$QMSG"
	       fi
	
	       sed "s/^DOM[ 	]*=.*$/DOM=$ypdomain/" /var/yp/Makefile \
               	    > /var/yp/Makefile.tmp
	       mv /var/yp/Makefile.tmp /var/yp/Makefile

	       (cd $YPDIR/$ypdomain; $MAKE NOPUSH=1)
	       if [ $? -ne 0 ]
	       then
	       	    $ECHO "
Couldn't make the default NIS maps"
	       	    eval "$QMSG"
	       fi
	       $ECHO "Finished creating default NIS maps."
	  fi
          flavor="MASTER"
          ;;
     [2]*)
	  $ECHO "

			SLAVE Server Setup
			
As a slave NIS server, your system will receive periodic map updates 
from the master server.  

NOTE:  If you did not list this system as a slave server when you 
       set up the NIS master, you must add this system to the 
       master's list of slave servers by following the instructions
       in the NIS documentation.  You can do this AFTER you complete 
       nissetup."

	  if [ -d "$YPDIR/$ypdomain" ]
	  then
	       $ECHO "
*** The NIS maps have already been initialized for the domain \"${ypdomain}\".
If you are changing your current NIS configuration from a master server 
or a client to a slave server, the NIS maps must be recopied from the
master server. 

Would you like nissetup to copy the current maps from the master server? "
	       query="y"
	       while [ -n "$query" ]
	       do
	       	    query=""
	       	    $ECHO -n "
Enter \"y\", \"n\", or \"e\" to EXIT nissetup [no default]: "
	       	    read answer
	       	    case "$answer" in
	       	    [yY]*)
	            	 scratch_maps="y"
		    	 ;;
	       	    [eE]*)
		    	 eval "$QUIT"
		    	 ;;
	       	    [nN]*)
		    	 scratch_maps=""
		    	 ;;
	       	    *)
		    	 query="y"
		    	 ;;
	       	    esac
	       done
	  fi
	  if [ -n "$scratch_maps" ]
	  then
	       $ECHO "
To copy the NIS maps from the master server, you must know the name of
the NIS MASTER for domain $ypdomain and be sure that it is up."
	       query="y"
	       while [ -n "$query" ]
	       do
	       	    query=""
	       	    $ECHO -n "
Enter \"e\" to EXIT or \"c\" to CONTINUE [no default] ? "
	       	    read answer
	       	    case "$answer" in
	       	    [cC]*)
		    	 ;;
	       	    [eE]*)
		    	 eval "$QUIT"
		    	 ;;
	       	    *)
		   	 query="y"
		    	 ;;
	       	    esac
	       done

	       #
	       # get master server's name.
	       #
	       query="y"
	       while [ -n "$query" ]
	       do
		    $ECHO -n "
NIS MASTER server for domain \"${ypdomain}\" [no default]? "
		    read master
		    if [ -n "$master" ]
		    then
     	       	    	 # Check /etc/hosts for master server.
			 sed "s/#.*//" $HOSTS > $HOSTS.tmp
		     	 good=`egrep "[ 	]$master([	 \.]|$)" $HOSTS.tmp`
		     	 if [ -n "$good" ]
		     	 then
			      short=`echo $hname | sed 's/\..*//'`
			      if [ "$master" = "$hname" ]  || [ "$master" = "$short" ]
			      then
		    	      	   $ECHO "	
This host cannot be both a master and a slave server."
			      else
				   query=""
		              fi
			 else
			      $ECHO "   Cannot find host $master in the file ${HOSTS}"
			      queryhosts="yes"
			      $ECHO "   To add $master to the $HOSTS file you MUST know $master's 
   internet (IP) address."
			      while [ -n "$queryhosts" ]
			      do
			      	   queryhosts=""
			      	   $ECHO -n "
   Would you like to add $master to the $HOSTS file (y/n) [y] ? "
			      	   read answerhosts
			      	   case "$answerhosts" in
			      	   [yY]*|"")
				   	ip="yes"
				   	while [ -n "$ip" ]
				   	do
					     ip=""
					     $ECHO -n "   What is $master's internet (IP) address [no default] ? "
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
						       $ECHO -n "   Is $ipaddress correct (y/n) [no default] ? "
						       read ipchk
						       case "$ipchk" in
					 	       [yY]*)
							    $ECHO "$ipaddress \t $master" >> $HOSTS
							    query=""			 						   ;;
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
				   	;;
			      	   *)
				   	queryhosts="yes"
				   	;;
			      	   esac
			      done
			 fi
		     fi
	       done
		
	       trap 'eval "$QMSG"' 1 2 3 15

	       #
	       # Create NIS Domain
	       #
	       if [ -n "$scratch_maps" ]
	       then
		    rm -rf $YPDIR/$ypdomain
	       fi
	       mkdir $YPDIR/$ypdomain
	       if [ $? -ne 0 ]
	       then
		    $ECHO "
Couldn't create the NIS directory: ${YPDIR}/${ypdomain}."
		    eval "$QUIT"
	       fi

	       #
	       # Copy master's maps to slave
	       #
	       $ECHO "
Copying the NIS maps from $master..."
	       noerrs="y"
	       for mname in $DEFMAPS
	       do
		    $YPXFR -h "$master" -c -d "$ypdomain" "$mname" 2>> $YPDIR/$YPLOG
		    if [ $? -ne 0 ]
		    then
			 $ECHO "	Couldn't transfer map \"${mname}\""
			 noerrs="n"
		    fi
	       done
	       if [ "$noerrs" = "n" ]
	       then
		    $ECHO "
	Some NIS maps were not copied successfully. 
	See the file \"$YPDIR/$YPLOG\" for an explanation."
	       else
		    $ECHO "
	All NIS maps were successfully copied."
	       fi
	  fi	
	  flavor="SLAVE"
	  ;;
     [3]*|"")
	  $ECHO "

			CLIENT Setup

Please make sure that there is at least one system on the network 
configured as an NIS server for domain ${ypdomain}."
	  query="y"
	  while [ -n "$query" ]
	  do
	       query=""
	       $ECHO -n "
Enter \"e\" to EXIT or \"c\" to CONTINUE [no default] ? "
	       read answer
	       case "$answer" in
	       [cC]*)
		    ;;
	       [eE]*)
		    eval "$QUIT"
		    ;;
	       *)
		    query="y"
		    ;;
	       esac
	  done
			
	  flavor="CLIENT"
	  ;;
     *)
	  #
	  # Bad response...
	  #
	  query="y"
	  ;;
     esac
done

#
# Security (-S) option
#
$ECHO "
NOTE: The default answers are RECOMMENDED.

			Ypbind option: -S
			-----------------

	The -S security option locks the domain name and an authorized 
	list of NIS servers.  With the -S option, your system will not
	change domains or bind to an unauthorized NIS server.

	If your network uses SUBNETS, you must use the -S option in order 
	to bind to any servers which are not on the local subnet."

addname="true"
query="y"
while [ -n "$query" ]
do
     query=""
     $ECHO -n " 
	Would you like to use the -S security option (y/n) [y] ? "
     read answerS
     case "$answerS" in
     [yY]*|"")
	not_done="y"
	while [ -n "$not_done" ]
        do
	  $ECHO "
	Enter the authorized servers.  You may enter at most four
	servers; specifying three or four servers is recommended. 
	Press RETURN to terminate the list.
"
	  S_opt="yes"
     	  ypbind_args="-S $ypdomain"
     	  more="yes"
	  num_serv=1
          while [ $num_serv -lt 5 ] && [ "$more" = "yes" ]
     	  do 
	       $ECHO -n "	Server $num_serv name: " 
	      if [ "$flavor" = "CLIENT" ] || [ $num_serv -ne 1 ]
	      then
	       read servname
	       if [ -n "$servname" ]
	       then
		    #
		    # Search /etc/hosts for hostname,
		    #
		    sed "s/#.*//" $HOSTS > $HOSTS.tmp
		    good=`egrep "[ 	]$servname([	 \.]|$)" $HOSTS.tmp`
		    if [ -n "$good" ]
		    then
			short=`echo $hname | sed 's/\..*//'`
			if [ "$servname" = "$hname" ]  || [ "$servname" = "$short" ]
			then
			   addname="false"
			   if [ "$flavor" = "CLIENT" ]
			   then
			      $ECHO "	
\t$servname is not an NIS server."
			   else
			      $ECHO "
\t$servname is already on the list of authorized servers."
			   fi
			else
		      	 $ECHO $servname >> $YPTMP
			fi
		    else
		         $ECHO "\t   Cannot find $servname in the file $HOSTS."
			$ECHO "\t   To add $servname to the $HOSTS file you MUST know 
\t   $servname's internet (IP) address."
		    	 queryhosts="yes"
			 while [ -n "$queryhosts" ]
			 do
			      queryhosts=""
			      $ECHO -n "   
\t   Would you like to add $servname to the $HOSTS file (y/n) [y] ? "
			      read answerhosts
			      case "$answerhosts" in
			      [yY]*|"")
				   ip="yes"
				   while [ -n "$ip" ]
				   do
				   	ip=""
					$ECHO -n "\t   What is $servname's Internet address [no default] ? "
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
						  $ECHO -n "\t   Is $ipaddress correct (y/n) [no default] ? "
						  read ipchk
						  case "$ipchk" in
					 	  [yY]*)
						       $ECHO "$ipaddress \t $servname " >> $HOSTS
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
				   addname="false"
				   ;;
			      *)
				   queryhosts="yes"
				   ;;
			      esac
			 done
			 $ECHO ""
	    	    fi
		    if [ "$addname" = "true" ]
		    then
	            	 ypbind_args="$ypbind_args,$servname" 
			 num_serv=`expr $num_serv + 1`
			 $ECHO $servname >> $YPTMP3
		    fi
		    addname="true"
	       else
		    more="no"
	       fi
	      else
	       $ECHO "$hname   
\t\t\t(An NIS server must specify itself FIRST)"
	       ypbind_args="$ypbind_args,$hname"
	       $ECHO $hname >> $YPTMP3
	       num_serv=`expr $num_serv + 1`
	      fi
	  done
	  if [ -r $YPTMP3 ]
	  then
	  $ECHO "
\tThe list of authorized NIS servers is: "
	  awk '{ printf "\t\t%s\n", $0 }' $YPTMP3
	  query="y"
	  while [ -n "$query" ]
	  do
	     	query=""
	        $ECHO -n "
Enter \"r\" to REDO the authorized servers list, \"e\" to EXIT nissetup,  
or \"c\" to CONTINUE [no default]: "
	        read answer
	        case "$answer" in
		[cC]*)
			not_done=""
		     	;;
		[rR]*)
			if [ -r $YPTMP3 ]
			then
			      rm $YPTMP3
			fi
		     	;;
		[eE]*)
		     	eval "$QUIT"
		     	;;
		*)
			query="y"
		     	;;
		esac
	   done
          else
		$ECHO " 
\tYou must specify at least ONE authorized server."

	  fi
	done
	;;
     [nN]*)
	  ;;
     *)
	  query="y"
	  ;;
     esac
done

setopts="y"
while [ -n "$setopts" ]
do
  setopts=""
  $ECHO "
		Ypbind options: -ypset and -ypsetme
		-----------------------------------

	The ypset command allows local or remote users to set your 
	system's NIS server to a particular host.   

	Would you like to:

		1. ALLOW LOCAL ypset requests only (-ypsetme),
		2. ALLOW ALL ypset requests (-ypset), or
		3. DISALLOW ALL ypset requests (RECOMMENDED) ? "

  if [ "$S_opt" = "yes" ]
  then
    $ECHO "
	NOTE: If you choose 1 or 2, ypset requests will be limited to 
	      the authorized servers which you selected above."
  fi

  query="y"
  while [ -n "$query" ]
  do
     query=""
     $ECHO -n " 
	Enter your choice (1-3) [3]: "
     read s_option
     case $s_option in
     [1]*)
	  ypbind_args="$ypbind_args -ypsetme"
	  ;;
     [2]*)
	  ypbind_args="$ypbind_args -ypset"
	  ;;
     [3]*|"")
	  ;;
     *)
	  query="y"
	  ;;
     esac
     $ECHO -n "
	$hname will "
     case $s_option in
     [1]*)
	$ECHO "ALLOW LOCAL ypset requests only (-ypsetme)."
	;;
     [2]*)
	$ECHO "ALLOW ALL ypset requests (-ypset)."
	;;
     [3]*|"")
	$ECHO "DISALLOW ALL ypset requests."
	;;
     esac
     queryb="y"
     while [ -n "$queryb" ]
     do
	queryb=""
	$ECHO -n "
\tIs this correct (y/n) [no default]? "
	read answerb
	case $answerb in
	[yY]*)
	     ;;
	[nN]*)
	    setopts="y"
	    ;;
	*)
	    queryb="y"
	    ;;
	esac
    done
  done     
done

#
# PHASE TWO... Update files!
#

$ECHO "
Nissetup will configure your system to use all of the NIS databases served
by your NIS master server.  If you choose not to have your system configured
this way, nissetup will help you to customize your configuration."  

queryt="y"
while [ -n "$queryt" ]
do
	queryt=""
	custom="" 
	$ECHO -n "
Would you like nissetup to configure your system to use all of the NIS 
databases served by your NIS server [y]? "
	read answer 
	case "$answer" in
	[yY]*|"")
		useforall="y"
		;;
	[nN]*)
		custom="y"
		;;
	*)
		queryt="y"
		;;
	esac
done 

if [ "$custom" = "y" ] 
then 	

$ECHO "
To use NIS for passwd information, a \"+:\" must be appended to the 
/etc/passwd file on $hname."
query="y"
while [ -n "$query" ]
do
     query=""
     answerp=""
     $ECHO -n "
Would you like to append a \"+:\" to the /etc/passwd file (y/n) [y]? "
     read answer
     case "$answer" in
     [yY]*|"")
	  answerp="y"
	  # Remove blank lines from /etc/passwd
	  awk '$0 !~ /^[ \t]*$/ { print $0; }'  \
	       /etc/passwd > /etc/passwd_tmp
	  mv /etc/passwd_tmp /etc/passwd
		  
	  temp=`tail -1 /etc/passwd | awk '{ print $1 }' - `
	  if [ "$temp" != "+:" ]
	  then
	       $ECHO +: >> /etc/passwd
	       $ECHO ""
	       $ECHO "Updating files:"
	       $ECHO "  /etc/passwd"
	       if [ -r /etc/passwd.dir ] && [ -r /etc/passwd.pag ]
	       then
		    rm -f /etc/passwd.dir /etc/passwd.pag
		    mkpasswd /etc/passwd > /dev/null
		    $ECHO "  /etc/passwd.dir"
		    $ECHO "  /etc/passwd.pag" 
	       fi
	  fi
	  chmod 644 /etc/passwd
	  ;; 
     [nN]*)
          ;;
     *)
     	  query="y"
      	  ;;
     esac
done

$ECHO "
To use NIS for group information, a \"+:\" must be appended to the
/etc/group file on $hname."
query="y"
while [ -n "$query" ]
do
     query=""
     answerg=""
     $ECHO -n "
Would you like to append a \"+:\" to the /etc/group file (y/n) [y]? "
     read answer
     case "$answer" in
     [yY]*|"")
	  answerg="y"
	  # Remove blank lines from /etc/group
	  awk '$0 !~ /^[ \t]*$/ { print $0; }'  \
	       /etc/group > /etc/group_tmp
	  mv /etc/group_tmp /etc/group
		  
	  temp=`tail -1 /etc/group | awk '{ print $1 }' - `
	  if [ "$temp" != "+:" ]
	  then
               $ECHO +: >> /etc/group
	       $ECHO "Updating file: /etc/group" 
	  fi
	  chmod 644 /etc/group
     	  ;;
     [nN]*)
       	  ;;
     *)
     	  query="y"
     	  ;;
     esac
done
   
skipit=""	
grep '^hosts' $SVC | grep 'bind' $1 > /dev/null

if [ $? -ne 0 ] 
then
	$ECHO "
The other default databases served by NIS include the following: 

	aliases, hosts, netgroup, networks, 
	protocols, rpc, and services.

If you would like to use NIS for all of these databases, the system
will now edit the $SVC file such that \"local,yp\" is selected for
each database.
"

	query="y"
	while [ -n "$query" ]
	do
     		query=""
     		$ECHO -n "
Would you like to use NIS for all other default databases (y/n) [y]? "
     		read answer
     		case "$answer" in
     		[yY]*|"")
          		$ECHO "
Editing the $SVC file such that \"local,yp\" is selected for each 
database.  Please wait..."
			$SVCSETUP -auto_nis
			$ECHO "Updating file: /etc/svc.conf" 
			skipit="y"
			;;
     		[nN]*)
          		;;
     		*)
          		query="y"
          		;;
     		esac
	done
fi
  
if [ -z "$skipit" ] 
then
	$ECHO -n "
To use NIS for any other databases, the $SVC file must reference \"yp\"."
 
	query="y"
	while [ -n "$query" ]
	do
     		query=""
     		$ECHO -n "
Would you like to run the svcsetup script to edit $SVC (y/n) [y]? "
     		read answer
     		case "$answer" in
     		[yY]*|"")
	  		$SVCSETUP	
	  		;;
     		[nN]*)
	  		$ECHO "
NIS is NOT used on this system unless the $SVC file refers to \"yp\".

To edit $SVC, run $SVCSETUP.  See the svcsetup(8) 
reference page."
     	  		;;
     		*)
     	  		query="y"
       	  		;;
     		esac
	done
fi	
	
fi 	

#  Edit /etc/rc.config with new configuration information
#
	
if [ "$prev_conf" = "YES" ]
then
	$ECHO -n "
Killing previous NIS daemons..."		
	$NIS stop
	$ECHO "done."
fi
	
$ECHO -n "
Configuring your system to run NIS..."

$ECHO ""
$ECHO "Updating files:" 

trap "" 1 2 3 15

if [ "$useforall" = "y" ]
then
	 #
         # Add +: to /etc/passwd
         #

         # Remove blank lines from /etc/passwd
         awk '$0 !~ /^[ \t]*$/ { print $0; }'  \
                /etc/passwd > /etc/passwd_tmp
         mv /etc/passwd_tmp /etc/passwd
         temp=`tail -1 /etc/passwd | awk '{ print $1 }' - `
         if [ "$temp" != "+:" ]
         then
                 $ECHO +: >> /etc/passwd
		 $ECHO "  /etc/passwd"
                 if [ -r /etc/passwd.dir ] && [ -r /etc/passwd.pag ]
                 then
                          rm -f /etc/passwd.dir /etc/passwd.pag
                          mkpasswd /etc/passwd > /dev/null
			  $ECHO "  /etc/passwd.dir"
			  $ECHO "  /etc/passwd.pag" 
                 fi
         fi
         chmod 644 /etc/passwd

         #
         # Add +: to /etc/group
         #
         # Remove blank lines from /etc/group
         awk '$0 !~ /^[ \t]*$/ { print $0; }'  \
                /etc/group > /etc/group_tmp
         mv /etc/group_tmp /etc/group

         temp=`tail -1 /etc/group | awk '{ print $1 }' - `
         if [ "$temp" != "+:" ]
         then
               	$ECHO +: >> /etc/group
		$ECHO "  /etc/group" 
         fi
         chmod 644 /etc/group

         #
         # Use SVCSETUP to add "yp" to each line of /etc/svc.conf
         $SVCSETUP -auto_nis
	 $ECHO "  $SVC" 
fi 

case $flavor in
"SLAVE")
     #
     # Set updating entries into crontab. (Don't add these for a re-install
     # of a slave server.)
     #
     grep -qs '/var/yp/ypxfr' "$CRFILE"
     if [ $? -ne 0 ]
     then
	  chmod 640 "$CRFILE"
	  $ECHO "# Network Information Service: SLAVE server entries 
30 * * * * sh /var/yp/ypxfr_1perhour
31 1,13 * * * sh /var/yp/ypxfr_2perday
32 1 * * * sh /var/yp/ypxfr_1perday" >> $CRTMP
     fi
     ;;
*)
     ;;
esac

if [ -f "$CRTMP" ]
then
     cat "$CRTMP" >> "$CRFILE"
     rm "$CRTMP"
     $ECHO "  $CRFILE" 
fi

# Set domainname
$DOMAINNAME $ypdomain

$RCMGR set NIS_CONF "YES"
$RCMGR set NIS_TYPE "$flavor"
$RCMGR set NIS_DOMAIN "$ypdomain"
$RCMGR set NIS_ARGS "$ypbind_args"

if [ "$flavor" = "MASTER" ]
then
     $RCMGR set NIS_PASSWDD "$yppasswdd"
fi
$ECHO "  $RCCONF"   	

trap 'eval "$QUIT_AFTER"' 1 2 3 15

query="y"
while [ -n "$query" ]
do
     query=""
     $ECHO -n "
Would you like to start the NIS daemons now (y/n) [y]? "
     read answer
     case "$answer" in
     [yY]*|"")
	  $NIS start
	  ;;
     [nN]*)
	  $ECHO "
You may start the NIS daemons by typing \"$NIS start\", 
or you can allow the new NIS configuration to take effect on the 
next reboot."
	  ;;
     *)
	  query="y"
	  ;;
     esac
done
	
#
# Clean up
#
if [ -r "$YPTMP" ]
then
     rm "$YPTMP"
fi

if [ -r "$YPTMP3" ]
then
     rm "$YPTMP3"
fi

$ECHO "***** NISSETUP COMPLETE *****"

exit 0

