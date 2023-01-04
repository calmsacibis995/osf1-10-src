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
# @(#)$RCSfile: updadmin.sh,v $ $Revision: 1.1.2.7 $ (DEC) $Date: 1993/11/12 22:19:32 $
# 
##########################################################################
# sifsync_ui.sh
#	System Inventory File Syncronization User Interface
#
##########################################################################


: ActOnCustom -
#
#	Given:	Nothing
#
#	Does:	Sets various constants that are used globally through-
#		out this program.
#
#	Return: Nothing
#
ActOnCustom()
{
	[ -s $CUSTOM_FILE ] ||
	{
		[ "$UPDFLAG" ] ||
			echo "\nNo customized file list exists.\n"

		return 1
	}

	FILETYPE=customized

	while :
	do
		echo "


******* Unprotected Customized System File Administration ********"

		if [ "$UPDFLAG" ]
		then

			echo "
There are unprotected  customized  versions  of  DEC OSF/1  system
files currently installed  on  your  system  that are in danger of
being overwritten by new ${OSFVER} versions of
the files.

These files are  typically  customized  by users for projects they
may be working on, or by layered  product software implementation.

If you haven't already backed up your system,  save these files to
back up media at this time.  You can restore these files back onto
your system after the update installation has completed."

		else

			echo "
Unprotected customized system files are typically DEC OSF/1 system
files that have been  customized by users for projects they may be
working on, or by layered product software implementation.

Some of these  customized  files  may no longer be compatible with
the ${OSFVER} operating system."

		fi

		echo "\n\tUnprotected Customized System File Admin Menu"
		echo "\t---------------------------------------------"

		[ "$UPDFLAG" ] &&
			echo "\tq) Quit installation"

		echo "\ts) Save files"

		[ "$UPDFLAG" ] ||
			echo "\td) Delete files"

		echo "\tv) View list of files"
		echo "\tx) Return to previous menu"
		echo "\n\tEnter your choice: \c"
		read ans
		ans=`echo $ans`
		case $ans in
		d) [ "$UPDFLAG" ] || DeleteFiles $CUSTOM_FILE ;;
		q) return 1 ;;
		s) SaveFiles $CUSTOM_FILE ;;
		v) ViewFiles $CUSTOM_FILE ;;
		x) return 0 ;;
		esac
	done
}


ActOnDirectory()
{
	while :
	do
		echo "\nThere is no $1 directory on this system."
		echo "Do you want to create one? (y/n) [y]: \c"
		read ANS
		ANS=`echo $ANS`
		case $ANS in
		[Yy]* | "" )
			mkdir -p $1 ||
			{
				echo "\nCannot create '$1'."
				return 1
			}
			return 0
			;;
		[Nn]* )
			return 1
			;;
		esac
	done
}


: ActOnOrphan -
#
#	Given:	Nothing
#
#	Does:	Sets various constants that are used globally through-
#		out this program.
#
#	Return: Nothing
#
ActOnOrphan()
{
	[ -s $ORPHAN_FILE ] ||
	{
		[ "$UPDFLAG" ] ||
			echo "\nNo obsolete file list exists.\n"

		return 1
	}

	FILETYPE=obsolete

	while :
	do
		echo "


************* Obsolete System File Administration ***************

There are DEC OSF/1 files currently installed on your system that
are no longer shipped or supported in ${OSFVER}.

Some of these obsolete files may no longer be compatible with the
${OSFVER} operating system.  You may want to
back up these files and then delete them from your system to regain
disk space.  For your reference, the obsolete files are listed in
$ORPHAN_FILE.

	Obsolete System File Admin Menu
	-------------------------------
	s) Save obsolete files
	d) Delete obsolete files
	v) View list of obsolete files
	x) Return to previous menu 

	Enter your choice: \c"
		read ans
		ans=`echo $ans`
		case $ans in
		d) DeleteFiles $ORPHAN_FILE ;;
		s) SaveFiles $ORPHAN_FILE ;;
		v) ViewFiles $ORPHAN_FILE ;;
		x) return 0 ;;
		esac
	done
}


