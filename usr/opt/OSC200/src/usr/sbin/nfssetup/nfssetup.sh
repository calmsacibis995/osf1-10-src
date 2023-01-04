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
# @(#)$RCSfile: nfssetup.sh,v $ $Revision: 4.2.14.5 $ (DEC) $Date: 1993/11/09 22:15:33 $ 
#
# Purpose:	Set up NFS environment
# Usage:	nfssetup [client] [server]
# Environment:	Bourne shell script
# Date:		6/11/86
# Author:	Fred L. Templin
# 
# Remarks:
#    Sets up files:
#	/etc/rc.config
#	/etc/exports
#	/etc/fstab
#
#    Much of this has been borrowed from "netsetup.sh"
#

#
# Modification History:
#
#		
#	17-June-91 Larry Cohen
#		Modified for OSF.  No lock daemon or wall daemon.
#		Uses rcmgr to update rc.config.
#		Can now reuse nfssetup to modify parameters.
#
#	28-Jul-88 Fred Glover
#		Add newlines, error checking for #nfsds, #biods
#
#	16-Feb-88 fglover
#		Add support for NFS locking
#

#
# Set up interrupt handlers:
#
ECHO=echo
QUIT='
	if [ -r $EXTMP ]
	then
		rm $EXTMP
	fi
	if [ -r $FSTMP ]
	then
		rm $FSTMP
	fi
	if [ -r $RCTMP ]
	then
		rm $RCTMP
	fi
	${ECHO} "Nfssetup terminated with no installations made."
	exit 1
'
#
# Trap ^c signal, etc.
#



trap 'eval "$QUIT"' 1 2 3 15

EXTMP=/tmp/nfssetup.ex.$$
FSTMP=/tmp/nfssetup.fs.$$
RCTMP=/tmp/nfssetup.rc.$$
VMUNIX=/vmunix
RCFILE=/etc/rc.config
FSFILE=/etc/fstab
EXFILE=/etc/exports
SRCFILE=/sbin/init.d/nfs
NFILE=/etc/networks
NFSSETUP=/usr/sbin/nfssetup
LOCAL_KEY="${ECHO}  'local daemons:\c'"
USR_BIN=/usr/bin
RCMGR=/usr/sbin/rcmgr
NFSD=/usr/sbin/nfsd

nnfsd=0
pcnfsd=n
automount=n
automount_args=""
nbiod=7
#rwall=n
rlock=y
serving=""
verbose=y
first_time=y
full_config=n
nonroot=""
client_only=n
# max allowed nfsd is defined in: ./kernel/nfs/nfs.h: #define MAXNFSDS  128
MAXNFSDS=128
# max. number async_daemons def in: ./kernel/nfs/nfs_vnodeops.c: #define  NFS_MAXASYNCDAEMON 20 
NFS_MAXASYNCDAEMON=20   

#
# PHASE ONE: Gather data!!
#

if [ $1 ]
then
	#
	# Set up default environments for client or server. Runs fast
	# and silent for ease of use in other scripts.
	#
	if [ $1 = "client" ]
	then
		verbose=""
	elif [ $1 = "server" ]
	then
		verbose=""
		nnfsd=8
		serving=y
	else
		${ECHO} "usage: nfssetup [ client ] [ server ]"
		eval "$QUIT"
	fi

	#
	# Require it to be run by root
	#
	if [ \! -w $RCFILE ]
	then
		exit 1
	fi

	#
	# Run it multi-user
	#
	if [ \! -d $USR_BIN ]
	then
		exit 1
	fi

	nm $VMUNIX | grep -s 'nfs_svc' > /dev/null
	if [ $? -ne 0 ]
	then
		${ECHO} "
In order to make use of the network file system (NFS) services,
you must first configure the NFS support code into your 
kernel.  Please consult the System Management Guide for information
on how to configure and bootstrap the new kernel."
		eval "$QUIT"
	fi

	#
	# See if this is a re-install
	#
	nfsstart=`$RCMGR get NFS_CONFIGURED` 
else
	if [ \! -w $RCFILE ]
	then
		${ECHO} "
"$RCFILE" can't be written.  Check to make sure you are root and
that the filesystem is write-enabled. (you may need to issue the
'mount -u /' command if you are in single-user mode.)"
		eval "$QUIT"
	fi

	if [ \! -d $USR_BIN ]
	then
		${ECHO} "
Please bring the system to multi-user mode before running nfssetup."
		eval "$QUIT"
	fi

