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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BR_COMMON_DEFS.H*/
/* *20   25-JAN-1993 15:22:42 RAMSHAW "QAR #34 - fix ""Assign channel errors"""*/
/* *19   30-OCT-1992 18:31:34 BALLENGER "Fully sepcify UID file so as not to conflict with apps-default file on ULTRIX."*/
/* *18   13-AUG-1992 15:11:15 GOSSELIN "updating with necessary A/OSF changes"*/
/* *17   12-AUG-1992 14:44:09 ROSE "Renamed toplevel library names, added ISO literals"*/
/* *16    5-AUG-1992 21:35:47 BALLENGER "Remove the !@%^&$ BOOKREADER_CC conditionals"*/
/* *15   27-JUL-1992 18:18:46 KARDON "Force build"*/
/* *14   27-JUL-1992 16:10:55 KARDON "Change class name"*/
/* *13   15-JUL-1992 11:09:43 FITZELL "fix mswindows contional so it's not in a comment "*/
/* *12   13-JUL-1992 15:15:32 FITZELL "mswindows conditionals for PC port"*/
/* *11   19-JUN-1992 20:13:34 BALLENGER "Cleanup for Alpha/OSF port"*/
/* *10   18-JUN-1992 22:34:43 BALLENGER "add integer and pointer typedefs for Alpha/OSF"*/
/* *9    16-JUN-1992 17:30:02 BALLENGER "Use octal character codes for 8 bit character literals"*/
/* *8     8-MAY-1992 14:05:42 KARDON "Add string literal for display logical"*/
/* *7     2-MAY-1992 10:24:37 GOSSELIN "going back to earlier gen until ISO support can be debugged"*/
/* *6     1-MAY-1992 18:47:54 ROSE "Removed extension (Ultrix suffix) from toplevel library definition"*/
/* *5    13-MAR-1992 15:10:35 KARDON "Conditional change of application class"*/
/* *4     3-MAR-1992 17:11:55 KARDON "UCXed"*/
/* *3    16-DEC-1991 19:09:15 BALLENGER "Window resources names are the same on both UTLRIX and VMS"*/
/* *2    13-NOV-1991 14:27:32 GOSSELIN "conditionalized MAX and MIN"*/
/* *1    16-SEP-1991 12:48:22 PARMENTER "common #defines"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BR_COMMON_DEFS.H*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_COMMON_DEFS.H*/
/* *7    17-MAY-1991 13:51:12 ACKERMAN "Removed boolean tests from BKR_FLUSH_EVENTS macro"*/
/* *6    25-APR-1991 11:18:31 ACKERMAN "Changed resource name Bookreader to BookreaderNavigation"*/
/* *5    18-FEB-1991 16:25:37 BALLENGER "Add wildcard to directory masks for file dialogs on non-VMS systems."*/
/* *4    18-FEB-1991 14:22:17 BALLENGER "IFT2 Fixes, portability fixes, and Hyperhelp fixes"*/
/* *3    25-JAN-1991 16:40:33 FITZELL "V3_EFT_24_JAN"*/
/* *2    12-DEC-1990 12:03:15 FITZELL "V3 IFT Update snapshot"*/
/* *1     8-NOV-1990 11:14:20 FITZELL "V3 IFT"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_COMMON_DEFS.H*/

/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1990  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use,	     **
**  duplication or dissemination of the software and	     **
**  media is authorized only pursuant to a valid written     **
**  license from Digital Equipment Corporation.	    	     **
**  	    	    	    	    	    	    	     **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	     **
**  disclosure by the U.S. Government is subject to 	     **
**  restrictions as set forth in Subparagraph (c)(1)(ii)     **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as  	     **
**  applicable.	    	    	    	    	    	     **
***************************************************************
*/


