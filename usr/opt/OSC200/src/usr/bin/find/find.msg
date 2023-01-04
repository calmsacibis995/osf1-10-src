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
$ @(#)$RCSfile: find.msg,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/08/03 11:35:24 $
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
$ 1.12.1.1  com/cmd/scan/find.msg, bos320 2/28/91 09:00:24
$
$quote "         define quote character.
$len
$set MS_FIND        Define initial set#
$  Do not translate find.
$  Do not translate %s, %d, etc. These will be replaced by files, numbers etc.

BADPWD               "find: Cannot get the name of the current directory.\n"
$       Do not translate pwd.
USAGE                "Usage: find Path-list Expression-list\n"
$ $ Check usage statement.
PARSERR              "find: There is a command usage error.\n"
CONJ                 "find: There is a missing conjunction\n"
BADSTART             "find: The starting directory is not valid.\n"
OPFOP                "find: An operand follows another operand.\n"
$ NOTE
$ NOTE: the OPFOP message will never happen so don't bother translating
$ NOTE
NONAME               "find: The -name option requires filename argument.\n"
NOUSER               "find: The user name does not exist.\n"
NOGRP 	             "find: The group name does not exist.\n"
NOCREATE             "find: Cannot create file %s.\n"
NOACCES              "find: Cannot access file %s.\n"
BADNODE              "find: %s is not a valid node id.\n"
BADOPTION            "find: %s is not a valid option.\n"
INCSTATE             "find: An expression term lacks a required parameter.\n"
BADSTAT              "find: The status on %s is not valid.\n"
NOCOPY               "find: Cannot copy %s.\n"
OKPROMT              "%1$s %2$s (%3$s) "
$ Do not translate this message.
HUGEDIR              "find: Huge directory %s --call system administrator\n"
$ NOTE
$ NOTE: the HUGEDIR message no longer exists so don't bother translating
$ NOTE
NOCHDIR              "find: Cannot change directory to %s.\n"
NOOPEN               "find: Cannot open file %s.\n"
NOREAD               "find: Cannot read file %s.\n"
BADPATHLEN           "find: The pathname cannot be longer than 512 bytes.\n"
BADTREE              "find: Cannot change to the parent directory.\n"
NOSTAT               "find: Cannot get status on %s.\n"
WRTOUT               "find: Cannot write output.\n"
READIN               "find: Cannot read input.\n"
ASKDEV               "To continue, enter the device or file name.\n"
NOGOOD               "find: Cannot open the specified device.\n"
BADDIR               "find: %s is not a valid directory.\n"
NOCD		     "find: cannot change directory to <%s/%s>:\n "
NOCDROOT	     "find: cannot change directory to </%s>:\n "
$ NOCD and NOCDROOT can have the same message number, there needed to be
$ a separate message when the filename was root or /.
YES  		      "yes"
TOOMANYOPTS	      "find: Too many options\n"
CANTEXEC	      "find: Cannot execute %s"
CANTOPEN	      "find: Cannot open %s\n"
BADPERM		      "find: invalid permissions\n"
BADFILE		      "find: %s%s. Path name too long.\n"
NOMEM		      "find: no memory\n"
BADTYPE		      "find: Unknown -type name %c.\n"
$	Error numbers still available for this command: 085 through 086 