#  No longer have a separate server subset...
#
#	${ECHO} "
#If you are going to set up an NFS server, be sure you have installed
#the \"Server NFS(tm) Utilities\" subset."

	nfsstart=`$RCMGR get NFS_CONFIGURED` 
	if [ "$nfsstart" =  "1" ]
	then
		${ECHO} "
The network file system has already been installed.  Would
you like to change the current NFS configuration?"
		again=y
		while [ $again ]
		do
			again=""
			${ECHO} "
Enter \"y\" or \"n\" [n]: \c"
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

	if [ $first_time ]
	then
		${ECHO} "Checking kernel configuration..."
	fi

	nm $VMUNIX | grep -s 'nfs_svc' > /dev/null
	if [ $? -ne 0 ]
	then
		${ECHO} "
In order to make use of the network file system (NFS) services,
you must first configure the NFS support code into your 
kernel.  Please consult the System Management Guide for information
on how to configure and bootstrap the new kernel."
		eval "$QUIT"
	fi

	if [ $first_time ]
	then
		${ECHO} "
The nfssetup command configures the network file system (NFS)
environment for your system.  All systems using NFS facilities
must run the Remote Procedure Call (RPC) port mapper daemon.
An entry for this daemon is in ${SRCFILE}
along with entries for the optional daemons you select."
	else
		${ECHO} "
Changing the current NFS configuration."
	fi

# No longer have a separate server subset...
#
#	if [ \! -f "$NFSD" ]
#	then
#		${ECHO} "
#The \"Server NFS(tm) Utilities\" subset does not appear to be installed.
#You may continue the NFS server configuration, but any NFS server daemons
#you select fail to start until the NFS server subset is installed.
#
#\"c\" - continue with client-only configuration
#\"s\" - continue with client and server configuration
#\"q\" - quit the configuration procedure now"
#		again=y
#		while [ $again ]
#		do
#			again=""
#			${ECHO}  "
#Enter \"c\", \"s\", or \"q\" [c]: \c"
#			read ans
#			case $ans in
#			[cC]*|"")
#				client_only=y
#				;;
#			[sS]*)
#				client_only=n
#				;;
#			[qQ]*)
#				eval "$QUIT"
#				;;
#			*)
#				again=y
#				;;
#			esac
#		done
#	fi

	${ECHO} "
You will be asked a series of questions about your system.
Default answers are shown in square brackets ([]).  To use a
default answer, press the RETURN key."
	#
	# Ask if NFS locking should be enabled
	#
	if [ $first_time ]

	then

	${ECHO} ""
	${ECHO} "
	Local locking supports local file and file region locking.
	NFS locking supports local and remote (NFS) file and file region
	locking.  If you would like to disable the NFS locking
	functionality, then answer 'n' to the following question."
	fi	
	again=y
	while [ $again ]
	do
		again=""
		${ECHO} ""
		${ECHO}    "	NFS locking to be enabled [y] ? \c"
		read rlock
		case $rlock in
		[yY]*|"")
			rlock=y
			;;
		[nN]*)
			rlock=n
			;;
		*)
			again=y
			;;
		esac
	done

	#
	# Ask if he wants to run a full NFS Configuration...
	#
#	if [ -z "$first_time" ]
#	then
#	again=y
#	while [ $again ]
#	do
#		again=""
#		${ECHO} ""
#		${ECHO}    "	Would you like to change the rest of your NFS configuration [n] ? \c"
#		read full_config
#		case $full_config in
#		[yY]*)
#			full_config=y
#			;;
#		[nN]*|"")
#			full_config=n
#			;;
#		*)
#			again=y
#			;;
#		esac
#	done



#
#	NO NFS LOCKING YET
#
#	if [ $full_config = n ]
#	then
#	if [ $rlock = y ]
#	then
#		$RCMGR set NFSLOCKING 1
#	else
#		$RCMGR set NFSLOCKING 0
#	fi
#
#	${ECHO}
#	${ECHO}  "Nfssetup terminated with NFS locking \c"
#	if [ $rlock = y ]
#	then
#		${ECHO} "enabled"
#	else
#		${ECHO} "disabled"
#	fi	
#
#	${ECHO} "and prior NFS configuration maintained."
#	${ECHO}
#	exit 0
#	fi
#
#	 End of if not full config
#