/*
**++
**  FACILITY:
**
**      Bookreader User Interface (BKR)
**
**  ABSTRACT:
**
**	Common header file needed by most modules.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     2-Nov-1989
**
**  MODIFICATION HISTORY:
**
**
**      V03-0003    DBL0002     David L Ballenger       16-Feb-1990
**                  Add '*" to directory masks for file open dialogs
**                  to allow wildcarding on non-VMS systems.
**
**      V03-0002    DBL0001     David L Ballenger       6-Feb-1990
**                  Include <X11/Intrinsic.h> to pickup externaldef
**                  and externalref definitions rather than redefining
**                  them here.
**
**	V03-0001    JAF0001	James A. Ferguson   	2-Nov-1989
**  	    	    Created from V2 common header file source.
**
**--
**/

#ifndef _BR_COMMON_DEFS_H
#define _BR_COMMON_DEFS_H


/* 
 * Common typedefs to handle differences in integer type sizes and
 * pointers  across platforms.
 */

#include <limits.h> /* Make sure this gets included. */
#ifndef MSWINDOWS
#include <Xm/Xm.h>  /* Make sure the Motif and Xt Intrinsics 
		     * definitions, especially prototypes,
		     * are defined for all maodules.
		     */
#include <Mrm/MrmAppl.h> /* To the same for Mrm defintions */
#endif

#ifndef LONG_BIT
#define LONG_BIT 32
#endif

/* Since we're dealing with binary data from files that can be moved
 * between systems we need to be explicit about the size of integer
 * data.
 */
#if LONG_BIT == 64
typedef char    BR_INT_8; 
typedef short   BR_INT_16;
typedef int     BR_INT_32;
typedef unsigned char  BR_UINT_8;
typedef unsigned short BR_UINT_16;
typedef unsigned int   BR_UINT_32;
#else
typedef char    BR_INT_8;
typedef short   BR_INT_16;
typedef long    BR_INT_32;
typedef unsigned char  BR_UINT_8;
typedef unsigned short BR_UINT_16;
typedef unsigned long  BR_UINT_32;
#endif 

/* Define a generic pointer type for those case where we use handles
 * for data that we don't own.
 */
#ifdef __STDC__
typedef void *BR_GENERIC_PTR;
#else
typedef char *BR_GENERIC_PTR;
#endif
typedef BR_GENERIC_PTR BR_HANDLE;

typedef unsigned    	    	    STATUS;


/*
 *  Application shell and class names, and filespec names
 */

# define BKR_SELECTION_WINDOW_NAME  "BookreaderNavigation"
# define BKR_TOPIC_WINDOW_NAME	    "BookreaderTopic"
# define BKR_LIBRARY_WINDOW_NAME    "BookreaderLibrary"

#ifdef VMS
# define BKR_DISPLAY_LOGICAL	    "DECW$DISPLAY"
#else
# define BKR_DISPLAY_LOGICAL	    "DISPLAY"
#endif

#ifdef VMS
# define BKR_APPLICATION_CLASS	    "DECW$BOOKREADER"
# define BKR_UID_FILE    	    "DECW$BOOKREADER.UID"
# define BKR_TOPLEVEL_LIBRARY	    "LIBRARY"
# define BKR_DECW_BOOKSHELF_LOG	    "DECW$BOOKSHELF"
# define BOOK_DIRMASK	    	    "*.DECW$BOOK"
# define SHELF_DIRMASK	    	    "*.DECW$BOOKSHELF"
# define HOME_DIRECTORY	    	    "[]"
#else
# define BKR_APPLICATION_CLASS	    "DXBookreader"
# define BKR_UID_FILE    	    "DXBookreader.uid"
# define BKR_TOPLEVEL_LIBRARY	    "library"
# define BKR_DECW_BOOKSHELF_LOG	    "DECW_BOOKSHELF"
# define BOOK_DIRMASK	    	    "*.decw_book"
# define SHELF_DIRMASK	    	    "*.decw_bookshelf"
# define HOME_DIRECTORY	    	    "."
#endif


/*
 *  Other definitions
 */

#define TITLE_BAR_HEIGHT	8	    /* winmgr title bar in millimeters */
#define FORMATTED_300_DPI 	300	    /* dpi (dots per inch) */
#define FORMATTED_400_DPI 	400	    /* dpi (dots per inch) */
#define MILLIMETERS_PER_INCH	25.4	    /* used to convert to dots/inch */

