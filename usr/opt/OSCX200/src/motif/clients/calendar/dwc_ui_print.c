/* dwc_ui_print.c */
#ifndef lint
static char rcsid[] = "$Header$";
#endif /* lint */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**
**  FACILITY:
**
**	DECwindows Calendar; print routines
**
**  AUTHOR:
**
**	Marios Cleovoulou, March-1988
**
**  ABSTRACT:
**
**	This are the print routines for DECWindows Calendar.
**
**
**  MODIFICATION HISTORY:
**	16-dec-1993 pjw - Tony DeCarlo's fix for ps printing. 
**		ootb_bug 437. PS printing was totally broken
**		and only appeared to work due to sheer luck. 
**
**
**	19-Oct-1993 tdc (Tony DeCarlo)
**		Partial fix for ootb bug 322.  Printing in any format
**		produced an empty text field.  PS and text printing now
**		works.  Also, DDIF format printing produces a garbage file.
**		DDIF option disabled.
**
**--
*/

#include "dwc_compat.h"

#include <stdlib.h>
#include <stdio.h>

#if defined(vaxc) && !defined(__DECC)
#pragma nostandard
#endif
#include <Xm/Xm.h>		    /* for XmString routines */
#include <Xm/ToggleB.h>
#include <DXm/DXmPrint.h>	    /* for printwidget */
#include <X11/DECwI18n.h>
#include <DXm/DECspecific.h>
#include <dwi18n_lib.h>
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"	/* for DWC$k_db_normal */

#include "dwc_ui_calendar.h"	    	/* k_pixmap... */
#include "dwc_ui_dateformat.h"   	/* dtb.. */
#include "dwc_ui_catchall.h"		/* DWC$DRM... */
#include "dwc_cda_util.h"	    	/* DWCCDA_ContextRec.. */
#include "dwc_ui_misc.h"		/* MISCTestDayConditions */
#include "dwc_ui_icons.h"
#include "dwc_ui_datefunctions.h"
#include "dwc_ui_errorboxes.h"

#if 0
extern unsigned int	pixmap_foreground;  /* defined in dwc_ui_calendar.c */
#endif
extern Pixmap		big_pixmaps[256];

#define STR_FF		"\014"
#define CH_TAB		'\011'

static	    XImage	*icon_images[256];
static	    Boolean	icons_initialized = False;


#ifdef VAXC
typedef void *X_PrCtx;
#else
typedef char *X_PrCtx;
#endif

struct _PrContextRec;

typedef char *PrintIcons PROTOTYPE ((
	struct _PrContextRec	*ctx,
	int			class,
	char			*entry_text,
	int			flags,
	Boolean			alarm,
	Boolean			repeating));

typedef Boolean PrintDay PROTOTYPE ((
	struct _PrContextRec	*ctx,
	CalendarDisplay		cd,
	Boolean			ampm,
	Boolean			laser));

typedef Boolean PrintEntry PROTOTYPE ((
	struct _PrContextRec	*ctx,
	Cardinal		start_time,
	Cardinal		end_time,
	char			*entry_text,
	Boolean			ampm,
	Boolean			laser));

typedef enum { PS, TEXT, DDIF } PrintFormat;
typedef struct _PrContextRec
{
    DWCCDA_ContextRec	cda_rec;
    DWCCDA_Context		cda;
    char		*filespec;
    FILE		*printer;
    PrintFormat		print_format;
    int			pagesize;
    Cardinal		dsbot;
    PrintDay		*print_day;
    PrintEntry		*print_entry;
    PrintIcons		*print_icons;
    int			line_limit;
} PrContextRec,   *PrCtx;

static void PRINT_OK PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

static void PRINT_DG_OK PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

static void PRINT_OPTIONS_OK PROTOTYPE ((Widget w));

static void PRINT_DG_OPTIONS PROTOTYPE ((Widget w));

static void PRINT_DG_TEXT_CHANGED PROTOTYPE ((Widget w));

static void PRINT_CANCEL PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

static void PRINT_DG_CANCEL PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

static void PRINT_LIMIT PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

char *prolog_filespec PROTOTYPE (());


/*
 * get_text( comp_string )
 *      Returns the text from a compound string.
 *      Returns the null string if the compound string is null or invalid.
 *      The string is allocated by this routine so it must be deallocated
 *      with XtFree().
 * 
 */
char* get_text( cs )
    XmString cs;
{
    char        *ret_text;
    XmStringContext   context;
    char        *text;
    long        lang,rend;
    int         stat;
    Boolean     done, separator;
    XmStringCharSet     charset;
    XmStringDirection   dir;
    long byte_count, cvt_status;

    ret_text = NULL;

    /*
     * The following two lines were added 
     * in place of the original get_text routine.  The routine DXmCvtCStoFC 
     * should have been used originally but was not simple due to lack of 
     * knowledge about its existence.  The original code is left here for 
     * historically purposes.  It may be needed some day since DXmCvtCStoFC 
     * is a 'DEC' routine, not a Motif routine. 
     */
    ret_text = DXmCvtCStoFC(cs, &byte_count, &cvt_status);
    return ( ret_text );

#if 0
    if( XmStringInitContext(&context, cs) == TRUE) {
        ret_text = XtMalloc( 1 );
        *ret_text = '\0';
        while ( XmStringGetNextSegment( context, &text, &charset, &dir, &separator )) {
            ret_text = XtRealloc( ret_text, strlen(ret_text)+strlen(text)+1 );
            strcat( ret_text, text );
             *
             * Added the following line.  Without this, multiple line get
             * concatenated onto one line.
             *
            strcat( ret_text, "\n" );
        }
        XmStringFreeContext( context );
    }
    XtFree(charset);
    return ( ret_text );
#endif /* 0 */
}

static void PrintWrappedText_TEXT
#ifdef _DWC_PROTO_
	(
	PrCtx		ctx,
	char		*entry_text,
	Cardinal	form_width)
#else	/* no prototypes */
	(ctx, entry_text, form_width)
	PrCtx		ctx;
	char		*entry_text;
	Cardinal	form_width;
#endif	/* prototype */
{
    int		lines, start, current, spaces, nonspace, width;

    /*
    **  Output text, wrapping on the way.
    **
    **	lines	    the number of lines printed so far,
    **	start	    is the position along the text from which the next output
    **		    will be done,
    **	current	    is the position along the text at which we are currently
    **		    looking,
    **	spaces	    is the position of first of the last set of contiguous
    **		    spaces or tabs seen,
    **	nonspace    is the position of the last non-space after "spaces",
    **	width	    is the width of the current line so far.
    */
    
    lines = start = current = spaces = nonspace = width = 0;

    while (entry_text [current] != '\0')
    {
	/*
	**  Scan along the text looking for "form_width" number of characters.
	**  Reset the counters and write the line if we find a newline.
	*/
	
	if (entry_text [current] == '\n')
	{
	    /*
	    **	Found newline.  If current position is not equal to first
	    **	non-space after spaces, then write the whole line.  Otherwise
	    **	write it without all the trailing spaces.
	    */
	    
	    if (current != nonspace)
	    {
		spaces = current;
	    }
	    fprintf
		(ctx->printer, "%0.*s\n", spaces - start, entry_text + start);

	    if( ctx->line_limit )
		if( ++lines >= ctx->line_limit )
		    return;

	    current++;
	    width = 0;
	    start = spaces = nonspace = current;

	}
	else
	{

	    /*
	    **  If the character is a space or tab, then note the position of
	    **	the character and skip contiguous spaces and tabs (expanding
	    **	tabs along the way for counting purposes).  When a non-space or
	    **	tab is found, note its position and proceed.
	    */
	    
	    if ((entry_text [current] == ' ') ||
		(entry_text [current] == CH_TAB))
	    {
		spaces = current;
		while ((entry_text [current] == ' ') ||
		       (entry_text [current] == CH_TAB))
		{
		    if (entry_text [current] == ' ')
		    {
			width = width + 1;
		    }
		    else
		    {
			width = ((((width - 1) / 8) + 1) * 8) + 1;
		    }
		    current++;  
		}
		nonspace = current;
	    }
	    else
	    {
		current += DWI18n_ByteLengthOfChar (&entry_text[current]);
		width = width + 1;
	    }

	    /*
	    **  If we haven't reached the form width yet, just keep counting.
	    **	If we have then print the line up to the last space before the
	    **	overflow.  If there were no spaces then just do a straight chop.
	    **	Note that we proceed on the next line with the next non-space,
	    **	meaning that we trim trailing spaces.
	    */
	    
	    if (width >= form_width)
	    {
		if (spaces == start)
		{
		    nonspace = spaces = current;
		}
		fprintf
		(   ctx->printer
		,   "%0.*s\n"
		,   spaces - start
		,   entry_text + start
		);
		start = current = spaces = nonspace;
		width = 0;
	    }
	}

    }

    /*
    **	If there is anything left to print, do it now.  If current position is
    **	not equal to first non-space after spaces, then write the whole line.
    **	Otherwise write it without all the trailing spaces.
    */
    
    if( width != 0 )
    {
	if (current != nonspace) spaces = current;
	fprintf ( ctx->printer, "%0.*s\n", spaces - start, entry_text + start);
    }

}

