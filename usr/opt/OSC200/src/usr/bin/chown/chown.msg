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
$ @(#)$RCSfile: chown.msg,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/10 15:30:14 $
$
$ 
$ HISTORY
$ 
$ OSF/1 1.2

$
$ COMPONENT_NAME:  (CMDSDAC) security: access control
$
$ FUNCTIONS: 
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ chown.msg	1.9  com/cmd/oper,3.1,9013 2/26/90 13:37:03
$ chown.msg	4.1 23:48:53 7/12/90 SecureWare

$ messages for chown command

$set MS_CHOWN
$len
$quote "

USAGE	    "usage: chown [-fR] username[:groupname] file...\n"
NOBACK    "chown: can not change directory back to %1$s\n"
BADUID    "chown: unknown username %s\n"
BADGID    "chown: unknown groupname %1$s\n"
PRIV      "chown: insufficient privileges\n"
NODIR     "chown: cannot get directory path %1$s"
NAME      "chown: "
