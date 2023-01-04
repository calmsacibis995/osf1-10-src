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
# @(#)$RCSfile: gendisk.sh,v $ $Revision: 4.3.3.4 $ (DEC) $Date: 1992/11/06 16:34:17 $ 
#
#	gendisk.sh - sh script for generic remote disk production
#
#	MODS:
#
#		009	ech, 05-Nov-1991
#			create default disk label after zeero
#
#		008	ech, 17-Sep-1991
#			takes generic disks as input
#
#		007 	ech, 3-May-91
#			Ported to OSF/1
#
#		006	Jon Wallace, 8-Nov-1989
#			Fixed expr bug.
#
#		005	Tom Tresvik, 26-Sep-89
#			Merge Greg's and Jon's latest changes.
#
#		004	Greg Tarsa, 15-Sep-1989
#			Changed code that re-writes .crtl file to edit
#			it instead.
#			Added code to preprocess kitcap lines so that
#			"prettying" characters can be used.
#
#		003	jon	20-jun-1989
#			converted script to sh5.
#			Added product merge support.
#
#		002	ccb	01-jul-1988
#			added support for cz kits.
#
#		001	jon	31-dec-1987
#			Added call to ZEERO.
#
#		000	ccb	15-jul-1986
#
########################################################################
########################################################################

set -h	# hash subroutine references

: Disk_Check_Routine
Disk_Check_Routine()
{
	echo "\nChecking $DEV"
	fsck $RAW ||
		{
		echo "\nCannot fsck."
		exit
		}

	echo "\nMounting $DEV on $DISK_MNT"
	mount $DEV $DISK_MNT ||
		{
		echo "Cannot mount."
		exit
		}
}


: Pack_Dir_Routine
Pack_Dir_Routine()
{
	cd $DISK_MNT
	DESTDIR=$DISK_MNT
	set $1
	DDDIR=$1
	PACKDIR=`echo $1 | sed 's/dd=//'`

	case $WRITE$VFY in
	01 )
		DESTDIR=$DESTDIR/$PACKDIR
		;;
	* )
		PACKDIR=`echo $PACKDIR | sed 's/\// /g'`
		for PK in $PACKDIR
		do
			[ -d $DESTDIR/$PK ] &&
				{
				DESTDIR=$DESTDIR/$PK
				continue
				}
			mkdir $DESTDIR/$PK ||
				{
				echo "Cannot make $DESTDIR/$PK"
				exit 1
				}

			DESTDIR=$DESTDIR/$PK
		done
		cd $DESTDIR
		;;
	esac
}