static XImage *get_image_from_pixmap
#ifdef _DWC_PROTO_
	(
	Display	*dpy,
	Pixmap	bitmap)
#else	/* no prototypes */
	(dpy, bitmap)
	Display	*dpy;
	Pixmap	bitmap;
#endif	/* prototype */
{
    int		    x, y;
    unsigned int    width, height, border, depth;
    XImage	    *image;
    Status	    status;
    Window	    root;

    status = XGetGeometry( dpy,
			    bitmap,
			    &root,
			    &x,
			    &y,
			    &width,
			    &height,
			    &border,
			    &depth);

    image = XGetImage( dpy,bitmap,
			    0,
			    0,
			    width,
			    height,
			    AllPlanes,
			    ZPixmap);

    return image;
}

static void initialize_icons
#ifdef _DWC_PROTO_
	(
	Display	*dpy)
#else	/* no prototypes */
	(dpy)
	Display	*dpy;
#endif	/* prototype */
{
    int		i;

    for( i = 0; i < XtNumber( icon_images ); i++ )
	icon_images[i] = (XImage *) 0;
    for( i = k_pixmap_setable_start; i <= k_pixmap_setable_end; i++ )
	    icon_images[i] = get_image_from_pixmap( dpy, big_pixmaps[i] );
    for( i = k_pixmap_computed_start; i <= k_pixmap_computed_end; i++ )
	    icon_images[i] = get_image_from_pixmap( dpy, big_pixmaps[i] );
    icons_initialized = True;
}

static void cnv_image_to_hexstring
#ifdef _DWC_PROTO_
	(
	FILE	*ps,
	XImage	*image)
#else	/* no prototypes */
	(ps, image)
	FILE	*ps;
	XImage	*image;
#endif	/* prototype */
{
    int		    i, s, x, y;
    unsigned int    width, height, size;
    unsigned char   *picstr;

    width   = image->width;
    height  = image->height;

    size = height * (( width + 7 ) / 8);
    picstr = (unsigned char *) XtMalloc( size );

    for (i = -1, y = 0; y < height; y++)
    {	for (picstr[++i] = 0, s = 7, x = 0;
		x < width;
		    x++)
	{
	    unsigned int pixel;
	    pixel = XGetPixel (image, x, y);
	    if (s < 0)
	    {
		s = 7;
		picstr[++i] = 0;
	    }
	    picstr[i]  |= pixel << s--;
	}
	
    }
    for (s = 2, i = 0; i < size;
	    s += 2,
		i++)
    {
	if( s > 80 )
	{
	    s = 2;
	    fputc ('\n', ps);
	}
	fprintf (ps, "%02X", picstr[i]);
    }

    XtFree ((char *)picstr);
}

static void create_icon_font
#ifdef _DWC_PROTO_
	(
	Display	*dpy,
	FILE	*ps)
#else	/* no prototypes */
	(dpy, ps)
	Display	*dpy;
	FILE	*ps;
#endif	/* prototype */
{
    int		i;
    int		max_size;

    if( !icons_initialized ) initialize_icons( dpy );

    max_size = 1;

    for( i = 0; i < XtNumber( icon_images ); i++ )
	if( icon_images[i] != (XImage *) 0 )
	    {
	    fprintf( ps, "/Icon%03d {\n", i );
	    fprintf( ps, "%d %d true [ %d 0 0 %d 0 %d ] { <\n",
			icon_images[i]->width,
			icon_images[i]->height,
			icon_images[i]->width,
			-1 * icon_images[i]->height,
			icon_images[i]->height);
	    cnv_image_to_hexstring( ps, icon_images[i] );
	    fputs( "\n> } imagemask } def\n", ps );
	    if( icon_images[i]->width > max_size )
		max_size = icon_images[i]->width;
	    if( icon_images[i]->height > max_size )
		max_size = icon_images[i]->height;
	    }
    
    fputs( "end\n", ps );

    for( i = 0; i < XtNumber( icon_images ); i++ )
	if( icon_images[i] != (XImage *) 0)
	    fprintf( ps, "Encoding %3d /Icon%03d put\n", i, i );

    fputs( "/BuildChar { 1.4 0 -0.1 -0.1 1.1 1.1 setcachedevice\n", ps );
    fputs( "exch begin Encoding exch get CharProcs exch get end exec\n", ps );
    fprintf( ps, "1 %d div setlinewidth\n", max_size );
    fputs( "newpath -0.1 -0.1 moveto 1.2 0 rlineto 0 1.2 rlineto\n", ps );
    fputs( "-1.2 0 rlineto closepath stroke } def end\n", ps );
    fputs( "/IconFont NewIconFont definefont pop\n", ps );
    fputs( "%%\014\n", ps );
}

static void PrintWord_PS
#ifdef _DWC_PROTO_
	(
	PrCtx	ctx,
	int	length,
	char	*text)
#else	/* no prototypes */
	(ctx, length, text)
	PrCtx	ctx;
	int	length;
	char	*text;
#endif	/* prototype */
{
    char	c;
    int		i, byte_length;

    fputc ('(', ctx->printer);

    for (; length; length -= byte_length)
    {
	byte_length = DWI18n_ByteLengthOfChar (text);
	switch (*text)
	{
	case '(':
	    fputs ("\\(", ctx->printer);
	    text += byte_length;
	    break;

	case ')':
	    fputs ("\\)", ctx->printer);
	    text += byte_length;
	    break;

	case '\\':
	    fputs ("\\\\", ctx->printer);
	    text += byte_length;
	    break;

	default:
	    for (i = 0; i < byte_length; i++)
	    {
		c = *text;
		text++;
		fputc (c, ctx->printer);
	    }
	    break;
	}
    }

    fputs( ") DwcWord\n", ctx->printer );

}

static Boolean PrintIcon_PS
#ifdef _DWC_PROTO_
	(
	unsigned char	icon,
	FILE		*ps)
#else	/* no prototypes */
	(icon, ps)
	unsigned char	icon;
	FILE		*ps;
#endif	/* prototype */
{
    fprintf( ps, "<%02x> DwcIcon\n", icon);

    return True;
}

static char *PrintIcons_TEXT
#ifdef _DWC_PROTO_
	(
	PrCtx		ctx,
	int		class,
	char		*entry_text,
	int		flags,
	Boolean		alarm,
	Boolean		repeating)
#else	/* no prototypes */
	(ctx, class, entry_text, flags, alarm, repeating)
	PrCtx		ctx;
	int		class;
	char		*entry_text;
	int		flags;
	Boolean		alarm;
	Boolean		repeating;
#endif	/* prototype */
{
    int icon_count;
    char *current;

    current = entry_text;

    if( class == DWC$k_item_texti )
	if( ( icon_count = (int) *current++ ) > 0 )
	    while( icon_count-- > 0 )
		current++;
    return current;
}

static char *PrintIcons_DDIF
#ifdef _DWC_PROTO_
	(
	PrCtx		ctx,
	int		class,
	char		*entry_text,
	int		flags,
	Boolean		alarm,
	Boolean		repeating)
#else	/* no prototypes */
	(ctx, class, entry_text, flags, alarm, repeating)
	PrCtx		ctx;
	int		class;
	char		*entry_text;
	int		flags;
	Boolean		alarm;
	Boolean		repeating;
#endif	/* prototype */
{
    int icon_count;
    char *current;

    current = entry_text;

    if( class == DWC$k_item_texti )
	if( ( icon_count = (int) *current++ ) > 0 )
	    while( icon_count-- > 0 )
		current++;
    return current;
}

static char *NewPrintIcons_PS
#ifdef _DWC_PROTO_
	(
	PrCtx		ctx,
	char		*entry_text,
	int		flags,
    	unsigned char       *icons,
    	Cardinal            icons_number,
	Boolean		alarm,
	Boolean		repeating)
#else	/* no prototypes */
	(ctx, entry_text, flags, icons, icons_number, alarm, repeating)
	PrCtx		ctx;
	char		*entry_text;
	int		flags;
    	unsigned char       *icons;
    	Cardinal            icons_number;
	Boolean		alarm;
	Boolean		repeating;
