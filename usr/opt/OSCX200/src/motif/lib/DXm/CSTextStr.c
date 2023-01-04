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
#ifndef lint
static char rcsid[] = "$Header: /usr/sde/osf1/rcs/x11/src/motif/lib/DXm/CSTextStr.c,v 1.1.4.4 1993/12/17 21:16:51 Richard_June Exp $";
#endif /* lint */
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
**	International Text Widget String Source Data Handling Code 
**
**
**  MODIFICATION HISTORY:
**
**	12 APR 1990  Begin rework of old CSText widget code.
**
**	23 SEP 1991  Fix DXmCSTextGetString to function the same as
**		     XmText. It returns an empty string when the widget
**		     value is NULL. - Deb
**
**	10 MAR 1992  Fix up calls to _SourceInsertLine, SourceSplitSegment,
**		     and _SourceAppendText to have the correct number of
**		     parameters. - WDW
**
**		     Add test_length to CvtFromCSSSegmentCB - the optimizer
**		     didn't seem to like have a particular equation in the
**		     "if" statement. - WDW
**
**	12 Jan 1993  Removed SourceCacheCharset() because it's the same as
**		     _Xm_cache_charset() in Xm.  Replaced calls with
**		     _Xm_cache_charset() calls.
**
**	19 Mar 1993  Added VerifyBell handling in _DXmCSTextSourceInsertString()
**--
**/

#ifdef WIN32
#include <X11/Xlib_NT.h>
#endif
#include <Xm/XmP.h>
#include "CSTextI.h"
#include "DXmPrivate.h"
#include "CSTextSrc.h"
#include "CSTextOutP.h"
#include "CSTextInP.h"
#include "DECspecific.h"

#ifdef VMS
#include "Xatom.h"
#else
#include <X11/Xatom.h>
#endif

#ifndef MAX
#define MAX(x,y)        ((x) > (y) ? (x) : (y))
#endif

#define is_blank_line(widget, location) 		\
	       ( location.line->length == 0    ||	\
		 location.segment      == NULL ||	\
		 _all_blank(widget, location) )

extern void I18nSegmentMeasure();
extern void I18nSegmentLength();
extern Boolean I18nIsScanBreak();
extern Boolean I18nIsWhiteSpace();
extern char * _Xm_cache_charset();

/*
 * This struct is for support of Insert Selection targets.
 */
typedef struct {
    Atom selection;
    Atom target;
} CSTextInsertPair;
 
/*
 * forward declarations
 */
static void SourceAdjustRangeAfterRemove();
	/*	DXmCSTextPosition start;	     	*/
	/*	DXmCSTextPosition end;		     	*/
	/*	DXmCSTextPosition range_left;	     	*/
	/*	DXmCSTextPosition range_right;	     	*/

static void SourceAdjustRangeAfterInsert();
	/*	DXmCSTextPosition start;	     	*/
	/*	DXmCSTextPosition length;	     	*/
	/*	DXmCSTextPosition range_left;	     	*/
	/*	DXmCSTextPosition range_right;	     	*/

static void SourceFreeSegment();
	/*	TextSegment segment;			*/

static void LineLocatePosition();
	/*	TextLine line;				*/
	/*	DXmCSTextPosition position;		*/
	/*	TextSegment	*segment;		*/
	/*	DXmCSTextPosition *offset;		*/

static void SourceLocatePosition();
	/*	DXmCSTextWidget   widget;	     	*/
	/*	DXmCSTextPosition position;	     	*/
	/*	TextLine	 *line;		     	*/
	/*	DXmCSTextPosition *offset_in_line;    	*/
	/*	TextSegment	 *segment;	     	*/
	/*	DXmCSTextPosition *offset_in_segment; 	*/

static TextLine _SourceInsertLine();
	/*	DXmCSTextWidget	widget;	    		*/
	/*	TextLine	line;	    		*/

static DXmCSTextStatus SourceInsertLine();
	/*	DXmCSTextWidget	widget;	    		*/
	/*	TextLine	line;	    		*/
	/*	TextLine	*new_line;  		*/

static void SourceInsertSegment();
	/*	DXmCSTextWidget	  widget;    		*/
	/*	TextLine	  line;	     		*/
	/*	TextSegment	  segment;   		*/
	/*	int		  allocation;		*/
	/*	TextSegment	  *new_seg;  		*/

static DXmCSTextStatus _SourceAppendText();
	/*	DXmCSTextWidget    widget;    		*/
	/*	TextLine          line;      		*/
	/*	TextSegment       segment;   		*/
	/*	char		  *text;     		*/
	/*	int               length;    		*/
	/*	XmStringCharSet   charset;   		*/
	/*	XmStringDirection direction; 		*/
	/*	short             locale;    		*/

static void SourceConcatLine();
	/*	DXmCSTextWidget  widget;			*/
	/*	TextLine	line;	    		*/

static void SourceConcatSegment();
	/*	DXmCSTextWidget  widget;			*/
	/*	TextLine	line;	    		*/
	/*	TextSegment	segment;    		*/

static void SourceUnlinkSegment();
	/*	DXmCSTextWidget	widget;			*/
	/*	TextLine	line;	     		*/
	/*	TextSegment	segment;     		*/

static void SourceRemoveSegment();
	/*	DXmCSTextWidget	widget;			*/
	/*	TextLine	line;	     		*/
	/*	TextSegment	segment;     		*/
	/*	Boolean		concatenate; 		*/

static void SourceClearLine();
	/*	DXmCSTextWidget	widget;			*/
	/*	TextLine	line;	     		*/

static void SourceRemoveLine();
	/*	DXmCSTextWidget	widget;			*/
	/*	TextLine	line;	     		*/

static void SourceRemoveBunchOfLine();
	/*	DXmCSTextWidget	widget;			*/
	/*	TextLine	start_line;	     	*/
	/*	TextLine	end_line;	     	*/

static void SourceRemoveBunchOfSegment();
	/*      DXmCSTextWidget widget;			*/
	/*	TextLine	line;			*/
	/*	TextSegment	start_segment;		*/
	/*	TextSegment	end_segment;		*/

static void _SourcePopulateLine();
	/*	DXmCSTextWidget	 widget;		*/
	/*	TextLine	 line;      		*/
	/*	TextSegment	 segment;   		*/
	/*	TextLine	 new_line;  		*/

static DXmCSTextStatus SourceSplitLine();
	/*	DXmCSTextWidget	 widget;		*/
	/*	TextLine	 line;      		*/
	/*	TextSegment	 segment;   		*/
	/*	TextLine	 *out_line; 		*/

static TextSegment SourceSplitSegment();
	/*	DXmCSTextWidget	 widget;		*/
	/*	TextLine	 line;     		*/
	/*	TextSegment	 segment;  		*/
	/*	DXmCSTextPosition offset;   		*/

/**** NOTE!!!  Never use SourceRemove() to remove sources.  The output will not
 *             be notified for the change.  This is for optimization purpose.
 *	       ONLY use _DXmCSTextSourceReplaceString().
 */
static void SourceRemove();
	/*	DXmCSTextWidget widget;			*/
	/*	DXmCSTextPosition start, end;		*/

static void CvtFromCSSegmentCB();
	/*	DXmCSTextCvtContext  context;		*/

static void CvtFromCSLineCB();
	/*	DXmCSTextCvtContext context;		*/

static void LoseSelection();
	/*	Widget w;				*/
	/*	Atom *selection;  			*/

/* save for possible future use  
static DXmCSTextPosition MirrorPosition();
*/
	/*	DXmCSTextWidget widget;			*/
	/*	DXmCSTextPosition pos, start,   		*/
        /*                   auxpos1, auxpos2;		*/
	/*	TextSegment segment;			*/

static void call_source_changed_callback();
	/*	DXmCSTextWidget widget;			*/

static Boolean call_source_modify_callback();
	/*	DXmCSTextWidget widget;			*/
	/*	DXmCSTextPosition *start;		*/
	/*	DXmCSTextPosition *length;		*/
	/*	XmString *string;			*/

static void call_segment_modified_callback();
	/*	DXmCSTextWidget widget;			*/
	/*	TextLine line;				*/
	/*	TextSegment segment;			*/

static void call_segment_delete_callback();
	/*	DXmCSTextWidget widget;			*/
	/*	TextLine line;				*/
	/*	TextSegment segment;			*/

static void call_line_delete_callback();
	/*	DXmCSTextWidget widget;			*/
	/*	TextLine line;				*/

static void call_line_added_callback();
	/*	DXmCSTextWidget widget;			*/
	/*	TextLine line;				*/

static void call_line_modified_callback();
        /*      DXmCSTextWidget widget;                  */
        /*      TextLine line;                          */

static void call_single_line_delete_callback();
	/*	DXmCSTextWidget widget;			*/
	/*	TextLine	line;			*/

static void call_single_segment_delete_callback();
	/*	DXmCSTextWidget widget;			*/
	/*	TextLine	line;			*/
	/*	TextSegment	segment;		*/

static void call_output_validate_rest_line();
	/*	DXmCSTextWidget widget;			*/
	/*	TextLine	line;			*/
	/*	TextSegment	segment;		*/

static void call_output_invalidate_single_line();
	/*	DXmCSTextWidget widget;			*/
	/*	TextLine	line;			*/
	/*	TextSegment	segment;		*/

static void call_output_text_empty();
	/*	DXmCSTextWidget widget;			*/

/*======================================================================*/
/*
 *    International string and internal data format handling routines
 */
/*======================================================================*/

static 
void _fix_length (widget, who)
    DXmCSTextWidget widget;
     char * who;
{
  TextLine line;
  TextSegment seg;
  int lc, wc = 0;

  /* just return.  Doesn't work for word wrap
   */

  for (line = (TextLine)(widget->cstext.lines);
       line != NULL;
       line = line->next)
    {
      lc = 0;

      for (seg = line->segments;
	   seg != NULL;
	   seg = seg->next)
	{
	  wc += seg->length;
	  lc += seg->length;

	  if (seg->direction > 2 )
	    {
	      printf ("%s: illegal direction value, is %d\n", 
		      who, seg->direction );
	    }
	}


      if (line->length != lc)
	{
	  printf ("%s: line count wrong, is %d should be %d, resetting\n", 
		  who, line->length, lc);

	  line->length = lc;
	}

      wc++;
    }

  if (wc) wc--;

  if (widget->cstext.length != wc)
    {
      printf ("%s: widget count wrong, is %d should be %d, resetting\n", 
	      who, widget->cstext.length, wc);
      widget->cstext.length = wc;
    }
}



static void
_dump_source (widget)
    DXmCSTextWidget   widget;
{
  TextLine line;
  TextSegment seg;
  int i, j, k;
  unsigned char c;

  for (line = (TextLine)(widget->cstext.lines), i=0;
       line != NULL;
       line = line->next, i++)
    {
      printf ("\n");
      printf ("line [%5d], address %10d:\n", i, line);
      printf ("\t\tnext:   %10d\n", line->next);
      printf ("\t\tprev:   %10d\n", line->prev);
      printf ("\t\tlength: %10d\n", line->length);

      for (seg = line->segments, j=0;
	   seg != NULL;
	   seg = seg->next, j++)
	{
	  printf ("\n");
	  printf ("\tseg [%3d], address %10d:\n", j, seg);

	  printf ("\t\t\tnext:       %10d\n", seg->next);
	  printf ("\t\t\tprev:       %10d\n", seg->prev);

	  printf ("\t\t\tallocation: %10d\n", seg->allocation);
	  printf ("\t\t\tused_allocation: %10d\n", seg->used_allocation);
	  printf ("\t\t\tlength:     %10d\n", seg->length);
	  printf ("\t\t\ttext:       ");
	  for (k=0; k<seg->used_allocation; k++)
	    {
	      c = (unsigned char) seg->text[k];
	      if( c == '\0')c = '.';
	      printf ("%c", c);
	    }
	  printf ("\n");

	  if( seg->charset == NULL )
	  {
	    printf ("\t\t\tcharset:  NULL" );
	  }else{
	    printf ("\t\t\tcharset:    %10s\n", seg->charset);
	  }
	  printf ("\t\t\tdirection : %10d\n", seg->direction);
	  printf ("\t\t\tnest_level:    %10d\n", seg->nest_level);
	}
    }
}



/*======================================================================
 * frees up the storage for a segment
 */
static void SourceFreeSegment( segment )
TextSegment segment;	
{
    if( segment == (TextSegment)NULL ) 
    {
	return;
    }

    if( segment->text != (char*)NULL )
    {
	XtFree( segment->text );
        segment->text = NULL;
    }

    XtFree((char *) segment );

    segment = NULL;
}


/*======================================================================
 * find the segment in the given line that contains the given character 
 * position, return the segment and the offset in the segment of the    
 * character.  If the position is greater than the line length then     
 * return the last segment in line, and offset equal to length of the   
 * last segment
 */
static void LineLocatePosition( line, position, segment, offset )
TextLine line;			/* line				*/
DXmCSTextPosition position;	/* requested line position	*/
TextSegment	*segment;	/* return segment addr.		*/
DXmCSTextPosition *offset;	/* offset within return segment	*/
				/* note that offset is specified */
				/* units of one position        */
{
    DXmCSTextPosition total;
    DXmCSTextPosition next_count;
    TextSegment next_segment;

    if( line == (TextLine)NULL )
    {
	*segment = (TextSegment)NULL;
	*offset = 0;
	return;
    }

    if( position > line->length )
    {
	position = line->length;
    }

    if( position < 0 )
    {
	position = 0;
    }

    /* run through all the segments in the line until character count */
    /* exceeds requested count, char will be in that segment
    */
    next_count = 0; 
    total      = 0;

    *segment = line->segments;

    for( next_segment = line->segments;
	 next_segment != (TextSegment)NULL;
	 next_segment = next_segment->next)
    {
	next_count += next_segment->length;

	*segment = next_segment;

	if( next_count > position ) 
	{
	    break;                             /* too far */
	}

	total += next_segment->length;
    }

    *offset = position - total;
}





/*======================================================================*/
/* This routine will look for specified "position" within "source"	*/
/* and return pointer to source "segment" and "offset" within		*/
/* the segment data containing the requested position. If position	*/
/* is out of the source boundary, the routine will return pointer to	*/
/* last segment in last line with offset equal to length of last	*/
/* segment.  Offset is an index and not a byte offset.			*/
/*									*/
static
void SourceLocatePosition( widget, 
			  position, 
			  line, 
			  offset_in_line,
			  segment, 
			  offset_in_segment )

DXmCSTextWidget   widget;	     /* source				  */
DXmCSTextPosition position;	     /* requested position		  */
TextLine	 *line;		     /* line position occurs in		  */
DXmCSTextPosition *offset_in_line;    /* offset of position within line    */
TextSegment	 *segment;	     /* segment position occurs in	  */
DXmCSTextPosition *offset_in_segment; /* offset of position within segment */
{
    DXmCSTextPosition char_count;
    DXmCSTextPosition next_count;
    TextLine next_line;

#ifdef DEBUG
_fix_length (widget, "SourceLocatePosition, beginning");
#endif

    if( position < 0 )
    {
	position = 0;
    }

    if( position > CSTextLength (widget) )
    {
	position = CSTextLength (widget);
    }

    *line = NULL;
    *segment = NULL;
    *offset_in_line = 0;
    *offset_in_segment = 0;

    /* run through all the lines in the source until character count */
    /* exceeds requested count, char will be in that line
    */
    next_count = 0;
    char_count = 0;

    for( next_line = (TextLine)CSTextLines (widget);
	 next_line != (TextLine)NULL;
	 next_line = next_line->next )
    {
	*line = next_line;

	next_count = char_count + next_line->length;

	if( next_count > position ) 
	{
	  /*
	   * this line starts before and ends after the position we 
	   * are looking for, so dig around in the segment
	   */

	  *offset_in_line = position - char_count;

	  LineLocatePosition (*line, 
			      *offset_in_line, 
			      segment, 
			      offset_in_segment );

	  return;
	}

	if (next_count == position)
	  {
	    /*
	     * the line ends exactly on the position we are looking for
	     * so set up the location and return;
	     */

	    *offset_in_line = next_line->length;
	    *segment = NULL;
	    *offset_in_segment = 0;
	    return;
	  }

	char_count += next_line->length;

	char_count++;           /* the extra count for end of line */
    }

    /*
     * if we get here then we're looking for the position which is 
     * one beyond the last line, and depends on the last char_count++
     * so we better fix *line to be NULL indicating no line...
     */

    *line = NULL;

    return;
}



/*======================================================================*/
/* This routine will return the position relative to the beginning of   */
/* the source of the character represented by the given line, segment,  */
/* and offset.  
*/
#ifdef _NO_PROTO
DXmCSTextPosition _DXmCSTextSourceGetPosition( widget, 
					      line, 
					      segment, 
					      offset )

DXmCSTextWidget   widget;	/* source				*/
TextLine	 line;		/* line position occurs in		*/
TextSegment	 segment;	/* segment position occurs in		*/
DXmCSTextPosition offset;       /* offset of position within segment	*/
#else
DXmCSTextPosition _DXmCSTextSourceGetPosition(
				DXmCSTextWidget   widget,
				TextLine	  line,
				TextSegment	  segment,
				DXmCSTextPosition offset )
