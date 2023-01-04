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

	      Copyright (c) Digital Equipment Corporation, 1990  
	      All Rights Reserved.  Unpublished rights reserved
	      under the copyright laws of the United States.
	      
	      The software contained on this media is proprietary
	      to and embodies the confidential technology of 
	      Digital Equipment Corporation.  Possession, use,
	      duplication or dissemination of the software and
	      media is authorized only pursuant to a valid written
	      license from Digital Equipment Corporation.

	      RESTRICTED RIGHTS LEGEND   Use, duplication, or 
	      disclosure by the U.S. Government is subject to
	      restrictions as set forth in Subparagraph (c)(1)(ii)
	      of DFARS 252.227-7013, or in FAR 52.227-19, as
	      applicable.

*****************************************************************************
**++
**  FACILITY:
**
**	DECwindows Toolkit
**
**  ABSTRACT:
**
**	International Text Widget String Source Data Handling Structures
**
**
**  MODIFICATION HISTORY:
**
**	12 APR 1990  Begin rework of old CSText widget code.
**
**	8 Jan 1993  Restructured to be compliant with coding practices
**
**--
**/

#ifndef _DXmCSTextSrc_h
#define _DXmCSTextSrc_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <X11/DECwI18n.h>

#define FMT_I_STRING	((DXmCSTextFormat) XA_I_STRING)	/* International
                                                           string text. */
#define SEGMENT_EXTRA_ALLOCATION 20 
#define NODELTA		MAXINT

#define DXmLanguageNotSpecified 0
#define DXmRendMaskNone 0
#define MAX_SEGMENT_SIZE 50


#define Is16BIT(set) ( set != 0 && \
		      (!strcmp( set, "JISX0201.1976-0") ||  \
		       !strcmp( set, "JISX0208.1983-1") ||  \
		       !strcmp( set, "GB2312.1980-1")) )       /*others?+++++*/

#define DXmCSTextScanType XmTextScanType

#define DXmstPositions  XmSELECT_POSITION
#define DXmstWhiteSpace XmSELECT_WHITESPACE
#define DXmstWordBreak  XmSELECT_WORD
#define DXmstEOL        XmSELECT_LINE
#define DXmstAll	XmSELECT_ALL
#define DXmstParagraph  XmSELECT_PARAGRAPH


/****************************************************************
 *
 * Data structures for the source 
 *
 ****************************************************************/


/* BOGUS this should move to cstextoutp.h.....
 * information the output module keeps in the segment and line 
 * source structures
 */

/* Rendition Manager */

typedef enum {
  RmHIGHLIGHT_NORMAL = XmHIGHLIGHT_NORMAL,
  RmHIGHLIGHT_SELECTED = XmHIGHLIGHT_SELECTED,
  RmHIGHLIGHT_SECONDARY_SELECTED = XmHIGHLIGHT_SECONDARY_SELECTED,
  RmSEE_DETAIL = 99  /* BOGUS - maybe none of above enums doesn't have a 
		       * value 99 */
} RmHighlightMode;


/*
 * the source segment DXmCSTextOutputSegment keeps a pointer to an 
 * array of these structs, one per char, tells us all the stuff we
 * need to know.
 */
typedef struct
{
    Boolean         need_redisplay;   /* this char is dirty */

    XmHighlightMode draw_mode;        /* how this char is to be drawn */
}
    DXmCSTextOutputCharRec, *DXmCSTextOutputChar;

/*
 * the source segment keeps a pointer to one of these structs,
 * tells us all the stuff we need to know about this segment
 */
typedef struct
{
    Boolean             need_redisplay,   /* some chars of segment are dirty */
                        need_recompute;   /* something is know to be wrong */

    DXmCSTextOutputChar  per_char;         /* per-char data array */
    int                 offset;           /* where to start in source seg */
    int                 char_count;       /* number of chars in output seg */

    short               pixel_width,      /* size of this segment */
                        pixel_height;

    Position            x, y;             /* location of this segment, relative
					   * to the line x, y */

    I18nXlibBuffers	xlib_buff;	  /* pointer to the xlib_buffer for
					     display */

    short		xlib_buff_count;  /* number of xlib buffer */

    XmFontListEntry     *font_entry;      /* pointer to the font entry for 
                                             displaying the single segment */
    Dimension		tab_width;	  /* tab width in pixel */
    Boolean		new_line;         /* indicate if segment starts at */
					  /* beginning of line		   */
    RmHighlightMode	draw_mode;	  /* highlight mode */
}
    DXmCSTextOutputSegmentRec, *DXmCSTextOutputSegment;

/*
 * the source segment keeps a pointer to one of these structs for
 * us.  keep all the line level data here
 */
typedef struct
{
    Boolean             need_redisplay,   /* segment in the line is dirty */
                        need_recompute;   /* something is know to be wrong */

    short               pixel_width,      /* size of this line */
                        pixel_height;

    Position            x, y;             /* location of this line in window */
    int			char_count;	  /* num of char in this out_line */
    RmHighlightMode	draw_mode;	  /* highlight mode */
}
    DXmCSTextOutputLineRec, *DXmCSTextOutputLine;




