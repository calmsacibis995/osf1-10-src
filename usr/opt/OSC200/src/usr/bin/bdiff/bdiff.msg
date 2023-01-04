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
$ @(#)$RCSfile$ $Revision$ (DEC) $Date$
$
$ 
$ HISTORY
$ 
$ OSF/1 1.2

$ @(#)62	1.7  com/cmd/files/bdiff.msg, cmdfiles, bos320, 9125320 6/2/91 17:30:16
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 
$ $ Message number 001 - 014 reserved for this file.
$
$set MS_BDIFF
$quote "

$ Messages ARGCNT, BOTHSTDIN, NONNUMLMT, CANTFORK, CANTEXEC, and DIFFFAIL below
$ are obsolete and have been replaced.
ARGCNT "Usage: \tbdiff File1 File2 [Number] [-s]\n\tbdiff - file2 [number] [-s]\n\tbdiff file1 - [number] [-s]\n"
BOTHSTDIN "bdiff: Specify only one file as standard input."
NONNUMLMT "bdiff: Use a numeric value for file segment number of lines."
CANTFORK "bdiff: Cannot create another process now."
CANTEXEC "bdiff: Cannot run %s."
$ %s is replaced by a file name.
DIFFFAIL "bdiff: The %s command failed."
$ The %s is replaced by the command name diff.
$
$ The next six messages below are CHANGED for AIX 3.2.
$ Messages ARGCNT2, BOTHSTDIN2, and CANTFORK2 have newlines added at the end.
$ Message NONNUMLMT2  replaces NONNUMLMT.
$ Messages CANTFORK2 and DIFFFAIL2 have the command name  diff  in place of %s.
ARGCNT2 "Usage: \tbdiff File1 File2 [Number] [-s]\n\tbdiff - file2 [number] [-s]\n\tbdiff file1 - [number] [-s]\n"
BOTHSTDIN2 "bdiff: Specify only one file as standard input.\n"
NONNUMLMT2 "bdiff: Specify file segment size as a line number.\n"
$ This message refers to the  [Number]  option in the  ARGCNT2  message.
$ Number must be a positive, nonzero decimal number.
CANTFORK2 "bdiff: Cannot create another process now.\n"
DIFFFAIL2 "bdiff: The diff command failed.\n"
$ Do not translate  diff . It is a command name.
CANTEXEC2 "bdiff: Cannot run the diff command.\n"
$ Do not translate  diff . It is a command name.
$
$ The next five messages below are NEW for AIX 3.2.
TMPWRTERR "bdiff: Error writing temporary file.\n"
PIPEFAIL "bdiff: The pipe command failed.\n"
UNLNKERR "bdiff: Cannot unlink temporary file.\n"
TMPCREERR "bdiff: Cannot create temporary file.\n"
CANTOPEN "bdiff: Cannot open file %s .\n"