#endif	/* prototype */
/*
 * Function:
 * 	NewPrintIcons_PS - outputs the entry icons to the PS file
 * Inputs:
 *	
 *	ctx	   	calendar context structure
 *	entry_text 	compund string containing the data for the entry
 *	flags	    	specifies the type of repeating entry
 *    	icons		contains one byte for each icon
 *    	icons_number	number of icons
 *	alarm	   	true if an alarm is set for this entry
 *	repeating  	true if this is a repeating entry
 *
 * Outputs:
 *	
 * Notes:
 */
{
    Boolean	    icons_printed;
    int		    icon_count;
    int		    i;

    icons_printed = False;

	/*
	 * For all the icons, print each one.
	 */
	for (i=0;i<icons_number;i++) {
		icons_printed = PrintIcon_PS( icons[i], ctx->printer );
	}

	/*
	 * Now do the 'special case' icons, which don't appear in the
	 * icons variable.
	 */
    if( repeating )
    {	int repeat_icon;
	switch( flags & DWC$m_item_rpos )
	{   case DWC$k_item_last:
		repeat_icon = k_pixmap_repeatend;
		break;
	    case DWC$k_item_first:
		repeat_icon = k_pixmap_repeatstart;
		break;
	    case DWC$k_item_middle:
		repeat_icon = k_pixmap_repeat;
		break;
	}
	icons_printed = PrintIcon_PS( repeat_icon, ctx->printer );
    }

    if( alarm )
	icons_printed = PrintIcon_PS( k_pixmap_bell, ctx->printer );

    if( icons_printed )
	fputs( "DwcNewLine\n", ctx->printer );

    return;
}		/* end routine NewPrintIcons_PS */

/*
** This routine is obselete.  It should no longer be called unless you
** know what you are doing.  Please read the comments in the routine
** DwcPRPrintDay_PS for a more detailed explanation around why this is so..
** The routine NewPrintIcons_PS is the replacement.
**
** [pjw] This routine looks like it was designed to print the icons
** in the days when calendar did not use compound strings. This routine
** clearly expects an ascii string. It was missed when Dick converted over
** to compound strings. 
*/
static char *PrintIcons_PS
#ifdef _DWC_PROTO_
	(
	PrCtx		ctx,
	int		class,
	char		*entry_text,
	int		flags,
	Boolean		alarm,
	Boolean		repeating)
#else	/* no prototypes */
	(ctx, class, entry_text, flags, alarm, repeating)
	PrCtx		ctx;
	int		class;
	char		*entry_text;
	int		flags;
	Boolean		alarm;
	Boolean		repeating;
#endif	/* prototype */
{
    Boolean	    icons_printed;
    int		    icon_count;
    char	    *current;

    icons_printed = False;
    current = entry_text;
/*
 * This is more of an educated guess than actually knowing whats going on.
 * Apparently the icons information used to be pre-pended to the compound 
 * string.  The next four lines of code would attempt to increment the 
 * pointer past the pre-pended icon information so that the pointer (current)
 * would point to the beginning of the compund string.  As it traversed 
 * through the string, it would print each icon which was found.  None of this
 * nonsense is needed anymore.  Use NewPrintIcons_PS instead.
 */
    if( class == DWC$k_item_texti )
	if( ( icon_count = (int) *current++ ) > 0 )
	    while( icon_count-- > 0 )
		icons_printed = PrintIcon_PS( *current++, ctx->printer);

    if( repeating )
    {	int repeat_icon;
	switch( flags & DWC$m_item_rpos )
	{   case DWC$k_item_last:
		repeat_icon = k_pixmap_repeatend;
		break;
	    case DWC$k_item_first:
		repeat_icon = k_pixmap_repeatstart;
		break;
	    case DWC$k_item_middle:
		repeat_icon = k_pixmap_repeat;
		break;
	}
	icons_printed = PrintIcon_PS( repeat_icon, ctx->printer );
    }

    if( alarm )
	icons_printed = PrintIcon_PS( k_pixmap_bell, ctx->printer );

    if( icons_printed )
	fputs( "DwcNewLine\n", ctx->printer );

    return current;
}

static void PrintWrappedText_PS
#ifdef _DWC_PROTO_
	(
	PrCtx		ctx,
	char		*entry_text)
#else	/* no prototypes */
	(ctx, entry_text)
	PrCtx		ctx;
	char		*entry_text;
#endif	/* prototype */
{
    int		start, current;
    int		byte_length;

#define PRINT_PENDING_WORD			\
    if (current - start > 0)			\
	PrintWord_PS (ctx, current - start, entry_text + start);

    for (current = 0, start = 0;
	    entry_text[current] != '\0';
		current += byte_length)
    {
	byte_length = DWI18n_ByteLengthOfChar (&entry_text[current]);
	switch (entry_text[current])
	{
	case '\n':
	    PRINT_PENDING_WORD;
	    fputs ("DwcNewLine\n", ctx->printer);
	    start = current + byte_length;
	    break;

	case ' ':
	    /*	  
	    **  Include the space at the beginning of the word
	    */	  
	    PRINT_PENDING_WORD;
	    start = current;
	    break;

	case '\011':
	    PRINT_PENDING_WORD;
	    fputs ("DwcTab\n", ctx->printer);
	    start = current + byte_length;
	    break;

	case '\014':
	    PRINT_PENDING_WORD;
	    fputs ("DwcNewPage\n", ctx->printer);
	    start = current + byte_length;
	    break;

	default:
	    break;
	}
    }
    PRINT_PENDING_WORD;
}

static void PrintDayTimes_PS
#ifdef _DWC_PROTO_
	(
	PrCtx		ctx,
	Cardinal	start_time,
	Cardinal	end_time,
	Boolean		ampm)
#else	/* no prototypes */
	(ctx, start_time, end_time, ampm)
	PrCtx		ctx;
	Cardinal	start_time;
	Cardinal	end_time;
	Boolean		ampm;
#endif	/* prototype */
{
    dtb		date_time;
    char	*text;
    XmString	cs;
    long	cvt_size, cvt_status;
        
    /*	  
    **  Is this a daynote?
    */	  
    if (( start_time == end_time ) && ( end_time == 0 )) 
    {
	cs = MISCFetchDRMValue(dwc_k_print_daynote_text);   /* don't free CS */
	text = (char *)DXmCvtCStoFC(cs, &cvt_size, &cvt_status);

	fprintf( ctx->printer, "(%s)DwcFrom\n", text);
	fputs( "()DwcTo\n", ctx->printer );

	XtFree (text);
	return;
    }

    date_time.hour   = start_time / 60;
    date_time.minute = start_time % 60;
    if (ampm)
    {
	text = DATEFORMATTimeToText (dwc_k_print_time_ampm_format, &date_time);
    }
    else
    {
	text = DATEFORMATTimeToText (dwc_k_print_time_format, &date_time);
    }
    fprintf( ctx->printer, "(%s)DwcFrom\n", text );
    XtFree (text);

    date_time.hour   = end_time / 60;
    date_time.minute = end_time % 60;
    if (ampm)
    {
	text = DATEFORMATTimeToText (dwc_k_print_time_ampm_format, &date_time);
    }
    else
    {
	text = DATEFORMATTimeToText (dwc_k_print_time_format, &date_time);
    }
    fprintf( ctx->printer, "(%s)DwcTo\n", text );
    XtFree (text);
}

static void PrintDayDate_PS
#ifdef _DWC_PROTO_
	(
	PrCtx	ctx)
#else	/* no prototypes */
	(ctx)
	PrCtx	ctx;
#endif	/* prototype */
{
    dtb		date_time;
    char	*text;


    DATEFUNCDateForDayNumber
	(ctx->dsbot, &date_time.day, &date_time.month, &date_time.year);

    date_time.weekday = DATEFUNCDayOfWeek
	(date_time.day, date_time.month, date_time.year);

    text = DATEFORMATTimeToText
	(dwc_k_print_full_date_format, &date_time);

    fprintf( ctx->printer, "(%s)DwcDate\n", text );
    XtFree (text);
	
}

static Boolean DwcPRPrintDay_PS
#ifdef _DWC_PROTO_
	(
	PrCtx		ctx,
	CalendarDisplay	cd,
	Boolean		ampm,
	Boolean		laser)
#else	/* no prototypes */
	(ctx, cd, ampm, laser)
	PrCtx		ctx;
	CalendarDisplay	cd;
	Boolean		ampm;
	Boolean		laser;
