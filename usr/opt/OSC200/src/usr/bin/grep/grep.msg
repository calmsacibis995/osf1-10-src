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
$ @(#)$RCSfile: grep.msg,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 22:02:24 $
$
$ 
$ HISTORY
$ 
$ OSF/1 1.2

$
$ COMPONENT_NAME: (CMDSCAN) commands that scan files
$
$ FUNCTIONS:
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ 1.7.2.3  com/cmd/scan/grep.msg, bos320 4/12/91 15:28:14
$
$quote "         define quote character.
$len
$set MS_GREP        Define initial set#

PARG                 "%s: Do not use the -p flag more than 20 times.\n"
USAGE "Usage: %s [-E | -F] [-c|-l|-q] [-insvxbhwy] [-p parasep] -e pattern_list...\n\t[-f pattern_file...] [file...]\nUsage: %s [-E | -F] [-c|-l|-q] [-insvxbhwy] [-p parasep] [-e pattern_list...]\n\t-f pattern_file... [file...]\nUsage: %s [-E | -F] [-c|-l|-q] [-insvxbhwy] [-p parasep] pattern [file...]\n"
$ $ Check the usage statement.
OPERR                "%s: Cannot open %s.\n"
OVFLO                "%s: The maximum paragraph length is 5,000 characters.\n"
REERR                "%s: The specified pattern is not valid.\n"

$ 0652-036 was not checked as being unique.

OVERFLO              "%s: The number of patterns (including those in -f files) must not exceed 400.\n"

$ These usage forms are for "grep -E" when called as egrep and "grep -F"
$   when called as fgrep.

EUSAGE "Usage: %s [-hisvwxy] [[-bn]|[c|l|q]] -e pattern_list...\n\t[-f pattern_file...] [file...]\nUsage: %s [-hisvwxy] [[-bn]|[c|l|q]] [-e pattern_list...]\n\t-f pattern_file... [file...]\nUsage: %s [-hisvwxy] [[-bn]|[c|l|q]] pattern [file...]\n"

FUSAGE "Usage: %s [-hisvwxy] [[-bn]|[c|l|q]] -e pattern_list...\n\t[-f pattern_file...] [file...]\nUsage: %s [-hisvwxy] [[-bn]|[c|l|q]] [-e pattern_list...]\n\t-f pattern_file... [file...]\nUsage: %s [-hisvwxy] [[-bn]|[c|l|q]] pattern [file...]\n"
STDIN	"(standard input)"
