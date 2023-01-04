 
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

$ @(#)$RCSfile: timed.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:33:28 $

$ COMPONENT_NAME: TCPIP timed.msg
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
$ timed.msg	1.4  com/sockcmd/timed,3.1,9021 11/16/89 17:30:21
$ 

$set MS_TIMED
$quote "

ACKDEND		"acksend: "
AMBIGUOUS	 "?Ambiguous command\n"
BIND		"bind: %m"
BROADCAST	"broadcast: "
CAND		"candidate: "
CANDIDATE	"THIS MACHINE IS A CANDIDATE\n"
CMDS		"Commands may be abbreviated.  Commands are:\n\n"
CONFLICT	"conflict error: no reply to QUIT"
DATECHANGE	"date changed from: %s"
DATECHANGE2	"date changed by %s from: %s"
DATEREQ		"DATEREQ from uncontrolled machine"
DISCARD		"readmsg: discarded: "
ELECTION	"error in election"
ERRCMD		"?Privileged command\n"
ERRHELP		"?Ambiguous help command %s\n"
ERRHELP2	"?Invalid help command %s\n"
ERRNORPY	"election error: no reply to QUIT"
ERRSEND		"error on sending TSP_CONFLICT"
FORWARD		"forwarding date change request for %s"
GARBAGE		"garbage: "
GETADDR		"get broadaddr: %m"
GETCONF		"get interface configuration: %m"
GETDSTADDR	"get destaddr: %m"
GETFLSAG	"get interface flags: %m"
GETHOST		"gethostname: %m"
GETMASK		"get netmask: %m"
IGNACK		"Ignoreack: "
IGN		"IGNORE\n"
INVALIDSTAT	"Attempt to enter invalid state"
INVALIDSTATE	"invalid state %d\n"
KILLERR		 "loop kill error"
KILLERR2	 "loop kill error2"
LOOKING		 "looking for %s from %s\n"
MACK		 "Masterack: "
MALLOC		"malloc failed"
MASTER0		"MASTER\n"
MASTER1		"This machine is master"
MASTER2		 "THIS MACHINE IS MASTER\n"
MAST_COR	 "master correct: %d ms.\n"
MEAS		"measure: %m"
MSGQUEUE	"msgqueue:\n"
NETS		"Looking for nets to master and loops\n"
NETSTAT		"Net status:\n"
NOANS		"no answer from master candidate\n"
NONET		"No network usable"
NOREPLY		"loop breakage: no reply to QUIT"
NOSETTIME	"can't set time"
NOSETTIME2	 "no reply to SETTIME from: %s"
NOSLOT		 "no more slots in host table"
NOTIN		"timed: -i and -n make no sense together\n"
NOTROOT		"Timed: not superuser\n"
NO_INIT_SETTIME		 "no reply to initial SETTIME from: %s"
PRIVILEDGE		 "Could not get priviledged resources\n"
RAWSOCK		"opening raw socket: %m"
READMSG		 "readmsg: "
REVDATE		 "receiving datagram packet: %m"
RM_MACH		 "rmmach: %s\n"
SACK		 "Slaveack: "
SENDQUITERR		 "error on sending QUIT"
SENDTO		 "acksend: sendto: %m"
SENDTO2		 "sendto: %m"
SEQ		"acksend: seq # %d != %d\n"
SETSOCK		"setsockopt: %m"
SIGNORE		 "slave ignored: "
SLAVE1		"SLAVE\n"
SLAVE2		 "THIS MACHINE IS A SLAVE\n"
SOCKET		 "socket: %m"
SRCNORM		 "normal"
SRCFORCE	 "force"
SRCCANCEL	 "cancel"
SRCEXIT		 "timed exiting: SRC stop %s received"
SRCNO		 "timed: SRC not found, continuing without SRC support"
SRVRUNNING		"server already running"
SUBMASTER1		 "THIS MACHINE IS A SUBMASTER\n"
SUBMASTER2		 "submaster correct: %d ms.\n"
TEND		 "Tracing ended on: %s\n"
TIMEDC		 "timedc> "
TIMEEXPIR		 "election timer expired\n"
TOOLARGE		 "clock correction too large to adjust (%d sec)")
TRACE2		"\tnets = %d, masters = %d, slaves = %d, ignored = %d\n"
TSTART		 "Tracing started on: %s\n\n"
UNKCMD		 "?Invalid command\n"
UNKNET		"getnetbyname: unknown net %s"
UNKOPTION		"timed: -%c: unknown option\n"
UNKSERVICE		"unknown service 'timed/udp'"
VERSMISMATCH		 "readmsg: version mismatch\n"
WAIT		 "readmsg: wait: (%d %d)\n"

$set MS_TIMEDC

C_CLOCKDIFF	"clockdiff"
C_ELECTION	"election"
C_HELP		"help"
C_MSITE		"msite"
C_QUIT		"quit"
C_TRACE		"trace"
C_QUES		"?"
H_CLOCKDIFF	"measures clock differences between machines"
H_ELECTION	"causes election timers to expire"
H_HELP		"gets help on commands"
H_MSITE		"finds location of master"
H_QUIT		"exits timedc"
H_TRACE		"turns tracing on or off"
