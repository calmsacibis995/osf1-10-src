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
 * $XConsortium: bufio.c,v 1.3 92/02/11 17:17:39 eswu Exp $
 *
 * Copyright 1991 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */


#include    <fontmisc.h>
#include    <bufio.h>
#include    <errno.h>
extern int errno;

BufFilePtr
BufFileCreate (private, io, skip, close)
    char    *private;
    int	    (*io)();
    int	    (*skip)();
    int	    (*close)();
{
    BufFilePtr	f;

    f = (BufFilePtr) xalloc (sizeof *f);
    if (!f)
	return 0;
    f->private = private;
    f->bufp = f->buffer;
    f->left = 0;
    f->io = io;
    f->skip = skip;
    f->close = close;
    return f;
}

#define FileDes(f)  ((int) (f)->private)

static int
BufFileRawFill (f)
    BufFilePtr	f;
{
    int	left;

    left = read (FileDes(f), f->buffer, BUFFILESIZE);
    if (left <= 0) {
	f->left = 0;
	return BUFFILEEOF;
    }
    f->left = left - 1;
    f->bufp = f->buffer + 1;
    return f->buffer[0];
}

static int
BufFileRawSkip (f, count)
    BufFilePtr	f;
    int		count;
{
    int	    curoff;
    int	    fileoff;
    int	    todo;

    curoff = f->bufp - f->buffer;
    fileoff = curoff + f->left;
    if (curoff + count <= fileoff) {
	f->bufp += count;
	f->left -= count;
    } else {
	todo = count - (fileoff - curoff);
	if (lseek (FileDes(f), todo, 1) == -1) {
	    if (errno != ESPIPE)
		return BUFFILEEOF;
	    while (todo) {
		curoff = BUFFILESIZE;
		if (curoff > todo)
		    curoff = todo;
		fileoff = read (FileDes(f), f->buffer, curoff);
		if (fileoff <= 0)
		    return BUFFILEEOF;
		todo -= fileoff;
	    }
	}
	f->left = 0;
    }
    return count;
}

static int
BufFileRawClose (f, doClose)
    BufFilePtr	f;
{
    if (doClose)
	close (FileDes (f));
    return 1;
}

BufFilePtr
BufFileOpenRead (fd)
    int	fd;
{
    return BufFileCreate ((char *) fd, BufFileRawFill, BufFileRawSkip, BufFileRawClose);
}

static
BufFileRawFlush (c, f)
    int		c;
    BufFilePtr	f;
{
    int	cnt;

    if (c != BUFFILEEOF)
	*f->bufp++ = c;
    cnt = f->bufp - f->buffer;
    f->bufp = f->buffer;
    f->left = BUFFILESIZE;
    if (write (FileDes(f), f->buffer, cnt) != cnt)
	return BUFFILEEOF;
    return c;
}

BufFilePtr
BufFileOpenWrite (fd)
    int	fd;
{
    BufFilePtr	f;

    f = BufFileCreate ((char *) fd, BufFileRawFlush, 0, BufFileFlush);
    f->bufp = f->buffer;
    f->left = BUFFILESIZE;
    return f;
}

BufFileRead (f, b, n)
    BufFilePtr	f;
    char	*b;
    int		n;
{
    int	    c, cnt;
    cnt = n;
    while (cnt--) {
	c = BufFileGet (f);
	if (c == BUFFILEEOF)
	    break;
	*b++ = c;
    }
    return n - cnt - 1;
}

BufFileWrite (f, b, n)
    BufFilePtr	f;
    char	*b;
    int		n;
{
    int	    c, cnt;
    cnt = n;
    while (cnt--) {
	if (BufFilePut (*b++, f) == BUFFILEEOF)
	    return BUFFILEEOF;
    }
    return n;
}

int
BufFileFlush (f)
    BufFilePtr	f;
{
    if (f->bufp != f->buffer)
	(*f->io) (BUFFILEEOF, f);
}

int
BufFileClose (f, doClose)
    BufFilePtr	f;
{
    (void) (*f->close) (f, doClose);
    xfree (f);
}

int
BufFileFree (f)
    BufFilePtr	f;
{
    xfree (f);
}
