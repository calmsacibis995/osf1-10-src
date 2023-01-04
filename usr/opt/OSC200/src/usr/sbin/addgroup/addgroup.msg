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
$ @(#)$RCSfile: addgroup.msg,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/04 08:21:57 $
$
$quote "
$set MS_ADDGROUP
1	"Please su to root first.\n"
2	"\nNIS in use. Refer to the network documentation for adding new groups.\n"
3	"Enter a new group name or <Return> to exit: "
4	"Error, illegal characters in group name.\n"
5	"New entry not created.\n"
6	"Error, the group name is too long (must be 8 characters or less).\n"
7	"Error, group %1$s is already in the /etc/group file.\n"
8	"\nError, the /etc/group file is corrupt.\n"
9	"Exiting %1$s.  New entry not created.\n"
10	"\nEnter a new group number [%1$s]: "
11	"\nError, the group number must be all digits.\n"
12	"\nError, the group number %1$s is already in the /etc/group file.\n"
13	"\nError, the /etc/group file is busy.\n"
14	"Could not read the new group name.\n"
15	"\nGroup %1$s was added to the /etc/group file.\n"