/* NODISPLAY is the error status for display routines that return handles 
 * (i.e. pointers) to display data that may be owned and displayed by other
 * components. The resulting handle should have all bits set regardless of 
 * the platform.
 */
#define NODISPLAY               ((BR_HANDLE)ULONG_MAX)



#define ON	    	    	TRUE
#define	OFF     	    	FALSE
#define	SELECT  	    	TRUE
#define	UNSELECT 	    	FALSE
#define OPEN	    	    	TRUE
#define CLOSE	    	    	FALSE

#define VERSION_V1		1
#define V1_TOPIC_ADJUST	    	-(bkr_monitor_resolution >> 2)  /* in pixels */

#define MAX_TITLE_BAR_CHARS 	256

#define SELECTION_UNIT_INCREMENT	20 
#define TOPIC_UNIT_INCREMENT	    	17  /*  One line+ of 14 pt NCB  */
#define BKR_TOPLEVEL_NODE   	    	0   /* Toplevel SVN node */

#ifndef NULLCHAR
# define NULLCHAR '\0'
#endif

#define ASCIZ	3  /* MUST be the same value as MrmRtypeChar8 in <Mrm/MrmPublic.h> */

#ifndef MIN
# define MIN(x,y)	((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
# define MAX(x,y)	((x) > (y) ? (x) : (y))
#endif

/*
 *  White space at top of Selection drawing window
 */

#define WINDOW_TOP_SPACE    7	/* in pixels */

#define	    NATIVE_BYTE_ORDER	    LSBFirst
#define	    NATIVE_BIT_ORDER	    LSBFirst

/*
 *  Literals passed to bkr_selection_scroll_to_entry
 */
#define POSITION_TOP	    	1
#define POSITION_MIDDLE	    	2
#define POSITION_BOTTOM	    	3

/*
 *  Literals passed to BriFileParse
 */                
#define DEFAULT_EXTENSION	1
#define ISO_EXTENSION		2
#define NFS_EXTENSION		3

#define SEARCHING		4
#define NOT_SEARCHING		5

/*
 *  CALCULATE_MAXIMUM_VALUE - calculates the maximum scroll bar value
 *
 *  	    NOTE:  screen_ht MUST include one line of overlap
 */

#define CALCULATE_MAXIMUM_VALUE( max_val, topic_ht, wind_ht )   \
  { 	    	    	    	    	    	    	    	\
    int	    screen_ht;  	    	    	    	    	\
    int	    num_screens;    	    	    	    	    	\
    screen_ht = (int) (wind_ht) - TOPIC_UNIT_INCREMENT;	    	\
    num_screens = ( (topic_ht) + screen_ht ) / screen_ht;	\
    (max_val) = num_screens * screen_ht;	    	    	\
  }


/*
 * SCALE_VALUE -- decide which SCALE_VALUEXXX to use based upon the book version
 */

#define SCALE_VALUE( value, book_ver ) 	    	\
    ( ( ( book_ver ) == VERSION_V1 )	    	\
    	    ? ( SCALE_VALUE300( ( value ) ) )	\
    	    : ( SCALE_VALUE400( ( value ) ) ) 	\
    )


/*
 * SCALE_VALUE300 -- Calculate a new value based on monitor resolution
 */

#define SCALE_VALUE300( value )	\
    ( ( bkr_monitor_resolution == 75 ) ? ( ( ( value ) + 2 ) >> 2 ) : 	    	\
    	( bkr_monitor_resolution == 100 ) ? ( ( ( value ) + 2 ) / 3 ) :	    	\
		( ( ( value ) * bkr_monitor_resolution ) / FORMATTED_300_DPI )  \
    )

/*
 * SCALE_VALUE400 -- Calculate a new value based on monitor resolution
 *
 */

