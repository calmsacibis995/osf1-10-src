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
/***********************************************************
Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/include/smtstr.h,v 1.1.2.5 92/08/03 09:32:18 Dave_Hill Exp $
 *  smtstr.h
 *	Shared Memory Transport definitions
 */

#ifndef _SMTSTR_H_
#define _SMTSTR_H_

#include <X11/Xproto.h>
#include <X11/extensions/smt.h>
#include <X11/extensions/ip.h>

/*
 * Minor Version      Description
 * -------------        -----------
 *      0               Wakeup request increments serial number
 *      1               not implemented
 *      2               Wakeup request doesn't increment serial number
 */
#define SMT_MAJOR_VERSION     1
#define SMT_MINOR_VERSION     2


typedef struct _SmtProcInfo {
    int whoami;
    int status;
    int pid;
} SmtProcInfo, *SmtProcInfoPtr;

typedef struct _SmtPrivate {
    SmtProcInfo server;
    SmtProcInfo client;
} SmtPrivate, *SmtPrivatePtr;

typedef struct _SmtQueryVersion {
    CARD8	reqType;
    CARD8	smtReqType;
    CARD16	length B16;
} xSmtQueryVersionReq;
#define sz_xSmtQueryVersionReq 4

typedef struct {
    BYTE	type;		/* X_Reply */
    BYTE	bpad0;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD16	majorVersion B16;
    CARD16	minorVersion B16;
    CARD16	uid B16;
    CARD16	gid B16;
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
} xSmtQueryVersionReply;
#define sz_xSmtQueryVersionReply 32;

typedef struct _SmtAttach {
    CARD8	reqType;
    CARD8	smtReqType;
    CARD16	length B16;
    CARD16	flags;
    CARD8	majorVersion;
    CARD8	minorVersion;
    Channel	chan;
} xSmtAttachReq;
#define sz_xSmtAttachReq sizeof(xSmtAttachReq)

typedef struct {
    BYTE	type;		/* X_Reply */
    BYTE	bpad0;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	status B32;
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
} xSmtAttachReply;
#define sz_xSmtAttachReply 32;

typedef struct _SmtWakeupServer {
    CARD8	reqType;
    CARD8	smtReqType;
    CARD16	length B16;
    CARD32	flags;
} xSmtWakeupServerReq;
#define sz_xSmtWakeupServerReq 8

typedef struct _SmtWakeup {
    BYTE	type;		/* eventBase + SmtWakeup */
    BYTE	bpad0;
    CARD16	sequenceNumber B16;
    CARD32	flags; 
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
} xSmtWakeupEvent;
#define sz_xSmtWakeupEvent 32

#ifdef _SMT_SERVER_

typedef struct { 		/* SMT_DEFER_FREE */
    int (*freep)(/* ptr,dat */);
    unsigned char *Q;
    unsigned long *ptr;
    unsigned long dat;
    unsigned int  len;
} smtQEntry, *smtQEntryPtr;

#define SMT_Q_DEPTH	11 		/* SMT_DEFER_FREE */

typedef struct {			/* SMT_DEFER_FREE */
    int last;				/* next to be freed */
    int this;				/* current request */
    int next;				/* next to be written */
    smtQEntry e[SMT_Q_DEPTH];
} smtQueue, *smtQueuePtr;

typedef struct _smtControlBlock {
    Channel chan;			/* channel control block */
    SmtPrivatePtr pPriv;
    int wakeRec;			/* wakeup requests received */
    int wakeSent;			/* wakeup events sent */
    int (*ReadRequestFromClient)();
    smtQueue q;				/* queue of req's to free */
    short majorVersion, minorVersion;	/* client versions */
} smtControlBlock, *smtControlBlockPtr;

#else				/* client-side definitions */

typedef struct _smtDisplay {
    Channel chan;
    char *buffer;		/* saved non-smt buffer starting address. */
    char *bufptr;		/* saved non-smt buffer index pointer. */
    char *bufmax;		/* saved non-smt buffer index pointer. */
    SmtPrivatePtr pPriv;	/* pointer to server-client private area */
    short majorVersion, minorVersion;	/* server versions */
} smtDisplay, *smtDisplayPtr;

#endif /* _SMT_SERVER_ */

#endif /* _SMTSTR_H_ */