: Subset_List_Routine
Subset_List_Routine()
{
	set $KITLIST
	KITNAME=$1; shift
	KITNAME=`echo $KITNAME | sed 's/_/ /g'`
	KITLIST=`echo $*`
	for K in $KITLIST
	do
		case $K in
		dd* )
			shift
			KITLIST=`echo $*`
			SUBSETLIST="$SUBSETLIST $K"
			break
			;;
		/* )
			case $RCMD in
			"")
				[ -d $K ] ||
					{
					echo "^GDirectory $K does not exist."
					exit 1
					}
				;;

			* )
				[ X`$RCMD "test -d $K || echo a"` = Xa ] &&
					{
					echo "Directory $K does not exist."
					exit 1
					}
				;;
			esac

			DIRELIST="$DIRELIST $K"
			shift
			;;
		* )
			SUBSETLIST="$SUBSETLIST $K"
			shift
			;;
		esac
	done
}


: Image_Merge - Merge new image file
Image_Merge()
{

	for K in $DIRELIST
	do
		rcp -p $HOST:$K/instctrl/* $WORKDIR
	done

	touch $MERGEDIR/$NEWIMAGE.image

	for K in $SUBSETLIST
        do
		case $K in
		*ROOT )  ;;

		* )
			[ -f ${WORKDIR}/${K}.ctrl ] || continue
			TAPELOC=`expr $TAPELOC + 1`
       			OLDIMAGE=`expr $K : '\([0-9A-Z][0-9A-Z][0-9A-Z]\)'`
			VERSCODE=`expr $K : '.*\([0-9][0-9][0-9]\)'`
			touch $MERGEDIR/$OLDIMAGE$VERSCODE.comp

			#
			# We edit the ctrl file because there may be
			# backquote substitutions that need to be
			# evaluated at setld loading time and not now.
			#
			sed	-e "/^NAME=/s.*NAME='$KITNAME'" \
				    ${WORKDIR}/$K.ctrl > /usr/tmp/ctrl.$$ &&
			    {
			    if test "$UTIL_DEBUG"
			    then
				echo "\noriginal file:" |
					cat - ${WORKDIR}/$K.ctrl
				echo "\nnew file:" |
					cat - /usr/tmp/ctrl.$$
			    fi

			    #
			    # Replace the old file with the edited version
			    #
			    mv /usr/tmp/ctrl.$$ ${WORKDIR}/$K.ctrl
			    }
			ed - ${WORKDIR}/$K.ctrl <<xxEOFxx 1>/dev/null
				/^MTLOC/d
i
MTLOC=1:$TAPELOC
.
				w
				q
xxEOFxx
			;;
		esac


		FOUND=0
		for IMAGEFILE in $WORKDIR/*.image
		do
			SSINFO=`egrep "$K" $IMAGEFILE` && FOUND=1
			case $SSINFO in
			"" )
				continue
				;;

			* )
				CKINFO=`egrep "$K" $MERGEDIR/$NEWIMAGE.image` ||
					{
					case $CKINFO in
					"" | $SSINFO )
						echo "$SSINFO" >> $MERGEDIR/$NEWIMAGE.image
						;;
					* )
						echo "Fatal error. Cannot Merge $KITCODE."
						echo "Subset $K differs in more than one image file."
						exit 1
						;;
					esac
					}
				;;
			esac
		done

		[ "$FOUND" = 1 ] ||
			{
			echo "Cannot find $K in image file."
			exit 1
			}
	done
}


: Instctrl_Routine
Instctrl_Routine()
{
	mkdir $DESTDIR/instctrl ||
		{
		echo "Error: mkdir $DESTDIR/instctrl failed!"
		exit 1
		}

	cd $MERGEDIR; tar cpf - $NEWIMAGE.image |
		(cd $DESTDIR/instctrl; tar xpf - )

        for K in $MERGEDIR/*.comp
	do
		K=`basename $K`
		[ -f $WORKDIR/$K ] &&
                        {
			cd $WORKDIR; tar cpf - $K |
				(cd $DESTDIR/instctrl; tar xpf - )
			}
	done

        for K in $SUBSETLIST
	do
		[ -f $WORKDIR/$K.* ] &&
			{
                        cd $WORKDIR; tar cpf - $K.* |
				(cd $DESTDIR/instctrl; tar xpf - )
			}
	done
}


: Image_Search - Search directory list for the image or subset
Image_Search()
{
	FOUND=0
	for DIR in $DIRELIST
	do
		case $RCMD in
		"")
			[ -f $DIR/$K ] &&
				{
				FOUND=1
				break
				}
			;;

		* )
			[ X`$RCMD "test -f $DIR/$K && echo a"` = Xa ] &&
				{
				FOUND=1
				break
				}
			;;
		esac
	done
	[ "$FOUND" = 1 ] ||
		{
		echo "^G\nCannot find $K in any of the following directories:"
		echo $DIRELIST
		exit 1
		}
}


: Write_Routine - Write subsets to disk from here
Write_Routine()
{
        echo "\nWriting Images ($DDDIR).\n"
	STATE=0

	for K in $SUBSETLIST
	do
		case $K in 
		dd=* )
			Stateofwrite_Routine
			case $WRITE$VFY in
			01 | 11 )
				Verify_Routine
				;;
			esac
			echo 	# spacer for text
			rm -rf $WORKDIR/* $MERGEDIR/*
			DIRELIST=
			SUBSETLIST=
			Pack_Dir_Routine $K
			Subset_List_Routine
			Write_Routine
			;;
		instctrl )
			echo "	Image $K...\c"
			Image_Merge
			Instctrl_Routine
			echo "done.\n"
			continue
			;;
		* )
			DFLAG=
			case $RCMD in
			"")
				[ -d $K ] && DFLAG="YES"
				;;

			* )
				[ X`$RCMD "test -d $K && echo a"` = Xa ] && DFLAG="YES"
				;;
			esac

			# copy in a directory
			[ "$DFLAG" = "YES" ] &&
				{
				rsh $HOST "cd $DIR;tar cf - $K" |
					(cd $DESTDIR;tar xpf -) ||
					{
					echo "Failed."
					wait;sync;sync;sync
					exit
					}
				echo "done. \n"
				continue
				}

			echo "	Image $K...\c"
			Image_Search $K

			case "$STATE" in
			0)
				rcp -rp $HOST:$DIR/$K $DESTDIR &
				PID=$!
				STATE=1
				SAVE=$K
				echo "backgrounded. \n"
				sleep 3
				;;
			1)
				rcp -rp $HOST:$DIR/$K $DESTDIR ||
					{
					echo "Failed."
					wait;sync;sync;sync
					exit
					}
				STATE=0
				echo "done.\n"
				wait ||
					{
					echo "$SAVE failed."
					sync;sync;sync
					exit
					}
				echo "	$SAVE done.\n"
				;;
			esac
			;;
		esac
	done

}

: Stateofwrite_Routine
Stateofwrite_Routine()
{
	case "$STATE" in
	1)	wait ||
			{
			echo "$SAVE failed"
			sync;sync;sync
			exit
			}
		echo "	$SAVE done.\n"
	esac
	sync;sleep 2;sync;sleep 2;sync
	STAT=0
}


: Verify_Routine - Verify subsets here.
Verify_Routine()
{
	echo "\nVerifying Images ($DDDIR)."

	for K in $SUBSETLIST
	do
		case $K in
		dd*)
			case $WRITE$VFY in
			01 )
				rm -rf $WORKDIR/* $MERGEDIR/*
				DIRELIST=
				SUBSETLIST=
				Pack_Dir_Routine $K
				Subset_List_Routine
				Image_Merge
				Verify_Routine
				;;
			esac
			continue
			;;
		*osf_boot | *vmunix )	# Boot stuff, skip
			echo "\n	Image $K...\c"
			sleep 3
			echo "done."
			continue
			;;

		*TK50* )
			echo "\n	Image $K...\c"
			K=`expr $K : '.*\([T][K].*\)'`
			FOUND=0
			for VS in $DIRELIST
			do
				case $RCMD in
				"")
					[ -f $DIR/$K ] &&
						{
						FOUND=1
						break
						}
					;;

				* )
					[ X`$RCMD "test -f $VS/$K && echo a"` = Xa ] &&
						{
						FOUND=1
						break
						}
					;;
				esac
			done
			if [ "$FOUND" = 1 ]
			then
				{
				[ "`$RCMD sum $VS/$K`" = "`sum $DESTDIR/$K`" ] ||
					{
					echo "bad checksum."
					continue
					}
				}
			else
				{
				echo "can't perform checksum."
				echo "source $HOST:$VS/$K does not exist."
				continue
				}
			fi
			echo "done."
			;;
		esac
 
		echo "\n	Image $K...\c"

		case $K in
		*/* )
			FILENAME=`expr $K : '.*[/][/]*\(.*\)'`
			[ -f $MERGEDIR/$NEWIMAGE.image ] ||
				{
				echo "done."
				continue
				}
			egrep "$FILENAME" $MERGEDIR/$NEWIMAGE.image > $TD/sum$FILENAME$$ ||
				{
				echo "done."
				continue
				}
			set x `cat $TD/sum$FILENAME$$`
			;;
		* )
			egrep "$K" $MERGEDIR/$NEWIMAGE.image > $TD/sum$K$$ ||
				{
				echo "done."
				continue
				}
			set x `cat $TD/sum$K$$`
			;;
		esac
		shift
		SUM=$1
		SIZE=$2

		sum $DESTDIR/$K > $TD/SUM$$ ||
			{
			echo "sum program error image $K."
			continue
			}
		set x `cat $TD/SUM$$`
		shift
		case "$1$2" in
		$SUM$SIZE)
			;;
		*)
			echo "bad checksum."
			continue
			;;
		esac
		echo "done."
	done
	sync;sleep 2;sync;sleep 2;sync;sleep 2
}


