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
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988, 1991 BY                      *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**                         ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
** THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**      PS Previewer widget -- DECwindows widget for viewing PostScript
**
**  AUTHOR:
**
**      Susan Angebranndt
**	Philip Karlton
**	Terry Weissman
**	Joel Gringorten
**      Burns Fisher
**
**  ABSTRACT:
**
**      Private Widget Include File
**
**  ENVIRONMENT:
**
**      User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**	name		change					date
**	----		------					----
**	dam		bring in fileops			3/28/91
** 	dam		add field for redisplay on errors	6/10/91
**
**
**--
**/

/*
 * Include file for internal routines of the Postscript Previewer Widget.
 */

#ifndef _psviewwgP_h
#define _psviewwgP_h


/***********************************************************************
 *
 * Useful constants.
 *
 ***********************************************************************/

#define MMPERINCH	25.4
#define BITS_PER_BYTE	8
#define MAXINT		2147483647


/***********************************************************************
 *
 * PSView Widget Private Data
 *
 ***********************************************************************/

#include "psstruct.h"

/* New fields for the PSView widget class record */
typedef struct {
    Boolean psInited;
} PSViewClassPart;

/* Full class record declaration */
typedef struct _PSViewClassRec {
    CoreClassPart	core_class;
    PSViewClassPart	PSView_class;
} PSViewClassRec;


typedef struct _PixmapDesc {
    Pixmap pixmap;
    int lastUse;
    int page;
    int offsetX,offsetY;
} PixmapDescRec, *PixmapDesc;

/*
 * Structure for describing pieces of text to send to context.
 */

typedef struct _PosInfoRec {
    long start;
    long length;
    long end;
    Drawable drawable;		/* Where to send this chunk to*/
    int offsetX,offsetY;	/* Offset between PS 0,0 and drawable 0,0*/
    void (*proc)();		/*
				 * Proc to call when this chunk has been
				 * sent and processed.
				 */
} PosInfoRec, *PosInfo;