#define SCALE_VALUE400( value )						    	\
    ( ( bkr_monitor_resolution == 75) ?					    	\
	( ( ( ( value ) + 4 ) >> 3 ) + ( ( ( value ) + 8 ) >> 4 ) ) :       	\
	    ( bkr_monitor_resolution == 100 ) ? ( ( ( value ) + 2 ) >> 2 ) :    \
		( ( ( value ) * bkr_monitor_resolution ) / FORMATTED_400_DPI )  \
    )


/*
 * SCALE_GRAPHIC_VALUE75 -- scale the graphic width or height from the current
 *			resolution to 75dpi coordinates.
*/

#define SCALE_GRAPHIC_VALUE75( value )	\
	( ( ( value ) * 75 ) / bkr_monitor_resolution )



#ifndef MSWINDOWS

/*
 * SetArg -- Performs a XtSetArg and hide the arglist array and the increment
 *	     of argcnt.
 */

#define SET_ARG( arg, new_value )    \
    { XtSetArg( arglist[argcnt], (arg), (new_value) ); argcnt++; }

/*
 * RAISE_WINDOW -- Raises a window to the top of the screen and de-iconifies
 *		       the window if need be.  
 *  	    	    NOTE: if the window manager running isn't Motif's
 *  	    	    	Mwm then assume its not ICCCM compliant
 */

#define RAISE_WINDOW( sh_id_ptr )	    	    	\
    {   	    	    	    	    	    	\
    	Window 	win = XtWindow( sh_id_ptr );	    	\
    	Display *dpy = XtDisplay( sh_id_ptr );	    	\
    	if ( win != 0 ) 	    	    	    	\
    	{   	    	    	    	    	    	\
    	    if ( ! XmIsMotifWMRunning( sh_id_ptr ) )	\
    	    {	    	    	    	    	    	\
	    	XWMHints    *wmhints;	    	    	\
    	    	wmhints = XGetWMHints( dpy, win );  	\
		if (wmhints != NULL )			\
		{					\
	    	   wmhints->flags |= StateHint;	    	\
	    	   wmhints->initial_state = NormalState;\
	    	   XSetWMHints( dpy, win, wmhints );   	\
		}					\
    	    	XFree( wmhints );   	    	    	\
    	    }	    	    	    	    	    	\
	    XMapRaised( dpy, win );   	    	    	\
    	}   	    	    	    	    	    	\
    }

/*
 *  ICONIFY_WINDOW - iconifies a window given its shell widget id.
 *  	    	     NOTE: if the window manager running isn't Motif's
 *  	    	    	Mwm then assume its not ICCCM compliant
 */

#define ICONIFY_WINDOW( sh_id_ptr ) 	    	    	\
    {   	    	    	    	    	    	\
    	Window 	    win = XtWindow( sh_id_ptr );	\
    	Display     *dpy = XtDisplay( sh_id_ptr );	\
    	int 	    screen = XDefaultScreen( dpy );	\
    	if ( win != 0 ) 	    	    	    	\
    	{   	    	    	    	    	    	\
    	    if ( ! XmIsMotifWMRunning( sh_id_ptr ) )	\
    	    {	    	    	    	    	    	\
	    	XWMHints    *wmhints;	    	    	\
    	    	wmhints = XGetWMHints( dpy, win );  	\
	    	wmhints->flags |= StateHint;	    	\
	    	wmhints->initial_state = IconicState;	\
	    	XSetWMHints( dpy, win, wmhints );   	\
    	    	XFree( wmhints );   	    	    	\
    	    }	    	    	    	    	    	\
    	    XIconifyWindow( dpy, win, screen );	    	\
    	}   	    	    	    	    	    	\
    }

/*
 * COMPOUND_STRING_FREE -- free's the data for a compound string and zeros the
 *			 the pointer
 */

#define COMPOUND_STRING_FREE( ptr )	\
    {	    	    	    	    	\
	XmStringFree( (ptr) );	    	\
	(ptr) = NULL;	    	    	\
    }


/*
 * CONVERT_MM_TO_PIXELS -- convert from millimeters to pixels
 */

#define CONVERT_MM_TO_PIXELS( millimeters )    \
	(  ( ( millimeters ) * bkr_monitor_resolution ) / MILLIMETERS_PER_INCH )