#endif /* _NO_PROTO */
{
    DXmCSTextPosition next_count;
    TextLine    next_line;
    TextSegment next_segment;

#ifdef DEBUG
_fix_length (widget, "_DXmCSTextSourceGetPosition, beginning");
#endif

    /* if the source is empty, bail out 
    */
    if( CSTextLength (widget) == 0 ) 
    {
	return (DXmCSTextPosition)0;
    }

    for( next_line = (TextLine)CSTextLines (widget), next_count = 0;
	 next_line != line;
	 next_line = next_line->next )
    {
	next_count += next_line->length + 1;
    }

    for( next_segment = next_line->segments;
	 next_segment != segment;
	 next_segment = next_segment->next )
    {
	next_count += next_segment->length;
    }

    next_count += offset;

    if (next_count < 0)
	return ((DXmCSTextPosition) 0);

    if (next_count > CSTextLength(widget))
	return (CSTextLength(widget));

    return (next_count);
}

/*======================================================================*/
/* fill in the location data structure with results of call to		*/
/* to SourceLocatePosition
*/
#ifdef _NO_PROTO
void _DXmCSTextSourceLocate( widget, position, location )
DXmCSTextWidget   widget;
DXmCSTextPosition position;	     /* requested position		  */
TextLocation	 location;	     /* location data structure pointer   */
#else
void _DXmCSTextSourceLocate( DXmCSTextWidget   widget,
			     DXmCSTextPosition position,
			     TextLocation      location )
#endif /* _NO_PROTO */
{
    if( position < 0 ) position = 0;

    if( position > CSTextLength( widget ) )
    {
	location->position = CSTextLength( widget );
    }else{
        location->position = position;
    }

    SourceLocatePosition( widget, 
			 location->position, 
			 &location->line,
			 &location->line_offset,
			 &location->segment,
			 &location->offset );

    location->end_of_line = location->segment == (TextSegment)NULL;
}

/*** Duplicate of _Xm_cache_charset in Xm ***/
/*** REMOVE AFTER TESTING ***/
#ifdef NEVER
static XmStringCharSet *cstext_charset_cache = NULL;
static int    cstext_charset_count = 0;

/*======================================================================*/
/* this routine checks character set string against cached strings and  */
/* creates new string in cache if needed
*/
static XmStringCharSet SourceCacheCharset( charset )
XmStringCharSet charset; 
{
    int length;
    char *a;
    int i;
    int l;

    length = strlen( charset );

    /* run through the charset strings in the cache
    */
    for( i = 0; i < cstext_charset_count; i++ )
    {
	l = strlen( cstext_charset_cache[i] );

	/* first check the lengths, if not the same then don't compare strings
	*/
        if( l == length &&
	    strncmp( charset, cstext_charset_cache[i], l ) == 0 ) 
	{
	    return ( cstext_charset_cache[i] );
	}
    }
    
    /* not in cache, so need to add it to cache
    */
    cstext_charset_cache = (XmStringCharSet*)
	XtRealloc( (char *)cstext_charset_cache, 
		   sizeof(XmStringCharSet *) * (cstext_charset_count + 1) );

    a = XtMalloc( length + 1 );

    bcopy( charset, a, length );

    a[ length ] = '\0';

    cstext_charset_cache[ cstext_charset_count++ ] = a;

    return( a );
}


#endif /* NEVER */
/*======================================================================*/
/* inserts a new text line after the given (possibly null) line */
/* if the line is null then it creates a new line as the first  */
/* (not necessarily last) line of the source
*/
static TextLine _SourceInsertLine( widget, line )
DXmCSTextWidget	widget;	    /* the line containing the segment	*/
TextLine	line;	    /* line next to the new one		*/
{
    TextLine new_line;

    /* allocate the new text line
    */
    new_line = (TextLine) XtMalloc( sizeof(TextLineRec) );

    new_line->prev        = line;
    new_line->length      = 0;
    new_line->output_line = NULL;
    new_line->segments    = (TextSegment)NULL;

    if ( line == (TextLine)NULL )
    {
	/* given line is null, so insert as first line of source,
	 * note that this line does NOT count for the length of the
	 * the source
	*/
	if ( (TextLine)CSTextLines (widget) != (TextLine)NULL )
	{
	    /* some lines already exist, so update pointers
	    */
	    ((TextLine)CSTextLines (widget))->prev = new_line;
	}

	new_line->next = (TextLine)CSTextLines (widget);
	CSTextLines (widget) = new_line;

    }else{
	/* adding a linefeed to the source
	 */
	CSTextLength (widget)++;
	
	/* given line is not null, adjust pointers
	 */
	if ( line->next != (TextLine)NULL )
	  {
	    /* given line is not the last in the source
	    */
	    (line->next)->prev = new_line;
	  }

	new_line->next = line->next;
	line->next = new_line;
    }

    call_line_added_callback(widget, new_line);

    return new_line;
}

/*======================================================================*/
/* inserts a new text line after the given (possibly null) line */
/* if the line is null then it creates a new line as the first  */
/* (not necessarily last) line of the source
*/
static DXmCSTextStatus SourceInsertLine( widget, line, new_line )
DXmCSTextWidget	widget;	    /* the line containing the segment	*/
TextLine	line;	    /* line next to the new one		*/
TextLine	*new_line;  /* the new line created		*/
{
    /* creating a new line is the moral equivalent of adding a new-line */
    /* character to the string, make sure we can
    */

    /*
     * The CSTextLength - 1 is to compensate for the "!" character added
     * in the _DXmCSTextSourceInsertString routine. The "!" character is
     * used a placeholder for the 'marker' segment during the logical to
     * virtual conversion. (See _DXmCSTextSourceInsertString routine for more
     * details). 
     *
     * If CSTextLength is used instead of CSTextLength-1, XmNmaxLength
     * does not work and only max-1 characters are allowed to be entered.
     */
    if( (CSTextLength (widget) - 1) >= CSTextMaxLength (widget) ) 
    {
	return I_STR_EditError;
    }

    *new_line = _SourceInsertLine( widget, line );

#ifdef DEBUG
_fix_length (widget, "SourceInsertLine, end");
#endif

    return I_STR_EditDone;
}

/*======================================================================*/
/* inserts a new text segment after the given (possibly null) segment */
/* if the segment is null then it creates a new segment as the first  */
/* (not necessarily last) segment of the given line.  
*/
static void SourceInsertSegment( widget, 
				 line, 
				 segment,
				 allocation,
				 new_seg )
DXmCSTextWidget	  widget;    /* the text widget				 */
TextLine	  line;	     /* the line containing the segment          */
TextSegment	  segment;   /* segment next to (before) new one	 */
int		  allocation;/* number of text characters to allocate    */
TextSegment	  *new_seg;  /* created segment return			 */
{
    TextSegment     new_segment;

    /* allocate the new text segment, and the requested text string storage
    */
    new_segment = (TextSegment) XtMalloc( sizeof(TextSegmentRec) );
    new_segment->text = XtMalloc( allocation );
    new_segment->allocation  = allocation;
    new_segment->used_allocation = 0;
    new_segment->length      = 0;
    new_segment->prev        = segment;

    new_segment->num_output_segments = 0;
    new_segment->output_segment = NULL;

    if ( segment == (TextSegment)NULL )
    {
	/* given segment is null so put new segment into first */
	/* position in line 
	*/
	if ( line->segments != (TextSegment)NULL )
	{
	    /* line does have segments, so fix up pointer
	    */
	    (line->segments)->prev = new_segment;
	}
	new_segment->next = line->segments;
	line->segments = new_segment;
    }else{
	/* given segment is not null, so insert new segment after it
	*/
	if ( segment->next != (TextSegment)NULL )
	{
	    /* given segment is not last in line, so fix pointers
	    */
	    (segment->next)->prev = new_segment;
	}
	new_segment->next = segment->next;
	segment->next = new_segment;
    }

    *new_seg = new_segment;

    return;
}

/*======================================================================*/
/* appends new text at the end of the given segment.  Allocates if needed.
*  modifies only segment info, not line or widget info
*/
static DXmCSTextStatus _SourceAppendText( widget, 
				         line,
				         segment, 
				         text, 
					 char_length,
				         byte_length, 
				         charset,
				         direction,
				         locale,
					 nest_level )

DXmCSTextWidget   widget;     /* the text source                  */
TextLine          line;       /* the line containing the segment  */
TextSegment       segment;    /* segment to append into		  */
char              *text;      /* text to append into segment	  */
int               char_length;/* length of text in no. of chars   */
int               byte_length;/* length of text in no. of bytes   */
XmStringCharSet   charset;    /* charset for the new segment      */
XmStringDirection direction;  /* direction for the new segment    */
short             locale;     /* locale for the new segment       */
short             nest_level; /* directionality nesting level     */
{
    int new_byte_length, old_byte_length;

    if( segment == (TextSegment)NULL )
    {
	return I_STR_EditError;
    }

    old_byte_length = segment->used_allocation;
    new_byte_length = old_byte_length + byte_length;

    /* if there isn't currently enough allocation, reallocate
    */
    if(	new_byte_length > segment->allocation )
    {
	/* allocate one more for safety
	*/
	segment->allocation = new_byte_length + 1;

	segment->text = XtRealloc( segment->text, segment->allocation );
    }

    /* copy the text into the new segment
    */
    bcopy( text, &segment->text[ old_byte_length], byte_length );

    segment->length = segment->length + char_length;
    segment->used_allocation = new_byte_length;

    segment->charset    = _Xm_cache_charset( charset, (int)(strlen(charset)) );
    segment->direction  = direction;
    segment->locale     = locale;
    segment->nest_level = nest_level;

    return I_STR_EditDone;
}

/*======================================================================*/
/*  This routine will concatenate two consecutive lines
*/
static void SourceConcatLine( widget, line )
DXmCSTextWidget  widget;
TextLine	line;	    /* first line */
{
    TextLine next_line;
    TextSegment first_segment, next_segment;

    if( line == (TextLine)NULL )
    { 
	return;
    }

    next_line = line->next;

    if( next_line == (TextLine)NULL )
    { 
	return;
    }

    /* point to the last segment in the first line
    */
    for( next_segment = line->segments, first_segment = line->segments; 
	 next_segment != (TextSegment)NULL;
	 first_segment = next_segment, next_segment = first_segment->next )
    {}

    /* point to the first segment in the second line
    */
    next_segment = next_line->segments;

    call_line_delete_callback( widget, next_line );

    /* clean up the pointers
    */
    line->next = next_line->next;

    if( next_line->next != (TextLine)NULL )
    {
	(next_line->next)->prev = line;
    }

    if( first_segment != (TextSegment)NULL )
    {
        first_segment->next = next_segment;
    }else{
	line->segments = next_segment;
    }

    if( next_segment != (TextSegment)NULL )
    {
        next_segment->prev = first_segment;
    }

    /*
     * update counters
     */
    line->length += next_line->length;
    CSTextLength (widget)--;

    /* we haven't gotten rid of any segments, so don't free them, just the line
    */
    XtFree( (char *)next_line );

    next_line = NULL;

    call_line_modified_callback(widget, line);

    return;
}


/*======================================================================*/
/*  This routine will concatenate two consecutive segments if their */
/*  attributes are identical, freeing up deleted resources.   
*/
static void SourceConcatSegment( widget, line, segment )
DXmCSTextWidget  widget;
TextLine	line;	    /* line containing segments */
TextSegment	segment;    /* first segment		*/
{
    TextSegment next_seg;
    int append_length;

    if( line == (TextLine)NULL )
    { 
	return;
    }

    if( segment == (TextSegment)NULL )
    { 
	return;
    }

    next_seg = segment->next;

    if(	next_seg == (TextSegment)NULL )
    { 
	return;
    }

    /* first check to see if we can collapse the two segments 
    */
    if(( segment->charset    != next_seg->charset    ) || 
       ( segment->direction  != next_seg->direction  ) ||
       ( segment->locale     != next_seg->locale     ) ||
       ( segment->nest_level != next_seg->nest_level ) ||
       ( segment->charset    != next_seg->charset    ))
    {
        return;
    }

    /* save length because output callback diddles with it
    */
    append_length = next_seg->length;

    call_segment_delete_callback( widget, line, next_seg );

    /* take next_seg out of the segment list
    */
    SourceUnlinkSegment( widget, line, next_seg );

    /* now concatenate the text strings
    */
    _SourceAppendText( widget, 
		      line,
		      segment, 
		      next_seg->text, 
		      append_length, 
		      next_seg->used_allocation,
		      next_seg->charset,
		      next_seg->direction,
		      next_seg->locale,
		      next_seg->nest_level );

    call_segment_modified_callback(widget, line, segment);

    SourceFreeSegment( next_seg );

    return;
}


/*======================================================================*/
/* Unlink a segment from a segment list.				
*/
static void SourceUnlinkSegment( widget, line, segment )
DXmCSTextWidget	widget;
TextLine	line;	     /* line containing the segment   */
TextSegment	segment;     /* segment to be removed	      */
{
    TextSegment prev_seg, next_seg;

    prev_seg = segment->prev,
    next_seg = segment->next;

    /* take the segment out of the list
    */
    if( prev_seg != (TextSegment)NULL )
	prev_seg->next = next_seg;
    else
	line->segments = next_seg;

    if( next_seg != (TextSegment)NULL )
	next_seg->prev = prev_seg;

    return;
}

/*======================================================================*/
/* Remove a segment from a line.  If we have after the deletion two */
/* adjacent segments with same characteristics (character set and   */
/* direction), concatenate these segments if requested.
*/
static void SourceRemoveSegment( widget, line, segment, concatenate )
DXmCSTextWidget	widget;
TextLine	line;	     /* line containing the segment   */
TextSegment	segment;     /* segment to be removed	      */
Boolean		concatenate; /* should we try to concatenate? */
{
    int delete_length;
    int location;

    if( segment == (TextSegment)NULL )
	return;

    /* find out offset in the line of the start of segment for callback
    */

    delete_length = segment->length;

/* Bug fix - move after SourceUnlinkSegment /Mak
    ** call the single segment deleted callback
    **
    call_single_segment_delete_callback( widget, line, segment );
 */

    /* update length after calling HandleData
     */
    CSTextLength (widget) -= delete_length;
    line->length   -= delete_length;

    SourceUnlinkSegment( widget, line, segment );

/* Bug fix - move after SourceUnlinkSegment /Mak */
    /* call the single segment deleted callback
    */
    call_single_segment_delete_callback( widget, line, segment );

    /* now concatenate if possible
    */
    if( concatenate ) SourceConcatSegment( widget, line, segment->prev );

    /* free up the deleted segment
    */
    SourceFreeSegment( segment );

    return;
}


/*======================================================================*/
/* Removes the line's segments.
*/
static void SourceClearLine( widget, line )
DXmCSTextWidget	widget;
TextLine	line;	     /* line containing the segment   */
{
    TextSegment delete_segment, next_segment;

    if( line == (TextLine)NULL)
	return;

    /* run through the line and free up any segments
    */

    SourceRemoveBunchOfSegment(widget, line, line->segments, NULL);

    line->segments = (TextSegment)NULL;

    return;
}


/*======================================================================*/
/* Remove a line from the source.  Removes the line's segments also.
*/
static void SourceRemoveLine( widget, line )
DXmCSTextWidget	widget;
TextLine	line;	     /* line containing the segment   */
{
    TextLine prev_line, next_line;

    if( line == (TextLine)NULL)
	return;

    /* run through the line and free up any segments
    */
    SourceClearLine( widget, line );

    call_single_line_delete_callback( widget, line );

    prev_line = line->prev;
    next_line = line->next;

    /* take the line out of the list
    */
    if( prev_line != (TextLine)NULL )
	prev_line->next = next_line;
    else
	CSTextLines( widget ) = next_line;

    if( next_line != (TextLine)NULL )
	next_line->prev = prev_line;

    /* subtract the virtual new line character
    */
    /* Single line has no newline character to compensate for !
    */
    if (CSTextLines( widget ) != (TextLine)NULL )
    CSTextLength (widget) = CSTextLength (widget) - 1;

    /* free up the deleted line
    */
    XtFree( (char *)line );

    line = NULL;

    return;
}

/*======================================================================*/
/* remove the lines in between the start_line and the end_line (or the single
/* start_line if start_line == end_line)
/* optimized by calling the Output's HandleData as single line delete and
/* validate the rest of the lines afterwards
*/
static void SourceRemoveBunchOfLine( widget, start_line, end_line )
DXmCSTextWidget	widget;
TextLine	start_line;
TextLine	end_line;
{

     TextLine next_line, next_one;

     for( next_line  = start_line;
	  next_line != end_line;
	  next_line  = next_one
	)
     {
          /* save the next pointer for the "next_line" will be freed inside
             the SourceRemoveLine()
          */
          next_one = next_line->next;

	  SourceRemoveLine( widget, next_line );
     }

}

/*======================================================================*/
/* remove the segments in between the start_segment and the end_segment
/* optimized by calling the Output's HandleData as single segment delete and
/* validation is done afterwards in SourceRemoveBunchOfLine() routine.
*/
static void SourceRemoveBunchOfSegment( widget,
					line,
					start_segment,
					end_segment )
DXmCSTextWidget	widget;
TextLine	line;
TextSegment	start_segment;
TextSegment	end_segment;
{

      TextSegment next_segment, next_one;

	for( next_segment  = start_segment;
	     next_segment != (TextSegment)NULL && next_segment != end_segment;
	     next_segment  = next_one
	   )
	{
             /* save the next pointer for the "next_segment" will be freed 
                inside the SourceRemoveSegment()
             */
             next_one = next_segment->next;

	     SourceRemoveSegment(widget, line, next_segment, False );
	}

}




