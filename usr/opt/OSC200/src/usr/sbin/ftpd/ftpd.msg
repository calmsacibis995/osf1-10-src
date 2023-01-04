 
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
$ @(#)$RCSfile: ftpd.msg,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/12/04 17:04:51 $
$
$ COMPONENT_NAME: TCPIP ftpd.msg
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
$ message file for ftpd
$

$set MS_FTPD

$quote  "

PEERNAME	"getpeername (%s): %m"
SOCKNAME	"getsockname (%s): %m"
BADVALUE	"ftpd: Bad value for -u\n"
UNKFLAG		"ftpd: Unknown flag -%c ignored.\n"
SIGNAL		"signal: %m"
SOCKOPT		"setsockopt: %m"
EIOCTL		"ioctl SIOCSPGRP: %m"
$ Do not translate above message
SERVREADY	"%s FTP server (%s) ready."
LOSTCONN	"lost connection"
MALLOCERR	"Local resource failure: malloc"
LOGINOK		"Guest login ok, send ident as password."
NOCHANGE	"Can't change user from guest login."
USRUNK		"User %s unknown."
DENIED		"User %s access denied."
PWDREQ		"Password required for %s."
LOGINUSER	"Login with USER first."
LOGINERR	"Login incorrect."
DIRERR		"User %s: can't change directory to %s."
NODIR		"No directory! Logging in with home=/"
NOSEETUID	"Can't set uid."
GUESTLOGIN	"Guest login ok, access restrictions apply."
ANONYMLOGIN	"ANONYMOUS FTP LOGIN FROM %s, id=%s"
LOGIN		"User %s logged in."
LOGINFROM	"FTP LOGIN FROM %s, %s"
NOTPLAINF	"%s: not a plain file."
TRANSCOMP	"Transfer complete (unique file name:%s)."
TRANSOK		"Transfer complete."
BYTES		" (%ld bytes)"
NOCONN		"Can't open data connection."
OPENCONN	"Opening %s mode data connection for %s%s."
BINARY		"BINARY"
USECONN		"Using existing data connection for %s%s."
NODATASOCK	"Can't create data socket (%s,%d): %s."
CANTBUDCONN	"Can't build data connection"
UNIMPTYPESEND	"Unimplemented TYPE %d in send_data"
DATACONNERR	"Data connection"
ERRINPUT	"Error on input file"
NOTYPEE		"TYPE E not implemented."
UNIMPTYPERECV	"Unimplemented TYPE %d in receive_data"
ERRWRITE	"Error writing file"
CNTRCONN	"control connection"
ENDSTATUS	"End of Status"
SERVSTATUS	"%s FTP server status:"
CONN		"     Connected to %s"
ANONYM		"     Logged in anonymously\r\n"
USERLOG		"     Logged in as %s\r\n"
WAITPWD		"     Waiting for password\r\n"
WAITNAME	"     Waiting for user name\r\n"
DATACONN1	"     Data connection open\r\n"
PASSIVE		"     in Passive mode"
NODATACONN	"     No data connection\r\n"
ERRSERV		"Error in server: %s\n"
CLOSECONN	"Closing connection due to server error."
CMDOK		"%s command successful."
NOCMD		"%s command not implemented."
CMDUNK		"'%s': command not understood."
CURRDIR		"\"%s\" is current directory."
FILEEXIST	"File exists, ready for destination name"
GOODBYE		"You could at least say goodbye."
CONNABORT	"Transfer aborted. Data connection closed."
ABORT_SUCCESS	"Abort successful"
STATUS1		"Status: %lu of %lu bytes transferred"
STATUS2		"Status: %lu bytes transferred"
NOPASSCONN	"Can't open passive connection"
ENTRPASS	"Entering Passive Mode (%d,%d,%d,%d,%d,%d)"
NOFILENAME	"Unique file name cannot be created."
UNKERR		"%s: unknown error %d."
ASCII		"ASCII"
LOGINFAIL	"repeated login failures from %s."
REFUSED		"FTP LOGIN REFUSED FROM %s, %s"
PORTCMD_OK	"PORT command successful."
TYPESET_A	"Type set to A."
FORM_IS_N	"Form must be N."
TYPESET_I	"Type set to I."
BYTE_8		"Byte size must be 8."
STUR_F		"STRU F ok."
UNIMP_STRU	"Unimplemented STRU type."
MODE_S_OK	"MODE S ok."
UNIMP_MODE	"Unimplemented MODE type."
ALLO_IGN	"ALLO command ignored."
BADSEQ_CMD	"Bad sequence of commands."
ABOR_OK		"ABOR command successful."
NOOP_OK		"NOOP command successful."
UMASKIS		"Current UMASK is %03o"
BAD_UMASK	"Bad UMASK value"
CHMOD_OK	"CHMOD command successful."
CHMOD_ERR	"CHMOD: Mode value must be between 0 and 0777"
IDLELIMIT	"Current IDLE time limit is %d seconds; max %d"
MAX_IDLE	"Maximum IDLE time must be between 30 and %d seconds"
MAX_IDLE_SET	"Maximum IDLE time set to %d seconds"
UNIX_T_V	"UNIX Type: L%d Version: BSD-%d"
UNIXTYPE	"UNIX Type: L%d"
UNKTYPE		"UNKNOWN Type: L%d"
GOODBYE1	"Goodbye."
LOGUSERPASS	"Please login with USER and PASS."
TIMEOUT		"Timeout (%d seconds): closing control connection."
USER_TIMEOUT	"User %s timed out after %d seconds at %s"
GOODBY		"You could at least say goodbye."
UNK_STATE	"Unknown state in scanner."
NOMEMORY	"Ran out of memory."
CMDS_RECOGNIZED	"The following %scommands are recognized %s."
DIR_COMMENT	"Direct comments to ftp-bugs@%s."
UNK_CMD		"Unknown command %s."
SYNTEX		"Syntax: %s%s %s"
UNIMP1		"%s%-*s\t%s; unimplemented."
SIZENOTIMP	"SIZE not implemented for Type %c."
UNIMP_CMD	"(* =>'s unimplemented)"
CMDMSG		"command: %s"
TYPESET_L	"Type set to L (byte size 8)."
SET_UMASK	"UMASK set to %03o (was %03o)"
UNKNOWN_USER	"Unknown user name after ~"
BAD_DIR		"Bad directory components"
ARG_TOO_LONG	"Arguments too long"
PATH_TOO_LNG	"Pathname too long"
FATAL_NO_MEM	"Out of memory"
GETCONFATTR	"Getconfattr failed with error: %m"
MKD_OK		"MKD command successful."
FCNTL_SET	"fcntl F_SETOWN: %m"

FTPD_FILE_INC	"   File may not have transferred correctly.\r\n"
FTPD_TYPE	"     TYPE: %s"
FTPD_FORM	", FORM: %s"
FTPD_FMT3	" %d"
FTPD_TRANS_MODE "; STRUcture: %s; transfer MODE: %s\r\n"
FTPD_PORT	"     PORT"
FTPD_DIRNAME	"%s\n"
FTPD_BARE_LFS	"WARNING! %d bare linefeeds received in ASCII mode"
FTPD_STAT_FILE	"status of %s:"
FTPD_SETSOCK_TOS "setsockopt (IP_TOS): %m"
FTPD_DEBUG	"<--- %d "
FTPD_DEBUG2	"<--- %d- "
FTPD_CONN_FROM	"connection from %s at %s"
FTPD_3SPACES	"   "
FTPD_RESTARTING	"Restarting at %ld. %s"
FTPD_FMT1	"     %s\r\n"
FTPD_FMT2	" (%s)"
FTPD_FMT4	" (%d,%d,%d,%d,%d,%d)\r\n"
FTPD_FMT5	"%d "
FTPD_FMT6	"%d-"
FTPD_FMT7	"%s%s\n"
FTPD_FMT8	"%c%c%c"
FTPD_FMT9	"%s%c"
FTPD_FMT10	"%lu"
FTPD_FMT11	"%ld"
FTPD_CRNL	"\r\n"
CMDPAS		"command: PASS XXXX"
YYERR		"yacc error: %s"
ENDHELP		"End of help"
LOGINFAIL1	"login from %s failed, user unknown"
LOGINFAIL2	"login %s from %s failed, no password"
LOGINFAIL3	"login %s from %s failed."
LOGINFAIL4	"login from %s failed, user %s unknown"
RETRIEVE1	"retrieve %s "
RETRIEVE2	"retrieve %s/%s "
CMD1		"%s (cwd=%s) "
FAILED		"failed"
FAILED1		"failed: %m"
FAILED2		"failed: not a plain file"
SUCCEEDED1	"succeeded, %d bytes."
STORE1		"store %s "
STORE2		"store %s/%s "
NLST1		"NLST %s"
NLST2		"NLST %s/%s"
LOGOUT		"FTP LOGOUT, %s"
TERMLOGIN	"terminate FTP login as previous user %s"
ENDLOGIN	"End login %s: %m"