#endif	/* prototype */
{
    struct DWC$db_access_block	*cab = cd->cab;
    Cardinal			repeat_start_day;
    Cardinal			repeat_start_min;
    int				repeat_p1;
    int				repeat_p2;
    int				repeat_p3;
    Cardinal			end_day;
    Cardinal			end_min;
    Cardinal			start_day;
    int				item_id;
    int				start_time;
    int				duration;
    int				duration_days;
    int				alarms_number;
    unsigned short int		*alarms_times;
    int				alarm_time;
    int				was_repeated;
    char			*text;
    int				text_length;
    int				text_class;
    char			*entry_text;
    int				form_width = 80;
    int				entry_flags;
    int				status;
    unsigned char       *icons;
    Cardinal            icons_number;

    PrintDayDate_PS( ctx );
    while
    (
	/*
	 * The DWC$DB_Get_r_item routine has been replaced with the 
	 * DWCDB_GetRItem routine.
	 */
	/*
	status = DWC$DB_Get_r_item
	(
	    cab,
	    ctx->dsbot,
	    &item_id,
	    (int *)&repeat_start_day,
	    (int *)&repeat_start_min,
	    &start_time,
	    &duration_days,
	    &duration, 
	    &alarms_number,
	    &alarms_times,
	    &entry_flags,
	    &text_length,
	    &text,
	    &text_class,
	    &repeat_p1,
	    &repeat_p2,
	    &repeat_p3,
	    (int *)&end_day,
	    (int *)&end_min
	)
	== DWC$k_db_normal
	*/
	status = DWCDB_GetRItem
	(
	    cab,
	    ctx->dsbot,
	    &item_id,
	    (int *)&repeat_start_day,
	    (int *)&repeat_start_min,
	    &start_time,
	    &duration_days,
	    &duration, 
	    &alarms_number,
	    &alarms_times,
	    &entry_flags,
	    (unsigned char **)&text,
	    &text_class,
            &icons_number,
            &icons,
	    &repeat_p1,
	    &repeat_p2,
	    &repeat_p3,
	    (int *)&end_day,
	    (int *)&end_min
	)
	== DWC$k_db_normal
    )
    {
	/*	  
	**  Is this a MEMEX entry holder?
	*/	  
	if (!((start_time == 0) && (duration == 1)))
	{
	    /*
	     * text_length is not returned by DWCDB_GetRItem, so 
	     * we must calculate it here.
	     */
	    text_length = XmStringLength(text);
	    entry_text = (char *) XtMalloc (text_length + 1);
	    memcpy( entry_text, text, text_length );
	    entry_text [text_length] = '\0';
	    fprintf( ctx->printer, "DwcInitEntry\n" );
	    {
		char   *txt;

                /*
                 * The routine PrintIcons_PS is now obselete.  Each entry,
		 * entry_text, at this point in the code contained icon
                 * information.  The PrintIcons_PS routine would 'print'
		 * the icons and return a compund string (txt) without the
		 * icon information.  Txt could then be used to 'print'
		 * the actually text.  Since entry_text is now returned from
		 * DWCDB_GetRItem instead of the older, obselete routine
		 * DWC$DB_Get_r_item, entry_text no longer contains icon
		 * information, so passing it off to PrintIcons_PS is useless.
		 * If you try it, the txt which is returned is garbage.
                 */
#if 0
		txt = PrintIcons_PS
		(
		    ctx,
		    text_class,
		    entry_text,
		    entry_flags,
		    (alarms_number != 0),
		    ( repeat_p1 != 0 )
		);
#endif /* 0 */
		/*
		 * We still have to print the icons.  The icon information 
		 * however is not in entry_text anymore.  Icons and 
		 * icons_number now contain the needed information.  We 
		 * therefore now have a new routine to print the icons.
		 */
		NewPrintIcons_PS ( ctx, 
				entry_text, 
				entry_flags, 
				icons, 
				icons_number, 
				(alarms_number != 0), 
				( repeat_p1 != 0 )
		);
		PrintDayTimes_PS
		    (ctx, start_time, start_time + duration, ampm);
		/*
		 * Original code.
		 *
		 * PrintWrappedText_PS (ctx, txt);
		 *
		 * The PrintWrappedText_PS routine expects 'normal' text.
     		 * We now call the get_text routine, to pull the text out of a 
		 * compund string. 
 		 * We also use entry_text instead of txt because entry_text is
		 * already the 'actual text' without the icon information.
		 */
		PrintWrappedText_PS (ctx, get_text(entry_text));
	    }
	    XtFree (entry_text);
	    fprintf( ctx->printer, "DwcEndEntry\n" );
	}
    }
    fprintf (ctx->printer, "DwcEndDay\n");

    /*
    **  That's it
    */
    return(TRUE);
}

static Boolean DwcPRPrintEntry_PS
#ifdef _DWC_PROTO_
	(
	PrCtx		ctx,
	Cardinal	start_time,
	Cardinal	end_time,
	char		*entry_text,
	Boolean		ampm,
	Boolean		laser)
#else	/* no prototypes */
	(ctx, start_time, end_time, entry_text, ampm, laser)
	PrCtx		ctx;
	Cardinal	start_time;
	Cardinal	end_time;
	char		*entry_text;
	Boolean		ampm;
	Boolean		laser;
#endif	/* prototype */
{
    PrintDayDate_PS( ctx );
    fprintf( ctx->printer, "DwcInitEntry\n" );
    PrintDayTimes_PS( ctx, start_time, end_time, ampm );
    PrintWrappedText_PS( ctx, entry_text );
    fprintf( ctx->printer, "DwcEndEntry\n" );
    fprintf( ctx->printer, "DwcEndDay\n" );
    return TRUE;
}

static void PrintDayDate_TEXT
#ifdef _DWC_PROTO_
	(
	PrCtx		ctx,
	Cardinal	form_width)
#else	/* no prototypes */
	(ctx, form_width)
	PrCtx		ctx;
	Cardinal	form_width;
#endif	/* prototype */
{
    dtb		date_time;
    char	*text;
    int		l, w;


    DATEFUNCDateForDayNumber
	(ctx->dsbot, &date_time.day, &date_time.month, &date_time.year);

    date_time.weekday = DATEFUNCDayOfWeek
	(date_time.day, date_time.month, date_time.year);

    text = DATEFORMATTimeToText (dwc_k_print_full_date_format, &date_time);
    l = strlen (text);
    w = l + MAX (0, (form_width - l) / 2);
    fprintf( ctx->printer, "%*s\n\n", w, text );
    XtFree (text);
	
}

static void PrintDayTimes_TEXT
#ifdef _DWC_PROTO_
	(
	PrCtx		ctx,
	Cardinal	start_time,
	Cardinal	end_time,
	Boolean		ampm,
	Cardinal	form_width)
#else	/* no prototypes */
	(ctx, start_time, end_time, ampm, form_width)
	PrCtx		ctx;
	Cardinal	start_time;
	Cardinal	end_time;
	Boolean		ampm;
	Cardinal	form_width;
#endif	/* prototype */
    {
    dtb		date_time;
    char	*dashes;
    char	*text;
    int		i, l, w;
    XmString	cs;
    long	cvt_size, cvt_status;
        
    dashes = XtMalloc (form_width + 1);
    for (i = 0;  i < form_width;  i++)
    {
	dashes [i] = '-';
    }
    dashes [form_width] = '\0';

    /*	  
    **  Is this a daynote?
    */	  
    if(( start_time == end_time ) && ( end_time == 0 ))
    {
	cs = MISCFetchDRMValue(dwc_k_print_daynote_text);   /* don't free CS */
	text = (char *)DXmCvtCStoFC(cs, &cvt_size, &cvt_status);

	l = strlen (text);
	w = MAX (0, form_width - l);
	fprintf( ctx->printer, "%s%0.*s\n", text, w, dashes);
	fprintf (ctx->printer, "\n\n", text);

	XtFree(text);
	return;
    }

    date_time.hour   = start_time / 60;
    date_time.minute = start_time % 60;
    if (ampm)
    {
	text = DATEFORMATTimeToText
	    (dwc_k_print_dash_from_ampm_fmt, &date_time);
    }
    else
    {
	text = DATEFORMATTimeToText (dwc_k_print_dash_from_fmt, &date_time);
    }
    l = strlen (text);
    w = MAX (0, form_width - l);
    fprintf( ctx->printer, "%s%0.*s\n", text, w, dashes);

    XtFree (text);
    XtFree (dashes);

    date_time.hour   = end_time / 60;
    date_time.minute = end_time % 60;
    if (ampm)
    {
	text = DATEFORMATTimeToText (dwc_k_print_dash_to_ampm_fmt, &date_time);
    }
    else
    {
	text = DATEFORMATTimeToText (dwc_k_print_dash_to_fmt, &date_time);
    }
    fprintf (ctx->printer, "%s\n\n", text);
    XtFree (text);

}

Boolean DwcPRPrintEntry_TEXT
#ifdef _DWC_PROTO_
	(
	PrCtx		ctx,
	Cardinal	start_time,
	Cardinal	end_time,
	char		*entry_text,
	Boolean		ampm,
	Boolean		laser)
#else	/* no prototypes */
	(ctx, start_time, end_time, entry_text, ampm, laser)
	PrCtx		ctx;
	Cardinal	start_time;
	Cardinal	end_time;
	char		*entry_text;
	Boolean		ampm;
	Boolean		laser;
