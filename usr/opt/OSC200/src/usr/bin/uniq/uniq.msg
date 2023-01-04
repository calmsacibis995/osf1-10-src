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
$ @(#)$RCSfile: uniq.msg,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/10 20:20:48 $
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
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ 1.7  com/cmd/files/uniq.msg, 9121320k, bos320 5/5/91 19:25:08
$
$quote "
$		GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate uniq -- this is the command name.
$set MS_UNIQ
$
OPERR "uniq: Cannot open file %s.\n"
$ %s is the input file name.
CRERR "uniq: Cannot create file %s.\n"
$ %s is the output file name.
USAGE "Usage: uniq [-cdu] [-f Fields] [-s Chars] [-Fields] [+Chars] [InFile [OutFile]]\n"
$ Fields  is a number of fields.  Chars  is a number of characters.
$ InFile  is an input file name.  OutFile is an output file name.
BADNUM "uniq: Specify a numeric value for option %s.\n"
$ %s is an option that requires a numeric value.
$ This invocation of uniq had a non-numeric string following %s.
BADOPT "uniq: Too many File operands.\n"
$ The uniq command accepts 0, 1, or 2 file operands.
$ This invocation of uniq had 3 or more file operands. 
