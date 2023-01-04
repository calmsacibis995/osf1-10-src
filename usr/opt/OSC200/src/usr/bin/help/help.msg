 
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

$ @(#)$RCSfile: help.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:35:59 $

$
$ COMPONENT_NAME: (CMDMAN) commands that allow users to read online
$                          documentation
$
$ FUNCTIONS: none
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
$ 	help.msg\t1.3  com/cmd/man,3.1,9021 3/4/90 17:17:05
$

$
$quote "         define quote character.
$ messages for help
$ note: translation of this message file is limited within one screen.
$len
$set MS_HELP        Define initial set#
$ Leave spaces in message.

HELP                 "\n\
Some basic Commands are:\n\
\n\
    man -k keyword\t- lists commands relevant to a keyword \n\
    man command\t\t- prints out the manual pages for a command \n\
    cat\t\t\t- concatenates files (and just prints them out) \n\
    vi\t\t\t- text editor \n\
    finger\t\t- user information lookup program \n\
    ls\t\t\t- lists contents of directory \n\
    mail\t\t- sends and receives mail \n\
    passwd\t\t- changes login password \n\
    sccshelp\t\t- views information on the Source Code Control System \n\
    tset\t\t- sets terminal modes \n\
    who\t\t\t- who is on the system \n\
    write\t\t- writes to another user \n\
\n\
To find programs about mail, use the command: \t\tman -k mail \n\
and print out the man command documentation via:\tman mail \n\
You can log out of the system by typing: \t\texit\n\
\n"
