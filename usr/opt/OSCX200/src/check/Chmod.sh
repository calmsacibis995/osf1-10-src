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

if [ x$1 = x ]; then
	echo "Usage: $0 directory"
	exit
fi

userName=`id | awk 'BEGIN{FS="("}{print $2}' | awk 'BEGIN{FS=")"}{print $1}'`

if [ xroot != x$userName ]; then
    echo "Must be run as root"
    exit
fi

HERE=`pwd`

if [ ! -d $1 ]; then
	echo "directory $1 does not exist."
	exit
fi

rm -f _Chmod.sh _Chown.sh

rm -f master.s
grep -v \# master.sh > master.s
cat master.s | awk ' $1 !~ /bin/ || $2 !~ /bin/ {printf("chown %s %s;chgrp %s %s\n",$1,$NF,$2,$NF)}' > _Chown.sh
cat master.s | awk -f chmod.awk > _Chmod.sh
chmod +x _Chmod.sh _Chown.sh

cd $1
find usr -print | xargs chown bin
find usr -print | xargs chgrp bin
$HERE/_Chown.sh
$HERE/_Chmod.sh
chown bin .
chgrp bin .
chmod 755 .

#rm master.s _Chmod.sh _Chown.sh
