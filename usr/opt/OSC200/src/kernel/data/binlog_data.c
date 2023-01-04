/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
#ifndef lint
static char *rcsid = "@(#)$RCSfile: binlog_data.c,v $ $Revision: 1.1.3.5 $ (DEC) $Date: 1992/06/25 22:30:30 $";
#endif


#include <kern/queue.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dec/binlog/binlog.h>


/*
 * kernel binary event log buffer size
 *
 * The buffer size, in bytes, is enforced to be within minimum/maximum limits:
 *
 *            BINLOG_BUFMIN    (8 * 1024)
 *            BINLOG_BUFMAX   (48 * 1024)
*/
int binlog_bufsize = (24 * 1024);



/*
 * Initialized binary event logging status.
 *
 * Options are ('or' multiple options together):
 *
 *   BINLOG_ON       -  Enables binary event logging. [normally enabled]
 *
 *   BINLOG_ASCIION  -  Enables having ascii messages that get sent to the
 *                      syslog facility are also put into the binary event
 *                      log.  [normally disabled]  By default panic and
 *                      system startup messages are always logged.
*/
int binlog_status = BINLOG_ON;

