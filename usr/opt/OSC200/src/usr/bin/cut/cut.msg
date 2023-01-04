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
$ @(#)$RCSfile: cut.msg,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 17:39:00 $
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
$ cut.msg	1.7  com/cmd/files,3.1,9021 3/19/90 13:09:46
$
$quote "
$set MS_CUT
$		GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate cut -- this is the command name.
$ Do not translate %s.
$ The program replaces %s with a string of 
$ characters.   

NODELIMITER "cut: Provide a delimiting character with the -d flag.\n"
$ 
USAGE "Usage: cut -b List [-n] [File...]\n\
   or: cut -c List [File...]\n\
   or: cut -f List [-d Character] [-s] [File...]\n" 
$ 
CANTOPEN "cut: Cannot open %s.\n"
$ 
BCKSPC "cut: File: %s Cannot process multiple adjacent backspaces.\n" 
$ 
BADLIST "cut: The list arguments following the %c option are not correct.\n" 
$ 
MAXARGS "cut: Too many list arguments for the %c option.\n"
$
NOMEMORY "cut: Out of memory.\n"
$
