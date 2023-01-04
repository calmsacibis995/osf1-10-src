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
$ @(#)$RCSfile: cmp.msg,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/10 15:42:53 $
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
$ cmp.msg	1.9  com/cmd/files,3.1,9021 3/16/90 14:34:01
$
$quote "

$set MS_CMP
$ Do not translate cmp

DIFF "%s %s differ: byte %d, line %d\n"
$ DO NOT TRANSLATE
$ This is part of the output of the diff command. The standards dictate
$ that it be in precisely this format. Customers will expect it this way.
USG "Usage: cmp [-l|-s] File1 File2\n"
NOTOPEN "cmp: Cannot find or open %s.\n"
EOFMSG "cmp: EOF on %s\n"
$ DO NOT TRANSLATE
$ This is part of the output of the diff command. The standards dictate
$ that it be in precisely this format. Customers will expect it this way.
