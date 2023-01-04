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
/* $XConsortium: XFontInfo.c,v 11.21 91/01/29 08:40:25 rws Exp $ */
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

#define NEED_REPLIES
#include "Xlibint.h"

#if NeedFunctionPrototypes
char **XListFontsWithInfo(
register Display *dpy,
_Xconst char *pattern,  /* null-terminated */
int maxNames,
int *actualCount,	/* RETURN */
XFontStruct **info)	/* RETURN */
#else
char **XListFontsWithInfo(dpy, pattern, maxNames, actualCount, info)
register Display *dpy;
char *pattern;  /* null-terminated */
int maxNames;
int *actualCount;	/* RETURN */
XFontStruct **info;	/* RETURN */
#endif
{       
    register long nbytes;
    register int i;
    register XFontStruct *fs;
    register int size = 0;
    XFontStruct *finfo = NULL;
    char **flist = NULL;
    xListFontsWithInfoReply reply;
    register xListFontsReq *req;
    int j;

    LockDisplay(dpy);
    GetReq(ListFontsWithInfo, req);
    req->maxNames = maxNames;
    nbytes = req->nbytes = pattern ? strlen (pattern) : 0;
    req->length += (nbytes + 3) >> 2;
    _XSend (dpy, pattern, nbytes);
    /* use _XSend instead of Data, since subsequent _XReply will flush buffer */

    for (i = 0; ; i++) {
	if (!_XReply (dpy, (xReply *) &reply,
		      ((SIZEOF(xListFontsWithInfoReply) - 
			SIZEOF(xGenericReply)) >> 2), xFalse)) {
	    for (j=(i-1); (j >= 0); j--) {
		Xfree(flist[j]);
		if (finfo[j].properties) Xfree((char *) finfo[j].properties);
	    }
	    if (flist) Xfree((char *) flist);
	    if (finfo) Xfree((char *) finfo);
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return ((char **) NULL);
	}
	if (reply.nameLength == 0)
	    break;
	if ((i + reply.nReplies) >= size) {
	    size = i + reply.nReplies + 1;

	    if (finfo) {
		XFontStruct * tmp_finfo = (XFontStruct *) 
		    Xrealloc ((char *) finfo,
			      (unsigned) (sizeof(XFontStruct) * size));
		char ** tmp_flist = (char **)
		    Xrealloc ((char *) flist,
			      (unsigned) (sizeof(char *) * (size+1)));

		if ((! tmp_finfo) || (! tmp_flist)) {
		    /* free all the memory that we allocated */
		    for (j=(i-1); (j >= 0); j--) {
			Xfree(flist[j]);
			if (finfo[j].properties)
			    Xfree((char *) finfo[j].properties);
		    }
		    if (tmp_flist) Xfree((char *) tmp_flist);
		    else Xfree((char *) flist);
		    if (tmp_finfo) Xfree((char *) tmp_finfo);
		    else Xfree((char *) finfo);
		    goto clearwire;
		}
		finfo = tmp_finfo;
		flist = tmp_flist;
	    }
	    else {
		if (! (finfo = (XFontStruct *)
		       Xmalloc((unsigned) (sizeof(XFontStruct) * size))))
		    goto clearwire;
		if (! (flist = (char **)
		       Xmalloc((unsigned) (sizeof(char *) * (size+1))))) {
		    Xfree((char *) finfo);
		    goto clearwire;
		}
	    }
	}
	fs = &finfo[i];

	fs->ext_data 		= NULL;
	fs->per_char		= NULL;
	fs->fid 		= None;
	fs->direction 		= reply.drawDirection;
	fs->min_char_or_byte2	= reply.minCharOrByte2;
	fs->max_char_or_byte2 	= reply.maxCharOrByte2;
	fs->min_byte1 		= reply.minByte1;
	fs->max_byte1 		= reply.maxByte1;
	fs->default_char	= reply.defaultChar;
	fs->all_chars_exist 	= reply.allCharsExist;
	fs->ascent 		= cvtINT16toInt (reply.fontAscent);
	fs->descent 		= cvtINT16toInt (reply.fontDescent);
    
#ifdef MUSTCOPY
	{
	    xCharInfo *xcip;

	    xcip = (xCharInfo *) &reply.minBounds;
	    fs->min_bounds.lbearing = xcip->leftSideBearing;
	    fs->min_bounds.rbearing = xcip->rightSideBearing;
	    fs->min_bounds.width = xcip->characterWidth;
	    fs->min_bounds.ascent = xcip->ascent;
	    fs->min_bounds.descent = xcip->descent;
	    fs->min_bounds.attributes = xcip->attributes;

	    xcip = (xCharInfo *) &reply.maxBounds;
	    fs->max_bounds.lbearing = xcip->leftSideBearing;
	    fs->max_bounds.rbearing = xcip->rightSideBearing;
	    fs->max_bounds.width = xcip->characterWidth;
	    fs->max_bounds.ascent = xcip->ascent;
	    fs->max_bounds.descent = xcip->descent;
	    fs->max_bounds.attributes = xcip->attributes;
	}
#else
	/* XXX the next two statements won't work if short isn't 16 bits */
	fs->min_bounds = * (XCharStruct *) &reply.minBounds;
	fs->max_bounds = * (XCharStruct *) &reply.maxBounds;
#endif /* MUSTCOPY */

	fs->n_properties = reply.nFontProps;
	if (fs->n_properties > 0) {
	    nbytes = reply.nFontProps * sizeof(XFontProp);
	    if (! (fs->properties = (XFontProp *) Xmalloc((unsigned) nbytes)))
		goto badmem;
	    nbytes = reply.nFontProps * SIZEOF(xFontProp);
#if ( defined (__osf__) && defined(__alpha) )
	    {
		/* sizeof(CARD32) != sizeof(long) */
		xFontProp * props;
		register int i;
		props = (xFontProp *)_XAllocScratch(dpy,(unsigned long) nbytes);
	        _XRead32 (dpy, (char *)props, nbytes);
		for ( i = 0; i < reply.nFontProps; i++ ) {
		    fs->properties[i].name = props[i].name;
		    fs->properties[i].card32 = props[i].value;
		}
	    }
#else
	    _XRead32 (dpy, (char *)fs->properties, nbytes);
#endif

	} else
	    fs->properties = NULL;

	j = reply.nameLength + 1;
	if (!i)
	    j++; /* make first string 1 byte longer, to match XListFonts */
	flist[i] = (char *) Xmalloc ((unsigned int) j);
	if (! flist[i]) {
	    if (finfo[i].properties) Xfree((char *) finfo[i].properties);
	    nbytes = reply.nameLength + 3 & ~3;
	    _XEatData(dpy, (unsigned long) nbytes);
	    goto badmem;
	}
	if (!i) {
	    *flist[0] = 0; /* zero to distinguish from XListFonts */
	    flist[0]++;
	}
	flist[i][reply.nameLength] = '\0';
	_XReadPad (dpy, flist[i], (long) reply.nameLength);
    }
    *info = finfo;
    *actualCount = i;
    if (flist)
	flist[i] = NULL; /* required in case XFreeFontNames is called */
    UnlockDisplay(dpy);
    SyncHandle();
    return (flist);


  badmem:
    /* Free all memory allocated by this function. */
    for (j=(i-1); (j >= 0); j--) {
	Xfree(flist[j]);
	if (finfo[j].properties) Xfree((char *) finfo[j].properties);
    }
    if (flist) Xfree((char *) flist);
    if (finfo) Xfree((char *) finfo);

  clearwire:
    /* Clear the wire. */
    do {
	if (reply.nFontProps)
	    _XEatData(dpy, (unsigned long)
		      (reply.nFontProps * SIZEOF(xFontProp)));
	nbytes = (reply.nameLength + 3) & ~3;
	_XEatData(dpy, (unsigned long) nbytes);
    }
    while (_XReply(dpy,(xReply *) &reply, ((SIZEOF(xListFontsWithInfoReply) -
					    SIZEOF(xGenericReply)) >> 2),
		   xFalse) && (reply.nameLength != 0));

    UnlockDisplay(dpy);
    SyncHandle();
    return (char **) NULL;
}


XFreeFontInfo (names, info, actualCount)
char **names;
XFontStruct *info;
int actualCount;
{
	register int i;
	if (names) {
		Xfree (names[0]-1);
		for (i = 1; i < actualCount; i++) {
			Xfree (names[i]);
		}
		Xfree((char *) names);
	}
	if (info) {
		for (i = 0; i < actualCount; i++) {
			if (info[i].per_char)
				Xfree ((char *) info[i].per_char);
			if (info[i].properties)
				Xfree ((char *) info[i].properties);
			}
		Xfree((char *) info);
	}
}
