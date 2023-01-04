#!/sbin/sh
#
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
# @(#)$RCSfile: .profile,v $ $Revision: 4.2.13.6 $ (DEC) $Date: 1994/01/03 20:21:30 $ 
# 
#	.rootprofile - start the GENERIC ROOT phase of an OSF/1
#		initial system load.
#
#	SCCSID	"@(#).profile	3.9	(DEC OSF/1)	12/31/91"
#
#	*********************************************************************
#	*                                                                   *
#	*      Copyright (c) Digital Equipment Corporation, 1991, 1994      *
#	*                                                                   *
#	*                       All Rights Reserved.                        *
#	*                                                                   *
#	*********************************************************************
#
#	Modification History:
#
#	000	20-APR-1991	ccb
#	001	30-may-1991	ccb
#		many bugfixes
#
#	002	21-jun-1991	ccb
#		inspections, more bugfixes. This is the version that
#		went as TIN BL5
#
#	003	02-jul-1991	ccb
#		clean up removal of netstart
#		leave .profile in / instead of /tmp
#		print message about logfile locations
#		stty sane for autoboot
#
#	004	12-jul-1991	ccb
#		change destination directory for logfiles
#		install configured version of lib_admin.conf so that
#			"it" can find it
#		clean up /sbin/pwd
#
#	005	18-jul-1991	ccb
#		clean up /tmp/{fstmp,showboot}
#		set CLOAD, BLOAD for use by CDROM install
#
#	006	11-oct-1991	ccb
#		add support for shared lib changes and lib_admin.dir
#
#	007	04-dec-1991	ccb
#		remove finder.devs finder.tab and showboot from /tmp
#
#	008	31-dec-1991	ccb
#		Redirect output of mv in Cleanup to /dev/null


# Name of the Installation Guide.
#
#	this gets used if the ISLINFO file cannot
#		be accessed.

ISL_IGNAME="DEC OSF/1 Installation Guide"


:	-Cleanup
#		clean up all of the installation leftovers.
#
#	given:	nil - uses global $ISLINFO
#	does:	place the system in a suitable state for confiuration
#		cleans up installation specific trash

Cleanup()
{
	ISLFILES="$ISLINFO sbin/pwd tmp/finder.dev tmp/finder.tab tmp/fstmp"

	[ -f /netstart ] && rm -rf /netstart

	cp -p /real.profile /.profile			#! should use .proto..
	cp -p /etc/.proto..inittab /etc/inittab

	mkdir isl 2> /dev/null
	mv $ISLFILES isl 2> /dev/null
	mv *.log /var/adm/smlogs

	rm -rf isl
	rm -rf /var/tmp/MUPCTRL /var/tmp/hMUPCTRL
	rm -rf /var/tmp/mupctrl /var/tmp/hmupctrl

	# dismount the distribution
	umount /kit
	rmdir /kit

	echo "
The installation software has successfully installed your system.

There are logfiles that contain a record of your installation.
These are:

	/var/adm/smlogs/install.log	- general log file
	/var/adm/smlogs/install.FS.log	- file system creation logs
	/var/adm/smlogs/setld.log	- log for the setld(8) utility
	/var/adm/smlogs/fverify.log	- verification log file

" | tee -a /etc/motd

	echo "The above message is also recorded in /etc/motd for your
future reference.
"

}



:	-Connect
#		read data from previous installation phase.
#
#	given:	nil
#	does:	reads $ISLINFO
#	return:	0 if successful. 1 if $ISLINFO is zero-length or
#		does not exist
#
#	side effect:
#		sets all variables from $ISLINFO

Connect()
{
	#%{ LOCALIZE
	Connect_INFO1="
The installation procedure has failed to determine the type of
load and system devices you are using.

The installation procedure cannot continue.

Consult the $ISL_IGNAME for troubleshooting information."

	Connect_NOT_FOUND="Cannot find device file."
	#}% LOCALIZE

	[ -s $ISLINFO ] ||
	{
		echo "$Connect_INFO1"
		return 1
	}

	. $ISLINFO || return 1

	return 0
}



:	-Error
#		Print an error message on stderr
#
#	given:	$* - strings
#	does:	print strings on stderr
#	return:	NIL

Error()
{
	1>&2 echo $*
}



