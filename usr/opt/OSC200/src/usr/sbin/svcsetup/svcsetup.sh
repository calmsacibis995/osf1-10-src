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
#       @(#)svcsetup.sh	3.3     (ULTRIX/OSF)        8/9/91
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
# Purpose:	Set up /etc/svc.conf
# Usage:	svcsetup
# Environment:	Bourne shell script
# 
# Remarks:
#    Sets up files:
#	/etc/svc.conf
#

# files
case $1 in
DEBUG)
	shift
	DEBUG=1
	SVC=/tmp/svc.conf
	RCCONF=/tmp/rc.config
echo "Running in DEBUG mode ...
"
	;;
-auto_nis)
	shift
	SVC=/etc/svc.conf
        RCCONF=/etc/rc.config
	AUTO_NIS=1
	;;
-auto-bind-1)
	shift
	SVC=/etc/svc.conf
	RCCONF=/etc/rc.config
	AUTO_BIND_1=1
	;;
-auto-bind-2)
	shift
	SVC=/etc/svc.conf
        RCCONF=/etc/rc.config
        AUTO_BIND_2=1
        ;;
-auto-bind-remove)
	shift
	SVC=/etc/svc.conf
	RCCONF=/etc/rc.config
	AUTO_BIND_REMOVE=1
	;; 
*)
	SVC=/etc/svc.conf
	RCCONF=/etc/rc.config
	;;
esac
#
# Other declarations
#
ECHO=/bin/echo
umask 022
NULL=/dev/null
SVCTMP=/tmp/svc.$$
DBTMP=/tmp/svc_db.$$
ALL="0 1 2 3 4 5 6 7 8"
SVCORDER1="local"
SVCORDER2="local,yp"
SVCORDER3="local,bind"
SVCORDER4="bind,local"
SVCORDER5="local,bind,yp"
SVCORDER6="bind,local,yp"

DB0=aliases
DB1=group
DB2=hosts
DB3=netgroup
DB4=networks
DB5=passwd
DB6=protocols
DB7=rpc
DB8=services

verbose=y

#
# Set up interrupt handlers:
#
QUIT='
	if [ -r $SVCTMP ]
	then
		rm $SVCTMP
	fi
	if [ -r $DBTMP ]
	then
		rm $DBTMP
	fi
	$ECHO "
Svcsetup terminated without editing the $SVC file."
	exit 1
'

#
# Trap ^c signal, etc.
#
trap 'eval "$QUIT"' 1 2 3 15

#
# PHASE ONE: Gather data!!
#

	#
	# Require it to be run by root
	#
	if [ \! -w $RCCONF ]
	then
		$ECHO "Please su to root."
		eval "$QUIT"
	fi

	#
	# Be sure network has already been set up, and this baby has a name!!
	#

	hname=`hostname`
	if [ $? -ne 0 ]
	then
		$ECHO "
Bring the system to multi-user mode before running svcsetup."
		eval "$QUIT"
	fi
	#
	# Introduction
	#
	if [ "$AUTO_NIS" -ne 1 -a "$AUTO_BIND_1" -ne 1 -a "$AUTO_BIND_2" -ne 1 -a "$AUTO_BIND_REMOVE" -ne 1 ] 
	then
	$ECHO "
***************************************************************
*							      *
*	Svcsetup:  Modify the /etc/svc.conf file              *
*							      *
***************************************************************
		

Svcsetup modifies the $SVC file, allowing only valid 
entries.
    
Your system uses the $SVC file to locate system and 
network information.  For each type of information available, 
the file contains an entry which tells the system where to find 
that information.  

For example, the /etc/services file may exist locally on your 
system and be served by the Network Information Service (NIS,
formerly YP).  If the $SVC file contains the line 
\"services=local,yp\", your system will search the local 
/etc/services file for Internet service information and then
query NIS if it could not find the information locally.

[ Press the RETURN key to continue ]: "
	read junk
	$ECHO "  
For all databases EXCEPT the hosts database, the two choices for 
finding the information are \"local\" and \"local,yp\".  Use \"local\" 
if you do not want your system to use NIS for a particular database, 
or if you are not using NIS at all.  Use \"local,yp\" if you want 
your system to query NIS for information that is not found locally. 

For the hosts database, four additional settings are possible which 
allow the system to query the Berkeley Internet Name Domain (BIND) 
service for host information. 

The passwd and group entries exist only for compatibility.  To use 
NIS for passwd information or group information, a \"+:\" must be 
added to the end of the /etc/passwd or /etc/group file, respectively.

NOTE that \"yp\" corresponds to NIS which was formerly called 
\"yellow pages\".   

Changes to $SVC take effect immediately.

[ Press the RETURN key to continue ]: "
	read junk

	done=
	
	while test -z "$done"
	do
		$ECHO "
	Configuration Menu for the $CLIENTROOT$SVC file

	Modify File      => m
	Print File       => p
	Exit             => e"

		$ECHO
		$ECHO -n "Enter your choice [m]: "
		read action
		case "$action" in
		e|E)
			repeat=n
			done=done
			;;
		m|M|"")
			done=done
			;;
		p|P)
			#
			# Print file contents and check
			#
			$ECHO "
