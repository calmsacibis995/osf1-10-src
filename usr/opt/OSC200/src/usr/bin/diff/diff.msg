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
$ @(#)$RCSfile: diff.msg,v $ $Revision: 4.1.8.2 $ (DEC) $Date: 1993/06/10 18:24:46 $
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
$ diff.msg	1.12  com/cmd/files/diff/diff.msg, bos320 4/28/91 20:43:21	
$
$ messages for command diff (programs diff and diff -h)
$
$set MS_DIFF
$quote "
$
$ Do not translate diff, %s, %1$s, %d, -D, -s, -l, -e
$
ENOI "diff:Cannot use the -D flag when comparing directories.\n"
ENOSL "diff: warning: The -s or -l flag should not be used with the -e flag.\n"
EONLY "Only in %.*s"
EIDENT "Common identical files"
EBINARY "Binary files which differ"
ESUBDIR "Common subdirectories"
USAGE "Usage: diff [-bitw] [-c |-C Lines|-D String|-e|-f|-h|-n] File1 File2\n\ diff [-bilrstw] [-c |-C Lines|-e|-f|-h|-n] [-S File] Directory1 Directory2\n"
ETWO "diff: Specify two file names.\n"
EBADCNT2 "diff: Specify a number with the -c flag.\n"
EBAD "diff: Cannot use the -h flag with the -e, -f, -n, -c, or -D flags.\n"
EMEM "diff: There is not enough memory available now.\n"
SUBDIR "Common subdirectories: %s and %s\n"
FILES "Files %s and %s are identical\n"
DBINARY "Binary files %s and %s differ\n"
NOPROC "diff: Cannot create another process.\n"
SUBARE "Common subdirectories are "
IDENT "%s and %s are identical\n"
SYMLINK "symbolic link files:\n\t"
ESIZE "diff: The specified files are too large. Try the -h flag.\n"
ECREATE "diff: Cannot create %s.\n"
EOPEN "diff: Cannot open %s.\n"
EDIFFH "Cannot find the diffh command.\n"
EARGC "diff: Command usage error.\n"
DMAGIC "Cannot open %s.\n"
DDIFFER1 "%s --> %s "
DDIFFER2 " and %s --> %s differ\n"
DSTDINS "diff: Cannot use the - character for more than one file name.\n"
ENODIF "There are no differences between the files.\n"
DSTDIN "Cannot compare standard input against the directory.\n"
EENDIF "#endif /* %s */\n"
EELSE "#else /* %s%s */\n"
EIFNDEF "#ifndef %s\n"
EIFDEF "#ifdef %s\n"
EARGS "Specify two file names.\n"
ESTDINS "Cannot use the - character for more than one file name.\n"
EACCESS2 "Cannot access or get status of file "
EPROGERR "Program error "
ECHANGE "---change record omitted\n"
ERESYNC "Cannot recover. Try using the bdiff command.\n"
EILLOPT "diff: Illegal option -- %c\n"
EOPTARG "diff: Option requires an argument -- %c\n"
EALLOTMP "diff: Cannot allocate space for temporary file.\n"
ENOPATHMAX "diff: Cannot get _PC_NAME_MAX.\n"
ESLINK "diff: Cannot allocate symbolic link space.\n"
EARGS2 "diffh: Specify two file names.\n"
DIFFHUSE "Usage: diffh [-b] File1 File2\n"
EOPEN3 "diffh: 0653-819 Cannot open file %s.\n"
ISCHR "diff: Cannot compare character special file.\n"
ISBLK "diff: Cannot compare block special file.\n"
ISFIFO "diff: Cannot compare FIFO special file.\n"
NONEW  "diff: Missing new line at the end of file %s.\n"
MDIR	"directory"
MREG	"regular file"
MCHAR	"character special"
MLINK	"symbolic link"
MFIFO	"fifo"
MBLOCK	"block special"
UNKNOWN	"unknown"
TYPDIF	"File %s is a %s while file %s is a %s\n"
