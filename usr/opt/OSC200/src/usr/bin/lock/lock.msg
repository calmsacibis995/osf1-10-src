 
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

$ @(#)$RCSfile: lock.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:39:43 $

$ COMPONENT_NAME: (CMDOPER) commands needed for basic system needs  
$
$ FUNCTIONS:   
$
$ ORIGINS: 27  
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$ combined with the aggregated modules for this product)  
$ OBJECT CODE ONLY SOURCE MATERIALS  
$
$ (C) COPYRIGHT International Business Machines Corp. 1989  
$ All Rights Reserved  
$
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$ messages for lock  
$
$ lock.msg	1.4  com/cmd/oper,3.1,9021 2/28/90 16:17:01
$

$quote "         define quote character.
$len 
$set MS_LOCK        Define initial set#
$ Do not translate lock:, %d, %s, %1$d, %1$s, etc ...

LNOTTERM             "lock: This command runs only on a display device.\n"
$ lock was not run on a terminal.  Error recovery is ask user to re-run  
$ command on a terminal.  

LKEY                 "Type the password or key:\n"
$ Do not translate key. 
$ the "key" message asks the user for the password to lock the   
$ terminal with.  

LAGAIN               "Type the password or key again.\n"
$ Do not translate key. 
$ the "Again" message re-asks the user for the key that he/she just typed  
$ in  to make sure that they typed the right thing.  

LWHENTIMEOUT         "lock: Session for %1$s on %2$s will timeout in %3$d minutes.\n"
$ the parameters are:   tty  host           # of minutes till timeout  

LTIMENOWIS           "The current time is %s.\n"

LUSAGE               "Usage: lock [-Timeout]\n"
$ standard useage message: 'timeout' is the number of minutes to wait   
$ before timing out.  

LLOCKTIMOUT          "\nlock: A timeout occurred.\n"
$ this message is printed when lock's input times out.  

LTYPINULK            "\nlock: Type in the unlock password or key. Timeout will occur in %d minutes.\n"
$ %d is a number of minutes till timeout.  

LTIMENOW20           "The current time is %.20s.\n"
$ %s is the first 20 chars of a time string (the results of asctime)  

LDATEFMT             "%a %sD %T %z %Y"
$ Do not translate %a %sD %T %z %Y 
$ Do not translate NLstrtime. 
$ date format for NLstrtime  
