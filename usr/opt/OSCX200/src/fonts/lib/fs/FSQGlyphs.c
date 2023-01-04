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
/* $XConsortium: FSQGlyphs.c,v 1.5 92/11/18 21:31:15 gildea Exp $ */
/*
 * Copyright 1990 Network Computing Devices;
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

#include "FSlibint.h"

int
FSQueryXBitmaps8(svr, fid, format, range_type, str, str_len, offsets, glyphdata)
    FSServer   *svr;
    Font        fid;
    FSBitmapFormat format;
    Bool        range_type;
    unsigned char *str;
    unsigned long str_len;
    FSOffset  **offsets;
    unsigned char **glyphdata;
{
    fsQueryXBitmaps8Req *req;
    fsQueryXBitmaps8Reply reply;
    FSOffset   *offs;
    fsOffset32 local_offs;
    unsigned char *gd;
    int         left;
    int		i;

    GetReq(QueryXBitmaps8, req);
    req->fid = fid;
    req->range = range_type;
    req->format = format;
    req->num_ranges = str_len;
    req->length += (str_len + 3) >> 2;
    _FSSend(svr, (char *) str, str_len);

    /* get back the info */
    if (!_FSReply(svr, (fsReply *) & reply,
     (SIZEOF(fsQueryXBitmaps8Reply) - SIZEOF(fsGenericReply)) >> 2, fsFalse))
	return FSBadAlloc;

    offs = (FSOffset *) FSmalloc(sizeof(FSOffset) * reply.num_chars);
    *offsets = offs;
    if (!offs)
	return FSBadAlloc;
    left = (reply.length << 2) - SIZEOF(fsQueryXBitmaps8Reply)
	- (SIZEOF(fsOffset32) * reply.num_chars);
    gd = (unsigned char *) FSmalloc(left);
    *glyphdata = gd;
    if (!gd) {
	FSfree((char *) offs);
	return FSBadAlloc;
    }
    for (i=0; i<reply.num_chars; i++)
    {
	_FSReadPad(svr, (char *) &local_offs, (SIZEOF(fsOffset32)));
	offs->position = local_offs.position;
	offs->length = local_offs.length;
	offs++;
    }
    _FSReadPad(svr, (char *) gd, left);

    SyncHandle();
    return FSSuccess;
}

int
FSQueryXBitmaps16(svr, fid, format, range_type, str, str_len,
		  offsets, glyphdata)
    FSServer   *svr;
    Font        fid;
    FSBitmapFormat format;
    Bool        range_type;
    FSChar2b   *str;
    unsigned long str_len;
    FSOffset  **offsets;
    unsigned char **glyphdata;
{
    fsQueryXBitmaps16Req *req;
    fsQueryXBitmaps16Reply reply;
    FSOffset   *offs;
    fsOffset32 local_offs;
    unsigned char *gd;
    int         left;
    int		i;

    GetReq(QueryXBitmaps16, req);
    req->fid = fid;
    req->range = range_type;
    req->format = format;
    req->num_ranges = str_len;
    req->length += ((str_len * SIZEOF(fsChar2b)) + 3) >> 2;
    if (FSProtocolVersion(svr) == 1)
    {
	int i;
	fsChar2b_version1 *swapped_str;

	swapped_str = (fsChar2b_version1 *)
	    FSmalloc(SIZEOF(fsChar2b_version1) * str_len);
	if (!swapped_str)
	    return FSBadAlloc;
	for (i = 0; i < str_len; i++) {
	    swapped_str[i].low = str[i].low;
	    swapped_str[i].high = str[i].high;
	}
	_FSSend(svr, (char *)swapped_str, (str_len*SIZEOF(fsChar2b_version1)));
	FSfree(swapped_str);
    } else
	_FSSend(svr, (char *) str, (str_len * SIZEOF(fsChar2b)));

    /* get back the info */
    if (!_FSReply(svr, (fsReply *) & reply,
	      (SIZEOF(fsQueryXBitmaps16Reply) - SIZEOF(fsGenericReply)) >> 2,
		  fsFalse))
	return FSBadAlloc;

    offs = (FSOffset *) FSmalloc(sizeof(FSOffset) * reply.num_chars);
    *offsets = offs;
    if (!offs)
	return FSBadAlloc;
    left = (reply.length << 2) - SIZEOF(fsQueryXBitmaps16Reply)
	- (SIZEOF(fsOffset32) * reply.num_chars);
    gd = (unsigned char *) FSmalloc(left);
    *glyphdata = gd;
    if (!gd) {
	FSfree((char *) offs);
	return FSBadAlloc;
    }
    for (i=0; i<reply.num_chars; i++)
    {
	_FSReadPad(svr, (char *) &local_offs, (SIZEOF(fsOffset32)));
	offs->position = local_offs.position;
	offs->length = local_offs.length;
	offs++;
    }
    _FSReadPad(svr, (char *) gd, left);

    SyncHandle();
    return FSSuccess;
}
