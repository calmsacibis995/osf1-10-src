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
 * $XConsortium: AsciiTextP.h,v 1.15 89/07/17 18:09:37 kit Exp $ 
 */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/***********************************************************************
 *
 * AsciiText Widget
 *
 ***********************************************************************/

/*
 * AsciiText.c - Private header file for AsciiText Widget.
 *
 * This Widget is intended to be used as a simple front end to the 
 * text widget with an ascii source and ascii sink attached to it.
 *
 * Date:    June 29, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium 
 *          kit@expo.lcs.mit.edu
 */

#ifndef _AsciiTextP_h
#define _AsciiTextP_h

#include <X11/Xaw/TextP.h>
#include <X11/Xaw/AsciiSrc.h> /* no need to get private header. */
#include <X11/Xaw/AsciiText.h>

typedef struct {int empty;} AsciiClassPart;

typedef struct _AsciiTextClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    TextClassPart	text_class;
    AsciiClassPart	ascii_class;
} AsciiTextClassRec;

extern AsciiTextClassRec asciiTextClassRec;

typedef struct { char foo; /* keep compiler happy. */ } AsciiPart;

typedef struct _AsciiRec {
    CorePart		core;
    SimplePart		simple;
    TextPart		text;
    AsciiPart           ascii;
} AsciiRec;

/************************************************************
 *
 * Ascii String Emulation widget.
 *
 ************************************************************/ 

#ifdef ASCII_STRING

typedef struct {int empty;} AsciiStringClassPart;

typedef struct _AsciiStringClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    TextClassPart	text_class;
    AsciiClassPart	ascii_class;
    AsciiStringClassPart string_class;
} AsciiStringClassRec;

extern AsciiStringClassRec asciiStringClassRec;

typedef struct { char foo; /* keep compiler happy. */ } AsciiStringPart;

typedef struct _AsciiStringRec {
    CorePart		core;
    SimplePart		simple;
    TextPart		text;
    AsciiPart           ascii;
    AsciiStringPart     ascii_str;
} AsciiStringRec;

#endif /* ASCII_STRING */

#ifdef ASCII_DISK

/************************************************************
 *
 * Ascii Disk Emulation widget.
 *
 ************************************************************/ 

typedef struct {int empty;} AsciiDiskClassPart;

typedef struct _AsciiDiskClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    TextClassPart	text_class;
    AsciiClassPart	ascii_class;
    AsciiDiskClassPart	disk_class;
} AsciiDiskClassRec;

extern AsciiDiskClassRec asciiDiskClassRec;

typedef struct { char foo; /* keep compiler happy. */ } AsciiDiskPart;

typedef struct _AsciiDiskRec {
    CorePart		core;
    SimplePart		simple;
    TextPart		text;
    AsciiPart           ascii;
    AsciiDiskPart       ascii_disk;
} AsciiDiskRec;
#endif /* ASCII_DISK */

#endif /* _AsciiTextP_h */
