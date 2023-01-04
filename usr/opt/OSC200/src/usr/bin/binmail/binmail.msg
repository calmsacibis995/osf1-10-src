 
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
$ @(#)$RCSfile: binmail.msg,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/12/03 15:23:26 $
$
$ @(#)binmail.msg	1.6  com/cmd/mailx,3.1,9013 3/14/90 13:12:40
$
$ COMPONENT_NAME: CMDMAILX binmail.msg
$
$ FUNCTIONS:
$
$ ORIGINS: 3  10  27
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "         define quote character.

$set MS_binmail        Define initial set#

M_MSG_0              "%s: can't open %s for writing\n"
M_MSG_1              "usage:  %s [-epqbhr] [-f file]\n    or  %s [-d] [-r name] [-h N] user ...\n"
M_MSG_5              "%s: cannot open %s: "
M_MSG_6              "No mail.\n"
M_MSG_7              "Your mail is being forwarded to "
M_MSG_9              "and your mailbox contains extra stuff\n"
M_MSG_10             "%s: can't open %s\n"
M_MSG_12             "usage\n"
M_MSG_14             "invalid command\n"
M_MSG_15             "%s: cannot append to %s\n"
M_MSG_16             "%s: cannot save mail\n"
M_MSG_17             "invalid command\n"
M_MSG_19             "%s: can't re-read %s\n"
M_MSG_20             "%s: can't re-write %s\n"
M_MSG_21             "%s: can't re-read %s\n"
M_MSG_22             "copyback:Too many letters\n"
M_MSG_23             "%s: can't rewrite %s\n"
M_MSG_24             "%s: cannot copy mail back\n"
M_MSG_25             "new mail arrived\n"
M_MSG_26             "copymt:Too many letters\n"
M_MSG_27             "%s: flattened name %s ambiguous\n"
M_MSG_32             "%s: cannot reopen %s for reading\n"
M_MSG_33             "%s: can't malloc\n"
M_MSG_34             "%s: cannot create %s\n"
M_MSG_35             "Mail saved in %s\n"
M_MSG_36             "Mail saved in %s\n"
M_MSG_37             "%s: cannot save in dead letter\n"
M_MSG_38             "null name\n"
M_MSG_42             "%s: cannot pipe to mail command\n"
M_MSG_43             "unbounded forwarding\n"
M_MSG_44             "%s: can't send to %s\n"
M_MSG_45             "%s: cannot append to %s\n"
M_MSG_46             "%s: cannot append to %s\n"
M_MSG_47             "%s: error signal %d\n"
M_MSG_49             "%s: %s not creatable after %d tries\n"
M_MSG_51             "To: "
M_MSG_52             " remote from %s\n"
M_MSG_53             " forwarded by %s\n"
M_MSG_55             "%s: no space for temp file\n"
M_MSG_56             "q\t\tquit\n"
M_MSG_57             "x\t\texit without changing mail\n"
M_MSG_58             "p\t\tprint\n"
M_MSG_59             "s [file]\tsave (default mbox)\n"
M_MSG_60             "w [file]\tsame without header\n"
M_MSG_61             "-\t\tprint previous\n"
M_MSG_62             "d\t\tdelete\n"
M_MSG_63             "+\t\tnext (no delete)\n"
M_MSG_64             "m [user]\tmail to user\n"
M_MSG_65             "! cmd\t\texecute cmd\n"
M_MSG_66             "localhost name is not defined\n"
M_MSG_67             "biff service is not defined in /etc/services\n"
M_MSG_68             "error getting biff socket: %m\n"
M_MSG_69             "error sending message to biff: %m\n"
M_MSG_70             "%s: bad letter type %d to copylet.\n" 
M_MSG_71             "remote system name too long\n"
M_MSG_72             "%s: can't create proc for remote\n"
M_MSG_73             "%s: creating %s"
M_MSG_74             "%s: opening %s"
M_MSG_75             "%s: stating %s"
M_MSG_76	     "%s: %s has more than one link or is a symbolic link\n"
M_MSG_77	     "%s: %s is not owned by you\n"
M_MSG_78	     "%s: mail file %s exceeds message number maximum of %d\n"
BL_CHOWN	"%s: error changing ownership of file"
BL_ELOCK	"%s: cannot create lock file %s\n"
ILL_LKSTYLE		"%s: illegal %s value in %s\n"
LF_ELOCK	"%s: cannot lockf %s\n"
