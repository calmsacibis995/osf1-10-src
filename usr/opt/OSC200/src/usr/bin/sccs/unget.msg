 
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
$ @(#)$RCSfile: unget.msg,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/07/29 17:37:45 $
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
$        unget.msg 1.4 com/cmd/sccs/hdr,3.1,9021 3/24/90 16:04:35
$
$quote "         define quote character.

$len 
$set 11        Define initial set#
SIDSPEC1      "\nThere is more than one outstanding SID in the p-file.\n"
$ Do not translate p-file, SID.
LOGNAME       "\nUser name is not in the p-file.\n"
$ Do not translate p-file.
SIDSPEC3      "\nThe specified SID does not exist.\n"
$ Do not translate SID. 
NOOUTDELTA    "\nThere are no outstanding deltas for %s.\n"
$
SACTUSAGE     "Usage: sact file... | -\n"
$
UNGETUSAGE    "Usage: unget [-n] [-s] [-r SID] file... | -\n"
$
UNGETBADSID   "Invalid SID:( %c ) The SID must be a numeric value. (cm1) \n"
$
KEYLTRTWC     "Use the -%c flag only once on the command line. (cm2)\n"
$