#endif	/* prototype */
{
    int		form_width = 80;

    PrintDayDate_TEXT     (ctx, form_width);
    PrintDayTimes_TEXT    (ctx, start_time, end_time, ampm, form_width);
    PrintWrappedText_TEXT (ctx, entry_text, form_width );

    fputs  (STR_FF, ctx->printer);
    fputs  ("\n",   ctx->printer);
    return (TRUE);

}

Boolean DwcPRPrintDay_TEXT
#ifdef _DWC_PROTO_
	(
	PrCtx		ctx,
	CalendarDisplay	cd,
	Boolean		ampm,
	Boolean		laser)
#else	/* no prototypes */
	(ctx, cd, ampm, laser)
	PrCtx		ctx;
	CalendarDisplay	cd;
	Boolean		ampm;
	Boolean		laser;
#endif	/* prototype */
{
    struct DWC$db_access_block	*cab = cd->cab;
    Cardinal			repeat_start_day;
    Cardinal			repeat_start_min;
    int				repeat_p1;
    int				repeat_p2;
    int				repeat_p3;
    Cardinal			end_day;
    Cardinal			end_min;
    Cardinal			start_day;
    int				item_id;
    int				start_time;
    int				duration;
    int				duration_days;
    int				alarms_number;
    unsigned short int		*alarms_times;
    int				alarm_time;
    int				was_repeated;
    char			*text;
    int				text_length;
    int				text_class;
    char			*entry_text;
    int				form_width = 80;
    int				entry_flags;
    int				status;
    unsigned char       *icons;
    Cardinal            icons_number;

    PrintDayDate_TEXT( ctx, form_width);

    while
    (
	/*
	 * The DWC$DB_Get_r_item routine has been replaced with the 
	 * DWCDB_GetRItem routine.
	 */
	/*
	status = DWC$DB_Get_r_item
	(
	    cab,
	    ctx->dsbot,
	    &item_id,
	    (int *)&repeat_start_day,
	    (int *)&repeat_start_min,
	    &start_time,
	    &duration_days,
	    &duration, 
	    &alarms_number,
	    &alarms_times,
	    &entry_flags,
	    &text_length,
	    &text,
	    &text_class,
	    &repeat_p1,
	    &repeat_p2,
	    &repeat_p3,
	    (int *)&end_day,
	    (int *)&end_min
	)
	== DWC$k_db_normal
	*/
	status = DWCDB_GetRItem
	(
	    cab,
	    ctx->dsbot,
	    &item_id,
	    (int *)&repeat_start_day,
	    (int *)&repeat_start_min,
	    &start_time,
	    &duration_days,
	    &duration, 
	    &alarms_number,
	    &alarms_times,
	    &entry_flags,
	    (unsigned char **)&text,
	    &text_class,
            &icons_number,
            &icons,
	    &repeat_p1,
	    &repeat_p2,
	    &repeat_p3,
	    (int *)&end_day,
	    (int *)&end_min
	)
	== DWC$k_db_normal
    )
    {
	/*	  
	**  Is this a MEMEX entry holder?
	*/	  
	if (!((start_time == 0) && (duration == 1)))
	{
	    /*
	     * text_length is not returned by DWCDB_GetRItem, so 
	     * we must calculate it here.
	     */
	    text_length = XmStringLength(text);
	    entry_text = (char *) XtMalloc (text_length + 1);
	    memcpy( entry_text, text, text_length );
	    entry_text [text_length] = '\0';

	    {
		char *txt;
		txt = PrintIcons_TEXT( ctx, text_class, entry_text, entry_flags,
					( alarms_number != 0 ),
					( repeat_p1 != 0 ));
		PrintDayTimes_TEXT( ctx, start_time, start_time + duration,
					ampm,form_width);
		/*
		 * Original code.
		 *
		 * PrintWrappedText_TEXT( ctx, entry_text, form_width);
		 *
		 * The PrintWrappedText_TEXT routine expects 'normal' text.
     		 * We now call the get_text routine, pull text out of a 
		 * compund string. 
		 */
		PrintWrappedText_TEXT( ctx, get_text(entry_text), form_width);
	    }

	    XtFree (entry_text);
	    fputs ("\n", ctx->printer);
	}
    }

    fputs  (STR_FF, ctx->printer);
    fputs  ("\n",   ctx->printer);
    return (TRUE);

}

static void PrintWrappedText_DDIF
#ifdef _DWC_PROTO_
	(
	PrCtx		ctx,
	char		*entry_text)
#else	/* no prototypes */
	(ctx, entry_text)
	PrCtx		ctx;
	char		*entry_text;
#endif	/* prototype */
{
    int		lines, start, current, spaces, nonspace, width;

    /*
    **  Output text, wrapping on the way.
    **
    **	lines	    the number of lines printed so far,
    **	start	    is the position along the text from which the next output
    **		    will be done,
    **	current	    is the position along the text at which we are currently
    **		    looking,
    **	spaces	    is the position of first of the last set of contiguous
    **		    spaces or tabs seen,
    **	nonspace    is the position of the last non-space after "spaces",
    **	width	    is the width of the current line so far.
    */
    
    DWCCDA_Segment_Type (ctx->cda, "DWC$DV_EntryText");
    DWCCDA_Enter_Segment (ctx->cda);
    DWCCDA_Text (ctx->cda, entry_text);
    DWCCDA_Leave_Segment (ctx->cda);
}

static void PrintDayDate_DDIF
#ifdef _DWC_PROTO_
	(
	PrCtx	ctx)
#else	/* no prototypes */
	(ctx)
	PrCtx	ctx;
#endif	/* prototype */
{
    dtb		date_time;
    char	*text;


    DATEFUNCDateForDayNumber
	(ctx->dsbot, &date_time.day, &date_time.month, &date_time.year);

    date_time.weekday = DATEFUNCDayOfWeek
	(date_time.day, date_time.month, date_time.year);

    text = DATEFORMATTimeToText (dwc_k_print_full_date_format, &date_time);

    DWCCDA_Segment_Type( ctx->cda,	"DWC$DV_DayDate");
    DWCCDA_Enter_Segment( ctx->cda);
    DWCCDA_char( ctx->cda, text);
    DWCCDA_Sft_New_Line( ctx->cda);
    DWCCDA_Leave_Segment( ctx->cda);

    XtFree (text);
	
}

static void PrintDayTimes_DDIF
#ifdef _DWC_PROTO_
	(
	PrCtx		ctx,
	Cardinal	start_time,
	Cardinal	end_time,
	Boolean		ampm)
#else	/* no prototypes */
	(ctx, start_time, end_time, ampm)
	PrCtx		ctx;
	Cardinal	start_time;
	Cardinal	end_time;
	Boolean		ampm;
#endif	/* prototype */
{
    dtb		date_time;
    char	*text;
    XmString	cs;
    long	cvt_size, cvt_status;

    /*	  
    **  Is this a daynote?
    */	  
    if(( start_time == end_time ) && ( end_time == 0 ))
    {
	cs = MISCFetchDRMValue(dwc_k_print_daynote_text);   /* don't free CS */
	text = (char *)DXmCvtCStoFC(cs, &cvt_size, &cvt_status);

	DWCCDA_Segment_Type( ctx->cda, "DWC$DV_DayNotesText");
	DWCCDA_Enter_Segment( ctx->cda);
	DWCCDA_Sft_New_Line( ctx->cda);
	DWCCDA_Sft_New_Line( ctx->cda);
	DWCCDA_Text( ctx->cda, text);
	DWCCDA_Sft_New_Line( ctx->cda);
	DWCCDA_Sft_New_Line( ctx->cda);
	DWCCDA_Sft_New_Line( ctx->cda);
	DWCCDA_Leave_Segment( ctx->cda);

	XtFree (text);
	return;
    }

    date_time.hour   = start_time / 60;
    date_time.minute = start_time % 60;
    if (ampm)
    {
	text = DATEFORMATTimeToText (dwc_k_print_time_ampm_format, &date_time);
    }
    else
    {
	text = DATEFORMATTimeToText (dwc_k_print_time_format, &date_time);
    }

    DWCCDA_Segment_Type( ctx->cda, "DWC$DV_DayTimeFrom");
    DWCCDA_Enter_Segment( ctx->cda);
    DWCCDA_Sft_New_Line( ctx->cda);
    DWCCDA_Sft_New_Line( ctx->cda);
    DWCCDA_char( ctx->cda, text);
    DWCCDA_Sft_New_Line( ctx->cda);
    DWCCDA_Leave_Segment( ctx->cda);
    XtFree (text);

    date_time.hour   = end_time / 60;
    date_time.minute = end_time % 60;
    if (ampm)
    {
	text = DATEFORMATTimeToText (dwc_k_print_time_ampm_format, &date_time);
    }
    else
    {
	text = DATEFORMATTimeToText (dwc_k_print_time_format, &date_time);
    }

    DWCCDA_Segment_Type( ctx->cda,	"DWC$DV_DayTimeTo");
    DWCCDA_Enter_Segment( ctx->cda);
    DWCCDA_char( ctx->cda, text);
    DWCCDA_Sft_New_Line( ctx->cda);
    DWCCDA_Sft_New_Line( ctx->cda);
    DWCCDA_Leave_Segment( ctx->cda);
    XtFree (text);
}

