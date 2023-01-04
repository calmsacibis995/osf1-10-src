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
# 
# %W%	(DEC/OSF)	%G%
#
########################################################################
# First, check to make sure the user is root, unless it's an
# installation.
[ "$ISL_ADVFLAG" ] ||
{
	case `whoami` in
	root ) ;;
	* )
		echo "You must have root privileges to run doconfig."
		exit 1
		;;
	esac
}

########################################################################
# DECLARATION SECTION ##################################################
########################################################################
PROG=$0				# this program name
PWD=`/bin/pwd`			# get present dir
C_FLAG=0			# -c flag: use existing config file (def=0)
D_FLAG=0			# -d flag: only make new devices (def=0)
E_FLAG=0			# -e flag: use file to edit config file (def=0)
I_FLAG=1			# default flag: will build new kernel
ROOTPATH=/			# Rootpath is root (/)
TMP_PATH=/tmp			# temp directory is /tmp
KERNELPATH=/sys			# kernel location
CONFIGPATH=/sys/conf		# config file location
EDITOR=${EDITOR:-ed}		# run environment editor or `ed` by default
MACHINE=`/bin/machine`		# machine architecture type
RC_FILE=/etc/rc.config		# rc config default file
OPTION_FILE=/tmp/.config	# tmp file that has k_options in it
TMP_CONF_FILE=xxxxxx		# config file name for making devices only
LP_DATA_FILE=/sys/conf/.product.list	# file that holds lp kernel info
REBOOT_FILE=/sbin/it.d/data/reboot	# file that tells system to reboot
KERN_OPT_PROGRAM="/usr/sbin/kopt"	# kernel options program

PATH=/dev:/sys/conf:/sbin:/usr/sbin:/usr/bin:/usr/lbin
export PATH
export MACHINE
umask 022
trap 15

[ -f $RC_FILE ] && . $RC_FILE

SHELL_LIB=${SHELL_LIB:-/usr/share/lib/shell}
. $SHELL_LIB/Ticker
. $SHELL_LIB/Wait

###############################
# BANNER HEADERS DEFINED HERE #
###############################
MAIN_BANNER="*** KERNEL CONFIGURATION AND BUILD PROCEDURE ***"

AUTO_EDIT_BANNER="*** PERFORMING AUTO-EDIT OF CONFIGURATION FILE ***"

KERN_BUILD_BANNER="*** PERFORMING KERNEL BUILD ***"

KERN_RETRY_BANNER="*** Retrying Kernel Build ***"

###############################
# ERROR MESSAGES DEFINED HERE #
###############################
KERN_ERR_1_MSG="
*** NOTE ***
The customized kernel for this machine could not be successfully
created.  This system will operate with the generic kernel until
a  customized  kernel  can  be built for it using the 'doconfig'
utility."

KERN_ERR_2_MSG="
*** NOTE ***
A new kernel for this machine could not be successfully created."

CD_ERROR_MSG="
'\$PROG': Could not change directory to '\$KERNELPATH/\$CONFIGNAME'"

CONF_ERROR_MSG="
*** WARNING ***
An error has occurred during system configuration.  A partial listing
of the error log file (./errs) follows: \n"

DEV_ERR_MSG="
*** ERROR ***
Could not make special device files!!"

CF_ED_ERR_MSG="
Are you satisfied with the changes made during the editing
session? (y/n) [y]: \c"

##############################
# MISC MESSAGES DEFINED HERE #
##############################
USAGE="
Usage: $0
	[-d]			Make devices only.
	[-c config_file]	Build kernel using specified config file.
	[-e ed_script]		Use specified file to edit config file before
				building the kernel.
"

CONF_SAVE_MSG="
Saving '\${CONFIGPATH}/\${CONFIGNAME}' as '\${CONFIGPATH}/\${CONFIGNAME}'.bck"

CONF_RSTR_MSG="
Restoring original configuration file - '\${CONFIGPATH}/\${CONFIGNAME}'"

KERN_LOCA_MSG="
The new kernel is '\${KERNELPATH}/\${CONFIGNAME}'/vmunix"

KERN_RETRY_MSG="
*** NOTE ***
The customized kernel for this machine could not be successfully
created.  One possible problem could be kernel layered products
that might be incompatible with the base operating system.  This
script will now automatically attempt to build a kernel using the
base operating system only."

DEV_LOG_MSG="
A log file listing special device files is located in /dev/MAKEDEV.log"

USER_INFO_MSG="


       The system will now automatically build a kernel
       and then reboot. This will take approximately 15
       minutes, depending on the processor type."

USER_INFO_MSG1="

       When  the login prompt appears after the system
       has rebooted, use 'root' as the  login name and
       the SUPERUSER  password that was entered during
       this procedure, to log into the system."

