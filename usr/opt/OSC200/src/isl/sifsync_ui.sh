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
# @(#)$RCSfile: sifsync_ui.sh,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/07/06 18:09:00 $
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
	[ -s $CUSTOM_FILE ] || return 1

	echo "
*** Custom System File Administration ***

There are customized versions of DEC OSF/1 system files currently
installed on your system that are in danger of being overwritten by
new ${OSFVER} versions of the files.

These files are typically customized by users for projects they may
be working on, or by layered product software implementation.

You may want to save these files and copy them back onto your system
after the update installation has completed.  If you haven't already
backed up your system, save these files to back up media at this time."

	while :
	do
		echo "
Please make a selection from the following menu:

	Custom File Admin Menu
	-----------------------
	c) Continue installation
	q) Quit installation
	s) Save files to backup device
	v) View list of files

Enter your choice: \c"
		read ans
		ans=`echo $ans`
		case $ans in
		c) return 0 ;;
		q) return 1 ;;
		s) SaveFiles $CUSTOM_FILE ;;
		v) ViewFiles $CUSTOM_FILE ;;
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
	[ -s $ORPHAN_FILE ] || return 1

	echo "\n
*** Obsolete System File Administration ***

There are DEC OSF/1 files currently installed on your system that are
no longer shipped or supported in ${OSFVER}.

Some of these obsolete files may no longer be compatable with the
${OSFVER} operating system.  You may want to
back up these files and then delete them from your system to regain
disk space.  For your reference, the obsolete files are listed in
$ORPHAN_FILE."

	while :
	do
		echo "
Please make a selection from the following menu:

	Obsolete File Admin Menu
	-------------------------
	s) Save obsolete files
	d) Delete obsolete files
	v) View list of obsolete files
	x) Exit 

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


CleanUp()
{
	rm -f $TARFILE
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

	for K in `grep -v "=" $1`
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
	SaveDetection $1 &&
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
Please confirm your intent to delete $FILETYPE system files from 
the system. (y/n) [n]: \c"
		read ANS
		ANS=`echo $ANS`
		case $ANS in
		[Nn]* | "" )	return	;;
		[Yy]* )		break	;;
		esac
	done

	echo "\nRemoving $FILETYPE files..."
	for K in `grep -v "=" $1 | sort -r`
	do
		if [ -d $K ]
		then
			rmdir $K >/dev/null
		else
			rm -f $K
		fi
	done

	return
}


DuplicateSave()
{
	while :
	do
		echo "
You have already saved the $FILETYPE files.  Do you want to save them
again? (y/n) [n]: \c"
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
			. Error
			FILETYPE=customized
			ActOnCustom || Exit 1
			;;
		orphan*)
			. /usr/share/lib/shell/Error
			FILETYPE=obsolete
			ActOnOrphan || Exit 1
			;;
		esac
	done
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
	while :
	do
		SaveDetection $1 || DuplicateSave || break

		echo "

	Select a method of archiving your files:

	d) Save files to directory on disk
	i) Save files to tar image on disk
	t) Save files to tape media device

	Enter your choice from the menu above, or
	press <RETURN> to go back to the previous menu: \c"
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

Enter the directory that you want to use to back up
$FILETYPE files, or press <RETURN> to go back to the
previous menu: \c"
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

Enter the name of the tar file that you want to use to back
up $FILETYPE files, or press <RETURN> to go back to the 
previous menu: \c"
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
			Error "\nCannot locate a special device named '$DEVICE'"
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
	[ "$OSFVER" ] ||
	{
		[ -f /sbin/it.d/data/updpinfo ] &&
			. /sbin/it.d/data/updpinfo
	}

	TARLIST=/tmp/t1
	ORPHAN_FILE=/var/adm/smlogs/upd_obsolete_files
	CUSTOM_FILE=/var/adm/smlogs/upd_custom_files
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
File conflict.  '$1' already exists.
Do you want to try appending to it? (y/n) [n]: \c"
		read ANS
		ANS=`echo $ANS`
		case $ANS in
		[Yy]* )
			[ -f ${1}.Z ] &&
			{
				uncompress ${1}.Z ||
				{
					echo "Cannot uncompress '$1'"
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


ViewFiles()
{
	R1="Enter 'r' to review files again, or press <RETURN> to go back"
	R2="to the previous menu: "

	while :
	do
		cat "$1"
		while :
		do
			echo "\n$R1\n$R2\c"
			read ANS
			ANS=`echo $ANS`
			case $ANS in
			"" )	break 2 ;;
			[Rr]* )	break ;;
			esac
		done
	done
}


[ "$CHECK_SYNTAX" ] || Main "$@"
