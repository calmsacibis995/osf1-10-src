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
$ @(#)$RCSfile: xargs.msg,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/09/02 16:04:48 $
$
$ 
$ HISTORY
$ 
$ OSF/1 1.2

$
$ COMPONENT_NAME: (CMDSH) Bourne shell and related commands
$
$ FUNCTIONS:
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ 1.7  com/cmd/sh/xargs.msg, bos320 3/18/91 15:36:56
$
$quote "
$set MS_XARGS

LINECNT "%s is not valid. Specify a positive integer after the -l.\n"
$ The %s will be a command line argument like "-l4" or something similar.
$ Do not translate -l.
LINESIZ "%s is not valid. Specify a positive integer after the -s.\n"
$ Do not translate -s
$ The %s will be a command line argument
ARGCNT "%s is not valid. Specify a positive integer after the -n.\n"
$ The %s will be a command line argument
TTYREAD "Cannot find or open /dev/tty for prompting.\n"
$ "/dev/tty" is a file name and shouldn't be translated
UNKWNOPT "The %s flag is not valid.\n"
ARGSIZ "Cannot replace more than 5 instances of %s.\n"
$ The %s will be {} or whatever follows the -i flag on the command line
NOEXEC "The %s command could not be run or was terminated.\n"
NOTFOUND_POSIX "The %s command was not found.\n"
NOEXEC_POSIX "The %s command could not be run.\n"
ARGLIST "A command line size exceeds the specified system limit.\n"
SNGLARG "The size of a single parameter exceeded the command line limit.\n"
MSNGQUOT "An end quote is missing from the string: %s.\n"
DEBUG "xargs doesn't return correct status.\n"
$ %s is replaced by a string of varying length. You may wish to put
$ it on a separate line.
MAXSARGSIZ "A parameter constructed by replacing %s exceeded 255 bytes.\n"
$ The %s will be {} or whatever follows the -i flag on the command line

OVRFLOW "A command line constructed by replacement exceeded 512 bytes.\n"
QUESTION        "?..."
USEMSG  "Usage: xargs [-e[EndOfFileString]] [-E EndOfFileString] \n\
\t[-i[ReplacementString]] [-I ReplacementString] [-l[Number]] [-L Number] \n\
\t[-n Number] [-ptrx] [-s Length] [CommandString] [argument ...]\n"
NEEDARG  "%s: option requires an argument -- %c\n"
POSIX_STOP "%s returned 255 exit code.\n"
