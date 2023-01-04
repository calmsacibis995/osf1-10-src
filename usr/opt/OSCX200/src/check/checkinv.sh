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
# @(#)$RCSfile: checkinv.sh,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/09/24 20:49:08 $
#
# inventory checker/permission changer script
#
# arguments are:
#
# $1 path to tree to check (should contain ./usr/lib...)
# $2... components to match in the first field
#

if [ $# -lt 1 ]
then
    echo "Error: no install directory specified !"
    echo "Usage: $0 dest_dir [ target keywords ]"
    exit 1
fi

DESTDIR=$1
shift
MATCH_COMPONENTS="$*"

if [ ! -d $DESTDIR ]
then
    echo "Error: could not find: $DESTDIR"
    exit 1
fi

if [ ! -d $DESTDIR/usr ]
then
    echo "Error: could not find: $DESTDIR/usr"
    echo "Error: install tree appears not to be present"
    exit 1
fi

id | grep '(root)' > /dev/null
if [ $? -eq 0 ]
then
	IM_ROOT='yes'
else
	IM_ROOT=''
fi

MATCHING=_specific.inv		# subset of the master inv. applicable
INV_LIST=_check.list		# list of names found in MATCHING
CH_CMD=_chown_cmd		# chown/chgrp command scipt built
CHMOD_CMD=_chmod_cmd		# permissions overrides
ACTUAL_INV=_installed.inv	# installed inventory of files in install tree
MISSING_INV=_missing.inv	# files MIA
NEW_INV=_new.inv		# unexplained new files
SED_CMD_NOT=_sed_cmd_not	# sed filters
SED_CMD_MATCH=_sed_cmd_match	# sed filters
HERE=`pwd`
DEBUG=''

export CHMOD_CMD HERE CH_CMD

#
# create 'short' list of matching inventory entries
#
echo "Creating matching full inventory list"
echo "\tUsing keywords \"$MATCH_COMPONENTS\""

rm -f $SED_CMD_NOT
(
    echo "s/[ \t][ \t]*/ /g"
    echo "/^#/d" 
    for i in $MATCH_COMPONENTS
    do
	echo "/.*!${i}[|\t ]/d" 
    done
) > $SED_CMD_NOT

rm -f $SED_CMD_MATCH
(
    echo "s/ $//"
    echo "/^.*!.*[\t ]/p" 
    echo "/^any[\t ]/p" 
    echo "/^$/d" 
    for i in $MATCH_COMPONENTS
    do
	echo "/.*${i}[|\t ]/p" >> $SED_CMD_MATCH
    done
) > $SED_CMD_MATCH

cat master.inv | 
    sed -f $SED_CMD_NOT | 
    sed -n -f $SED_CMD_MATCH > $MATCHING

#rm -f $SED_CMD_MATCH  $SED_CMD_NOT

echo "\tdone"


#
# create matching inv list
#
echo "Creating name only list"
rm -f $INV_LIST
cat $MATCHING |
    sed -e 's/^.* .* .* .* \(.*\)$/\1/' | sort -u > $INV_LIST
#rm -f MATCHING
echo "\tdone"

#
# create the .new.. files
#
echo "Creating .new.. files"
if [ -z "$IM_ROOT" ]
then
    echo "\tWarning: Script not executed as root, no .new.. files actually created."
    echo "\t Copy commands will be echoed but not executed."
fi
cat $INV_LIST |
while read i
do
    if { echo $i | fgrep ".new.." > /dev/null ; }
    then
	# cut out the first 6 chars of the basename (6 chars in ".new..")
	NewFile=`dirname $i`/`basename $i | cut -c7-`
	if [ "$IM_ROOT" ]
	then
	    cp $DESTDIR/$NewFile $DESTDIR/$i
	else
	    echo cp $DESTDIR/$NewFile $DESTDIR/$i
	fi
    fi
done

#
# create matching chown/chgrp list
#
echo "Creating chown/chgrp script"
rm -f $CH_LIST
cat $MATCHING |
    sed -e "s/[  	]/ /g"  \
	-e "/bin bin/d" \
	-e 's/^.* \(.*\) \(.*\) .* \(.*\)$/chown \1 \3; chgrp \2 \3/' \
	> $CH_CMD
echo "\tdone"

#
# create matching chmod list
#
echo "Creating permissions script"
rm -f $PARM_LIST
cat $MATCHING | awk -f chmod.awk > $CHMOD_CMD
echo "\tdone"

#
# now lets do some real work
#
echo "Changing ownerships"
if [ "$IM_ROOT" ]
then
    chown -R bin $DESTDIR/*
    chgrp -R bin $DESTDIR/*
    cd $DESTDIR
    $DEBUG sh $HERE/$CH_CMD
    cd $HERE
else
    echo "\tWarning: Script not executed as root, skipping this step"
fi
echo "\tdone"

echo "Changing permissions"
if [ "$IM_ROOT" ]
then
    cd $DESTDIR 
    $DEBUG sh $HERE/$CHMOD_CMD 
    cd $HERE
else
    echo "\tWarning: Script not executed as root, skipping this step"
fi
echo "\tdone"

echo "Checking inventory contents of $DESTDIR"
rm -f $ACTUAL_INV $MISSING_INV $NEW_INV
(    # find paths in ./* format
     # but don't include . itself because we don't have a entry
     # for it in master.inv and don't really want one
     cd $DESTDIR
     for i in *
     do
	find ./$i -print 
     done
) | sort  > $ACTUAL_INV
NEW_OR_MISSING=n
comm -23 $INV_LIST $ACTUAL_INV > $MISSING_INV
comm -13 $INV_LIST $ACTUAL_INV > $NEW_INV

#
# handle a special pain in the rear, emacs which has a variable
# file name for two files. Rather than add code to handle wildcards
# I have put in the following special case. (ddhill)
#
fgrep './usr/lib/emacs/etc/DOC-18.' $MISSING_INV > /dev/null
if [ $? -eq 0 ]
then
    fgrep './usr/lib/emacs/etc/DOC-18.' $NEW_INV > /dev/null
    stat1=$?
    fgrep './usr/lib/emacs/src/emacs-18.' $NEW_INV > /dev/null
    if [ \( $? -eq 0 \) -a \( $stat1 -eq 0 \) ]
    then
	sed  \
	    -e '/\/usr\/lib\/emacs\/etc\/DOC-18./d' \
	    -e '/\/usr\/lib\/emacs\/src\/emacs-18./d' \
		< $MISSING_INV > tmp$$
	mv tmp$$ $MISSING_INV
	sed  \
	    -e '/\/usr\/lib\/emacs\/etc\/DOC-18./d' \
	    -e '/\/usr\/lib\/emacs\/src\/emacs-18./d' \
		< $NEW_INV > tmp$$
	mv tmp$$ $NEW_INV
    fi
fi

if [ -s $MISSING_INV ]; then
    echo ""
    echo "PREP FAILURE: `cat $MISSING_INV | wc -l ` Files missing from kit..."
    cat $MISSING_INV
    echo ""
    NEW_OR_MISSING=y
else
    rm -f $MISSING_INV
fi
if [ -s $NEW_INV ]; then
    echo ""
    echo "PREP FAILURE: `cat $NEW_INV | wc -l ` New files in kit..."
    cat $NEW_INV
    echo ""
    NEW_OR_MISSING=y
else
    rm -f $NEW_INV
fi

if [ xn = x$NEW_OR_MISSING ]; then
    echo ""
    echo "PREP: no new or missing files. Good job!"
    echo ""
fi

echo "\tdone"

