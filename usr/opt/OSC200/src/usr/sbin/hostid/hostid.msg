 
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
$ @(#)$RCSfile: hostid.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:12:00 $
$
$ COMPONENT_NAME: TCPIP hostid.msg
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
$
$ messages for hostid
$ The range of message numbers assigned to this file is 821
$ hostid.msg	1.4  com/sockcmd/util,3.1,9021 3/5/90 17:08:43
$
$len 
$quote "         define quote character.
$set MS_HOSTID        Define initial set#

SYS_ATTR              "%s: insufficient privileges\n"
$  1	This message is printed when hostid program is ran
$  2	by user with insufficient privileges.
SYS_ADM              "%s: need sysadmin authorization\n"
$  1	This message is printed when hostid program is ran
$  2	by user other then system administrator.
USAGE                "Usage: %s [HexNumber or InternetAddress or HostName]\n"
$  2    This message is printed when the hostid program is not 
$  2    used correctly. 
SETHOST              "hostid: sethostid.\n"