Boolean DwcPRPrintEntry_DDIF
#ifdef _DWC_PROTO_
	(
	PrCtx		ctx,
	Cardinal	start_time,
	Cardinal	end_time,
	char		*entry_text,
	Boolean		ampm,
	Boolean		laser)
#else	/* no prototypes */
	(ctx, start_time, end_time, entry_text, ampm, laser)
	PrCtx		ctx;
	Cardinal	start_time;
	Cardinal	end_time;
	char		*entry_text;
	Boolean		ampm;
	Boolean		laser;
#endif	/* prototype */
{
    int				form_width  = 80;

    DWCCDA_Segment_Type( ctx->cda,	"DWC$DV_Entry");
    DWCCDA_Enter_Segment( ctx->cda);

    PrintDayDate_DDIF     (ctx );
    PrintDayTimes_DDIF    (ctx, start_time, end_time, ampm );
    PrintWrappedText_DDIF (ctx, entry_text );

    DWCCDA_Hrd_New_Page( ctx->cda);
    DWCCDA_Leave_Segment( ctx->cda);
    return (TRUE);

}

Boolean DwcPRPrintDay_DDIF
#ifdef _DWC_PROTO_
	(
	PrCtx		ctx,
	CalendarDisplay	cd,
	Boolean		ampm,
	Boolean		laser)
#else	/* no prototypes */
	(ctx, cd, ampm, laser)
	PrCtx		ctx;
	CalendarDisplay	cd;
	Boolean		ampm;
	Boolean		laser;
#endif	/* prototype */
{
    struct DWC$db_access_block	*cab = cd->cab;
    Cardinal			repeat_start_day;
    Cardinal			repeat_start_min;
    int				repeat_p1;
    int				repeat_p2;
    int				repeat_p3;
    Cardinal			end_day;
    Cardinal			end_min;
    Cardinal			start_day;
    int				item_id;
    int				start_time;
    int				duration;
    int				duration_days;
    int				alarms_number;
    unsigned short int		*alarms_times;
    int				alarm_time;
    int				was_repeated;
    char			*text;
    int				text_length;
    int				text_class;
    char			*entry_text;
    int				form_width = 80;
    int				status;
    int				entry_flags;


    DWCCDA_Segment_Type( ctx->cda,	"DWC$DV_Day");
    DWCCDA_Enter_Segment( ctx->cda);

    PrintDayDate_DDIF( ctx );

    while
    (
	status = DWC$DB_Get_r_item
	(
	    cab,
	    ctx->dsbot,
	    &item_id,
	    (int *)&repeat_start_day,
	    (int *)&repeat_start_min,
	    &start_time,
	    &duration_days,
	    &duration, 
	    &alarms_number,
	    &alarms_times,
	    &entry_flags,
	    &text_length,
	    &text,
	    &text_class,
	    &repeat_p1,
	    &repeat_p2,
	    &repeat_p3,
	    (int *)&end_day,
	    (int *)&end_min
	)
	== DWC$k_db_normal
    )
    {
	/*	  
	**  Is this a MEMEX entry holder?
	*/	  
	if (!((start_time == 0) && (duration == 1)))
	{
	    entry_text = (char *) XtMalloc (text_length + 1);
	    memcpy( entry_text, text, text_length );
	    entry_text [text_length] = '\0';
	    {
		char   *txt;

		txt = PrintIcons_DDIF( ctx, text_class, entry_text, entry_flags,
					( alarms_number != 0 ),
					( repeat_p1 != 0 ));
		PrintDayTimes_DDIF( ctx, start_time, start_time + duration, ampm );
		PrintWrappedText_DDIF( ctx, txt );
	    }
	    XtFree (entry_text);
	}
    }

    if (status == DWC$k_db_failure)
    {
	ERRORReportErrno( cd->mainwid, "ErrorGetDayItems", NULL, NULL);
	return (FALSE);
    }

    DWCCDA_Hrd_New_Page( ctx->cda);
    DWCCDA_Leave_Segment(ctx->cda);
    return (TRUE);

}

void FILE_PRINT_MENU
#ifdef _DWC_PROTO_
	(
	Widget	w,
	caddr_t	tag,
	int	*reason)
#else	/* no prototypes */
	(w, tag, reason)
	Widget	w;
	caddr_t	tag;
	int	*reason;
#endif	/* prototype */
{
    Arg			argv[10];
    int			argc;
    dtb			date_time;
    Cardinal		start_time;
    Cardinal		end_time;
    XmString		text;
    XmString		empty_text;
    CalendarDisplay	cd;
    int			status;
    XtWidgetGeometry	request;
    XtGeometryResult	result;


    status = MISCFindCalendarDisplay (&cd, w);

    ICONSWaitCursorDisplay (cd->mainwid, cd->ads->wait_cursor);

    if (!cd->db_print_created)
    {
    	static MrmRegisterArg   NamesList[] =
	{
	    {"DwcPrintOK",		(caddr_t) PRINT_OK},
	    {"DwcPrintCancel",		(caddr_t) PRINT_CANCEL},
	    {"DwcPrintDgLimitLines",	(caddr_t) PRINT_LIMIT},
	    {"DwcPrintDgOK",		(caddr_t) PRINT_DG_OK},
	    {"DwcPrintDgCancel",	(caddr_t) PRINT_DG_CANCEL},
	    {"DwcPrintDgOptions",	(caddr_t) PRINT_DG_OPTIONS},
	    {"DwcPrintDgTextChanged",	(caddr_t) PRINT_DG_TEXT_CHANGED}
	};
	MrmType			class;
	int			status;

	MrmRegisterNames (NamesList, XtNumber(NamesList));

	status = MrmFetchWidget
	(
	    cd->ads->hierarchy,
	    "DwcPrintWidget",
	    cd->toplevel,
	    &cd->db_print,
	    &class
	);
	if (status != MrmSUCCESS)
	    DWC$UI_Catchall (DWC$DRM_NOPRINTWIDGET, status, 0);

	status = MrmFetchWidget
	(
	    cd->ads->hierarchy,
	    "DwcPrintDialog",
	    cd->toplevel,
	    &cd->db_print_dg,
	    &class
	);
	if (status != MrmSUCCESS)
	    DWC$UI_Catchall(DWC$DRM_NOPRINTDIALOG, status, 0);

	cd->db_print_created = True;
	cd->db_print_up = False;
	cd->db_print_text_changed = True;
    }

    start_time	    = cd->print_arg_start;
    end_time	    = cd->print_arg_end;
    date_time.day   = cd->print_arg_day;
    date_time.month = cd->print_arg_month;
    date_time.year  = cd->print_arg_year;

    empty_text = XmStringCreateSimple("");

    if (cd->print_arg_type == MwWeekSelected)
    {
	/*
	** don't free this CS
	*/
	text = (XmString) MISCFetchDRMValue (dwc_k_pw_week_starting);
	XtSetArg(argv[0], XmNlabelString, text);
	XtSetValues (cd->db_print_lb_week, argv, 1);
    }
    else
    {
	XtSetArg(argv[0], XmNlabelString, empty_text);
	XtSetValues (cd->db_print_lb_week, argv, 1);
    }

    date_time.weekday = DATEFUNCDayOfWeek
	(date_time.day, date_time.month, date_time.year);

    text = DATEFORMATTimeToCS (dwc_k_print_full_date_format, &date_time);
    XtSetArg (argv[0], XmNlabelString, text);
    XtSetValues (cd->db_print_lb_date, argv, 1);
    XmStringFree (text);

    if (cd->print_arg_type != MwNothingSelected)
    {
	XtSetArg(argv[0], XmNlabelString, empty_text);
	XtSetValues (cd->db_print_lb_start,  argv, 1);
	XtSetValues (cd->db_print_lb_finish, argv, 1);
    }
    else
    {
	date_time.hour   = start_time / 60;
	date_time.minute = start_time % 60;
	if (cd->profile.time_am_pm)
	{
	    text = DATEFORMATTimeToCS
		(dwc_k_print_from_ampm_format, &date_time);
	}
	else
	{
	    text = DATEFORMATTimeToCS (dwc_k_print_from_format, &date_time);
	}
	XtSetArg (argv[0], XmNlabelString, text);
	XtSetValues (cd->db_print_lb_start, argv, 1);
	XmStringFree (text);

	date_time.hour   = end_time / 60;
	date_time.minute = end_time % 60;
	if (cd->profile.time_am_pm)
	{
	    text = DATEFORMATTimeToCS (dwc_k_print_to_ampm_format, &date_time);
	}
	else
	{
	    text = DATEFORMATTimeToCS (dwc_k_print_to_format, &date_time);
	}
	XtSetArg (argv[0], XmNlabelString, text);
	XtSetValues (cd->db_print_lb_finish, argv, 1);
	XmStringFree (text);
    }

    XmStringFree (empty_text);
    
    if (cd->db_print_up)
    {
	/*
	**  Make sure it is on top
	*/
	request.request_mode = CWStackMode;
	request.stack_mode   = Above;
	result = XtMakeGeometryRequest (cd->db_print_dg, &request, NULL);
    }
    else
    {
	/*
	** Only need to do the protocol add the 1st time.
	*/
	Boolean	protocol = (XtWindow(cd->db_print_dg) == 0);

	/*
	** Get focus when it comes up.
	*/
	MISCFocusOnMap (XtParent(cd->db_print_dg), NULL);

        /*
        **  Pop up our dialog box
        */
        XtManageChild (cd->db_print_dg);
	cd->db_print_up = True;

	/*
	** Make the CLOSE item in the window manager menu call the CANCEL
	** callback.
	*/
	if (protocol)
	{
	    MISCAddProtocols
	    (
		XtParent(cd->db_print_dg),
		(XtCallbackProc) PRINT_DG_CANCEL,
		NULL
	    );
	}
    }
    
    ICONSWaitCursorRemove (cd->mainwid);
}