/*======================================================================*/
/* populate new line just split.  The given segment will be		*/
/* the first segment of the new line.
*/
static void _SourcePopulateLine( widget, line, segment, new_line )
DXmCSTextWidget	 widget;
TextLine	 line;      /* the line to split */
TextSegment	 segment;   /* make split at this segment */
TextLine	 new_line;  /* the new line created after split point */
{
    TextSegment next_seg, prev_seg;
    int		length;

    if( line     == (TextLine)NULL ) return;
    if( new_line == (TextLine)NULL ) return;

    /* the below works even if segment is null, which means all of line */
    /* remains in line and nothing goes into new_line
    */
    length = 0;

    /* how long is the line up to this segment? 
    */
    for( next_seg = line->segments; 
	 next_seg != segment; 
	 next_seg = next_seg-> next )
    {
	length += next_seg->length;
    }

    new_line->segments = segment;
    new_line->length   = line->length - length;

    line->length       = length;

    if( segment != (TextSegment)NULL )
    {
        prev_seg = segment->prev;
	segment->prev  = (TextSegment)NULL;

	if (prev_seg == (TextSegment)NULL )
	  {
	    line->segments = NULL;
	  }
	else
	  {
	    prev_seg->next = (TextSegment)NULL;
	  }
    }
}


/*======================================================================*/
/* split a line into two at the given segment.  The given segment will be */
/* the first segment of the new line thus created
*/
static DXmCSTextStatus SourceSplitLine( widget, line, segment, out_line )
DXmCSTextWidget	 widget;
TextLine	 line;      /* the line to split */
TextSegment	 segment;   /* make split at this segment */
TextLine	 *out_line; /* the new line created after split point */
{
    TextLine	new_line;
    DXmCSTextStatus ret_status;

    if( line == (TextLine)NULL ) 
    {
	*out_line = (TextLine)NULL;
	return I_STR_EditDone;;
    }

    /* create a new line
    */
    ret_status = SourceInsertLine( widget, line, &new_line );

    if( ret_status != I_STR_EditDone ) return ret_status;

    _SourcePopulateLine( widget, line, segment, new_line );

    call_line_modified_callback(widget, line);

    *out_line = new_line;

    return I_STR_EditDone;
}    

/*======================================================================*/
/* split a segment into two at the given (character) offset, return */
/* pointer to new segment thus created.  Does not deallocate the part of */
/* segment beyond split point.
*/
static TextSegment SourceSplitSegment( widget, line, segment, offset )
DXmCSTextWidget	 widget;
TextLine	 line;    /* the line the segment is in */
TextSegment	 segment; /* the segment to split */
DXmCSTextPosition offset; /* how far into segment (char offset) to make split */
{
    TextSegment	new_segment;
    char *text;
    int new_length, dummy, old_used_allocation, new_allocation;

    if( segment == (TextSegment)NULL ) return segment;

    if( offset >= segment->length ) return segment->next;

    if( offset == 0 ) return segment;

    /* the second (new) segment will contain everything after (and including)*/
    /* the split point
    */
    new_length = segment->length - offset;

    old_used_allocation = segment->used_allocation;

    /* get the byte length that will remain in existing segment
    */
    I18nSegmentMeasure( (char *)NULL,  /* I18nContext */
			(char *)segment->charset,
			(char *)segment->text,
			0,	 /* character offset     */
			offset,  /* number of characters */
			&dummy,  /* will be zero */
			&segment->used_allocation ); /* byte offset of end */

    /* the rest of the old used allocation will be moved to new segment
    */
    new_allocation = old_used_allocation - segment->used_allocation;

    /* keep the original segment and allocation, but reduce it's length to */
    /* length of text before the split point
    */
    segment->length = offset;

    text = segment->text;

    /* new segment will contain trailing part of original segment
    */
    SourceInsertSegment( widget,	
			 line, 
			 segment,  
			 new_allocation, 
			 &new_segment ); 

    /* now write the new text into the previous (possibly new) segment */
    /* note _SourceAppendText does not modify length of widget or line
    */
    _SourceAppendText( widget, 
		       line,
		       new_segment, 
		       &text[ segment->used_allocation ], 
		       new_length, 
		       new_allocation,
		       segment->charset,
		       segment->direction,
		       segment->locale,
		       segment->nest_level );

    call_segment_modified_callback(widget, line, segment);

    call_segment_modified_callback(widget, line, new_segment);

    return new_segment;
}    

/*======================================================================
 * this routine removes source text beginning at the given position, up to 
 * but not including the end position given
 */
static void SourceRemove( widget, start, end )
DXmCSTextWidget widget;
DXmCSTextPosition start, end;
{
    TextLine	     line, start_line, last_line, next_line;
    TextSegment	     segment, start_segment, end_segment, next_segment; 
    DXmCSTextPosition line_offset, start_offset, end_offset;
    TextLocationRec  start_loc, end_loc;
    Boolean concatenate;
    DXmCSTextPosition new_cursor_pos;
    
    if( start > CSTextLength (widget) )
	start = CSTextLength (widget);

    if( end > CSTextLength (widget) )
	end = CSTextLength (widget);

    if( end <= start )
	return;

    _DXmCSTextSourceLocate( widget, start, &start_loc );
    _DXmCSTextSourceLocate( widget, end,   &end_loc   );

    next_line = start_loc.line;
    last_line = end_loc.line;

    /* delete all the lines in between the start and end
    */
    if( next_line != last_line )
    {
	SourceRemoveBunchOfLine( widget, next_line->next, end_loc.line );
    }

    /* end_segment will point to the next segment not to delete */
    /* must do it in this order
    */
    end_segment = SourceSplitSegment( widget,
				      end_loc.line, 
				      end_loc.segment, 
				      end_loc.offset );

    /* start_segment will point to first segment to delete
    */
    start_segment = SourceSplitSegment( widget,
					start_loc.line, 
					start_loc.segment, 
					start_loc.offset );

    /* delete all the segments after the start_segment it its line
    */

    SourceRemoveBunchOfSegment( widget, start_loc.line,
			        start_segment, end_segment );

    /* now concatenate the lines, if necessary
     */
    if( start_loc.line != end_loc.line )
    {
	/* delete all the segments before the end_segment in its line
	*/
	SourceRemoveBunchOfSegment( widget, end_loc.line,
				    end_loc.line->segments, end_segment );

        SourceConcatLine( widget, start_loc.line );
    }else {

        /* invalidate single line just to make sure this line is
           to be recomputed and redisplayed
        */
	call_output_invalidate_single_line(widget, start_loc.line, NULL);

    }

    /* concatenate segments if possible
    */
    if( end_segment != (TextSegment)NULL )
    {
        SourceConcatSegment( widget, start_loc.line, end_segment->prev );
    }

    /* adjust selection range, if any
     */
    if( CSTextHasSelection( widget ) ) 
    {
	/* adjust the selection endpoints
	*/
	SourceAdjustRangeAfterRemove( start, 
				      end, 
				      &CSTextSelLeft( widget ),
				      &CSTextSelRight( widget ) );

	if( CSTextSelLeft( widget ) == CSTextSelRight( widget ) ) 
	{
	    CSTextHasSelection( widget ) = FALSE;
	}else{
	    _DXmCSTextSetSelection( widget,
				    CSTextSelLeft( widget ),
				    CSTextSelRight( widget ), CurrentTime );
	}
    }

    /* adjust the cursor position, but don't set it yet
    */
    new_cursor_pos = CursorPos( widget );

    SourceAdjustRangeAfterRemove( start, 
				  end, 
				  &new_cursor_pos,
				  0 );

    /* just set CursorPos(widget) but not calling DXmCSTextSetCursorPosition
     * because output structure is not updated yet
     */
    CursorPos( widget ) = new_cursor_pos;

    if (CSTextLength(widget) == 0)
	call_output_text_empty(widget);

    return;
}


static
void SourceAdjustRangeAfterRemove( start, end, range_left, range_right )
DXmCSTextPosition start;
DXmCSTextPosition end;
DXmCSTextPosition *range_left;
DXmCSTextPosition *range_right;
{
    int removed_length = end - start;

    /* if start of removed section is to the left of the range 
     */
    if( start <= *range_left )
    {
	/* the removed text will change the left end of the range
	*/
	*range_left -= removed_length;

	/* if some of the range was removed
	*/
	if( *range_left < start ) 
	    *range_left = start;
    }

    /* now check the right end of the range ( no, not <=, just < )
     */
    if( range_right != 0 && start < *range_right )
    {
	/* the removed text will change the right end of the range
	*/
	*range_right -= removed_length;

	/* if some stuff to right of selected stuff was removed
	*/
	if( *range_right < start ) 
	    *range_right = start;
    }
}

static
void SourceAdjustRangeAfterInsert( start, length, range_left, range_right )
DXmCSTextPosition start;
DXmCSTextPosition length;
DXmCSTextPosition *range_left;
DXmCSTextPosition *range_right;
{
    /* if start of removed section is to the left of the range 
     */
    if( start <= *range_left )
    {
	/* the added text will change the left end of the range
	*/
	*range_left += length;
    }

    /* now check the right end of the range ( no, not <=, just < )
     */
    if( range_right != 0 && start < *range_right )
    {
	/* the added text will change the right end of the range
	*/
	*range_right += length;
    }
}


/*======================================================================*/
/* this routine takes the given compound string and calls source        */
/* insert string.  This routine is meant to be called for SelfInsert    */
/* and represents an optimization
 */
#ifdef _NO_PROTO
DXmCSTextStatus _DXmCSTextSourceInsertChar( widget, start, string )
DXmCSTextWidget    widget;
DXmCSTextPosition  start;     /* where to copy text into the source */
XmString	  string;    /* text to be copied into allocation  */
#else
DXmCSTextStatus _DXmCSTextSourceInsertChar( DXmCSTextWidget    widget,
					    DXmCSTextPosition  start,
					    XmString	       string )
#endif /* _NO_PROTO */
{
    DXmCSTextStatus ret_status;
    XmString	 newstring;	/* potentially modified by callback */
    DXmCSTextPosition newstart, newend;

    newstring = string;
    newstart = start;
    newend = start;

    if( !call_source_modify_callback( widget, &newstart, &newend, &newstring ) )
    {
	if (newstring != string)
	    XmStringFree(newstring);
	if (CSTextVerifyBell(widget)) XBell(XtDisplay(widget), 0);
	return I_STR_EditDone;
    }

    ret_status = 
	_DXmCSTextSourceInsertString( widget, newstart,
				      newstring, SEGMENT_EXTRA_ALLOCATION );

    if (newstring != string)
	XmStringFree(newstring);

    call_source_changed_callback(widget);

    return ret_status;
}

static
char *_flip_text( text, length) 
char *text ;
int  length ;
{
     /* Flip text in place
     */
     char c ;
     int  i ;

     if (length < 2) return text ; /* Nothing to flip */
     length-- ;
     for (i = 0 ; i < length; i++, length--)
     {
         c            = text[length] ;
         text[length] = text[i] ;
         text[i]      = c ; 
     }
     return text ;
}

static
TextSegment FlipItext (main_path, start_seg, end_seg,
                          rtrn_tail_seg, rtrn_next_seg) 
XmStringDirection  main_path; 
TextSegment        start_seg, end_seg,
                   *rtrn_tail_seg, *rtrn_next_seg ;
{
    TextSegment head_seg, tail_seg,
                next_seg,
                seg      = start_seg ;

    /* start_seg should never be NULL !!! */
    if (start_seg == (TextSegment)NULL)   return (TextSegment)NULL ;
        
    next_seg = seg ;
    if (seg->direction == main_path)
    {
        while (next_seg  != (TextSegment)NULL &&
               seg->next != (TextSegment)NULL &&
               seg->next != end_seg           &&
               seg->next->nest_level >= seg->nest_level)
        {
           if (seg->next->nest_level > seg->nest_level)
           {
              seg->next = FlipItext (main_path,
                                        seg->next, end_seg, 
       					&tail_seg, &next_seg ) ;      
              seg->next->prev = seg ;
              tail_seg->next = next_seg ;
              if (next_seg != (TextSegment)NULL) 
              {
                 next_seg->prev = tail_seg ;
                 seg = next_seg ;        
              }
              else 
                 seg = tail_seg ; /* This is so that *rtrn_tail_seg won't
                                     be NULL   (loop ends now) */
           }
           else /* seg->next->nest_level == seg->nest_level */
           {
              seg = next_seg = seg->next ;
           }
        }

        *rtrn_tail_seg = seg ;                /* becomes tail_segment */
        *rtrn_next_seg = (seg != (TextSegment)NULL) ? seg->next 
 						    : (TextSegment)NULL ;
        return start_seg ;                   /*  becomes head_segment */
    }
    else /* seg->direction is secondary path */
    {
        while (next_seg  != (TextSegment)NULL && 
               seg->next != (TextSegment)NULL &&
               seg->next != end_seg           &&
               seg->next->nest_level >= seg->nest_level)
        {
           if (seg->next->nest_level > seg->nest_level)
           {
              head_seg = FlipItext (main_path, 
                                       seg->next, end_seg,
				       &tail_seg, &next_seg) ;   
              seg->next = seg->prev ;
              seg->prev = tail_seg ;
              tail_seg->next = seg ;
              head_seg->prev = next_seg ;  /* No way for tail_seg or 
                                              head_seg to be NULL  (I hope) ! 
                                           */
              if (next_seg != (TextSegment)NULL) 
              {
                   /* This should really be next_seg->next but since we 
                      are in secondary direction the pointers will be 
                      reversed later
                   */
                   next_seg->prev = head_seg ;
                   seg = next_seg ;
              }
              else /* no next_seg - nothing to swap after flip */
              {
                   seg = head_seg ;   /* prepare for return */
              }
           }
           else /* seg->next->nest_level == seg->nest_level */
           {
              /* switch directions */
              next_seg = seg->next ;
              seg->next = seg->prev ;
              seg->prev = next_seg   ;
              if (next_seg != (TextSegment)NULL ) /* take care that head_seg */
                    seg = next_seg ;               /* should never be NULL   */ 
           }
        }

        if (next_seg != (TextSegment)NULL)
        {
            if (seg != next_seg)
                /* This should never happen */
                printf("INTERNAL ERROR: FlipItext: invalid condition\n");
            *rtrn_next_seg = next_seg->next ;    /*  next segment to process */
            /* switch pointers (wasn't done before) */
            /* (tail_seg is used as a temp variable) */
            tail_seg = next_seg->prev ;
            next_seg->prev  = next_seg->next ;
            next_seg->next = tail_seg ;
        }
        else 
            *rtrn_next_seg = (TextSegment)NULL ;
        *rtrn_tail_seg = start_seg ;         /*  becomes tail segment */
        return seg ;   			     /*  head_segment */
    }
}

/* AdjustItextAfterInsert converts the inserted string from Logical
   order to visual order and adjusts the nesting levels.
*/
static
void AdjustItextAfterInsert ( widget, start_line, start_segment,
                                      end_line,   end_segment )
