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
$ @(#)$RCSfile: sort.msg,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 16:46:32 $
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
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ sort.msg  1.9  com/cmd/files/sort.msg, bos320 5/1/91 08:37:02

$quote "
$
$set MS_SORT
$
$ Note to NLS translator: don't add a new line character to
$ the end of any of these that do not already have one.
$ sort:         Will be added to the front of all the messages
$

USAGE "Usage:  sort\t[-Abcdfimnru]... [-T Directory] [-tCharacter] [-y kilobytes] [-o File]\n\
\t\t[-k Keydefinition]... [[+Position1][-Position2]]... [File]...\n"
$ Translate these words in the "Usage" message:
$  "Usage", "Directory", "Character", and "File" : Translate as usual.
$  "Keydefinition": Definition of a sort key field.
$  "Position": Position of a sort key field in a line.
PATH 	"The path name is too long: %s\n"
KEYS 	"Cannot use more than 10 sort keys.\n"
CHECK 	"Specify only one input file when using the -c flag.\n"
ALLOC 	"There is not enough memory available to perform the sort.\n"
$ Do not translate sort
LOCATE 	"Cannot locate the temporary directory or /tmp.\n"
MALLOC 	"There is not enough memory available to perform the merge.\n"
TOOLONG "A line of the input file contains more than %d characters.\n"
CREATE 	"Cannot create %s\n"
OPEN 	"Cannot open %s\n"
$ %s is a file name. The sort command cannot open the file.
EREAD 	"The read system call failed. Cannot read from %s\n"
$ Do not translate "read" in "read system call". %s is a file name.
STDIN 	"standard input"
$ This is the standard input name. 
$ It may be substituted into message EREAD above.
EWRITE1 "A write error occurred while sorting.\n"
EWRITE2	"A write error occurred while merging.\n"
NULLS 	"Cannot process the specified file. It contains null characters.\n"
DISORDER "This line is out of order: %s\n"
NUNIQUE "Duplicate line: %s\n"
NEWLINE "Warning: A newline character was added to the end of the input.\n"
NEWLINE1 "Warning: A newline character was added to the end of file %s.\n"
