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
/* $XConsortium: XStNColor.c,v 11.22 91/07/22 22:35:01 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

/*
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

#include <stdio.h>
#include "Xlibint.h"
#include "Xcmsint.h"

extern void _XcmsRGB_to_XColor();

#if NeedFunctionPrototypes
XStoreNamedColor(
register Display *dpy,
Colormap cmap,
_Xconst char *name, /* STRING8 */
unsigned long pixel, /* CARD32 */
int flags)  /* DoRed, DoGreen, DoBlue */
#else
XStoreNamedColor(dpy, cmap, name, pixel, flags)
register Display *dpy;
Colormap cmap;
char *name; /* STRING8 */
unsigned long pixel; /* CARD32 */
int flags;  /* DoRed, DoGreen, DoBlue */
#endif
{
    unsigned int nbytes;
    register xStoreNamedColorReq *req;
    XcmsCCC ccc;
    XcmsColor cmsColor_exact;
    XColor scr_def;

    /*
     * Let's Attempt to use TekCMS approach to Parse Color
     */
    if ((ccc = XcmsCCCOfColormap(dpy, cmap)) != (XcmsCCC)NULL) {
	if (_XcmsResolveColorString(ccc, &name, &cmsColor_exact,
		XcmsRGBFormat) != XcmsFailure) {
	    _XcmsRGB_to_XColor(&cmsColor_exact, &scr_def, 1);
	    scr_def.pixel = pixel;
	    scr_def.flags = flags;
	    XStoreColor(dpy, cmap, &scr_def);
	    return;
	}
	/*
	 * Otherwise we failed; or name was changed with yet another
	 * name.  Thus pass name to the X Server.
	 */
    }

    /*
     * The TekCMS and i18n methods failed, so lets pass it to the server
     * for parsing.
     */

    LockDisplay(dpy);
    GetReq(StoreNamedColor, req);

    req->cmap = cmap;
    req->flags = flags;
    req->pixel = pixel;
    req->nbytes = nbytes = strlen(name);
    req->length += (nbytes + 3) >> 2; /* round up to multiple of 4 */
    Data(dpy, name, (long)nbytes);
    UnlockDisplay(dpy);
    SyncHandle();
}


