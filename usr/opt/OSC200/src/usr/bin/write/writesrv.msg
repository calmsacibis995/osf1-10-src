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
$ @(#)$RCSfile: writesrv.msg,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/10 21:24:00 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2

$
$ COMPONENT_NAME: (CMDCOMM) user to user communication
$
$ FUNCTIONS: 
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ writesrv.msg	1.7  com/cmd/write/writesrv.msg, bos, bos320 4/23/91 13:57:18
$

$quote "         define quote character.
$set MS_writesrv        Define initial set#

M_MSG_0 "%s cannot create another process.\n"
$ %s is replaced by the system that cannot create the process.
E_MSG_1 "Accept"
$ will be followed by a perror message.
M_MSG_2 "writesrv: Reading the stream message.\n"
M_MSG_3 "writesrv: The service specified is not known.\n"
$ Do not translate "writesrv".
M_MSG_4 "Opening the stream socket"
$ will be followed by a perror message.
E_MSG_5 "Binding the stream socket"
$ will be followed by a perror message.
M_MSG_6 "Getting the socket name"
$ will be followed by a perror message.
M_MSG_12 "writesrv: Cannot open %s.\n"
$ %s is replaced by a file name.
M_MSG_13 "writesrv: Cannot continue.\n"
$ Do not translate "writesrv".
M_MSG_14 "writesrv: The specified tty line does not exist.\n"
$ tty is a unix term for a terminal
M_MSG_15 "writesrv: Write permission has been denied on the target tty.\n"
M_MSG_16 "writesrv: Cannot open the target tty line for writing.\n"
M_MSG_17 "Node     Service  Handle      Time\n"
$ These words are column heads for a table. Do not change spacing.
M_MSG_18 "\tTo reply, enter:  write -h %d,%s|%s|%s\n"
$ %d is replaced by a number string.
$ the three %s are replaced by the three possible replies.
M_MSG_19 "writesrv: Cannot communicate with the write command.\n"
E_MSG_20 "writesrv: accept"
$ will be followed by a perror message.
M_MSG_44 "writesrv: Error in multibyte character conversion.\n"
