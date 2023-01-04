 
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

$ @(#)$RCSfile: fingerd.msg,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/09/29 16:50:56 $
$ 
$ COMPONENT_NAME: TCPIP fingerd.msg
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
$ fingerd.msg       1.3  com/sockcmd/finger,3.1,9021 11/8/89 10:37:54
$ 
$ messages for fingerd
$ 

$set    MS_FINGERD

$quote  "

$ The name of the peer to which a connection will be made cannot be
$ ascertained. Getpeername returns an error message which replaces %m.
PEER         "getpeername"
PIPE		"pipe"
FORK		"fork"
FOPEN		"fopen"
ERRMSG		"fingerd: %s: %s\r\n"
SETDEBUG        "setsockopt (SO_DEBUG): %m"
NO_ROOT		"NOT super-user\n"
