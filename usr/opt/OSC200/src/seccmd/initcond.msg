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
$ HISTORY
$ 
$ 
$ @(#)$RCSfile: initcond.msg,v $ $Revision: 4.1.6.2 $ (DEC) $Date: 1992/04/14 17:03:44 $
$ 
$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ OSF/1 Release 1.0
$quote "
$set MS_INITCOND

USAGE1		"Usage: %s [init|getty|init_shell] args . . .\r\n"
USAGE2		"Usage: %s [init|getty] args . . .\r\n"
NORUN		"%s: Cannot run after a session is established.\r\n"
NOBOOT		"cannot get system defaults for boot authentication"
NODATABASE	"init: can't read defaults database\r\n"
BOOTAUTH1	"boot authentication"
NOTREQ		"not required"
BOOTAUTH2	"\nBoot Authentication:\n\n"
ENTERNAME	"Please enter your login name: "
DPASSWD		"Password:"
UNRECOG		"unrecognized user name for boot authentication"
AUTHFAILED	"Authentication incorrect\n\n"
BOOTFAILED1	"boot authentication failed - bad password"
BOOTFAILED2	"boot authentication failed - not authorized"
NOAUTH		"No authorization to boot the system\n\n"
AUTHSUCCEED	"boot authentication succeeded"
NOTLOCATE	"cannot locate Protected Password entry"
NOTPROTECT	"init: cannot find protected password entry for %s\r\n"
NOTERMINAL	"init: cannot update terminal control entry for %s\r\n"
TERMSTAT	"status of terminal"),
FAILURE		"failure"
NOTGETSTATUS	"getty: cannot get status of %s\r\n"
TERNTYPE	"type of file for terminal"
NOTATERM	"not a terminal"
GNOTERM	"getty: %s is not a terminal\r\n"
TERMINAL	"terminal"
NOTRETRIVE	"cannot retrieve"
CHMOD		"change mode of terminal to prevent opening by users"
GNOMODE		"getty: cannot reset mode of %s\r\n"
GNOOWNER	"change owner of terminal to prevent opening by users"
NOGETTERM	"Can not get terminal device assignment entry"
NOTERMSTAT	"Unable to stat terminal device"
NOTCREATE	"Can not create temporary node"
NOCHMOD		"Unable to chmod new device node\n"
NOCHOWN		"Unable to chown new device node\n"
NOLINK		"Unable to link new device node\n"
NOREGADE	"Unable to regade terminal device\n"
NORESET		"Unable to reset terminal caveat set\n"
PREVENTTERM	"prevent use of terminal"
INITNOSTOP	"init: could not stop I/O on %s.\r\n"
NOMEMTTYNAME	"Unable to get memory to build tty name\n"
NOMEMTTYPATH	"Unable to get memory to save tty path\n"
NOLOCIDPW	"Unable to locate user ID %d in /etc/passwd\n"
NOLOCPDB	"Unable to locate user %d in protected password DB\n"
NOSETCLR	"Unable to set clearance\n"
NOSETSENS	"Unable to set sensitivity label\n"
NOSETILB	"Unable to set information label\n"
NOSETPLBL	"Unable to set process labels\n"
NOKERNAUTH	"Unable to set kernel authorizations\n"
NOSETBASEPRIV	"Unable to set base privileges\n"
NOCHDIR		"Unable to change directory to %s\n"
