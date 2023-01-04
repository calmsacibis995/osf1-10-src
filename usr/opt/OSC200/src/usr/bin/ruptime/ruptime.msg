 
$ *****************************************************************
$ *                                                               *
$ *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
$ *                                                               *
$ *   All Rights Reserved.  Unpublished rights  reserved  under   *
$ *   the copyright laws of the United States.                    *
$ *                                                               *
$ *   The software contained on this media  is  proprietary  to   *
$ *   and  embodies  the  confidential  technology  of  Digital   *
$ *   Equipment Corporation.  Possession, use,  duplication  or   *
$ *   dissemination of the software and media is authorized only  *
$ *   pursuant to a valid written license from Digital Equipment  *
$ *   Corporation.                                                *
$ *                                                               *
$ *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
$ *   by the U.S. Government is subject to restrictions  as  set  *
$ *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
$ *   or  in  FAR 52.227-19, as applicable.                       *
$ *                                                               *
$ *****************************************************************
$
$ HISTORY
 
$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0

$
$ @(#)$RCSfile: ruptime.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:48:11 $
$
$ COMPONENT_NAME: TCPIP ruptime.msg
$
$ FUNCTIONS:
$
$ ORIGINS: 10  26  27
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ messages for ruptime
$ The range of message numbers assigned to this file are 401-405
$
$len 
$quote "         define quote character.
$set MS_RUPTIME        Define initial set#
$ Do not translate ruptime, /usr/spool/rwho

USEAGE               "Usage: ruptime [-arlut]\n"
$ Usage message printed when invoked with incorrect arguments. 
$ -a: all users, -r: reverse sort 
$ -lut: sort options (load, users, uptime) 
RWHODERR             "/usr/spool/rwho"
$ Can't change directory to usr/spool/rwho (exist/permission) 
RWHODERR2            "ruptime: Check path /usr/spool/rwho name and permissions.\n"
$ Can't open directory usr/spool/rwho. 
TOOMANYHOSTS         "too many hosts\n"
$ Number of hosts = NHOSTS (currently 2048), the maximum 
$ allowed. Originally limited due to memory concerns ? 
$ Does not exit, just limits summary to 2048 hosts. 
NOHOSTS              "ruptime: no hosts!?!\n"
$ Didn't find any hosts listed in usr/spool/rwho. Exits. 
DNTIM                "%-*.*s%s\n"
$ Format for host name and the amount of time host has been 
$ down. (Current time - last receive time). Time is formatted 
$ in routine interval(). 
DNLAB                "down"
$ Do not let translation exceed 4 bytes
$ $ Tag printed in RAWTIM, DYHRMN, or HRMIN format below. 
$ $ If the size of this label is changed, (4 bytes), the size 
$ $ of the static string allocated in interval() should be  
$ $ changed accordingly or strange things will happen. 
UPTIME               "%-*.*s%s,  "
$ Format for hostname and uptime. (Uptime formatted in 
$ routine interval(). 
UPLAB                "  up"
$ Do not let translation exceed 4 bytes
$ $ Tag printed in RAWTIM, DYHRMN, or HRMIN format below. 
$ $ If the size of this label is changed, (4 bytes), the size 
$ $ of the static string allocated in interval() should be  
$ $ changed accordingly or strange things will happen. 
USER1                "%4d user, "
$ Format for number of users when number = 1. Printed after 
$ "up" tag above. 
USER2                "%4d users,"
$ Same as above except for more than 1 user. 
LOAD                 "  load %*.2f, %*.2f, %*.2f\n"
$ Format for average load statistics. For aix = cpu busy or 
$ awaiting I/O versus idle time. Values are one, 5, and  
$ 15 minute averages. 
RAWTIM               "   %s ??:??"
$ Indicates up or down time is out of range. The string 
$ "up" or "down" is printed at the %s. Range is 0-365 days. 
DYHRMN               "%s %2d+%02d:%02d"
$ Format for up/down time when time is greater than 1 day. 
$ Prints "up" or "down" followed by days+hours:minutes. 
HRMIN                "%s    %2d:%02d"
$ Format for up/down time when time is less than 1 day. 
$ Pints "up" or "down" followed by hours:minutes. 
NOMEM		     "Can't allocate memory\n"




