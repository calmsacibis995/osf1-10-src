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
/* $XConsortium: fontinfo.c,v 1.9 92/11/18 21:30:13 gildea Exp $ */
/*
 * font data query
 */
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

#include        "FS.h"
#include        "FSproto.h"
#include        <stdio.h>
#include        <X11/Xos.h>
#include        "clientstr.h"
#include        "difsfontst.h"
#include        "fontstruct.h"
#include        "closestr.h"
#include        "globals.h"

extern void (*ReplySwapVector[NUM_PROC_VECTORS]) ();

void
CopyCharInfo(ci, dst)
    CharInfoPtr ci;
    fsXCharInfo *dst;
{
    xCharInfo  *src = &ci->metrics;

    dst->ascent = src->ascent;
    dst->descent = src->descent;
    dst->left = src->leftSideBearing;
    dst->right = src->rightSideBearing;
    dst->width = src->characterWidth;
    dst->attributes = 0;
}


int
convert_props(pinfo, props)
    FontInfoPtr pinfo;
    fsPropInfo **props;
{
    int i;
    int data_len, cur_off;
    char *str;
    pointer ptr, off_ptr, string_base;
    fsPropOffset local_offset;

    /*
     * compute the size of the property data
     */
    data_len = 0;
    for (i = 0; i < pinfo->nprops; i++)
    {
	data_len += strlen(NameForAtom(pinfo->props[i].name));
	if (pinfo->isStringProp[i])
	    data_len += strlen(NameForAtom(pinfo->props[i].value));
    }

    /*
     * allocate the single chunk that the difs layer requires
     */
    ptr = (pointer) fsalloc(SIZEOF(fsPropInfo)
			    + SIZEOF(fsPropOffset) * pinfo->nprops
			    + data_len);
    if (!ptr)
	return AllocError;
    string_base = ptr + SIZEOF(fsPropInfo) + SIZEOF(fsPropOffset) * pinfo->nprops;
  
    /*
     * copy in the header
     */
    ((fsPropInfo *)ptr)->num_offsets = pinfo->nprops;
    ((fsPropInfo *)ptr)->data_len = data_len;

    /*
     * compute the offsets and copy the string data
     */
    off_ptr = ptr + SIZEOF(fsPropInfo);
    cur_off = 0;
    for (i = 0; i < pinfo->nprops; i++)
    {
	local_offset.name.position = cur_off;
	str = NameForAtom(pinfo->props[i].name);
	local_offset.name.length = strlen(str);
	bcopy(str, string_base+cur_off, local_offset.name.length);
	cur_off += local_offset.name.length;
	if (pinfo->isStringProp[i])
	{
	    local_offset.value.position = cur_off;
	    str = NameForAtom(pinfo->props[i].value);
	    local_offset.value.length = strlen(str);
	    bcopy(str, string_base+cur_off, local_offset.value.length);
	    cur_off += local_offset.value.length;
	    local_offset.type = PropTypeString;
	} else {
	    local_offset.value.position = pinfo->props[i].value;
	    local_offset.value.length = 0; /* protocol says must be zero */
	    local_offset.type = PropTypeSigned;
	}
	bcopy(&local_offset, off_ptr, SIZEOF(fsPropOffset));
	off_ptr += SIZEOF(fsPropOffset);
    }

    assert(off_ptr == string_base);
    assert(cur_off == data_len);

    *props = (fsPropInfo *) ptr;
    return Successful;
}


/*
 * does the real work of turning a list of range (or chars) into
 * a list of ranges
 */
static fsRange *
build_range(type, src, item_size, num, all)
    Bool        type;
    pointer     src;
    int         item_size;
    int        *num;
    Bool       *all;
{
    fsRange    *new = (fsRange *) 0,
               *np;
    unsigned long src_num;
    int         i;

    if (type) {			/* range flag is set, deal with data as a list
				 * of char2bs */
	char *rp = (char *) src;

	src_num = *num / 2;
	if (src_num == 0) {
	    *all = TRUE;
	    return new;
	}
/* XXX - handle odd length list -- this could get nasty since
 * it has to poke into the font
 */
	/* yes, a real "sizeof".  This is not a protocol object.
	   XXX - probably should use a different struct */
	np = new = (fsRange *) fsalloc(sizeof(fsRange) * src_num);
	if (!np) {
	    return np;
	}
	/* unpack the ranges from the protocol buffer */
	for (i = 0; i < src_num; i++) {
	    np->min_char_high = *rp++;
	    np->min_char_low  = *rp++;
	    np->max_char_high = *rp++;
	    np->max_char_low  = *rp++;
	    np++;
	}
	*num = src_num;
	return new;
    } else {			/* deal with data as a list of characters */
	pointer     pp = src;

	src_num = *num / item_size;
	np = new = (fsRange *) fsalloc(SIZEOF(fsRange) * src_num);
	if (!np) {
	    return np;
	}
	/* convert each char to a range */
	for (i = 0; i < src_num; i++) {
	    if (item_size == 1) {
		np->min_char_low = *pp;
		np->min_char_high = 0;
	    } else {
		np->min_char_low = ((fsChar2b *) pp)->low;
		np->min_char_high = ((fsChar2b *) pp)->high;
	    }
/* XXX - eventually this should get smarter, and coallesce ranges */
	    np->max_char_high = np->min_char_high;
	    np->max_char_low = np->min_char_low;
	    np++;
	    pp += item_size;
	}
	*num = src_num / item_size;
	return new;
    }
}

/*
 * provide backward compatibility with version 1, which had
 * the bytes of char2b backwards
 */
static void
swap_char2b (values, number)
    fsChar2b *values;
    int number;
{
    fsChar2b temp;
    int i;

    for (i = 0; i < number; i++) {
	temp.low = ((fsChar2b_version1 *)values)->low;
	temp.high = ((fsChar2b_version1 *)values)->high;
	*values++ = temp;
    }
}


