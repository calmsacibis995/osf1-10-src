#!/bin/sh
#
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
# @(#)$RCSfile: .mrg..config.sh,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/05/11 21:43:50 $
# 

if (`grep controller $1 | grep -q asc`)
then
	if (`grep bus $1 | grep -q tcds`)
	then
		exit
	fi
	cp $1 $1.orig
	cp /dev/null /tmp/sed.tcds
	cat >> /tmp/sed.tcds << \xyzzy
/controller/{
/asc/{
/tc0/{
/slot[ \t]*0/{
h
s/^.*$/bus		tcds0	at tc0 slot 0	vector tcdsintr/w /tmp/tcds_list
g
s/tc0/tcds0/
s/tcdsintr/ascintr/
s/slot[ \t]*[0123456]/slot 0/
b
}
/slot[ \t]*1/{
h
s/^.*$/bus		tcds1	at tc0 slot 1	vector tcdsintr/w /tmp/tcds_list
g
s/tc0/tcds1/
s/tcdsintr/ascintr/
s/slot[ \t]*[0123456]/slot 0/
b
}
/slot[ \t]*2/{
h
s/^.*$/bus		tcds2	at tc0 slot 2	vector tcdsintr/w /tmp/tcds_list
g
s/tc0/tcds2/
s/tcdsintr/ascintr/
s/slot[ \t]*[0123456]/slot 0/
b
}
/slot[ \t]*3/{
h
s/^.*$/bus		tcds3	at tc0 slot 3	vector tcdsintr/w /tmp/tcds_list
g
s/tc0/tcds3/
s/tcdsintr/ascintr/
s/slot[ \t]*[0123456]/slot 0/
b
}
/slot[ \t]*4/{
h
s/^.*$/bus		tcds4	at tc0 slot 4	vector tcdsintr/w /tmp/tcds_list
g
s/tc0/tcds4/
s/tcdsintr/ascintr/
s/slot[ \t]*[0123456]/slot 0/
b
}
/slot[ \t]*5/{
h
s/^.*$/bus		tcds5	at tc0 slot 5	vector tcdsintr/w /tmp/tcds_list
g
s/tc0/tcds5/
s/tcdsintr/ascintr/
s/slot[ \t]*[0123456]/slot 0/
b
}
/slot[ \t]*6/{
h
s/^.*$/bus		tcds6	at tc0 slot 6	vector tcdsintr/w /tmp/tcds_list
g
s/tc0/tcds6/
s/tcdsintr/ascintr/
s/slot[ \t]*[0123456]/slot 0/
b
}
}
}
}
xyzzy
	sed -f /tmp/sed.tcds $1 > /tmp/$1.tmp
	for I in tcds0 tcds1 tcds2 tcds3 tcds4 tcds5 tcds6
	do
    		export I
    		Z=`grep controller /tmp/$1.tmp | grep -c $I`
    		if (`/sbin/expr $Z - 2 | grep -q 0`)
    		then
			Q=`nl -ba /tmp/$1.tmp | grep controller | grep $I | sed 1d | awk '{print $1}'`
			sed "${Q}s/slot 0/slot 1/" /tmp/$1.tmp > /tmp/$1.tmp1
			mv /tmp/$1.tmp1 /tmp/$1.tmp
    		fi
	done
	sort /tmp/tcds_list | uniq > /tmp/tcds.list
	X=`grep -n 'bus[	 ]*tc0' /tmp/$1.tmp | awk -F: '{print $1}'`
	# If the callout line is there, then skip over it also
	if (`tail +$X /tmp/$1.tmp | head -2 | grep -q callout`)
	then
	    X=`/sbin/expr $X + 1`
	fi
	sed "${X}r /tmp/tcds.list" /tmp/$1.tmp > $1
	rm /tmp/sed.tcds /tmp/tcds_list /tmp/tcds.list /tmp/$1.tmp
fi

