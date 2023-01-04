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
#
#	updmv.sh -
#		move user customizations and data files to/from mass storage.
#
#	updmv(8) is part of the customer system upgrade toolkit. It
#	is used by the customer to move customizations to and from the
#	system. The are 2 commmand keys:
#
#	updmv -o <loc>		copy info out to <loc>
#	updmv -i <loc>		copy info in from <loc>
#
#       @(#)$RCSfile: updmv.sh,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/05/05 13:36:32 $
#
#	004	03-dec-1991	ech
#		create /var/adm/install and /var/adm/install/update in Input()
#
#	003	20-may-1991	ech
#		Ported to OSF/1.
#
#	002	24-jan-1990	ccb
#		Fix mispelling of ROOT variable.
#
#	001	16-oct-1990	ccb
#		bugfixes
#
#	000	15-aug-1990	ccb
#		New.
#

# establish environment
CDPATH=
PATH=/isl:/usr/bin:/usr/sbin:/sbin:/usr/ucb
export CDPATH PATH

ARGS=$*


:	-Conflict
#		Determine which files in an inventory list would
#		cause a conflict
#
#	given:	$1 - inventory file to scan for conflicts
#	does:	writes all inventory records for which a file exists

Conflict()
{ (
	INV=$1

	cd $_R
	cat $INV |
		while read REC
		do
			set xxx $REC; shift
			shift
			[ -b $9 -o -c $9 -o -d $9 -o -f $9 -o -p $9 ] &&
				echo $9
		done
) }
	


Error()
{
	1>&2 echo $*
}



Exit()
{
	exit $1
}


Globals()
{
	_IAM=			# client hostname.
	_ROOT=/			# root directory.
	CTRLFILES="CFI MSI UFI"
	TARP=/usr/tmp/updpipe
	SMDB=usr/.smdb.
	VAI=/var/adm/install
	UPDDEST=$VAI/update
}



:	-Input
#		load customizations from offline storage
#
#	given:	$1 - media type in {INET,DISK,TAPE}
#		$2 - device specifier
#
#	does:	load customizations from MEDIA
#	return:	0 is all is OK

Input()
{
	MEDIA=$1
	UNIT=$2

	# get CFI and UFI from media
	cd /usr/tmp
	LoadID $MEDIA $UNIT
	# Conflict UFI > UFI.!ok

	# protect the original files which would be overwritten
	#  by user files

	# mkdir var/adm/install/original

	# [ -s UFI.!ok ] && Tarchive UFI.!ok |
	#	(cd var/adm/install/original; tar xpf -)

	# just in case
	[ -d $UPDDEST ] &&
		rm -rf $UPDDEST

        for X in $VAI $UPDDEST
	{
                [ -d $X ] || mkdir $X ||
                                return 1
	}


	cd $UPDDEST
	Retrieve $MEDIA $UNIT

	# stash customer files causing conflict

	# mkdir var/adm/install/conflict

	# [ -s UFI.!ok ] && Tarchive UFI.!ok |
	#	(cd var/adm/install/conflict; tar xpf -)

	# recover originals
	# [ -s UFI.!ok ] && (cd var/adm/install/conflict; tar cf - .) |
	#	tar xpf -

	echo "Files restored to $UPDDEST"
}


:	-LoadID
#		Load Inventory Data from MEDIA
#
#	given:	$1 - media type
#		$2 - media unit
#	does:	load the CFI and UFI from the MEDIA
#

LoadID()
{
	MEDIA=$1
	UNIT=$2

	case "$MEDIA" in
	INET)
		rcp ris@$UNIT:clients/risdb .
		RISDATA=`egrep '^'$IAM: risdb`
		set xxx RISDATA; shift
		set xxx $3; shift
		PROCDATA=$1
		rsh -n $UNIT "dd if=clients/$IAM.upd/updctrl bs=10k" 2>/dev/null |
			tar xpf -
		;;
	TAPE)	mt -f $UNIT rew
		tar xpf $UNIT
		mt -f $UNIT rew
		;;
	DISK)	tar xpf $UNIT/updctrl
		;;
	esac
}


Main()
{
	MEDIA=			# customization storage media
	PROCEDURE=		# procedure name
	UNIT=			# storage handle
	
	case `whoami` in
	root)	;;
	*)	Error "You must be super-user to use this procedure"
		Exit
	esac

	# Parse the arguments

	case "$1" in
	-i)	PROCEDURE=Input
		;;
	-o)	PROCEDURE=Output
		;;
	*)	Usage
		Exit 1
		;;
	esac

	case "$2" in
	*:)	MEDIA=INET
		UNIT=`expr $2 : '\(.*\):'`
		IAM=`hostname`
		set xxx `Parse . $IAM`; shift
		IAM=$1
		case "$UNIT" in
		"")	Error "cannot determine hostname"
			Exit 1
		esac
		;;
	/*)	if [ -d "$2" ]; then
		{
			MEDIA=DISK
			UNIT=$2
		}
		elif [ -c "$2" ]; then
		{
			MEDIA=TAPE
			UNIT=$2
		}
		else
		{
			Error "$2: unrecognized location code"
			Exit 1
		}; fi
		;;
	*)	Error "$2: Unrecognized location code"
		;;
	esac

	# get the work done
	cd $_ROOT	# 002

	$PROCEDURE $MEDIA $UNIT
	Exit $?
}



Output()
{
	MEDIA=$1
	UNIT=$2

	case "$MEDIA" in
	DISK)	CTRLOUT="dd of=$UNIT/updctrl"
		DATAOUT="dd of=$UNIT/upddata"
		;;
	INET)	RSHCMD="rsh $UNIT -l ris "
		CTRLOUT="$RSHCMD dd of=clients/$IAM.upd/updctrl bs=4k"
		DATAOUT="$RSHCMD dd of=clients/$IAM.upd/upddata bs=4k"
		$RSHCMD mkdir clients/$IAM.upd
		;;
	TAPE)	CTRLOUT="dd of=$UNIT bs=10k"
		DATAOUT=$CTRLOUT
		mt -f $UNIT rew
	esac

	cd /usr/.smdb.
	tar cf - $CTRLFILES | $CTRLOUT
	(cd /;Tarchive $SMDB/CFI $SMDB/UFI) | $DATAOUT
	case "$MEDIA" in
	TAPE)	mt -f $UNIT rew
	esac
}


Parse()
{ (
	IFS=$1;shift
	echo $*
) }


Retrieve()
{
	MEDIA=$1
	UNIT=$2

	case "$MEDIA" in
	INET)
		rsh $UNIT -l ris -n "dd if=clients/$IAM.upd/upddata bs=10k" |
			tar xpf -
		;;
	TAPE)	mt -f $UNIT rew
		mt -f $UNIT fsf
		tar xpf $UNIT
		mt -f $UNIT rew
		;;
	DISK)	tar xpf $UNIT/upddata
		;;
	esac
}



Tarchive()
{
	rm -rf $TARP				#remove
	for INV in $*				#CFI & UFI
	{
		awk '{print $10}' $INV >> $TARP	#append fn
	}
	tar cRf $TARP -				#read back filenames
	rm -f $TARP				#remove the file
}



Globals

[ "$UPDMV_DEBUG" ] || Main $ARGS