/* New fields for the PSView widget record */
typedef struct {
    Pixel foreground;		/* Foreground color for display. */
    char *filename;		/* File being displayed. */
    int page;			/* Page being requested. */
    int curpage;		/* What page we're currently processing. */
    char *initstring;		/* String to pass to postscript on startup. */
    PSViewProc textProc;	/* Procedure to call with text */
				/* from postscript. */
    PSViewProc errorProc;	/* Procedure to call with errors */
				/* from postscript. */
    Boolean  NeedToDisplayCopyrightNotice;
    Boolean parseComments;	/* Whether to bother parsing for comments. */
    Boolean fakeTrays;		/* whether to use fake trays */
    Boolean bitmapWidths;	/* whether to use bitmap widths */
    Boolean windowDrawMode;	/* DPS draws into widget window, no pixmaps */
    Boolean newWindowDrawMode;	/* What to change windowDrawMode to be next
				   time we rebuild the device. */
    Boolean subsetPixmaps;	/* The backing store pixmap is smaller than
				   the actual window */
    int pixmapHeight;		/* Actual size of backing store pixmap*/
    int pixmapWidth;		/* ..may be less than window if can't get full
				     size*/
    int cacheSize;		/* How many pixmaps to keep.  >= 0. */
    int		pixmapCacheLRU;
    PixmapDesc	pixmapCache;
    int		curPD;		/* This is an index to the Pixmap Cache
				   Descriptor which is to be written into for
				   the next page.  It is always a COPY of the
				   one of the PDs in the cache.*/
    int orientation;		/* What orientation {0,90,180,270} we're at. */
    double xdpi;		/* Dots per inch in the X direction. */
    double ydpi;		/* Dots per inch in the Y direction. */

    Structure sinfo;		/* Info on structured comments, if any. */
    int numpages;		/* Number of pages in this file (NOPAGE if */
				/* unknown. */
    Boolean rebuildcontext;	/* MakeItTrue should rebuild the postscript */
				/* context first. */
    Boolean rebuilddevice;	/* MakeItTrue should rebuild pixmaps first */

    Boolean redopage;		/* TRUE means rebuild a page, even if we're */
				/* already displaying it. */
    int disabledepth;		/* If > 0, then don't do any redisplays. */

    Pixmap dpsPixmap;		/* pixmap dedicated to DPS context */
    Pixmap redisplayPixmap;	/* Pointer to Pixmap containing current page.
				   It will always point to either dpsPixmap or
				   one of the cache pixmaps, so it can be zeroed
				   or otherwise left to dangle at will.*/
    int dpsPixmapOffsetX;	/* The offsets of these pixmaps from the*/
    int dpsPixmapOffsetY;	/* actual window, in case the pixmaps can't*/
    int redisplayPixmapOffsetX;	/* be as big as the window*/
    int redisplayPixmapOffsetY;
    int visibleX,visibleY;	/* Visible area on window (upper left)*/
    int visibleWidth,visibleHeight;
				/* Visiable area on window...these are inferred
				   from the window offset and the height and
                                   width of the last exposure*/
    GC pixmapGC;		/* GC to use to create the pixmap. */
    GC copyGC;			/* GC to use to copy pixmap to window. */
    DPSContext context;		/* The current postscript context. */
    int contextid;		/* Unique number corresponding to above. */
    DPSContext secondcontext;	/* Context for turning on and off drawing */
    Boolean deactivatePending;	/* Drawing should be turned off, but has
				   not been yet. */
    Boolean drawingdeactivated;	/* Whether drawing is turned off. */
    FILE *file;			/* File pointer */
    double scale;
    void (*appErrorProc)() ;	/* application text callback */
    void (*AppStatusProc)() ;	/* application status callback */
    Opaque AppStatusData;	/* Application private callback data */
    Boolean waitingInput;	/* Context is idle but not frozen */
    Boolean frozen;		/* If in a showpage wait */
    long curSeekPos;		/* Where we've currently seeked in the file */
    PosInfo posInfo;		/* Array of positions to send */
    int numPos;			/* How many sets of positions to send */
    int maxPos;			/* How many sets of positions we've alloced
				   space for */
    Drawable ctxDrawable;	/* If there is a valid context, this is its
				   drawable*/
    int ctxOffsetX,ctxOffsetY;	/* Offsets between ctx and ctxDrawable*/
    int skipcount;		/* skip these many pages */
    Font copyrightFont;
    Boolean showpageisok;	/* FALSE iff we're processing the prologue of
				   a structured document */
    Boolean headerRequired;     /* If TRUE and no %!, abort processing */
    Boolean errorRedisplay;	/* TRUE if error has occurred on partial page
				   - this will allow redisplay on partial pages
				     with errors */

} PSViewPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _PSViewRec {
    CorePart	    core;
    PSViewPart      psview;
} PSViewRec;

typedef struct _PSViewClassRec *PSViewWidgetClass;
typedef struct _PSViewRec      *PSViewWidget;


/* Definitions for file operations. */

#define myopen(name, flags, mode)	open((name), (flags), (mode))
#define myread(file, buf, size)		read((file), (buf), (size))
#define myclose(file)			close(file)

#define myfopen(name, mode)		fopen((name), (mode))
#define myfseek(file, position, mode)	fseek((file), (position), (mode))
#define myfread(buf, unit, size, file)	fread((buf), (unit), (size), (file))
#define myfclose(file)			fclose(file)

/****************************************************************
 *
 * Internal support routines.
 *
 ****************************************************************/

/*
 * Initialize things.
 */

extern void _PSViewInitialize(); /**/

#ifdef NOTDEF

/*
 * Execute postscript.  If showpage is called, decrements skipcount and
 * continue.  If showpage is called and skipcount is zero, copy the data into
 * the given pixmap, and return TRUE.  If the postscript process yields
 * waiting on stdin, feed it data from the given positions in the given
 * stream.  If it still blocks on stdin, return FALSE; numskipped will contain
 * the actual number of pages skipped.  Note that it will use data from stream
 * positions in previous calls to _PSViewExecutePostscript before it will
 * use the stream positions indicated in this call.
 */


extern Boolean _PSViewExecutePostscript();	/* context, display, pixmap,
						   pixmapGC, start, end,
						   skipcount, numskipped,
						   doOutput */
/* ViewContext context; */
/* Display *display; */
/* Pixmap pixmap; */
/* GC pixmapGC; */
/* long start, end; */
/* int skipcount, *numskipped; */
/* void (*doOutput)(); */

#endif /* NOTDEF */

#endif /* _psviewwgP_h */
/* DON'T ADD STUFF AFTER THIS #endif */



