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
 * xmodmap - program for loading keymap definitions into server
 *
 * $XConsortium: pf.c,v 1.4 91/07/17 22:26:40 rws Exp $
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
 * Author:  Jim Fulton, MIT X Consortium
 */

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <ctype.h>
#include "xmodmap.h"

#define NOTINFILEFILENAME "commandline"
char *inputFilename = NOTINFILEFILENAME;
int lineno = 0;

void process_file (filename)
    char *filename;			/* NULL means use stdin */
{
    FILE *fp;
    char buffer[BUFSIZ];

    /* open the file, eventually we'll want to pipe through cpp */

    if (!filename) {
	fp = stdin;
	inputFilename = "stdin"; 
    } else {
	fp = fopen (filename, "r");
	if (!fp) {
	    fprintf (stderr, "%s:  unable to open file '%s' for reading\n",
		     ProgramName, filename);
	    parse_errors++;
	    return;
	}
	inputFilename = filename;
    }


    /* read the input and filter */

    if (verbose) {
	printf ("! %s:\n", inputFilename);
    }

    for (lineno = 0; ; lineno++) {
	buffer[0] = '\0';
	if (fgets (buffer, BUFSIZ, fp) == NULL)
	  break;

	process_line (buffer);
    }

    inputFilename = NOTINFILEFILENAME;
    lineno = 0;
    (void) fclose (fp);
}


void process_line (buffer)
    char *buffer;
{
    int len;
    int i;
    char *cp;

    len = strlen (buffer);

    for (i = 0; i < len; i++) {		/* look for blank lines */
	register char c = buffer[i];
	if (!(isspace(c) || c == '\n')) break;
    }
    if (i == len) return;

    cp = &buffer[i];

    if (*cp == '!') return;		/* look for comments */
    len -= (cp - buffer);		/* adjust len by how much we skipped */

					/* pipe through cpp */

					/* strip trailing space */
    for (i = len-1; i >= 0; i--) {
	register char c = cp[i];
	if (!(isspace(c) || c == '\n')) break;
    }
    if (i >= 0) cp[len = (i+1)] = '\0';  /* nul terminate */

    if (verbose) {
	printf ("! %d:  %s\n", lineno, cp);
    }

    /* handle input */
    handle_line (cp, len);
}