#	fi
#
#	 End of if not first time
#
	#
	#   Determine state of this NFS machine.  PURE client (no exports made),
	# or exporter.
	#
	if [ $client_only = n ]
	then
		again=y
		while [ $again ]
		do
			again=""
			${ECHO} ""
			${ECHO}  "	Will you be exporting any directories [n] ? \c"
			read serving
			case $serving in
			[yY]*)
				serving=y
				;;
			[nN]*|"")
				serving=""
				;;
			*)
				again=y
				;;
			esac
		done
	fi

	if [ $serving ]
	then

		again=y
		while [ $again ]
		do
			again=""
			${ECHO} ""
			${ECHO}    "	Do you want to allow non-root mounts [n] ? \c"
			read nonroot
			case $nonroot in
			[yY]*)
				nonroot=y
				;;
			[nN]*|"")
				nonroot=""
				;;
			*)
				again=y
				;;
			esac
		done
	fi

	if [ $serving ]
	then
	#
	# We will promt the user if he/she wants more or fewer daemons than usual.
	#
		nnfsd=8
		${ECHO} ""
		if [ $first_time ]
		then
			${ECHO} "
	Systems that export NFS directories must run /usr/sbin/nfsd to
	handle NFS requests from clients.  It is suggested that between 8 and 24
	(inclusive) nfsd daemons be run.  For average workload situations,
	8 is a good number to run."
		fi
		flag=y
		while [ $flag ]
		do
			flag=""
			${ECHO} ""
			${ECHO}    "	Enter the number of nfsd servers to run [8] : \c"
			read num
			if [ $num ]
			then
				if [ $num -le 0 ]
				then
					flag=y
					${ECHO} "	Number must be greater than zero"
				elif [ $num -ge $MAXNFSDS ]
				then
					nnfsd=$MAXNFSDS
					${ECHO} "	Number reduced to system defined maximum of : $MAXNFSDS"
				else
					nnfsd=$num
				fi
			fi
		done

	fi
	#
	# Ask if he wants any "nfsiod" daemons.
	#
	if [ $first_time ]
	then
		${ECHO} ""
		${ECHO} "
	NFS clients can use block I/O daemons for buffering
	data transfers, although their use is not required.
	It is suggested that between 4 and 12 nfsiod daemons
	be run.  The best number to run is dependent on server
	speed and network bandwidth.  To work most efficiently
	with DEC OSF/1 servers, $nbiod is a good number to run."
	fi
	flag=y
	while [ $flag ]
	do
		flag=""
		${ECHO} ""
		${ECHO}    "	Enter the number of block I/O daemons to run [$nbiod] : \c"
		read num
		if [ $num ]
		then
			if [ $num -lt 0 ]
			then
				flag=y
				${ECHO} "	Number must be greater than or equal zero"
			elif [ $num -ge $NFS_MAXASYNCDAEMON  ]
			then
				nbiod=$NFS_MAXASYNCDAEMON 
				${ECHO} "	Number reduced to system defined maximum of : $NFS_MAXASYNCDAEMON"
			else
				nbiod=$num
			fi
		fi
	done

	#
	# Ask if he wants to run the rpc.pcnfsd daemon...
	#
	if [ $client_only = n ]
	then
		if [ $first_time ]
		then
			${ECHO} ""
			${ECHO} "
		The PC-NFS daemon (rpc.pcnfsd) provides authentication
		and print services for PC-NFS clients.  The daemon is
		not started by default.  To enable rpc.pcnfsd, answer
		'y' to the following question."
		fi
		again=y
		while [ $again ]
		do
			again=""
			${ECHO} ""
			${ECHO}    "	Would you like to run the PC-NFS daemon [n] ? \c"
			read pcnfsd
			case $pcnfsd in
			[yY]*)
				pcnfsd=y
				;;
			[nN]*|"")
				pcnfsd=n
				;;
			*)
				again=y
				;;
			esac
		done
	fi

	#
	# Ask if he wants to run the automount daemon...
	#
	if [ $first_time ]
	then
		${ECHO} ""
		${ECHO} "
	The automount daemon automatically and transparently mounts
	and unmounts NFS file systems on an as-needed basis.  See
	the automount(8) manpage for details."
	fi
	again=y
	while [ $again ]
	do
		again=""
		${ECHO} ""
		${ECHO}    "	Would you like to run the automount daemon [n] ? \c"
		read automount
		case $automount in
		[yY]*)
			automount=y
			;;
		[nN]*|"")
			automount=n
			;;
		*)
			again=y
			;;
		esac
	done

	if [ $automount = y ]
	then

		again=y
		while [ $again ]
		do
			${ECHO} ""
			${ECHO} "  Enter line of arguments to follow 'automount' (if any)."
			${ECHO} "  See the automount(8) manpage for argument definitions."
			${ECHO} "  Note: the rcmgr(8) utility can later be used to change"
			${ECHO} "  the arguments to automount (AUTOMOUNT_ARGS variable)."
			${ECHO} ""
			${ECHO}  "   % automount \c"
			read automount_args

			${ECHO} ""
			${ECHO} "  The automount daemon will be started as follows:"
			${ECHO} ""
			${ECHO} "   % automount" $automount_args
			${ECHO} ""
			${ECHO}  "  Is this correct [y] ? \c"
			read ans
			case $ans in
               	        [yY]*|"")
                                again=""
                                ;;
                        *)
                                again=y
                                ;;
                        esac
		done  
	fi

	#
	# Ask if he wants to run the rwalld daemon...
	#
