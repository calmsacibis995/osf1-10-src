 
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
$ @(#)$RCSfile: sccsdiff.msg,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/12/11 14:26:19 $
$
$ IBM CONFIDENTIAL
$ Copyright International Business Machines Corp. 1989
$ Unpublished Work
$ All Rights Reserved
$ Licensed Material - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$        sccsdiff.msg 1.4 com/cmd/sccs/hdr,3.1,9021 3/24/90 16:04:17
$
$quote "         define quote character.

$len 
$set 1        Define initial set#

$ This is the sccsdiff message catalog. 
BADFILE    "%1$s: %2$s is not an SCCS file.\n"
$ %1$s is the command path name 
$ %2$s is the file name. 
USAGE     "usage: %s -rSID1 -rSID2 [-p] [-sNumber] SCCSFile ...\n"
$ %s is the command name 
NODIFF    "%s: No differences.\n"
$ %s is the file name. 
HEADER    "%1$s: %2$s versus %3$s\n"
$ %1$s is the file name. 
$ %2$s and %3$s are SID numbers. 