Clean_Up_Routine()
{
	echo "\nCleaning up working directories."
	wait
	CKMOUNT=`mount | egrep $DEV`
	[ -n "$CKMOUNT" ] &&
		{
		echo "Unmounting $DEV"
		cd /
		umount $DEV || echo "WARNING: umount failed."
		}
	(rmdir $DISK_MNT 2>&1) > /dev/null
	rm -rf $WORKDIR $MERGEDIR
	rm -f /usr/tmp/*$$
}


################################################################
# Set variables
#
PATH=/usr/ucb:/sbin:/usr/sbin:/bin:/etc:/usr/bin:usr/hosts
export PATH
PROG=$0
TAPELOC=0
DIRELIST=
SUBSETLIST=
TD=/usr/tmp
TMP=$TD/mttmp$$
WORKDIR=/usr/tmp/instctrl$$
DISK_MNT=/usr/tmp/cd_mnt$$
MERGEDIR=/usr/tmp/merge$$
KITCAP=/etc/kitcap
DFS="$IFS"
NL="
"
trap 'Clean_Up_Routine; exit $STAT' 0 1 2 3 15
STAT=1	# assume failure

readonly PROG PATH TMP DFS

#################################################################
# Check command line arguments to see if we are writing or 
# verifying.  Default is write AND verify.
#
WRITE=1 VFY=1
case "$1" in
-v)	# verify only.
	WRITE=0
	shift
	;;
-w)	# write only
	VFY=0
	shift
esac


#################################################################
# Now get the device, check to see if we are using a remote node,
# and then initialize some kitcap variables.
#
case $# in
2)	
	DEV=$2
	info=`file $DEV`
	echo "$info" | egrep -s "disk" ||
	{
		echo "$2 is not a valid character special device for disks"
		exit 1
	}
	DEVICE=`echo "$info" | awk '{print $7}' | dd conv=lcase 2>/dev/null`
	DEVTYPE=`expr $DEVICE : '\([a-z][a-z]\).*'`

	IFS=:
	set $1
	IFS="$DFS"
	case "$#" in
	2)	HOST=$1
		shift
	esac
	KITCODE=$1
	KITTYPE=${KITCODE}HD
	;;
*)	echo "Use: $PROG [-vw] [server:]kitcode device"
	exit 1
esac


#################################################################
# More network setups.
#
HERE=`hostname`
case "$HOST" in
"" | $HERE )
	RCMD=
	HOST=$HERE
	cp $KITCAP $TMP
	;;
*)
	RCMD="rsh $HOST -n"
	NUM=`egrep "[ 	]$HOST[.]" /etc/hosts | wc -l`
	[ "$NUM" -gt 1 ] &&
		{
		echo "\nHost $HOST has multiple BIND domain names. "
		echo "Select one and enter the full name on the command line."
		exit 1
		}
	[ "$NUM" ] &&
		{
		egrep -s "[ 	]$HOST[ 	]" /etc/hosts ||
			egrep -s "[ 	]$HOST$" /etc/hosts ||
				{
				echo "\nHost $HOST not in /etc/hosts."
				exit 1
				}
		}	
	ERROR=`$RCMD "echo hello" 2>&1` ||
		{
		echo "\nCannot connect to $HOST.  $ERROR"
		exit 1
		}
	rcp $HOST:$KITCAP $TMP
	;;
esac ||
	{
	echo "\nCannot access $HOST:$KITCAP."
	exit 1
	}

# GT004:
# Write out the sed script that will filter into single lines.
#
cat <<- '-EOFsedfile' > /usr/tmp/tmpkitcap.sed$$
    /^#/d;			0s**	delete comment lines *
    /^[ 	]*$/d;		0s**	delete blank lines *
    /:#/s/:#[^:]*//g;		0s**	delete in-line comments *

    0s**---------------------------------------------------- *
    0s** Combine all lines ending with backslash             *
    0s** When there are no more backslashes, jump to comprss *
    0s**---------------------------------------------------- *
    :combine
    /\\$/!bcomprss

    s/\\$//;			0s**	remove continuation character *
    N;				0s**	Append next line w/newline *
    s/[ 	]*\n[ 	]*//;	0s**	remove newline & assoc whitespace *

    bcombine

    0s**----------------------------------------------- *
    0s** Remove duplicates that arise out of prettyness *
    0s**----------------------------------------------- *
    :comprss

    s/[ 	]*:[ 	]/:/g;	0s**	remove spaces around colons *
    s/:::*/:/g;			0s**	change 2 or more colons into 1 colon *
    s/[ 	]*|[ 	]/|/g;	0s**	remove spaces around vertical bars *

    0s**	kitcap now consists of single lines *

    /:#/s/:#[^:]*//g;		0s**	delete in-line comments *
    s/:$//;			0s**	strip trailing colon *
-EOFsedfile

echo "    /${KITROOT_TOKEN}/s==${KIT_ROOT}=g" >> /usr/tmp/tmpkitcap.sed$$

sed -f /usr/tmp/tmpkitcap.sed$$ $TMP > ${TMP}xxx ||
	{
	echo 1>&2 "\07$FUNC: Internal error: cannot filter local kitcap file."
	return 1
	}

#
# Now filter the kitcap file, replacing the KITROOT_TOKEN, if necessary.
#
#sed -e "s=${KITROOT_TOKEN}=${KIT_ROOT}=g" \
#    -f /usr/tmp/tmpkitcap.sed$$ $TMP > ${TMP}xxx ||
#	{
#	echo 1>&2 "\07$FUNC: Internal error: cannot filter local kitcap file."
#	return 1
#	}

#
# Now remove the sed script and save the new tmpfile
# for use by the remaining script.
#
rm /usr/tmp/tmpkitcap.sed$$
mv ${TMP}xxx ${TMP}

#end GT004

#################################################################
# Seperate kitcap fields, then place each one into a variable list
# to be used later.
# 
KITDESC=`egrep "^$KITTYPE" $TMP` ||
	{
	echo "$PROG: can't find $KITTYPE kit descriptor in kitcap."
	exit
	}
IFS=:
set $KITDESC
IFS="$DFS"
shift
DISKPAR=$1
PACKDIR=$2
shift;shift
KITDESC=`echo $*`
KITLIST=$KITDESC
UD=`expr $DEV : '.*[a-z][a-z]\([0-9][0-9]*\).'`
DEV=/dev/$DEVTYPE$UD$DISKPAR
RAW=/dev/r$DEVTYPE$UD$DISKPAR
CLEAN_DISK=/dev/r$DEVTYPE${UD}c
NEWIMAGE=`expr $KITCODE : '\([A-Z][A-Z][A-Z]\)'`

echo "\nGenerating $KITCODE Kit from $HOST on $DEV"

mkdir $WORKDIR $MERGEDIR $DISK_MNT

[ -w $DEV ] ||
	{
	echo "No write access to $DEV"
	exit
	}
[ -w $RAW ] ||
	{
	echo "No write access to $RAW"
	exit
	}

case $WRITE$VFY in
10 | 11 )
	echo "\nWARNING: this will remove any information stored in $DEV."
	echo "Are you sure you want to do this? (y/n): \c"
	read ANS
	case "$ANS" in
	[Yy]*)
		echo "\nDo you want to clean the entire disk first? Note: This will replace" 
		echo "your current disk label with a default one. (y/n) [n]: \c"

		read ANS
		case "$ANS" in
		[Yy]*)
			/usr/lbin/zeero -f $CLEAN_DISK 
			disklabel -wr $RAW $DEVICE > /dev/null ||
			{
				echo "Cannot create disk label."
				exit
			}
			;;
		esac

		echo "\nPreparing $DEV ($DEVICE)"

		newfs $RAW $DEVICE > /dev/null ||
		{
			echo "Cannot newfs."
			exit
		}
		echo "done.\n"

		;;
	*)
		exit 1
		;;
	esac
	;;
esac


###################################################################
# Program Control Area - Merging, Writing, and Verifying controlled
# from here.
#
cd $WORKDIR
case $WRITE$VFY in
01 )
	Disk_Check_Routine
	Pack_Dir_Routine $PACKDIR
	Subset_List_Routine
	Image_Merge
	Verify_Routine
	;;
10 )
	Disk_Check_Routine
	Pack_Dir_Routine $PACKDIR
	Subset_List_Routine
	Write_Routine
	Stateofwrite_Routine
	;;
11 )
	Disk_Check_Routine
	Pack_Dir_Routine $PACKDIR
	Subset_List_Routine
	Write_Routine
	Stateofwrite_Routine
	Verify_Routine
	;;
esac

echo "\nKit $KITCODE done."