DXmCSTextWidget    widget;
TextLine           start_line, end_line;
TextSegment        start_segment, end_segment;
{
     TextLine    line       = start_line ;
     TextSegment segment    = start_segment ;
     TextSegment head_seg, tail_seg, prev_seg, next_seg, previous_segment ;
     int         base_level = 0 ;  /* base nest_level for calculations */

     if (line == (TextLine) NULL)         /* started with empty CStext */
     {
        if ( (line = CSTextLines (widget)) == (TextLine) NULL)
                return ;                                /* still empty */
        start_line  = line ;
     }

     /* Get first segment for inverting and adjust the base level accordingly
     */
     if (start_segment == NULL) /* started with empty CStext */
         segment = line->segments ;        /* but it's no longer empty */
     else 
     {
         base_level = segment->nest_level ;
         segment = start_segment->next ;
     }

     /* Get 1st line that needs shuffling
     */
     while (segment == (TextSegment)NULL )
     {
         if ( (line = line->next) == end_line) 
                    return ;                 /* nothing to invert */
         segment = line->segments ;
     } 

     /* Adjust base_level if needed */
     if (start_segment == (TextSegment)NULL)
     {
        /* Empty cstext - assure that level 0 will correspond to Text Path
        */
        if ( segment->direction != CSTextPath( widget ))
              base_level++ ;

     }
     else 
     {
        /* Can't have different adjacent directions on same level!
        */
        if (start_segment->direction != segment->direction &&
            segment->nest_level == 0 ) /* relative level */
        {
            if (segment->direction == CSTextPath( widget ))
                  base_level-- ;          /* the base_level must'a been > 0 */
            else  base_level++ ;
        }
     }

     while (line != end_line &&              /* DON'T do end_line! */
            line != (TextLine) NULL &&       
            segment != end_segment )         /* finished ! */
     {
        TextSegment seg = segment;
        int   first_level  = segment->nest_level;

        /* balance the nest levels before flip
        */
        for ( ; seg != (TextSegment)NULL && seg != end_segment ;
                seg = seg->next )
        {
           if (seg->nest_level < first_level) 
           {
              /* build a new first segment */
              first_level = seg->nest_level ;
        
              SourceInsertSegment( widget, line, segment->prev,
 			           0, &segment );

              /* fill in details */
              _SourceAppendText  (  widget, line, segment,
  			            "", 0, 0, 
                                    seg->charset,
			            seg->direction,
			            seg->locale,
			            seg->nest_level + base_level);
  
           }
           seg->nest_level += base_level ;
           if (seg->length > 0 && seg->direction != CSTextPath( widget ))
           {
              /* flip to visual order 
	      */
              _flip_text( seg->text, seg->length) ;
           }
        }

/*  Kremer: Consider optimizing by some preliminary checks for 
    single segment on line which should NOT even try to flip
    if segment->next == end_segment ...
*/
        /* Reset previous segment: prev_seg == NULL means we 
         * are at start of line 
        */
        prev_seg =  segment->prev ;
        head_seg =  FlipItext (CSTextPath( widget ),
                                  segment, end_segment,
                                  &tail_seg, &next_seg) ;

        if (prev_seg ==  (TextSegment)NULL)  /* 1st seg on line */
        {
           head_seg->prev = (TextSegment)NULL ;   
           line->segments = head_seg ;
        } 
        else                                 /* starting at prev_seg */
        {
           head_seg->prev = prev_seg ;
           prev_seg->next = head_seg ;
        }

        if (tail_seg != (TextSegment)NULL)
        {
           tail_seg->next = next_seg ;
           if (next_seg != (TextSegment)NULL)  /* must'a reached end_seg */
           {
              if (next_seg  != end_segment)
              /* This should never get executed */
                  printf("INTERNAL ERROR: AdjustItextAfterInsert\n");
              next_seg->prev = tail_seg ;
              break ;                     /* All done  -  Leave the loop */
           }
        }
        /* Didn't finish. Advance to next non-empty line 
        */
        while (
                (line = line->next)        !=  (TextLine)NULL  &&
                line                       !=  end_line        &&
                (segment = line->segments) ==  (TextSegment)NULL
              ) ;
     }

     /* Remove the marker segments 
        and flatten out the nest-levels
     */
     end_line = line ;      /* save */
     line = start_line ;
     if (start_segment == (TextSegment)NULL)
        segment = line->segments ;
     else
     {
	previous_segment = start_segment->prev;
        segment = start_segment->next ;
        SourceRemoveSegment(widget, start_line, start_segment, True );
	if (previous_segment != (TextSegment)NULL)
	   segment = previous_segment->next;
     }
     
     while ( line != (TextLine)NULL ) 
     {
         if (segment != (TextSegment)NULL    &&
             segment == line->segments       &&
             segment->direction != CSTextPath( widget ) )
         {
            /* add 0 level segment to beginning of line to make life
               easy for the parsers
            */
            SourceInsertSegment( widget, line, segment->prev,
 			           0, &prev_seg );

            /* fill in details */
            _SourceAppendText  (  widget, line, prev_seg,
  			            "", 0, 0, 
                                    "ISO8859-1",
			            CSTextPath( widget ),
			            line->segments->locale,
			            0);
  
         }
         while (segment != (TextSegment)NULL)
         {
             if  ( line    == end_line &&
                   segment == end_segment
                 )
             {
                 SourceRemoveSegment(widget, line, end_segment, True );
		 break ;
             }

             if  (segment->direction != CSTextPath( widget )) {
/*
 * Deb 2/92 - Add call_segment_modified_callback to fix secondary text 
 * displaying incorrectly. This only affects RtoL performance.
 */
                  call_segment_modified_callback( widget, line, segment );
                  segment->nest_level = 1 ;
             }
             else segment->nest_level = 0 ;
             segment = segment->next ;
         }
         if  ( line == end_line) break;
         line = line->next ;
         if ( line != (TextLine)NULL )    /* NULL line = the end */
            segment = line->segments ;
     }
}
        
/*======================================================================*/
/* this routine takes the given compound string and calls the converter */
/* to insert the new text at the given position.  
 */
#ifdef _NO_PROTO
DXmCSTextStatus _DXmCSTextSourceInsertString( widget, start, string, alloc )
DXmCSTextWidget    widget;
DXmCSTextPosition  start;     /* where to copy text into the source */
XmString	   string;    /* text to be copied into allocation  */
int                alloc;     /* how much extra to alloc if create new segment*/
#else
DXmCSTextStatus _DXmCSTextSourceInsertString( DXmCSTextWidget    widget,
					      DXmCSTextPosition  start,
					      XmString		 string,
					      int                alloc )
#endif /* _NO_PROTO */
{
    DXmCSTextCvtContextRec context;
    DXmCSTextPosition dummy;
    DXmCSTextPosition length;
    DXmCSTextPosition new_cursor_pos;
    TextSegment segment;
    TextLine    line, first_dirty_line;
    DXmCSTextPosition line_offset, seg_offset;
    DXmCSTextStatus ret_status;

    XmStringContext strctx;
    XmStringCharSet charset;
    XmStringDirection dir;
    char *text;
    Boolean sep;

    TextLine    end_line ;
    TextSegment start_segment, 
		end_segment ;

    /* string == NULL is handled in DXmCvtCSToIText()
     */
    if( !CSTextEditable( widget ) )
    {
	return I_STR_EditDone;
    }

    if ( string )
    {
        context.widget      = (Widget)widget;
        context.line_cb     = (DXmSegmentProc) CvtFromCSLineCB;
        context.segment_cb  = (DXmSegmentProc) CvtFromCSSegmentCB;
        context.initialized = 0;

        /* don't anticipate that consecutive calls to the callback will be 
         * for text with the same attributes 
         */
        context.alloc_extra = alloc;

        /* find the start position and get ready to convert
         */
        SourceLocatePosition( widget, start, &context.line, 
					     &dummy, 
					     &context.segment, 
					     &context.offset );

	first_dirty_line = context.line;

        if (context.segment == NULL)
        {
	    /*
	     * we're inserting the string at the end of the line, so
	     * indicate to the converter where we are.
	     */
	    context.segment = _DXmCSTextGetLastSegment (context.line);

	    if (context.segment)
	    {
	        context.offset = context.segment->length;
	    }
        }

        length = CSTextLength( widget );

    /* After the string is inserted, it has to undergo logical to 
       visual order conversion and nesting level adjustments. For this
       purpose make sure that the string begins on a new segment and place 
       markers to indicate start and end segments.
       The marker segments are actual addresses of the segments, the 
       assumption being that these will not change and that the string will 
       always be sandwiched between these 2 segments.
    */
    segment = context.segment ;

    if( segment == (TextSegment)NULL )
    {
       /* A NULL context.segment indicates an empty line (or empty widget).
          Check for  previous segments and to obtain its vital statistics.
       */
       for (line = first_dirty_line; 
                   line            != (TextLine)NULL &&
                   line->segments  == (TextSegment)NULL ;
                               line = line->prev ) ;
       if ( line != (TextLine)NULL)
       {
          segment = _DXmCSTextGetLastSegment (line);
       }                   
    }

    /* If by now the segment is still NULL, then the widget must be empty.
    */
    if( segment != (TextSegment)NULL )   
    {
        TextSegment seg ;
        /*
        If segment is NULL    then insert at beginning of line.
        If segment is EMPTY   then insert after the segment.
        If offset  is 0 AND segment is NOT EMPTY
                              then insert BEFORE the segment.
        Otherwise             split the segment and insert at split point.
        */
        seg = context.segment ;         /* segment before insertion point */
        if (seg != (TextSegment)NULL && seg->length > 0)              
        {
           if (context.offset == 0)        /* Nothing to split */
              seg = context.segment->prev; /* might be NULL (Start of line) */
           else 
           {  
              SourceSplitSegment ( widget, 
    	                           context.line, 
   	                           context.segment, 
		                   context.offset );
           }
        }
        /* Mark start segment */
        SourceInsertSegment( widget,	
			     context.line, 
			     seg,   
			     0,
			     &start_segment );

        /* fill in details */
       _SourceAppendText  (  widget, 
			     context.line,
			     start_segment, 
			     "", 0, 0, 
                             segment->charset,
			     segment->direction,
			     segment->locale,
			     segment->nest_level );

        /* Mark end segment */
        SourceInsertSegment( widget,	
			     context.line, 
			     start_segment,
			     0,
			     &end_segment );

        /* fill in details */
       _SourceAppendText  (  widget, 
			     context.line,
			     end_segment, 
			     "!", 1, 2, 
			     segment->charset,
			     segment->direction,
			     segment->locale,
			     999 );
        /* This length is needed for calculations but will be removed after 
           the insertion.
        */
        CSTextLength (widget)++ ;
        context.line->length++ ;

        /* Reset context to new segment- hopefully all new additions
           will be added AFTER the context.segment 
        */
        context.segment = start_segment ;
        context.offset = 0;

    }
    else  start_segment = end_segment = (TextSegment)NULL ;

    end_line = (first_dirty_line == (TextLine)NULL) ? (TextLine) NULL
                                              : first_dirty_line->next ;
    /* A NULL end_segment means end of (possibly empty) line
       A NULL end_line    means end of (possibly empty) widget        
    */

        /* find character set - Should we take the first one with text ?? */

        if ( XmStringInitContext (&strctx, string) )
        {
           XmStringGetNextSegment (strctx, &text, &charset, &dir, &sep) ;
           context.charset = charset;
           XtFree (text);
           XmStringFreeContext (strctx);
        }

        DXmCvtCStoIText( &context, string );

	ret_status = context.status;
        XtFree( charset );

	/* notify output module here
	 */
	if (first_dirty_line == (TextLine) NULL)
		line = _DXmCSTextGetFirstLine(widget);
	else
		line = first_dirty_line;

        /* first adjust the newly inserted text */
        AdjustItextAfterInsert(widget,
                           line,       start_segment,
                           end_line,   end_segment);

/* Removed - Here is not suitable to notify the changes

        call_segment_modified_callback( widget,
        				line, 
        				_DXmCSTextGetFirstSegment(line) );
 */

        length = CSTextLength( widget ) - length;

        if( CSTextHasSelection( widget ) ) 
        {
	   /* adjust the selection endpoints
	    */
	    SourceAdjustRangeAfterInsert( start, 
				          length, 
				          &CSTextSelLeft( widget ),
				          &CSTextSelRight( widget ) );
	    DXmCSTextSetHighlight( widget,
			           CSTextSelLeft( widget ),
			           CSTextSelRight( widget ),
			           XmHIGHLIGHT_SELECTED );
        }

        /* adjust the cursor position
         */
        new_cursor_pos = CursorPos( widget );

        SourceAdjustRangeAfterInsert( start, 
				      length, 
				      &new_cursor_pos,
				      0 );
    } else {

	length = CSTextLength( widget );

        SourceLocatePosition( widget, start, &line, 
					     &line_offset,
					     &segment, 
					     &seg_offset );

/* Remove - Why does it notify the change which isn't actually done? 
	if ( line != (TextLine) NULL )
            call_segment_modified_callback( widget, line, segment );
 */

	new_cursor_pos = CursorPos(widget);
	ret_status = I_STR_EditDone;
    }

    DXmCSTextSetCursorPosition( widget, new_cursor_pos );

#ifdef DEBUG
_fix_length (widget, "_DXmCSTextSourceInsertString, end");
#endif

    return (ret_status);
}



/*======================================================================
 * this routine is called by the compound string to CSText internal      
 * structure converter.  Here are the rules:				
 * If we are in the middle of a segment, first split it.  Then:		
 * If we are at the beginning of a segment, try to add new stuff to     
 * existing allocation at end of previous segment.  If we are at the    
 * end of segment, try to add new stuff in existing allocation in that  
 * segment.  Else add an empty segment and put the new stuff there.  
 */

static void CvtFromCSSegmentCB( context )
    DXmCSTextCvtContext  context;
{
    DXmCSTextWidget    widget;
    TextSegment        prev, segment;
    TextLine	       line;
    XmStringCharSet    charset;
    XmStringDirection  direction;
    char	       *text;
    int		       nest_level;
    short	       locale;
    DXmCSTextPosition  offset, line_offset;
    int                char_length, byte_length, start_byte, end_byte;
    DXmCSTextStatus    ret_status;
    int		       byte_offset_start, byte_offset_end;
    Boolean	       need_new_segment;

    int		       app_char_length, app_byte_length, num_char_left;
    int		       test_length;
           
    widget = (DXmCSTextWidget)context->widget;

    if( !CSTextEditable( widget ) )
    {
	context->status = I_STR_EditError;
	return;
    }

    /* determine the total char length for context->text
    */
    I18nSegmentLength( (XtPointer)NULL,  /* I18nContext */
		       (char *)context->charset,
		       (XtPointer)context->text,
		       context->byte_length,
		       &char_length );

    /* take care of empty CSText
     */
    if( context->line == (TextLine)NULL )
    {
	/* source is empty, so create the first line
	*/

        /* First set the widget direction
        */
        if (!CSTextLines (widget) || !CSTextLines (widget)->segments)
        {
           /* We are entering a first segment
           */
           CSTextPath (widget)        = context->direction ;
           CSTextEditingPath (widget) = context->direction ;
        }

	context->line = _SourceInsertLine( widget, context->line );
    }

   num_char_left = char_length;
   offset  = 0;
   segment = context->segment;
   line	   = context->line;
   charset = context->charset;
   text    = context->text;
   locale  = context->locale;
   nest_level = context->nest_level;
   direction  = context->direction;

   /* loop until all char is processed
    */
   do 
   {
	/* see how many bytes we are going to append
	 */
	if ( num_char_left > MAX_SEGMENT_SIZE )
	     app_char_length = MAX_SEGMENT_SIZE;
	else
	     app_char_length = num_char_left;

	num_char_left -= app_char_length;

       /*
        * The CSTextLength-1 is to compensate for the "!" character added
        * in the _DXmCSTextSourceInsertString routine. The "!" character is
        * used a placeholder for the 'marker' segment during the logical to
        * virtual conversion. (See _DXmCSTextSourceInsertString routine for more
        * details). 
        *
        * If CSTextLength is used instead of CSTextLength-1, XmNmaxLength
        * does not work and only max-1 characters are allowed to be entered.
        */

	/* use test_length because the optimizer doesn't seem to like
	 * having the equation in the "if" statement.
	 */
	test_length = app_char_length + (CSTextLength(widget) - 1);
	
	if( test_length > CSTextMaxLength (widget) )
	{
	    /* too long.  we will take as many characters as fit
	     */
	    app_char_length = CSTextMaxLength (widget) - CSTextLength (widget);

	    /* set num_char_left to 0 to terminate loop
	     */
	    num_char_left = 0;
	}

	/* get the byte length of part that will fit
	*/
	I18nSegmentMeasure( (char *)NULL,  /* I18nContext */
			    (char *)charset,
			    (char *)text,
			    offset,	 /* character offset     */
			    app_char_length,  /* number of characters */
			    &start_byte,
			    &end_byte );

	app_byte_length = end_byte - start_byte;

	/* update the length before calling HandleData
	 */
	CSTextLength (widget) += app_char_length;
	line->length 	      += app_char_length;

	/* note: visual order conversion: can't flip here because we may 
           not be done with segment yet. Do it in the AdjustItextAfterInsert
           function along with the nest_level adjustment.
	*/

	need_new_segment = True;

	prev = segment;

        if( segment != (TextSegment)NULL )
        {
	    segment = SourceSplitSegment( widget, 
					  line, 
					  segment, 
					  context->offset + offset );

	    /* could be null if location was at end of line
	     */
	    if( segment != (TextSegment)NULL )
	    {
	        /* then split built a new segment (segment) so reset
	         * the previous segment pointer.
	         */
	        prev = segment->prev;
	    }

	    /* prev could be null if location is at beginning of line
	     */
	    if( prev != (TextSegment)NULL			  &&
	        prev->charset    == _Xm_cache_charset (charset, 
					   (int)strlen(charset))  &&
	        prev->direction  == direction		          &&
	        prev->locale     == locale 			  &&
	        prev->nest_level == nest_level )
	    {
	       /* enough space in prev?
	        */
	        if( app_byte_length <= prev->allocation -
				       prev->used_allocation )
	        {
		    /* write it in the prev segment 
		     */
		    ret_status = _SourceAppendText( widget, 
						    line,
						    prev, 
						    &(text[offset]), 
						    app_char_length,
						    app_byte_length, 
						    charset,
					            direction,
						    locale,
						    nest_level );

		    call_segment_modified_callback(widget, line, prev);

		    if( ret_status != I_STR_EditDone )
		    {
		        context->status = ret_status;
		        return;
		    }

		    need_new_segment = False;
	        }
	    }
        }
    
        if( need_new_segment )
        {
	    /* build a new segment.
	     *  allocate at least enough to hold this piece of text, plus
	     *  maybe some extra for later 
	     */
	    int alloc_length = app_byte_length + context->alloc_extra;

/* COMMENTS:
   The segments coming here have a 'relative' nest_level. If it's 
   coming from parse_stream, then it is relative to the first segment
   in the string we are parsing. Normally this string is only one
   character (i.e. what we just typed at the keyboard) so that it's 
   level is 0. 

   The orig. code here (the following #else clause) is wrong.
   If we have a series of segments with the same non-zero nest_level,
   then the nest_level will keep increasing (linearly). To fix, the 
   relative nest level has to be compared to that of the last segment 
   BEFORE the entire insert operation. 
  
   Since it's too messy figuring out the real level  here so do it 
   later during the logical->visual conversion. Therefore, nest_level should 
   not be updated here.
   */

             SourceInsertSegment( widget,	
			          line, 
			          prev,    
			          alloc_length,
			          &prev );

	    /* now write into the new segment, 
	     */
	    ret_status = _SourceAppendText( widget, 
				            line,
				            prev, 
				            &(text[offset]), 
				            app_char_length, 
				            app_byte_length, 
				            charset,
				            direction,
				            locale,
				            nest_level );

	    call_segment_modified_callback(widget, line, prev);
        }

	offset += app_byte_length;

	segment = prev;

    } while ( num_char_left > 0 );

    if( ret_status != I_STR_EditDone )
    {
	context->status = ret_status;
	return;
    }

    context->segment = prev;
    context->offset  = prev->length;

    context->status = I_STR_EditDone;
}


