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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: miscutil_static.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 92/09/25 20:35:40 $";
#endif
/*
 * $XConsortium: miscutil.c,v 1.2 91/05/13 16:40:27 gildea Exp $
 * 
 * Copyright 1991 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission.  M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * The X Window System is a Trademark of MIT.
 */

#define XK_LATIN1
#include    <X11/keysymdef.h>
/* #include    <X11/Xmu/CharSet.h> */

/* make sure everything initializes themselves at least once */

 /*
 * AGH!!!!!!!!!!!!!!!
 * This redefines the one in the server. 
 * This is only for a static library 
 * */
long serverGeneration = 1;

unsigned long *
Xalloc (m)
{
    return (unsigned long *) malloc (m);
}

unsigned long *
Xrealloc (n,m)
    unsigned long   *n;
{
    if (!n)
	return (unsigned long *) malloc (m);
    else
	return (unsigned long *) realloc ((char *) n, m);
}

Xfree (n)
    unsigned long   *n;
{
    if (n)
	free ((char *) n);
}

CopyISOLatin1Lowered (dst, src, len)
    char    *dst, *src;
    int	    len;
{
    register unsigned char *dest, *source;

    for (dest = (unsigned char *)dst, source = (unsigned char *)src;
	 *source && len > 0;
	 source++, dest++, len--)
    {
	if ((*source >= XK_A) && (*source <= XK_Z))
	    *dest = *source + (XK_a - XK_A);
	else if ((*source >= XK_Agrave) && (*source <= XK_Odiaeresis))
	    *dest = *source + (XK_agrave - XK_Agrave);
	else if ((*source >= XK_Ooblique) && (*source <= XK_Thorn))
	    *dest = *source + (XK_oslash - XK_Ooblique);
	else
	    *dest = *source;
    }
    *dest = '\0';
}
