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
# @(#)$RCSfile: rcinet.sh,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/04 09:27:51 $
# 

START="/sbin/rc3.d/S*inet /sbin/rc3.d/S*rwho /sbin/rc3.d/S*route \
/sbin/rc3.d/S*gateway /sbin/rc3.d/S*settime /sbin/rc3.d/S*named \
/sbin/rc3.d/S*nis /sbin/rc3.d/S*nfsmount /sbin/rc3.d/S*nfs \
/sbin/rc3.d/S*sendmail /sbin/rc3.d/S*xntpd /sbin/rc3.d/S*snmpd \
/sbin/rc3.d/S*inetd"

STOP="/sbin/rc0.d/K*inetd /sbin/rc0.d/K*snmpd /sbin/rc0.d/K*xntpd \
/sbin/rc0.d/K*sendmail /sbin/rc0.d/K*nfs /sbin/rc0.d/K*nfsmount \
/sbin/rc0.d/K*nis /sbin/rc0.d/K*named /sbin/rc0.d/K*gateway \
/sbin/rc0.d/K*route /sbin/rc0.d/K*rwho /sbin/rc0.d/K*inet"

# Get actual configuration 
NUM_NETCONFIG=0
if [ -f /etc/rc.config ]; then
       	. /etc/rc.config
else
       	echo "$0 ERROR: /etc/rc.config defaults file MISSING"
	exit 1
fi
export NUM_NETCONFIG

# if network not configured, just exit
if [ "$NUM_NETCONFIG" = '' -o "$NUM_NETCONFIG" = 0 ]; then
	echo "$0: network is not configured"
	exit 1
fi

#
# start, stop or restart Internet services on this system
#
case $1 in
'start')
	echo "Starting Internet services on this system. Please wait..."
	for f in $START
	do
		if [ -s $f ]; then
			/sbin/sh $f start
		fi
	done
	echo "Internet services on this system are started."
        ;;
'stop')
	echo "Stopping Internet services on this system. Please wait..."
	for f in $STOP
	do
		if [ -s $f ]; then
			/sbin/sh $f stop
		fi
	done
	echo "Internet services on this system are stopped."
        ;;
'restart')
	echo "Stopping Internet services on this system. Please wait..."
	for f in $STOP
	do
		if [ -s $f ]; then
			/sbin/sh $f stop
		fi
	done
	echo "Internet services on this system are stopped."
	echo "Starting Internet services on this system. Please wait..."
	for f in $START
	do
		if [ -s $f ]; then
			/sbin/sh $f start
		fi
	done
	echo "Internet services on this system are started."
        ;;
*)
        echo "usage: $0 {start|stop|restart}"
	exit 1
        ;;
esac
exit 0
