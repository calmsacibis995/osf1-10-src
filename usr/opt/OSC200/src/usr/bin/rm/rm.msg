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
$ @(#)$RCSfile: rm.msg,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/10 21:30:27 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2

$ 
$ COMPONENT_NAME: (CMDFS) commands that deal with the file system
$ 
$ FUNCTIONS:
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$ 
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 
$ rm.msg	1.18  com/cmd/files bos320 5/31/91 10:34:20
$
$quote "
$set MS_RM
$		GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate %o %s  %1$s %2$s etc.
$ Do not translate rm. It is the command name.

CANTREMOVE 	"rm: Cannot remove '.' or '..'\n"

NOEXIST 	"rm: File %s does not exist.\n"
$ %s is a file name.

DIRECTORY 	"rm: Remove files in directory %s? "

CANTREAD 	"rm: Cannot read %s.\n"

ISDIRECT 	"rm: Cannot remove directory %s\n"

OVERRIDE 	"rm: Do you wish to override protection setting %1$o for %2$s? "
$ %1$o is a number.  %2$s is a file name or directory name.

NOTREMOVED 	"rm: Cannot remove %s.\n"
$ %s is a file name.

DIRECTORY_MSG 	"%s: Cannot remove directory.\n"
$ Do not translate %s. %s is a directory name.

PERM_MSG "rm: Cannot remove %s.\n"

USAGE 		"Usage: rm [-efirR] [--] File ...\n"

USAGE_RMDIR	"Usage: rmdir [-p] Directory ...\n"

REMOVEDDIR 	"rm: Removing directory %s\n"

FILEREMOVED 	"rm: Removing %s\n"
$ Do not translate %s, or rm.

REMPROMPT 	"rm: Remove %s? "

NOMEM		"rm: out of memory\n"

SYNCERR		"rm: internal synchronization error: %s, %s, %s\n"

NOCHANGE	"rm: %s not changed\n"
