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
$quote "         define quote character.

$    
$ COMPONENT_NAME: UUCP uusend.msg   
$    
$ FUNCTIONS: paragraph    
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989   
$ All Rights Reserved   
$ Licensed Materials - Property of IBM   
$ US Government Users Restricted Rights - Use, duplication or   
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.   
$ @(#)uusend.msg\t1.4  com/cmd/uucp,3.1,9008d 2/2/90 17:07:21   
$ Copyright International Business Machines Corp. 1988   
$ Unpublished Work   
$ All Rights Reserved   
$ Licensed Material - Property of IBM   
$ messages for uusend 
$ The range of message numbers assigned to this file is 249-258.
$ Do not translate uusend:
$ len 
$set MS_UUSEND        Define initial set#

NO_APPEND            "uusend: Cannot append to %s.\n"
BADFLAG              "uusend: The flag %s is not known.\n"
USAGE                "Usage: uusend [-m ooo] [-r] -/file system!system!...!remotefile\n"
BADINPUT             "uusend: The input is not valid.\n"
REMOTE               "remote: nextsys='%s', destname='%s', cmd='%s'\n"
$ Do not trnaslate this message.  
BEFORE               "before ~: '%s'\n"
$ Do not translate this message.  
BADUSER              "uusend: The user name is not valid.\n"
NOUSER               "uusend: Ther user %s is not valid.\n"
BADFILE              "uusend: The file %s is not valid.\n"
RPT                  "local, file='%s'"
$ Do not translate this message.  
COUNT                "count %ld bytes\n"
$ Do not translate this message.  
