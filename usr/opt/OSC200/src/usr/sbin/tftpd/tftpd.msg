 
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
$ @(#)$RCSfile: tftpd.msg,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/06/25 12:06:18 $
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ messages for tftpd
$

$set MS_TFTPD

$quote "

BAD_BUS         "bad bus open, rc = %d \n"
UNDEFERR        "Undefined error code"
BADFILE         "File not found"
ILLACESS        "Access violation"
DISKFULL        "Disk full or allocation exceeded"
ILLOPE          "Illegal TFTP operation"
BADID           "Unknown transfer ID"
FILEXIST        "File already exists"
BADUSER         "No such user"
MAXTIME         "timer: maxtimeout reached.\n"
SENDERR1        "tftpd: write: %m\n"
RECERR1         "tftpd: read: %m\n"
SENDERR2        "tftpd: write: %m\n"
RECERR2         "tftpd: read: %m\n"
LOGSND		"Sent %s to %s\n"
LOGRCV		"Recv %s from %s\n"
SETDEBUG	"setsockopt (SO_DEBUG): %m"
TFTPD_IOCTL	"ioctl(FIONBIO): %m\n"
TFTPD_RECVFROM	"recvfrom: %m\n"
TFTPD_FORK	"fork: %m\n"
TFTPD_SOCKET	"socket: %m\n"
TFTPD_BIND	"bind: %m\n"
TFTPD_CONNECT	"connect: %m\n"
TFTPD_NAK	"nak: %m\n"
BADOPT		"invalid option -%c (ignored)\n"
SENDFILE	"sending file %s\n"
FINSEND		"finished sending %s\n"
