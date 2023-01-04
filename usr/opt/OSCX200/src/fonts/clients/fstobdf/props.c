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
/* $XConsortium: props.c,v 1.3 92/11/18 21:31:27 gildea Exp $ */
/*
 * Copyright 1990, 1991 Network Computing Devices;
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

static char *
AddQuotes(string, length)
    unsigned char *string;
    int         length;
{
    static unsigned char new[256] = "\"";
    unsigned char *cp;
    unsigned char *end;

    end = string + length;
    for (cp = &new[1]; string < end; cp++, string++) {
	*cp = *string;
	if (*cp == '"')
	    *++cp = '"';
    }
    *cp++ = '"';
    *cp = '\0';
    return (char *)(new);
}

Bool
EmitProperties(outFile, fontHeader, propInfo, propOffsets, propData)
    FILE       *outFile;
    FSXFontInfoHeader *fontHeader;
    FSPropInfo *propInfo;
    FSPropOffset *propOffsets;
    unsigned char *propData;
{
    int         nProperties;
    FSPropOffset *property;
    Bool        needDefaultChar;
    Bool        needFontAscent;
    Bool        needFontDescent;

    needDefaultChar = True;
    needFontAscent = True;
    needFontDescent = True;

    nProperties = propInfo->num_offsets;
    for (property = &propOffsets[0]; nProperties--; property++) {
	unsigned char *name;
	int         length;

	name = propData + property->name.position;
	length = property->name.length;

	if ((length == 12) && (!strncmp(name, "DEFAULT_CHAR", 12)))
	    needDefaultChar = False;
	else if ((length == 11) && (!strncmp(name, "FONT_ASCENT", 11)))
	    needFontAscent = False;
	else if ((length == 12) && (!strncmp(name, "FONT_DESCENT", 12)))
	    needFontDescent = False;
    }

    nProperties = propInfo->num_offsets;
    fprintf(outFile, "STARTPROPERTIES %d\n", nProperties +
	    (needDefaultChar ? 1 : 0) + (needFontAscent ? 1 : 0) +
	    (needFontDescent ? 1 : 0));

    for (property = &propOffsets[0]; nProperties--; property++) {
	unsigned long value;

	/* Don't emit properties that are computed by bdftosnf */

	fwrite(propData + property->name.position, 1, property->name.length,
	       outFile);
	fputc(' ', outFile);

	value = property->value.position;
	switch (property->type) {
	case PropTypeString:
	    fprintf(outFile, "%s\n", AddQuotes(propData + value,
					       property->value.length));
	    break;
	case PropTypeUnsigned:
	    fprintf(outFile, "%lu\n", value);
	    break;
	case PropTypeSigned:
	    fprintf(outFile, "%ld\n", value);
	    break;
	default:
	    fprintf(stderr, "unknown property type\n");
	    return (False);
	}
    }
    if (needDefaultChar) {
	fprintf(outFile, "DEFAULT_CHAR %lu\n",
		(long) (fontHeader->default_char.high << 8)
		     | (fontHeader->default_char.low));
    }
    if (needFontAscent)
	fprintf(outFile, "FONT_ASCENT %d\n", fontHeader->font_ascent);
    if (needFontDescent)
	fprintf(outFile, "FONT_DESCENT %d\n", fontHeader->font_descent);
    fprintf(outFile, "ENDPROPERTIES\n");
    return (True);
}