static Bool
do_query_extents(client, c)
    ClientPtr   client;
    QEclosurePtr c;
{
    int         err;
    unsigned long lendata,
                num_extents;
    fsXCharInfo *extents;
    fsQueryXExtents8Reply reply;

    err = GetExtents (c->client, c->pfont,
		     c->flags, c->nranges, c->range, &num_extents, &extents);
    if (err == Suspended) {
	if (!c->slept) {
	    c->slept = TRUE;
	    ClientSleep(client, do_query_extents, (pointer) c);
	}
	return TRUE;
    }
    if (err != Successful) {
	SendErrToClient(c->client, FontToFSError(err), (pointer) 0);
	goto finish;
    }
    reply.type = FS_Reply;
    reply.sequenceNumber = c->client->sequence;
    reply.num_extents = num_extents;
    lendata = SIZEOF(fsXCharInfo) * num_extents;
    reply.length = (SIZEOF(fsQueryXExtents8Reply) + lendata) >> 2;
    if (client->swapped)
	SwapExtents(extents, num_extents);
    WriteReplyToClient(c->client, SIZEOF(fsQueryXExtents8Reply), &reply);
    (void) WriteToClient(c->client, lendata, (char *) extents);
    fsfree((char *) extents);
finish:
    if (c->slept)
	ClientWakeup(c->client);
    fsfree(c->range);
    fsfree(c);
    return TRUE;
}

int
QueryExtents(client, cfp, item_size, nranges, range_flag, range_data)
    ClientPtr   client;
    ClientFontPtr cfp;
    int         item_size;
    int         nranges;
    Bool        range_flag;
    pointer     range_data;
{
    QEclosurePtr c;
    fsRange    *fixed_range;
    Bool        all_glyphs = FALSE;

    if (item_size == 2  &&  client->major_version == 1)
	swap_char2b (range_data, nranges);

    fixed_range = build_range(range_flag, range_data, item_size,
			      &nranges, &all_glyphs);

    if (!fixed_range && !all_glyphs) {
	SendErrToClient(client, FSBadRange, 0);
	return FSBadRange;
    }
    c = (QEclosurePtr) fsalloc(sizeof(QEclosureRec));
    if (!c)
	return FSBadAlloc;
    c->client = client;
    c->slept = FALSE;
    c->pfont = cfp->font;
    c->flags = (all_glyphs) ? LoadAll : 0;
    c->flags |= (item_size == 1) ? EightBitFont : SixteenBitFont;
    c->nranges = nranges;
    c->range = fixed_range;

    (void) do_query_extents(client, c);
    return FSSuccess;
}

static Bool
do_query_bitmaps(client, c)
    ClientPtr   client;
    QBclosurePtr c;
{
    int         err;
    unsigned long num_glyphs, data_size;
    fsOffset32   *offsets;
    pointer     glyph_data;
    fsQueryXBitmaps8Reply reply;
    int		freedata;

    err = GetBitmaps (c->client, c->pfont, c->format,
				    c->flags, c->nranges, c->range,
			     &data_size, &num_glyphs, &offsets, &glyph_data, &freedata);

    if (err == Suspended) {
	if (!c->slept) {
	    c->slept = TRUE;
	    ClientSleep(client, do_query_bitmaps, (pointer) c);
	}
	return TRUE;
    }
    if (err != Successful) {
	SendErrToClient(c->client, FontToFSError(err), (pointer) 0);
	goto finish;
    }
    reply.type = FS_Reply;
    reply.sequenceNumber = c->client->sequence;
    reply.replies_hint = 0;
    reply.num_chars = num_glyphs;
    reply.nbytes = data_size;
    reply.length = (SIZEOF(fsQueryXBitmaps8Reply) + data_size +
		    (SIZEOF(fsOffset32) * num_glyphs) + 3) >> 2;

    WriteReplyToClient(c->client, SIZEOF(fsQueryXBitmaps8Reply), &reply);
    if (client->swapped)
	SwapLongs((long *)offsets, num_glyphs * 2);
    (void) WriteToClient(c->client, (num_glyphs * SIZEOF(fsOffset32)),
			 (char *) offsets);
    (void) WriteToClient(c->client, data_size, (char *) glyph_data);
    fsfree((char *) offsets);
    if (freedata)
	fsfree((char *) glyph_data);
finish:
    if (c->slept)
	ClientWakeup(c->client);
    fsfree(c->range);
    fsfree(c);
    return TRUE;
}

int
QueryBitmaps(client, cfp, item_size, format, nranges, range_flag, range_data)
    ClientPtr   client;
    ClientFontPtr cfp;
    int         item_size;
    fsBitmapFormat format;
    int         nranges;
    Bool        range_flag;
    pointer     range_data;
{
    QBclosurePtr c;
    fsRange    *fixed_range;
    Bool        all_glyphs = FALSE;

    if (item_size == 2  &&  client->major_version == 1)
	swap_char2b (range_data, nranges);

    fixed_range = build_range(range_flag, range_data, item_size,
			      &nranges, &all_glyphs);

    if (!fixed_range && !all_glyphs) {
	SendErrToClient(client, FSBadRange, 0);
	return FSBadRange;
    }
    c = (QBclosurePtr) fsalloc(sizeof(QBclosureRec));
    if (!c)
	return FSBadAlloc;
    c->client = client;
    c->slept = FALSE;
    c->pfont = cfp->font;
    c->flags = (all_glyphs) ? LoadAll : 0;
    c->nranges = nranges;
    c->range = fixed_range;
    c->format = format;

    (void) do_query_bitmaps(client, c);
    return FSSuccess;
}
