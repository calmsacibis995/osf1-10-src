 
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

$ @(#)$RCSfile: talkd.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:31:13 $

$ COMPONENT_NAME: TCPIP talkd.msg
$ 
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$ 
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 
$ talkd.msg	1.3  com/sockcmd/talkd,3.1,9021 11/8/89 10:43:27
$ 
$ messages for talkd
$ 

$set MS_TALKD

$quote "

SETDEBUG	"setsockopt (SO_DEBUG): %m"
$ SETDEBUG	This message is printed when SO_DEBUG can't be set with a
$ SETDEBUG	setsockopt call.
ANN_WAIT_SYSLOG	"announce: wait: %m"
$ ANN_WAIT_SYSLOG	This message should not be printed.
TALK_MES        "Message from Talk_Daemon@%s at %d:%02d ..."
$ TALK_MES	This message is printed when inviting a user to respond to a
$ TALK_MES	talk invitation.
CONN_REQ        "talk: connection requested by %s@%s."
$ CONN_REQ	This message is printed when inviting a user to respond to a
$ CONN_REQ	talk invitation.
RESPOND_WITH    "talk: respond with:  talk %s@%s"
$ RESPOND_WITH  This message is printed when inviting a user to respond to a
$ RESPOND_WITH  talk invitation.
PROCESS_REQ     "process_request"
$ PROCESS_REQ	This message is printed when in debug mode. 
MSG_TYPE        "type %d"
$ MSG_TYPE	This message is printed to indicate the type of process 
$ MSG_TYPE	requested.
MSG_LV_INV 	"leave_invite"
$ MSG_LV_INV	This message is the first of four messages that must be in a
$ MSG_LV_INV	specific order: MSG_LV_INV, MSG_LOOKUP, MSG_DEL, and
$ MSG_LV_INV	MSG_ANNOUNCE.
MSG_LOOKUP 	"look_up"
$ MSG_LOOKUP	This message prints look_up as the type of process requested.
MSG_DEL	 	"delete"
$ MSG_DEL	This message prints delete as the type of process requested.
MSG_ANNOUNCE 	"announce"
$ MSG_ANNOUNCE	This message prints announce as the type of process requested.
ID_INFO_SYS     "%s: %s: id %d, l_user %s, r_user %s, r_tty %s"
$ ID_INFO_SYS	This message prints information about the local and remote
$ ID_INFO_SYS	users, the id, and the remote tty.
ANS_SYS         "answer %d"
$ ANS_SYS	This message prints the type of answer to a request.
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
ID_SYSLOG       "%s: %s: %s, id %d"
$ ID_SYSLOG	This message is printed to give the id number, and the type and
$ ID_SYSLOG	and response of the request.
BAD_PROTO_VSN   "Bad protocol version %d"
$ BAD_PROTO_VSN	This message says that the protocol version is bad.
BAD_ADDR_FAM    "Bad address, family %d"
$ BAD_ADDR_FAM	This message says that the family address is bad.
BAD_CTL_ADDR    "Bad control address, family %d"
$ BAD_CTL_ADDR	This message says that the family control address is bad.
ERR_ETC_UTMP    "Can't open /etc/utmp"
$ ERR_ETC_UTMP	This message is printed when /etc/utmp cannot be opened.
FIND_MATCH      "find_match"
$ FIND_MATCH	This message is printed when an invitation is found that matches
$ FIND_MATCH	the current request looking for an invitation.
DEL_EXP         "deleting expired entry"
$ DEL_EXP	This message is printed when an entry is too old.
FIND_REQ        "find_request"
$ FIND_REQ	This message is printed when looking for an identical request,
$ FIND_REQ	as opposed to a complimentary one.
OUT_OF_MEM      "insert_table: Out of memory"
$ OUT_OF_MEM	This message is printed when more entries cannot be added to
$ OUT_OF_MEM	list.
DEL_INVITE      "delete_invite(%d)"
$ DEL_INVITE	This message is printed when the invitation is taken back, it
$ DEL_INVITE 	also prints the id number of the invitation.
DELETE_MSG	"delete"
$ DELETE_MSG	This message is printed when the invitation is deleted.
ERR_GETUID      "%s: getuid: not super-user"
$ ERR_GETUID	This message is printed when not running as the superuser.
GTHSTNM_SYSLOG  "gethostname: %m"
$ GETHSTNM_SYSLOG	This message is printed when using gethostname.
CHDIR_SYSLOG    "chdir: /dev: %m"
$ CHDIR_SYSLOG	This message is printed when using chdir.
RECV_SYSLOG     "recv: %m"
$ RECV_SYSLOG	This message is printed when using recv.
SENDTO_SYSLOG   "sendto: %m"
$ SENDTO_SYSLOG	This message is printed when using sendto.
SOCK_SYSLOG     "%s/%s: socket: fd %d: %m"
$ SOCK_SYSLOG	This message is printed when using socket.
SETSCKOPT_SYS   "setsockopt (SO_REUSEADDR): %m"
$ SETSCKOPT_SYS	This message is printed when setsockopt is used with
$ SETSCKOPT_SYS	SO_REUSEADDR.
BIND_SYSLOG     "%s/%s: bind: %m"
$ BIND_SYSLOG	This message is printed when using bind.
