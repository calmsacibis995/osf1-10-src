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
# Purpose:	Set up the POLYCENTER Common Agent (SNMP Agent)
# Usage:	snmpsetup
# 
# Remarks:
#    Sets up files:
#       snmp_pe.conf
#       internet_mom.conf
#
#
# Modification History:
#
#
# Set up interrupt handlers:
#
QUIT='
	if [ -r $SNMPCONFTMP ]
	then
		rm $SNMPCONFTMP
	fi
	if [ -r $INETCONFTMP ]
	then
		rm $INETCONFTMP
	fi
	echo "snmpsetup terminated with no configuration changes made."
	exit 1
'
#
# Trap ^c signal, etc.
#
trap 'eval "$QUIT"' 1 2 3 15


SNMPCONFTMP=/tmp/snmp_pe.conf.$$.tmp
INETCONFTMP=/tmp/internet_mom.conf.$$.tmp

if [ `machine` = "mips" ]
then
	SNMPCONF=/var/kits/ecalocal/snmp_pe.conf
	SNMPCONFSAV=/var/kits/ecalocal/snmp_pe.conf.sav
	INETCONF=/var/kits/ecalocal/internet_mom.conf
	INETCONFSAV=/var/kits/ecalocal/internet_mom.conf.sav
fi
if [ `machine` = "alpha" ]
then
	SNMPCONF=/etc/eca/snmp_pe.conf
	SNMPCONFSAV=/etc/eca/snmp_pe.conf.sav
	INETCONF=/etc/eca/internet_mom.conf
	INETCONFSAV=/etc/eca/internet_mom.conf.sav
fi


SNMPCONF_HEADER="#
# snmp_pe.conf
# SNMP Network Management Agent Configuration File
# for POLYCENTER Common Agent
#"
INETCONF_HEADER="#
# This is the configuration file for the TCP/IP MOM.
# A '#' in the first line indicates a comment.
# A line should not be greater than 1023 characters.
# The first line should contain information about the
# location of the system.
# The second line should contain information about the
# contact person for the system.
# The third line should contain the default Link Polling
# Interval value used internally by the Internet MOM (in seconds)."

USR_BIN=/usr/bin

serving=""
startup=""
verbose=y
first_time=y
full_config=n


#
# PHASE ONE: Gather data!!
#

if [ \! -w $SNMPCONF ]
then
	echo "Please su to root first."
	eval "$QUIT"
fi

echo  "
	This script sets up the appropriate configuration
	files for the POLYCENTER Common Agent (SNMP Agent).

	Press return to continue : \c"
read junk



#
# gather the information for internet_mom.conf
#

echo ""
echo  "	Enter the name of the system administrator [Unknown] : \c"
read admin_name
if [ "$admin_name" = "" ]
then
	admin_name="Unknown"
fi

echo ""
echo  "	Enter the physical location of the system [Unknown] : \c"
read location
if [ "$location" = "" ]
then
	location="Unknown"
fi

echo ""
echo  "	Enter the link polling interval [60] : \c"
read timerval
if [ "$timerval" = "" ]
then
	timerval="60"
fi

echo "$INETCONF_HEADER" >> $INETCONFTMP
echo "$location" >> $INETCONFTMP
echo "$admin_name" >> $INETCONFTMP
echo "$timerval" >> $INETCONFTMP


#
# Community configuration for snmp_pe.conf
#
echo "

	A community name is used by the SNMP protocol to authenticate 
	requests from a Network Management Station (NMS).  To define
	a community that accepts management requests from a particular
	NMS, you must assign a community name, and know the IP address of 
	the NMS.  For example, the following entry defines a read-only 
	community whose name is "test" that can be monitored by the NMS 
	whose IP address is 128.45.10.100:

		community	test	128.45.10.100	readonly

	A community is required to allow an NMS to monitor your system.
	If you would like your system to be monitored by any network manager,
	use an IP address associated with the community of 0.0.0.0.  For
	example,

		community	public  0.0.0.0   readonly
"
echo >> $SNMPCONFTMP
echo "$SNMPCONF_HEADER" >> $SNMPCONFTMP
pub=f
again=y
while [ $again ]
do
	echo  "	Enter community name [RETURN when done] : \c"
	read comm
	if [ "$comm" = "" ]
		then
		break
	fi
	echo 	"	Enter IP address associated with community $comm [0.0.0.0]? \c"
	read ipaddr
	if [ "$ipaddr" = "" ]
		then
		ipaddr="0.0.0.0"
	fi
	echo 	"	Select community type (readonly,readwrite) [readonly]? \c"
	read commtype
	case $commtype in
		readonly)	;;
		readwrite)	;;
		*)		commtype="readonly" ;;
	esac
	echo "community	$comm	$ipaddr	$commtype" >> $SNMPCONFTMP
	echo
	if [ $comm = public -a $ipaddr = 0.0.0.0 -a $commtype = readonly ]
		then
		pub=t
	fi
	echo 	"	Do you wish to add another community [n]? \c"
	read ans
	case $ans in
	 [yY]*)
		again="y" ;;
	 [nN]*|"")
		again="" ;;
	 *)
		again="" ;;
	esac
