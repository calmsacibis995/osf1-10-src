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
/* $XConsortium: client.h,v 1.4 92/11/18 21:00:40 gildea Exp $ */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * M.I.T. not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND M.I.T. DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES,
 * DIGITAL OR M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#ifndef	_CLIENT_H_
#define	_CLIENT_H_

typedef struct _Client *ClientPtr;

extern ClientPtr *clients;
extern ClientPtr serverClient;

#define	NullClient	((ClientPtr) NULL)

#define	SERVER_CLIENT	0
#define	MINCLIENT	1

#define	CLIENT_ALIVE		0
#define	CLIENT_GONE		1
#define	CLIENT_AGED		2
#define	CLIENT_TIMED_OUT	4

extern int  currentMaxClients;

#define	REQUEST(type)	\
	type *stuff = (type *)client->requestBuffer

#define	REQUEST_FIXED_SIZE(fs_req, n)					\
	if (((SIZEOF(fs_req) >> 2) > stuff->length) ||			\
		(((SIZEOF(fs_req) + (n) + 3) >> 2) != stuff->length)) {	\
	    int lengthword = stuff->length;				\
	    SendErrToClient(client, FSBadLength, (pointer)&lengthword); \
	    return (FSBadLength);	\
	}

#define	REQUEST_SIZE_MATCH(fs_req)				\
	if ((SIZEOF(fs_req) >> 2) != stuff->length) {	\
	    int lengthword = stuff->length;				\
	    SendErrToClient(client, FSBadLength, (pointer)&lengthword); \
	    return (FSBadLength);	\
	}

#define	REQUEST_AT_LEAST_SIZE(fs_req)					\
	if ((SIZEOF(fs_req) >> 2) > stuff->length) {			\
	    int lengthword = stuff->length;				\
	    SendErrToClient(client, FSBadLength, (pointer)&lengthword); \
	    return (FSBadLength);	\
	}

#define	WriteReplyToClient(client, size, reply)			\
	if ((client)->swapped)						\
	    (*ReplySwapVector[((fsReq *)(client)->requestBuffer)->reqType]) \
		(client, (int)(size), reply);				\
	else	(void)WriteToClient(client, (int)(size), (char *)(reply));

#define	WriteSwappedDataToClient(client, size, pbuf)		\
	if ((client)->swapped)						\
	    (*(client)->pSwapReplyFunc)(client, (int)(size), pbuf);	\
	else (void) WriteToClient(client, (int)(size), (char *)(pbuf));


extern void SendErrToClient();

extern void	SwapFontHeader();
extern void	SwapExtents();
extern void	SwapPropInfo();
extern void	SwapCharInfo();
extern void	WriteSConnSetup();
extern void	WriteSConnectionInfo();
extern void	SErrorEvent();

typedef struct _WorkQueue       *WorkQueuePtr;
extern void	ProcessWorkQueue();

#endif				/* _CLIENT_H_ */