:	-Main
#		This is the main body of the program
#
#	given:	NIL
#	does:	gets root filesystem into a writable state, calls
#		install.osf by way of the logger. Cleans up.
#	return:	ends by either halting the processor or running
#		init 3

Main()
{
	#%{ LOCALIZE

	LC_Y=y			# localized 'yes'
	LC_N=n			# localized 'no'

	Main_CONFIGURE="
You can now choose to configure the system for use or you can
defer configuration. If you choose to defer configuration, the
system will halt. Configuration will happen next time the system
is booted.

If you choose to configure now, you will be asked a series
of questions. The system will then generate a customized kernel,
reboot and be ready to use.

Would you like to configure the system for use at this time? ($LC_Y/$LC_N): \c"

	Main_INITSTART="
***	CONFIGURING THE SYSTEM"

	Main_INSTERR="
Installation failed. Exiting"

	Main_MOUNTERR="
Cannot update root (/) mount. The installation cannot continue."

	#}% LOCALIZE

	ISL_PATH=/kit/isl
	[ -f /hinstall ] &&
	{
		HINSTALL=1
		ISL_PATH="/kit/hisl:$ISL_PATH"
	}
	PATH="$ISL_PATH:/sbin:/etc:/bin:/usr/sbin:/usr/lbin:."
	ISLINFO=/.islinfo

	export HINSTALL ISLINFO ISL_PATH PATH

	stty sane dec

	echo "\n\n"

	Connect || return 1

	fsck -y /dev/$CROOT 2>&1 > /dev/null && mount -u /dev/$BROOT / ||
	{
		Error "$Main_MOUNTERR"
		return 1
	}

	# clean up RIS' Pieces and other debris
	rm -f vmunix.sas vmunix.bootp hvmunix.sas hvmunix.bootp
	rm -f RisFiles netload restoresymtable
	rm -rf /isl

	sync

	# turn on the network
	[ "$TLOAD" = REMOTE ] &&
	{
		chmod +x /netstart
		/netstart ||
		{
			Error "Cannot restart the network"
			return 1
		}
		routed -q 2>&1 > /dev/null
	}

	MountMedia || return 1

	INSTALLOSF=/kit/isl/install.osf
	HINSTALLOSF=/kit/hisl/install.osf
	[ "$HINSTALL" ] &&
	{
		[ -f $HINSTALLOSF ] && INSTALLOSF=$HINSTALLOSF
	}

	log /install.log /sbin/sh  $INSTALLOSF ||
	{
		Error "$Main_INSTERR"
		return 1
	}

	Cleanup

	while :
	do
		CONFIG=
		if [ $ISL_ADVFLAG -eq 1 ]; then 
		{
			echo "$Main_CONFIGURE"
			read USER_INPUT
			CONFIG=`echo $USER_INPUT | dd conv=lcase 2> /dev/null`
		}
		else
                        CONFIG=$LC_Y
                fi

		case "$CONFIG" in
		$LC_Y)	echo "$Main_INITSTART"
			rm -f /tmp/showboot
			# find and stop routed process before fsck'ing root
			kill -9 `ps -o pid,comm -e | grep routed | grep -v grep  | awk ' { print $1 } ' `
			sync;sync;sync
			umount -A
			sleep 5
			init 3
			exit 0
			;;
		$LC_N)	umount -A
			cat /tmp/showboot
			rm -f /tmp/showboot
			sync;sync;sync
			halt
			;;
		esac
	done
	return 0	# cosmetic only
}



:	-MountMedia
#
#		mount the distribution media on /kit
#

MountMedia()
{
	if [ -d /kit ]; then
	{
		umount /kit > /dev/null 2>&1
	}
	else
	{
		mkdir /kit

	}; fi


	case $TLOAD in
	REMOTE)
		[ "$RISDIR" ] || RISDIR=`GetRisDir` || return 1

		mount $NLOAD:$RISDIR/kit /kit ||
		{
			Error "Cannot mount $NLOAD:$RISDIR/kit on /kit"
			return 1
		}
		;;

	CDROM)
		(cd /dev; MAKEDEV $NLOAD$ULOAD 2>&1 > /dev/null)
		mount -dr /dev/$BLOAD /kit ||
		{
			Error "Cannot mount $TLOAD $BLOAD on /kit\n$ERMSG1"
			return 1
		}
		;;
	esac
	return 0
}

[ "$CHECK_SYNTAX" ] || Main "$@"

