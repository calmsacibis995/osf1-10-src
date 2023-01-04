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
$ @(#)$RCSfile: time.msg,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 19:14:30 $
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
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ 1.7 com/cmd/stat/time.msg, 9121320k, bos320 4/25/91 17:46:33

$quote "

$set MS_TIME

NOFORK "time: Cannot create another process.\n"
ABTERM "time: The process was stopped abnormally. Try again.\n"

$
$ For the following three values, make them the same lenght so
$ the output will align properly
$
REAL "Real  "
$ For this and the following two messages, make them the same length so
$ the output will align properly
$ For example, if System translates to 10 letters, make REAL and USER be
$ 10 letters (include blanks at the end.)

USER "User  "
SYS  "System"
USAGE "Usage: time [-p] [CommandString]\n"
