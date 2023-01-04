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
$ @(#)$RCSfile: tail.msg,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/09/03 03:42:40 $
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
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ 1.9  com/cmd/scan/tail.msg, bos320 1/29/91 10:20:01
$
$quote "         define quote character.
$len
$set MS_TAIL        Define initial set#

USAGE                "Usage: tail [-f | -r] [-c number | -n number] [file]\n   or: tail [+number | -number][unit] [-f | -r] [file]\n"
TOOMUCH              "tail: There is not enough memory available now.\n"
FSHRUNK              "tail: The size of the file has changed during the run of tail.\n"
CANNOTR		     "tail: reverse whole file or last n lines only.\n"	 		
