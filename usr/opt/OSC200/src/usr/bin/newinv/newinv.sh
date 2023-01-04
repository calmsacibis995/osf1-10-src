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
# @(#)$RCSfile: newinv.sh,v $ $Revision: 4.2.3.5 $ (DEC) $Date: 1992/10/20 14:52:42 $ 
# @(#)$RCSfile: newinv.sh,v $ $Revision: 4.2.3.5 $ (DEC) $Date: 1992/10/20 14:52:42 $ 
#
#	Merge Inventory with Software Build
#
# Three fields database - new database
#	facility	path	subsetname
#
#	Mods:
#
#	000	Summer	1985	ccb
#		Initial version for ULTRIX-32w V1.0
#	001	7-JAN-1988	ccb
#		Fix initial blank bug caused when user gets out of
#		vi invoked on empty MAS.{EXTRA,DEAD}
#	002	12-Aug-1991	ech
#		added -b flag to sort command on OSF/1 
#


PROG=$0
BYE="failed. Original master inventory saved in .bkp file."
CHANGE=

EDITOR=${EDITOR-vi}

BKP=.bkp
TMP=.tmp
EXTRA=.extra
DEAD=.dead
JOIN=.join
SED=.sed 

# process args.
case $# in
0|1)	echo "Usage: $0 <masterdatabase>  <source dir> <source dir...>"
	exit 1
	;;
*)	MAS=$1
	shift
	;;
esac

# is the old master here?
if [ ! -f $MAS  ]
then
	echo "$PROG: $MAS not found"
	exit 1
else
# backup old master
	cp $MAS $MAS$BKP
fi

# is the image valid.
echo "\nScanning new baselevel files...\c"
> $MAS$TMP
while test $# -gt 0
do
	(cd $1;find . -print)|grep -v '^\.$' >> $MAS$TMP ||
	{
		echo "$BYE"
		exit 1
	}
	shift
done

echo "done."

trap '
	[ -f "$MAS$SED" ] && rm -f $MAS$SED
	[ -f "$MAS$$" ] && rm -f $MAS$$
	 exit 1
' 1 2 3 15 
 

# create sed command file to eliminate blank lines and undesirable spaces/tabs 
cat << 'sedEOF'> $MAS$SED
	/^[ 	]*$/d
	/[ 	][ 	]*/s/[ 	][ 	]*/	/g
	/^	/s/^	//
sedEOF

# eliminate blank lines and illegal spaces/tabs   
sed -f $MAS$SED $MAS > $MAS$$
mv $MAS$$ $MAS 

echo "\nSorting inventories...\c"
sort -o $MAS$TMP $MAS$TMP &
sort -o $MAS -b +1 -2 $MAS
wait
echo "done." 

echo "\nJoining...\c"
join -a1 -a2 -j1 2 -o 1.1 1.2 2.1 1.3 $MAS $MAS$TMP > $MAS$JOIN ||
{
	echo "$BYE"
	exit 1
}

echo "done.

Awking...\c"

# new ones.
awk 'NF == 1 {printf "%s\n", $1}' $MAS$JOIN > $MAS$EXTRA ||
{
	echo "*** 1 Field *** $BYE"
	exit 1
}

# dead ones.
awk 'NF == 3 {printf "%s\t%s\t%s\n", $1, $2, $3}' $MAS$JOIN > $MAS$DEAD ||
{
	echo "*** 3 Fields *** $BYE"
	exit 1
}

# old (continuing) ones.
awk 'NF == 4 {printf "%s\t%12s\t%s\n", $1, $2, $4}' $MAS$JOIN > $MAS ||
{
	echo "*** 4 Fields *** $BYE"
	exit 1
}

echo "done."

#+AUDIT	001 begins here
#		solution is to not edit empty files and not to merge in
#		files that have been emptied down to a single newline.
#
[ -s $MAS$DEAD ] &&
{
	echo "

	*** THERE ARE FILES THAT ARE NO LONGER BEING BUILT ***

	You will be placed in the editor with the inventory
		records corresponding to these files.

	Any records remaining in the file when you exit the
	editor will become part of the new inventory.

	Type <RETURN> when you are ready or CTRL/C to quit: \c "

	read X

	$EDITOR $MAS$DEAD
}

[ -s $MAS$EXTRA ] &&
{
	echo "

	*** THIS BUILD CONTAINS FILES THAT ARE NOT IN THE PREVIOUS BUILD ***

	You will be placed in the editor with the file containing
		the names of these new files.

	If you wish these new files to become part of the product,
		you must convert the line for the wanted files into
		an inventory record.

	Any records remaining in the file when you exit the editor
		will become part of the new inventory.

	Type <RETURN> when you are ready or CTRL/C to quit: \c"
	read X

	$EDITOR $MAS$EXTRA
}

echo "\nMerging...\c"

set xx `wc -c $MAS$EXTRA`
case "$2" in
0|1)	;;
*)	cat $MAS$EXTRA >> $MAS
	CHANGE=1
esac

set xx `wc -c $MAS$DEAD`
case "$2" in
0|1)	;;
*)	cat $MAS$DEAD >> $MAS
	CHANGE=1
esac
#-AUDIT 001	ends here
echo "done." 

# eliminate blank lines and illegal spaces/tabs if there was user interaction. 
[ "$CHANGE" = 1 ] && 
{
	sed -f $MAS$SED $MAS > $MAS$$
	mv $MAS$$ $MAS 
}

# remove leftover sed command file 
rm -f $MAS$SED  

echo "\nSorting...\c"

sort -u -o $MAS -b +1 -2 $MAS ||
{
	echo "BYE"
	exit 1
}

echo "done.

Master inventory update complete.\n" 

exit 0

