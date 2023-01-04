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
/* $XConsortium: XChProp.c,v 11.22 91/01/06 11:44:22 rws Exp $ */
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

#include "Xlibint.h"

#if NeedFunctionPrototypes
XChangeProperty (
    register Display *dpy,
    Window w,
    Atom property,
    Atom type,
    int format,  /* 8, 16, or 32 */
    int mode,  /* PropModeReplace, PropModePrepend, PropModeAppend */
    _Xconst unsigned char *data,
    int nelements)
#else
XChangeProperty (dpy, w, property, type, format, mode, data, nelements)
    register Display *dpy;
    Window w;
    Atom property, type;
    int format;  /* 8, 16, or 32 */
    int mode;  /* PropModeReplace, PropModePrepend, PropModeAppend */
    unsigned char *data;
    int nelements;
#endif
    {
    register xChangePropertyReq *req;
    register long len;

    LockDisplay(dpy);
    GetReq (ChangeProperty, req);
    req->window = w;
    req->property = property;
    req->type = type;
    req->format = format;
    req->mode = mode;
    if (nelements < 0) {
	req->nUnits = 0;
	req->format = 0; /* ask for garbage, get garbage */
    } else
	req->nUnits = nelements;
    
    switch (format) {
      case 8:
	len = req->length + (((long)nelements + 3)>>2);
	if (len <= 65535) {
	    req->length = len;
	    Data (dpy, (char *)data, nelements);
	} /* else force BadLength */
        break;
 
      case 16:
	len = req->length + (((long)nelements + 1)>>1);
	if (len <= 65535) {
	    req->length = len;
	    len = (long)nelements << 1;
	    Data16 (dpy, (short *) data, len);
	} /* else force BadLength */
	break;

      case 32:
	len = req->length + (long)nelements;
	if (len <= 65535) {
	    req->length = len;
	    len = (long)nelements << 2;
#if ( defined (__osf__) && defined(__alpha) )
	    {
	    /* sizeof(CARD32) != sizeof(long) */
	    int i;
	    CARD32 *cptr;
	    unsigned long *lptr = (unsigned long *) data;
	    /* per clarification of spec.. unsigned longs are passed
	       into this function but only the lower 32 bits will
	       actually be used - ddhill */
	    cptr = (CARD32 *)_XAllocScratch(dpy,(long)sizeof(CARD32) * len);
	    for(i=0;i<nelements;i++)
		cptr[i] = lptr[i];
	    Data32 (dpy, (long *) cptr, len);
	    }
	    
#else
	    Data32 (dpy, (long *) data, len);
#endif
	} /* else force BadLength */
	break;

      default:
        /* BadValue will be generated */ ;
      }

    UnlockDisplay(dpy);
    SyncHandle();
    }





