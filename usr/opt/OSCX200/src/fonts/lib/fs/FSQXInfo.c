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
/* $XConsortium: FSQXInfo.c,v 1.4 92/11/18 21:31:17 gildea Exp $ */
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

#include	"FSlibint.h"

/*
 * Note:  only the range in the first FSQuery is sent to the server.
 * the others exist as return values only.
 */

int
FSQueryXInfo(svr, fid, info, props, offsets, prop_data)
    FSServer   *svr;
    Font        fid;
    FSXFontInfoHeader *info;
    FSPropInfo *props;
    FSPropOffset **offsets;
    unsigned char **prop_data;
{
    fsQueryXInfoReq *req;
    fsQueryXInfoReply reply;
    FSPropOffset *offset_data;
    unsigned char *pdata;
    fsPropInfo local_pi;
    fsPropOffset local_po;
    int j;

    GetReq(QueryXInfo, req);
    req->id = fid;

    /* get back the info */
    if (!_FSReply(svr, (fsReply *) & reply, ((SIZEOF(fsQueryXInfoReply) -
			    SIZEOF(fsGenericReply)) >> 2), fsFalse)) {
	return FSBadAlloc;
    }

    FSUnpack_XFontInfoHeader(&reply, info, FSProtocolVersion(svr));

    /* get the prop header */
    _FSReadPad(svr, (char *) &local_pi, SIZEOF(fsPropInfo));
    props->num_offsets = local_pi.num_offsets;
    props->data_len = local_pi.data_len;

    /* prepare for prop data */
    offset_data = (FSPropOffset *)
	FSmalloc(props->num_offsets * sizeof(FSPropOffset));
    if (!offset_data)
	return FSBadAlloc;
    pdata = (unsigned char *) FSmalloc(props->data_len);
    if (!pdata) {
	FSfree((char *) offset_data);
	return FSBadAlloc;
    }
    /* get offsets */
    for (j=0; j<props->num_offsets; j++)
    {
	_FSReadPad(svr, (char *) &local_po, SIZEOF(fsPropOffset));
	offset_data[j].name.position = local_po.name.position;
	offset_data[j].name.length = local_po.name.length;
	offset_data[j].value.position = local_po.value.position;
	offset_data[j].value.length = local_po.value.length;
	offset_data[j].type = local_po.type;
    }

    /* get data */
    _FSReadPad(svr, (char *) pdata, props->data_len);
    *offsets = offset_data;
    *prop_data = pdata;

    SyncHandle();
    return FSSuccess;
}
