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
$ @(#)$RCSfile: msgfac.msg,v $ $Revision: 4.3.11.3 $ (DEC) $Date: 1993/10/18 14:07:56 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2

 
$ COMPONENT_NAME: CMDMSG
$
$ FUNCTION: msgfac.msg
$ 
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ 1.14  com/cmd/msg/msgfac.msg, cmdmsg, bos320, 9125320 5/29/91 10:24:36




$quote "         define quote character.

$set MS_DSPCAT        Define initial set#

M_CAT_NO_OPEN	"dspcat: Cannot open specified catalog: %s\n"
M_NO_ID	"dspcat: Cannot specify message number with the -g flag."
M_USAGE	"Usage:  dspcat [-g] CatalogName [SetNumber [MessageNumber]]"
M_BADSET	"dspcat: Invalid set - catalog only has %d set(s).\n"
M_MISSINGSET	"dspcat: Invalid set - set %d not found.\n"
M_MISSINGMSG	"dspcat: Invalid msg - msg %d not found.\n"
M_ILLEGALSET	"dspcat: Invalid set number.\n"
M_ILLEGALMSG	"dspcat: Invalid msg number.\n"

$set MS_GENCAT        Redefine set#

M_MSG_0	"Usage: gencat CatalogFile [SourceFile...]"
M_MSG_1	"gencat: Cannot open target file %s.\n"
M_MSG_2	"gencat: The realloc system call failed."
M_MSG_3	"gencat: The following message text is longer than the $len value:\n\t%s\n"
M_MSG_5	"gencat: The set number in the following line is not valid:\n\t%s\n"
M_MSG_7	"gencat: The length of the hex number in the following line is not valid.\n\
It must be either two or four digits.\n\t%s\n"
M_MSG_8	"gencat: Reached end of line before the defined closing quote.\n\t%s\n"
M_MSG_9	"gencat: The following message string is longer than NL_TEXTMAX:\n%s\n"
M_MSG_10	"gencat: Reached end of string before expected.\n\t%s\n"
M_MSG_11	"gencat: Internal error: The file pointer offset is not correct."
M_MSG_12	"gencat: Internal error: The file pointer offset is not correct."
M_MSG_13	"gencat: Cannot load the catalog file %s.\n"
M_MSG_14	"gencat: Cannot read the existing catalog file %s.\n"
M_MSG_15	"gencat: There is not enough memory available now."
M_MSG_18	"gencat: The following set uses a symbolic identifier:\n\t%s\n"
M_MSG_19	"gencat: The following message uses a symbolic identifier:\n\t%s\n"
M_NOMSG	"gencat: There is no message defined in a source file."
M_ORDER	"gencat: Set or message numbers are not in ascending sequence after:\n\
\tMessage:  %d,  Set: %d\n\
\t%s"
M_MSG_EILSEQ	"gencat: Invalid character encountered."

$set MS_MKCATDEFS        Redefine set#


MKCATUSAGE	"Usage: mkcatdefs [-h] SymbolName SourceFile[...SourceFile] \n"
MNAMTOOLONG	"mkcatdefs: The message catalog name is too long.\n"
MKCATOPN	"mkcatdefs: Cannot open %s.\n"
BADCONTINUE	"mkcatdefs: Only message text can use '/' to continue: %s\n"
MSGTOOLONG	"mkcatdefs: The message text is too long [%d]:\n\t%s\n"
INVTAG	"mkcatdefs: The symbolic set or message identifier is not valid:\n\
\t%s\n"
TAGTOOLONG	"mkcatdefs: The symbolic identifier is too long [%d]:\n\t%s\n"
INVLDSET	"mkcatdefs: A set number is not in ascending sequence.\n"
MISSSET		"mkcatdefs: A set number or identifier is missing:\n\t%s\n"
ZEROINV	"mkcatdefs: %s is not a valid set identifier.\n"
SETTOOLONG	"mkcatdefs: The set identifier is too long [%d]:\n\t%s\n"
MULTOPN	"mkcatdefs: The message identifier %s is used more than once.\n"
MULTNR	"mkcatdefs: The message id %s has already been assigned to identifier.\n"
NOTASC	"mkcatdefs: The message numbers are not in ascending sequence.\n"
READERRS	"mkcatdefs: There were read errors in file %s.\n"
WRITERRS	"mkcatdefs: There were write errors in file %s.\n"
ERRFND	"mkcatdefs: Errors found: The %s header file is not created.\n"
HCREAT	"mkcatdefs: %s is created.\n"
NOSYMB	"mkcatdefs: No symbolic identifiers; %s is not created.\n"
NOHDR	"mkcatdefs: The %s header file is not created.\n"
IMBCHD	"mkcatdefs: Catname contains invalid International Language character.\n"
IMBCTX	"mkcatdefs: SourceFile contains invalid International Language character in the\
 following text:\n\t%s\n"
MKCATNOMEM      "mkcatdefs: Out of memory."

$set MS_DSPMSG        Define initial set#

M_DSPMSG	"Usage: dspmsg [-s SetNumber] Catalogname MessageNumber ['default'] [Argument...]"
M_REORDER	"\nNone or all arguments must use %n$ format"
M_INVRE		"\n$ missing from %n$ format"
M_REINDEX	"\nInvalid argument index"
M_FORMAT	"\nInvalid format specifier"

$set MS_RUNCAT        Define initial set#
$
$ Do not symbolic names.  runcat is a shell script.
$
1	"Usage: runcat CatalogName SourceFile [CatalogFile]\n"
2	"No $set in source\n"
3	"Can't open %s\n"