########################################################################
# SUB-ROUTINE SECTION ################################################## 
########################################################################

# hash subr references
set -h 

: Auto_Editor -
Auto_Editor()
{
	Auto_Edit_Routine
	case $? in
	1 )
		Exit 1
	esac
	return $?
}


: Auto_Edit_Routine -
Auto_Edit_Routine()
{
	echo "

$AUTO_EDIT_BANNER
Auto-editing $CONFIGPATH/$CONFIGNAME using ${EDITFILE}.....\c"
	cp $CONFIGPATH/$CONFIGNAME $TMP_PATH/$CONFIGNAME
	ed - $TMP_PATH/$CONFIGNAME <${EDITFILE} >${TMP_PATH}/ed$$.err 2>&1
	egrep -s "^\?" $TMP_PATH/ed$$.err && 
	{
		echo "
$PROG: Error: Command errors in the ed script used to edit
$CONFIGPATH/${CONFIGNAME}."
		cat $TMP_PATH/ed$$.err
		return 1
	}
	cp $TMP_PATH/$CONFIGNAME $CONFIGPATH/$CONFIGNAME
	echo "done."
	return 0
}


: Build_Kernel -
Build_Kernel()
{
	# If an update installation, the /sys/NAME/param.c file must be
	# replaced unconditionally.  Since it is user modifiable, it will
	# be saved.
	[ "$UPDFLAG" ] &&
	{
		[ -f ${KERNELPATH}/${CONFIGNAME}/param.c ] &&
			mv ${KERNELPATH}/${CONFIGNAME}/param.c ${KERNELPATH}/${CONFIGNAME}/param.c.PreUPD
	}

	STAT=
    	cd $CONFIGPATH
	config $CONFIGNAME 2>&1 > /tmp/config$$
	case $? in
	0 )
	    rm /tmp/config$$
	    if [ -d $KERNELPATH/$CONFIGNAME ]
	    then
		cd $KERNELPATH/$CONFIGNAME
		Ticker on
		make clean 2>&1 > /dev/null 
		if (make depend vmunix 2>&1) > errs
		then
			Ticker off
			STAT=0 
		else
			Ticker off
			echo "$CONF_ERROR_MSG"
			tail -20 errs
			echo "Press <RETURN> to continue:\c"
			read ANS
			STAT=1
		fi
	    else
		STAT=2
	    fi
	    ;;
	* )
	    echo "\nConfiguration file has errors."
	    more /tmp/config$$
	    rm -f /tmp/config$$
	    STAT=3
	    ;;
	esac

	cd $PWD
	return $STAT
}


: Exit -
Exit()
{
	STAT=$1
	Ticker off
	[ $STAT -ne 0 -a -f $CONFIGPATH/${CONFIGNAME}.bck ] && Move_Configfile -r
	rm -f $OPTION_FILE 
	[ "$CONFIGNAME" ] &&
	{
		rm -f $TMP_PATH/$CONFIGNAME $TMP_PATH/$CONFIGNAME.devs
	}
	exit $STAT
}


: Get_Configname -
Get_Configname()
{
	[ "$HOSTNAME" ] || HOSTNAME=`/bin/hostname`
	[ "$HOSTNAME" ] &&
	{
		HOSTNAME=`expr "$HOSTNAME" : '\([a-zA-Z0-9]*\)'`
		HOSTNAME=`echo "$HOSTNAME" | dd conv=ucase 2>/dev/null`
		[ "$ISL_ADVFLAG" ] &&
		{
			CONFIGNAME=$HOSTNAME
			return 0
		}
	}

	while :
	do
		while :
		do
			echo "
Enter a name for the kernel configuration file. [${HOSTNAME}]: \c"
			read CONFIGNAME
			CONFIGNAME=`echo $CONFIGNAME`
			if [ "$CONFIGNAME" ]
			then
				break
			else
				[ "$HOSTNAME" ] &&
				{
					CONFIGNAME=$HOSTNAME
					break
				}
			fi
		done

		CONFIGNAME=`echo $CONFIGNAME | dd conv=ucase 2>/dev/null`

		if [ -f $CONFIGPATH/$CONFIGNAME ]
		then
			while :
			do
	 			echo "
A configuration file with the name '$CONFIGNAME' already exists.
Do you want to replace it? (y/n) [n]: \c"
	 			read ANS
				ANS=`echo $ANS`
	 			case $ANS in
				[yY]* )
					break 2
					;;
				"" | [nN]* )
					break
					;;
	 			esac
			done
		else
			while :
			do
				echo "
You want to name the configuration file '$CONFIGNAME'
Is that correct? (y/n) [y]: \c"
				read ANS
				ANS=`echo $ANS`
				case $ANS in
				"" | [yY]* )
					break 2
					;;
				[nN]* )
					break
					;;
				esac
			done
		fi
	done
}


