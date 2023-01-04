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
# @(#)$RCSfile: updeng.sh,v $ $Revision: 1.1.2.26 $ (DEC) $Date: 1994/01/20 18:16:44 $
# 
############################################################################

:	ADVfileMove 
#
#	Given:	$1 - PREMOVE, POSTMOVE
#
#	Does:	Moves configured state kernel files that have not been
#		merged yet out of the way, and copies in .proto.. state
#		files so that the kernel build can occur.
#
#		In POSTMOVE phase, the original files are moved back into
#		into place.
#
#	Return: Nothing
#
ADVfileMove()
{
	case $1 in
	PREMOVE )
		mv /sys/conf/param.c		/sys/conf/param.c.preADV
		mv /sys/conf/files		/sys/conf/files.preADV
		mv /sys/conf/alpha/files 	/sys/conf/alpha/files.preADV
		cp /sys/conf/.proto..param.c	/sys/conf/param.c
		cp /sys/conf/.proto..files	/sys/conf/files
		cp /sys/conf/alpha/.proto..files  /sys/conf/alpha/files
		cp /sys/io/common/.proto..conf.c  /sys/io/common/conf.c
		cp /sys/streams/.proto..str_config.c  /sys/streams/str_config.c
		;;
	POSTMOVE )
		mv /sys/conf/param.c.preADV /sys/conf/param.c
		mv /sys/conf/files.preADV /sys/conf/files
		mv /sys/conf/alpha/files.preADV /sys/conf/alpha/files
		rm /sys/io/common/conf.c /sys/streams/str_config.c
		;;
	esac
}


: 	ADVfsCheck
#
#	Given:	Nothing
#
#	Does:	Checks to see if any advfs file systems are mounted on the
#		system.  If yes, it builds a generic kernel with the ADVFS
#		option and then moves the new kernel into place.
#
#	NOTE:	This fix is needed for STERLING because advfs does not work
#		on /usr filesystems.  This should go away in the gold time-
#		frame.  When ADVFS if fully incorporated into the base
#		product, this entire function can be removed.
#
#	Return: 
#		0 - System does not have ADVFS in use on it.
#
#		1 - System has ADVFS in use, and an ADVFS kernel was
#		    built successfully.
#
#		2 - System has ADVFS in use, but the ADVFS kernel build
#		    had errors.
#
ADVfsCheck()
{
	#
	# Return status.  Default is 0.
	#
	STAT=0

	#
	# if advfs is used on the system, we need to build a generic
	# kernel with the ADVFS option and then move the kernel into
	# place.
	#
	[ -f /usr/.smdb./OSFADVFSBIN*.lk ] &&
	{
		CONFDIR=/usr/sys/conf
		LOGFILE=/sys/conf/GENERIC.advfs.log

		#
		# Copy the generic config file to GENERIC.advfs,
		# add in the ADVFS option, and create a yes/no
		# answer file for doconfig questions.
		#
		cp $CONFDIR/GENERIC $CONFDIR/GENERIC.advfs
		echo "options\t\tMSFS" >> $CONFDIR/GENERIC.advfs
		echo "\n\n\n\n\n\n" >/tmp/d_ans

		#
		# Force kernel .proto.. files into configured state
		# for purposes of this kernel build.
		#
		ADVfileMove PREMOVE

		#
		# Now build the kernel.
		#
		echo "
This system is using Advanced File Systems and requires that a generic
kernel be built before continuing with the  update  installation.  The
kernel build will not require any user intervention,  and depending on
the processor type, may take up to 15 minutes to complete.

Building ADVFS generic kernel..."
		Ticker on
		if /usr/sbin/doconfig -c GENERIC.advfs </tmp/d_ans 1>$LOGFILE
		then
			Ticker off
			mv /sys/GENERIC.advfs/vmunix /genvmunix
			echo "Finished building ADVFS generic kernel."
		else
			Ticker off
			Error "
Doconfig_ERROR: Could not build an ADVFS generic kernel.

Check the log file /sys/conf/GENERIC.advfs.log for build and error
information.  After system configuration, this system will boot on
the /vmunix kernel that was in place PRIOR to the system update."
			STAT=1
		fi

		#
		# Move kernel files back to original state, and do
		# general cleanup.
		#
		ADVfileMove POSTMOVE
		rm -rf /sys/GENERIC.advfs
		rm -f /tmp/d_ans
	}

	#
	# if ADVfs not used on this system, return 0 so that the normal
	# generic kernel will be moved into place.
	#
	return $STAT
}


: ADVgate -
#
#	Given:	Nothing
#
#	Does:	This is all special code to accomodate ADVFS for Sterling.
#		When ADVFS is finally incorporated completely into the base
#		product, this entire function should be deleted.
#
#		This function calls ADVfsCheck, and based on the return
#		status, performs system setup functions.  The two major
#		things we are concerned about here, is whether to do a
#		doconfig during it(8), and whether to copy /genvmunix to
#		/vmunix.
#
#		A return of 2 indicates that the system has ADVfs in use,
#		but that an ADVFS generic kernel could not be built.  In
#		this case, we want to setup c_install only.  Hopefully,
#		they will be able to boot on their V1.3 kernel and work
#		out any errors that have occured.
#
#	Return: Nothing
#
ADVgate()
{
	ADVfsCheck
	if [ "$?" = 1 ]
	then
		#
		# For some reason, we couldn't build a generic advfs
		# kernel.  So, only setup c_install to be run by it.
		#
		# We don't run doconfig.
		#
		(cd /; itruns c_install in 23)
	else
		#
		# setup c_install and doconfig in the proper run state
		# directory, then copy genvmunix to vmunix.
		#
		(cd /; itruns c_install doconfig in 23)
		cp /genvmunix /vmunix
	fi
}


