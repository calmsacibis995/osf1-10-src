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
 * @(#)$RCSfile: psviewwg.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/03 00:03:49 $
 */
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988,1991 BY                       *
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
** THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE   *
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
**
**  ABSTRACT:
**
**      This module contains the user interface and mainline.
**
**  ENVIRONMENT:
**
**      User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**--
**/
/*
 * Include file for users of the Postscript Previewer Widget.
 */

/* 
 *  */

#define NOPAGE		-1


#define PsNfileName	"fileName"
#define PsNpage	"page"
#define PsNinitString	"initString"
#define PsNerrorProc	"errorProc"
#define PsNtextProc	"textProc"
#define PsNparseComments	"parseComments"
#define PsNpixmapCacheSize	"pixmapCacheSize"
#define PsNorientation	"orientation"

#define PsCFileName	"FileName"
#define PsCPage	"Page"
#define PsCInitString	"InitString"
#define PsCErrorProc	"ErrorProc"
#define PsCTextProc	"TextProc"
#define PsCParseComments	"ParseComments"
#define PsCPixmapCacheSize	"PixmapCacheSize"
#define PsCOrientation	"Orientation"


enum FileState {PSFile, NoPercentBang, NoFile};

typedef void (*PSViewProc)();




/*
 * Display the given page of the given file in the given widget.  page ==
 * NOPAGE means don't start displaying yet.  If name is NULL or a
 * zero-length string, then this just closes any file that was open but
 * doesn't open a new one.
 *
 * The initstring will be passed to the postscript interpreter immediately,
 * before any of the file is parsed.  Any calls to showpage or marking of the
 * page performed by the initstring will be ignored; its purpose is only to
 * define the initial state of the interpreter.  For example, it may define
 * fonts so that Document files can be interpreted.
 */

PROTO (void PSViewSetFile, (Widget, char *, int, char *));		/* widget, name, page, initstring */

/*
 * Abort the viewing in progress.
 */

PROTO (void PSViewAbort, (Widget));					/* widget */

/*
 * Display a specific page.
 */

PROTO (void PSViewShowPage, (Widget, int));				/* widget, page */


/*
 * Return what file and page are being shown.  If the returned name is NULL,
 * then we're not displaying any file.
 */

PROTO (void PSViewGetPageAndFile, (Widget, char **, int *));		/* widget, file, page */


/*
 * Determine how many pages are in the given file.  If unknown, returns NOPAGE.
 */

PROTO (int PSViewGetNumPages, (Widget));				/* widget */


/*
 * Sets the procedures to be called when text is received or postscript errors
 * occur.
 */

PROTO (void PSViewSetProcs, (Widget, PSViewProc, PSViewProc));		/* widget, textproc, errorproc */


/*
 * Gets the procedures to be called when text is received or postscript errors
 * occur.
 */

PROTO (void PSViewGetProcs, (Widget, PSViewProc *, PSViewProc *));	/* widget, textproc, errorproc */


/*
 * Sets the orientation being displayed.  This will cause the current page
 * to be reparsed and repainted.
 */

PROTO (void PSViewSetOrientation, (Widget, int));			/* widget, orientation (one of 0, 90, 180, 270) */

/*
 * Sets the resolution being shown.  This will cause the current page to be
 * reparsed and repainted.
 */

PROTO (void PSViewSetResolution, (Widget, double, double));		/* widget, xdpi, ydpi */

/* Widget widget; */
/* double xdpi; */	/* Pixels per inch in the horizontal direction. */
/* double ydpi; */	/* Pixels per inch in the vertical direction. */


/*
 * Gets the resolution being shown.
 */

PROTO (void PSViewGetResolution, (Widget, double *, double *));		/* widget, xdpi, ydpi */
/* Widget widget; */
/* double *xdpi; */	/* Pixels per inch in the horizontal direction. */
/* double *ydpi; */	/* Pixels per inch in the vertical direction. */



/*
 * Sets whether structured comments should be used.
 */

PROTO (void PSViewSetParseComments, (Widget, Boolean));			/* widget, value */
/* Widget widget; */
/* Boolean value; */



/*
 * Gets whether structured comments are being used.
 */

PROTO (Boolean PSViewGetParseComments, (Widget));			/* widget */
/* Widget widget; */


/*
 * Sets whether a %! header is required.  A status callback will always be
 * made if there is no %! header present.  However, if it is required and it
 * is not the first two characters of the file, processing will abort after
 * the status callback.  It is legal to set the header to be not
 * required from the status callback.
 */

PROTO (void PSViewSetHeaderRequired, (Widget, Boolean));		/* widget, value */
/* Widget widget; */
/* Boolean value; */

/*
 * Gets whether the header is required.
 */

PROTO (Boolean PSViewGetHeaderRequired, (Widget));			/* widget */
/* Widget widget; */



/*
 * Show the next page.
 */

PROTO (void PSViewNextPage, (Widget));				/* widget */
/* Widget widget; */



/*
 * Show the previous page.  Returns TRUE if there is one.
 */

PROTO (void PSViewPreviousPage, (Widget));				/* widget */
/* Widget widget; */



/*
 * Disable redisplaying and reparsing.  The widget will do no more parsing
 * until a matching call to PSViewEnableRedisplay is called.
 */

PROTO (void PSViewDisableRedisplay, (Widget));				/* widget */
/* Widget widget; */


PROTO (void PSViewProcessEvent, ());					/* event */

PROTO (void PSViewSetStatusProc, (Widget, void (*proc)(), Opaque));

    /* Widget, void (*StatusProc)(), Opaque */
    /* void StatusProc (StatusCode, Opaque, pagenum) */

enum StatusCode {starting, finished, badfile, ok, noPage,
		 badComments, noHeader};

/*
 * Enable redisplaying and reparsing.  Undoes one call to
 * PSViewDisableRedisplay.  Note that these calls nest; if
 * PSViewDisableRedisplay is called twice, then it will take two calls to
 * PSViewEnableRedisplay before the widget starts parsing and displaying.
 */

PROTO (void PSViewEnableRedisplay, (Widget));				/* widget */
/* Widget widget; */

/*
 * Return document info like Creator, For, etc from the PostScript
 * structure comments.  Function value is the size of the array.
 * NULL means there was no structure info, or it is
 * otherwise unavailable. infoPtr contains the actual data from
 * the document.  Any string in infoPtr may be NULL if that information
 * was not supplied.  Note that it is quite possible for a non-null
 * value to be returned, but for there to be no non-NULL strings in the
 * array.  The value is more for use when additional document information
 * items are added than to tell how many docinfos a particular document has.
 */

PROTO (int PSViewGetDocumentInfo, (Widget, char **[]));
/*
    Widget w;
    char *infoPtr[];
*/

/*
enum DocInfoIndex {PSVWGCreator,PSVWGCreationDate,PSVWGTitle,PSVWGFor};
*/

#define PSVWGCreator 		0
#define PSVWGCreationDate 	1
#define PSVWGTitle		2
#define PSVWGFor		3


extern Boolean PSViewIsStructured ();	/* widget */
extern void PSViewSetScale();		/* widget, scale */
extern void PSViewFakeTrays();		/* widget, Boolean */
extern void PSViewBitmapWidths(); 	/* widget, Boolean */
extern void PSViewSetWindowDrawMode();	/* widget, Boolean */
extern void PSErrorAbort();		/* widget */
