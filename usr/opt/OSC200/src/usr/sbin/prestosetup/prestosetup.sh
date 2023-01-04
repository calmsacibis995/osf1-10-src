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
# @(#)$RCSfile: prestosetup.sh,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/11/10 21:23:07 $
# 
QUIT='
	if [ -r $PRESTOTAB_TMP ]
	then
		rm $PRESTOTAB_TMP
	fi
	/bin/echo "Prestosetup terminated with no installations made."
	exit 1
'
# In the event of a termination signal prior to actually setting the
# configuration parameters, print out the above message saying that
# nothing has been changed yet.  The process has been non-disruptive
# to this point.
trap 'eval "$QUIT"' 1 2 3 15

# Fixed constants
VMUNIX=/vmunix
PRESTO_SETUP=/usr/sbin/prestosetup
PRESTO_CMD=/usr/sbin/presto
PRESTO_DAEMON=/usr/sbin/prestoctl_svc
DXPRESTO_CMD=/usr/bin/X11/dxpresto
PRESTO_DEV_DIR=/dev
PRESTO_DEV_NAME=pr0
USR_BIN=/usr/bin
RCMGR=/usr/sbin/rcmgr
LMF_CMD=/usr/sbin/lmf
NFS_INIT_SCRIPT=/sbin/init.d/nfs
PRESTO_INIT_SCRIPT=/sbin/init.d/presto
RCFILE=/etc/rc.config
USR_BIN=/usr/bin
PORTMAP=/usr/sbin/portmap
PRESTOTAB=/etc/prestotab
PRESTOTAB_TMP=/tmp/prestotab.$$

# Variables used to control program flow.
first_time=y
presto_makedev_needed=""

# First perform some sanity checks to inusre that the environment (tools &
# admin files) needed by prestosetup are in place.

if [ \! -w $RCFILE ]
then
	/bin/echo "
"$RCFILE" can't be written.  Check to make sure you are root and
that the filesystem is write-enabled. (you may need to issue the
'mount -u /' command if you are in single-user mode.)"
	eval "$QUIT"
fi

if [ \! -d $USR_BIN ]
then
	/bin/echo "
Please bring the system to multi-user mode before running nfssetup."
	eval "$QUIT"
fi


prestostart=`$RCMGR get PRESTO_CONFIGURED` 
if [ "$prestostart" =  "1" ]
then
	/bin/echo "
The Prestoserve subsystem has already been installed.  Would
you like to change the current Prestoserve configuration?"
		again=y
		while [ $again ]
		do
			again=""
			/bin/echo -n "
Enter \"y\" or \"n\" [n]: "
			read ans
			case $ans in
			[yY]*)
				first_time=""
				;;
			[nN]*|"")
				eval "$QUIT"
				;;
			*)
				again=y
				;;
			esac
		done
fi

# Verify that the LMF licence has been registered.
/bin/echo "Checking LMF licensing..."
$LMF_CMD list for PRESTOSERVE-OA | grep -s active > /dev/null
if [ $? -ne 0 ]
then
	/bin/echo "
The LMF license for Prestoserve has not been registered.  Consult
the Guide to Prestoserve for a full description of using the commands
lmf register, and lmf reset to register the software license.
"
		eval "$QUIT"
fi

# Check to see if presto has been configured into the kernel.
/bin/echo "Checking kernel configuration..."
nm $VMUNIX | grep -s 'prdisable' > /dev/null
if [ $? -ne 0 ]
then
	/bin/echo "
In order to make use of the Prestoserve disk write acceleration services,
you must first configure the prestoserve support code into your 
kernel.  This is done by adding the following to your system specific
kernel configuration file:

pseudo-device	presto

After the addition to the config file the:

doconfig -c HOSTNAME

command may be used to rebuild a new kernel to include the Prestoserve 
kernel support. Please consult the Guide to System Administration for 
information on how to configure and bootstrap the new kernel."
		eval "$QUIT"