: 	ADVssCheck
#
#	Given:	Nothing
#
#	Does:	Fools update install into believing that the correct
#		subset names for ADVfs have been installed.
#
#	NOTE:	This fix is needed for STERLING because subset names changed
#		between MAINT130 and STERLING200, for ADVfs.  If a customer
#		has ADVfs installed on their 130 machine, it won't get up-
#		dated unless this code is executed.  Depending on whether
#		subsequent updates occur from 130 or 200, this code will
#		either need to be modified or deleted.
#
#	Return: Nothing.
#
ADVssCheck()
{
	[ -f /usr/.smdb./AFABASIC1*.lk ] &&
	{
		for K in OSFADVFS130.lk \
			 OSFADVFS130.inv \
			 OSFADVFS130.ctrl \
			 OSFADVFSBIN130.lk \
			 OSFADVFSBIN130.inv \
			 OSFADVFSBIN130.ctrl
		do
			touch /usr/.smdb./$K
		done
	}
}


:	CheckCtrlFiles
#
#	given:	a directory path
#
#	does:	changes to the specified directory and reads in .ctrl
#		files to determine the value of FLAGS variable within
#		each .ctrl file for a particular subset.  If the value
#		of the FLAGS variable is 1, it means the subset is man-
#		datory, and we add that subset to our list of subsets
#		to be installed. (DATALIST).  If the value of the FLAGS
#		variable is 0, it means the subset is optional, and in
#		this case we add the subset to our list of subsets to
#		be installed ONLY if a previous version of the subset
#		has been installed.  
#
#	return: nothing
#
CheckCtrlFiles()
{(
	SMDB=/usr/.smdb.
	ACT=C; CMDSW=l; L_DIR="."; ADVFLAG=1
	export ACT CMDSW L_DIR ADVFLAG

	[ -f $1/*.ctrl ] &&
	{
		cd $1
		for FILE in `ls *.ctrl`
		do
			#
			# Filter out non-OSF subsets
			#
			[ "`expr $FILE : '\(...\)'`" = "OSF" ] &&
			{
				L_SUB=`echo $FILE | sed 's/.ctrl//'`
				W_SUB=`expr "$L_SUB" : '\(.*\)...'`
				export L_SUB
				. ./$FILE
				BitTest $FLAGS 1
				case $? in
				0 )
				    [ -f $SMDB/${W_SUB}[0-9][0-9][0-9].lk ] &&
						SSLIST="${SSLIST}${L_SUB},"
					;;
				1 )
				    SSLIST="${SSLIST}${L_SUB},"
					;;
				esac
			}
		done

		[ "$SSLIST" ] &&
		{
			echo $SSLIST
			return 0
		}
	}
	return 1
)}


: CheckSystemMode -
#
#	Given:	Nothing
#
#	Does:	Blocks execution of this program by systems in multi-user
#		mode.  Single-user mode is required.
#
#	Return: Nothing
#
CheckSystemMode()
{
	#
	# who -r tells us the run-level.  set the output, and
	# then shift through until we find the key-word "run-level"
	#
	set -- `who -r`

	while [ "$1" -a "$1" != "run-level" ]
	do
		shift
	done

	#
	# This is single user mode, or else we exit immediately.
	# If they go to single, from multi, we'll find an uppercase
	# "S".  If they boot to single, we'll find nothing.  We
	# have to test for both cases.
	#
	[ "$2" = "S" -o "$2" = "" ] ||
	{
		echo "\n*** WARNING:  Incorrect system state \c"
		echo "detected. ***"
		echo "Please shut down system to single user \c"
		echo "mode before"
		echo "attempting an update installation."
		exit 1
	}
}


: ContinueInstall -
#
#	Given:	Nothing
#
#	Does:	Generic "do you want to continue" question.
#
#	Return: 0 - Yes, continue
#		1 - No, Stop.
#
ContinueInstall()
{
    while :
    do
	echo "\nDo you want to continue the installation? (y/n) [n]: \c"
	read ANSWER
	ANSWER=`echo $ANSWER`
	case $ANSWER in
	[Yy]* )		return 0 ;;
	[Nn]* | "" )	return 1 ;;
	esac
    done
}


: CustomFileBackup -
#
#	Given:	$1 (save or restore) argument
#
#	Does:	save -
#			Saves files listed in CUSTOM_FILE to file.PreUPD
#			before attempting update of the system.
#		restore -
#			Removes the saved copies of the files.
#
#	Return: 0 - No Error
#		1 - Error
#
CustomFileBackup()
{
	cd /
	STAT=0
	OKTOCOPY=

	#
	# Check to see if the file exists.
	#
	[ -f $CUSTOM_FILE ] &&
	{
	    case $1 in
	    save)
		#
		# Copy each file listed in CUSTOM_FILE to
		# filename.PreUPD.
		#
		for K in `grep -v "=" $CUSTOM_FILE`
		do
		    #
		    # If the file exists, attempt a copy
		    #
		    [ -f $K ] &&
		    {
			#
			# If a 'filename.PreUPD' already exists, then we
			# won't overwrite it unless we get specific
			# permission to do so.
			#
			[ -f $K.PreUPD ] &&
			{
				[ "$OKTOCOPY" ] ||
				{
					#
					# Ask if we can overwrite the
					# existing .PreUPD file.
					#
					Ticker off
					if GetCopyPermission
					then
						OKTOCOPY=1
						Ticker on
					else
						RestoreSystem
					fi
				}
			}

			#
			# Do the copy.  If we fail on any one copy, we fail
			# completely and make the user interact with us on
			# the return.
			#
			cp -p $K $K.PreUPD ||
			{
			    STAT=1
			    break
			}
		    }
		done
		;;
	    restore)
		#
		# Delete any file listed in CUSTOM_FILE that
		# had previously been save to filename.PreUPD
		#
		for K in `grep -v "=" $CUSTOM_FILE`
		do
		    rm -f $K.PreUPD
		done
		;;
	    esac
	}

	cd $PWD
	return $STAT
}


: CustomFileNotice -
#
#	Given:	Nothing
#
#	Does:	Informs the user that they have customized files on the
#		system, and if CFUI is not set, we rename the files listed
#		in the customized log file.
#
#	Return: Nothing
#
CustomFileNotice()
{
	#
	# If CFUI is set, it means that we couldn't get through
	# fitset with backup copies of their customized system
	# files.  We'll just tell them where they can find a log
	# listing the customized system files.
	#
	# If CFUI is not set, we were able to get through fitset
	# with backup copies of customized system files.  Since
	# the backup copies are renamed to 'filename.PreUPD', we
	# need to call EditCFL so that the log file will reflect
	# the new filename.  Then we can give them messages that
	# tell them where to find the log file.
	#
	if [ "$CFUI" ]
	then
		echo "


A listing of unprotected customized system files
found by the Update Installation has been logged
in $CUSTOM_FILE."
	else
		#
		# Change the names of the files listed in the log.
		#
		EditCFL
		echo "


Unprotected customized system files have been found on
this system and have been saved to  'filename.PreUPD'.
A listing of the files has been logged in
$CUSTOM_FILE."
	fi

	echo "
After the update installation has completed,  use the
Update  Administration  Utility  (/usr/sbin/updadmin)
to perform system administration tasks on these files."
}


: CustomFileSpace -
#
#	Given:	Nothing
#
#	Does:	Adds the sizes of all customized files found on the system
#		and uses the data to determine if there is enough room to
#		possibly finish the installation.
#
#	Return: 0 - Can install
#		1 - Can't install
#
CustomFileSpace()
{
	#
	# Check for custom file.  If there is nothing in the file,
	# then there is no space to be gained, so we'll return 1.
	#
	[ -s $CUSTOM_FILE ] &&
	{
		#
		# Custom File Data Space starts off set to 0
		#
		CFDS=0
		cd /
		for K in `grep -v "=" $CUSTOM_FILE`
		do
			#
			# For each file found in the custom file list,
			# make sure it exists, then 'ls' it and get it's
			# size.  Add the size to CFDS till we get the
			# total amount of space we can buy back.
			#
			[ -f $K ] &&
			{
				set -- `cd /; ls -s $K`
				CFDS=`expr $CFDS + $1`
			}
		done
		cd $PWD

		#
		# Now get the disk space requirements defined by fitset.
		#
		for K in `cat /tmp/fitset.msg | grep Kbytes`
		do
			case $K in
			[0-9]* )
				#
				# if the total disk space we can get back
				# is greater than the disk space required
				# by fitset, we can continue with the
				# installation.  Otherwise, we exit now.
				#
				if [ "$CFDS" -gt "$K" ]
				then
					return 0
				else
					break
				fi
				;;
			esac
		done
	}
	return 1
}


: DemarcateMessage -
#
#	Given:	Nothing
#
#	Does:	Makes the fitset error message more presentable.
#
#	Return: Nothing
#
DemarcateMessage()
{
	FSEM=/tmp/fitset.msg
	TFSEM=/tmp/TFSEM

	echo "

-----------------------------------------------------------------" > $TFSEM
	egrep -v fitset $FSEM >> $TFSEM
	echo "
-----------------------------------------------------------------" >> $TFSEM

	mv $TFSEM $FSEM
}


DisplayStartupMessages()
{
	MASTERMSG=/tmp/updmm
	BIGBORDER="----------------------------------------------------------------------"
	SMALLBORDER="\t-------------------------------------"

	#
	# Echo out some startup messages about doing backups, etc.
	#
	echo "$STARTUP_MSG" > $MASTERMSG
	echo "Digital Equipment Corporation recommends \c" >> $MASTERMSG
	echo "that you perform complete" >> $MASTERMSG
	echo "system software backups before proceeding." >> $MASTERMSG

	#
	# Now echo layered product messages
	#
	[ -s $LPWARNMSG -o -s $LPBLOCKMSG ] &&
	{
		echo "\n$BIGBORDER" >> $MASTERMSG
		[ -s $LPWARNMSG ] && cat $LPWARNMSG >> $MASTERMSG
		[ -s $LPBLOCKMSG ] &&
		{
			[ -s $LPWARNMSG ] &&
				echo "\n$SMALLBORDER" >> $MASTERMSG
			cat $LPBLOCKMSG >> $MASTERMSG
		}
		echo "\n$BIGBORDER" >> $MASTERMSG
	}

	while :
	do
		if [ -f "$UPDMORE" ]
		then
			$UPDMORE $MASTERMSG
		else
			cat $MASTERMSG
		fi

		echo "\nPress <RETURN> to review message again."

		if [ -s $LPBLOCKMSG ]
		then
			echo "Enter any character and press <RETURN> to \c"
			echo "stop: \c"
			read ans
			ans=`echo $ans`
			case "$ans" in
			"" )	;;
			*  )	Exit 1 ;;
			esac
		else
			while :
			do
				echo "Do you want to continue the update \c"
				echo "installation? (y/n) []: \c"
				read ans
				ans=`echo $ans`
				case "$ans" in
				"" )	break ;;
				[Yy]* ) break 2 ;;
				[Nn]* ) Exit 1 ;;
				esac
			done
		fi
	done
}


: EditCFL -
#
#	Given:	Nothing
#
#	Does:	Edits CUSTOM_FILE and replaces the filename with
#		filename.PreUPD.
#
#	Return: Nothing
#
EditCFL()
{
	>/tmp/cfl

	cd /

	#
	# We want to put a .PreUPD extension on each filename in
	# CUSTOM_FILE, but not disturb the comments, etc.
	#
	cat $CUSTOM_FILE | while read LINE
	do
		#
		# if the line read in is a filename, put the extension
		# on and write it out to the new file.  Otherwise, we
		# probably pulled in a comment, or blank line, so we
		# won't disturb it.
		#
		[ -f "$LINE" ] &&
		{
			#
			# if the line in the file already has the PreUPD
			# extension on it, we don't want to put another
			# one on.
			#
			case "$LINE" in
			*PreUPD )
				;;
			* )
				echo "${LINE}.PreUPD" >> /tmp/cfl
				continue
				;;
			esac
		}
		echo "$LINE" >> /tmp/cfl
	done

	#
	# Now copy the new file over the original file.
	#
	cd $PWD
	mv /tmp/cfl $CUSTOM_FILE
}


: Exit -
#
#	Given:	$1 - An exit status
#
#	Does:	Generic exit handler
#
#	Return: Nothing
#
Exit()
{
	ESTAT=$1
	Ticker off
	InflateFS deflate
	rm -rf $PIID
	rm -f /tmp/lpbm /tmp/lpwm /tmp/updmm
	rm -f /tmp/fitset.msg /tmp/fitseterror /tmp/all.inv
	exit $ESTAT
}


: FitsetError -
#
#	Given:	Nothing
#
#	Does:	Offers the user options for dealing with customized
#		system files.
#
#	Return: Nothing
#
FitsetError()
{
	#
	# Define the fitset message and error files
	#
	FSEM=/tmp/fitset.msg
	FSEF=/tmp/fitseterror

        if [ "$UPDFLAG" ]
        then
                if [ -f $UPDMORE ]
                then
                        VCMD=$UPDMORE
                else
                        VCMD=cat
                fi
        else
                VCMD=more
        fi

	cat $FSEM > $FSEF
	echo "The $OSFVER Update Installation has used
$CFDS Kbytes of disk space to save unprotected customized system
files on your system. 

Unprotected customized system files are typically  DEC OSF/1 system
files that have been customized by users for projects that they may
be working on, or by layered product implementation.  The files are
unprotected because they are in danger of being overwritten without
the customizations within them being saved.

If you have already backed up your entire system, you  can continue
with the  update  installation  and  restore the  customized system
files at a later date.  

If you want to ensure that the unprotected customized system  files
on this system have been backed up,  you can select option 'u' from
the menu below and enter  the  Update Administration Utility.  This
utility allows you to view and save  unprotected  customized system
files on your system.  The utility will also  allow you to continue
or quit the update installation procedure.

If you are unsure about the action you should take, you should stop
the update installation at this point and read the 'DEC OSF/1 Guide
to Update Install'  for more information on  unprotected customized
system files and what action should be taken with them. If you stop
the update installation procedure at this point,  the  system  will
return to the same  state  it  was  in prior to invoking the update
installation." >> $FSEF

	$VCMD $FSEF

	while :
	do
		echo "


	DEC OSF/1 Update Installation Main Menu
	----------------------------------------
	c) Continue installation
	q) Quit installation
	u) Update Administration Utility
	v) View error message again

	Enter your choice: \c"
		read ANS
		ANS=`echo $ANS`
		case $ANS in
		"" )	;;
		c)
			echo "\nContinuing with installation..."
			return 0
			;;
		q)
			return 1
			;;
		u)
			$UPDADMIN custom || return 1
			;;
		v)	$VCMD $FSEF ;;
		esac
	done
}


: FitsetSystem -
#
#	Given:	$1 - A datalist of paths and products to install.
#
#	Does:	Checks to see if the subsets will fit on the system.
#
#	Return: 0 - Continue the installation
#		1 - Stop the installation
#
FitsetSystem()
{
	STAT=0
	>/tmp/all.inv

	#
	# Add inflation files to ensure there is enough room for a
	# kernel build.  
	#
	if [ -f /usr/.smdb./AFABASIC1*.lk ]
	then
		InflateFS 21 || RestoreSystem
	else
		InflateFS 13 || RestoreSystem
	fi

	#
	# collect all inventory information into one place.
	#
	set -- `Parse : $1`
	for K in $*
	do
		set -- `Parse , $K`
		PRODAREA=$1
		shift
		for Z in $*
		do
			cat $PRODAREA/instctrl/$Z.inv >> /tmp/all.inv
		done
	done

	#
	# get rid of this directory...it's no longer needed and it
	# will just take up precious space on the file system.
	#
	[ -d $PIID ] && rm -rf $PIID

	#
	# Run fitset and analyze it's return status.
	#
	while :
	do
		> /tmp/fitset.msg

		Ticker on

		( cd /; fitset / < /tmp/all.inv 2>/tmp/fitset.msg )
		STAT=$?

		Ticker off

		[ -s /tmp/fitset.msg ] && DemarcateMessage

		case $STAT in
		1)
			cat /tmp/fitset.msg
			Error "\nUnrecoverable fitset error encountered."
			return 1
			;;
		2)
			[ "$CFUI" ] ||
			{
				CustomFileSpace &&
				{
					if FitsetError
					then
						CFUI=1
						CustomFileBackup restore
						continue
					else
						return 1
					fi
				}
			}
		
			cat /tmp/fitset.msg
			echo "
You will need to remove some software subsets and/or user files from
this system before performing an update installation."
			return 1
			;;
		3)
			cat /tmp/fitset.msg
			if ContinueInstall
			then
				echo "\nContinuing update installation..."
				return 0
			else
				return 1
			fi
			;;
		*)
			InflateFS deflate
			return 0
			;;
		esac
	done
}


: GetCopyPermission -
#
#	Given:	Nothing
#
#	Does:	Displays a message describing PreUPD file conflicts
#		and asks whether to continue with the installation.
#
#	Return: 0 - Yes, continue with the installation.
#		1 - No, Stop the installation.
#
GetCopyPermission()
{
	echo "

*** Files with '.PreUPD' extensions detected ***
-----------------------------------------------------------------

The Update Installation has detected files on this system with
'.PreUPD' file name extensions.  This could be the result of a
previous update installation on this system.   These files are
in danger of being overwritten  unless some action is taken to
save them.

If you have  already backed up  your system,  you can continue
with the installation by answering 'y' to the question below.

If you have  not  backed up your system,  or if you are unsure
about what action you should take,  answer 'n' to the question
below and the  system will return to the state it was in prior
to invoking the update installation.

-----------------------------------------------------------------"

	if ContinueInstall
	then
		echo "\nContinuing Update Installation..."
		return 0
	else
		return 1
	fi
}


: InflateFS -
#
#	Given:	$1 - Can be a number, or the string "deflate"
#
#	Does:	Inflates the /usr file system prior to running fitset,
#		to ensure there will be enough room to build a new kernel.
#
#	Return: 0 - success
#		1 - failure
#
InflateFS()
{
	IIF=$LOCMNT/isl/updeng		# Input Inflator File
	OIF=/usr/sys/updvmunix		# Output Inflation File

	if [ "$1" = "deflate" ]
	then
		rm -f ${OIF}*
	else
		FPX=0
		FPY=$1

		#
		# Now write out 1mb block files until we get the amount of
		# inflation padding we want.
		#
		while [ $FPX -lt $FPY ]
		do
			dd if=$IIF of=${OIF}.${FPX} bs=1024k conv=sync \
			>$NUL 2>&1 ||
			{
				Error "
----------------------------------------------------------------------
Error detected while copying fitset inflation files to /usr/sys, to
ensure adequate disk space for kernel building.

$FPY megabytes of file system padding required, but only able to
write approximately $FPX megabytes to /usr/sys file system.

This system cannot be updated until more /usr disk space is available,
and may require removal of some optional software subsets.
----------------------------------------------------------------------"

				return 1
			}
			FPX=`expr $FPX + 1`
		done
	fi

	return 0
}


: InstallSubsets -
#
#	Given:	$1 - A datalist of paths and subsets
#
#	Does:	Sorts the path from the subsets, and uses setld to
#		install the subsets.
#
#	Return:	exit status of setld
#
InstallSubsets()
{
	set -- `Parse : $1`
	for K in $*
	do
		set -- `Parse , $K`
		case $TLOAD in
		REMOTE )
			NLOAD="${SERVER}:"
			shift
			;;
		* )
			
			NLOAD=$1
			shift
			;;
		esac
			
		#
		# Run setld and check the error status when it's
		# finished.  Fatal errors return a 1, non-fatal
		# errors return greater than 1.  We are only concerned
		# with fatal errors, so if setld returns 1, we'll
		# return 1 from here and exit the procedure immediately.
		# If there were non-fatal errors, we'll return 0, and
		# just continue with the procedure.
		# 
		setld -D / -l $NLOAD $*
		if [ "$?" = 1 ]
		then
			return 1
		else
			return 0
		fi
	done
}


: KillProcesses -
#
#	Given:	Nothing
#
#	Does:	For ease of use, we have the customer simply type
#		'rcinet start' to get the network going.  Unfortunately,
#		rcinet starts many things that we don't want going during
#		the update install.  Until /sbin/installupdate is smart
#		enough to start what it needs automatically, we'll kill
#		off the processes we don't want around.
#
#	Return: Nothing
#
KillProcesses()
{
	ps ax > /tmp/psdata

	for K in rwhod nfsiod inetd snmpd automount \(sendmail\)
	do
		if [ "$K" = "automount" ]
		then
		    kill -TERM `grep $K /tmp/psdata | awk '{ print $1 }'` \
			>$NUL 2>&1
		else
		    kill -9 `grep $K /tmp/psdata | awk '{ print $1 }'` \
			>$NUL 2>&1
		fi
	done

	rm /tmp/psdata
}


: LogMotd -
#
#	Given:	Nothing
#
#	Does:	Puts a log entry in motd.
#
#	Return: Nothing
#
LogMotd()
{
	#
	# Define files to be accessed
	#
	MOTD_FILE=/etc/motd
	TEMP_FILE=/tmp/motd
	UWSVER_FILE=/usr/share/uwsvers

	#
	# PATTERN - Matching pattern for OS version string
	# NEWREV  - The OS version updated to.
	# OLDREV  - The OS version updated from.
	# UPDLOG  - The text written to the motd file
	#
	PATTERN="DEC OSF\(.*\)(Rev\(.*\))"
	NEWREV=`/usr/bin/strings -14 /genvmunix | grep -e "$PATTERN"`
	OLDREV=`grep -e "$PATTERN" $MOTD_FILE`
	UPDLOG="
****************************************************************************

The Update Installation software has successfully updated your system from:

`echo "$OLDREV" | \
	sed -e 's/^DEC/     DEC/g' \
	    -e '/(Rev/s/(/[/g' \
	    -e '/\[Rev\(.*\))/s/)/]/g'`

The following files contain a record of the Update Installation session.
        
        /var/adm/smlogs/it.log             - log for it(8) utility

        /var/adm/smlogs/update.log         - update installation log file

        /var/adm/smlogs/upd_custom_files   - log of unprotected customized
					     system files found during the
					     update installation.

        /var/adm/smlogs/upd_obsolete_files - log of obsolete system files
					     found during the update
					     installation.

****************************************************************************"

	#
	# If this is an update on top of an update of the same version,
	# AND an update has already occured on the system (update.log
	# exists), don't bother entering the log information again.
	#
	[ "`grep "$NEWREV" $MOTD_FILE`" -a "`grep "update.log" $MOTD_FILE`" ] ||
	{
		#
		# Put in latest revision information.
		#
		echo "$NEWREV" > $TEMP_FILE
		[ -f $UWSVER_FILE ] && $UWSVER_FILE >> $TEMP_FILE

		#
		# Add in update log file information.
		#
		echo "$UPDLOG \n\n\n" >> $TEMP_FILE

		#
		# The following line changes (Rev. xxx) to [Rev. xxx]
		# so that the information is not stripped by the motd
		# startup script during system reboot.
		#
		grep -v "$OLDREV" $MOTD_FILE >> $TEMP_FILE

		#
		# Now copy the temp file to the real motd file.
		#
		cp $TEMP_FILE $MOTD_FILE
		rm $TEMP_FILE
	}
}


: LPblocking -
#
#	Given:	Nothing
#
#	Does:	Executes $LOCMNT/isl/lpblocker that checks to make sure
#		the system can be updated without impacting Layered Pro-
#		ducts.  Pass the following arguments:
#		
#		-m :	Mount point of isl area
#		-w :	name of warning msg file
#		-b :	name of blocking msg file
#
#	Return: Nothing
#
LPblocking ()
{
	LPBLOCKER=$LOCMNT/isl/lpblocker
	LPBLOCKMSG=/tmp/lpbm
	LPWARNMSG=/tmp/lpwm

	[ -f $LPBLOCKER ] &&
		$LPBLOCKER -m $LOCMNT -w $LPWARNMSG -b $LPBLOCKMSG
}


: Main -
#
#	Given:	$1 - potential command line arguments.
#
#	Does:	Drives the rest of the program.
#
#	Return: Nothing
#
Main()
{
	echo "\tWorking....`date`"
	#
	# Make sure system is in single user mode.
	#
	CheckSystemMode

	#
	# Set up global variables and other environment stuff.
	#
	SetEnvironment

	#
	# Perform layered product testing.  Warn of any LP's that
	# might not work properly after an update.  Exit if any LP's
	# will render the system unusable after an update.
	#
	LPblocking

	#
	# echo out startup messages
	#
	DisplayStartupMessages

	#
	# Make a list of product areas and subsets that should be
	# updated on this system.
	#
	echo "\n\n\n******   Checking current state of system"
	echo "\nDepending on the system configuration, this may take"
	echo "up to 20 minutes..."
	Ticker on

	#
	# This call to ADVssCheck needs to be revisited after STERLING ssb.
	# See the ADVssCheck function for details.
	# 
	ADVssCheck

	#
	# rcinet starts many more things then we want to be executing
	# during this procedure.  Remove excess stuff from running.
	#
	KillProcesses

	#
	# Create list of subsets to be updated.
	#
	MakeDataList $TLOAD $PRODPATH "$PRODUCTS"
	[ "$DATALIST" ] ||
	{
		Error "\nNo installable subsets for this system found."
		Error "Exiting update installation procedure..."
		Exit 1
	}

	#
	# Shut ticker off.  Sifsync runs it's own ticker.
	#
	Ticker off

	#
	# Check for orphans and customized files on system.
	#
	RitKitSystem "$DATALIST"

	#
	# Turn Ticker back on.
	#
	Ticker on

	#
	# If repackaging of files has occured, then upd_obsolete_files
	# more than likely has bogus entries.  We need to remove those
	# entries, to have an accurate log file of obsolete system files.
	#
	RectifyObsoleteLogFile

	#
	# Save custom files to filename.PreUPD
	#
	CustomFileBackup save ||
	{
		CFUI=1
		Ticker off
		CustomFileBackup restore
		$UPDADMIN custom || Exit 1
	}

	#
	# Check if the subset list will fit on the system
	#
	FitsetSystem "$DATALIST" || RestoreSystem

	#
	# If we get to this point without CFUI being set, it means that
	# we were able to 'CustomFileBackup save' their custom files 
	# and get through fitset at the same time.  Now we need to append
	# .PreUPD to every file listed in CUSTOM_FILE.
	#
	[ -s $CUSTOM_FILE ] && CustomFileNotice
	
	#
	# Install the subsets via setld
	#
	echo "\n\n\n******   Updating system to ${OSFVER}"
	InstallSubsets "$DATALIST" || Exit $?

	#
	# Put log information in /etc/motd.
	#
	LogMotd

	#
	# remove old ADVfsinstctrl information
	# This can be removed in GOLD.
	#
	rm -f /usr/.smdb./AFABASIC1*

	#
	# Finish setting up the system for configuration
	#
	SetItUp
	echo "\n\nUpdate Installation complete."
	echo "rebooting system..."
	sleep 6
	Exit 0
}


: MakeDataList -
#
#	Given:	$1 - load device type
#		$2 - a product area
#		$3 - a product to load
#
#	Does:	Creates a list of subsets to update.
#
#	Return: Nothing
#
MakeDataList()
{
	LDEV=$1			# Load device	(CDROM NFS or RIS)
	PAREA=$2		# Product area	(ris0.alpha, /updmnt, etc)
	PRODS=$3		# Products 	(ALPHA/BASE, etc)

	case $LDEV in
	REMOTE )
		#
		# Always set to 1 for RIS updates
		#
		UPDFLAG=1

		#
		# Safety check, to make sure there are no remaining
		# instctrl files in this area.  Just in case the
		# directory was not removed from a previous iteration
		# of this script.
		#
		if [ -d $PIID/instctrl ]
		then
			(cd $PIID/instctrl; rm -rf *)
		else
			mkdir -p $PIID/instctrl || Exit 1
		fi

		#
		# For each product the client is registered for, copy
		# over the instctrl information.  We need to look at
		# this info to determine which subsets to update.
		#
		for P in $PRODS
		do
			INSTPATH="$PAREA/$P/instctrl"
			rsh $SERVER $RCMD "cd $INSTPATH; tar cf - ." \
				2>$NUL | (cd $PIID/instctrl; /sbin/tar xf -)
		done

		#
		# Pass off to CheckCtrlFiles and it will return us the
		# list of subsets to update.
		#
		DATALIST="${PIID},`CheckCtrlFiles $PIID/instctrl`"
		;;
	* )
		#
		# For CDROM, the starting value of UPDFLAG is 0. For
		# each product detected, the value is increased by 1.
		# This is for setld loading requirements.
		#
		UPDFLAG=0

		#
		# Check each product area on the cdrom, to see if there
		# are any subsets that we want to update.  Because setld
		# can only take one product argument at a time, we need
		# to embed ":" and "," in DATALIST so that we can tell
		# which subsets belong to which products.  We'll use
		# Parse later on to decode DATALIST.
		#
		for P in $PRODS
		do
			UPDFLAG=`expr $UPDFLAG + 1`
			CAREA=$PAREA/$P/instctrl
			PSUBS="`CheckCtrlFiles $CAREA`" &&
				DATALIST="${DATALIST}${PAREA}/${P},${PSUBS}:"
		done
		;;
	esac
	export UPDFLAG
}


: RectifyObsoleteLogFile -
#
#	Given:	Nothing
#
#	Does:	The ORDF file contains file names of system files known
#		to be non-obsolete.  The files are in the list are usually
#		re-packaged files that show up as obsolete.  (but we know
#		they really aren't).  This function removes those files
#		from the obsolete log file created by sifsync.
#
#	Return: Nothing.
#
RectifyObsoleteLogFile()
{
	[ -f $ORDF ] &&
	{
		TMPFILE=/tmp/orf

		cat $ORDF | while read LINE
		do
			fgrep -v "$LINE" $ORPHAN_FILE > $TMPFILE
			mv $TMPFILE $ORPHAN_FILE
		done
	}
}


: RestoreSystem -
#
#	Given:	Nothing
#
#	Does:	Returns the machine back to it's state prior to starting
#		the update installation.  This basically means that any
#		customized files saved as 'filename.PreUPD' are removed
#		from the system.
#
#	Return:	Nothing
#
RestoreSystem()
{
	Error "\nReturning system to Pre-Update state...\c"
	CustomFileBackup restore
	Error "done.\nUpdate Installation exiting."
	Exit 1
}


: RitKitSystem -
#
#	Given:	$1 - A datalist of paths and products.
#
#	Does:	Finds orphan and customized system files on the system.
#
#	Return: Nothing
#
RitKitSystem()
{
	set -- `Parse : $1`
	for K in $*
	do
		set -- `Parse , $K`
		SSPATH=$1
		shift
		$SIFSYNC $SSPATH $* || RestoreSystem
	done
}


: SetEnvironment -
#
#	Given:	Nothing
#
#	Does:	Prepares the program environment for further execution of
#		this script.  Sets variables, reads in information, exports
#		variables.
#
#	Return: Nothing
#
SetEnvironment()
{
	#
	# Variables defined by /sbin/installupdate are read in here.
	#
	if [ -f /tmp/updinfo ]
	then
		#
		# We'll read in the file with all the variables and
		# then make an update directory and copy it there.
		# All other scripts used during the update install,
		# and updadmin in the post-install environment, will
		# get information from this file.
		#
		. /tmp/updinfo
		UPDDIR=/var/adm/update
		UPDINFO=$UPDDIR/updinfo
		[ -d $UPDDIR ] || mkdir -p $UPDDIR
		cp /tmp/updinfo $UPDDIR
	else
		echo "Cannot locate /tmp/updinfo"
		Exit 1
	fi

	#
	# Set up the PATH and export some variables here.
	#
	#	LOCMNT and OSFVER are used by updadmin later on.
	#
	#	ISL_PATH is used somewhere else.
	#
	PATH=${ISL_PATH}:/sbin:/usr/sbin:/usr/lbin:/bin:/usr/share/lib/shell
	export PATH ISL_PATH

	#
	# Read in shell libraries
	#
	# These will be read in from ISL_PATH if they exist there.
	# Otherwise they will come in from /usr/share/lib/shell
	#
	. Wait
	. Error
	. Ticker
	. BitTest
	. Strings

	#
	# Variables defined by the build procedure are read in here.
	#
	#	OSFVER is the O/S version the system is being updated to.
	#
	#	OSFUPD are the products that OSFVER can update from.
	#
	[ -f $LOCMNT/isl/updpinfo ] && . $LOCMNT/isl/updpinfo

	#
	# Set some local variables.
	#
	DATALIST=
	NUL=/dev/null
	PIID=/tmp/risupdinfo
	PWD=`/bin/pwd`
	RCMD="-l ris -n"
	ORDF=$LOCMNT/isl/updrepack.dat
	SIFSYNC=$LOCMNT/isl/sifsync
	UPDADMIN=$LOCMNT/isl/updadmin
	UPDMORE=$LOCMNT/isl/updmore
	CUSTOM_FILE=/var/adm/smlogs/upd_custom_files
	ORPHAN_FILE=/var/adm/smlogs/upd_obsolete_files
	STARTUP_MSG1="\nThe $OSFVER Update Installation will update the"
	STARTUP_MSG2="\nfollowing DEC OSF/1 products:"
	STARTUP_MSG="${STARTUP_MSG1}${STARTUP_MSG2}\n${OSFUPD}"

	#
	# Set UPDFLAG and export it, so that we know we're doing an update.
	# Default is 0 
	# 
	UPDFLAG=0
	export UPDFLAG

	#
	# Make sure that OSFVER, OSFUPD, CUSTOM_FILE and ORPHAN_FILE
	# are defined in UPDINFO.
	#
	echo "OSFVER=\"$OSFVER\"" >> $UPDINFO
	echo "OSFUPD=\"$OSFUPD\"" >> $UPDINFO
	echo "UPDMORE=\"$UPDMORE\"" >> $UPDINFO
	echo "CUSTOM_FILE=\"$CUSTOM_FILE\"" >> $UPDINFO
	echo "ORPHAN_FILE=\"$ORPHAN_FILE\"" >> $UPDINFO

	#
	# Set trap
	#
	TRAPMSG="\nInterrupt detected.\nExiting Update Installation...."
	trap 'echo $TRAPMSG; Exit 1' 1 2 3 15
}


: SetItUp -
#
#	Given:	Nothing
#
#	Does:	Sets up the system for it(8) execution.
#
#	Return: Nothing
#
SetItUp()
{
	ITDATA=/sbin/it.d/data		# it(8) data directory

	#
	# set up inst.cnfg with a ISL_ADVFLAG variable so that
	# doconfig and other scripts will execute as though this
	# were an installation.  (which it is)
	#
	echo "ISL_ADVFLAG=0" > $ITDATA/inst.cnfg
	echo "UPDFLAG=$UPDFLAG" >> $ITDATA/inst.cnfg
	echo "export UPDFLAG" >> $ITDATA/inst.cnfg


#########################################################################
#									#
# The code in this box is commented out until ADVFS is fully incorpor-	#
# ated into the base product.  When that time comes, this code should	#
# become active, and ADVgate() should be removed entirely.  		#
#									#
#									#
#	#								#
#	# setup c_install and doconfig in the proper run state		#
#	# directory, then copy genvmunix to vmunix.			#
#	#								#
#	(cd /; itruns c_install doconfig in 23)				#
#	cp /genvmunix /vmunix						#
#									#
#									#
#########################################################################
	#
	# Use ADVgate until ADVFS is fully incorporated into the base
	# product.  ADVgate checks to see if ADVfs file systems are
	# in use on the system, and if so, performs operations that
	# are necessary to bring the system to a running state after
	# an update installation has completed.
	#
	ADVgate
}


[ "$CHECK_SYNTAX" ] || Main "$@"
