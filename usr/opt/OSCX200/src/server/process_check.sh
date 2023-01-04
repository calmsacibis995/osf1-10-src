#!/bin/csh 
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
# @(#)$RCSfile: process_check.sh,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 92/10/06 08:00:32 $
# 

rm -f check_list
rm -f check_list.unique
rm -f check_list.duplicates
rm -f check_list_libs
# change all the .a's to .so's
ed check_file << END
1,\$s/\\.a/.so/g
w
q
END
foreach i  (`cat check_file`)
	# rebuild the .so name to be lib_externals.c
	set tmp=`echo $i:t | sed -e s/lib// | sed -e s/\\.so// | sed -e s/\$/_externals.c/ `
	set tmp=${i:h}/${tmp}
	/bin/test -r $tmp
	if ( $status == 0 ) then
	    # strip out all the extern void lines
	    sed -n -e "s/extern void \* //p" $tmp >> check_list
	    # make a second list with the file names added 
	    sed -n -e "s/extern void \* //p" $tmp > tmp_file
	    cat tmp_file | sed -e s\?\$\?\	$i\? >> check_list_libs
	else
	    echo Cannot find file $tmp
	endif
end
# find duplicate globals
sort check_list | uniq -d > check_list.duplicates
# sort the file name list so we can use this file to look up where
# the duplicate names came from (I know, this is more functionality than
# ld provides...
sort check_list_libs > tmp
mv tmp check_list_libs
/bin/test -s check_list.duplicates
if ( $status == 0 ) then
	echo Duplicates found.
	foreach i (`cat check_list.duplicates`)
		grep \^${i} check_list_libs
	end
else
	echo Congrates\! No duplicates found.
endif