fi
if [ $first_time ]
then
/bin/echo ""
/bin/echo "
Note: If the Prestoserve hardware was not present in your system
at installation time it may be necessary to add device specific
information to your system configuration file and to reconfigure
your kernel.  For more information, refer to the Guide to Prestoserve."
/bin/echo ""
fi	

# Now verify that the utilities are installed.
	if [ \! -f $PRESTO_CMD ]
	then
		/bin/echo "
In order to make use of the Prestoserve disk write acceleration services,
the $PRESTO_CMD command must be installed on your system."
		eval "$QUIT"
	fi
	if [ \! -f $PRESTO_DAEMON ]
	then
		/bin/echo "
The Prestoserve daemon $PRESTO_DAEMON is not installed on your system.
This precludes you from running the $DXPRESTO_CMD graphical
interface or to allow remote administration."
		eval "$QUIT"
	fi
	if [ \! -f $DXPRESTO_CMD ]
	then
		/bin/echo "
Warning: The $DXPRESTO_CMD graphical utility is not 
installed on your system.  Therefore all administration of the 
presto functions must be done via the $PRESTO_CMD utility or 
remotely from another host.  The $DXPRESTO_CMD utility 
is installed on a separate subset.  Refer to the Guide to Prestoserve 
for additional information."
	fi

	if [ \! -f $PRESTO_INIT_SCRIPT ]
	then
		/bin/echo "
Warning: The $PRESTO_INIT_SCRIPT Prestoserve startup script is not
installed on your system.  Therefore prestoserve will not startup 
automatically upon system reboot."
	fi

# Create the Prestoserve control device if it doesn't already exist
	/bin/echo "Verifying that the Prestoserve control device is present..."
	if [ \! -c $PRESTO_DEV_DIR/$PRESTO_DEV_NAME ]
	then
		presto_makedev_needed=y
	fi

# Now prompt the user to see what they want to run.
	/bin/echo "
You will be asked a series of questions about which Prestoserve
utilities to run.  Default answers are shown in square 
brackets ([]).  To use a default answer, press the RETURN key."
	if [ $first_time ]
	then
	/bin/echo ""
	/bin/echo "
	Do you wish to have the Prestoserve enabled automatically
	at system startup time?  This involves executing the
	presto command with the -u option."
	fi	

	again=y
	while [ $again ]
	do
		again=""
		/bin/echo ""
		/bin/echo -n "	Automatically enable Prestoserve [y] ? "
		read presto_enable
		case $presto_enable in
		[yY]*|"")
			presto_enable=y
			;;
		[nN]*)
			presto_enable=""
			;;
		*)
			again=y
			;;
		esac
	done
if [ $presto_enable ]
then
	if [ $first_time ] && [ \! -f $PRESTOTAB ]
	then
		/bin/echo ""
		/bin/echo "
	You have selected to automatically enable Prestoserve.
	Now enter the names of the filesystems you wish to
	accelerate.  These names will be entered into the
	$PRESTOTAB file.  If no names are specified then all 
	writable filesystems will be accelerated.  Consider 
	the implications of this question carefully.

	When finished entering filesystem pathnames press 
	the RETURN key only."
		more_paths=y
	else
		/bin/echo "
	If you wish to delete any existing entries in the ${PRESTOTAB}
	file you will have to manually edit that file.
	Would you like to add any filesystems to the ${PRESTOTAB} 
	file?"
		again=y
		while [ $again ]
		do
			again=""
			/bin/echo -n "
	Enter \"y\" or \"n\" [n]: "
			read ans
			case $ans in
				[yY]*)
				more_paths=y
				;;
				[nN]*|"")
				more_paths=""
				;;
				*)
				again=y
				;;
			esac
		done
	fi
	while [ $more_paths ]
	do
		more_paths=""
		/bin/echo ""
		/bin/echo -n "Enter the filesystem pathname: "
		read dirname
		if [ $dirname ]
		then
			more_paths=y
			if [ -d $dirname ] || [ -b $dirname ]
			then
				/bin/echo "$dirname " >> $PRESTOTAB_TMP
			else 
				/bin/echo "
