 
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
$ @(#)$RCSfile: wall.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:35:08 $
$ 
$ COMPONENT_NAME: (CMDCOMM) user to user communication
$ 
$ FUNCTIONS:
$ 
$ ORIGINS: 27
$ 
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$ OBJECT CODE ONLY SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$ 
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 
$ wall.msg 1.10  com/cmd/comm,3.1,9021 3/17/90 17:42:50
$ 
$set MS_WALL
$quote "

BRODCAST "\r\n\007\007Broadcast Message from %s@%s (%.*s) at %d:%02d ...\r\n\n"
$ %s is the user who is sending the message.
$ %s is the workstation of the user sending the message.
$ %.*s is the tty line of the sending user.
$ %d:$02d is the HH:MM time the message was sent
TOOLONG "wall: The message cannot be more than %d characters long."
$ %d is the message character limit.
TRYAGAIN "wall: Cannot create another process at this time.\n"
AUTH	"%s: need sysadmin authorization\n"
PRIV	"%s: insufficient privileges\n"