typedef Atom DXmCSTextFormat;

/* this is obsolete
 *typedef enum {
 *   DXmstPositions,
 *   DXmstWhiteSpace,
 *   DXmstWordBreak,
 *   DXmstEOL,
 *   DXmstAll,
 *   DXmstPhysPositions,
 *   DXmstPhysWhiteSpace,
 *   DXmstPhysWordBreak,
 *   DXmstPhysEOL,
 *   DXmstPhysStartDir,
 *   DXmstPhysEndDir 
 *} DXmCSTextScanType;
*/
typedef enum { DXmsdLeft, DXmsdRight } DXmCSTextScanDirection;
    
typedef struct _TextSegment {
    struct _TextSegment	*next;	    /* next text segment		     */
    struct _TextSegment	*prev;	    /* prev text segment		     */
    char		*text;	    /* Pointer to array of 4 byte characters */
    int			length;     /* segment length in characters	     */
    int			used_allocation;/* segment length in bytes 	     */
    int			allocation; /* segment length in characters	     */
    XmStringCharSet	charset;    /* char set name string		     */
    XmStringDirection	direction;  /* text path direction		     */
    short		locale;	    /* locale information		     */
    short		nest_level; /* directionality nesting level	     */

    /*
     * these are output data private
     */
    short                 num_output_segments; /* num output segs for this */
                                               /* source segment */
    DXmCSTextOutputSegment output_segment;      /* array of  per seg data */

  } TextSegmentRec, *TextSegment;

typedef struct _TextLine {
    struct _TextLine	*next;	    /* next text line			    */
    struct _TextLine	*prev;	    /* prev text line			    */
    struct _TextSegment	*segments;  /* Pointer to 1st text segment in line. */
    DXmCSTextPosition    length;	    /* length of line in characters	    */ 

    DXmCSTextOutputLine  output_line;  /* output's line data                 */

    int			 num_output_lines;/* num of output lines in that text */
					  /* line			      */

  } TextLineRec;

/* data structure for holding source location information
*/
typedef struct _TextLocation {
    DXmCSTextPosition position;    /* linear logical position in source  */
    TextLine         line;        /* source line of position            */
    DXmCSTextPosition line_offset; /* text offset in line of position    */
    TextSegment      segment;     /* source segment of position         */
    DXmCSTextPosition offset;      /* text offset in segment of position */
    Boolean          end_of_line; /* is position at the end of a line?  */
  } TextLocationRec, *TextLocation;

typedef struct _DXmCSTextCvtContextRec *DXmCSTextCvtContext;
#ifdef _NO_PROTO
typedef void (*DXmSegmentProc)();
#else
typedef void (*DXmSegmentProc)(DXmCSTextCvtContext);
#endif
 
/*
 * data struct shared by CSText source and DXmCvtCStoCSText and DXmCvtCSTextToCS
 * routines
 */

typedef struct _DXmCSTextCvtContextRec
{
    /* 
     * fields used for communication between the caller and callee,
     *  **** DON'T TOUCH THIS DATA STRUCTURE *** cause XmString.c must match
     */

    DXmCSTextStatus    status;	    /* return status from converter          */

    DXmSegmentProc     line_cb;      /* ptr to proc to call when new line is  */
                                    /* found during conversion               */
    DXmSegmentProc     segment_cb;   /* ptr to proc to call when segment is   */
                                    /* found during conversion               */

                                    /* info about sement being converted     */
    char	      *text;	    /* the next string of characters         */
    int		      byte_length;  /* length of the string in chars         */
    XmStringCharSet   charset;      /* character set of the string           */
    XmStringDirection direction;    /* direction of the string               */
    short	      locale;	    /* the locale of the string		     */
    int		      nest_level;   /* nesting level of this segment         */

    XmString          stream;       /* result of CSText -> CS conversion      */

    /* 
     * fields private to the CSText source during conversion
     */
    Widget	      widget;	    /* the widget id of the CSText widget     */
    TextLine	      line;	    /* address of line for use by callback   */
    TextSegment	      segment;      /* address of segment for use by callback*/
    DXmCSTextPosition  offset;	    /* offset in segment for use by callback */
    short	      alloc_extra;  /* extra anticipatory allocation	     */
    short	      initialized;  /* has the convert been initialized	     */ 

    /*
     * fields private to the compound string converter
     */

    XmString          *answer;      /* array of CS fragments made during cvt */
    int               answers;
    Opaque            prev_seg;     /* last segment seen */
    Boolean           emitted_extra; /* you don't want to know... */
}   
    DXmCSTextCvtContextRec;

extern void DXmCSTextInvalidate();
extern void DXmCSTextSetHighlight();

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _DXmCSTextSrc_h */