/*======================================================================
 * this routine is called by the compound string to CSText internal     
 * structure converter when a new line has to be added.  
 */
static void CvtFromCSLineCB( context )
    DXmCSTextCvtContext context;
{
    if( !CSTextEditable( context->widget ) )
    {
	context->status = I_STR_EditError;
	return;
    }

    if( context->initialized == 0 )
    {
	context->initialized = 1;
	return;
    }

    if (CSTextEditMode (context->widget) == XmSINGLE_LINE_EDIT)
    {
	return;
    }

    context->segment = SourceSplitSegment( context->widget, 
					   context->line, 
					   context->segment, 
					   context->offset );  

    context->status = SourceSplitLine( context->widget, 
				       context->line, 
				       context->segment, 
				       &context->line );  

    context->offset  = 0;

    return;
}


/*======================================================================
* get the requested selection for the insert selection and replace the
* primary selection with the result
*/
static void InsertSelection( w, closure, seltype, type, value, length, format )
Widget w;
XtPointer closure;
Atom *seltype;
Atom *type;
char *value;
int *length;
int *format;
{
    XmAnyCallbackStruct cb;
    Boolean *select_done   = (Boolean *)closure;
    DXmCSTextWidget widget = (DXmCSTextWidget) w;
    DXmCSTextPosition left, right;
    DXmCSTextPosition cursorPos;

    if (!value) 
    {
        select_done[0] = True;
        return;
    }

    /* Don't do replace if there is no text to add 
    */
    if (*value == (char) NULL || *length == 0)
    {
       XtFree( value );
       select_done[0] = True;
       return;
    }

    if ( !DXmCSTextGetSelectionInfo( (Widget) widget, &left, &right) )
    {

       XBell(XtDisplay(widget), 0);

       XtFree(value);
       select_done[0] = True;
       select_done[1] = False;
       return;
    }

    DXmCSTextDisableRedisplay ( widget, True );

    if ( _DXmCSTextSourceReplaceString( (DXmCSTextWidget)widget,
                           left, right, (XmString)value ) != I_STR_EditDone ) 
    {
       if (CSTextVerifyBell(widget)) XBell(XtDisplay(widget), 0);
       select_done[1] = False;
       return;

    } else {
      /*
       * Call ValueChanged Callback to indicate that
       * text has been modified.
       */
       cb.reason = XmCR_VALUE_CHANGED;
       cb.event  = NULL;

       XtCallCallbackList( (Widget)widget, (XtCallbackList)ValueCB (widget), (XtPointer)&cb);
       select_done[1] = True;
    }

    DXmCSTextEnableRedisplay ( widget );

    XtFree(value);

    select_done[0] = True;
}
  

/*======================================================================
* Converts requested target of insert selection.
*/
static Boolean ConvertInsertSelection( w, selection, type,
                                       value, length, format )
Widget w;
Atom *selection, *type;
XtPointer *value;
int *length, *format;
{
    XtAppContext app = XtWidgetToApplicationContext(w);
    XSelectionRequestEvent * req_event;
    static unsigned long old_serial = 0;
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes;
    unsigned char *prop;
    Boolean select_done[2];
    CSTextInsertPair *pair;

    select_done[0] = False;
    select_done[1] = False;

    req_event = XtGetSelectionRequest(w, *selection, NULL);

    /* Work around for intrinsics selection bug 
    */
    if( old_serial == req_event->serial )
    {
	return False;
    }
    old_serial = req_event->serial;

    XGetWindowProperty( req_event->display, 
			req_event->requestor,
			req_event->property, 
			0L, 
			10000000, 
			False,
			AnyPropertyType, 
			&actual_type, 
			&actual_format,
			&nitems, 
			&bytes, 
			&prop);

    pair = (CSTextInsertPair *)prop;

    /*
    * Make selection request to replace the primary selection
    * with the insert selection.
    */
    XtGetSelectionValue( w, 
			 pair->selection, 
			 pair->target,     /* need to convert ++++++*/
    			 (XtSelectionCallbackProc)InsertSelection,
			 (XtPointer)select_done, 
			 CurrentTime);
    /*
    * Make sure the above selection request is completed
    * before returning from the convert proc.
    */
    for (;;) 
    {
       XEvent event;

       if( select_done[0] )
	  break;
       XtAppNextEvent( app, &event );
       XtDispatchEvent( &event );
    }

    *type = XInternAtom( XtDisplay(w), "INSERT_SELECTION", False );
    *format = 8;
    *value = NULL;
    *length = 0;

    XtFree( (char *)prop );

    return( select_done[1] );
}
 

/*======================================================================
 * the convert selection routine - this routine is also called by the
 * input module to do secondary selection
 */
#ifdef _NO_PROTO
Boolean _DXmCSTextSrcConvert( w,
			      seltype,
			      desiredtype,
			      type,
			      value,
			      length,
			      format )
Widget w;
Atom *seltype;
Atom *desiredtype;
Atom *type;
XtPointer *value;
unsigned long *length;
int *format;
#else
Boolean _DXmCSTextSrcConvert( Widget  w,
			      Atom    *seltype,
			      Atom    *desiredtype,
			      Atom    *type,
			      XtPointer *value,
			      unsigned long *length,
			      int     *format )
#endif /* _NO_PROTO */
{
    DXmCSTextPosition left, right;
    DXmCSTextWidget widget;
    Atom *a;
    Atom target;
    XmString i_str;
    XtPointer c_ptr;
    DXmCSTextInputData data;
    Arg args[5];
    int status;
    Atom XA_C_STRING = XInternAtom(XtDisplay(w), "DEC_COMPOUND_STRING", FALSE);
    Atom XA_DDIF = XInternAtom(XtDisplay(w), "DDIF", FALSE);
    Atom XA_C_TEXT = XInternAtom(XtDisplay(w), "COMPOUND_TEXT", FALSE);
    Atom XA_TARGETS = XInternAtom( XtDisplay(w), "TARGETS", FALSE );
    Atom XA_INSERT_SELECTION = XInternAtom( XtDisplay(w), "INSERT_SELECTION", False);
    Atom XA_DELETE = XInternAtom(XtDisplay(w), "DELETE", False);
    Atom MOTIF_DROP = XmInternAtom(XtDisplay(w), "_MOTIF_DROP", False);

    if ( *seltype == XA_PRIMARY )
    {
	widget = (DXmCSTextWidget) w;
	data   = InputData (widget);
	
	if( !CSTextHasSelection( widget ) ) 
	{
	    return False;
	}

	left  = CSTextSelLeft( widget );
	right = CSTextSelRight( widget );

    }else{
    if ( *seltype == XA_SECONDARY ) 
    {
	widget = (DXmCSTextWidget) w;
	data   = InputData (widget);

	if ( !data->hasSel2 )
	{
	    return False;
	}

	left  = data->sel2Left;
	right = data->sel2Right;

    }else{
    if ( *seltype == MOTIF_DROP) 
    {
	XtSetArg(args[0], XmNclientData, &c_ptr);
	XtGetValues(w, args, 1);
	widget = (DXmCSTextWidget) c_ptr;
	data   = InputData (widget);

	if (!CSTextHasSelection(widget)) 
	    return False;

	left  = CSTextSelLeft( widget );
	right = CSTextSelRight( widget );

    }else{
	/* must be either primary or secondary selection
	*/
	return False;
    }}}

    if( *desiredtype == XA_TARGETS ) 
    {
	*format = 32;

	a = (Atom *)XtMalloc((unsigned) (6*sizeof(Atom)));
	*value = (XtPointer)a;
	*a++ = XA_STRING;
	*a++ = XA_C_STRING;
	*a++ = XA_C_TEXT;
	*a++ = XA_DELETE;
	*a++ = XA_INSERT_SELECTION;
	*a   = XA_TARGETS;

	*length = (6*sizeof(Atom)) >> 2; /* convert to work count */
	*type = XA_ATOM; /* or do we need a "list of atoms" type? */
	return True;
    }

    if( *desiredtype == XA_INSERT_SELECTION ) 
    {
	return(
	ConvertInsertSelection( w, 
				seltype, 
				type, 
				value, 
				length, 
				format )
	);
    }

    if( *desiredtype == XA_DELETE ) 
    {
	_DXmCSTextSourceReplaceString ( widget, left, right, (XmString) NULL );
	if ( *seltype == MOTIF_DROP) 
	    *type = XmInternAtom(XtDisplay(w), "NULL", False);
	else
	    *type = (Atom)NULL;
        *value = NULL;
        *length = 0;
        *format = 8;
	return True;
    }

    if( _DXmCSTextSourceReadString( widget, 
				    left,
				    right,
				    &i_str ) != I_STR_EditDone )
    {
	return False;
    }

    status = DXmCvtStatusFail;

    if ( *desiredtype == XA_STRING ) 
    {
        *type = XA_STRING;	
        *format = 8;
	*value = (XtPointer)DXmCvtCStoFC( i_str, (long *)length, (long *) &status );
    }

    if( *desiredtype == XA_DDIF) 
    {
	*type = XA_DDIF;
	*format = 8;
	*value = (XtPointer)DXmCvtCStoDDIF( i_str, (long *) length, (long *) &status );
    }

    if( *desiredtype == XA_C_STRING ) 
    {
	*type = XA_C_STRING;
	*format = 8;
	*value = (XtPointer)i_str;
	*length = XmStringLength( *value );
	status = DXmCvtStatusOK;
    }

    if( *desiredtype == XA_C_TEXT ) 
    {
	*type = XA_C_TEXT;
	*format = 8;
	*value = (XtPointer)XmCvtXmStringToCT( i_str );
	*length = strlen( *value );
	status = DXmCvtStatusOK;
    }

    XmStringFree (i_str );

    return( status == DXmCvtStatusOK );
}


/*======================================================================*/
/* the lose selection routine
*/
static void LoseSelection( w, selection )
Widget w;
Atom *selection;  /* not used */
{
    DXmCSTextWidget widget = (DXmCSTextWidget) w;

    DXmCSTextDisableRedisplay( widget, FALSE );

    /* unhighlight the current selection, if any
    */
    if( CSTextHasSelection( widget ) )
    {
	DXmCSTextSetHighlight( widget, 
			      CSTextSelLeft( widget ), 
			      CSTextSelRight( widget ),
			      XmHIGHLIGHT_NORMAL );

	CSTextHasSelection( widget ) = FALSE;
    }

    DXmCSTextEnableRedisplay( widget );

    return;
}

static void _set_highlight(widget, old_left, old_right, new_left, new_right)
DXmCSTextWidget widget;
DXmCSTextPosition old_left;
DXmCSTextPosition old_right;
DXmCSTextPosition new_left;
DXmCSTextPosition new_right;
{
    struct {
	DXmCSTextPosition start;
	DXmCSTextPosition end;
	XmHighlightMode mode;
    } area1, area2;

    if (new_left <= old_left && old_right <= new_right) {
	area1.start = new_left;
	area1.end   = old_left;
	area1.mode  = XmHIGHLIGHT_SELECTED;
	area2.start = old_right;
	area2.end   = new_right;
	area2.mode  = XmHIGHLIGHT_SELECTED;
    } else if (new_left <= old_left && old_left <= new_right
		&& new_right <= old_right) {
	area1.start = new_left;
	area1.end   = old_left;
	area1.mode  = XmHIGHLIGHT_SELECTED;
	area2.start = new_right;
	area2.end   = old_right;
	area2.mode  = XmHIGHLIGHT_NORMAL;
    } else if (new_right <= old_left) {
	area1.start = new_left;
	area1.end   = new_right;
	area1.mode  = XmHIGHLIGHT_SELECTED;
	area2.start = old_left;
	area2.end   = old_right;
	area2.mode  = XmHIGHLIGHT_NORMAL;
    } else if (old_left <= new_left && new_left <= old_right
		&& old_right <= new_right) {
	area1.start = old_left;
	area1.end   = new_left;
	area1.mode  = XmHIGHLIGHT_NORMAL;
	area2.start = old_right;
	area2.end   = new_right;
	area2.mode  = XmHIGHLIGHT_SELECTED;
    } else if (old_left <= new_left && new_right <= old_right) {
	area1.start = old_left;
	area1.end   = new_left;
	area1.mode  = XmHIGHLIGHT_NORMAL;
	area2.start = new_right;
	area2.end   = old_right;
	area2.mode  = XmHIGHLIGHT_NORMAL;
    } else if (old_right <= new_left) {
	area1.start = old_left;
	area1.end   = old_right;
	area1.mode  = XmHIGHLIGHT_NORMAL;
	area2.start = new_left;
	area2.end   = new_right;
	area2.mode  = XmHIGHLIGHT_SELECTED;
    }

    if (area1.start < area1.end)
	DXmCSTextSetHighlight(widget, area1.start, area1.end, area1.mode);
    if (area2.start < area2.end)
	DXmCSTextSetHighlight(widget, area2.start, area2.end, area2.mode);
}


/*======================================================================*/
/* the set selection routine
*/
#ifdef _NO_PROTO
void _DXmCSTextSetSelection( widget, left, right, time )
DXmCSTextWidget   widget;
DXmCSTextPosition left;
DXmCSTextPosition right;		
Time		 time;
#else
void _DXmCSTextSetSelection( DXmCSTextWidget   widget,
			     DXmCSTextPosition left,
			     DXmCSTextPosition right,
			     Time	       time )
#endif /* _NO_PROTO */
{
    DXmCSTextInputData data;
    Boolean had_selection = CSTextHasSelection(widget);

    data = InputData (widget);

    if ( left < 0 ) left = right = 0;

    /* should we take the selection?
    */
    if( left < right )
    {
	/* take ownership of the selection if we don't already have it
	*/
	if( !CSTextHasSelection( widget ) )
	{
	    CSTextHasSelection( widget ) = XtOwnSelection( (Widget)widget, 
						   XA_PRIMARY, 
						   time, 
						   (XtConvertSelectionProc)_DXmCSTextSrcConvert, 
						   (XtLoseSelectionProc) LoseSelection, 
						   (XtSelectionDoneProc) NULL);
	}
    }else{
	/* shouldn't take selection, did we have it coming in?
	*/
	if( CSTextHasSelection( widget ) )
	{
	    XtDisownSelection( (Widget)widget, XA_PRIMARY, time );

	    CSTextHasSelection( widget ) = FALSE;
	}
    }

    /* set/unset highlight */
    _set_highlight(widget,
		   had_selection ? CSTextSelLeft(widget) : 0,
		   had_selection ? CSTextSelRight(widget) : 0,
		   CSTextHasSelection(widget) ? left : 0,
		   CSTextHasSelection(widget) ? right : 0);

    if( CSTextHasSelection( widget ) )
        data->pendingoff = False;

    CSTextSelLeft(  widget ) = left;
    CSTextSelRight( widget ) = right;

    return;
}
     

#define MAX_CS_FRAGMENTS 10
#define SCAN_PREV_TO_NEXT 0
#define SCAN_NEXT_TO_PREV 1


static 
void AddLineToCntx (context, seg, main_path)
DXmCSTextCvtContext context ;
TextSegment         seg ;
XmStringDirection   main_path ;
{

   TextSegment tail_seg, next_seg ;
 
   /*  First reorder the segments 
   */
   if (seg != (TextSegment)NULL )
   {
      seg = FlipItext( main_path, seg, (TextSegment) NULL, 
                          &tail_seg, &next_seg) ;
   }

   if (next_seg != (TextSegment) NULL)
   {
        /* This should never really happen but...
           it might happen if nest levels are missing
        */
   }

   for (; seg != (TextSegment) NULL ; seg = seg->next )
   {   
       context->text        = seg->text ;
       context->byte_length = seg->length;
       context->charset     = seg->charset;
       context->direction   = seg->direction;
       context->locale      = seg->locale;
       context->nest_level  = seg->nest_level;

       /* Add the flipped text to context. We don't want to change the text 
          within the CSText so don't forget to re-flip after adding 
          to context.
       */
       /* (...flipping to an allocated buffer and freeing might be cheaper) */
       if (main_path != context->direction )
           _flip_text( context->text, context->byte_length) ;   /* Flip...  */
       (*context->segment_cb) (context);                  
       if (main_path != context->direction )
           _flip_text( context->text, context->byte_length) ;   /* and re-flip */
   }

}

TextSegment bld_new_seg(old)
TextSegment old;
{
     TextSegment new_seg = (TextSegment) XtMalloc(sizeof(TextSegmentRec)) ;
     new_seg->next = new_seg->prev = (TextSegment)NULL ;
     new_seg->text        = (char *) NULL ;
     new_seg->length      = 0 ;
     new_seg->charset     = old->charset;     
     new_seg->direction   = old->direction;
     new_seg->locale      = old->locale;
     new_seg->nest_level  = 1 ;   /* Always level 1 */

     return new_seg ;
}

