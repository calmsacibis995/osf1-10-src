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
 * $XConsortium: showrgb.c,v 1.8 91/06/30 16:39:03 rws Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
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
 * Author:  Jim Fulton, MIT X Consortium
 */

#ifdef NDBM
#include <ndbm.h>
#else
#ifdef SVR4
#include <rpcsvc/dbm.h>
#else
#include <dbm.h>
#endif
#define dbm_open(name,flags,mode) (!dbminit(name))
#define dbm_firstkey(db) (firstkey())
#define dbm_fetch(db,key) (fetch(key))
#define dbm_close(db) dbmclose()
#endif

#undef NULL
#include <stdio.h>
#include <X11/Xos.h>
#include "rgb.h"			/* off in server/include/ */
#include "site.h"
#include <X11/Xfuncs.h>

char *ProgramName;

main (argc, argv)
    int argc;
    char *argv[];
{
    char *dbname = RGB_DB;

    ProgramName = argv[0];
    if (argc == 2)
	dbname = argv[1];

    dumprgb (dbname);
    exit (0);
}

dumprgb (filename)
    char *filename;
{
#ifdef NDBM
    DBM *rgb_dbm;
#else
    int rgb_dbm;
#endif
    datum key;

    rgb_dbm = dbm_open (filename, O_RDONLY, 0);
    if (!rgb_dbm) {
	fprintf (stderr, "%s:  unable to open rgb database \"%s\"\n",
		 ProgramName, filename);
	exit (1);
    }

#ifndef NDBM
#define dbm_nextkey(db) (nextkey(key))	/* need variable called key */
#endif

    for (key = dbm_firstkey(rgb_dbm); key.dptr != NULL;
	 key = dbm_nextkey(rgb_dbm)) {
	datum value;

	value = dbm_fetch(rgb_dbm, key);
	if (value.dptr) {
	    RGB rgb;
	    unsigned short r, g, b;
	    bcopy (value.dptr, (char *)&rgb, sizeof rgb);
#define N(x) (((x) >> 8) & 0xff)
	    r = N(rgb.red);
	    g = N(rgb.green);
	    b = N(rgb.blue);
#undef N
	    printf ("%3u %3u %3u\t\t", r, g, b);
	    fwrite (key.dptr, 1, key.dsize, stdout);
	    putchar ('\n');
	} else {
	    fprintf (stderr, "%s:  no value found for key \"", ProgramName);
	    fwrite (key.dptr, 1, key.dsize, stderr);
	    fprintf (stderr, "\"\n");
	}
    }

    dbm_close (rgb_dbm);
}
