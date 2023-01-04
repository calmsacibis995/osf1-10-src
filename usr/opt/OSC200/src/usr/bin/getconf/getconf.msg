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
$ HISTORY
$ 
$ (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
$ ALL RIGHTS RESERVED 
$
$
$ OSF/1 1.2
$
$ @(#)$RCSfile: getconf.msg,v $ $Revision: 1.1.3.2 $ (OSF) $Date: 1993/06/11 18:11:36 $
$
$ COMPONENT_NAME: (CMDPOSIX) new commands required by Posix 1003.2
$
$ FUNCTIONS: 
$
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ 1.1  com/cmd/posix/getconf.msg, bos320 12/12/90 15:04:29
$
$quote "
$set MS_GETCONF

USAGE1	"Usage: getconf system_var\n"
USAGE2	"       getconf path_var pathname\n"
NOTVAL  "getconf: specified variable is not valid on this system\n"
NOTDEF  "undefined\n"
NOPATH  "getconf: must specify pathname with path_var\n"
BADPATH "getconf: must not specify pathname with system_var\n"
