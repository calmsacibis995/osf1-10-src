 
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
$ @(#)$RCSfile: rshd.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:27:10 $
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$len 
$quote "         define quote character.
$set MS_RSHD        Define initial set#
$ Do not translate system call names (such as getpeername)
$ Do not translate anything that is in all caps (such as SO_DEBUG)

PEERNMERR            "rshd: %s: "
$               The call to getpeername() failed, i.e., could not get the name 
$               of the peer connected to socket 0. This message prints argv[0], 
$               the process name, to stderr which should be connected to the 
$               remote host. 
PEERNM               "rshd: The getpeername system call failed."
$               This meesage is printed at the same time as the one above except 
$               that it is written by perror on the local host. 
SETSOCKOPT           "rshd: Cannot set the SO_KEEPALIVE socket level option: %m."
$               This warning message is written to syslog when when the 
$               socket level option KEEP_ALIVE can't be set. This option 
$               forces the connected party to respond to messages to keep the 
$               connection alive. 
SETSOCKLING          "rshd: Cannot set the SO_LINGER socket level option: %m."
$               This warning message is written to syslog when when the 
$               socket level option SO_LINGER can't be set. This option 
$               would block the process from closing the socket if unsent 
$               messages were still queued on the socket. 
FRMADDRERR           "rshd: The from address is not valid.\n"
$               This message is written to syslog prior to exit when 
$               the form socket family is not AF_INET or port > 1023 
$               (reserved ports). 
RDERR                "rshd: Cannot read the from socket.  The error is %m."
$               A read error has occurred on the from socket. Exits. 
STDERR               "rshd: Cannot get a reserved port for standard error: %m."
$               This message written to syslog when a reserved port can't be 
$               obtained for stderr. (rresvport() fails). Exits. 
NO2PORT              "rshd: The second port is not reserved.\n"
$               This message written to syslog when port number read from 
$               socket is not a reserved port. (< IPPORT_RESERVED). 
NO2CONN              "rshd: Cannot connect the second port.  The error is %m."
$               This message written to syslog when an attempt to connect 
$               to the port read from the remote fails. 
LOGINERR             "rshd: The remote user login is not correct."
$               The user read from the remote does not have an entry in 
$               etc/passwd. 
NODIR                "rshd: No remote directory.\n"
$               The home dir read from the passwd file for the user does 
$               not exist or the user does not have execute permission for 
$               a dir in the path. 
NOPERMIT             "rshd: Permission is denied.\n"
$               The remote user is not super user and the remote host is 
$               not in host.equiv or the remote host and user are not in 
$               the .rhosts file in users home directory. 
PIPERR               "rshd: The pipe system call failed.\n"
$               This message written to the remote user when pipe() fails. 
RETRY                "rshd: Cannot run another process at this time.\n"
$               This message written to the remote user when fork() fails. 
TOOLONG              "rshd: %s is too long for its associated buffer."
$               This message is written to remote when the remote user, 
$               local user, or command string longer than its associated 
$               buffer (16,16,NCARGS). 
USAGE                "Usage: rshd [-lns]"
CONN_IP              "rshd: Received the connection.  IP options %s ignored."
SETSCKIP             "rshd: setsockopt IP_OPTIONS NULL: %m"
$ Do not translate
ILLEGAL_PORT         "rshd: Received a connection from %s on a port that is not valid."
NOADDR               "rshd: Cannot look up the address for %s."
NOADDR2              "rshd: Cannot look up the address for your host."
HOSTADDR1            "rshd: Host address %1$s is not listed for host %2$s."
HOSTADDR2            "rshd: There is a host address that does not match."
HOSTUNK              "rshd: The host name for your address is not known.\n"
LOGINDISABLE         "rshd: Logins not enabled currently.\n"
ROOT_SHELL	"ROOT shell from %s@%s, comm: %s\n"
NOPRPWENT	"No protected password entry.\n"
PRPWSYNC	"Protected password data out of sync.\n"
DISABLED	"Account is disabled.\n"
RETIRED		"Account has been retired.\n"
UNAVAIL		"Account is unavailable at this time.\n"
CLEARANCE	"Can't set process clearance.\n"
SETSL		"Can't set process sensitivity label.\n"
SETIL		"Can't set process information label.\n"
