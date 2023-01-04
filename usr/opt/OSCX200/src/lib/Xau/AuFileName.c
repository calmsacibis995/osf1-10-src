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
/*
 * Xau - X Authorization Database Library
 *
 * $XConsortium: AuFileName.c,v 1.2 91/01/08 15:09:00 gildea Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include <X11/Xauth.h>

char *
XauFileName ()
{
    char    *name, *malloc (), *getenv ();
    char    *strcat (), *strcpy ();
    static char	*buf;
    static int	bsize;
    int	    size;

    if (name = getenv ("XAUTHORITY")) {
	return name;
    } else if (name = getenv ("HOME")) {
	size = strlen (name) + strlen(".Xauthority") + 2;
	if (size > bsize) {
	    if (buf)
		free (buf);
	    buf = malloc ((unsigned) size);
	    if (!buf)
		return 0;
	    bsize = size;
	}
	strcpy (buf, name);
	strcat (buf, "/.Xauthority" + (name[1] == '\0' ? 1 : 0));
	return buf;
    }
    return 0;
}
