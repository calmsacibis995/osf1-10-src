 
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
$ @(#)$RCSfile: ipcs.msg,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/05/12 13:02:59 $ 8/15/90 SecureWare
$ @(#)ipcs.msg	1.1  com/cmd/ipc,3.1,9021 4/18/89 16:07:09
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$ OBJECT CODE ONLY SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989 
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 

$quote "
$set MS_IPCS


USAGE    "usage:  ipcs [-abcmopqst]\n"

CHDR "T     ID     KEY      MODE       OWNER    GROUP"
CHDR2 "  CREATOR   CGROUP"
QOMSG " CBYTES  QNUM"
QBMSG " QBYTES"
QPMSG " LSPID LRPID"
QTMSG "  STIME    RTIME    CTIME "
QMSG "Message Queues:\n"
MQNIS "Error accessing Message Queue facility.\n"
MOMSG " NATTCH"
MBMSG "  SEGSZ"
MPMSG "  CPID  LPID"
MTMSG "   ATIME    DTIME    CTIME "
MMSG "Shared Memory:\n"
NOSHMEM "Error accessing Shared Memory facility.\n"
SBMSG " NSEMS"
STMSG "   OTIME    CTIME "
SMSG "Semaphores:\n"
SFNIS "Error accessing Semaphore facility.\n"
NOENTRY " no-entry"

$set MS_IPCS_SEC

PRIV "ipcs: insufficient privileges\n"
MACINIT "ipcs: MAC initialization failed\n"
