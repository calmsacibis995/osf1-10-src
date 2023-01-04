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
: "@(#) R4setup.sh		Shell Script for X11R4 build on A/UX"
#
# Run this script as the super-user from the server/ddx/macII directory.
#
(cd ../../../util/cpp; make ; 
    if [ ! -f /usr/lib/big/cpp.save ]; then
	echo installing public domain cpp
	mv /usr/lib/big/cpp /usr/lib/big/cpp.save
	cp /usr/lib/big/cpp.save /usr/lib/big/cpp
    fi
 cp cpp /usr/lib/big/cpp)


# bsdinstall is taken from the Andrew Toolkit, which grants privileges of
# free use.
#
cp ./bsdinstall /etc/bsdinstall

#
# A/UX Release 1.0 flubbed the placement and protection on the following.
#
if [ -f /usr/include/net/un.h ]
then
	echo fixing misplaced un.h 
	chmod 0644 /usr/include/net/un.h
	ln -s /usr/include/net/un.h /usr/include/sys/un.h
fi
#
if [ -f /usr/include/net/unpcb.h ]
then
	echo fixing misplaced unpcb.h
	chmod 0644 /usr/include/net/unpcb.h
	ln -s /usr/include/net/unpcb.h /usr/include/sys/unpcb.h
fi
#
echo "Now do 'make SHELL=/bin/sh BOOTSTRAPCFLAGS=-DmacII World' from"
echo "the top of the source tree."

