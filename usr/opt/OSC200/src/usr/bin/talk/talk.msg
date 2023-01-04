$
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
$
$ (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ @(#)$RCSfile: talk.msg,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 17:39:29 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2


$ COMPONENT_NAME: TCPIP talk.msg
$ 
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$ 
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 
$ talk.ms 1.4  com/sockcmd/talk,3.1,9021 4/29/90 11:09:20
$ 
$ messages for talk
$ 

$set MS_TALK

$quote "

BAD_SOCK_ERR	"Bad socket"
$ BAD_SOCK_ERR	This message is printed when there is a bad socket.
BIND_SOCK_ERR	"Binding local socket"
$ BIND_SOCK_ERR	This message is printed when binding a name to the local
$ BIND_SOCK_ERR	socket.
BADSCKADR_ERR	"Bad address for socket"
$ BADSCKADR_ERR	This message is printed when the address for the socket
$ BADSCKADR_ERR	is bad.
NOBND_CSCK_ERR	"Couldn't bind to control socket"
$ NOBND_CSCK_ERR	This message is printed when unable to bind a name
$ NOBND_CSCK_ERR	to the control socket.
BADADR_CSCK_ERR	"Bad address for ctl socket"
$ BADADR_CSCK_ERR	This message is printed when the address for the
$ BADADR_CSCK_ERR	control socket is bad.
ADR_PORT_FAM_Z  "addr = %x, port = %o, family = %o zero = "
$ ADR_PORT_FAM_Z	This prints information on the address, port, and
$ ADR_PORT_FAM_Z	family for debugging purposes.
WRITE_ERR       "Error on write to talk daemon"
$ WRITE_ERR	This message is printed on the screen when unable to write
$ WRITE_ERR	to the talk daemon.
DAEMON_ERROR    "Error waiting for daemon response"
$ DAEMON_ERROR	This message is printed on the screen when there is an error
$ DAEMON_ERROR	when waiting for a response from the daemon.
READ_ERR        "Error on read from talk daemon"
$ READ_ERR	This message is printed on the screen when unable to read
$ READ_ERR	from the talk daemon.
HOST  		"talk: %s: Can't figure out network address.\n"
$ ERR_NETWK_ADDR	This message is printed talk can't determine the
$ ERR_NETWK_ADDR	network address for either the local or remote hosts
$ ERR_NETWK_ADDR	involved in the talk.
ERR_NO_SERVICE  "talk: %s/%s: service is not registered.\n"
$ ERR_NO_SERVICE	This message is printed when the ntalk/udp service
$ ERR_NO_SERVICE	is not registered.
USAGE           "Usage: talk user [ttyname]\n"
$ USAGE		This message is printed when the talk program is uncorrectly
$ USAGE		used.
IS_TTY          "Standard input must be a tty, not a pipe or a file\n"
$ IS_TTY	This message is printed when attempting to talk to a pipe
$ IS_TTY	or a file.  You must use talk to a tty.
GO_AWAY         "You don't exist. Go away.\n"
$ GO_AWAY	This message is printed if unable to find an entry in
$ GO_AWAY	/etc/utmp for the initiator of talk.
NO_CONN_YET	"No connection yet"
$ NO_CONN_YET	This message is printed if the current state is that there
$ NO_CONN_YET	is no connection established.
LOST_CONN       "Lost the connection"
$ LOST_CONN	This message is printed when the connection has been lost. 
CONN_CLOSE      "Connection closed. Exiting"
$ CONN_CLOSE	This message is printed when the connection is closed and
$ CONN_CLOSE	the program is exiting.
ERROR_LISTEN    "Error on attempt to listen for caller"
$ ERROR_LISTEN	This message is printed when unable to listen for the caller
$ ERROR_LISTEN	on the socket.
WAIT_TO_RESP	"Waiting for your party to respond"
$ WAIT_TO_RESP	This message is printed on the screen when waiting for the
$ WAIT_TO_RESP	callee to respond with talk.
ERROR_CONN      "Unable to connect with your party"
$ ERROR_CONN	This message is printed when unable to connect to the party's
$ ERROR_CONN	socket.
WAIT_RESP	"Waiting for your party to respond"
$ WAIT_RESP	This message is printed if the current state is that talk is
$ WAIT_RESP	waiting for a response from the callee.
RINGING_AGAIN	"Ringing your party again"
$ RINGING_AGAIN	This message is printed on the screen when trying to contact
$ RINGING_AGAIN	the callee again.
MSG_SUCCESS	"Successfully connected"
$ MSG_SUCCESS	This message is printed on the screen when a connection has 
$ MSG_SUCCESS	been successfully made.  The following messages must be in the
$ MSG_SUCCESS	named order: MSG_SUCCESS, MSG_NOT_HERE, MSG_FAILED, 
$ MSG_SUCCESS	MSG_MACH_UNKN, MSG_PERM_REF, MSG_UNKN_REQ, MSG_BADVERSION, 
$ MSG_SUCCESS	MSG_BADADDR, and MSG_BADCTLADDR.
MSG_NOT_HERE	"Your party is not logged on"
$ MSG_NOT_HERE	This message is printed on the screen when the callee is not 
$ MSG_NOT_HERE	logged in.
MSG_FAILED	"Target machine is too confused to talk to us"
$ MSG_FAILED	This message is printed on the screen when the callee fails
$ MSG_FAILED	for some unknown reason.
MSG_MACH_UNKN	"Target machine does not recognize us"
$ MSG_MACH_UNKN	This message is printed on the screen when the caller's
$ MSG_MACH_UNKN	machine name is unknown to the callee.
MSG_PERM_REF	"Your party is refusing messages"
$ MSG_PERM_REF	This message is printed on the screen when the caller's tty
$ MSG_PERM_REF	doesn't permit announce.
MSG_UNKN_REQ	"Target machine can not handle remote talk"
$ MSG_UNKN_REQ	This message is printed on the screen when the request has
$ MSG_UNKN_REQ	an invalid type value and can't handle remote talk.
MSG_BADVERSION	"Target machine indicates protocol mismatch"
$ MSG_BADVERSION	This message is printed on the screen when the request
$ MSG_BADVERSION	has an invalid protocol version.
MSG_BADADDR	"Target machine indicates protocol botch (addr)"
$ MSG_BADADDR	This message is printed on the screen when the request has an
$ MSG_BADADDR	invalid address value.
MSG_BADCTLADDR	"Target machine indicates protocol botch (ctl_addr)"
$ MSG_BADCTLADDR	This message is printed on the screen when the request
$ MSG_BADCTLADDR	has an invalid control address value.
CONN_TALK_DAEM  "Trying to connect to your party's talk daemon"
$ CONN_TALK_DAEM	This message is printed if the current state is that 
$ CONN_TALK_DAEM	talk is tryint to connect to the callee's talk daemon.
ERR_SND_DEL_R   "send_delete (remote)"
$ ERR_SND_DEL_R	This message is printed when the invitation on the remote
$ ERR_SND_DEL_R	machine is revoked.
ERR_SND_DEL     "send_delete (local)"
$ ERR_SND_DEL	This message is printed when the invitation on the local
$ ERR_SND_DEL	machine is revoked.
CONN_EST        "Connection established"
$ CONN_EST	This message is printed on the screen when the connection
$ CONN_EST	has been established.
UNEXPECT_ERR    "Unexpected error from select"
$ UNEXPECT_ERR	This message is printed when an unexpected error is issued
$ UNEXPECT_ERR	from select.
UNKNOWN_ERR     "Unknown error"
$ UNKNOWN_ERR	This message is printed when calling the p_error routine
$ UNKNOWN_ERR	when there is no other message to be printed.
WAIT_TO_CONN    "Waiting to connect with caller"
$ WAIT_TO_CONN	This message is printed when the current state is waiting
$ WAIT_TO_CONN	to connect with the caller.
ERR_INV_RESP	"Response uses invalid network address"
$ ERR_INV_RESP	This message is printed when the response is using an
$ ERR_INV_RESP	invalid network address.
ERR_CONNECT     "Unable to connect with initiator"
$ ERR_CONNECT	This message is printed when unable to connect with the
$ ERR_CONNECT	caller.
CHK_INVITE	"Checking for invitation on caller's machine"
$ CHK_INVITE	This message is printed when the current state is checking
$ CHK_INVITE	for an invitation on the caller's machine.
CONN_WAIT      "Connection closed. Interrupt to exit..."
$ This message is printed when the other party disconnected and
$ the user must interrupt talk in order to exit.