#	NO WALL DAEMON YET
#
#	if [ $first_time ]
#
#	then
#		${ECHO} ""
#		${ECHO} "
#	NFS clients that rely heavily on having certain NFS
#	directories mounted may wish to be notified in the
#	event of NFS servers going down.  In order for users
#	on your system to receive notifications, you must run
#	the remote wall daemon. (rwalld)"
#	fi
#	again=y
#	while [ $again ]
#	do
#		again=""
#		${ECHO} ""
#		${ECHO}    "	Would you like to run the rwalld daemon [n] ? \c"
#		read rwall
#		case $rwall in
#		[yY]*)
#			rwall=y
#			;;
#		[nN]*|"")
#			rwall=n
#			;;
#		*)
#			again=y
#			;;
#		esac
#	done

	#
	# He's exporting directories.  Find out which ones and validate them
	# but don't add them to "/etc/exports" just yet!
	#
	if [ $serving ]
	then
		if [ $first_time ] || [ \! -f $EXFILE ]
		then
			${ECHO} "
You are now setting up your directory export list.  Enter the
full pathnames of the directories to be exported.  For each
pathname, enter the network group names and/or machine names to
be given access permission to this directory, or a null list to
indicate general permission.  (Network groups are ONLY available
on machines using NIS).  This information is placed in the
/etc/exports file.  Press the RETURN key to terminate the pathname
and permissions lists."
			more_paths=y
		else
			${ECHO} "
	Would you like to add any directory pathnames to the ${EXFILE} file?"
			again=y
			while [ $again ]
			do
				again=""
				${ECHO}  "
	Enter \"y\" or \"n\" [n]: \c"
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
			permlist=""
			${ECHO} ""
			${ECHO}  "Enter the directory pathname: \c"
			read dirname
			if [ $dirname ]
			then
				more_paths=y
				if [ -d $dirname ]
				then
					more_perms=y
					while [ $more_perms ]
					do
						more_perms=""
						${ECHO}  "	Netgroup/Machine name: \c"
						read permname
						if [ -n "$permname" ]
						then
							more_perms=y
							permlist=`${ECHO} $permlist $permname | cat`
						fi
					done
					${ECHO} "$dirname		$permlist" >> $EXTMP
				else 
					${ECHO} "
The pathname: ${dirname}
is not a valid directory.
"
				fi
			else
			${ECHO} "Directory export list complete..."
			fi
		done
	#
	# end of serving
	#
	fi

	#
	# Find out which file systems from which machines are to be imported.
	#
	if [ $first_time ]
	then
		more_hosts=y
		${ECHO} "
