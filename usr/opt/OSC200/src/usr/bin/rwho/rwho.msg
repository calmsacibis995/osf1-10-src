 
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
$ @(#)$RCSfile: rwho.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:48:20 $
$ 
$ COMPONENT_NAME: TCPIP rwho.msg
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
$
$ messages for rwho
$

$set MS_RWHO

$quote "

USAGE		"usage: rwho [-a]\n"
RWHOCD          "/usr/spool/rwho"
$				Perror will print this followed by the system error message
$				when the chgdir() system call fails. If the rwho default
$				dir is changed in rwho.c, this string should be changed to
$				also.
TOOMANYUSERS    "too many users\n"
$				If 1000 or more users are found, the program exits.
$				NUSER is set to 1000 and is the size of an array to hold
$				host names, idle times, and user information.
HOSTNM          "%s:%s"
$				Format for host name and tty name. These are printed in a
$				buffer[BUFSIZ] and then output in TERM format below.
TERM            "%-8.8s %-*s %.12s"
$				Format for user name followed by the host and tty name in
$				the second %s and finally the user login time. The time
$				is Mon dd hh:mm format. This is done using ctime() and
$				printing the 4th - 16th bytes. If use is made of an
$				NL time function, then the code will have be changed based
$				on the format of the time string.
IDLE            " %2d"
$				Format for idle time hours. Only printed if -a flag.
SPC3            "   "
$				Space for hours idle time if not printed. 
SPC1            " "
$				Space after login time between idle time when no -a flag.
IDLE2           ":%02d"
$				Idle time in minutes format.
CRLF            "\n"
$				Line feed after each user record.
