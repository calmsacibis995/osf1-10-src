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
$ @(#)$RCSfile: who.msg,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/09/03 03:48:55 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2
$
$ COMPONENT_NAME: (CMDSTAT) status
$
$ FUNCTIONS:
$
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ who.ms 1.10  com/cmd/stat,3.1,9021 4/14/90 16:32:08

$quote "         define quote character.

$len
$set MS_WHO        Define initial set#

NOTERM               "who: The process is not attached to a terminal.\n"
BADSWTCH             "who: The flag is not recognized.\n"
NOEXST               "who: %s either does not exist or is not readable.\n"
$
$ %s is replaced with a name.
EMPTYSLT             "There is an empty slot in the accounting file.\n"
ID		" id=%-*.*s"
$ Do NOT translate
TERMID		" term=%d exit=%d"
$ Do Not translate

AMI                  "am i"
$ MAY be translated, if there is any appropriate word as in "who 'am I' ?".

NAME            "Name"
$ yes translate....eight characters maximum
TTYSTATUS       "ST"
$ Do not translate
ACTIVITY        "Activity"
$ Yes translate....8 characters maximum
WHO_LINE	"Line "
$ Do not translate
WHO_TIME        "   Time   "
$ Yes translate....10 characters max
WHO_PID		"PID"
$ Do not translate
WHO_EXIT        "/Exit"
$ Do not translate

$ New or Modified messages for options for 3.2

WHO_HOST	"Hostname"

USAGE		"\nUsage: who [-AabdHhlmMpqrsTtu] [am {i,I}] [utmp_like_file]\n"
$ Do not translate [-AabdHlmpqrsTtu] and [am {i,I}]

ACC_ENT		"\t  -A  Accounting entries\n"
OPTIONS		"\t  -a  All (AbdHlprTtu) options\n"
$ Do not translate (AbdHlprTtu)

BOOTTM		"\t  -b  Boot time\n"
DEADPROC	"\t  -d  Dead processes\n"
HEADERS		"\t  -H  Display a Header (title)\n"
HELP		"\t  -h  Display this Help\n"
PROCCNT		"\t  -l  Login processes\n"
CURTERM		"\t  -m  Information about current terminal (same as 'am i')\n"
$ Do not translate 'am i'

PROCESS		"\t  -p  Processes other than getty or user process\n"
QUICK		"\t  -q  Quick (only user name and number of users currently logged on)\n"
RUNLEVEL	"\t  -r  Run level\n"
SHRTFRM		"\t  -s  Short form (suppress Activity and PID)\n"
$ Do not translate PID

TIMECHG		"\t  -t  Time changes\n"
TTYMSG		"\t  -T  Status of tty \n\t\t(+ writable, - not writable, x exclusive open, ? no response)\n"
USEINFO		"\t  -u  Activity and PID of shell\n"

HOSTNAME	"\t  -M  Display host machine name associated with user\n"