: ActOnPreMRG -
#
#	Given:	Nothing
#
#	Does:	Sets various constants that are used globally through-
#		out this program.
#
#	Return: Nothing
#
ActOnPreMRG()
{
	PREMRG_FILE=/var/adm/smlogs/upd_PreMRG_files

	#
	# Check for any PreMRG files on the system.
	#
	FIND=1
	while :
	do
		if [ -s $PREMRG_FILE ]
		then
			break
		else
			[ "$FIND" = 0 ] &&
			{
				echo "\nNo PreMRG file list exists.\n"
				return 1
			}
			echo "\n\n\nChecking system for .PreMRG files. \c"
			echo "Depending on the number"
			echo "of filesystems mounted, \c"
			echo "this may take a few minutes..."
			cd /
			find . -fstype ufs -name "*.PreMRG" -print \
				> $PREMRG_FILE
			FIND=0
		fi
	done

	FILETYPE=PreMRG

	while :
	do
		echo "


*************  PreMRG System File Administration  ***************

There are .PreMRG files left over on your system from an update
installation.   PreMRG files are copies of system files as they
existed prior to  updating  your  system.  They are left on the
system after an update installation for reference purposes only.

If any system file customization merges were not successful, you
can reference the  .PreMRG file  to include those customizations
in the post merged file now residing on the system.

In most cases, PreMRG files can be deleted from the system.


	PreMRG System File Admin Menu
	-------------------------------
	s) Save PreMRG files
	d) Delete PreMRG files
	v) View list of PreMRG files
	x) Return to previous menu 

	Enter your choice: \c"
		read ans
		ans=`echo $ans`
		case $ans in
		d) DeleteFiles $PREMRG_FILE ;;
		s) SaveFiles $PREMRG_FILE ;;
		v) ViewFiles $PREMRG_FILE ;;
		x) return 0 ;;
		esac
	done
}


CleanUp()
{
	rm -f $TARLIST
	return
}


CompressImage()
{
	while :
	do
		echo "
Compressing the tar image will result in less disk space used.
Do you want to compress the tar image? (y/n) [y]: \c"
		read ANS
		ANS=`echo $ANS`
		case $ANS in
		[Yy]* | "" )
			echo "\nCompressing $1..."
			if compress $1
			then
				echo "Compressed tar image ${1}.Z created."
			else
				Error "Unable to compress $1"
			fi
			break
			;;
		[Nn]* )
			;;
		*)
			continue
			;;
		esac
		break
	done
}


: CreateSaveList -
#
#	Given:	A file name
#
#	Does:	Differentiates directories from regular files and puts
#		directories in the TARLIST only if there are no files
#		in the directory.  Packaging errors may cause some dir-
#		ectories to show up as obsolete, even though they are
#		not.  For both saving and deleting options, we need to
#		make sure the directory is really empty before acting
#		on it.
#
#	Return: Nothing
#
CreateSaveList()
{
	cd /
	>$TARLIST

	#
	# grep file entries from $1.  Skip anything beginning with "=",
	# and anything containing (DELETED.
	#
	for K in `grep -v "=" $1 | grep -v "(DELETED"`
	do
		if [ -d $K ]
		then
			ls $K >/dev/null || echo $K >> $TARLIST
		else
			echo $K >> $TARLIST
		fi
	done
}


DeleteFiles()
{
	DELETEFILE=$1
	SaveDetection $DELETEFILE &&
	{
		echo "


===================================================================
Back up of $FILETYPE files not detected.

If you have not backed up the $FILETYPE files yet, please do so at
this time by answering 'no' to the question below and selecting the
's' option from the previous menu.
==================================================================="
	}

	while :
	do
		echo "
Please confirm your intent to delete $FILETYPE system files
from the system. (y/n) [n]: \c"
		read ANS
		ANS=`echo $ANS`
		case $ANS in
		[Nn]* | "" )
			echo "\n"
			return
			;;
		[Yy]* )	break	;;
		esac
	done

	#
	# Get ready to delete files
	#
	echo "\nRemoving $FILETYPE files...\n"

	#
	# Set up temporary file, and variables used to display
	# the proper delete result message.
	#
	cd /
	> /tmp/tsmf
	FILE_DELETED=0
	DELETE_ERROR=0

	DEL_STAT=`cat $DELETEFILE |
		(
			while read LINE
			do
				case "$LINE" in
				"" | =* | *DELETED* )
					echo "$LINE" >> /tmp/tsmf
					continue
				esac

				set -- $LINE

				if [ -d $1 ]
				then
					rmdir $1
				else
					rm $1
				fi

				if [ "$?" = 0 ]
				then
					FILE_DELETED=1
					echo "$1\t(DELETED FROM SYSTEM)" >> /tmp/tsmf
				else
					DELETE_ERROR=1
					Error "Cannot delete $1."
					echo "$LINE" >> /tmp/tsmf
				fi
			done
			echo "${FILE_DELETED}${DELETE_ERROR}"
		)`


	case $DEL_STAT in
	00)
		Error "\nNo $FILETYPE files deleted."
		Error "Please use the 'v' option to view current status of"
		Error "$FILETYPE files on this system."
		;;
	01)
		Error "\nNo $FILETYPE files deleted."
		Error "Some errors occured during the deletion process. \c"
		Error "Please use the 'v' option"
		Error "to view current status of $FILETYPE files on \c"
		Error "this system."
		;;
	10)
		Error "\nFinished deleting $FILETYPE files."
		;;
	11)
		Error "\nFinished deleting $FILETYPE files."
		Error "Some errors occured during the deletion process. \c"
		Error "Please use the 'v' option"
		Error "to view current status of $FILETYPE files on \c"
		Error "this system."
		;;
	esac

	mv /tmp/tsmf $DELETEFILE
	return
}