TextSegment bld_nest_level0(main_path)
XmStringDirection   main_path ;
{
     TextSegment new_seg = (TextSegment) XtMalloc(sizeof(TextSegmentRec)) ;
     new_seg->next = new_seg->prev = (TextSegment)NULL ;
     new_seg->text        = (char *) NULL ;
     new_seg->length      = 0 ;
     new_seg->charset     = 0 ;
     new_seg->direction   = main_path ;
     new_seg->locale      = 0 ;
     new_seg->nest_level  = 0 ;

     return new_seg ;
}

/*======================================================================
 * this routine reads from the source data structure, and calls the     
 * converter to produce a compound string
 */
#ifdef _NO_PROTO
DXmCSTextStatus _DXmCSTextSourceReadString( widget, start, end, string )
DXmCSTextWidget   widget;
DXmCSTextPosition start;
DXmCSTextPosition end;
XmString         *string;
#else
DXmCSTextStatus _DXmCSTextSourceReadString( DXmCSTextWidget   widget,
					    DXmCSTextPosition start,
					    DXmCSTextPosition end,
					    XmString          *string )
#endif /* _NO_PROTO */
{
/*  This routine was modified to support visual order (and nesting)
    as follows:

     For each IText LINE between start and end locations
     {
         For each segment in LINE not exceeding start and end locations
         {
           Add an entry into a segment list (Itext segments)
         }

         Send the seg list to the AddLineToCntx routine which converts the
         segments to logical order and adds them to the context.
     }

    NOTE:
       This routine must traverse ALL segments (not just those containing 
       characters) so as not to miss some levels. Therefore we can't use the 
       regular navigation routines which rely on character positions.
*/
    TextSegment   seg1 = (TextSegment) NULL, /* ptr to 1st segment in temp list */
                  seg  = (TextSegment) NULL; /* pointer to current Segment */

    DXmCSTextCvtContextRec context;
    TextLocationRec location, end_location, start_location;
    TextLine line;
    TextSegment segment,next_seg;
    int start_byte_offset, end_byte_offset, char_length;
    int state, offset ;
#define BEFORE_TEXT  0
#define WITHIN_TEXT  1
#define AFTER_TEXT   2
    

    *string = (XmString)NULL;

    if( CSTextLength (widget) == 0 ){	/* When CSText is empty */
	*string = XmStringCreateSimple("");
	return I_STR_EditDone;
    }

    if( start >= CSTextLength (widget) || start > end )
    {
	return I_STR_EditError;
    }

    /* Find location of start and end positions. 
    */
    _DXmCSTextSourceLocate( widget, start, &start_location );
    _DXmCSTextSourceLocate( widget, end, &end_location );

    if( start_location.line == (TextLine) NULL )
    {
	return I_STR_EditDone;
    }

    context.status = I_STR_EditDone;

    DXmCvtITextToCSInit( &context );


    /* This code assumes that each line begins at nest-level 0
       We need to process all the segments in the lines between 
       start_location.line and end_location.line so that we don't
       miss nesting levels. 
       If the current segment is BEFORE start_location.segment or
                                 AFTER  end_location.segment
       then emit empty segments (no text) 
       If the current segment is WITHIN the requested text 
       then emit full segments
    */

    state = BEFORE_TEXT ;

    for(line = start_location.line; line != (TextLine)NULL; line = line->next)
    {
        seg1 = seg = bld_nest_level0( CSTextPath(widget)) ;

        for (segment = line->segments; 
             ;          /* break is done in loop */
             segment = segment->next)
        {
            /*  start_location.segment might be NULL (end of line)
            */
            if (line == start_location.line && 
                segment == start_location.segment)
               state = WITHIN_TEXT ;

            /* Now break if NULL
            */
            if (segment == (TextSegment)NULL) break ;

            /* Check if direction changed (implies level change too)
            */
            if (seg->direction  != segment->direction &&
                seg->nest_level != 0 )              /* one's enough */
            {
               seg->next = bld_nest_level0( CSTextPath(widget)) ;
               seg->next->prev  = seg ;
 	       seg              = seg->next ;
            }
 
            /* Empty? Now that direction info is recorded - ignore
            */
            if (segment->length == 0) continue ;

            /* Add a segment to the segment list
            */
     	    seg->next        = bld_new_seg(segment) ;
            seg->next->prev  = seg ;
 	    seg              = seg->next ;
	    
            if (state == WITHIN_TEXT)
            {
               offset = 0 ;                    /* defaults - unless start or end */
               char_length = segment->length ;
               if (segment == start_location.segment)
               {
	            char_length = start_location.segment->length - 
 			          start_location.offset;
                    offset = start_location.offset ;
	       }
               if (segment == end_location.segment)
               {
		    char_length = end_location.offset - 
				  offset ;
                    state = AFTER_TEXT ;
	       }

	       I18nSegmentMeasure( (char *)NULL,  /* I18nContext */
				   (char *)segment->charset,
				   (char *)segment->text,
				   offset,          /* character offset */
				   char_length,     /* number of characters */
				   &start_byte_offset, /* of text to read   */
				   &end_byte_offset );      
                
	       seg->text        = segment->text + start_byte_offset;
	       seg->length      = end_byte_offset - start_byte_offset;
            }
        }
        /* Add the line to context 
        */
        /* Convert Line to CS */
        if (seg1) 
        {
           /* End off at level 0
           */
           if (seg->nest_level != 0) 
           {
               seg->next = bld_nest_level0( CSTextPath(widget)) ;
               seg->next->prev  = seg ;
 	       seg              = seg->next ;
           }
           AddLineToCntx(&context, seg1, CSTextPath (widget )) ;
           /* Free the segment list  */
           for (seg = seg1; seg != (TextSegment) NULL ; )
	   {
		next_seg = seg->next;
                XtFree((char *)seg) ;
		seg = next_seg;
	   }
           seg1 = (TextSegment) NULL;
        }
        /* reached end of last line  ? */
        if (line == end_location.line )
        {
	    DXmCvtITextToCSEnd( &context );
	    *string = context.stream;
	    return( context.status );
        }

        (*context.line_cb)( &context );
    }  /* loop for each line */

    /* get here because the last postion is not the last position in
    * the last line but one more...
    */

    if (seg1) 
    {
       if (seg->nest_level != 0) 
       {
          seg->next = bld_nest_level0( CSTextPath(widget)) ;
          seg->next->prev  = seg ;
 	  seg              = seg->next ;
       }
       AddLineToCntx(&context, seg1, CSTextPath (widget )) ;
       /* Free the segment list  */
       for (seg = seg1; seg != (TextSegment) NULL ; )
       {
	    next_seg = seg->next;
            XtFree((char *)seg) ;
	    seg = next_seg;
       }
       seg1 = (TextSegment) NULL;
    }
    DXmCvtITextToCSEnd( &context );
    *string = context.stream;
    return( context.status );
}


/*======================================================================
 * this routine replaces a given portion of the source with the given new
 * text
 */
#ifdef _NO_PROTO
DXmCSTextStatus _DXmCSTextSourceReplaceString( widget, start, end, string )
DXmCSTextWidget widget;
DXmCSTextPosition start;  /* the start position in the source to be replaced */
DXmCSTextPosition end;    /* the end position in the part to be replaced     */
XmString string;	 /* the text that will replace the specified part   */
#else
DXmCSTextStatus _DXmCSTextSourceReplaceString( DXmCSTextWidget   widget,
					       DXmCSTextPosition start,
					       DXmCSTextPosition end,
					       XmString          string )
#endif /* _NO_PROTO */
{
    DXmCSTextStatus ret_status;
    TextLine  line;
    TextSegment segment;
    DXmCSTextPosition seg_offset, line_offset;
    XmString		newstring;	/* potentially modified by callback */
    DXmCSTextPosition	newstart, newend;

    if( !CSTextEditable( widget ) )
    {
	return I_STR_EditError;
    }

    newstring = string;
    newstart = start;
    newend = end;

    if( !call_source_modify_callback( widget, &newstart, &newend, &newstring ) )
    {
	if (newstring != string)
	    XmStringFree(newstring);
	return I_STR_EditError;
    }

    /* remove all existing data from the source	
    */
    if (newstart != newend)
	SourceRemove( widget, newstart, newend );

    ret_status = _DXmCSTextSourceInsertString( widget, newstart, newstring,
					SEGMENT_EXTRA_ALLOCATION ); 

    if (newstring != string)
	XmStringFree(newstring);

    call_source_changed_callback(widget);

    if( ret_status != I_STR_EditDone)
    {
	return ret_status;
    }

    return ret_status;
}

/* save for possible future use
static DXmCSTextPosition MirrorPosition( widget, pos, segment, start, auxpos1,
								     auxpos2)
DXmCSTextWidget widget;
DXmCSTextPosition pos, start, auxpos1, auxpos2;
TextSegment segment;
{
    if (segment->direction == CSTextPath (widget))
	return pos;
    else
	return (MAX(start, auxpos1) + MIN(start + segment->length, auxpos2)
								- 1 - pos);
}
*/

/*=======================================================================*/
/* This routine finds the position that is count characters from the     */
/* start position in the given direction.  It returns the new position   */
/* Note that count may put you						 */
/* outside the current limit of the text, in which case the first (last) */
/* position is returned depending on the direction 
 */
#ifdef _NO_PROTO
void _DXmCSTextScanPositions( widget, 
		              position,
		              direction, 
		              count,
		              new_position 
		              )

DXmCSTextWidget widget;
DXmCSTextPosition position;	 /* the start position */
DXmCSTextScanDirection direction; /* the direction to go */
int count;			 /* number of positions to scan */
DXmCSTextPosition *new_position;	 /* (output) new position */
#else
void _DXmCSTextScanPositions( DXmCSTextWidget        widget,
			      DXmCSTextPosition      position,
			      DXmCSTextScanDirection direction,
			      int 		     count,
			      DXmCSTextPosition      *new_position )
#endif /* _NO_PROTO */
{
    /* are we scanning left or right?
     */
    if( direction == DXmsdRight )
    {
	*new_position = position + count;

	/* don't go beyond the end
	 */
	if( *new_position > CSTextLength (widget) ) 
	    *new_position = CSTextLength (widget);
    }else{

	*new_position = position - count;

	/* don't go beyond the end
	 */
	if( *new_position < 0 ) 
	    *new_position = 0;
    }

    return;
}


/*======================================================================
 *
 * This routine determines if the character in the source segment at given
 * offset represents a scan break, which is in fact done in I18n
 */

#ifdef _NO_PROTO
Boolean _DXmCSTextIsScanBreak( widget, location, scan_direction, scan_type )
Widget widget;
TextLocation location;
int	scan_direction;
I18nScanType scan_type;
#else
Boolean _DXmCSTextIsScanBreak( Widget	    widget,
			       TextLocation location,
			       int	    scan_direction,
			       I18nScanType scan_type )
#endif /* _NO_PROTO */
{
    char *temp_char;
    TextSegment this_segment;

    char  *next_charset;
    char  *next_char;
    char  *prev_charset;
    char  *prev_char;
    char  *this_charset;
    char  *this_char;

    TextLocationRec prev_location;
    TextLocationRec next_location;

    TextSegment  prev_segment;
    TextSegment  next_segment;

    int  start_byte_offset;
    int  end_byte_offset;
    int  num_byte;

    Boolean is_break;

    this_segment = location->segment;

    if ( location->end_of_line )
    {
	this_char = (char *) XtMalloc ( sizeof (char) * 2 );

        this_char[0] = '\n';
        this_char[1] = '\0';

        this_charset = "ISO8859-1";

    } else {

        if ( this_segment == (TextSegment)NULL )  return True;

        /* get this char's info
         */
        temp_char = this_segment->text;
        
        I18nSegmentMeasure( (char *)NULL,             /* I18nContext      */
        		    (char *)this_segment->charset,/* this segment     */
        		    (char *)temp_char,	      /* this text        */
        		    location->offset,	      /* character offset */
        		    1,  		      /* number of characters */
        		    &start_byte_offset,	      /* start byte offset */
        		    &end_byte_offset );	      /* end byte offset */
        
        num_byte = end_byte_offset - start_byte_offset;
        
        this_char = (char *) XtMalloc ( sizeof (char) * num_byte + 1 );
        
        bcopy ( &(temp_char[start_byte_offset]), this_char, num_byte );
        
        this_char[num_byte] = '\0';
        
        this_charset = (char *) this_segment->charset;
     }

    /* if scanning from previous to next (or l_to_r), set don't care for
     * prev_char and prev_charset
     */
    if ( scan_direction == SCAN_PREV_TO_NEXT )
    {
	prev_char    = NULL;
	prev_charset = NULL;
    } else {
	/* get previous char info
	 */
	_DXmCSTextSourceLocate((DXmCSTextWidget) widget, location->position - 1, &prev_location);

	if ( prev_location.end_of_line )
	{
	    prev_char = (char *) XtMalloc ( sizeof (char) * 2 );

	    prev_char[0] = '\n';
	    prev_char[1] = '\0';

	    prev_charset = "ISO8859-1";
	} else {
	    prev_segment = prev_location.segment;

	    temp_char = prev_segment->text;

	    I18nSegmentMeasure( (char *)NULL,	      /* I18nContext       */
				(char *)prev_segment->charset, /* prev char segment */
				(char *)temp_char,    /* prev char text    */
				prev_location.offset, /* character offset  */
				1,		      /* number of characters*/
				&start_byte_offset,   /* start byte offset */
				&end_byte_offset );   /* end byte offset   */

	    num_byte = end_byte_offset - start_byte_offset;

	    prev_char = (char *) XtMalloc ( sizeof (char) * num_byte + 1 );

            bcopy ( &(temp_char[start_byte_offset]), prev_char, num_byte );

	    prev_char[num_byte] = '\0';

            prev_charset = (char *) prev_segment->charset;
	}
    }


    /* if scanning from next to previous (or r_to_l), set don't care for
     * next_char and next_charset
     */
    if ( scan_direction == SCAN_NEXT_TO_PREV )
    {
	next_char    = NULL;
	next_charset = NULL;

    } else {
	/* get next char info
	 */
	_DXmCSTextSourceLocate((DXmCSTextWidget) widget, location->position + 1, &next_location);

	if ( next_location.end_of_line )
	{
	    next_char = (char *) XtMalloc ( sizeof (char) * 2 );

            next_char[0] = '\n';
            next_char[1] = '\0';

	    next_charset = "ISO8859-1";
	} else {
            next_segment = next_location.segment;

            temp_char = next_segment->text;

            I18nSegmentMeasure( (char *)NULL,         /* I18nContext       */
			    	(char *)next_segment->charset, /* next char segment */
			    	(char *)temp_char,    /* next char text    */
			    	next_location.offset, /* character offset  */
			    	1,  		      /* number of characters*/
			    	&start_byte_offset,   /* start byte offset */
			    	&end_byte_offset );   /* end byte offset   */

	    num_byte  = end_byte_offset - start_byte_offset;

	    next_char = (char *) XtMalloc ( sizeof (char) * num_byte + 1 );

            bcopy ( &(temp_char[start_byte_offset]), next_char, num_byte );

	    next_char[num_byte] = '\0';

            next_charset = (char *) next_segment->charset;
        }
    }



    /* call I18nIsScanBreak() to determine if it is a break
     */

    is_break = I18nIsScanBreak ( (I18nContext)	NULL,
				 (char *)	prev_charset,
				 (XtPointer)	prev_char,
				 (char *)	this_charset,
				 (XtPointer)	this_char,
				 (char *)	next_charset,
				 (XtPointer)	next_char,
				 (int)		scan_direction,
				 (I18nScanType)	scan_type);

    /* clean up memory
     */
    if ( prev_char )
	XtFree ( prev_char );

    if ( this_char )
	XtFree ( this_char );

    if ( next_char )
	XtFree ( next_char );

    return ( is_break );
}


/*
 * This routine determines if the character in the source segment at given
 * offset represents whitespace
 */

#ifdef _NO_PROTO
Boolean _DXmCSTextIsWhiteSpace( widget, location )
Widget widget;
TextLocation location;
#else
Boolean _DXmCSTextIsWhiteSpace( Widget 	     widget,
				TextLocation location )
#endif /* _NO_PROTO */
{

    char *temp_char;
    TextSegment this_segment;
    char  *this_char;
    char  *this_charset;
    Boolean is_white;

    int  start_byte_offset;
    int  end_byte_offset;
    int  num_byte;

    this_segment = location->segment;

    if ( location->end_of_line )
    {
	this_char = (char *) XtMalloc ( sizeof (char) * 2 );

        this_char[0] = '\n';
        this_char[1] = '\0';

    } else {

        if ( this_segment == (TextSegment)NULL )  return True;

        /* get this char's info
         */
        temp_char = this_segment->text;
        
        I18nSegmentMeasure( (char *)NULL,             /* I18nContext      */
        		    (char *)this_segment->charset,     /* this segment     */
        		    (char *)temp_char,	      /* this text        */
        		    location->offset,	      /* character offset */
        		    1,  		      /* number of characters */
        		    &start_byte_offset,	      /* start byte offset */
        		    &end_byte_offset );	      /* end byte offset */
        
        num_byte = end_byte_offset - start_byte_offset;
        
        this_char = (char *) XtMalloc ( sizeof (char) * num_byte + 1 );
        
        bcopy ( &(temp_char[start_byte_offset]), this_char, num_byte );
        
        this_char[num_byte] = '\0';
        this_charset = (char*) this_segment->charset;
    }

    is_white = I18nIsWhiteSpace ( (char *) NULL,
				  (char *) this_charset,
				  (XtPointer) this_char );

    if (this_char)
	XtFree (this_char);

    return (is_white);
}



