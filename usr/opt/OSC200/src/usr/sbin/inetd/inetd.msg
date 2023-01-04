 
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
$ @(#)$RCSfile: inetd.msg,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/05/26 17:46:12 $
$
$ COMPONENT_NAME: TCPIP inetd.msg
$
$ FUNCTIONS: msetsockopt
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
$ messages for inetd
$

$set MS_INETD

$quote "

$               GENERAL: Debug messages are written by syslog under
$               LOG_DEBUG. The debug option is enabled by a cmd line arg.
USAGE		"usage: inetd [-d]"
SELECTWRN   "select: %m\n"
$               Select call failed in loop checking for ports requiring
$               service. This message is written to syslog.
NEEDSERV    "someone wants %s\n"
$               Debug message written when select() call has identified
$               a connection requiring service. The type of service
$               corresponding to the port is printed.
ACCEPT      "accept, ctrl %d\n"
$               Debug message printing the results of an accept() call.
$               This call returns a socket file descriptor for the 1st
$               connection from possibly a que which forms a connection
$               to the port.
ACCEPTWRN   "accept: %m"
$               This message is output by syslog/LOG_WARNING if the accept
$               call fails to make a connection.
SERVFAIL    "%s/%s server failing (looping), service terminated\n"
$               This message is written when the service requires a fork()
$               (in general, not a builtin) and the number of servers started
$               within the past 60 sec is up to 40. Limits the rate of
$               process creation to 40 processes/sec.
PWNAM       "getpwnam: %s: No such user"
$               This message is written by the fork() child process created
$               to provide requested service. The user name was not found
$               in the passwd file by getpwnam(). The user name is usually
$               "root".
EXECL       "%d execl %s\n"
$               Debug message written for forked servers indicating pid and
$               and server. This message has been commented out where print
$               was done with fprintf(stderr,...). Second write is to
$               syslog/LOG_DEBUG.
EXECV       "execv %s: %m"
$               Syslog message: written after the server has been exec'ed
$               with the server name and errno if any.
REAPED      "%d reaped\n"
$               Debug message: the child pid is printed when the child dies.
EXITSTAT    "%s: exit status 0x%x"
$               Syslog message: the server name and exit code are printed
$               when a process dies with a non zero exit code.
RESTORED    "restored %s, fd %d\n"
$               Debug message: prints service and file descriptor. Indicates
$               the port is ready for a new connection.
CONFMSG     "%s: %m"
$               Syslog message: The configuration file name and error are
$               written if the file could not be opened.
UNKPROTO    "%s: unknown protocol: %s"
$               Syslog message: A protocol in the config file was not found
$               in etc/protocols by getprotobyname(). The service and protocol
$               are printed.
UNKSERV     "%s/%s: unknown service"
$               Syslog message: A service in the config file was not found
$               in etc/services by getservbyname(). The service/protocol
$               pair are printed in the message.
UNKKIND     "%s: unknown kind for service: %d"
$               Syslog message: Written when the kind of socket is niether
$               sunrpc or socket. The service and kind number are printed.
SOCKSETUP   "%s/%s: setup_socket: %m"
$               Syslog message: The call to set up a socket of kind "socket"
$               failed (routine setup_socket()). The service/protocol pair
$               are printed in the message.
UNKSETUP    "setup: unknown kind: %d "
$               Syslog message: This message printed when the kind is not
$               socket or sunrpc. (1 or 2).
SOCKERR     "%s/%s: socket: %m"
$               Syslog message: Printed when an Internet socket can not be
$               created for a server. The requested service/protocol pair
$               is printed.
SETSOCK     "setsockopt (SO_DEBUG): %m"
$               Syslog message: Printed when the socket level option SO_DEBUG
$               could not set for the socket. SO_DEBUG enables debugging in
$               underlying protocols. This message is also used in the
$               sunrpc routine.
SETSOCKOPT  "setsockopt (SO_REUSEADDR): %m"
$               Syslog message: Printed when the socket level option
$               SO_REUSEADDR could not set for the socket. Allows reuse
$               of local addresses in bind() calls.
BIND        "%s/%s: bind: %m"
$               Syslog message: Printed when the bind() call on the socket
$               fails. Bind names the unnamed socket, in this case ties it
$               to the Internet port.
MEMERR      "Out of memory."
$               Syslog message: No memory is availble for a new server table
$               entry or for service name string (used in 2 places). exits().
UNKINTSERV  "internal service %s unknown\n"
$               The service designated as builtin in the config file is not one
$               the internal services.
TIMERR      "Unable to get time of day\n"
$               The gettimeofday() system call failed.
PRNTSERV    "%s: %s proto=%s, wait=%d, user=%s builtin=%x server=%s\n"
$		format string for print_service function to print
$		info to stderr	
NO_ROOT         "NOT super-user\n"
REDO	     "REDO"
FREE	     "FREE"
ADD	     "ADD"
SETGID	    "setgid(%d): %m"
$		Unable to set the group id to the specified value
SETUID	    "setgid(%d): %m"
$		Unable to set the user id to the specified value
TOOMNYSVC   "%s/%s: too many services (max %d)"
$		In order to limit the amount of imposed system load a 
$		count is maintained on the number of servers started up.
GETSOCKNAME "%s/%s: getsockname: %m"
$		Failure to get socket name via a call to getsockname when
$		attemptimg to start up an rpc based server.
BADHIVERS   "%s/%s: bad high version number"
$		An rpc based server has explicitly specified a version 
$		number.  The high version is invalid.
BADVERS     "%s/%s: bad version number"
$		An rpc based server has explicitly specified a version 
$		number.  The version is invalid.
