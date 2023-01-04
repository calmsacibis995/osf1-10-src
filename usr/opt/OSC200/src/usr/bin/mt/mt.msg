 
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

$ @(#)$RCSfile: mt.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:42:35 $

$quote "
$set MS_MT
$ do not translate mt

USAGE "Usage: mt [ -f device ] command [ count ]\n"

INVALID "mt: %s is not a recognized command.\n"
$ %s is replaced by a string

NEGRCNT "mt: negative repeat count\n"

FAILED "failed"
$ This is preceded by the tape & command names, and followed by a perror msg.

UNKNOWN "unknown tape drive type (%d)\n"

STATUS "%s tape drive, residual=%d\n"

AUTH "mt: need tape authorization.\n"

NOPRIV "mt: insufficient privileges.\n"

OFFLINE "\7\nError on device named %s - Place %s tape drive unit #%u ONLINE\n" 

WRTLCKD "\7\nError on device named %s - WRITE ENABLE %s tape drive unit #%u\n"

