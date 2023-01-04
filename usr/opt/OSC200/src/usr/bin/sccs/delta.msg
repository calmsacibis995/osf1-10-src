 
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
$ @(#)$RCSfile: delta.msg,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/08/03 22:19:41 $
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
$       delta.msg 1.5 com/cmd/sccs/hdr,3.1,9021 4/3/90 17:02:13
$
$quote "         define quote character.

$len 
$ Do not translate %s, %d, etcc...
$ Do not translate delta:, SID, CMR, MR
$ Do not translate the parenthesis (de#) at the end of each line.
$set 6        Define initial set#

MISSRKYLTR "\ndelta: Specify an SID with the -r flag.\n"
$ do not translate -r
NAMORSID   "\ndelta: The specified SID or your user name is not in the p-file.\n"
$ Do not translate p-file  
INPTCMRIGNRD  "delta: The specified CMR's are ignored."
$ Do not translate this message. The user will never see it. 
BADDIFF "\ndelta:diff program failed when SCCS tried to create the delta.\n"
$ Do not translate diff
INTRNERR "\ndelta: There is an internal software error.\n"
RETRYSEG "%s failed.  Retrying with a segmentation size of %d.\n"
$ %s is the name of the program that delta uses to compare the files. 
$ %d is the segmentation size. 
DELTADND "\ndelta: Delta denied due to CMR difficulties.\n"
$ Do not translate this message. The user will never see it. 
AMBIG   "\ndelta: There is more than one outstanding delta.\n"
$ Do not translate -y or -m
STDINSPCD  "delta: Specify the -y or -m flags when using standard input.\n"
$ %1$d is the line number. 
$ %2$s is the file name. 
ILLDAT  "delta: The SOH character is in the first position of line %1$d of file %2$s.\n"
INSM    "%u inserted\n"
$ %u is the number of lines. 
DELM    "%u deleted\n"
$ %u is the number of lines. 
UNC     "%u unchanged\n"
$ %u is the number of lines. 
AUTONDELTA   "AUTO NULL DELTA\n"
$ Do not translate. The user does not see this message. 
MCOMMENTS     "comments"
$ Note to translators: translate the above word exactly as it is. 
DELTAUSAGE	"Usage: delta [-g List] [-m MRlist] [-nps] [-r SID]\n\t\t[-y[comment]] file... | -"
INTRNERR5            "\n\
delta: There is an internal software error. (de5)\n"
INTRNERR6            "\n\
delta: There is an internal software error. (de6)\n"
INTRNERR7            "\n\
delta: There is an internal software error. (de7)\n"
ARGRQD		" Flag -%c requires an argumnet\n\
\tUsage: delta [-g List] [-m MRlist] [-nps] [-r SID]\n\
\t\t[-y[comment]] file... | -"
