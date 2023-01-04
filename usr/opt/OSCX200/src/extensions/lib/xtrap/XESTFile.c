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
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
/*****************************************************************************
Copyright 1987, 1988, 1989, 1990, 1991 by Digital Equipment Corp., Maynard, MA

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*****************************************************************************/
#include "Xos.h"
#include <sys/stat.h>
#include <errno.h>
#include "xtraplib.h"
#include "xtraplibp.h"

#ifndef lint
static char SCCSID[] = "@(#)XEStreamFile.c	1.4 - 90/08/02  ";
static char RCSID[] = "$Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/lib/xtrap/XESTFile.c,v 1.1.2.2 92/02/06 11:57:17 Jim_Ludwig Exp $";
#endif

#ifdef FUNCTION_PROTOS
int OpenFileStream(XESTREAM *st, char *file, CARD32 mode, int mask)
#else
int OpenFileStream(st, file, mode, mask)
    XESTREAM *st;
    char     *file;
    CARD32   mode;
    int      mask;
#endif
{
    int status = True;

    if ((st->u.D.fd = open(file, mode, mask)) < 0L)
    {
        SIOErr(st, strerror(errno));
        status = False;
    }
    return(status);
}

#ifdef FUNCTION_PROTOS
int CloseFileStream(XESTREAM *st)
#else
int CloseFileStream(st)
    XESTREAM *st;
#endif
{
    int status = True;

    if ((close(st->u.D.fd)) != 0L)
    {
        SIOErr(st, strerror(errno));
        status = False;
    }
    return(status);
}

#ifdef FUNCTION_PROTOS
int ReadFileStream(XESTREAM *st, void *buf, CARD16 size)
#else
int ReadFileStream(st, buf, size)
    XESTREAM *st;
    void     *buf;
    CARD16    size;
#endif
{
    return(read(st->u.D.fd, buf, size));
}

#ifdef FUNCTION_PROTOS
int WriteFileStream(XESTREAM *st, void *buf, CARD16 size)
#else
int WriteFileStream(st, buf, size)
    XESTREAM *st;
    void     *buf;
    CARD16   size;
#endif
{
    return(write(st->u.D.fd, buf, size));
}
