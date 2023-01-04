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
$ @(#)$RCSfile: sed.msg,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 15:03:13 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2

$
$ COMPONENT_NAME: (CMDEDIT) sed.msg
$
$ FUNCTIONS: none
$
$ (C) COPYRIGHT International Business Machines Corp. 1984, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ 1.8  com/cmd/edit/sed.msg, 9123320, bos320 5/1/91 12:15:12 
$
$quote "
$set    MS_SED
$len 
$       Do not translate sed.
$       Do not translate %s, %c, %d, etc. These will be replaced by file names,
$       commands, numbers, etc.
AD0MSG       "sed: Cannot specify an address with the %s function.\n"
AD1MSG       "sed: Function %s allows only one address.\n"
APPNDCNT     "sed: The file was not appended after line %ld.\n"
BADCMND      "sed: %s is not a recognized function.\n"
CGMSG        "sed: Function %s cannot be parsed.\n"
CMNDCNT      "sed: There are too many commands for the %s function.\n"
CREATERR     "sed: Cannot create file %s.\n"
DUPLBL       "sed: There are more than one %s labels.\n"
FILECNT      "sed: The w function allows a maximum of ten files.\n"
FILEOPEN     "sed: Cannot open file %s.\n"
FRSTRE       "sed: The first regular expression cannot be null.\n"
$       The term "regular expression" is a Unix technical term.
HOLDSPC      "sed: The hold space work area is full.\n"
LABELCNT     "sed: There are too many labels in file %s.\n"
LEFTBRC      "sed: There are too many '{'.\n"
LINECNT      "sed: There are too many line numbers specified.\n"
LINELEN      "sed: The output line exceeds maximum length.\n"
LTLMSG       "sed: The label %s is greater than eight characters.\n"
MALLOCMSG    "sed: Memory allocation failed.\n"
OPENERR      "sed: Cannot find or open file %s.\n"
PATTFIL      "sed: Cannot open pattern file %s.\n"
PATTSPC      "sed: The pattern space work area is full.\n"
READCNT      "sed: There are too many reads after line %ld.\n"
RGHTBRC      "sed: There are too many '}'.\n"
TMMSG        "sed: The %s function did not end the command line,\n\
\tor was not preceded by a blank character.\n"
TOOMANYCMDNS "sed: There are more than 99 commands in pattern file %s.\n"
UNDFNLBL     "sed: %s is not a defined label.\n"
USAGE	     "Usage: sed [-n] [-e script] [-f source_file] [file...]\n"
NONL         "sed: Missing newline at end of file %s.\n"
FNDESC       "sed: Found escape character at end of editing script.\n"
