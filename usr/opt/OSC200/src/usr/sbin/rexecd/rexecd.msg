 
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
$ @(#)$RCSfile: rexecd.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:25:42 $
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ messages for rexecd
$
$len 
$quote "         define quote character.
$set MS_REXECD        Define initial set#
$ Do not translate %s, %c, %m and rexecd. 

GPEERERR             "%s: "
$               Writes argv[0] (procedure name) to stderr when the   
$               getpeername() call fails (retrieves the name bound to the   
$               socket). Stderr should be the remote. (Passed by inetd   
$               when rexecd exec'ed).   
PEERERR              "getpeername"
$ Do not translate getpeername.  
$               Writes message above via perror for the local system.   
LOGINERR             "rexecd: The login is not correct."
$ \t\t\tSecurity message - see below.   
LOGINERR2            "rexecd: The login is not correct.\n"
$               The message used in 2 places. The first case is when   
$               getpwnam() fails (no passwd entry for this user). The 2nd   
$               case occurs if received password after encryption does not   
$               match that in the password file. Written to stderr.   
NODIR               "rexecd: There is no remote home directory.\n"
RETRY                "rexecd: Cannot create another process at this time.\n"
$               This message is wriiten to stderr when fork() system call   
$               fails. (Too many processes usually).   
TOOLNG               "rexecd: The string %s is too long for the buffer.\n"
$ \t\t\tMessage printed to stderr whenn the string read from the   
$ \t\t\tremote (user, password, command) is longer than the buffer   
$               size for it (16,16,NCARGS). Exits.   
