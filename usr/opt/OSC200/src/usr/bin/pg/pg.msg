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
$ @(#)$RCSfile: pg.msg,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/10 20:55:40 $
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
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ pg.msg\t1.4  com/cmd/scan,3.1,9013 3/11/90 12:23:58
$
$quote "         define quote character.
$len
$set MS_PG        Define initial set#

$      The term "regular expression" is a Unix technical term.
$      Do not add a new line unless it is included in the source.
$      Do not translate %s, %d, %1$s, etc.
$
$ The help messages, H01 - H24 cannot be expanded with new lines.
$ The text MUST all fit on one line for each of these messages.

1                    "The pattern is not found.\n"
USGE                 "Usage: pg [-Number] [-p String] [-cefns] [+Line] [+/Pattern/] Files\n"
$ $ Check the Usage statment.
PATLONG              "The pattern is too long.\n"
PRMPTLNG             "The prompt is too long.\n"
SKIPBW               "...skipping backward\n"
SKIPFW               "...skipping forward\n"
OPSAVERR             "Cannot find or open the save file.\n"
SAVFIL               "Saving file %s.\n"
SAVED                "Saved\n"
$ This comes back to the screen to show successful completion of the file save.
EXECFAIL             "Cannot run the command.\n"
11                   "The range endpoint is too large.\n"
NOPREV               "There is not a previous file.\n"
NONEXT               "There is not a next file.\n"
DIRECT               "%s is a directory.\n"
SPECFIL              "Special files are handled only as standard input.\n"
16                   "The number is not valid.\n"
NOTEMP               "Cannot create the temporary file.\n"
TEMPERRW             "Cannot write to the temporary file.\n"
TEMPERRR             "Cannot read the temporary file.\n"
PIPSAV               "The piped input is only partially saved.\n"
MEMOUT               "There is not enough memory available now. Stopped at line %d.\n"
NOREOPN              "Cannot open the standard output.\n"
NXTFIL               "The next file is %s.\n"
25                   "The \\digit is out of range.\n"
$ Do not translate \digit
TERMTYP              "Cannot find the terminal type.\n"
36                   "Delimeter is not valid or not found.\n"
41                   "There is not a remembered search string.\n"
42                   "There is a missing \\( or \\).\n"
$ Do not translate \( or \)
43                   "There are too many ( characters.\n"
44                   "There are more than two numbers in \\{ \\}.\n"
$ Do not translate \{ or \}
45                   "} is expected after \\\n"
$ Do not translate { or \
46                   "The first number exceeds the second in \\{ \\}.\n"
$ Do not translate \{ or \}
48                   "The range endpoint is not valid.\n"
49                   "There is a missing [ or ] character.\n"
50                   "There is a regular expression overflow.\n"
H01                  "-------------------------------------------------------\n"
$    This message is to form a frame for the menu. Do not translate.
H02                  "   h               help\n"
$    Start the items in the same columns.
$    Do not translate h.
H03                  "   q or Q          quit\n"
$    Start the items in the same columns.
$     Do not translate the first q or Q.
H04                  "   <blank> or \\n   next page\n"
$    Start the items in the same columns.
$     Do not translate \n
H05                  "   l               next line\n"
$    Start the items in the same columns.
$     Do not translate the first l
H06                  "   d or ^D         display half a page more\n"
$    Start the items in the same columns.
$     Do not translate the first d and ^D
H07                  "   . or ^L         display current page again\n"
$    Start the items in the same columns.
$     Do not translate . and ^L
H08                  "   f               skip the next page forward\n"
$    Start the items in the same columns.
$     Do not translate the first f
H09                  "   n               next file\n"
$    Start the items in the same columns.
$     Do not translate the first n
H11                  "   p               previous file\n"
$    Start the items in the same columns.
$     Do not translate the first p
H12                  "   $               last page\n"
$    Start the items in the same columns.
$     Do not translate $
H13                  "   w or z          set window size and display next page\n"
$    Start the items in the same columns.
$     Do not translate w, z on the left.
H14                  "   s savefile      save current file in savefile\n"
$    Start the items in the same columns.
$     Do not translate the first character s
H15                  "   /pattern/       search forward for pattern\n"
$    Start the items in the same columns.
$      Do not translate character /
$      "pattern" refers to the pattern in the Usage statement. Keep consistent.
H16                  "   ?pattern? or\n"
$    Start the items in the same columns.
$     Do not translate the character ?
$      "pattern" refers to the pattern in the Usage statement. Keep consistent.
H17                  "   ^pattern^       search backward for pattern\n"
$    Start the items in the same columns.
$     Do not translate the character ^
$      "pattern" refers to the pattern in the Usage statement. Keep consistent.
H18                  "   !command        run command\n"
$    Start the items in the same columns.
$     Do not translate the character !
H19                  "\n"
$ This line contains only a new line ... no text.
H20                  "Most commands can be preceded by a number, as in:\n"
H21                  "+1\\n (next page); -1\\n (previous page); 1\\n (page 1).\n"
$ You may translate only the words "next page", "previous page", "page 1"
$ This line may not exceed 80 characters, you cannot create a new line.
H22                  "\n"
$ This line contains only a newline...no text.
H23                  "See manual page for more details.\n"
H24                  "-------------------------------------------------------\n"
BADREG               "The regular expression is not valid.\n"
SAYEOF               "(EOF)"
$ Do not translate.
CANTFORK             "Cannot create a process.\n"
