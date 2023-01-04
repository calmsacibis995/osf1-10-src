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
$ @(#)$RCSfile: csplit.msg,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/10/04 21:16:23 $
$
$ 
$ HISTORY
$ 
$ OSF/1 1.2

$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ 1.9  com/cmd/files/csplit.msg, cmdfiles, bos320, 9125320 5/29/91 19:22:40
$
$quote "
$
$ $ Message numbers 100 - 129 reserved for this file.
$
$set MS_CSPLIT

BADLNUM "csplit: The specified line number %s is not valid.\n"
BOGUSERR "csplit: Regular expression error code %d.\n"
CANTCREAT "csplit: Cannot create %s.\n"
CANTOPEN "csplit: Cannot open %s.\n"
FILELIM "csplit: The file limit %d was exceeded.\n"
ILL2ND "csplit: The second byte is not valid: 0%o\n"
$ This message is seen by users of Kanji only.
$ Only Japan needs to translate it.
ILLOFF "csplit: %s: The offset is not valid.\n"
$ Do not move the %s from its current position. It is filled in with
$ the regular expression string.
ILLOPT "csplit: The %s flag is not recognized.\n"
ILLRE ""
$ This is a null message.
ILLRPT "csplit: %s is not valid. Specify a numeric repeat count.\n"
$ %s is replaced with the specified repeat count.
INTSIG "csplit: The program was interrupted at parameter %s.\n"
MISSBRKT "csplit: %s: There is a missing } character.\n"
MISSDEL "csplit: %s: There is a missing delimiter.\n"
NOOP "csplit: Cannot determine what operation to repeat for %s.\n"
OUTRNG "csplit: %s is greater than the current line or the end of the file.\n"
PRELONG "csplit: The prefix %s is too long.\n"
RPT2LNG "csplit: The repeat count %s is larger than 11 bytes.\n"
USAGE "Usage: csplit [-ks] [-fPrefix] [-nNumber] File Parameters ...\n"
ILLCHAR "csplit: Pattern has an illegal character: %s"
BADSUFFIX "csplit: Illegal value for filename digits\n"
SUFLONG "Numerical suffix size %d too long\n"
NOEXEC  "csplit: can't exec %s.\n"
