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
/* $XConsortium: xteststr.h,v 1.5 92/04/20 13:14:15 rws Exp $ */
/*

Copyright 1992 by the Massachusetts Institute of Technology

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/

/* The following are needed for correctly-sized protocol requests */

#include <X11/Xmd.h>
#define Window CARD32
#define Time CARD32
#define Cursor CARD32


#define XTestCurrentCursor ((Cursor)1)

typedef struct {
    CARD8	reqType;	/* always XTestReqCode */
    CARD8	xtReqType;	/* always X_XTestGetVersion */
    CARD16	length B16;
    CARD8	majorVersion;
    CARD8	pad;
    CARD16	minorVersion B16;
} xXTestGetVersionReq;
#define sz_xXTestGetVersionReq 8

typedef struct {
    BYTE	type;			/* X_Reply */
    CARD8	majorVersion;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD16	minorVersion B16;
    CARD16	pad0 B16;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
} xXTestGetVersionReply;
#define sz_xXTestGetVersionReply 32

typedef struct {
    CARD8	reqType;	/* always XTestReqCode */
    CARD8	xtReqType;	/* always X_XTestCompareCursor */
    CARD16	length B16;
    Window	window B32;
    Cursor	cursor B32;
} xXTestCompareCursorReq;
#define sz_xXTestCompareCursorReq 12

typedef struct {
    BYTE	type;			/* X_Reply */
    BOOL	same;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
} xXTestCompareCursorReply;
#define sz_xXTestCompareCursorReply 32

/* used only on the client side */
typedef struct {
    CARD8	reqType;	/* always XTestReqCode */
    CARD8	xtReqType;	/* always X_XTestFakeInput */
    CARD16	length B16;
    BYTE	type;
    BYTE	detail;
    CARD16	pad0 B16;
    Time	time B32;
    Window	root B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    INT16	rootX B16, rootY B16;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
} xXTestFakeInputReq;
#define sz_xXTestFakeInputReq 36

#undef Window CARD32
#undef Time CARD32
#undef Cursor CARD32
