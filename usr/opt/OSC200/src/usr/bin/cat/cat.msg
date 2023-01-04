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
$ @(#)$RCSfile: cat.msg,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 15:14:35 $
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
$ cat.msg 1.6  com/cmd/scan,3.1,9021 3/4/90 17:33:02
$
$quote "         define quote character.
$len
$set MS_CAT        Define initial set#

$ messages for command cat
$   GLOBAL COMMENT FOR TRANSLATORS
$       Do not translate cat -- this is the command name.
$       Do not translate %o %s %1$s %2$s etc.
$       These are output formats for file names, directory
$       names, permission codes, command names, and the like,
$       depending on their context within each message.
ESTATOUT             "cat: Cannot get status on standard output.\n"
EOPEN                "cat: Cannot open %s.\n"
ESTAT                "cat: Cannot get status on %s.\n"
$
$ %s is a file name.
$
EINISOUT             "cat: Cannot use %s as both input and output.\n"
$
$ %s is a file name.
$
ECLOSE               "cat: Cannot close the file.\n"
$
EOUTPUT              "cat: Cannot write to output.\n"
$
EUSAGE               "Usage: cat [-benrstuv] [-|File ...]\n"
$
$       Do not translate any of -rsu, -n, b, -v, et.
EREAD		     "cat: read error"
