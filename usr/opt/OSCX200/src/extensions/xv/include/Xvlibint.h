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
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

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
#ifndef XVLIBINT_H
#define XVLIBINT_H
/*
** File: 
**
**   Xvlibint.h --- Xv library internal header file
**
** Author: 
**
**   David Carver (Digital Workstation Engineering/Project Athena)
**
** Revisions:
**
**   01.24.91 Carver
**     - version 1.4 upgrade
**
*/

#define NEED_REPLIES

#include "Xlibint.h"
#include "Xext.h"
#include "extutil.h"
#include "Xvproto.h"
#include "Xvlib.h"

#if defined(__STDC__) && !defined(UNIXCPP)
#define XvGetReq(name, req) \
        WORD64ALIGN\
	if ((dpy->bufptr + SIZEOF(xv##name##Req)) > dpy->bufmax)\
		_XFlush(dpy);\
	req = (xv##name##Req *)(dpy->last_req = dpy->bufptr);\
	req->reqType = info->codes->major_opcode;\
        req->xvReqType = xv_##name; \
        req->length = (SIZEOF(xv##name##Req))>>2;\
	dpy->bufptr += SIZEOF(xv##name##Req);\
	dpy->request++

#else  /* non-ANSI C uses empty comment instead of "##" for token concatenation */
#define XvGetReq(name, req) \
        WORD64ALIGN\
	if ((dpy->bufptr + SIZEOF(xv/**/name/**/Req)) > dpy->bufmax)\
		_XFlush(dpy);\
	req = (xv/**/name/**/Req *)(dpy->last_req = dpy->bufptr);\
	req->reqType = info->codes->major_opcode;\
	req->xvReqType = xv_/**/name;\
	req->length = (SIZEOF(xv/**/name/**/Req))>>2;\
	dpy->bufptr += SIZEOF(xv/**/name/**/Req);\
	dpy->request++
#endif


#if defined(__STDC__) && !defined(UNIXCPP)
#define XvGetReqExtra(name, n, req) \
        WORD64ALIGN\
	if ((dpy->bufptr + SIZEOF(xv##name##Req) + n) > dpy->bufmax)\
		_XFlush(dpy);\
	req = (xv##name##Req *)(dpy->last_req = dpy->bufptr);\
	req->reqType = info->codes->major_opcode;\
	req->xvReqType = xv_##name;\
	req->length = (SIZEOF(xv##name##Req) + n)>>2;\
	dpy->bufptr += SIZEOF(xv##name##Req) + n;\
	dpy->request++
#else
#define XvGetReqExtra(name, n, req) \
        WORD64ALIGN\
	if ((dpy->bufptr + SIZEOF(xv/**/name/**/Req) + n) > dpy->bufmax)\
		_XFlush(dpy);\
	req = (xv/**/name/**/Req *)(dpy->last_req = dpy->bufptr);\
	req->reqType = info->codes->major_opcode;\
	req->xvReqType = xv_/**/name;\
	req->length = (SIZEOF(xv/**/name/**/Req) + n)>>2;\
	dpy->bufptr += SIZEOF(xv/**/name/**/Req) + n;\
	dpy->request++
#endif


#endif XVLIBINT_H
