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
/* $XConsortium: fstobdf.c,v 1.4 92/11/18 21:31:25 gildea Exp $ */
/*
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * M.I.T. not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND M.I.T. DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES,
 * DIGITAL OR M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#include	<stdio.h>
#include	"FSlib.h"

extern Bool EmitHeader();
extern Bool EmitProperties();
extern Bool EmitCharacters();
extern Bool EmitTrailer();

static void
usage(progName)
    char       *progName;
{
    fprintf(stderr, "Usage: %s [-s <font server>] -fn <font name>\n",
	    progName);
    exit(0);
}

main(argc, argv)
    int         argc;
    char      **argv;
{
    FSServer   *fontServer;
    Font        fontID,
                dummy;
    FSBitmapFormat bitmapFormat;
    FSXFontInfoHeader fontHeader;
    FSPropInfo  propInfo;
    FSPropOffset *propOffsets;
    unsigned char *propData;

    FILE       *outFile;
    char       *fontName;
    char       *serverName;
    int         i;

    fontName = NULL;
    serverName = NULL;
    outFile = stdout;

    for (i = 1; i < argc; i++) {
	if (!strncmp(argv[i], "-s", 2)) {
	    if (argv[++i])
		serverName = argv[i];
	    else
		usage(argv[0]);
	} else if (!strncmp(argv[i], "-fn", 3)) {
	    if (argv[++i])
		fontName = argv[i];
	    else
		usage(argv[0]);
	}
    }

    if (fontName == NULL)
	usage(argv[0]);

    fontServer = FSOpenServer(serverName);
    if (!fontServer) {
	fprintf(stderr, "can't open font server \"%s\"\n",
		FSServerName(serverName));
	exit(0);
    }
    bitmapFormat = 0;
    fontID = FSOpenBitmapFont(fontServer, bitmapFormat, (FSBitmapFormatMask) 0,
			      fontName, &dummy);
    if (!fontID) {
	printf("can't open font \"%s\"\n", fontName);
	exit(0);
    }
    FSQueryXInfo(fontServer, fontID, &fontHeader, &propInfo, &propOffsets,
		 &propData);

    if (!EmitHeader(outFile, &fontHeader, &propInfo, propOffsets, propData))
	Fail(argv[0]);
    if (!EmitProperties(outFile, &fontHeader, &propInfo, propOffsets, propData))
	Fail(argv[0]);
    if (!EmitCharacters(outFile, fontServer, &fontHeader, fontID))
	Fail(argv[0]);
    fprintf(outFile, "ENDFONT\n");

    FSFree((char *) propOffsets);
    FSFree((char *) propData);
}

Fail(progName)
    char       *progName;
{
    fprintf(stderr, "%s: unable to dump font\n", progName);
    exit(1);
}
