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
$ @(#)$RCSfile: kill.msg,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/09/03 03:31:20 $
$
$
$ HISTORY
$ 
$ OSF/1 1.2

$
$ COMPONENT_NAME: (CMDCNTL) system control commands
$
$ FUNCTIONS:
$
$
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1990
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ kill.msg	1.9  com/cmd/cntl,3.1,9013 3/11/90 12:15:12
$ kill.msg	4.1 23:50:11 7/12/90 SecureWare
$
$ messages for command kill
$

$set MS_KILL
$ Do not translate kill

$quote "

EPGROUP "Cannot kill the specified process group.\n"
EPDENIED "Permission denied.\n"
ESIGNAL "Specify a signal number less than 32.\n"
ENOPROC "The specified process does not exist.\n"
USAGE  "Usage: kill [ -s SignalName ] ProcessID ...\n"
USAGE1 "Usage: kill [ -SignalNumber|-SignalName ] ProcessID ...\n"
$ The default signal number is -15 (software terminate).
USAGE2 "Usage: kill -l [ExitStatus]\n"
EKILL "kill: %d: %s\n"
$ Do not translate
BADNO   "The specified signal number or name is not valid.\n"
PRIV    "%s: insufficient privileges\n"
BADEXIT	"Invalid exit status value.\n"
