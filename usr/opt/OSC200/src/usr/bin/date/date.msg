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
$ @(#)$RCSfile: date.msg,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 17:46:04 $
$
$ 
$ HISTORY
$ 
$ OSF/1 1.2

$
$ COMPONENT_NAME: (CMDSTAT) status
$
$ FUNCTIONS:
$
$ (C) COPYRIGHT International Business Machines Corp. 1989,1990
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ date.msg	1.6  com/cmd/stat,3.1,9021 3/21/90 23:42:22
$ date.msg	4.1 15:15:03 7/9/90 SecureWare

$
$ messages for command date
$

$quote "
$set MS_DATE

EMALLOC "date: No memory. \n"

BADFORMAT "date: %c is not a recognized field descriptor.\n"

ECONV "date: bad conversion\n"

BADPERM "date: You must be root to change the date.\n"
$ Do not translate root

UNK_SERVICE "date: The time service is not available on your system.\n"

NOPORTS "date: All ports are in use.\n"

WRONG_ACK "date: Wrong acknowledgement received from time server: %s\n"

CANTREACH "date: Cannot reach the time server; time set locally.\n"

USAGEMONTH "mm"
$ This must be 2 letters for the month
USAGEDAY "dd"
$ This must be 2 letters for the day
USAGEHOUR "HH"
$ This must be 2 letters for the hour
USAGEMINUTE "MM"
$ This must be 2 letters for the minutes

USAGEROOT  "\tdate [-nu] [%s[.SS[yy]]]\n"

USAGEROOT2 "\tdate [-nu] [yy%s[.SS]]\n"

USAGE "Usage:  date [-u] [+FieldDescriptors]\n"
$ The system generates usage statements for the BADFORMAT and ECONV messages.
$ Two flags are added; -n -u.

E_DIG "date: Expected a digit.\n"
EPRIV "date: insufficient privileges\n"

USAGEROOT3 "\tdate [-nu] [%s[yy]]\n"