DuplicateSave()
{
	while :
	do
		echo "

You have already saved the $FILETYPE files.
Do you want to save them again? (y/n) [n]: \c"
		read ANS
		ANS=`echo $ANS`
		case $ANS in
		[Yy]* )		return 0 ;;
		[Nn]* | "" )	return 1 ;;
		esac
	done
}


: Exit -
#
#	Given:	An error status
#
#	Does:	Calls cleanup routine and then exits with status STAT.
#
#	Return: Nothing
#
Exit()
{
	STAT=$1

	CleanUp
	exit $STAT
}


: Main -
#
#	Given:	At set of command arguments.
#
#	Does:	Runs the rest of the program.
#
#	Return: Nothing.
#
Main()
{
	CMDLIST=$*
	SetGlobalConstants
	for CMD in $CMDLIST
	do
		case $CMD in
		custom*)
			ActOnCustom || Exit 1
			;;
		orphan*)
			ActOnOrphan || Exit 1
			;;
		esac
	done

	[ "$CMDLIST" ] || TopLevelMenu

	Exit 0
}


SaveDetection()
{
	if [ "$1" = "$ORPHAN_FILE" ]
	then
		[ "$ORPHAN_SAVED" ] && return 1
	else
		[ "$CUSTOM_SAVED" ] && return 1
	fi

	return 0
}


SaveFiles()
{
	SaveDetection $1 || DuplicateSave || break

	while :
	do
		echo "


	Select a method of archiving your files:
	----------------------------------------
	d) Save files to directory on disk
	i) Save files to tar image on disk
	t) Save files to tape media device

	Enter your choice from the menu above, or press <RETURN>
	to go back to the previous menu: \c"
		read ANS 
		ANS=`echo $ANS`
		case $ANS in
		"")
			echo
			echo
			return
			;;
		d)
			SaveToDirectory $1 && break
			;;
		i)
			SaveToImage $1 && break
			;;
		t)
			SaveToTape $1 && break
			;;
		esac
	done

	return
}


SaveToDirectory()
{
	while :
	do
		echo "


Enter the directory that you want to use to back up $FILETYPE
files, or press <RETURN> to go back to the previous
menu: \c"
		read DIR
		DIR=`echo $DIR`
		[ "$DIR" ] &&
		{
			[ -d $DIR ] ||
				ActOnDirectory $DIR ||
					continue

			echo "\nSaving $FILETYPE files to $DIR..."
			CreateSaveList $1
			(cd /; tar cpRf $TARLIST - | (cd $DIR; tar xf -))
			if [ $? = 0 ]
			then
				echo "Finished saving $FILETYPE files."
				SetSavedSwitch $1
				return 0
			else
				Error "Error performing tar."
				rm -rf $DIR
			fi
		}
		break
	done

	return 1
}


SaveToImage()
{
	while :
	do
		echo "


Enter the name of the tar file that you want to use to back up
$FILETYPE files, or press <RETURN> to go back to the previous
menu: \c"
		read TFILE
		TFILE=`echo $TFILE`

		[ "$TFILE" ] &&
		{
			# tar flag is c by default
			TF=c

			[ -f $TFILE -o -f ${TFILE}.Z ] &&
			{
				if TarFileConflict $TFILE
				then
					TF=u
				else
					continue
				fi
			}

			DIR=`dirname $TFILE`
			TFA=`basename $TFILE`
			[ -d $DIR ] ||
			{
				ActOnDirectory $DIR ||
					continue
			}

			DIR=`(cd $DIR; /bin/pwd)`
			TFA=$DIR/$TFA

			echo "\nSaving $FILETYPE files to tar file $TFILE..."
			CreateSaveList $1
			(cd /; tar ${TF}pRf $TARLIST $TFA)
			if [ $? = 0 ]
			then
				echo "Finished saving $FILETYPE files."
				SetSavedSwitch $1
				CompressImage $TFA
				return 0
			else
				Error "Error performing tar."
				rm -f $TFA
			fi
		}
		break
	done

	return 1
}


