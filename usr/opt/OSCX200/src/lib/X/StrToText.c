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
/* $XConsortium: StrToText.c,v 1.4 91/01/08 14:40:18 gildea Exp $ */
/* Copyright 1989 Massachusetts Institute of Technology */

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

#include <X11/Xlibint.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

/*
 * XStringListToTextProperty - fill in TextProperty structure with 
 * concatenated list of null-separated strings.  Return True if successful 
 * else False.  Allocate room on end for trailing NULL, but don't include in
 * count.
 */

Status XStringListToTextProperty (argv, argc, textprop)
    char **argv;
    int argc;
    XTextProperty *textprop;
{
    register int i;
    register unsigned int nbytes;
    XTextProperty proto;

    /* figure out how much space we'll need for this list */
    for (i = 0, nbytes = 0; i < argc; i++) {
	nbytes += (unsigned) ((argv[i] ? strlen (argv[i]) : 0) + 1);
    }

    /* fill in a prototype containing results so far */
    proto.encoding = XA_STRING;
    proto.format = 8;
    if (nbytes)
	proto.nitems = nbytes - 1;	/* subtract one for trailing <NUL> */
    else
	proto.nitems = 0;
    proto.value = NULL;

    /* build concatenated list of strings */
    if (nbytes > 0) {
	register char *buf = Xmalloc (nbytes);
	if (!buf) return False;

	proto.value = (unsigned char *) buf;
	for (i = 0; i < argc; i++) {
	    char *arg = argv[i];

	    if (arg) {
		(void) strcpy (buf, arg);
		buf += (strlen (arg) + 1);
	    } else {
		*buf++ = '\0';
	    }
	}
    } else {
	proto.value = (unsigned char *) Xmalloc (1);	/* easier for client */
	if (!proto.value) return False;

	proto.value[0] = '\0';
    }

    /* we were successful, so set return value */
    *textprop = proto;
    return True;
}
