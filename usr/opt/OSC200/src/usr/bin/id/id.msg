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
$ @(#)$RCSfile: id.msg,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 18:13:47 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2
$
$ COMPONENT_NAME: (CMDSAUTH) security: authentication functions
$
$ FUNCTIONS:
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "

$set MS_id

M_SUID  "uid=%u"
$
$ preserve the spaces in front of these strings.  Do not translate %u.
$
M_SGID  " gid=%u"
M_SEUID " euid=%u"
M_SEGID " egid=%u"
M_GROUPS " groups="

M_NOUSER	"Could not get \"user\" information\n"
$ Translate "user" 
M_NOGROUP	"Could not get \"group\" information\n"
$ Translate "group" 
M_BADUSER	"User not found in /etc/passwd file\n"
$ Do not translate "/etc/passwd" 
M_USE1		"usage: id [user]\n"
M_USE2		"       id  -G [-n] [user]\n"
M_USE3		"       id  -g [-nr] [user]\n"
M_USE4		"       id  -u [-nr] [user]\n"
$ All four lines above are usage messages and are printed together
$ Do not translate any occurance of "id" 
$ Translate "user" 