The pathname: ${dirname}
is not a valid directory.
"
			fi
		else
		/bin/echo "Prestoserve acceleration list complete..."
		fi
	done
fi

	if [ $first_time ]
	then
	/bin/echo ""
	/bin/echo "
	Do you wish to have the prestoctl_svc daemon enabled automatically
	at system startup time?  This involves executing the prestoctl_svc 
	command.  The prestoctl_svc daemon must be running if you intend 
	to use the dxpresto graphical interface or if you are allowing
	remote administration of the Prestoserve functions."
	fi	

	again=y
	while [ $again ]
	do
		again=""
		/bin/echo ""
		/bin/echo -n "	Automatically enable prestoctl_svc [y] ? "
		read prestoctl_svc_enable
		case $prestoctl_svc_enable in
		[yY]*|"")
			prestoctl_svc_enable=y
			;;
		[nN]*)
			prestoctl_svc_enable=""
			;;
		*)
			again=y
			;;
		esac
	done

if [ $prestoctl_svc_enable ]
then
	if [ $first_time ]
	then
	/bin/echo ""
	/bin/echo "
	You have selected to run the prestoctl_svc daemon.  Do you 
	wish to allow any network client to be able to change your
	Prestoserve state?  Consider the security implications of 
	this question carefully.  This involves executing the
	prestoctl_svc daemon with the -n option."
	fi	

	again=y
	while [ $again ]
	do
		again=""
		/bin/echo ""
		/bin/echo -n "	Allow remote Prestoserve management [n] ? "
		read prestoctl_svc_n
		case $prestoctl_svc_n in
		[yY]*)
			prestoctl_svc_n="y"
			;;
		[nN]*|"")
			prestoctl_svc_n=""
			;;
		*)
			again=y
			;;
		esac
	done
fi

if [ $prestoctl_svc_enable ]
then
# If they want to run the prestoctl_svc daemon then portmapper must
# be running.  For informational purposes check that out.
	/bin/echo "Verifying that the portmap daemon is running..."
	pid=`/bin/ps -e | grep portmap| grep -v grep | sed -e 's/^  *//' -e 's/ .*//' | head -1`
        if [ "X$pid" = "X" ] 
	then
		/bin/echo "
Warning: The ONC portmap daemon $PORTMAP 
is not currently running.  This daemon is necessary in order for 
the prestoctl_svc daemon to function.  Typically the ONC portmap 
daemon is started in the $NFS_INIT_SCRIPT startup script"
        fi
fi

# Now that all the questions have been asked verify the settings.
/bin/echo "
Please confirm the following information which you
have entered for your Prestoserve setup:
"
if [ $presto_enable ]
then
	/bin/echo "	Automatically start up Prestoserve"
	if [ -f $PRESTOTAB_TMP ] || [ -f $PRESTOTAB ]
	then
		/bin/echo "	Accelerate the following filesystems : "
		if [ -f $PRESTOTAB ]
		then
			cat $PRESTOTAB
		fi
		if [ -f $PRESTOTAB_TMP ]
		then
			cat $PRESTOTAB_TMP
		fi
		echo ""
	else
		/bin/echo "	Accelerate all filesystems"
	fi
else
	/bin/echo "	Do NOT automatically start up Prestoserve"
fi
if [ $prestoctl_svc_enable ]
then
	/bin/echo "	Automatically start up prestoctl_svc"
	if [ $prestoctl_svc_n ]
	then
		/bin/echo "	Any network host can change presto state"
	fi
else
	/bin/echo "	Do NOT automatically start up prestoctl_svc"
fi

# Prompt for confirmation.
again=y
while [ $again ]
do
	/bin/echo -n "
Enter \"c\" to CONFIRM the information, \"q\" to QUIT prestosetup
without making any changes, or \"r\" to RESTART the procedure [no default]: "
	read conf
	case $conf in
		[qQ]*)
		eval "$QUIT"
		;;
		[rR]*)
		[ -r $PRESTOTAB_TMP ] && rm $PRESTOTAB_TMP
		exec $PRESTO_SETUP $*
		;;
		[cC]*)
		again=""
		;;
		*)
		again=y
		;;
	esac