done
echo
egrep -s community $SNMPCONFTMP
if [ $? = 1 ]
	then
	echo "

	You have not configured any community.  Without a community, 
	the POLYCENTER Common Agent will not respond to any NMS 
	management requests."

	echo "
	A public community accessible by ANYONE can be configured for
	you as:

             community   public   0.0.0.0   readonly
"
	again=y
	while [ $again ]
	do
		echo 	"	Add the public community [y]? \c"
		read ans
		case $ans in
		 [yY]*|"")
			echo "community	public	0.0.0.0	readonly" >> $SNMPCONFTMP
			again=""
			;;
		 [nN]*)
			again="" ;;
		 *)
			again=y ;;
		esac
	done
fi


#
# TRAP Community configuration for snmp_pe.conf
#
echo "

	A trap community name is used by the SNMP protocol to send SNMP
	traps to an interested Network Management Station (NMS).  To 
	define an NSM that is to receive SNMP traps, you must assign a 
	trap community name, and know the IP address of the NMS.  For 
	example, the following entry defines a trap community whose name
	is "trap1" at IP address 129.40.11.200 that will receive SNMP traps
	sent out by this agent:

		trap	trap1	129.40.11.200


	Trap community specifications are optional.  No traps are sent
	if there are no trap communities specified.

"
echo >> $SNMPCONFTMP
again=y
while [ $again ]
do
	echo  "	Enter trap community name [RETURN when done] : \c"
	read comm
	if [ "$comm" = "" ]
		then
		break
	fi
	echo 	"	Enter IP address associated with trap community $comm? \c"
	read ipaddr
	if [ "$ipaddr" = "" ]
		then
		break
	fi

	echo "trap	$comm	$ipaddr" >> $SNMPCONFTMP
	echo
	echo 	"	Do you wish to add another trap community [n]? \c"
	read ans
	case $ans in
	 [yY]*)
		again="y" ;;
	 [nN]*|"")
		again="" ;;
	 *)
		again="" ;;
	esac
done
echo


#
# Authentication Failure Trap configuration for snmp_pe.conf
#
echo "

	Do you wish to disable authentication failure traps [n]? : \c"
read disable_authen
case $disable_authen in
 [nN]*|"")
	;;
 [yY]*)
	echo "" >> $SNMPCONFTMP
	echo "no_auth_traps" >> $SNMPCONFTMP
	;;
     *)	
	;;
esac


#
# PHASE TWO...  Update files!!
#
trap "" 1 2 3 15


if [ $verbose ]
then
	echo ""
	echo ""
	echo "	Saving old files in:"
	echo "		$SNMPCONFSAV"
	echo "		$INETCONFSAV"
	echo ""
	echo "	Creating new files:"
fi

#
# Update snmp_pe.conf
#
if [ -r $SNMPCONFTMP ]
then
 	echo "		$SNMPCONF"
	if [ -r $SNMPCONF ]
	then
		mv $SNMPCONF $SNMPCONFSAV
	fi
	cat $SNMPCONFTMP >> $SNMPCONF
	rm $SNMPCONFTMP
fi

#
# Update Internet MOM's configuration file
#
if [ -r $INETCONFTMP ]
then
	echo "		$INETCONF"
	if [ -r $INETCONF ]
	then
		mv $INETCONF $INETCONFSAV
	fi
	cat $INETCONFTMP >> $INETCONF
	rm $INETCONFTMP
fi


#
# Inquire if they want to restart the POLYCENTER Common Agent (SNMP Agent)
#

echo	""
echo	"	Do you wish to restart the POLYCENTER Common Agent (SNMP Agent) [y]? \c"
read ans
case $ans in
 [nN]*)
	;;
 *)
	if [ `machine` = "mips" ]
	then
		/usr/etc/common_agent start
        fi

	if [ `machine` = "alpha" ]
	then
		/sbin/init.d/common_agent start
        fi
	;;
esac

	echo ""
	echo "***** SNMPSETUP COMPLETE *****"
exit 0