/*======================================================================
 * This routine finds the previous word to the left			
 */
static
void ScanPrevWord( widget, location )
Widget widget;
TextLocation location;
{
    do
    {
	if( !_DXmCSTextNavPrevChar( location )   ) break;
    }
    while( !_DXmCSTextIsScanBreak( widget, location, SCAN_NEXT_TO_PREV, I18NWORDSELECT ) );

    if ( _DXmCSTextIsWhiteSpace ( widget, location ) )
    {
	do
	{
	    if ( !_DXmCSTextNavPrevChar( location ) 	) break;
	}
	while( !_DXmCSTextIsScanBreak( widget, location, SCAN_NEXT_TO_PREV, I18NWORDSELECT ) );

	/* gone too far
	 */
	if ( location->position < 0 )
		location->position = 0;
    }

    return;
}


/*======================================================================
 * This routine finds the next word to the right			
 */
static void ScanNextWord( widget, location )
Widget widget;
TextLocation location;
{
    /* until we find word break or run out of source
     */
    while( !_DXmCSTextIsScanBreak( widget, location, SCAN_PREV_TO_NEXT, I18NWORDSELECT ) )
    {
	if( !_DXmCSTextNavNextChar( location ) ) break;

	if( location->end_of_line ) break;
    }

    if ( !location->end_of_line )
	_DXmCSTextNavNextChar ( location );

    /* move right over all whitespace.
    */
    if ( _DXmCSTextIsWhiteSpace ( widget, location ) )
    {
	while( !_DXmCSTextIsScanBreak( widget, location, SCAN_PREV_TO_NEXT, I18NWORDSELECT ) )
    	{
        	if ( !_DXmCSTextNavNextChar( location ) ) break;
	}

	_DXmCSTextNavNextChar ( location );
    }
	

    return;
}


static
Boolean _all_blank ( widget, location )
DXmCSTextWidget widget;
TextLocationRec location;
{
    while ( !location.end_of_line )
    {
	if ( !_DXmCSTextIsWhiteSpace((Widget) widget, &location) )
		return (False);

	if ( !_DXmCSTextNavNextChar (&location) )
		break;
    }

    return (True);
}

/*======================================================================
 * This routine finds the previous paragraph in the given position.
 * If the current position is inside a line (paragraph), then the routine
 * returns the start position of the current paragarph, else, the starting
 * position of the previous paragraph will be returned.
 */
#ifdef _NO_PROTO
void _DXmCSTextScanPreviousParagraph( widget,
				      position,
				      new_position)
DXmCSTextWidget   widget;
DXmCSTextPosition position;
DXmCSTextPosition *new_position;
#else
void _DXmCSTextScanPreviousParagraph( DXmCSTextWidget   widget,
				      DXmCSTextPosition position,
				      DXmCSTextPosition *new_position )
#endif /* _NO_PROTO */
{
    TextLocationRec   location, prev_location;
    DXmCSTextPosition *temp_position;
    TextLine	      first_line;

    /* find the start of the existing line
     */
    _DXmCSTextScanStartOfLine( widget, position, new_position );

    /* find start position in the source
     */
    _DXmCSTextSourceLocate( widget, *new_position, &location );

    first_line = _DXmCSTextGetFirstLine(widget);

    if ( position == *new_position )
    {
	NavPrevLine (&location);
    }

    /* if started from a blank line, loop backward until hit a non-blank one
     */
    if ( is_blank_line(widget, location) )
    {
	while ( location.line != first_line )
	{
		if ( !is_blank_line(widget, location) )
		{
			break;
		}

		NavPrevLine (&location);
    	}
    }

    /* if first_line is hit, just return 0
     */

    if ( location.line == first_line )
    {
	*new_position = (DXmCSTextPosition) 0;
	return;
    }

    /* now it is a non-blank line, find until previous line is blank
     */

    prev_location = location;

    while ( location.line != first_line )
    {
	if ( is_blank_line(widget, location)    &&
	     position != prev_location.position )
        {
	     *new_position = prev_location.position;
	     return;
	}

	prev_location = location;
	NavPrevLine (&location);
    }

    *new_position = prev_location.position;
}

/*======================================================================
 * This routine finds the next paragraph in the given position.
 * If the current position is inside a line (paragraph), then the routine
 * returns the start position of the current paragarph, else, the starting
 * position of the next paragraph will be returned.
 */
#ifdef _NO_PROTO
void _DXmCSTextScanNextParagraph( widget,
				  position,
				  new_position)
DXmCSTextWidget   widget;
DXmCSTextPosition position;
DXmCSTextPosition *new_position;
#else
void _DXmCSTextScanNextParagraph( DXmCSTextWidget   widget,
				  DXmCSTextPosition position,
				  DXmCSTextPosition *new_position )
#endif /* _NO_PROTO */
{
    TextLocationRec   location;
    TextLine   last_line;

    last_line = _DXmCSTextGetLastLine (widget);

    /* find the start of the existing line
     */
    _DXmCSTextScanStartOfLine( widget, position, new_position );

    /* find start position in the source
     */
    _DXmCSTextSourceLocate( widget, *new_position, &location );

    /* if it is the last line already, return the last position
     */

    if ( location.line == last_line )
    {
	*new_position = DXmCSTextGetLastPosition( widget );
	return;
    }

    /* find first blank line
     */

    while ( location.line != last_line )
    {
        if ( is_blank_line(widget, location) )
        {
            break;
        }
        NavNextLine (&location);
    }

    /* starting position within the last paragraph, just return the last
     * position of the widget
     */
    if ( location.line == last_line )
    {
	*new_position = DXmCSTextGetLastPosition (widget);
	return;
    }

    /* now, location should contain a blank line.  Scan the next non blank one
     */

    while ( location.line != last_line )
    {
	if ( !is_blank_line(widget, location) )
	{
	     *new_position = location.position;
	     return;
	}
	NavNextLine (&location);
    }

    if ( location.line == last_line )
    {
    	_DXmCSTextScanStartOfLine( widget, location.position, new_position );
	return;
    }
}


/*======================================================================
 * This routine finds the next word in the given			
 * direction starting at the given position.  If the current position	
 * is in a word and the direction is left, then the routine returns the 
 * beginning of the word.  
 */
#ifdef _NO_PROTO
void _DXmCSTextScanWord( widget, 
	                 position,
	                 direction, 
	                 new_position)

DXmCSTextWidget widget;
DXmCSTextPosition position;	 /* the start position */
DXmCSTextScanDirection direction; /* the direction to go */
DXmCSTextPosition *new_position;	 /* (output) new position */
#else
void _DXmCSTextScanWord( DXmCSTextWidget        widget,
			 DXmCSTextPosition      position,
			 DXmCSTextScanDirection direction,
			 DXmCSTextPosition      *new_position )
#endif /* _NO_PROTO */
{
    TextLocationRec location;

    /* find the start position in the source
     */
    _DXmCSTextSourceLocate( widget, position, &location );

    /* are we scanning left or right?
     */
    if( direction == DXmsdRight )
    {
	ScanNextWord( widget, &location );
    }else{
	ScanPrevWord( widget, &location );
    }

    *new_position = location.position;
	
    return;
}


/*======================================================================
 * This routine finds the beginning and end of the current word		
 * If the current position is not in a word then the nearest word on 
 * the line is found.  If there is no word on the line False is 
 * returned.  
 */
#ifdef _NO_PROTO
Boolean _DXmCSTextScanWordLimits( widget, 
	                          position,
	                          first_position,
			          last_position )

DXmCSTextWidget widget;
DXmCSTextPosition position;	 /* the start position */
DXmCSTextPosition *first_position;	 /* (output) 1st pos in word */
DXmCSTextPosition *last_position;	 /* (output) last pos in word */
#else
Boolean _DXmCSTextScanWordLimits( DXmCSTextWidget   widget,
				  DXmCSTextPosition position,
				  DXmCSTextPosition *first_position,
				  DXmCSTextPosition *last_position )
#endif /* _NO_PROTO */
{
    TextLocationRec    location, temp_loc;
    
    /* find the start position in the source
     */
    _DXmCSTextSourceLocate( widget, position, &location );

    *first_position = *last_position = position;

    /* if we are in between of a word ...
     */

    if ( !_DXmCSTextIsWhiteSpace((Widget) widget, &location) )
    {
	/* find the left boundary
	 */
	temp_loc = location;

	while ( !_DXmCSTextIsScanBreak((Widget) widget,&temp_loc,SCAN_NEXT_TO_PREV,I18NWORDSELECT) )
	{
		if ( temp_loc.end_of_line )    break;

		if ( !_DXmCSTextNavPrevChar(&temp_loc) ) break;
	}

	/* are we too far left?
	 */
	if ( temp_loc.position < 0 )

		*first_position = 0;
	else
		*first_position = temp_loc.position;

	/* find the right boundary
	 */
	temp_loc = location;

	while ( !_DXmCSTextIsScanBreak ((Widget) widget,&temp_loc, SCAN_PREV_TO_NEXT, I18NWORDSELECT) )
	{
		if ( !_DXmCSTextNavNextChar(&temp_loc) ) break;
	}

	/* Nav once more for the desired position
	 */
	_DXmCSTextNavNextChar(&temp_loc);

	*last_position = temp_loc.position;

	return (True);

    } else {

	temp_loc = location;

	/* see if prev char is a white space also, if so, return with false
	 */

	_DXmCSTextNavPrevChar(&temp_loc);

	if ( _DXmCSTextIsWhiteSpace( (Widget) widget, &temp_loc ) )
		return (False);

	/* The current position at this point must be the right_position, Now
	 * let's find the left one
	 */

	while ( !_DXmCSTextIsScanBreak ((Widget) widget,&temp_loc,SCAN_NEXT_TO_PREV,I18NWORDSELECT) )
	{
		if ( !_DXmCSTextNavPrevChar ( &temp_loc ) ) break;
		if ( temp_loc.end_of_line ) 	  break;
	}

	*first_position = temp_loc.position;

	return (True);
    }

}


/*======================================================================
 * This routine finds the end of the current line
 */
#ifdef _NO_PROTO
void _DXmCSTextScanEndOfLine( widget, 
		              position,
		              new_position 
		              )
DXmCSTextWidget        widget;
DXmCSTextPosition      position;	     /* the start position */
DXmCSTextPosition      *new_position; /* (output) new position */
#else
void _DXmCSTextScanEndOfLine( DXmCSTextWidget        widget,
			      DXmCSTextPosition      position,
			      DXmCSTextPosition      *new_position )
#endif /* _NO_PROTO */
{
    TextLocationRec location;
    DXmCSTextOutput output;

    /* find the start position in the source
    */
    _DXmCSTextSourceLocate( widget, position, &location );

    output = (DXmCSTextOutput)(widget->cstext.output);

    (*output->ScanEndOfLine)(   widget,
			        &location,
			        new_position );
    return;
}


/*======================================================================
 * This routine finds the beginning of the current line
 */
#ifdef _NO_PROTO
void _DXmCSTextScanStartOfLine( widget, 
		                position,
		                new_position 
		                )
DXmCSTextWidget        widget;
DXmCSTextPosition      position;	     /* the start position */
DXmCSTextPosition      *new_position; /* (output) new position */
#else
void _DXmCSTextScanStartOfLine( DXmCSTextWidget        widget,
				DXmCSTextPosition      position,
				DXmCSTextPosition      *new_position )
#endif /* _NO_PROTO */
{
    TextLocationRec location;
    DXmCSTextOutput output;

    /* find the start position in the source
    */
    _DXmCSTextSourceLocate( widget, position, &location );

    output = (DXmCSTextOutput)(widget->cstext.output);

    (*output->ScanStartOfLine)( widget,
			        &location,
			        new_position );
    
    return;
}


/*======================================================================
 * This routine finds the start of the previous or next line, depending 
 * on the given direction.  
 */
#ifdef _NO_PROTO
void _DXmCSTextScanNextLine( widget, 
		             position,
		             new_position 
		             )

DXmCSTextWidget        widget;
DXmCSTextPosition      position;	     /* the start position */
DXmCSTextPosition      *new_position; /* (output) new position */
#else
void _DXmCSTextScanNextLine( DXmCSTextWidget        widget,
			     DXmCSTextPosition      position,
			     DXmCSTextPosition      *new_position )
#endif /* _NO_PROTO */
{
    TextLocationRec  location;
    DXmCSTextOutput  output;

    /* find the start position in the source
    */
    _DXmCSTextSourceLocate( widget, position, &location );

    output = (DXmCSTextOutput)(widget->cstext.output);

    (*output->ScanNextLine)( widget,
			     position,
			     &location,
			     new_position );
    return;
}


/*======================================================================
 * This routine finds the start of the previous line
 */
#ifdef _NO_PROTO
void _DXmCSTextScanPrevLine( widget, 
		             position,
		             new_position 
		             )

DXmCSTextWidget        widget;
DXmCSTextPosition      position;	     /* the start position */
DXmCSTextPosition      *new_position; /* (output) new position */
#else
void _DXmCSTextScanPrevLine( DXmCSTextWidget        widget,
			     DXmCSTextPosition      position,
			     DXmCSTextPosition      *new_position )
#endif /* _NO_PROTO */
{
    TextLocationRec  location;
    DXmCSTextOutput  output;

    /* find the start position in the source
    */
    _DXmCSTextSourceLocate( widget, position, &location );

    output = (DXmCSTextOutput)(widget->cstext.output);

    (*output->ScanPrevLine)( widget,
			     position,
			     &location,
			     new_position );
    return;
}


/*======================================================================*/
/* This routine finds the end or beginning of source depending		*/
/* on the given direction.  
*/
#ifdef _NO_PROTO
void _DXmCSTextScanAll( widget, 
	                position,
	                direction, 
	                new_position
	                )

DXmCSTextWidget        widget;
DXmCSTextPosition      position;	     /* the start position */
DXmCSTextScanDirection direction;     /* the direction to go */
DXmCSTextPosition      *new_position; /* (output) new position */
#else
void _DXmCSTextScanAll( DXmCSTextWidget        widget,
			DXmCSTextPosition      position,
			DXmCSTextScanDirection direction,
			DXmCSTextPosition      *new_position )
#endif /* _NO_PROTO */
{
    /* are we scanning left or right?
    */
    if( direction == DXmsdRight )
    {
	*new_position = CSTextLength (widget);
    }else{
	*new_position = 0;
    }

    return;
}

#ifdef _NO_PROTO
Boolean _DXmCSTextGetSelection( widget, left, right)
DXmCSTextWidget   widget;
DXmCSTextPosition *left;
DXmCSTextPosition *right; 
#else
Boolean _DXmCSTextGetSelection( DXmCSTextWidget   widget,
				DXmCSTextPosition *left,
				DXmCSTextPosition *right )
#endif /* _NO_PROTO */
{
    if( !CSTextHasSelection( widget ) )
    {
	return FALSE;
    }

    *left  = CSTextSelLeft( widget );
    *right = CSTextSelRight( widget );

    return TRUE;
}


#ifdef _NO_PROTO
void _DXmCSTextSourceDestroy( widget )
DXmCSTextWidget widget;
#else
void _DXmCSTextSourceDestroy( DXmCSTextWidget widget )
#endif /* _NO_PROTO */
{
    TextLine next_line, next_one;

    for( next_line  = (TextLine)CSTextLines (widget); 
	 next_line != (TextLine)NULL; 
	 next_line  = next_one )
    {
        /* save the next pointer for the "next_line" will be freed inside
           the SourceRemoveLine()
        */
        next_one = next_line->next;

	SourceRemoveLine( widget, next_line );
    }

    return;
}



#ifdef _NO_PROTO
XmString _DXmCSTextSourceGetValue( widget )
DXmCSTextWidget widget;
#else
XmString _DXmCSTextSourceGetValue( DXmCSTextWidget widget )
#endif /* _NO_PROTO */
{
    XmString str;

    if( _DXmCSTextSourceReadString( widget, 0, CSTextLength (widget), &str ) 
	!= I_STR_EditDone )
    {
	return (XmString)NULL;
    }

    return str;
}


#ifdef _NO_PROTO
DXmCSTextStatus _DXmCSTextSourceSetValue( widget, value )
DXmCSTextWidget widget;
XmString value;
#else
DXmCSTextStatus _DXmCSTextSourceSetValue( DXmCSTextWidget widget,
					  XmString 	  value )
#endif /* _NO_PROTO */
{
    Boolean editable;
    
    _DXmCSTextSetSelection( widget, 0, 0, CurrentTime );

    DXmCSTextDisableRedisplay( widget, TRUE );
    
    editable = CSTextEditable (widget);
    CSTextEditable (widget) = TRUE;

    DXmCSTextSetTopPosition( widget, 0 );

    /* remove all existing data from the source	
    */
    SourceRemove( widget, 0, CSTextLength (widget));

/*
SourceRemove removes only those segments from the character at position 0.
Therefore it will not remove the initial empty segment(s). 
Removing the first line here should do the trick and it also conforms to
the check for a NULL line in the segment callback routine (CvtFromCSSegmentCB).
*/
    SourceRemoveLine( widget, CSTextLines( widget ) )  ;

    _DXmCSTextSourceInsertString( widget, 0, value, 0 );

    CSTextEditable (widget) = editable;

    call_source_changed_callback(widget);

    DXmCSTextEnableRedisplay (widget);

    return I_STR_EditDone;
}