: Makedev_Routine -
Makedev_Routine()
{
	MAKE=$1

	if [ -f $TMP_PATH/${MAKE}.devs ] 
	then
		cd /dev
		rm -f /dev/*mt*
		Ticker on
		(MAKEDEV std) >> MAKEDEV.log
		(sh -v $TMP_PATH/${MAKE}.devs 2>&1) >> MAKEDEV.log
		case `sizer -wu` in
		0 ) ;;
		* ) (MAKEDEV xcons pm0 2>&1) >> MAKEDEV.log ;;
		esac
		rm -f $TMP_PATH/${MAKE}*
		Ticker off
		cd $PWD
		return 0
	fi
	echo "$DEV_ERR_MSG"
	Exit 1
}


: Manual_Editor -
Manual_Editor()
{
	Manual_Edit_Routine
	case $? in
	9 )
		if [ "$ISL_ADVFLAG" ]
		then
			echo "$KERN_ERR_1_MSG"
		else
			echo "$KERN_ERR_2_MSG"
		fi
		Exit 1
		;;
	esac
}


: Manual_Edit_Routine -
Manual_Edit_Routine()
{
	echo "\nDo you want to edit the configuration file? (y/n) [n]: \c"
	read resp
	resp=`echo $resp`
	case $resp in
	"" | [nN]*)
		return 9
		;;
	*)
		cp $CONFIGPATH/$CONFIGNAME $TMP_PATH/$CONFIGNAME
		while : true
		do
			echo "
Using $EDITOR to edit the configuration file.  Press return when ready,
or type 'quit' to skip the editing session: \c"
			read resp
			resp=`echo $resp`
			case $resp in
			"" )
				if $EDITOR /tmp/$CONFIGNAME
				then
					cp $TMP_PATH/$CONFIGNAME $CONFIGPATH/$CONFIGNAME
				else
					echo "$CF_ED_ERR_MSG"
					read resp
					case $resp in
					[nN]*) continue ;;
					esac
				fi
				break
				;;
			quit )	return 9 ;;
			* )	continue ;;
			esac
		done
		;;
	esac
	return 0
}


: Move_Configfile -
Move_Configfile()
{
	case $1 in
	-s )
		eval echo "'$CONF_SAVE_MSG'"
		cp $CONFIGPATH/$CONFIGNAME $CONFIGPATH/${CONFIGNAME}.bck
		;;
	-r )
		eval echo "'$CONF_RSTR_MSG'"
		mv $CONFIGPATH/${CONFIGNAME}.bck $CONFIGPATH/$CONFIGNAME
		;;
	esac
}


: Sizer_Routine -
Sizer_Routine()
{
	case $ISL_ADVFLAG in
	0 ) $KERN_OPT_PROGRAM -m ;;	# get mandatory kernel options only
	4 ) $KERN_OPT_PROGRAM -a ;;	# get ALL options for FIS
	* ) $KERN_OPT_PROGRAM		# get all kernel options
	esac
	sizer -n "$1" -t "0 dst 0" 1>/dev/null	# use gmt as default TZ
	cp $TMP_PATH/$CONFIGNAME $CONFIGPATH/$CONFIGNAME
}


########################################################################
# PROCEDURE SECTION ####################################################
########################################################################

# sort through command line parameters
case "$ISL_ADVFLAG" in
0 | 4 ) ;;
* )	 echo "\n$MAIN_BANNER" ;;
esac

while [ $# -gt 0 ]
do
	case $1 in
	-c )
		[ -f $CONFIGPATH/$2 ] ||
		{
			echo "$PROG: $CONFIGPATH/$2 - No such file."
			exit 1
		} 
		CONFIGNAME=$2
		C_FLAG=1
		I_FLAG=0
		shift 2
		;;
	-d )
		D_FLAG=1
		I_FLAG=0
		shift
		;;
	-e )
		[ -f $2 ] ||
		{
			echo "\n$PROG: $2 - No such file!"
			exit 1
		}
		EDITFILE=$2
		E_FLAG=1
		shift 2
		;;
	* )
		echo "$PROG: '$1'- Unknown switch parameter"
		echo "$USAGE"
		exit 1
		;;
	esac
done


#
# Iterations of the configuration process start here.
#
trap '
while : true
do
        echo "\nDo you want to quit (y/n) [y]: \c"
        read ans
        ans=`echo $ans`
        case $ans in
        [yY] | "" )
                trap '' 1 2 3
                Ticker off
		wait
                Exit 1
                ;;
        [Nn] )
                break
                ;;
        esac
done ' 1 2 3


#
# If C_FLAG is set, then take care of building a kernel with an
# already existing configuration file.
#
case $C_FLAG in
1 )
	# Save original configfile
	[ -f $CONFIGPATH/$CONFIGNAME ] && Move_Configfile -s

	# if -e used on command line, auto edit the config file.
	if [ "$E_FLAG" = 1 ]
	then
		Auto_Edit_Routine
	else
		Manual_Edit_Routine
	fi

	# Kernel build starts here.
	echo "\n\n$KERN_BUILD_BANNER"
	[ -f $LP_DATA_FILE  -a ! -f $CONFIGPATH/$CONFIGNAME.list ] && 
		cp $LP_DATA_FILE $CONFIGPATH/$CONFIGNAME.list
	while :
	do
		Build_Kernel $CONFIGNAME
		case $? in
		0 )
			eval echo "'$KERN_LOCA_MSG'"
			Exit 0
			;;
		1 )
			if [ -f $CONFIGPATH/$CONFIGNAME.list ]
			then
				echo "$KERN_RETRY_MSG"
				while :
				do
					echo "Is this ok? (y/n) [y]: \c"
					read RESP
					RESP=`echo $RESP`
					case $RESP in
					[Yy]* | "" )    
						rm -f $CONFIGPATH/$CONFIGNAME.list
						break
						;;
					[Nn]* )
						echo "$KERN_ERR_2_MSG"
						Exit 1
					esac
				done
			else
				Manual_Editor
			fi
			;;
		2 )
			Exit 2
			;;
		3 )
			Manual_Editor
			;;
		esac
		echo "\n$KERN_RETRY_BANNER"
	done
	;;
esac
	

#
# If D_FLAG is set, then it means they only want to build their devices.
#
case $D_FLAG in
1 )
	# Make devices using a phoney config file name
	sizer -n $TMP_CONF_FILE
	Makedev_Routine $TMP_CONF_FILE
	Exit 0
	;;
esac


#
# This is the default action when no command line switches are present,
# and the default for installation purposes.
# 
case $I_FLAG in
1)
	# Get name of config file
	Get_Configname

	# Save original config file
	[ -f $CONFIGPATH/$CONFIGNAME ] && Move_Configfile -s

	# If present, copy .product.list to NAME.list - this must be done
	# before invoking sizer, which now scans the NAME.list file
	[ -f $LP_DATA_FILE ] && cp $LP_DATA_FILE $CONFIGPATH/$CONFIGNAME.list

	# run sizer to create system config file
	Sizer_Routine $CONFIGNAME

	# User gets a chance to edit the config file, except in the
	# case of BASIC installations.
	case $ISL_ADVFLAG in
	"" | 1 | 3 )
		# If the -e option was used on the command line, use
		# auto-edit.  Otherwise, use manual edit.
		case $E_FLAG in
		1 )
                        Auto_Edit_Routine
                        case $? in
                        1 )
                                Manual_Edit_Routine
			esac
			;;
		* )
			Manual_Edit_Routine
			;;
		esac
		;;
	esac

	####
	# Start kernel building process here.
	####
	[ "$ISL_ADVFLAG" ] && 
	{
		echo "$USER_INFO_MSG"
		[ "$UPDFLAG" ] || echo "$USER_INFO_MSG1"
	}
	echo "\n\n$KERN_BUILD_BANNER"

	#
	# Make devices found by sizer.  If this is not an installation,
	# give the user the location of the makedev log file.
	#
	Makedev_Routine $CONFIGNAME
	[ "$ISL_ADVFLAG" ] || echo "$DEV_LOG_MSG"

	while :
	do
	    Build_Kernel $CONFIGNAME
	    case $? in
	    0 )
		if [ "$ISL_ADVFLAG" ]
		then
		    cp /vmunix /genvmunix
		    mv ${KERNELPATH}/${CONFIGNAME}/vmunix /vmunix
		    >$REBOOT_FILE
		else
		    eval echo "'$KERN_LOCA_MSG'"
		fi
		break
		;;
	    1 )
		if [ -f $CONFIGPATH/$CONFIGNAME.list ]
		then
			echo "$KERN_RETRY_MSG"
			[ "$ISL_ADVFLAG" ] ||
			{
				while :
				do
					echo "Is this ok? (y/n) [y]: \c"
					read RESP
					RESP=`echo $RESP`
					case $RESP in
					[Yy]* | "" )    
						rm -f $CONFIGPATH/$CONFIGNAME.list
						break
						;;
					[Nn]* ) 
						echo "$KERN_ERR_2_MSG"
						break 2
					esac
				done
			}
			Manual_Edit_Routine
		else
			case $ISL_ADVFLAG in
			0 | 2 )
		    		echo "$KERN_ERR_1_MSG"
		    		break
			esac
	    		Manual_Editor
		fi
		;;
	    2 )
		eval echo "'$CD_ERROR_MSG'"
		Exit 2
		;;
	    esac
	    echo "\n$KERN_RETRY_BANNER"
	done
	;;
esac

Exit 0
