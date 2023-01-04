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
$ @(#)$RCSfile: join.msg,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/10 18:27:50 $
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
$ join.msg	1.9  com/cmd/files/join.msg, bos320 5/31/91 08:17:31
$
$set MS_JOIN
$quote "

BADFLDNUM "join: The %s parameter is not a valid field number\n"
$ do not translate "join:" or "%s"

CANTOPEN "join: Cannot open file %s\n"
$ do not translate "join:" or "%s"

USAGE "Usage: join [-aFilenum] [-vFilenum] [-e String] [-[j]Filenum Fieldnum]\n\
\t[-tCharacter] [-o [Filenum.Fieldnum[,Filenum.Fieldnum]...]] File1 File2\n"

BADSEP    "join: Invalid -t separator character\n"
$ do not translate "join:" or "-t"

BOTHSTDIN "join: Both files cannot be \"-\"\n"

FLDOVFLO  "join: Too many fields in line %s\n"