/*
 *  Initialize an XColor data structure
 */

#define  SET_XCOLOR( c,p,r,g,b,f )  \
    { 	    	    	    	    \
    	c.pixel = p; 	    	    \
    	c.red = r;  	    	    \
    	c.green = g;	    	    \
    	c.blue = b; 	    	    \
    	c.flags = f; 	    	    \
    }



/*
 *  Makes a local copy of the contents of an XExposeEvent structure.
 */

#define COPY_XEXPOSE_EVENT( dest, src )	    \
    {	    	    	    	    	    \
    	dest.type   	= src->type; 	    \
	dest.serial 	= src->serial;	    \
	dest.send_event	= src->send_event;  \
	dest.display	= src->display;	    \
	dest.window 	= src->window;	    \
	dest.x	    	= src->x;    	    \
    	dest.y	    	= src->y;    	    \
	dest.width  	= src->width;	    \
    	dest.height 	= src->height;	    \
	dest.count  	= src->count;	    \
    }



/*
 *  DRM macro to update "tag" value for callback data
 */

#define BKR_UPDATE_TAG( callback_value )    	    	 \
 {  	    	    	    	    	    	    	 \
    tag_reglist[0].value = (caddr_t) ( callback_value ); \
    (void)MrmRegisterNames( tag_reglist, 1 );    	 \
 }


/*
 *  Atom macros for ICCCM
 */

#define WM_PROTOCOLS_ATOM       \
    	    XmInternAtom( bkr_display, "WM_PROTOCOLS", False )
#define WM_DELETE_WINDOW_ATOM    \
    	    XmInternAtom( bkr_display, "WM_DELETE_WINDOW", False )


/*
 *  BKR_FLUSH_EVENTS - flushes the event queue and removes any
 *  	    	       button or key, presses or releases.
 *  	    This macro should be used at the end of any callback
 *  	    routine which could take a long time to process.
 */

#define BKR_FLUSH_EVENTS    	    	    	    	    	    	    	\
 {  	    	    	    	    	    	    	    	    	    	\
    externalref Display	*bkr_display;	    	    	    	    	    	\
    XEvent  	    event;   	    	    	    	    	    	    	\
    XtInputMask     eventtype;  	    	    	    	    	    	\
    XtAppContext    AppContext = XtDisplayToApplicationContext( bkr_display );	\
    while ( ( eventtype = XtAppPending( AppContext ) ) != 0 )	    	    	\
    {	    	    	    	    	    	    	    	    	    	\
    	if ( eventtype == XtIMTimer )	    	    	    	    	    	\
    	    XtAppProcessEvent( AppContext, XtIMAll );	    	    	    	\
    	else	    	    	    	    	    	    	    	    	\
    	{	    	    	    	    	    	    	    	    	\
    	    XtAppNextEvent( AppContext, &event );   	    	    	    	\
    	    XtDispatchEvent( &event );      	    	    	    		\
    	};  	    	    	    	    	    	    	    	    	\
    };  	    	    	    	    	    	    	    	    	\
 }


#endif

 /*
  *
  * these tolower functions work with all characters in the isoLatin1 
  * character set.  NOTE that you won't be able to read the macro properly,
  * until you set your terminal to isolatin1 mode.  
  */
#define _br_tolower(c) 							\
	 (((c) >= 'a' && (c) <= 'z') ? (c) : (_isolatin1_tolower(c)))

#define _isolatin1_tolower(c)						\
	((((c) >= 'A' && (c) <= 'Z') || 				\
	  ((c) >= (unsigned char) '\0300' && (c) <= (unsigned char) '\0326') || \
	  ((c) >= (unsigned char) '\0330' && (c) <= (unsigned char) '\0336')) ? \
	   (c) | 0x20 : (c))

 /*
  * BKR_SEARCH_CONTEXT_HASH determines how frequently to reallocate the 
  * results list structures.
  *
  */
#define 			BKR_SEARCH_CONTEXT_HASH	50

#endif /* _BR_COMMON_DEFS_H */

/* DONT ADD STUFF AFTER THIS #endif */

