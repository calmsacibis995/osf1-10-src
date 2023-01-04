 
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
$ @(#)$RCSfile: cm.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:50:18 $
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
$       cm.msg 1.4 com/cmd/sccs/hdr,3.1,9021 3/24/90 16:01:48
$
$quote "         define quote character.

$ Do not translate %c, %s, %d, etc...
$ Do not translate SCCS, :
$ Do not translate the parenthesis (cm#) at the end of each message.
$len 
$set 2        Define initial set#

UNKKEYLTR            "Flag -%c is not valid. (cm1)\n"
$ Do not translate -%c
KEYLTRTWC            "\n\
Use the -%c flag only once on the command line. (cm2)\n"
$ Do not translate -%c
MISSFLNAM            "\n\
Specify a file name. (cm3)\n"
LOCKCREAT            "\n\
Cannot create the lock file. (cm4)\n"
BADDTTM              "\n\
Specify the date and time in the correct format.\n\
\tThe correct format is yy[mm[dd[hh[mm[ss]]]]]. (cm5)"
NOIDKEYWRDS          "\n\
The file must contain SCCS identification keywords. (cm6)\n"
NOIDKEYWRDS7         "There are no SCCS identification keywords in the file. (cm7)\n"
VALAFTKEY            "Cannot use a value for the -%c flag. (cm8)\n"
$ Do not translate -%c
$  
USERID               "\n\
The /etc/passwd file is not accessible. (cm9)\n"
$ Do not translate /etc/passwd
INVIDKYWRDS          "\n\
The SCCS file requires one or more specific \n\
\tidentification keywords. (cm10)\n"
CURDIRFAIL           "\n\
Cannot determine the path name of the current directory. (cm21)\n"
SIDNOEXIST           "\n\
The SID does not exist. (cm20)\n"
CANTCHDIR            "Cannot chdir to %s. (cm22)\n"
$ Do not translate chdir.
ROUTOFRNG            "\n\
Specify a value for the -r flag  greater than 0 and\n\
\tless than 10000. (cm23)\n"
MRNOTALD             "\n\
The specified SCCS file does not allow MR numbers. (cm24)\n"
$ Do not translate MR.
INVMRS               "\n\
Specify a valid MR number or numbers. (cm25)\n"
$ Do not translate MR.
MRSREQ               "\n\
Specify an MR number or numbers. (cm26)\n"
$ Do not translate MR.
