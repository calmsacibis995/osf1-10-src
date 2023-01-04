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
$ IBM CONFIDENTIAL
$ Copyright International Business Machines Corp. 1989
$ Unpublished Work
$ All Rights Reserved
$ Licensed Material - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ @(#)$RCSfile: ps.msg,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/08/02 21:36:53 $
$
$ error/informational text for the ps command
$

$       NOTE: the portion of a text string that begins with "ps:" should
$               not be changed.  This is the command name.
$

$quote "

$set MS_PS

$
$ The following string gives the syntax of the ps command.  The '%s' fields
$ will contain the invariant part of the command (i.e., the command name and
$ flags).  Translation of the keywords following a flag may be facilitated by
$ reviewing their definitions in the AIX Operating System Commands Reference
$ manual 
$
$ USAGE "Usage: %1$s [%2$s]\n\
$ \t[%5$sTList][%6$sPList][%7$sUList][%8$sGList]\n\
$ \tReports process status.\n"
$
BSD_USAGE       "Usage: %s [%s] [%s format] [%stty] [process number]\n"
SYSV_USAGE      "Usage: %s [%s] [%s format] [%s tlist] [%s plist] [%s ulist] [%s ulist] [%s glist] [%s glist] [%s slist] [%s nlist]\n"

NOT_TTY         "Cannot identify terminal: %s"
FMT_2BIG        "Format specification too large"
UNKNOWN_VAR     "Unknown variable in format: %s"
VAR_2X          "Can't specify a variable twice: %s"
UNKNOWN_TYPE    "Unknown format type: %d"
NO_MEM          "Cannot allocate dynamic memory"
NO_USER		"Cannot find user: \"%s\""
NO_GROUP	"Cannot find group: \"%s\""
NO_UAREA	"Cannot get user info for process %d"
BAD_FMT		"Cannot specify more that one format for output."
NOT_INT		"Argument must be an integer"
BAD_FILE	"Cannot access file: %s"

$ This string is printed in place of the command arguments for processes
$ that are zombies.
ZOMBIE_STR	"<defunct>"

$ This string is printed in place of the command arguments for processes
$ that are exiting.
EXITING_STR	"<exiting>"

$ This string is appended to the command arguments for processes
$ that had errors while getting user info.
ERROR_STR	"<error>"

$ These are the strings for the column titles. Most are mnemonic.
$ If you change these be aware that the length of the fields are
$ important - try to keep them the same size.
COMMAND_MSG	"COMMAND"
LOGNAME_MSG	"LOGNAME"
F_MSG		"F"
STATUS_MSG	"STATUS"
UID_MSG		"UID"
RUID_MSG	"RUID"
SVUID_MSG	"SVUID"
RGID_MSG	"RGID"
SVGID_MSG	"SVGID"
PID_MSG		"PID"
PPID_MSG	"PPID"
CP_MSG		"CP"
WCHAN_MSG	"WCHAN"
CURSIG_MSG	"CURSIG"
PENDING_MSG	"PENDING"
BLOCKED_MSG	"BLOCKED"
IGNORED_MSG	"IGNORED"
CAUGHT_MSG	"CAUGHT"
RUSER_MSG	"RUSER"
GROUP_MSG	"GROUP"
RGROUP_MSG	"RGROUP"
PGID_MSG	"PGID"
JOBC_MSG	"JOBC"
SESS_MSG	"SESS"
TDEV_MSG	"TDEV"
TT_MSG		"TT"
TPGID_MSG	"TPGID"
TSESS_MSG	"TSESS"
STAT_MSG	"S"
PRI_MSG		"PRI"
UPR_MSG		"UPR"
NI_MSG		"NI"
VSZ_MSG		"VSZ"
RSS_MSG		"RSS"
UPROCP_MSG	"UPROCP"
UMASK_MSG	"UMASK"
ACFLG_MSG	"ACFLG"
STARTED_MSG	"STARTED"
ELAPSED_MSG	"ELAPSED"
TIME_MSG	"TIME"
USER_MSG	"USER"
SYSTEM_MSG	"SYSTEM"
CPU_MSG		"%CPU"
MEM_MSG		"%MEM"
SL_MSG		"SL"
PAGEIN_MSG	"PAGEIN"
MINFLT_MSG	"MINFLT"
MAJFLT_MSG	"MAJFLT"
NSWAP_MSG	"NSWAP"
INBLK_MSG	"INBLK"
OUBLK_MSG	"OUBLK"
MSGSND_MSG	"MSGSND"
MSGRCV_MSG	"MSGRCV"
NSIGS_MSG	"NSIGS"
VCSW_MSG	"VCSW"
IVCSW_MSG	"IVCSW"
SCNT_MSG	"SCNT"
PPR_MSG		"PPR"



