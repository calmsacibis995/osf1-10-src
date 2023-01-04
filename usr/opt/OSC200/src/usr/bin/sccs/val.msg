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

$
$ @(#)$RCSfile: val.msg,v $ $Revision: 4.2.3.2 $ (OSF) $Date: 1993/08/25 22:04:51 $
$
$ Copyright International Business Machines Corp. 1989
$ Unpublished Work
$ All Rights Reserved
$ Licensed Material - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ val.msg        1.6  com/cmd/sccs/hdr/val.msg, bos, bos320 4/25/91 19:27:15

$quote "         define quote character.

$len 
$ Do not translate val:, SCCS, SID
$ Do not translate %s, %d, %1$s, etc..
$set 12        Define initial set#

MMSMTCH              "val: The value specified by -m does not match the %2$cM%3$c\n\
\tidentification keyword value specified in the SCCS file %1$s.\n"
$ Do not translate -m 
$ %2$cM%3$c represents the identification keyword for module name. 
YMSMTCH              "val: The parameter specified by -y does not match the\n\
\ttext specified by %2$cY%3$c in the SCCS file %1$s.\n"
$ Do not translate -y 
$ %2$cY%3$c represents the identification keyword for module type. 
SIDNOEXST            "val: The specified SID does not exist for the file %s.\n"
$ %s is the file name. 
SIDINVLD             "val: The specified SID is not valid for the file %s.\n"
$ %s is the file name. 
OPNSCCS              "val: Cannot open the file %s or the file is not an SCCS file.\n"
$ %s is the file name. 
CRPTSCCS             "val: %s is a damaged SCCS file.\n"

DUPKEYLTR           "ERROR: Flag -%c is not valid. (cm1)\n"

MSGFILARG            "ERROR: Specify the file to process. (cm3)\n"

VALUSAGE "\tUsage:  val [-m Identifier] [-r SID] [-s] [-y Type...] file ...  | -\n\n"

TOOMNY             "val: Too many arguments in one command line.\n"
INVCHR             "val: Invalid character was found in command line.\n"