SaveToTape()
{
	while :
	do
		echo "


Enter the name of the tape backup device (for example: /dev/nrmt0h) 
that you want to use to back up $FILETYPE files, or press <RETURN>
to go back to the previous menu: \c"
		read DEVICE
		DEVICE=`echo $DEVICE`
		[ "$DEVICE" ] || break
	
		#
		# Test for existance of device	
		#
		[ -c $DEVICE ] ||
		{
			Error "\nCannot locate a special device named \c"
			Error "'$DEVICE'"
			continue
		}

		echo "\nInspecting $DEVICE..."
		# Test to see if device is writable.
		dd if=/vmunix of=/dev/rmt0h count=1 >/dev/null 2>&1 ||
		{
			Error "\nCannot write to '$DEVICE'"
			continue
		}

		echo "\nSaving $FILETYPE files to $DEVICE..."

		CreateSaveList $1

		(cd /; tar cpRf $TARLIST $DEVICE)
		if [ $? = 0 ]
		then
			echo "Finished saving $FILETYPE files."
			SetSavedSwitch $1
			return 0
		else
			Error "Error performing tar."
		fi
		break
	done

	return 1
}


: SetGlobalConstants -
#
#	Given:	Nothing
#
#	Does:	Sets various constants that are used globally through-
#		out this program.
#
#	Return: Nothing
#
SetGlobalConstants()
{
	#
	# Set up PATH
	#
	[ "$UPDFLAG" ] ||
	{
		echo $PATH | grep -q /usr/share/lib/shell ||
		{
			PATH=${PATH}:/usr/share/lib/shell
			export PATH
		}
	}

	#
	# Read in shell libraries
	#
	. Error

	#
	# Must be superuser to use this script.
	#
	[ "`whoami`" = "root" ] ||
	{
		echo "You must be superuser to use $0."
		exit 1
	}

	#
	# Get update install data.
	#
	if [ -f /var/adm/update/updinfo ]
	then
		. /var/adm/update/updinfo
	else
		if [ -d /var/adm/update ]
		then
			Error "Cannot find /var/adm/update/updinfo."
		else
			Error "This utility is only functional after an \c"
			Error "update installation"
			Error "has been performed on the system."
		fi
		exit 1
	fi

	TARLIST=/tmp/t1
}


SetSavedSwitch()
{
	if [ "$1" = "$ORPHAN_FILE" ]
	then
		ORPHAN_SAVED=1
	else
		CUSTOM_SAVED=1
	fi
}


TarFileConflict()
{
	while :
	do
		echo "
***********************
WARNING: File conflict  
***********************
A file named '$1' already exists on this system.
If this is a tar archive file you can append additional files to it.
Otherwise, answer 'n' to the question below.

Do you want to try appending to it? (y/n) [n]: \c"
		read ANS
		ANS=`echo $ANS`
		case $ANS in
		[Yy]* )
			[ -f ${1}.Z ] &&
			{
				uncompress ${1}.Z ||
				{
					Error "Cannot uncompress '$1'"
					Error "Please choose another file."
					return 1
				}
			}
			return 0
			;;
		[Nn]* | "" )
			return 1
			;;
		esac
	done
}


TopLevelMenu()
{
	while :
	do
		echo "


The Update Administration Utility is used to perform administration
functions on a system that has been updated by /sbin/installupdate.

Please make a selection from the following menu.

	Update Administration Utility Main Menu
	---------------------------------------
	c) Unprotected Customized File Administration
	o) Obsolete System File Administration
	p) PreMRG File Administration
	x) Exit this utility

	Enter your choice: \c"
		read ANS
		ANS=`echo $ANS`
		case $ANS in
		c) ActOnCustom ;;
		o) ActOnOrphan ;;
		p) ActOnPreMRG ;;
		x) Exit 0 ;;
		esac
	done
}


ViewFiles()
{
	R1="Enter 'r' to review files again, or press <RETURN> to go back"
	R2="to the previous menu: "

	#
	# The installation uses /usr/sbin/log, which can't deal with the
	# 'more' command.  If this is an installation, we need to check
	# for our installation specific 'updmore' program which will work
	# with /usr/sbin/log.  If 'updmore' doesn't exist, we'll just use
	# the 'cat' command.
	#
	# If this is not an installation, we'll let them use the 'more'
	# command.
	#
	if [ "$UPDFLAG" ]
	then
		if [ -f $UPDMORE ]
		then
			CMD=$UPDMORE
		else
			CMD=cat
		fi
	else
		CMD=more
	fi

	while :
	do
		#
		# Let them look at the file.  If this is a PreMRG file,
		# throw out a header as well.
		#

		[ "$FILETYPE" = "PreMRG" ] &&
			echo "\n\n---------------------------------------"

		$CMD $1
		while :
		do
			#
			# Check to see if they want to look at the file
			# again.
			#
			echo "\n$R1\n$R2\c"
			read ANS
			ANS=`echo $ANS`
			case $ANS in
			"" )	break 2 ;;
			[Rr]* )	break ;;
			esac
		done
	done

	echo
}


[ "$CHECK_SYNTAX" ] || Main "$@"