You will now be asked to provide information about the remote file
systems you wish to access.  First list the name of the remote host
serving the directories you wish to mount, then give the full directory
pathnames.  Also, for each remote directory, you must specify the full
directory pathname of the mount point on the local machine and whether
the mount is read-only or read-write.  (Nfssetup will create the mount
point directory if it does not already exist.)  Press the RETURN key to
terminate the host and directory pathname lists:"
	else
		${ECHO} "
	Would you like to add any remote file systems to be mounted?"
		again=y
		while [ $again ]
		do
			again=""
			${ECHO}  "
	Enter \"y\" or \"n\" [n]: \c"
			read ans
			case $ans in
				[yY]*)
				more_hosts=y
				;;
				[nN]*|"")
				more_hosts=""
				;;
				*)
				again=y
				;;
			esac
		done
	fi
	newdirs=""
	while [ $more_hosts ]
	do
		more_hosts=""
		${ECHO} ""
		${ECHO}  "Enter the remote host name: \c"
		read hostid
		if [ $hostid ]
		then
			more_hosts=y
			more_paths=y
			while [ $more_paths ]
			do
				more_paths=""
				${ECHO}  "
	Enter the remote directory pathname: \c"
				read rdir
				if [ -n "$rdir" ]
				then
					more_paths=y
					again=y
					while [ $again ]
					do
						again=""
						${ECHO}    "	Enter the local mount point: \c"
						read ldir
						if [ -z "$ldir" ]
						then
							again=y
						elif [ -f $ldir ]
						then
							${ECHO} "
	${ldir}: File exists! Please choose a new mount point."
							again=y
						elif [ \! -d $ldir ]
						then
							${ECHO} "
	${ldir}: Directory does not exist, but will be created."
							newdirs=`${ECHO} $newdirs $ldir | cat`
						fi
					done

					again=y
					while [ $again ]
					do
						again=""
						${ECHO}    "	Is this a read-only mount [y] ? \c"
						read readonly
						case $readonly in
						[nN]*)

							${ECHO} "${rdir}@${hostid}	${ldir}	nfs rw,bg 0 0" >> $FSTMP
							;;
						[yY]*|"")
							${ECHO} "${rdir}@${hostid}	${ldir}	nfs ro,bg 0 0" >> $FSTMP
							;;
						*)
							again=y
							;;
						esac
					done
				fi
			done
		else
			${ECHO} "Remote directory mount list complete..."
		#
		# end hostid
		#
		fi
	done

	#
	# Ask user for verification...
	#
	${ECHO} "
Please confirm the following information which you
have entered for your NFS environment:
"
	if [ $serving ]
	then
		${ECHO} "	${nnfsd} nfsd daemons"
		${ECHO} "	${nbiod} nfsiod daemons"
		if [ $rlock = y ]
		then
			${ECHO} "	locking daemons installed"
		fi

		if [ $pcnfsd = y ]
		then
			${ECHO} "	rpc.pcnfsd daemon installed"
		fi

		if [ $automount = y ]
		then
			${ECHO} "	automount daemon installed"
		fi
#
#
#
#	NO WALL DAEMON YET
#
#		if [ $rwall = y ]
#		then
#			${ECHO} "	rwalld daemon installed"
#		fi
#
		if [ -s $EXTMP ]
		then
			${ECHO} "
	Directory export list:"
			awk '
			{
				printf "\t\t%s", $1
				if ( NF > 1 ) {
					printf " exported to:"
					for ( i = 2; i <= NF; i++ )
						printf " %s", $i
					printf "\n"
				}
				else
					printf " exported with general permissions\n"
			}' $EXTMP
		else
			${ECHO} "
	No directories (in addition to those already in "$EXFILE") exported"
		fi
	else
		${ECHO} "	${nbiod} nfsiod daemons"


		if [ $rlock = y ]
		then
			${ECHO} "	locking daemons installed"
		fi

		if [ $pcnfsd = y ]
		then
			${ECHO} "	rpc.pcnfsd daemon installed"
		fi

		if [ $automount = y ]
		then
			${ECHO} "	automount daemon installed"
		fi
#
#       NO WALL DAEMON YET
#
#		if [ $rwall = y ]
#		then
#			${ECHO} "	rwalld daemon installed"
#		fi


		${ECHO} "
	No directories (in addition to those already in "$EXFILE") exported"

	#
	# end of serving
	#
	fi

	if [ -s $FSTMP ]
	then
		${ECHO} "
	Remote directory mount list:"
		awk '
		BEGIN { FS = " " }
		{
			printf "\t\t%s mounted on: %s, mount options are: %s \n", $1, $2, $4
		}' $FSTMP
	else
		${ECHO} "
	No additional remote directories to mount"
	fi
	again=y
	while [ $again ]
	do
		${ECHO}    "
Enter \"c\" to CONFIRM the information, \"q\" to QUIT nfssetup
without making any changes, or \"r\" to RESTART the procedure [no default]: \c"
		read conf
		case $conf in
			[qQ]*)
			[ -r $EXTMP ] && rm $EXTMP
			[ -r $FSTMP ] && rm $FSTMP
			eval "$QUIT"
			;;
			[rR]*)
			[ -r $EXTMP ] && rm $EXTMP
			[ -r $FSTMP ] && rm $FSTMP
			exec $NFSSETUP $*
			;;
			[cC]*)
			again=""
			;;
			*)
			again=y
			;;
		esac
	done
