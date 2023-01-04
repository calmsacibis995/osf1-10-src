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
$ @(#)$RCSfile: tr.msg,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/10 19:41:48 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2
$
$ @(#)08	1.9  com/cmd/files/tr.msg, cmdfiles, bos320, 9125320 5/30/91 18:36:27
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ RCSfile: tr.msg,v  Revision: 1.4  (OSF) Date: 90/10/07 17:15:46 
$
$quote "
$
$ $ Message numbers 710 - 719 reserved for this file.
$
$set MS_TR

BADSTR "tr: The string within the [ and ] characters is not correct.\n"
BADVAL "tr: Specify a correct octal value.\n"
$
$ The following messages are new or changed in AIX 3.2.
BADUSE2 "Usage: tr [ -c | -cds | -cs | -ds | -s ] [-A] String1 String2\n\
\       tr [ -cd | -cs | -d | -s ] [-A] String1\n"
$
BADCOMB "tr: The combination of options and String parameters is not legal.\n"
$ All of the options specified for the tr command were legal. But the combination of
$ options and String parameters was not legal. For example, the command may have 
$ specified both String1 and String2 with option -d or -s alone, or the command may
$ have specified String1 without String2 and with options -ds or -c .
$ Translate "String" the same way as "String1" and "String2" in the Usage message BADUSE2.
$
EQVINS2 "tr: String2 contains an equivalence class.\n"
$ The  String2  operand contains an equivalence class, but equivalence classes
$ are not permitted in String2 unless the -d and -s options are both used.
$ An equivalence class appears as   [=A=]  where  A  may be any character.
$ Translate "equivalence class" the same way it is translated for messages about
$ locales and regular expressions.
$ Translate "String2" the same way as in the Usage message BADUSE2.
$
REPINS1 "tr: String1 contains a character repetition.\n"
$ The  String1  operand contains a character repetition, but repetitions
$ are not permitted in String1. They may only appear in String2.
$ A character repetition appears as   [A*N]  where  A  may be any character and N is
$ a number.
$ Translate "String1" the same way as in the Usage message BADUSE2.
$
CLASINS2 "tr: String2 contains an invalid character class.\n"
$ The String2 operand contains a character class that is not used correctly.
$ A character class appears as  [:C:]  where  C  must be one of the character class
$ names: alnum, alpha, blank, cntrl, digit, graph, lower, print, punct, space, 
$ upper, and xdigit. If the -d and -s options are both used, any character class
$ may appear in String2. But if -d and -s are not both used, the only character classes
$ that may appear in String2 are [:lower:] and [:upper:], and each of those two character
$ classes can only appear in String2 if the other one appears in the same position in String1.
$ Translate "String2" the same way as in the Usage message BADUSE2.
$
BADORDER "tr: Character range endpoints out of order.\n"
$ A character range specifies a high endpoint that is lower than the low endpoint.
$
NOSPACE "tr: There is not enough storage available.\n"
BADUSE "Usage: tr [-Acs] [String1 [String2]]\n\
\       tr [-cd] [String1]\n"