The $CLIENTROOT$SVC file on \"`hostname`\" currently contains the following settings:"
			$ECHO
			cat $CLIENTROOT$SVC | awk '{
				if ($1 ~ /#/ || $2 ~ /#/ || NF == 0)
					next
				else
					print $0
			}'
			;;
		esac
	done
	case $action in
	[mM]|"")
		done=
		while test -z "$done"
		do
			$ECHO "
	Change Menu for the $CLIENTROOT$SVC file

	aliases	  	=> 0
	group   	=> 1
	hosts 	  	=> 2
	netgroup  	=> 3
	networks 	=> 4
	passwd  	=> 5
	protocols	=> 6
	rpc  		=> 7
	services  	=> 8

	ALL of the above  => 9
	NONE of the above => 10"

			$ECHO
			$ECHO -n "Enter your choice(s).  For example \"0 3 5\" [no default] : "
			read X
			case $X in
			"")
				;;
			*)
				for I in $X
				do
					#
					# Is it a number?
					#
					J=`expr $I : '\([0-9][0-9]*\)'`
					case $I in
					9)
						LIST=$ALL
						done=y
						;;
					10)
						eval "$QUIT"
						;;
					$J)
						#
						# is it in range?
						#
						if [ $I -gt 10 ]
						then
							$ECHO "
		Invalid Choice: $I (out of range)"
							continue
						else
							if [ "$LIST" != "$ALL" ]
							then
							    LIST="$LIST $I"
							fi
						fi
						done=y
						;;
					*)
						$ECHO "
		Invalid choice: $I (malformed number)"
						;;
					esac
				done
				;;
			esac
		done
		first="yes"
			
		for i in $LIST
		do
			case $i in
			0)
				db=aliases
				;;
			1)
				db=group
				;;
			2)
				db=hosts
				;;
			3)
				db=netgroup
				;;
			4)
				db=networks
				;;
			5)
				db=passwd
				;;
			6)
				db=protocols
				;;
			7)
				db=rpc
				;;
			8)
				db=services
				;;
			
			esac

			if [ $i -ge 0 -a $i -le 8 ]
			then
				
				done=
				while test -z "$done"
				do
				    if [ "$first" = "yes" ]
				    then 
				      first="no"
				      $ECHO "
*********************************
*  Name Service Order Selection *
*********************************

	local		     => 1
	local,yp             => 2
	local,bind	     => 3    (hosts database only) 
	bind,local	     => 4    (hosts database only) 
	local,bind,yp        => 5    (hosts database only) 
	bind,local,yp        => 6    (hosts database only)"
				     
					$ECHO "
Enter the name service order for each of the following databases:"
					$ECHO
				     fi
					$ECHO -n "\"$db\" database [2]: "
					read svc_order
					case $svc_order in
					1|2)
						done=done
						;;
					3|4|5|6)
						if [ "$db" != "hosts" ]
						then
							$ECHO "
Invalid selection.  Only choices 1 and 2 are valid. "
						else
							done=done
						fi
						;;
					"")
						svc_order=2
						done=done
						;;
					*)
						$ECHO "
Invalid selection."
						;;
					esac
				done
				ORDER=SVCORDER$svc_order
				eval ORD=\$$ORDER
				$ECHO $db=$ORD >> $DBTMP
			fi
		done
		;;
	[eE])
		eval "$QUIT"
		;;
	esac

else
   if [ "$AUTO_NIS" -eq 1  ]
   then 
	#
	# Add yp as the last switch on every line
	# 
	verbose=""
	LIST=$ALL
	grep -v yp $SVC | awk '{
				if ($1 ~ /#/ || $2 ~ /#/ || NF == 0)
					next
				else
					print $0",yp"
			}' >> $DBTMP 
   fi 
   if [ "$AUTO_BIND_1" -eq 1 ]
   then
	#
	# Add bind as the first switch on the hosts line of $SVC
	# 
	verbose=""
	grep hosts= $SVC | grep yp >> /dev/null
	if [ $? -eq 0 ]
	then 
		$ECHO "hosts=bind,local,yp" >> $DBTMP
		LIST="2" 
	else
		$ECHO "hosts=bind,local" >> $DBTMP
		LIST="2" 
	fi
   fi 	
   if [ "$AUTO_BIND_2" -eq 1 ]
   then
	#
        # Add bind as the second switch on the hosts line of $SVC
	# 
        verbose=""
        grep hosts= $SVC | grep yp >> /dev/null
        if [ $? -eq 0 ]
        then 
		$ECHO "hosts=local,bind,yp" >> $DBTMP
		LIST="2" 
        else
		$ECHO "hosts=local,bind" >> $DBTMP
		LIST="2" 
        fi
   fi 
   if [ "$AUTO_BIND_REMOVE" -eq 1 ]
   then
	#
	# Remove bind from the hosts line of $SVC
	#
	verbose=""
	grep bind $SVC | grep hosts= | grep yp >> /dev/null
	if [ $? -eq 0 ]
        then
        	$ECHO "hosts=local,yp" >> $DBTMP
                LIST="2"
        else
                grep bind /etc/svc.conf | grep hosts= >> /dev/null
                if [ $? -eq 0 ]
                then 
			$ECHO "hosts=local" >> $DBTMP
                        LIST="2"
                fi
        fi
   fi
fi 
#
# PHASE TWO... Update file
#
trap "" 1 2 3 15

if [ $verbose ]
then
        $ECHO ""
        $ECHO "Updating file:"
	$ECHO "	$CLIENTROOT$SVC"
fi
for i in $LIST
do
	case $i in
	0|1|2|3|4|5|6|7|8)
		db=DB$i
		eval DB=\$$db
		line=`grep "^$DB" $DBTMP`
		if [ -n "$line" ]
		then
			ed - $CLIENTROOT$SVC << END >> $NULL
/^$DB
d
-
a
$line
.
w
q
END
		fi
		;;
	esac
done
#
# Clean up
#
if [ -r $SVCTMP ]
then
	rm $SVCTMP
fi
if [ -r $DBTMP ]
then
	rm $DBTMP
fi
if [ $verbose ]
then
	$ECHO ""
	$ECHO "***** SVCSETUP COMPLETE *****"
fi

exit 0
