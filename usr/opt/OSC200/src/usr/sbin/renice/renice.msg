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
$ HISTORY

$ @(#)$RCSfile: renice.msg,v $ $Revision: 4.2.2.2 $ (OSF) $Date: 1993/08/25 22:01:17 $

$ 
$ COMPONENT_NAME: (CMDCNTL) system control commands
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
$ renice.msg	1.7  com/cmd/cntl,3.1,9021 3/11/90 12:16:06
$ renice.msg	4.2 02:01:03 7/13/90 SecureWare
$ 
$ messages for command renice
$ 
$quote "

$set MS_RENICE
$ Do not translate renice

USAGE   "Usage:  renice [-n increment] [-g | -p | -u] ID...\n\
\trenice priority [-p] pids... [-g pgrp ...] [-u user...]\n"

NEWPRI          "%d: old priority %d, new priority %d \n"
$ Only the words old priority and new priority can be translated.
$ Do not change their order.
UNK_USER        "%s: User %s is not known.\n"
$ Do not move the first %s from the beginning of the message. It is replaced
$ by the command name (renice)
$ The 2nd %s is replaced by a user name.
BADVALUE        "%s: The process or process group %s is not an integer.\n"
$ Do not move the first %s from the beginning of the message. It is replaced
$ by the command name (renice)
GETPRIO         "%s: %d: "
$ Do not translate
$ There should not be a trailing newline.
SETPRIO         "%s: %d: "
$ Do not translate
$ There should not be a trailing newline.
PRIV		"%s: insufficient privileges\n"
