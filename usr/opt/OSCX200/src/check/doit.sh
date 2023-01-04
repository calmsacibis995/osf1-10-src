#!/usr/bin/ksh 
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

# make new list
if [ x$1 = x ]; then
	echo "Usage: $0 directory"
	exit
fi

HERE=`pwd`

if [ ! -d $1 ]; then
	echo "directory $1 does not exist."
	exit
fi

# change the tree to be good
sh ./Chmod.sh $1

cd $1
find ./usr -print >> $HERE/newlisting 2>&1

rm -f $HERE/newlslisting

for i in `cat $HERE/newlisting`
do
	ls -ldg $i >> $HERE/newlslisting
done

rm $HERE/newlisting

cd $HERE
cat newlslisting | awk '{printf("%s\t%s\t%s\t%s\n",$3,$4,$1,$9)}' > newlist
rm newlslisting

# sort the lists for use with comm
cat master.sh | grep -v \# | awk '{print $NF}' | sort > master.s
cat newlist | awk '{print $NF}' | sort > newlist.s

NEW_OR_MISSING=n
comm -23 master.s newlist.s > missing.s
if [ -s missing.s ]; then
    echo ""
    echo "PREP FAILURE: Files missing from kit..."
    cat missing.s
    echo ""
    NEW_OR_MISSING=y
else
    rm -f missing.s
fi
comm -13 master.s newlist.s > new.s
if [ -s new.s ]; then
    echo ""
    echo "PREP FAILURE: New files in kit..."
    cat new.s
    echo ""
    NEW_OR_MISSING=y
else
    rm -f new.s
fi

if [ xn = x$NEW_OR_MISSING ]; then
    echo ""
    echo "PREP: no new or missing files. Good job!"
    echo ""
fi

rm -f misdifs.s misdifs.ss same.s newlist.s master.s