/*
**++
**  Functional Description:
**	print_initialize
**
**  Keywords:
**	None.
**
**  Arguments:
**	TBD.
**
**  Result:
**	0:  No error occured
**	1:  An error occured, errno isn't available
**	2:  An error occured, errno is available.
**
**  Exceptions:
**	None.
**--
*/
static int print_initialize
#ifdef _DWC_PROTO_
	(
	PrCtx	ctx,
	Display	*dpy)
#else	/* no prototypes */
	(ctx, dpy)
	PrCtx	ctx;
	Display	*dpy;
#endif	/* prototype */
{
    char		*filespec;
    Boolean		status;
    
    switch( ctx->print_format )
    {
    case PS:
	ctx->print_day	= DwcPRPrintDay_PS;
	ctx->print_entry	= DwcPRPrintEntry_PS;
	/*
	 * This may be a problem later on, but I can't find it now.
	 * The routine PrintIcons_PS should not be called anymore.
	 * But I don't know what changing it here will do.  See the comments
	 * in DwcPRPrintDay_PS.
	 */
	ctx->print_icons	= PrintIcons_PS;
#ifdef VMS
	ctx->printer = fopen( ctx->filespec, "w", "rfm=var", "rat=cr" );
#else
	ctx->printer = fopen( ctx->filespec, "w" );
#endif
	if( !ctx->printer ) return(2);

	{
	    DWCCDA_TextFileCtxRec  cda_ctx_rec;
	    DWCCDA_TextFileCtx	    cda_ctx;
	    char		    *line;
	    cda_ctx = &cda_ctx_rec;

	    filespec = prolog_filespec();
#ifdef VMS
	    status = DWCCDA_Open_Text_File
		(cda_ctx, filespec, "SYS$LIBRARY:.PS");
#else
	    status = DWCCDA_Open_Text_File
		(cda_ctx, filespec, NULL);
#endif
	    if (!status) return(1);
		    
	    while (DWCCDA_Read_Text_File( cda_ctx ))
		fprintf
		(
		    ctx->printer,
		    "%0.*s\n",
		    cda_ctx->length,
		    cda_ctx->buffer
		);

	    status = DWCCDA_Close_Text_File( cda_ctx );

	    if (!status) return(1);
		
	}

	create_icon_font( dpy, ctx->printer );
	fprintf( ctx->printer, "%d DwcSetPageSize\n", ctx->pagesize );
	fputs( "DwcInit\n", ctx->printer );

	if( !ctx->line_limit )
	    fputs( "/DwcLimitLines false def\n", ctx->printer );
	else
	    fprintf	(ctx->printer, "/DwcLineLimit %d def\n",
			ctx->line_limit);

	break;

    case TEXT:
	ctx->print_day	= DwcPRPrintDay_TEXT;
	ctx->print_entry	= DwcPRPrintEntry_TEXT;
	ctx->print_icons	= PrintIcons_TEXT;
#ifdef VMS
	ctx->printer = fopen( ctx->filespec, "w", "rfm=var", "rat=cr" );
#else
	ctx->printer = fopen( ctx->filespec, "w" );
#endif
	if( !ctx->printer ) return(2);
	break;



    case DDIF:
	ctx->print_day	= DwcPRPrintDay_DDIF;
	ctx->print_entry	= DwcPRPrintEntry_DDIF;
	ctx->print_icons	= PrintIcons_DDIF;

	DWCCDA_Init (ctx->cda, ctx->filespec, "DWC", "DECWindows Calendar V3", 1);

	break;


    default:
	return(1);
	break;
    }

    return(0);
}

static void print_rundown
#ifdef _DWC_PROTO_
	(
	PrCtx	ctx)
#else	/* no prototypes */
	(ctx)
	PrCtx	ctx;
#endif	/* prototype */
{
    switch( ctx->print_format )
    {
    case PS:
	fprintf( ctx->printer, "DwcEndJob\n" );
    case TEXT:
	fclose( ctx->printer );
	break;
    case DDIF:
	DWCCDA_Finish(ctx->cda);
	break;
    default:
	break;
    }
}