/* ----------------------------------------
*  move to the next character in the source
*  could be "end-of-line" character
*  ----------------------------------------
*/
#ifdef _NO_PROTO
Boolean _DXmCSTextNavNextChar( location )
TextLocation location;
#else
Boolean _DXmCSTextNavNextChar( TextLocation location )
#endif /* _NO_PROTO */
{
    TextLine	    line;
    TextSegment     segment;

    line    = location->line;
    segment = location->segment;

    if( line == (TextLine)NULL )
    {
	return False;
    }

    location->position++;
    location->line_offset++;
    location->offset++;

    if( location->end_of_line )
    {
      location->line = location->line->next;

      location->line_offset = 0;
      location->offset = 0;

      if (location->line)
	{
	  location->segment = location->line->segments;

	  while (location->segment && (location->segment->length == 0))
	    location->segment = location->segment->next;
	}
      else
	location->segment = NULL;
    }
    else
      {
	if (location->offset >= segment->length)
	  {
	    location->segment = location->segment->next;

	    /* now skip any emtpy segs */

	    while (location->segment && (location->segment->length == 0))
	      location->segment = location->segment->next;

	    location->offset = 0;
	  }
      }

    location->end_of_line = location->segment == NULL;

    return (True);
}



static void
_bump_up_line (location)
    TextLocation location;
{
  location->line = location->line->prev;
  
  if (location->line)
    {
      location->line_offset = location->line->length;
    }
  else
    {
      location->line_offset = 0;
    }
  
  location->segment = NULL;
  location->offset  = 0;
}



/* --------------------------------------------
*  move to the previous character in the source
*  could be "end-of-line" character
*  --------------------------------------------
*/
#ifdef _NO_PROTO
Boolean _DXmCSTextNavPrevChar( location )
TextLocation location;
#else
Boolean _DXmCSTextNavPrevChar( TextLocation location )
#endif /* _NO_PROTO */
{
  TextSegment a;

  if( location->line == (TextLine)NULL )
    {
      return False;
    }
  
  location->position--;
  
  if (location->end_of_line)
    {
      a = location->line->segments;
      
      /*
       * find end of last seg in this line
       */
      for ( ;
	   (a != NULL) && (a->next != NULL);
	   a = a->next)
	;
      
      /*
       * now find previous non-emtpy seg
       */
      
      for ( ;
	   (a != NULL) && (a->length == 0);
	   a = a->prev)
	;
      
      location->segment     = a;
      
      if (location->segment)
      {
	location->offset    = location->segment->length - 1;
	location->line_offset = location->line->length - 1;
      }
      else
	_bump_up_line (location);
    }
  else
    {
      location->line_offset--;
      location->offset--;
      
      if (location->offset < 0)
	{
	  location->segment = location->segment->prev;

	  /* now skip emtpy segs */

	  while (location->segment && (location->segment->length == 0))
	    location->segment = location->segment->prev;

	  if (location->segment == NULL)
	    {
	      _bump_up_line (location);
	    }
	  else
	    {
	      location->offset = location->segment->length - 1;
	    }
	}
    }

  location->end_of_line = location->segment == NULL;

  return (TRUE);
}


static void
_recover_start_of_source (location, line)
     TextLocation location;
     TextLine line;
{
  location->position    = 0;
  location->line        = line;
  if (line)
    location->segment   = line->segments;
  else
    location->segment   = NULL;
  location->line_offset = 0;
  location->offset      = 0;
  location->end_of_line = location->segment == NULL;
}


/* ------------------------------------------------
*  move to first position in next non-empty segment
*  ------------------------------------------------
*/
#ifdef _NO_PROTO
Boolean NavNextSegment( location )
TextLocation location;
#else
Boolean NavNextSegment( TextLocation location )
#endif /* _NO_PROTO */
{
  TextSegment seg;
  Boolean ok = TRUE;

  for (seg = location->segment;
       (seg == location->segment) && ok;
       ok = _DXmCSTextNavNextChar (location))
    ;

  return (ok);
}

/* --------------------------------------------------------
*  move to first position in next non-empty segment in line
*  --------------------------------------------------------
*/
#ifdef _NO_PROTO
Boolean NavNextSegmentInLine( location )
TextLocation location;
#else
Boolean NavNextSegmentInLine( TextLocation location )
#endif /* _NO_PROTO */
{
    TextLine         line;
    TextSegment      seg;
    Boolean ok = TRUE;

    for (seg = location->segment, line = location->line;
	 (seg == location->segment) && ok;
	 ok = NavNextSegment (location))
      ;

    if (line != location->line)
      for ( ;
	   (line != location->line) && ok;
	   ok = _DXmCSTextNavPrevChar (location))
	;
    
    return ( ! location->end_of_line);
}

/* ---------------------------------------------------------------
*  move to the last character in previous segment, even if it's in 
*  a previous line
*  ---------------------------------------------------------------
*/
#ifdef _NO_PROTO
Boolean NavEndPrevSegment( location )
TextLocation location;
#else
Boolean NavEndPrevSegment( TextLocation location )
#endif /* _NO_PROTO */
{
    TextLine line = location->line;
    TextSegment      seg;
    Boolean ok = TRUE;
 
    for (seg = location->segment;
	 (seg == location->segment) && ok;
	 ok = _DXmCSTextNavPrevChar (location))
      ;

  if (location->line == NULL)
    {
      _recover_start_of_source (location, line);
      return (TRUE);
    }

    return (ok);
}

/* -----------------------------------------------------------------
*  move to the first char of first non-empty segment of current line
*  -----------------------------------------------------------------
*/
static Boolean FirstSegmentInLine( location )
TextLocation location;
{
    TextLine         line;
    Boolean ok = TRUE;

    for (line = location->line;
	 (line == location->line) && ok;
	 ok = _DXmCSTextNavPrevChar (location))
      ;

  if (location->line == NULL)
    {
      _recover_start_of_source (location, line);
      return (TRUE);
    }

    ok = _DXmCSTextNavNextChar (location);

    return (ok);
}

/* ------------------------------------------------
*  move to the first character of the previous line
*  ------------------------------------------------
*/
#ifdef _NO_PROTO
Boolean NavPrevLine( location )
TextLocation location;
#else
Boolean NavPrevLine( TextLocation location )
#endif /* _NO_PROTO */
{
  TextLine line;
  Boolean ok = TRUE;

  FirstSegmentInLine( location );

  if (location->position > 0)
    {
      _DXmCSTextNavPrevChar (location);

      FirstSegmentInLine( location );
    }

  return True;
}

/* -----------------------------------------------
*  move to the end of the last segment in the line
*  in essence, sit on the "end-of-line" character
*  -----------------------------------------------
*/
#ifdef _NO_PROTO
Boolean NavEndLine( location )
TextLocation location;
#else
Boolean NavEndLine( TextLocation location )
#endif /* _NO_PROTO */
{
  Boolean ok = TRUE;

  while (( ! location->end_of_line) && ok)
      ok = _DXmCSTextNavNextChar (location);

    return (ok);
}

/* -----------------------------------------------------------------
*  move to the first char of first non-empty segment of current line
*  or to the "end-of-line" char
*  -----------------------------------------------------------------
*/

#ifdef _NO_PROTO
Boolean NavStartLine( location )
TextLocation location;
#else
Boolean NavStartLine( TextLocation location )
#endif /* _NO_PROTO */
{
  Boolean ok = TRUE;
  TextLine line = location->line;

  while (( location->line_offset > 0 ) && ok)
  {
    ok = _DXmCSTextNavPrevChar (location);
  }

  if (location->line == NULL)
    {
      _recover_start_of_source (location, line);
      return (TRUE);
    }

  return (ok);
}

/* -----------------------------------------------
*  move to the "end-of-line" for the previous line
*  -----------------------------------------------
*/
#ifdef _NO_PROTO
Boolean NavEndPrevLine( location )
TextLocation location;
#else
Boolean NavEndPrevLine( TextLocation location )
#endif /* _NO_PROTO */
{
  TextLine line = location->line;
  Boolean ok = TRUE;

  while (( ! location->end_of_line) && ok)
    ok = _DXmCSTextNavPrevChar (location);

  if (location->line == NULL)
    {
      _recover_start_of_source (location, line);
      return (TRUE);
    }

  return (ok);
}

/* --------------------------------------------------------------
*  move to the first char of first non-empty segment in next line
*  --------------------------------------------------------------
*/
#ifdef _NO_PROTO
Boolean NavNextLine( location )
TextLocation location;
#else
Boolean NavNextLine( TextLocation location )
#endif /* _NO_PROTO */
{
  TextLine         line;
  Boolean ok = TRUE;

  for (line = location->line;
       (line == location->line) && ok;
       ok = _DXmCSTextNavNextChar (location))
      ;

    return (ok);
}

#ifdef _NO_PROTO
Boolean _DXmCSTextEqualLocation (a, b)
    TextLocation a, b;
#else
Boolean _DXmCSTextEqualLocation ( TextLocation a,
			TextLocation b )
#endif /* _NO_PROTO */
{
    return ((a->position    == b->position)    &&
	    (a->line_offset == b->line_offset) &&
	    (a->line        == b->line)        &&
	    (a->segment     == b->segment)     &&
	    (a->offset      == b->offset)      &&
	    (a->end_of_line == b->end_of_line));
}

#ifdef _NO_PROTO
Boolean _DXmCSTextEqualSegment (a, b)
    TextSegment a, b;
#else
Boolean _DXmCSTextEqualSegment ( TextSegment a,
		       TextSegment b )
#endif
{
  return ((a->length      == b->length)      &&
	  (a->charset     == b->charset)     &&
	  (a->direction   == b->direction)   &&
	  (a->locale      == b->locale)      &&
	  (a->nest_level  == b->nest_level));
}




/* --------------------------
*/
#ifdef _NO_PROTO
TextLine _DXmCSTextGetFirstLine( widget )
DXmCSTextWidget widget;
#else
TextLine _DXmCSTextGetFirstLine( DXmCSTextWidget widget )
#endif /* _NO_PROTO */
{
    return ( (TextLine)CSTextLines (widget) );
}

/* --------------------------
*/
#ifdef _NO_PROTO
TextLine _DXmCSTextGetLastLine( widget )
DXmCSTextWidget widget;
#else
TextLine _DXmCSTextGetLastLine( DXmCSTextWidget widget )
#endif /* _NO_PROTO */
{
    TextLine next_line, last_line;

    for( next_line = (TextLine)CSTextLines (widget);
	 next_line != (TextLine)NULL;
	 next_line = next_line->next )
    {
	 last_line = next_line;
    }
    return ( last_line );
}
/* --------------------------
*/
#ifdef _NO_PROTO
TextLine _DXmCSTextGetNextLine( line )
TextLine line;
#else
TextLine _DXmCSTextGetNextLine( TextLine line )
#endif /* _NO_PROTO */
{
    return ( line->next );
}


/* --------------------------
*/
#ifdef _NO_PROTO
TextLine _DXmCSTextGetPrevLine( line )
TextLine line;
#else
TextLine _DXmCSTextGetPrevLine( TextLine line )
#endif /* _NO_PROTO */
{
    return ( line->prev );
}



/* -------------------------- */
/* get the first segment in the line
*/
#ifdef _NO_PROTO
TextSegment _DXmCSTextGetFirstSegment( line )
TextLine line;
#else
TextSegment _DXmCSTextGetFirstSegment( TextLine line )
#endif /* _NO_PROTO */
{
    return ( line->segments );
}

/* -------------------------- */
/* get the last segment in the line
*/
#ifdef _NO_PROTO
TextSegment _DXmCSTextGetLastSegment( line )
TextLine line;
#else
TextSegment _DXmCSTextGetLastSegment( TextLine line )
#endif /* _NO_PROTO */
{
  TextSegment s;

  if( line == (TextLine)NULL || line->segments == (TextSegment)NULL )
  {
    return (TextSegment)NULL;
  }

  for( s = line->segments;
      s->next != (TextSegment) NULL;
      s = s->next )
    ;
  
  return (s);
}

/* -------------------------- 
*/
#ifdef _NO_PROTO
TextSegment _DXmCSTextGetNextSegment( segment )
TextSegment segment;
#else
TextSegment _DXmCSTextGetNextSegment( TextSegment segment )
#endif /* _NO_PROTO */
{
    return ( segment->next );
}


/* -------------------------- 
*/
#ifdef _NO_PROTO
TextSegment _DXmCSTextGetPrevSegment( segment )
TextSegment segment;
#else
TextSegment _DXmCSTextGetPrevSegment( TextSegment segment )
#endif /* _NO_PROTO */
{
    return ( segment->prev );
}



/* -------------------------- 
*/
static Boolean call_source_modify_callback( widget, 
					    start,
                                            end,
					    string )
DXmCSTextWidget   widget;
DXmCSTextPosition *start;
DXmCSTextPosition *end;
XmString          *string;
{
    DXmCSTextVerifyCallbackStruct cb;

    if  ( *start == *end && *string == NULL ) return False;

    cb.reason = XmCR_MODIFYING_TEXT_VALUE;
    cb.event = NULL;
    cb.doit = True;
    cb.currInsert = CursorPos( widget );
    cb.newInsert  = CursorPos( widget );
    cb.startPos = *start,
    cb.endPos = *end;
    cb.text = *string;

    XtCallCallbackList( (Widget)widget, (XtCallbackList)ModifyCB (widget), (XtPointer)&cb);

    if (cb.doit)
    {
	*string = cb.text;
	*start = cb.startPos;
	*end =  cb.endPos;
    }

    return cb.doit;
}
/* -------------------------- 
*/
static void call_source_changed_callback( widget )
DXmCSTextWidget widget;
{
    XmAnyCallbackStruct cb;

    cb.reason = XmCR_VALUE_CHANGED;
    cb.event  = NULL;

    XtCallCallbackList( (Widget)widget, (XtCallbackList)ValueCB (widget), (XtPointer)&cb);

    return;
}

/* -------------------------- 
*/
static void call_segment_modified_callback( widget, 
					 line,
                                         segment )
DXmCSTextWidget widget;
TextLine line;
TextSegment segment;
{
    DXmCSTextOutput output;

    output = (DXmCSTextOutput)(widget->cstext.output);

    (*output->HandleData)( widget, 
			   line,
			   segment,
			   DXmCSTextOutputSegmentModified );
}

static void call_segment_delete_callback( widget, line, segment )
DXmCSTextWidget widget;
TextLine line;
TextSegment segment;
{
    DXmCSTextOutput output;

    output = (DXmCSTextOutput)(widget->cstext.output);

    (*output->HandleData)( widget, 
			   line,
			   segment,
			   DXmCSTextOutputSegmentDelete );
}

static void call_line_delete_callback( widget, line )
DXmCSTextWidget widget;
TextLine line;
{
    DXmCSTextOutput output;

    output = (DXmCSTextOutput)(widget->cstext.output);

    (*output->HandleData)( widget, 
			   line,
			   (TextSegment)NULL,
			   DXmCSTextOutputLineDelete );
}

static void
call_line_added_callback( widget, line )
DXmCSTextWidget widget;
TextLine line;
{
    DXmCSTextOutput output;

    output = (DXmCSTextOutput)(widget->cstext.output);

    (*output->HandleData)( widget, 
			   line,
			   (TextSegment)NULL,
			   DXmCSTextOutputLineAdd );
}

static void call_line_modified_callback( widget, line )
DXmCSTextWidget widget;
TextLine line;
{
    DXmCSTextOutput output;

    output = (DXmCSTextOutput)(widget->cstext.output);

    (*output->HandleData)( widget, 
			   line,
			   (TextSegment)NULL,
			   DXmCSTextOutputLineModified );
}

static void call_single_line_delete_callback( widget, line)
DXmCSTextWidget widget;
TextLine 	line;
{
    DXmCSTextOutput output;

    output = (DXmCSTextOutput)(widget->cstext.output);

    (*output->HandleData)( widget,
			   line,
			   (TextSegment)NULL,
			   DXmCSTextOutputSingleLineDelete );
}

static void call_single_segment_delete_callback( widget, line, segment )
DXmCSTextWidget widget;
TextLine	line;
TextSegment	segment;
{
    DXmCSTextOutput output;

    output = (DXmCSTextOutput)(widget->cstext.output);

    (*output->HandleData)( widget,
			   line,
			   (TextSegment)segment,
			   DXmCSTextOutputSingleSegmentDelete );
}


static void call_output_validate_rest_line( widget, line, segment )
DXmCSTextWidget widget;
TextLine	line;
TextSegment	segment;
{
    DXmCSTextOutput output;

    output = (DXmCSTextOutput)(widget->cstext.output);

    (*output->HandleData)( widget,
			   line,
			   (TextSegment)NULL,
			   DXmCSTextOutputRestLineValidate );
}


static void call_output_invalidate_single_line( widget, line, segment )
DXmCSTextWidget widget;
TextLine	line;
TextSegment	segment;
{
    DXmCSTextOutput output;

    output = (DXmCSTextOutput)(widget->cstext.output);

    (*output->HandleData)( widget,
			   line,
			   (TextSegment)NULL,
			   DXmCSTextOutputSingleLineInvalidate );
}


static void call_output_text_empty(widget)
DXmCSTextWidget widget;
{
    register DXmCSTextOutput output =
	(DXmCSTextOutput)widget->cstext.output;

    (*output->HandleData)(widget, NULL, NULL,
			   DXmCSTextOutputTextEmpty);
}