done

# Do the work
# First set the values in /etc/rc.config which will later be read by  
# the presto startup script in /etc/init.d when the system is booting.
# Starting here the system configuration changes are being made.
# Therefore is isn't appropriate to print out a message saying that
# no changes have been made in response to a termination signal.  So
# change the trap vectors to not print out anything.
trap "" 1 2 3 15
/bin/echo ""
/bin/echo "Updating files:"
/bin/echo -n "	"
/bin/echo $RCFILE
if [ $first_time ]
then

	$RCMGR set PRESTO_CONFIGURED 1
fi
if [ $presto_enable ]
then
	$RCMGR set PRESTO_ENABLE 1
else
	$RCMGR set PRESTO_ENABLE 0
fi
if [ $prestoctl_svc_enable ]
then
	$RCMGR set PRESTO_SVC_ENABLE 1
else
	$RCMGR set PRESTO_SVC_ENABLE 0
fi
if [ $prestoctl_svc_n ]
then
	$RCMGR set PRESTO_SVC_ANY 1
else
	$RCMGR set PRESTO_SVC_ANY 0
fi

if [ -r $PRESTOTAB_TMP ]
then
	/bin/echo "	$PRESTOTAB"
	cat $PRESTOTAB_TMP >> $PRESTOTAB
	rm $PRESTOTAB_TMP
fi
if [ $presto_makedev_needed ]
then
	/bin/echo "
	Creating the Prestoserve control device $PRESTO_DEV_DIR/$PRESTO_DEV_NAME:"
	cur_dir=`pwd`
	cd $PRESTO_DEV_DIR
	./MAKEDEV $PRESTO_DEV_NAME
	if [ \! -c $PRESTO_DEV_DIR/$PRESTO_DEV_NAME ]
	then
		/bin/echo "
	Unable to create the Prestoserve control device:
	$PRESTO_DEV_DIR/$PRESTO_DEV_NAME"
		cd $cur_dir
		eval "$QUIT"
	fi
	cd $cur_dir
fi
if [ $first_time ]
then
/bin/echo "
The necessary Presto daemon entry and Presto enable command have been 
placed in the file ${PRESTO_INIT_SCRIPT}.  In order to begin using Presto, 
you must now start the daemon and enable Presto.  You may either allow 
prestosetup to perform these tasks automatically or you may invoke them 
by hand, but in either case they will be started automatically on 
subsequent reboots.
"
fi
answer=y
while [ $answer ]
do
	/bin/echo -n "
If you choose to have prestosetup stop and start Presto acceleration 
now (without a reboot), all Presto acceleration will be stopped, then those 
functions you chose to be run in the preceding questions will be started.
You probably do not want to automatically startup Prestoserve acceleration
unless all the filesystems targeted for acceleration are already created 
and mounted.

Would you like prestosetup to stop/start Presto acceleration now [n]? "
	answer=""
	read ans
	case $ans in
	[yY]*)
		if [ \! -f $PRESTO_INIT_SCRIPT ]
		then
			/bin/echo -n "
Unable to startup Presto services because $PRESTO_INIT_SCRIPT
is not installed on your system.
"
			exit 1
		fi
		${PRESTO_INIT_SCRIPT} stop
		${PRESTO_INIT_SCRIPT} start
		if [ $first_time ]
		then
			if [ $prestoctl_svc_enable ]
			then
				/bin/echo "
The Presto daemon for your machine has been started and Presto acceleration
has been enabled. 
"
			else
				/bin/echo "
The Presto acceleration has been enabled.
"
			fi

		fi

		;;
	[nN]*|"")
		;;
	*)
		answer=y
		;;
	esac
done
/bin/echo ""
/bin/echo "***** PRESTOSETUP COMPLETE *****"
exit 0
