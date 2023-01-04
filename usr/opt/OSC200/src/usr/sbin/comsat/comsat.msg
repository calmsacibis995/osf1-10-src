 
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
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0

$
$ @(#)$RCSfile: comsat.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:02:18 $
$
$ COMPONENT_NAME: CMDMAILX comsat.msg
$
$ FUNCTIONS:
$
$ ORIGINS: 10  26  27
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "         define quote character.

$len 
$set MS_comsat        Define initial set#

M_USAGE              "usage: comsat [-s] [-d directory]"
M_ECHDIR             "chdir: %s: %m"
M_ETMP               "open: /etc/utmp: %m"
M_EMALLOC            "malloc failed"
M_NEWMAIL            "%s\n\
\007New mail for %s@%.*s\007 has arrived:%s\n\
----%s\n"
M_MORE               "...more...%s\n"
M_GETSOCK            "getsockname"
M_COMSAT             "comsat"
SETDEBUG             "setsockopt (SO_DEBUG): %m"
