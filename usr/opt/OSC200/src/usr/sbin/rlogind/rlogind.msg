 
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
$ @(#)$RCSfile: rlogind.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:25:52 $
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ messages for rlogind
$

$set MS_RLOGIND

$quote "

PEERNM      "Couldn't get peer name of remote host: %m"
$               The call to getpeername() failed, i.e., could not connect to
$               rlogin service port. This error message is written to syslog
$		and records the hostname of the remote host.
PEERNMERR   "getpeername"
$               This meesage is printed at the same time as the one above except
$               that it is written by perror on the local host.
SETSOCKOPT  "setsockopt (SO_KEEPALIVE): %m"
$               This warning message is written to syslog when when the
$               socket level option KEEP_ALIVE can't be set. This option
$               forces the connected party to respond to messages to keep the
$               connection alive.
NOPERMIT    "Permission denied"
$               The port address was not a reserved port. (At present < 1024).
$               This message is written to remote host.
ILLEGALPORT "Connection from %s on illegal port"
$		This message is generated at the same time as NOPERMIT. It is
$		written to syslog as a notice recording the hostname of the
$		remote host.
INVIPOPT    "Connection received using IP options (ignored):%s"
$		IP options were received that are unknown/invalid to the 
$		local host.  This message is written as a notice to syslog.
NULLIPOPT   "setsockopt IP_OPTIONS NULL: %m"
$		This message is generated when NULL IP options are received
$		and are required.  This message is written as error to syslog.
ADMISMATCH  "rlogind: Host address mismatch.\r\n"
$		This message is written to the remote host when it's hostname
$		does not match the ip address in the local host's tables.
FATAL       "rlogind: %s: %s"
$               This message preface error messages written to the remote
$               host that include the system error message written by
$               fatalperror().
FATAL2      "%s: Error %d"
$               Same as above except sytem error value is out of range of
$               sys_nerr so the errno value is printed versus a string.
REMUSER	    "remuser too long"
$		This message is displayed if the remote user name is too long.
LOCUSER     "locuser too long"
$		This message is displayed if the local user name is too long.
TTTOOLONG   "Terminal type too long"
$		This message is displayed if the terminal type name is too long.