static void PRINT_OK
#ifdef _DWC_PROTO_
	(
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
/**
*** This is the OK response in the Print widget.  Take down the print widget
*** and do the print dialog's OK behavior.
**/
{
    CalendarDisplay	cd;

    MISCFindCalendarDisplay (&cd, w);

    /*
    **  take down the printwidget's dialog box
    */
    XtUnmanageChild (cd->db_print);

    /*
    ** Call print OK.
    */
    PRINT_DG_OK (cd->db_print_dg, NULL, NULL);

}

static void set_print_format
#ifdef _DWC_PROTO_
	(CalendarDisplay cd)
#else
	(cd)
	CalendarDisplay	cd;
#endif
{
    XmString		xm_filespec;
    XmString		format_choice = NULL;

    xm_filespec = DXmCSTextGetString (cd->tw_print);
#if !defined (VMS)
    if (xm_filespec)
    {
	char	*filespec;
	long	byte_count, cvt_status;
	filespec = DXmCvtCStoFC
	    (xm_filespec, &byte_count, &cvt_status);
	/*
	 * DDIF format is really busted, so remove the option
	 * from the user.
	 */
	/*
	if ((byte_count > 4) &&
	    (strcmp (&filespec[byte_count-5], ".ddif") == 0))
	{
	    format_choice = DXmCvtFCtoCS
		("DDIF", &byte_count, &cvt_status);
	}
	else 
	*/ 
	if ((byte_count > 3) &&
	    (strcmp (&filespec[byte_count-4], ".txt") == 0))
	{
	    format_choice = DXmCvtFCtoCS
		("Text", &byte_count, &cvt_status);
	}
	else
	{
	    format_choice = DXmCvtFCtoCS
		("PostScript(R)", &byte_count, &cvt_status);
	}
	XtFree (filespec);
    }
    else
    {
	long	byte_count, cvt_status;
	format_choice = DXmCvtFCtoCS
	    ("PostScript(R)", &byte_count, &cvt_status);
    }
#endif

    /*
    ** Bring up the print widget.
    */
    XtVaSetValues
    (
	cd->db_print,
#if defined(VMS)
	DXmNfileNameList, &xm_filespec,
	DXmNfileNameCount, 1,
#else
	DXmNprintFormatChoice, format_choice,
#endif
	NULL
    );

    if (format_choice) XmStringFree (format_choice);
    XmStringFree (xm_filespec);

}

static void PRINT_DG_OK
#ifdef _DWC_PROTO_
	(
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    PrContextRec	pr_ctx_rec;
    PrCtx		pr_ctx;
    Cardinal		day;
    Cardinal		month;
    Cardinal		year;
    Boolean		status;
    Cardinal		i;
    CalendarDisplay	cd;
    int			foo;
    Arg			arglist[1];
    Arg			argv[10];
    int			argc;
    Time		time;
    Boolean		send_to_printer;
    int			returned_value;
    Opaque		fc_format_fromprtwgt;
    static Opaque	fc_format_PS	= (Opaque) 0;
    static Opaque	fc_format_DDIF	= (Opaque) 0;
    XmString		format_fromprtwgt;
    static XmString	format_PS	= (XmString) 0;
    static XmString	format_DDIF	= (XmString) 0;
    XmString		xm_filespec;
    char		*print_arg_az;
    long		byte_count, cvt_status;
               
    pr_ctx_rec.cda = &pr_ctx_rec.cda_rec;

    foo =  MISCFindCalendarDisplay (&cd, w);

    if (cd->db_print_up)
    {
	XtUnmanageChild (cd->db_print_dg);
	cd->db_print_up = False;

	if (cd->db_print_text_changed) set_print_format (cd);
	cd->db_print_text_changed = False;
    }

    ICONSWaitCursorDisplay (cd->mainwid,  cd->ads->wait_cursor);

    send_to_printer = XmToggleButtonGetState (cd->tb_print_append);

    /*
    ** Make sure the pixmaps are loaded for printing.
    */
    MISCLoadPixmapArrays (cd->mainwid, 2);

    /*
    ** Check to see if it is there, in case no printwgt uid file was found
    */
    format_fromprtwgt = (XmString)NULL;
    if (cd->db_print != NULL)
    {
	argc = 0;
	XtSetArg (argv[argc], DXmNpageSize, &pr_ctx_rec.pagesize); argc++;
	XtSetArg (argv[argc], DXmNprintFormatChoice, &format_fromprtwgt); argc++;
	XtGetValues( cd->db_print, argv, argc );
    }

    /* Don't free format_fromprtwgt or you'll have problems */

    if( XmToggleButtonGetState( cd->tb_print_limit ))
    {
	argc = 0;
	XtSetArg (argv[argc], XmNvalue, &pr_ctx_rec.line_limit); argc++;
	XtGetValues( cd->sw_print, argv, argc );
    }
    else
	pr_ctx_rec.line_limit = 0;

    /*	  
    **  Initialize our formats if they haven't been done already
    */	  
    if( fc_format_PS == (Opaque)0)
    {
	fc_format_PS = (Opaque) "PostScript(R)";
    }		

    if( fc_format_DDIF == (Opaque)0)
    {
	fc_format_DDIF = (Opaque) "DDIF";
    }    

    /*
    **  We need to see what the format is, so convert from CS to FC
    */
    fc_format_fromprtwgt = (Opaque)DXmCvtCStoFC
	(format_fromprtwgt, &byte_count, &cvt_status);

    /*	  
    **  strcmp can handle FC
    */
    if (strcmp (fc_format_PS, fc_format_fromprtwgt) == 0)
	pr_ctx_rec.print_format = PS;
    else if (strcmp (fc_format_DDIF, fc_format_fromprtwgt) == 0)
	pr_ctx_rec.print_format = DDIF;
    else
	pr_ctx_rec.print_format = TEXT;

    XtFree (fc_format_fromprtwgt);

    xm_filespec = DXmCSTextGetString (cd->tw_print);
    if (xm_filespec)
    {
	pr_ctx_rec.filespec = DXmCvtCStoFC
	    (xm_filespec, &byte_count, &cvt_status);
    }
    else
    {
	pr_ctx_rec.filespec = XtMalloc(1);
	pr_ctx_rec.filespec[0] = '\0';
    }

    pr_ctx_rec.dsbot = DATEFUNCDaysSinceBeginOfTime
	(cd->print_arg_day, cd->print_arg_month, cd->print_arg_year);

    returned_value = print_initialize( &pr_ctx_rec, XtDisplay( w ));

    status = !returned_value;

    if (returned_value == 1)
    {
	ERRORDisplayError (cd->mainwid, "ErrorCantGetProlog");
    }
    else if (returned_value == 2)
    {
	ERRORReportErrno (cd->mainwid, "ErrorCantPrint", NULL, NULL);
    }

    if (status)
    {
	switch (cd->print_arg_type)
	{
	case MwNothingSelected :
	    print_arg_az = DXmCvtCStoFC
		(cd->print_arg_text, &byte_count, &cvt_status);
	    status = (*pr_ctx_rec.print_entry)
	    (
		&pr_ctx_rec,
		cd->print_arg_start,
		cd->print_arg_end,
		print_arg_az,
		cd->profile.time_am_pm,
		FALSE
	    );

	    XtFree (print_arg_az);
	    XmStringFree (cd->print_arg_text);
	    cd->print_arg_text = NULL;
	    break;
	    
	case MwDaySelected :
	    status = ( *pr_ctx_rec.print_day )
	    (
		&pr_ctx_rec,
		cd,
		cd->profile.time_am_pm,
		FALSE
	    );
	    break;

	  case MwWeekSelected :
	    for (i = 0;  i < 7;  i++)
	    {
		DATEFUNCDateForDayNumber
		    (pr_ctx_rec.dsbot, (int *)&day, (int *)&month,(int *)&year);
		if ((cd->profile.print_blank_days) ||
		    (MISCTestDayConditions (cd, FALSE, TRUE, day, month, year)))
		{
		    status = ( *pr_ctx_rec.print_day )
			(&pr_ctx_rec, cd, cd->profile.time_am_pm, FALSE);
		}
		pr_ctx_rec.dsbot++;
	    }
	    break;
	}
	
	if (!status)
	{
	    ERRORReportErrno (cd->mainwid, "ErrorCantPrint", NULL, NULL);
	}
    }


    if (status)
    {
	print_rundown( &pr_ctx_rec );

	if( send_to_printer )
	{
	    XmString	filenames[1];

	    filenames[0] = XmStringCreateSimple(pr_ctx_rec.filespec);

	    status = DXmPrintWgtPrintJob
		(cd->db_print, filenames, XtNumber(filenames));

	    XmStringFree (filenames[0]);

	    if (!status)
	    {
		ERRORReportErrno (cd->mainwid, "ErrorPrintWont", NULL, NULL);
	    }
	}
    } 

    XtFree( pr_ctx_rec.filespec );

    ICONSWaitCursorRemove (cd->mainwid);

    return;
}

static void PRINT_OPTIONS_OK
#ifdef _DWC_PROTO_
	(Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    CalendarDisplay	cd;

    /*
    ** Identify which Calendar.
    */
    (void) MISCFindCalendarDisplay (&cd, w);

    /*
    ** Take down the print widget.
    */
    XtUnmanageChild (w);

}

static void PRINT_DG_OPTIONS
#ifdef _DWC_PROTO_
	(Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
/**
*** Bring up the print widget.
**/
{
    CalendarDisplay	cd;

    MISCFindCalendarDisplay( &cd, w );

    /*
    ** Get focus when it comes up.
    */
    MISCFocusOnMap (XtParent(cd->db_print), NULL);

    /*
    ** Set the print format choice to match the file spec.
    */
    if (cd->db_print_text_changed) set_print_format (cd);
    cd->db_print_text_changed = False;

    /*
    ** Put up the print widget.
    */
    XtManageChild (cd->db_print);

    /*
    ** Take down the print dialog.
    */
    XtUnmanageChild (cd->db_print_dg);
    cd->db_print_up = False;
}

static void PRINT_DG_TEXT_CHANGED
#ifdef _DWC_PROTO_
	(Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    CalendarDisplay	cd;

    MISCFindCalendarDisplay (&cd, w);

    cd->db_print_text_changed = True;
}

static void PRINT_CANCEL
#ifdef _DWC_PROTO_
	(
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    CalendarDisplay	cd;
    int			status;
    Time		time;
        
    status = MISCFindCalendarDisplay (&cd, w);

    XtUnmanageChild (cd->db_print);
}

static void PRINT_DG_CANCEL
#ifdef _DWC_PROTO_
	(
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    CalendarDisplay	cd;
    int			status;
    Time		time;
    
    status = MISCFindCalendarDisplay (&cd, w);
    XtUnmanageChild (cd->db_print_dg);
    cd->db_print_up = False;

}

static void PRINT_LIMIT
#ifdef _DWC_PROTO_
	(
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    CalendarDisplay	cd;
    (void) MISCFindCalendarDisplay( &cd, w );
    XtSetSensitive( cd->sw_print, XmToggleButtonGetState( w ));
}

/* prolog_filespec() returns the name of the calendar prolog print file for */
/* this particular user.  On VMS this is constant, on non-VMS systems, will */
/* look in the env for an alternative file before picking the one in the    */
/* standard place							    */
char	*prolog_filespec
#ifdef _DWC_PROTO_
	()
#else	/* no prototypes */
	()
#endif	/* prototype */
{
    char	*ret;
    char	buf[512];

#ifdef VMS
    ret = "DECW$CALENDAR_PROLOG";
#else
    if (!(ret = (char *)getenv("DXCALENDAR_PROLOG")))
    {
	sprintf(buf, "%s", "/usr/lib/X11/app-defaults/dxcalendar_prolog.ps");
	ret = (char *)XtNewString(buf);
    }
    else
	ret = (char *)XtNewString(ret);
#endif /* VMS */


    return ret;
}
