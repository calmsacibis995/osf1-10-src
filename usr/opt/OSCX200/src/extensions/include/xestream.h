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
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
/* BuildSystemHeader added automatically */
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/include/xestream.h,v 1.1.2.2 92/02/06 16:53:05 Jim_Ludwig Exp $ */
#ifndef __XESTREAM__
#define __XESTREAM__ "@(#)xestream.h	1.11 - 90/11/07  "

/*****************************************************************************
Copyright 1987, 1988, 1989, 1990, 1991 by Digital Equipment Corp., Maynard, MA

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*****************************************************************************/
/*
 *
 *  CONTRIBUTORS:
 *
 *      Dick Annicchiarico
 *      Robert Chesler
 *      Dan Coutu
 *      Gene Durso
 *      Marc Evans
 *      Alan Jamison
 *      Mark Henry
 *      Ken Miller
 *
 *  DESCRIPTION:
 *      This header file describes the data structures and constants
 *      required to utilize the custom "stream" facility which presents
 *      a common interface to varying I/O transports.  Currently only
 *      partially implemented for (server-side).
 */
#include <stdio.h>
#if defined TCPCONN && !defined VMS
#include <netinet/in.h>
#endif
#if defined DNETCONN && !defined VMS
#include <netdnet/dn.h>
#endif

typedef struct
{
    INT32 flink;
    INT32 blink;
}
#ifdef VMS
    _align(QUADWORD)
#endif
    QUEUE_HDR;

/* Stream I/O data abstraction:
 * The following structures support a loose rendition of
 * stream I/O abstraction for use in writing more portable
 * applications between VMS and U*IX.  It was intentionally
 * decided NOT to attempt to implement streams per the 
 * published interface specification (beyond *our* scope).
 * However, the *principle* is being observed in the hope
 * that someday XTrap can take advantage of some future
 * streams implementation.
 */
typedef struct _XESTREAM /* stream I/O object */
{
    struct _XESTREAM *next;
    CARD8            type;
    Bool             queue;
    int_function     accept;
    int_function     close;
    int_function     read;
    int_function     write;
    void_function    err_func;
    CARD16           addr1_lgth;            /* in bytes (counted string) */
    char             *addr1;                /* string of what's opened */
    CARD16           addr2_lgth;            /* in bytes (counted string) */
    char             *addr2;                /* Maybe a return addr (DECnet) */
    union
    {
        struct _qio
        {
            INT16     read_chan;
            INT16     write_chan;
            CARD16    iosb[4L];
            CARD32    efn;
            CARD32    read_mode;
            CARD32    write_mode;
            QUEUE_HDR *read_queue;  /* must be quadword-aligned loc */
            QUEUE_HDR *free_queue;  /* must be quadword-aligned loc */
            CARD32    byte_cnt;
            CARD32    bufsiz;
            CARD32    bufinc;
            CARD32    errno;
#if defined TCPCONN || defined DNETCONN
            CARD32    accept_efn;
            INT16     port;
            INT16     mbx_chan;
            void      *ncb;     /* VMS-only Network Command Block Info */
            void      *netcmd;  /* VMS-only Network Command Mbx Logical */
#endif
        } Q;
        struct _dsc
        {
            INT32  fd;
            INT32  sd;  /* socket descriptor for init connection */
#if defined TCPCONN && !defined VMS
            struct sockaddr_in insock;
#endif
#if defined DNETCONN && !defined VMS
            struct sockaddr_dn dnsock;
#endif
        } D;
    } u;
} XESTREAM;

/* Currently defined Stream Types */
#define StreamInternet  1L
#define StreamDECnet    2L
#define StreamMbx       3L        /* Only on VMS */
#define StreamShMem     4L        /* Not Currently Implemented */
#define StreamFile      5L
#define StreamLAT       6L        /* Not Currently Implemented */


#define BUFSIZstream  1500L       /* Stream BUFSIZ used w/queued stream type */
                                  /* Note:  Can't be <110 bytes for DECnet */
#define BUFstream     1L          /* Initial increment of n buffers per */
                                  /* stream. Increases exponentially as more */
                                  /* are req'd */
#ifdef vms
typedef struct _QREC
{
    CARD32   queue_entry[2L];
    CARD16   iosb[4L];
    XESTREAM *st;
    CARD8    *data;
} 
#ifdef VMS
    _align(QUADWORD)
#endif
    QREC;
typedef struct _QREC *QRECPtr;

#define MbxSize  8192L
#define MbxBufs  2L

#endif

#if defined TCPCONN || defined DNETCONN
#define AcceptInterval  2       /* in seconds */
#define AcceptLoops     15      /* max times around the loop */
#endif

#define XEStreamEFN  7L     /* Arbitrary event flag in cluster 0 for VMS use */

#define SIOErr(st,err_str)      ((void)(*(st)->err_func)(err_str))
#define SIOSetErrFunc(st,rtn)   ((st)->err_func = (void_function)rtn)
#define SIOGetErrFunc(st)       ((st)->err_func)

#endif /* __XESTREAM__ */