fi
#
# PHASE TWO...  Update files!!
#
trap "" 1 2 3 15


if [ $verbose ]
then
	${ECHO} ""
	${ECHO} "Updating files:"
	${ECHO}    "	\c"
	${ECHO} $RCFILE
fi
if [ $first_time ]
then

	$RCMGR set NFS_CONFIGURED 1
fi

if [ $serving ]
then
	$RCMGR set NFSSERVING 1
	if [ $nonroot ]
	then
		$RCMGR set NONROOTMOUNTS 1
	else
		$RCMGR set NONROOTMOUNTS 0
	fi
else
	$RCMGR set NFSSERVING 0
fi


#
# Install optional daemons...
#
if [ $nnfsd -ge 0 ] 
then
	$RCMGR set NUM_NFSD ${nnfsd}
fi
if [ $nbiod -ge 0 ]
then
	$RCMGR set NUM_NFSIOD ${nbiod}
fi



if [ $rlock = y ]
then
	$RCMGR set NFSLOCKING 1
else
	$RCMGR set NFSLOCKING 0
fi


if [ $pcnfsd = y ]
then
	$RCMGR set PCNFSD 1
else
	$RCMGR set PCNFSD 0
fi


if [ $automount = y ]
then
	$RCMGR set AUTOMOUNT 1
	$RCMGR set AUTOMOUNT_ARGS "$automount_args"
else
	$RCMGR set AUTOMOUNT 0
fi
#
#       NO WALL DAEMON YET
#
#
#if [ $rwall = y ]
#then
#	$RCMGR set WALLD 1
#else
#	$RCMGR set WALLD 0
#fi




if [ $verbose ]
then
	#
	# Update export list
	#
	if [ -r $EXTMP ]
	then
		${ECHO} "	/etc/exports"
		cat $EXTMP >> $EXFILE
		rm $EXTMP
	fi

	#
	# Update fstab
	#
	if [ -r $FSTMP ]
	then
		${ECHO} "	/etc/fstab"
		cat $FSTMP >> $FSFILE
		rm $FSTMP
	fi

	#
	# Make new local mount point directories...
	#
	if [ -n "$newdirs" ]
	then
		${ECHO} ""
		${ECHO} "Creating local mount points:"
	fi
	for dirname in $newdirs
	do
		object=""
		for subdirs in `${ECHO} $dirname | awk '
		BEGIN { FS = "/" }
		{
			if (substr($0,1,1) != "/")
				print $1
			for (i = 2; i <= NF; ++i) print "/"$i
		}'` 
		do
			object=$object$subdirs
			if [ -f $object ]
			then
				${ECHO} "	Can't create ${object}. File exists!" 
				break
			fi
			if [ \! -d $object ]
			then
				mkdir $object 2> /dev/null
				if [ $? -ne 0 ]
				then
					${ECHO} "	Can't create ${object}. Mkdir failed!"
					break
				fi
			fi
		done
		${ECHO} "	"$dirname
	done
	if [ $first_time ]
	then
	${ECHO} "
The necessary NFS daemon entries have been placed in the file ${SRCFILE}.
In order to begin using NFS, you must now start the daemons and mount
any remote directories you wish to access.  You may either allow nfssetup
to start these daemons automatically or invoke them by hand, but in either
case they will be started automatically on subsequent reboots.
"
	fi
	answer=y
	while [ $answer ]
	do
		${ECHO}    "
If you choose to have nfssetup stop and start the daemons now (without
a reboot), all nfs-related daemons will be stopped, then those you chose
to be run in the preceding questions will be started.

Would you like nfssetup to stop/start the daemons now [y]? \c"
		answer=""
		read ans
		case $ans in
		[yY]*|"")
			${SRCFILE} stop
			${SRCFILE} start
			if [ $first_time ]
			then
				${ECHO} "
The NFS daemons for your machine have been started. In order to mount the
remote directories you wish to access, type the following command after
exiting from nfssetup:

	 /usr/sbin/mount -a -t nfs

Strike the return key to continue..."
			read pause
			fi

			;;
		[nN]*)
			;;
		*)
			answer=y
			;;
		esac
	done
${ECHO} ""
${ECHO} "***** NFSSETUP COMPLETE *****"
fi
exit 0
