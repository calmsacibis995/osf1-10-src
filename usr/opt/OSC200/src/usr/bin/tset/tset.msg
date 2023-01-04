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
$ @(#)$RCSfile: tset.msg,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/10 19:55:30 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2


$ COMPONENT_NAME: CMDTTY terminal control commands
$
$ FUNCTIONS: .msg file for tset-posix
$
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ @(#)tset-posix.msg	1.1  com/cmd/tty/tset,3.1,9021 2/20/90 18:09:05

$set MS_TSET

$quote "

USE1 "usage: tset [-] [-nrsIQS] [-eC] [-kC] [-iC] [-m [ident][test speed]:type] [type]\n"
USE2 "usage: tset [-] [-rsIQS] [-eC] [-kC] [-iC] [-m [ident][test speed]:type] [type]\n"
NOTTERM "Not a terminal\n"
BADFLAG "Bad flag -"
FIND "Cannot find termcap\n"
TYPE "Type "
UNKNOWN " unknown\n"
TERMTYPE "Terminal type is "
CANTUPDATE "Cannot update htmp\n"
IS " is "
SETTO " set to "
CANTOPEN "Cannot open "
TOOFEW "Too few args: "
UNEXPECT "Unexpected arg: "
