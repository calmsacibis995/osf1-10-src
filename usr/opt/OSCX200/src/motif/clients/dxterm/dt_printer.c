/*
 *  Title:	DT_printer
 *
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © Digital Equipment Corporation, 1988, 1993 All Rights       |
 *  | Reserved.  Unpublished rights reserved under the copyright laws of     |
 *  | the United States.                                                     |
 *  |                                                                        |
 *  | The software contained on this media is proprietary to and embodies    |
 *  | the confidential technology of Digital Equipment Corporation.          |
 *  | Possession, use, duplication or dissemination of the software and      |
 *  | media is authorized only pursuant to a valid written license from      |
 *  | Digital Equipment Corporation.                                         |
 *  |                                                                        |
 *  | RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the      |
 *  | U.S. Government is subject to restrictions as set forth in             |
 *  | Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19,    |
 *  | as applicable.                                                         |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  
 *  Module Abstract:
 *
 *	This module contains routines to support the printer port within
 *	the DECterm widget.
 *
 *  Author:	Bob Messenger
 *
 *  Modification history:
 *
 * Alfred von Campe     18-Dec-1993     BL-E
 *	- Change all occurrances of common.foreground to manager.foreground.
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Aston Chan		12-Mar-1993	V1.2/BL2
 *	- Add Turkish/Greek support.
 *
 *  Alfred von Campe    08-Oct-1992     Ag/BL10
 *      - Added typecasts to satisfy Alpha compiler
 *
 *  Eric Osman		25-Aug-1992	VXT V1.2
 *	- When starting ptrcon mode, clear number of exit characters seen.
 *
 *  Eric Osman		17-Aug-1992	VXT V1.2
 *	- When printing graphics, use real width and height instead of "3200"
 *	  so scroll bars don't print.  (If user really wants scroll bars, they
 *	  can use manager's "print portion of screen" feature.
 *
 *  Eric Osman		23-Jul-1992	VXT V1.2
 *	- Make sure in national-only printer mode, that lowercase æ prints
 *	  as "ae", and that lowercase å prints as "a".  Somebody reversed
 *	  them.
 *
 *  Eric Osman          11-June-1992    Sun
 *      - Change "char" to "unsigned char" for C compiler error.
 *
 *  Eric Osman		 9-Jan-1992	V3.1
 *	- If w or h is too large, assume we're printing entire screen dimension.
 *
 *  Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 *  Jim Bay		17-Dec-1991	V1.1
 *	- Incorporated Michele's change to fix bug that caused screens
 *	  generated with escape sequences to "squash" output to the left
 *
 *  Aston Chan		14-Nov-1991	V3.1
 *	- Add DECwTermWatchCursor and DECwTermNormalCursor
 *
 *  Aston Chan		09-Oct-1991	V3.1
 *	- when part of DECTerm is off the screen and print graphics is
 *	  activated, AccVio.  Problem is XGetImage() requires the specified
 *	  rectangle to be totally visible.  Solved by writing image and
 *	  text on DECTerm to a pixmap and get image from this pixmap.
 *	  QAR 00045 DECTERMV3
 *
 *  Aston Chan		09-Oct-1991	V3.1
 *	- When doing Print Graphics, DECterm can't be overlapped or else
 *	  that portion of overlapping image is printed as well.  Solved
 *	  by above algorithm.  QAR 00023, QAR 00024.
 *
 *  Aston Chan		04-Oct-1991	V3.1
 *	- "{" "}" missing inside the if statement of lookup_fallback()
 *	  routine.  DECTERMV3 QAR 00052
 *
 *  Alfred von Campe    06-Oct-1991     Hercules/1 T0.7
 *	- Added unsigned qualifier and typecast other chars to satisfy compiler.
 *
 *  Bob Messenger	 4-Jun-1991	V3.0
 *	- Fix the Background Printing option.
 *
 *  Bob Messenger	 9-Sep-1990	X3.0-7
 *	- Support Graphics To Host.
 *
 *  Bob Messenger	26-Aug-1990	X2.0-6
 *	- Add support for ReGIS print screen options.
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 *  Bob Messenger	20-Jul-1990	X3.0-5
 *	- Fix bug with compressed graphics printing.
 *
 *  Bob Messenger	17-Jul-1990	X3.0-5
 *	- Implement graphics print screen.
 *
 *  Bob Messenger	01-Jul-1990	X3.0-5
 *	- Initial version.
 */

#include "wv_hdr.h"

extern void o_update_rectangle();
extern unsigned char *cpystr();
extern void DECwTermWatchCursor();
extern void DECwTermNormalCursor();

#ifdef VXT_DECTERM
#include "msgboxconstants.h"
#include "vxtconfig.h"

extern print_window();
#endif VXT_DECTERM

void print_graphics_screen();

#define ERROR_CHARACTER_CODE (154)
#define ERROR_CHARACTER_SET (CRM_FONT_L)
#define ERROR_CHARACTER_EXT_SET (ONE_BYTE_SET)

#define IS_ISO_CHAR_SET(a)(((a) == ISO_LATIN_1) ||		\
			   ((a) == ISO_LATIN_5) ||		\
			   ((a) == ISO_LATIN_7) ||		\
			   ((a) == ISO_LATIN_8))

static unsigned char *send_line_attributes(), *send_graphic_rendition();
static XImage *get_window_image();
static int abs();

extern s_stop_output();
extern s_start_output();

#define LAST_LINE_INDICATOR	-1

/* fallback tables */

#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
globaldef
#endif

char *line_drawing_fallback[] = {
	"*",	"#",	"HT",	"FF",
	"CR",	"LF",	"o",	"+",
	"NL",	"VT",	"+",	"+",
	"+",	"+",	"+",	"~",
	"=",	"-",	"=",	"_",
	"+",	"+",	"+",	"+",
	"|",	"<=",	">=",	"Pi",
	"<>",	"#",	".",	" " },

/*
 * NOTE:  This version of the supplemental fallback characters has a bunch of
 * "_" instead of " " characters, per the change originally labeled:
 *
 *	I18N_MULTIBYTE	/* 910910, TN400, EIC_JPN
 *
 */

*supplemental_fallback[] = {
	" ",	" ",	"|",	"\"",
	"~",	"-",	"(R)",	"-",
	"'",	",",	"3/4",	"D",
	"x",	"Y",	"Th",	"d",
	"-",	"y",	"th",	"y",
	" ",	" ",	" ",	" ",
	" ",	" ",	" ",	" ",
	" ",	" ",	" ",	" ",
	"_",	"!",	"c",	"#",
	"_",	"Y",	"_",	"Sc",
	"O",	"(C)",	"a",	"<<",
	"_",	"_",	"_",	"_",
	"o",	"+",	"2",	"3",
	"_",	"u",	"Pr",	".",
	"_",	"1",	"o",	">>",
	"1/4",	"1/2",	"_",	"?",
	"A",	"A",	"A",	"A",
	"A",	"A",	"AE",	"C",
	"E",	"E",	"E",	"E",
	"I",	"I",	"I",	"I",
	"_",	"N",	"O",	"O",
	"O",	"O",	"O",	"OE",
	"O",	"U",	"U",	"U",
	"U",	"Y",	"_",	"ss",
	"a",	"a",	"a",	"a",
	"a",	"a",	"ae",	"c",
	"e",	"e",	"e",	"e",
	"i",	"i",	"i",	"i",
	"_",	"n",	"o",	"o",
	"o",	"o",	"o",	"oe",
	"o",	"u",	"u",	"u",
	"u",	"y",	"_",	" " },

*iso_latin_1_fallback[] = {
	"OE",	"oe",	"Y",	" ",
	" ",	" ",	" ",	" ",
	" ",	" ",	" ",	" ",
	" ",	" ",	" ",	" ",
	" ",	" ",	" ",	" ",
	" ",	" ",	" ",	" ",
	" ",	" ",	" ",	" ",
	" ",	" ",	" ",	" ",
	" ",	"!",	"c",	"#",
	"O",	"Y",	"|",	"Sc",
	"\"",	"(C)",	"a",	"<<",
	"~",	"-",	"(R)",	"-",
	"o",	"+",	"2",	"3",
	"'",	"u",	"Pr",	".",
	",",	"1",	"o",	">>",
	"1/4",	"1/2",	"3/4",	"?",
	"A",	"A",	"A",	"A",
	"A",	"A",	"AE",	"C",
	"E",	"E",	"E",	"E",
	"I",	"I",	"I",	"I",
	"D",	"N",	"O",	"O",
	"O",	"O",	"O",	"x",
	"O",	"U",	"U",	"U",
	"U",	"Y",	"Th",	"ss",
	"a",	"a",	"a",	"a",
	"a",	"a",	"ae",	"c",
	"e",	"e",	"e",	"e",
	"i",	"i",	"i",	"i",
	"d",	"n",	"o",	"o",
	"o",	"o",	"o",	"-",
	"o",	"u",	"u",	"u",
	"u",	"y",	"th",	"y" },

/*
 * NOTE:  This version of the technical fallback characters has a bunch of "_"
 *  instead of " " characters, per the change originally labeled:
 *
 *	I18N_MULTIBYTE	/* 910910, TN400, EIC_JPN
 *
 */

*technical_fallback[] = {
	"_",	"-v",	"|-",	"-",
	"(",	")",	"|",	"[",
	"[",	"]",	"]",	"(",
	"(",	")",	")",	"{",
	"}",	">",	"<",	"\\",
	"/",	"_",	"_",	">",
	"_",	"_",	"_",	"_",
	"<",	"=",	">",	"S",
	":.",	"a",	"oO",	":",
	"D",	"v",	"F",	"G",
	"~",	"~",	"J",	"x",
	"L",	"<=>",	"=>",	"=",
	"P",	"Q",	"_",	"S",
	"_",	"_",	"V~",	"W",
	"X",	"Y",	"c",	"_",
	"^",	"v",	"&",	"|",
	"~",	"a",	"b",	"c",
	"d",	"e",	"f",	"g",
	"h",	"i",	"j",	"k",
	"l",	"_",	"n",	"_",
	"p",	"q",	"r",	"s",
	"t",	"_",	"f",	"w",
	"x",	"y",	"z",	"<-",
	"^",	"->",	"v",	" " },

*CRM_fallback[] = {						/*001+*/
	"",	"SH",	"SX",	"EX",			/* 0/0 - 0/3	  001+*/
	"ET",	"EQ",	"AK",	"BL",			/* 0/4 - 0/7	  001+*/
	"BS",	"HT",	"LF",	"VT",			/* 0/8 - 0/11	  001+*/
	"FF",	"CR",	"SO",	"SI",			/* 0/12- 0/15	  001+*/
	"DL",	"D1",	"D2",	"D3",			/* 1/0 - 1/3	  001+*/
	"D4",	"NK",	"SY",	"EB",			/* 1/4 - 1/7	  001+*/
	"CN",	"EM",	"SB",	"EC",			/* 1/8 - 1/11	  001+*/
	"FS",	"GS",	"RS",	"US",			/* 1/12- 1/15	  001+*/
	" ",						/* 2/0		  001+*/
				/* 2/1-7/14 corresponding JIS-Roman	  001+*/
	"DT",						/* 7/15		  001+*/
	"80",	"81",	"82",	"83",			/* 8/0 - 8/3	  001+*/
	"IN",	"NL",	"SS",	"ES",			/* 8/4 - 8/7	  001+*/
	"HS",	"HJ",	"VS",	"PD",			/* 8/8 - 8/11	  001+*/
	"PU",	"RI",	"S2",	"S3",			/* 8/12- 8/15	  001+*/
	"DC",	"P1",	"P2",	"SE",			/* 9/0 - 9/3	  001+*/
	"CC",	"MW",	"SP",	"EP",			/* 9/4 - 9/7	  001+*/
	"98",	"99",	"9A",	"CS",			/* 9/8 - 9/11	  001+*/
	"ST",	"OS",	"PM",	"AP",			/* 9/12- 9/15	  001+*/
	"A0",						/* 10/0		  001+*/
				/* 10/1-13/15 corresponding JIS-Katakana  001+*/
	"_",	"_",	"_",	"_",			/* 14/0 -14/3	  001+*/
	"_",	"_",	"_",	"_",			/* 14/4 -14/7	  001+*/
	"_",	"_",	"_",	"_",			/* 14/8 -14/11	  001+*/
	"_",	"_",	"_",	"_",			/* 14/12-14/15	  001+*/
	"_",	"_",	"_",	"_",			/* 15/0 -15/3	  001+*/
	"_",	"_",	"_",	"_",			/* 15/4 -15/7	  001+*/
	"_",	"_",	"_",	"_",			/* 15/8 -15/11	  001+*/
	"_",	"_",	"_",				/* 15/12-15/14	  001+*/
	"FF" },						/* 15/15	  001+*/

*designator[] = {
	"B",	/* 0  ASCII */
	"0",	/* 1  line drawing */
	">",	/* 2  technical */
	"\"8",	/* 3  APL */
	" @",	/* 4  DRCS */
	"\32",	/* 5  USER_SET_1 (unused) */
	"A",	/* 6  ISO Latin 1 Supplemental (96 characters) */
	"%5",	/* 7  DEC Supplemental */
	"H",	  /* 8 ISO Latin 8 (96 characters) */
   	"\42\64", /* 9 DEC Hebrew Supplemental ("4) */
	"%0",	/* 10 Turkish Supplemental */
	"M",	/* 11 ISO Latin 5 (Turkish) */
	"\"?",	/* 12 Greek Supplemental */
	"F", 	/* 13 ISO Latin 7 (Greek) */
	"<"	/* User preferred supplemental (counts as 94 characters even
		   if it is ISO Latin 1) */
	};

#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
globaldef
#endif

unsigned char nrc_designator[] = {
	'B',	/* North American	ASCII */
	'R',	/* Flemish		French NRC set */
	'9',	/* Canadian (French)	French Canadian NRC set */
	'A',	/* British		U.K. NRC set */
	'`',	/* Danish		Norwegian/Danish NRC set */
	'5',	/* Finnish		Finnish NRC set */
	'K',	/* Austrian/German	German NRC set */
	'B',	/* Dutch		ASCII */
	'Y',	/* Italian		Italian NRC set */
	'=',	/* Swiss (French)	Swiss NRC set */
	'=',	/* Swiss (German)	Swiss NRC set */
	'7',	/* Swedish		Swedish NRC set */
	'`',	/* Norwegian		Norwegian/Danish NRC set */
	'R',	/* Belgian/French	French NRC set */
	'Z',	/* Spanish		Spanish NRC set */
	'%'	/* Portuguese		Portuguese NRC set
		   (note: the designator for Portuguese is %6) */
       ,'%'	/* Hebrew		Hebrew NRC set
		   (note: the designator for Hebrew is %=      */
       ,'%'	/* Turkish		Turkish NRC set
		   (note: the designator for Turkish is %2	*/
       ,'"'	/* Greek		Greek NRC set
		   (note: the designator for Greek is ">	*/
	},

/*
 * mcs_c1_to_iso maps the C1 characters of the DEC Multinational display (i.e.
 * ISO Latin 1 characters not in DEC Multinational) to their ISO Latin 1
 * encodings. The table starts with character 128.
 */

mcs_c1_to_iso[] = {
	026, 160, 166, 168, 172, 173, 174, 175,
	180, 184, 190, 208, 215, 221, 222, 240,
	247, 253, 254, 255, 026, 026, 026, 026,
	026, 026, 026, 026, 026, 026, 026, 026
	},

/*
 * iso_c1_to_mcs maps the C1 characters of the ISO Latin 1 display format
 * (i.e. DEC Multinational characters not in ISO Latin 1) to their DEC
 * Multinational encodings.  The table starts at character 128.
 */

iso_c1_to_mcs[] = { 215, 247, 221 };

void s_set_value_printMode( oldw, w )
    DECtermWidget oldw, w;
{
    DECwPrintMode new_mode;

    new_mode = w->common.printMode;

    if ( w->common.printMode == oldw->common.printMode )
	return;

    if ( oldw->common.printMode == DECwPrintControllerMode )
	WVT$EXIT_PRINT_CONTROLLER_MODE( w );
    else if ( oldw->common.printMode == DECwAutoPrintMode )
	WVT$EXIT_AUTO_PRINT_MODE( w );

    if ( new_mode == DECwPrintControllerMode )
	WVT$ENTER_PRINT_CONTROLLER_MODE( w );
    else if ( new_mode == DECwAutoPrintMode )
	WVT$ENTER_AUTO_PRINT_MODE( w );
}

void s_set_value_printExtent( oldw, w )
    DECtermWidget oldw, w;
{
    wvtp ld = w_to_ld( w );

    if ( w->common.printExtent == DECwScrollRegionOnly )
	_cld wvt$w_print_flags &= ~pf1_m_prt_extent_mode;
    else
	_cld wvt$w_print_flags |= pf1_m_prt_extent_mode;
}

    /* For VXT, there is only one printer destination, the local printer
       port.  There is no file system and no printer queue.  For VXT, the 
       customize printer menu has been modified to be different from DECterm 
       V3.0.  Printer destination option has been taken out. */

void s_set_value_printerPortName( oldw, neww )
    DECtermWidget oldw, neww;
{
    if ( oldw->common.printerPortName != NULL )
	XtFree( oldw->common.printerPortName );

    if ( neww->common.printerPortName != NULL )
	neww->common.printerPortName =
		XtNewString( neww->common.printerPortName );
}

void s_set_value_printerFileName( oldw, neww )
    DECtermWidget oldw, neww;
{
    if ( oldw->common.printerFileName != NULL )
	XtFree( oldw->common.printerFileName );

    if ( neww->common.printerFileName != NULL )
	neww->common.printerFileName =
		XtNewString( neww->common.printerFileName );
}

s_set_value_prt_to_host( oldw, neww )
    DECtermWidget oldw, neww;
{
    char *aptr;
    int junk;
    wvtp ld = w_to_ld( neww );
/*
 * It's not just for efficiency.  It's crucial that we return if old == new.
 * Otherwise, the SetValues in stop_read_data_from_prt would cause us to
 * call stop_read_data_from_prt again !
 */
    if ( oldw->common.printerToHostEnabled ==
	neww->common.printerToHostEnabled) return;

    if ( neww->common.printerToHostEnabled )
    {   /* We're turning it on.  Remember whether we succeeded */
	XtCallCallbacks( (Widget)neww, DECwNstartPrinterToHostCallback,
	    &neww->common.printerToHostEnabled );
    }
    else      
    {
	XtCallCallbacks( (Widget)neww, DECwNstopPrinterToHostCallback,
	    &junk );
    }
}

void s_set_value_printDisplayMode( oldw, w )
    DECtermWidget oldw, w;
{
    wvtp ld = w_to_ld( w );

    if ( w->common.printDisplayMode == DECwMainDisplay24 )
       _cld wvt$w_print_flags &= ~pf1_m_prt_display_mode;
    else
       _cld wvt$w_print_flags |= pf1_m_prt_display_mode;
}

/*
 * DECwTermPrintTextScreen - print the display list in text form
 *
 * This routine can be called from both inside and outside the DECterm widget.
 */

void DECwTermPrintTextScreen( w )
    DECtermWidget w;
{
    wvtp ld = w_to_ld( w );
    int call_data, line, top_line, bottom_line, first_col, last_col;
    int current_line = 0, begin_col, end_col;

/*
 * Notify the application that we are starting a print screen operation.
 */

    call_data = DECwCRStartPrintScreen;
    XtCallCallbacks( (Widget)w, DECwNstartPrintingCallback, &call_data );

/*
 * Send each line to the application.
 */

    if ( w->common.printerStatus == DECwPrinterReady )
	{
	switch (w->common.printExtent)
	    {
	    case DECwFullPage:
		first_col = 1;
		top_line = 1;
		bottom_line = _mld wvt$l_page_length;
		last_col = _mld wvt$w_widths[ bottom_line ];
		break;
	    case DECwFullPagePlusTranscript:
		first_col = 1;
		top_line = _mld wvt$l_transcript_top;
		bottom_line = _mld wvt$l_page_length;
		last_col = _mld wvt$w_widths[ bottom_line ];
		break;
	    case DECwScrollRegionOnly:
		first_col = 1;
		top_line = _mld wvt$l_top_margin;
		bottom_line = _mld wvt$l_bottom_margin;
		last_col = _mld wvt$w_widths[ bottom_line ];
		break;
	    case DECwSelectionOnly:
		first_col = DECtermPosition_column(Source_select_begin(w,0))+1;
		top_line = DECtermPosition_row(Source_select_begin(w,0));
		last_col = DECtermPosition_column(Source_select_end(w,0));
		bottom_line = DECtermPosition_row(Source_select_end(w,0));
		break;
	    }

	/* stop reading data from PTY driver to ensure a snap shot of the
	   display list is accurate.  this will prevent display list from
	   getting updated while taking a snap shot to be printed */

        s_stop_output( w, STOP_OUTPUT_PRINTER ); 

	/* check the status of printer for every line printed. when
	   the printer goes off-line, abort printing */

        for( current_line = top_line;
	     (w->common.printerStatus == DECwPrinterReady) && 
	                (current_line <= bottom_line); 
	     current_line++ )
        {
	    if (current_line == top_line)
		begin_col = first_col;
	    else
		begin_col = 1;
	    if (current_line == bottom_line)
		end_col = last_col;
	    else
		end_col = _mld wvt$w_widths[ current_line ];
	    print_lines( ld, begin_col, current_line, end_col, 
			current_line, '\n');
        }

        if ( _cld wvt$w_print_flags & pf1_m_prt_ff_mode &&
	     (w->common.printerStatus == DECwPrinterReady) )
	    {
	    _cld wvt$b_work_buffer[0] = '\f';	/* send a form feed ... */
	    _cld wvt$l_work_string.offset = 1;
	    WVT$PRINT_LINE(ld);			/* ... to the application */
	    }
	}
/*
 * Notify the application that printing has completed.
 */

    /* For non-blocking printing, when DECterm widget finishes sending data to 
	the DECterm application, DECterm application may not have finished
	writing data to the printer.  Therefore cannot close the printer port
	until all the data has been written to the printer from the DECterm
	application. */

    if( w->common.printerStatus == DECwPrinterReady )
    {
        _cld wvt$b_work_buffer[0] = LAST_LINE_INDICATOR;
        _cld wvt$l_work_string.offset = 1;
        WVT$PRINT_LINE(ld);			/* ... to the application */
    }

    /* resume reading data from the PTY driver */

    s_start_output( w, STOP_OUTPUT_PRINTER );

}

/*
 * DECwTermPrintGraphicsScreen - external entry point for printing the
 *	window contents in bitmap (sixel) form.
 */

void DECwTermPrintGraphicsScreen( w )
    DECtermWidget w;
{
    DECwTermWatchCursor ( w );

    print_graphics_screen( w, 0, 0, w->common.display_width,
      w->common.display_height, 0, 0, FALSE );

    DECwTermNormalCursor ( w );
}

/*
 * print_graphics_screen - called from within the DECterm widget to print
 *	the window contents in bitmap (sixel) form.
 */

void print_graphics_screen( w, src_x, src_y, src_width, src_height,
			    x_offset, y_offset, from_regis )
    DECtermWidget w;	/* widget context */
    int src_x, src_y;	/* upper left corner of the rectangle of the window
			   to be printed */
    int src_width, src_height;
			/* width and height of the rectangle of the window
			   to be printed */
    int x_offset, y_offset;
			/* offset of printed data on the output device */
    int from_regis;	/* boolean; TRUE means we were called from ReGIS,
			   so output might be sent to the host instead of
			   to the printer. */
{
    int call_data;
    char buffer[80];
    XImage *image;
    Pixmap pixmap;
    Display *dpy;
    int depth;
    wvtp ld = w_to_ld( w );
    int data_to_host = 0;
    Window win_id, dummy_id;
    int status = 0;

#ifdef VXT_DECTERM
    /* Some hardware platforms do not have printer support, such as 
       the VT1300.  In those cases, send a warning message. */

    if( VxtSystemType() != VXTDWTII)
    {
        vxt_msgbox_write( TM_MSG_WARNING, 1, k_dt_printer_not_supported, 0); 

    	return;
    }
#endif VXT_DECTERM

/*
 * Check for large width and height and trim to size of screen.  These are
 * known to be 32768 if we're called from "hardcopy" in regis.
 */
    if ( src_width > w->common.display_width ) src_width =
	w->common.display_width;

    if ( src_height > w->common.display_height ) src_height =
	w->common.display_height;

    if ( ! w->common.graphicsPrintingEnabled )
	return;

    dpy = XtDisplay(w);

    if ( from_regis && _cld wvt$w_print_flags & pf1_m_graphics_to_host )
    {
	_cld wvt$w_print_flags |= pf1_m_prt_data_to_host;
	data_to_host = 1;
    }
    else
    {
	_cld wvt$w_print_flags &= ~pf1_m_prt_data_to_host;
	data_to_host = 0;
    }

/* VXT has a separate graphic print screen routine which is shared by both the
   terminal manager and DECterm.  Therefore will not use the existing DECterm
   code for graphic print screen.  However, because VXT specific graphic print
   screen routine can only output to the local printer, when ReGIS needs to 
   output to the host instead of the printer, the DECterm graphic print screen
   code is used instead of VXT specific graphic print screen routine.  This is
   a hack, but hopefully this will be changed in the future.
*/

#if defined (VXT_DECTERM)
    if (data_to_host == 0 )
    {
        /* Call VXT specific graphic print screen routine */

#include "dt_output.h"

	win_id = XtWindow(w);
	dummy_id = XCreateSimpleWindow( XtDisplay(w), win_id, 0, 0, 1, 1, 0, 
		   0, 0);
/*
 * Print without border.
 */
	status = print_window(  XtDisplay(w), dummy_id, win_id,
	    src_x + X_MARGIN, src_y + Y_MARGIN,
	    src_width - 2*X_MARGIN, src_height - 2*Y_MARGIN, 0 );
	if (!status)
	    {
	    vxt_msgbox_write( TM_MSG_WARNING, 1, k_dt_printer_not_available, 0);
	    }
    }
    else

#endif

    {

/*
 * Notify the application that we are starting a print screen operation.
 * We return immediately if printer can't be opened.
 */

    if ( ! ( _cld wvt$w_print_flags & pf1_m_prt_data_to_host ) )
	{
	call_data = DECwCRStartPrintScreen;
	XtCallCallbacks( (Widget)w, DECwNstartPrintingCallback, &call_data );

	if ( w->common.printerStatus != DECwPrinterReady )
	    return;

	}

/*
 * Send the sixel introducer.
 */

    send_to_printer( w, "\33\\" );	/* ESC \ */
    switch ( w->common.printSixelLevel )
	{
	case DECwSixelLevel_1:
	    send_to_printer( w, "\33P1q" );
	    break;
	case DECwSixelLevel_2:
	    sprintf( buffer, "\33[2 I\33P0;%d;%dq\"1;1;%d;%d",
	      w->common.printBackgroundMode ? 2 : 1,
	      ( w->common.printFormat == DECwCompressedPrinting ) ? 6 : 9,
	      w->common.display_width,
	      w->common.display_height );
	    send_to_printer( w, buffer );
	    break;
	case DECwSixelLevel_LA210:
	    send_to_printer( w, "\33P9q" );
	    break;
	}

/*
 * If we're doing a color screen dump, send the color map.
 */

    if ( w->common.printColorMode )
	print_color_map( w );

/*
 * Print the contents of the window in sixel format.
 */

    depth = XDefaultDepthOfScreen ( XDefaultScreenOfDisplay(dpy) );

    /* create temporary pixmap for image capture
      */
    pixmap = XCreatePixmap( dpy,
			    XtWindow(w),
			    src_width  + X_MARGIN * 2,
			    src_height + Y_MARGIN * 2 ,
			    depth );

    /* jacket routine for update_rectangle to update everything on DECterm
     * to the pixmap just created.
     */
    o_update_rectangle ( w, 
			 w->output.top_visible_line,
			 w->output.visible_rows,
			 w->output.left_visible_column,
			 w->output.visible_columns,
			 pixmap );

    /* print image from the pixmap */
    print_image_data( w, 0, 0, src_width, src_height, 0, 0, pixmap );

    XFreePixmap (dpy, pixmap);

/*
 * Print the string terminator to complete the image, possibly followed by
 * a form feed.
 */

    send_to_printer( w, "\33\\" );
    flush_print_buffer( w );
    if ( _cld wvt$w_print_flags & pf1_m_prt_ff_mode )
	{
	_cld wvt$b_work_buffer[0] = '\f';	/* send a form feed ... */
	_cld wvt$l_work_string.offset = 1;
	WVT$PRINT_LINE(ld);			/* ... to the application */
	}

/*
 * Notify the application that printing has completed.
 */

    if ( ! ( _cld wvt$w_print_flags & pf1_m_prt_data_to_host ) )
	{
        _cld wvt$b_work_buffer[0] = LAST_LINE_INDICATOR;
        _cld wvt$l_work_string.offset = 1;
        WVT$PRINT_LINE(ld);
	}
    _cld wvt$w_print_flags &= ~pf1_m_prt_data_to_host;
    }
}

/*
 * WVT$PRINT_LINE
 *
 * This routine sends a line of text or sixel data to the printer port (i.e.
 * it calls the printLineCallback routine in the application).
 *
 * Implicit inputs:
 *
 *	_cld wvt$b_work_buffer		Start of data to print
 *	_cld wvt$l_work_string.offset	Number of characters to print
 *	_cld wvt$w_print_flags		If pf1_m_prt_data_to_host is set, send
 *					the data to the host instead
 */

int WVT$PRINT_LINE( ld )
    wvtp ld;
{
    DECtermWidget w = ld_to_w(ld);

    DECwTermInputCallbackStruct call_data;

    /*
     * If the pf1_m_prt_data_to_host flag is set (because we are executing a
     * ReGIS hardcopy command and Graphics To Host mode is enabled), send
     * the data to the host as a report.
     */
    if ( _cld wvt$w_print_flags & pf1_m_prt_data_to_host )
	{
	i_report_data( w, _cld wvt$b_work_buffer,
			  _cld wvt$l_work_string.offset );
	_cld wvt$l_work_string.offset = 0;
	return TRUE;
	}

    if ( _cld wvt$l_work_string.offset == 0 )
	return ( w->common.printerStatus );

    call_data.data = (char *) _cld wvt$b_work_buffer;
    call_data.count = _cld wvt$l_work_string.offset;
    call_data.reason = DECwCRPrintLine;

    XtCallCallbacks( (Widget)w, DECwNprintLineCallback, &call_data );

    _cld wvt$l_work_string.offset = 0;

    return ( w->common.printerStatus );
}

WVT$ENTER_PRINT_CONTROLLER_MODE( ld )
    wvtp ld;
{
    DECtermWidget w = ld_to_w(ld);
    int call_data;

	/*   Exit auto print mode before enter controller mode.	*/
    if ( _cld wvt$w_print_flags & pf1_m_auto_print_mode )
	WVT$EXIT_AUTO_PRINT_MODE( ld );

    call_data = DECwCREnterPrinterControllerMode;
    XtCallCallbacks( (Widget)w, DECwNstartPrintingCallback, &call_data );

    _cld wvt$w_print_flags |= pf1_m_prt_controller_mode;
    _cld n_pr_exit_chars = 0;
    w->common.printMode = DECwPrintControllerMode;
}

WVT$ENTER_AUTO_PRINT_MODE( ld )
    wvtp ld;
{
    DECtermWidget w = ld_to_w(ld);
    int call_data;

	/*   Exit controller mode before enter auto print mode.	*/
    if ( _cld wvt$w_print_flags & pf1_m_prt_controller_mode )
	WVT$EXIT_PRINT_CONTROLLER_MODE( ld );

    call_data = DECwCREnterAutoPrintMode;
    XtCallCallbacks( (Widget)w, DECwNstartPrintingCallback, &call_data );

    _cld wvt$w_print_flags |= pf1_m_auto_print_mode;
    w->common.printMode = DECwAutoPrintMode;
}

WVT$EXIT_PRINT_CONTROLLER_MODE( ld )
    wvtp ld;
{
    DECtermWidget w = ld_to_w(ld);
    int call_data;

    WVT$PRINT_LINE( w );	/* output the print buffer */

    _cld wvt$w_print_flags &= ~pf1_m_prt_controller_mode;
    w->common.printMode = DECwNormalPrintMode;

    /* For non-blocking printing, when DECterm widget finishes sending data to 
	the DECterm application, DECterm application may not have finished
	writing data to the printer.  Therefore cannot close the printer port
	until all the data has been written to the printer from the DECterm
	application. */

    _cld wvt$b_work_buffer[0] = LAST_LINE_INDICATOR;
    _cld wvt$l_work_string.offset = 1;
    WVT$PRINT_LINE(ld);			/* ... to the application */
}

WVT$EXIT_AUTO_PRINT_MODE( ld )
    wvtp ld;
{
    DECtermWidget w = ld_to_w(ld);
    int call_data;

    _cld wvt$w_print_flags &= ~pf1_m_auto_print_mode;
    w->common.printMode = DECwNormalPrintMode;

    /* For non-blocking printing, when DECterm widget finishes sending data to 
	the DECterm application, DECterm application may not have finished
	writing data to the printer.  Therefore cannot close the printer port
	until all the data has been written to the printer from the DECterm
	application. */

    _cld wvt$b_work_buffer[0] = LAST_LINE_INDICATOR;
    _cld wvt$l_work_string.offset = 1;
    WVT$PRINT_LINE(ld);			/* ... to the application */
}

/*
 * print_lines
 *
 * This routine prints one or more lines from the text display list.  It
 * prints lines first_row to last_row.  first_col is the first character
 * printed on the first row and last_col is the last character printed on
 * the last row; every character is printed in the intervening rows.
 *
 * terminator is the control character to print at the end of each line.
 * Normally it is a line feed, but in auto print mode it is the character that
 * caused the cursor to move off the line: it can be line feed, vertical tab
 * or form feed.
 */

print_lines( ld, first_col, first_row, last_col, last_row, terminator )
    wvtp ld;
    int first_col, first_row, last_col, last_row;
    char terminator;
{
    unsigned char *bptr, *char_row, char_code, new_char_code;
    REND *rend_row, renditions, char_rend;
    int line, start_char, end_char, x, char_set, g1, g3, invoked, gleft,
	gright, new_char_set, g_set;
    DECtermWidget w = ld_to_w(ld);
    EXT_REND *ext_rend_row;
    int ext_char_set, g2;
    Boolean g2_modified;

    /*
     * Start the buffer pointer at the beginning.
     */

    bptr = _cld wvt$b_work_buffer;

    /*
     * Handle each print data type separately.
     */

    switch (w->common.printDataType)
	{
	case DECwNationalOnly:
	    /*
	     * For each line.
	     */
	    if ( _cld wvt$l_ext_flags & vte1_m_tomcat ) {
		*bptr++ = '\33'; *bptr++ = '(';
		*bptr++ = _cld wvt$l_ext_specific_flags & vte2_m_jisroman_mode ?
			'J' : 'B';			  /* G0, Roman/ASCII */
		*bptr++ = '\33'; *bptr++ = '+'; *bptr++ = 'I';	/* G3, Kana  */
		*bptr++ = '\17'; gleft = 0;			/* G0 -> GL  */
		*bptr++ = '\33'; *bptr++ = '|'; gright = 3;	/* G3 -> GR  */
	    } else if ( _cld wvt$l_ext_flags & vte1_m_bobcat ) {
		*bptr++ = '\33'; *bptr++ = '('; *bptr++ = 'B';	/* G0, ASCII */
		*bptr++ = '\33'; *bptr++ = '$'; *bptr++ = '+';
		*bptr++ = '2';					/* G3, Hanzi */
		*bptr++ = '\17'; gleft = 0;			/* G0 -> GL  */
		*bptr++ = '\33'; *bptr++ = '|'; gright = 3;	/* G3 -> GR  */
	    } else if ( _cld wvt$l_ext_flags & vte1_m_dickcat ) {
		*bptr++ = '\33'; *bptr++ = '(';
		if ( _cld wvt$l_ext_specific_flags & vte2_m_ksroman_mode ) {
		    *bptr++ = '%'; *bptr++ = '?';
		} else
		    *bptr++ = 'B';			  /* G0, Roman/ASCII */
		*bptr++ = '\33'; *bptr++ = '$'; *bptr++ = '+';
		*bptr++ = '4';					/* G3, Hangul */
		*bptr++ = '\17'; gleft = 0;			/* G0 -> GL  */
		*bptr++ = '\33'; *bptr++ = '|'; gright = 3;	/* G3 -> GR  */
	    } else if ( _cld wvt$l_ext_flags & vte1_m_fishcat ) {
		*bptr++ = '\33'; *bptr++ = '('; *bptr++ = 'B';	/* G0, ASCII */
		*bptr++ = '\33'; *bptr++ = '$'; *bptr++ = '+';
		*bptr++ = '5';					/* G3, Hanyu */
		*bptr++ = '\17'; gleft = 0;			/* G0 -> GL  */
		*bptr++ = '\33'; *bptr++ = '|'; gright = 3;	/* G3 -> GR  */
	    }
	    if ( _cld wvt$w_print_flags & pf1_m_prt_display_mode )
		last_row += 1;
	    for ( line = first_row; line <= last_row; line++ )
		{
		if ( _cld wvt$w_print_flags & pf1_m_prt_display_mode &&
		    line == last_row ) {
		    char_row = _sld wvt$a_code_base[1];
		    rend_row = _sld wvt$a_rend_base[1];
		    ext_rend_row = _sld wvt$a_ext_rend_base[1];
		    start_char = 1;
		    end_char = _sld wvt$w_widths[1];
		} else {
		char_row = _mld wvt$a_code_base[ line ];
		rend_row = _mld wvt$a_rend_base[ line ];
		ext_rend_row = _mld wvt$a_ext_rend_base[ line ];
		start_char = ( line == first_row ) ? first_col : 1;
		/*
		 * Set end of line to skip trailing spaces.
		 */
		end_char = ( line == last_row ) ? last_col :
			_mld wvt$w_widths[ line ];
		while ( end_char >= start_char )
		    if ( char_row[ end_char ] == ' ' ||
				char_row[ end_char ] == '\0' )
			end_char--;
		    else
			break;
		}
		/*
		 * Print each character on the line.
		 */
		for ( x = start_char; x <= end_char; x++ )
		    {
		    char_code = char_row[x];
		    if (char_code == 0)
			/* put spaces in place of nulls */
			char_code = ' ';
		    char_set = rend_row[x] & csa_M_CHAR_SET;
		    ext_char_set =
			_cld wvt$l_ext_flags ?
			ext_rend_row[x] & csa_M_EXT_CHAR_SET : 
			STANDARD_SET;
		    switch ( ext_char_set )
		    {
		    case STANDARD_SET:
		    if ( char_set == ASCII || char_set >= ISO_LATIN_1 )
			{
			*bptr = char_code;
			if ( lookup_nrc_code( w, bptr ) )
			    bptr++;
			else
			    bptr += lookup_fallback( w, char_code, char_set,
					ext_char_set, bptr );
			}
		    else
			bptr += lookup_fallback( w, char_code, char_set,
				ext_char_set, bptr );
		    break;
		    case TWO_BYTE_SET:
			if ( char_set == DEC_KANJI )
			    *bptr++ = '_';
			else
			    *bptr++ = char_code;
		    break;
		    case FOUR_BYTE_SET:
			if (( char_set == DEC_HANYU_4 ) &&
			    ( ext_rend_row[x] == THIRD_OF_FOUR )) {
			    *bptr++ = LC1;
			    *bptr++ = LC2;
			}
			*bptr++ = char_code;
		    break;
		    case ONE_BYTE_SET:
			if ( char_set == JIS_KATAKANA || char_set == JIS_ROMAN
			    || char_set == KS_ROMAN )
			    *bptr++ = char_code;
			else
			    bptr += lookup_fallback( w, char_code, char_set,
				ext_char_set, bptr );
		    defaults:
			bptr += lookup_fallback( w, char_code, char_set,
				ext_char_set, bptr );
		    break;
		    }
		    }
		/*
		 * Terminate the line and send it to the printer port.
		 */

		*bptr++ = terminator;
		*bptr++ = 0x0d;	/* carriage return in ASCII */

		_cld wvt$l_work_string.offset = bptr - _cld wvt$b_work_buffer;
		if ( WVT$PRINT_LINE( ld ) != DECwPrinterReady )
	  	{
    		    _cld wvt$w_print_flags &= ~pf1_m_auto_print_mode;
    		    w->common.printMode = DECwNormalPrintMode;

		    return;
	  	}
		bptr = _cld wvt$b_work_buffer;
		}
	    break;

	case DECwNationalPlusLineDrawing:
	    /*
	     * Initialize.
	     */
	    if ( _cld wvt$l_ext_flags & vte1_m_tomcat ) {
		*bptr++ = '\33'; *bptr++ = '(';
		*bptr++ = _cld wvt$l_ext_specific_flags & vte2_m_jisroman_mode ?
			'J' : 'B';			  /* G0, Roman/ASCII */
		*bptr++ = '\33'; *bptr++ = ')'; *bptr++ = '0';	/* G1, Spec. */
		*bptr++ = '\33'; *bptr++ = '$'; *bptr++ = '+';
		*bptr++ = '1';					/* G3, Kanji */
		*bptr++ = '\17'; gleft = 0;			/* G0 -> GL  */
		*bptr++ = '\33'; *bptr++ = '|'; gright = 3;	/* G3 -> GR  */
		g1 = LINE_DRAWING;
	    } else if ( _cld wvt$l_ext_flags & vte1_m_bobcat ) {
		*bptr++ = '\33'; *bptr++ = '('; *bptr++ = 'B';	/* G0, ASCII */
		*bptr++ = '\33'; *bptr++ = ')'; *bptr++ = '0';	/* G1, Spec. */
		*bptr++ = '\33'; *bptr++ = '$'; *bptr++ = '+';
		*bptr++ = '2';					/* G3, Hanzi */
		*bptr++ = '\17'; gleft = 0;			/* G0 -> GL  */
		*bptr++ = '\33'; *bptr++ = '|'; gright = 3;	/* G3 -> GR  */
		g1 = LINE_DRAWING;
	    } else if ( _cld wvt$l_ext_flags & vte1_m_dickcat ) {
		*bptr++ = '\33'; *bptr++ = '(';
		if ( _cld wvt$l_ext_specific_flags & vte2_m_ksroman_mode ) {
		    *bptr++ = '%'; *bptr++ = '?';
		} else
		    *bptr++ = 'B';			  /* G0, Roman/ASCII */
		*bptr++ = '\33'; *bptr++ = ')'; *bptr++ = '0';	/* G1, Spec. */
		*bptr++ = '\33'; *bptr++ = '$'; *bptr++ = '+';
		*bptr++ = '4';					/* G3, Hangul */
		*bptr++ = '\17'; gleft = 0;			/* G0 -> GL  */
		*bptr++ = '\33'; *bptr++ = '|'; gright = 3;	/* G3 -> GR  */
		g1 = LINE_DRAWING;
	    } else if ( _cld wvt$l_ext_flags & vte1_m_fishcat ) {
		*bptr++ = '\33'; *bptr++ = '('; *bptr++ = 'B';	/* G0, ASCII */
		*bptr++ = '\33'; *bptr++ = ')'; *bptr++ = '0';	/* G1, Spec. */
		*bptr++ = '\33'; *bptr++ = '$'; *bptr++ = '+';
		*bptr++ = '5';					/* G3, Hanyu */
		*bptr++ = '\17'; gleft = 0;			/* G0 -> GL  */
		*bptr++ = '\33'; *bptr++ = '|'; gright = 3;	/* G3 -> GR  */
		g1 = LINE_DRAWING;
	    } else {
	    *bptr++ = '\33';	/* designate national set into G0 */
	    *bptr++ = '(';
	    *bptr++ = nrc_designator[ w->common.keyboardDialect ];
	    if ( w->common.keyboardDialect == DECwPortugueseDialect )
		*bptr++ = '6';
	    else
	    if ( w->common.keyboardDialect == DECwHebrewDialect )
		*bptr++ = '=';
	    else
	    if ( w->common.keyboardDialect == DECwTurkishDialect )
		*bptr++ = '2';
	    else
	    if ( w->common.keyboardDialect == DECwGreekDialect )
		*bptr++ = '>';

	    *bptr++ = '\33';	/* designate DEC Special Graphics into G1 */
	    *bptr++ = ')';
	    *bptr++ = '0';
	    *bptr++ = '\17';	/* LS0: invoke G0 into GL */
	    g1 = LINE_DRAWING;
	    }
	    invoked = 0;
	    *bptr++ = '\33';	/* initialize character rendition */
	    *bptr++ = '[';
	    *bptr++ = '0';
	    *bptr++ = 'm';
	    renditions = 0;
	    /*
	     * For each line.
	     */
	    if ( _cld wvt$w_print_flags & pf1_m_prt_display_mode )
		last_row += 1;
	    for ( line = first_row; line <= last_row; line++ )
		{
		bptr = send_line_attributes( bptr, _mld wvt$b_rendits[line] );
		if ( _cld wvt$w_print_flags & pf1_m_prt_display_mode &&
		    line == last_row ) {
		    char_row = _sld wvt$a_code_base[1];
		    rend_row = _sld wvt$a_rend_base[1];
		    ext_rend_row = _sld wvt$a_ext_rend_base[1];
		    start_char = 1;
		    end_char = _sld wvt$w_widths[1];
		} else {
		char_row = _mld wvt$a_code_base[ line ];
		rend_row = _mld wvt$a_rend_base[ line ];
		ext_rend_row = _mld wvt$a_ext_rend_base[ line ];
		start_char = ( line == first_row ) ? first_col : 1;
		/*
		 * Set end of line to skip trailing spaces.
		 */
		end_char = ( line == last_row ) ? last_col :
			_mld wvt$w_widths[ line ];
		}
		while ( end_char >= start_char )
		    if ( char_row[ end_char ] == '\0' ||
				char_row[ end_char ] == ' ' &&
		    		( rend_row[ end_char ] & ~(csa_M_BLINK |
				csa_M_UNDERLINE | csa_M_REVERSE)) == 0 )
			/*
			 * Character position is empty or has a space that
			 * is not blinking, underlined or reverse, so skip it.
			 */
			end_char--;
		    else
			break;
		/*
		 * Print each character on the line.
		 */
		for ( x = start_char; x <= end_char; x++ )
		    {
		    char_code = char_row[x];
		    if (char_code == 0)
			/* put spaces in place of nulls */
			char_code = ' ';
		    char_set = rend_row[x] & csa_M_CHAR_SET;
		    char_rend = rend_row[x] & ( csa_M_BOLD | csa_M_UNDERLINE |
				csa_M_BLINK | csa_M_REVERSE );
		    new_char_code = char_code;
		    new_char_set = 3;	/* fallback required */
		    ext_char_set =
			_cld wvt$l_ext_flags ?
			ext_rend_row[x] & csa_M_EXT_CHAR_SET : 
			STANDARD_SET;
		    switch ( ext_char_set )
		    {
		    case STANDARD_SET:
		    if ( char_set == ASCII || char_set >= ISO_LATIN_1 )
			{
			if ( lookup_nrc_code( w, &new_char_code ) )
			    new_char_set = 0;	/* national */
			else if ( char_set == ASCII )
			    new_char_set = 1;	/* ASCII */
			else if ( w->common.v1_encodings && char_code < 32 )
			    {
			    new_char_set = 2;	/* line drawing */
			    new_char_code = char_code + 95;
			    }
			}
		    else if ( char_set == LINE_DRAWING &&
				w->common.v1_encodings )
			{
			new_char_set = 2;		/* line drawing */
			new_char_code = char_code + 95;
			}
		    else if ( char_set == TECHNICAL && !w->common.v1_encodings
				&& char_code < 128 )
			{
			new_char_set = 2;		/* line drawing */
			new_char_code = char_code;
			}
		    break;
		    case TWO_BYTE_SET:
			new_char_set = 4;
		    break;
		    case FOUR_BYTE_SET:
			if (( char_set == DEC_HANYU_4 ) &&
			    ( ext_rend_row[x] == THIRD_OF_FOUR )) {
			    *bptr++ = LC1;
			    *bptr++ = LC2;
			}
			new_char_set = 4;
		    break;
		    case ONE_BYTE_SET:
			if ( char_set == JIS_ROMAN || char_set == KS_ROMAN )
			    new_char_set = 1;	/* JIS_ROMAN */
		    break;
		    }
		    /*
		     * Make sure the right character set is designated/invoked.
		     */
		    switch (new_char_set)
			{
			case 0:	/* national */
			case 3:	/* fallback required */
			    if ( invoked != 0 )
				{
				*bptr++ = '\17';	/* invoke G0 into GL */
				invoked = 0;
				}
			    break;
			case 1:	/* ASCII */
			    if ( invoked != 1 )
				{
				*bptr++ = '\16';	/* invoke G1 into GL */
				invoked = 1;
				}
			    if ( g1 != ASCII )
				{
				*bptr++ = '\33';	/* designate ASCII into G1 */
				*bptr++ = ')';
				*bptr++ = 'B';
				g1 = ASCII;
				}
			    break;
			case 2:	/* line drawing */
			    if ( invoked != 1 )
				{
				*bptr++ = '\16';	/* invoke G1 into GL */
				invoked = 1;
				}
			    if ( g1 != LINE_DRAWING )
				{
				*bptr++ = '\33';	/* designate line drawing */
				*bptr++ = ')';	/* ... into G1 */
				*bptr++ = '0';
				g1 = LINE_DRAWING;
				}
			    break;
			}
		    /*
		     * Make sure the right renditions are set.
		     */
		     if ( char_rend != renditions )
			{
			bptr = send_graphic_rendition( bptr, char_rend );
			renditions = char_rend;
			}
		    /*
		     * Print the character.
		     */
		    if ( new_char_set == 3 )
			bptr += lookup_fallback( w, char_code, char_set,
				ext_char_set, bptr );
		    else
			*bptr++ = new_char_code;
		    }
		/*
		 * Closing (last line only).
		 */
		if ( line == last_row )
		    {
		    if ( invoked != 0 )
			*bptr++ = '\17';	/* invoke G0 into GL */
		    if ( g1 != LINE_DRAWING )
			{
			*bptr++ = '\33';	/* designate line drawing into G1 */
			*bptr++ = ')';
			*bptr++ = '0';
			}
		    if ( renditions != 0 )
			{
			*bptr++ = '\33';
			*bptr++ = '[';
			*bptr++ = '0';
			*bptr++ = 'm';
			}
		    }
		/*
		 * Terminate the line and send it to the printer port.
		 */

		*bptr++ = terminator;
		*bptr++ = 0x0d;	/* carriage return in ASCII */
		_cld wvt$l_work_string.offset = bptr - _cld wvt$b_work_buffer;
		if ( WVT$PRINT_LINE( ld ) != DECwPrinterReady )
	  	{
    		    _cld wvt$w_print_flags &= ~pf1_m_auto_print_mode;
    		    w->common.printMode = DECwNormalPrintMode;
		    return;
	  	}
		bptr = _cld wvt$b_work_buffer;
		}
	    break;

	case DECwPrintAllCharacters:
	    /*
	     * Initialize.
	     */
	    if ( _cld wvt$l_ext_flags & vte1_m_tomcat ) {
		*bptr++ = '\33'; *bptr++ = '(';
		*bptr++ = _cld wvt$l_ext_specific_flags & vte2_m_jisroman_mode ?
			'J' : 'B';			  /* G0, Roman/ASCII */
		*bptr++ = '\33'; *bptr++ = ')'; *bptr++ = '0';	/* G1, Spec. */
		*bptr++ = '\33'; *bptr++ = '*'; *bptr++ = 'B';	/* G2, ASCII */
		*bptr++ = '\33'; *bptr++ = '$'; *bptr++ = '+';
		*bptr++ = '1';					/* G3, Kanji */
		*bptr++ = '\17'; gleft = 0;			/* G0 -> GL  */
		*bptr++ = '\33'; *bptr++ = '|'; gright = 3;	/* G3 -> GR  */
		g2_modified = False;
	    } else if ( _cld wvt$l_ext_flags & vte1_m_bobcat ) {
		*bptr++ = '\33'; *bptr++ = '('; *bptr++ = 'B';	/* G0, ASCII */
		*bptr++ = '\33'; *bptr++ = ')'; *bptr++ = '0';	/* G1, Spec. */
		*bptr++ = '\33'; *bptr++ = '*'; *bptr++ = 'B';	/* G2, ASCII */
		*bptr++ = '\33'; *bptr++ = '$'; *bptr++ = '+';
		*bptr++ = '2';					/* G3, Hanzi */
		*bptr++ = '\17'; gleft = 0;			/* G0 -> GL  */
		*bptr++ = '\33'; *bptr++ = '|'; gright = 3;	/* G3 -> GR  */
		g2_modified = False;
	    } else if ( _cld wvt$l_ext_flags & vte1_m_dickcat ) {
		*bptr++ = '\33'; *bptr++ = '(';
		if ( _cld wvt$l_ext_specific_flags & vte2_m_ksroman_mode ) {
		    *bptr++ = '%'; *bptr++ = '?';
		} else
		    *bptr++ = 'B';			  /* G0, Roman/ASCII */
		*bptr++ = '\33'; *bptr++ = ')'; *bptr++ = '0';	/* G1, Spec. */
		*bptr++ = '\33'; *bptr++ = '$'; *bptr++ = '*';
		*bptr++ = '4';					/* G2, Hangul */
		*bptr++ = '\33'; *bptr++ = '$'; *bptr++ = '+';
		*bptr++ = '4';					/* G3, Hangul */
		*bptr++ = '\17'; gleft = 0;			/* G0 -> GL  */
		*bptr++ = '\33'; *bptr++ = '|'; gright = 3;	/* G3 -> GR  */
		g2_modified = False;
	    } else if ( _cld wvt$l_ext_flags & vte1_m_fishcat ) {
		*bptr++ = '\33'; *bptr++ = '('; *bptr++ = 'B';	/* G0, ASCII */
		*bptr++ = '\33'; *bptr++ = ')'; *bptr++ = '0';	/* G1, Spec. */
		*bptr++ = '\33'; *bptr++ = '*'; *bptr++ = 'B';	/* G2, ASCII */
		*bptr++ = '\33'; *bptr++ = '$'; *bptr++ = '+';
		*bptr++ = '5';					/* G3, Hanyu */
		*bptr++ = '\17'; gleft = 0;			/* G0 -> GL  */
		*bptr++ = '\33'; *bptr++ = '|'; gright = 3;	/* G3 -> GR  */
		g2_modified = False;
	    } else {
	    *bptr++ = '\33';	/* designate the G0 set */
	    *bptr++ = '(';	/* 94 character set in G0 */
	    bptr = cpystr( bptr, designator[ _mld wvt$b_g_sets[0] ] );
	    *bptr++ = '\33';	/* designate the G1 set */
	    *bptr++ = IS_ISO_CHAR_SET(_mld wvt$b_g_sets[1]) ? '-' : ')';
				/* 96 or 94 character set in G1 */
	    bptr = cpystr( bptr, designator[ _mld wvt$b_g_sets[1] ] );
	    *bptr++ = '\33';	/* designate the G2 set */
	    *bptr++ = IS_ISO_CHAR_SET(_mld wvt$b_g_sets[2]) ? '.' : '*';
				/* 96 or 94 character set in G2 */
	    bptr = cpystr( bptr, designator[ _mld wvt$b_g_sets[2] ] );
	    *bptr++ = '\33';	/* designate the G3 set */
	    *bptr++ = IS_ISO_CHAR_SET(_mld wvt$b_g_sets[3]) ? '/' : '+';
				/* 96 or 94 character set in G3 */
	    bptr = cpystr( bptr, designator[ _mld wvt$b_g_sets[3] ] );
	    *bptr++ = '\17';	/* LS0 - invoke G0 into GL */
	    *bptr++ = '\33';
	    *bptr++ = '}';	/* LS2R - invoke G2 into GR */
	    g3 = _mld wvt$b_g_sets[3];
	    gleft = 0;
	    gright = 2;
	    }
	    *bptr++ = '\33';	/* initialize character rendition */
	    *bptr++ = '[';
	    *bptr++ = '0';
	    *bptr++ = 'm';
	    renditions = 0;
	    /*
	     * For each line.
	     */
	    if ( _cld wvt$w_print_flags & pf1_m_prt_display_mode )
		last_row += 1;
	    for ( line = first_row; line <= last_row; line++ )
		{
		bptr = send_line_attributes( bptr, _mld wvt$b_rendits[line] );
		if ( _cld wvt$w_print_flags & pf1_m_prt_display_mode &&
		    line == last_row ) {
		    char_row = _sld wvt$a_code_base[1];
		    rend_row = _sld wvt$a_rend_base[1];
		    ext_rend_row = _sld wvt$a_ext_rend_base[1];
		    start_char = 1;
		    end_char = _sld wvt$w_widths[1];
		} else {
		char_row = _mld wvt$a_code_base[ line ];
		rend_row = _mld wvt$a_rend_base[ line ];
		ext_rend_row = _mld wvt$a_ext_rend_base[ line ];
		start_char = ( line == first_row ) ? first_col : 1;
		/*
		 * Set end of line to skip trailing spaces.
		 */
		end_char = ( line == last_row ) ? last_col :
			_mld wvt$w_widths[ line ];
		}
		while ( end_char >= start_char )
		    if ( char_row[ end_char ] == '\0' ||
				char_row[ end_char ] == ' ' &&
		    		( rend_row[ end_char ] & ~(csa_M_BLINK |
				csa_M_UNDERLINE | csa_M_REVERSE)) == 0 )
			/*
			 * Character position is empty or has a space that
			 * is not blinking, underlined or reverse, so skip it.
			 */
			end_char--;
		    else
			break;
		/*
		 * Print each character on the line.
		 */
		for ( x = start_char; x <= end_char; x++ )
		    {
		    char_code = char_row[x];
		    if (char_code == 0)
			/* put spaces in place of nulls */
			char_code = ' ';
		    char_set = rend_row[x] & csa_M_CHAR_SET;
		    char_rend = rend_row[x] & ( csa_M_BOLD | csa_M_UNDERLINE |
				csa_M_BLINK | csa_M_REVERSE );
		    ext_char_set =
			_cld wvt$l_ext_flags ?
			ext_rend_row[x] & csa_M_EXT_CHAR_SET : 
			STANDARD_SET;
		    switch ( ext_char_set )
		    {
		    case STANDARD_SET:
		    switch ( char_set )
			{
			case ASCII:
			case LINE_DRAWING:
			    if ( char_code < 32 && w->common.v1_encodings )
				{
				/*
				 * In the DECwindows V1 font encoding, line
				 * drawing characters are in positions 1 to 31
				 * of the "ISO8859-1" fonts.  Map them back
				 * into their positions in the line drawing set.
				 */
				char_code += 95;
				char_set = LINE_DRAWING;
				}
			    break;
			case TECHNICAL:
			    if ( ! w->common.v1_encodings )
				{
				/*
				 * In the DECwindows V2 font coding, line
				 * drawing characters are in positions 33 to
				 * 126 of the DECtech fonts and technical
				 * characters are in positions 161 to 254.
				 * Map the technical characters into GL.
				 */
				if ( char_code >= 128 )
				    char_code -= 128;
				else
				    char_set = LINE_DRAWING;
				}
			    break;
			case ISO_LATIN_1:
			    if ( w->common.v1_encodings && char_code < 160 )
				/*
				 * In the DECwindows V1 font encoding there are
				 * DEC Supplemental characters in the right half
				 * of the "ISO8859-1" font, and ISO Latin 1
				 * Supplemental characters not in DEC
				 * Supplemental are mapped into the C1 space.
				 * We need to map these characters back into
				 * ISO Latin 1.
				 */
				char_code = mcs_c1_to_iso[ char_code & 127 ];
			    break;
			case SUPPLEMENTAL:
			    if ( ! w->common.v1_encodings && char_code <= 130 )
				/*
				 * In the DECwindows V2 font encoding there are
				 * ISO Latin 1 Supplemental characters in the
				 * right half of the ISO8859-1 font, and three
				 * DEC Supplemental characters not in ISO
				 * Latin 1 are mapped into the C1 space.  We
				 * need to map these characters back into DEC
				 * Supplemental.
				 */
				char_code = iso_c1_to_mcs[ char_code & 127 ];
			    break;
			}
		    break;
		    case ONE_BYTE_SET:
		    case TWO_BYTE_SET:
		    break;
		    case FOUR_BYTE_SET:
		    if (( char_set == DEC_HANYU_4 ) &&
			( ext_rend_row[x] == THIRD_OF_FOUR )) {
			*bptr++ = LC1;
			*bptr++ = LC2;
		    }
		    break;
		    }
		    /*
		     * Now we know the character set we want to use.  Find the
		     * first g-set that contains that character set.
		     */
		    if ( _cld wvt$l_ext_flags ) {
			switch ( ext_char_set ) {
			case STANDARD_SET:
			    if ( char_set == ASCII )
				g_set = 0;
			    else if ( char_set == LINE_DRAWING )
				g_set = 1;
			    else {
				if ( !( g2_modified ) || ( char_set != g2 )) {
				    *bptr++ = '\33';
				    *bptr++ = IS_ISO_CHAR_SET(char_set) ?
					'.' : '*';
				    bptr = cpystr( bptr,
					designator[char_set] );
				    g2 = char_set;
				    g2_modified = True;
				}
				g_set = 2;
			    }
			    break;
			case ONE_BYTE_SET:
			    g_set = 0;
			    break;
			case TWO_BYTE_SET:
			case FOUR_BYTE_SET:
			    g_set = 3;
			    break;
			}
		    } else {
		    for ( g_set = 0; g_set < 3; g_set++ )
			if ( char_set == _mld wvt$b_g_sets[g_set] )
			    break;
		    if ( g_set == 3 && char_set != g3 )
			{
			/*
			 * This character set isn't in any g-set, so we'll have
			 * to designate it.
			 */
			*bptr++ = '\33';
			*bptr++ = IS_ISO_CHAR_SET(char_set) ? '/' : '+';
				/* 96 or 94 character set in G3 */
			bptr = cpystr( bptr, designator[ char_set ] );
			g3 = char_set;
			}
		    }
		    if ( g_set < 2 )
			{
			/*
			 * G0 and G1 are in GL.  Invoke G0 or G1 if necessary,
			 * and strip the 8th bit off the character code.
			 */
			if ( g_set != gleft )
			    {
			    *bptr++ = ( g_set == 0 ) ? '\17' : '\16';
			    gleft = g_set;
			    }
			char_code &= 127;
			}
		    else
			{
			/*
			 * G2 and G3 are in GR.  Invoke G2 or G3 if necessary,
			 * and set the 8th bit in the character code.
			 */
			if ( g_set != gright )
			    {
			    *bptr++ = '\33';
			    *bptr++ = ( g_set == 2 ) ? '}' : '|';
			    gright = g_set;
			    }
			if ( _cld wvt$l_ext_flags & vte1_m_asian_common ) {
			    if ( g_set == 2 )
				char_code |= 128;
			} else
			char_code |= 128;
			}
		    /*
		     * Make sure the right renditions are set.
		     */
		    if ( char_rend != renditions )
			{
			bptr = send_graphic_rendition( bptr, char_rend );
			renditions = char_rend;
			}
		    /*
		     * Print the character.
		     */
		    *bptr++ = char_code;
		    }
		/*
		 * Closing (last line only).
		 */
		if ( line == last_row )
		    {
		    if ( _cld wvt$l_ext_flags & vte1_m_asian_common ) {
			if ( gleft != 0 )
			    *bptr++ = '\17';
			if ( gright != 3 ) {
			    *bptr++ = '\33';
			    *bptr++ = '|';
			}
			if ( g2_modified ) {
			    if ( _cld wvt$l_ext_flags & vte1_m_bobcat ) {
				*bptr++ = '\33'; *bptr++ = '*';
				*bptr++ = 'B';			/* G2, ASCII */
			    } else if ( _cld wvt$l_ext_flags & vte1_m_dickcat ) {
				*bptr++ = '\33'; *bptr++ = '$'; *bptr++ = '*';
				*bptr++ = '4';			/* G2, Hangul */
			    } else if ( _cld wvt$l_ext_flags & vte1_m_fishcat ) {
				*bptr++ = '\33'; *bptr++ = '*';
				*bptr++ = 'B';			/* G2, ASCII */
			    }
			    g2_modified = False;
			}
		    } else {
		    if ( gleft != 0 )
			*bptr++ = '\17';
		    if ( gright != 2 )
			{
			*bptr++ = '\33';
			*bptr++ = '}';
			}
		    if ( g3 != _mld wvt$b_g_sets[3] )
			{
			*bptr++ = '\33';
			*bptr++ = IS_ISO_CHAR_SET(_mld wvt$b_g_sets[3]) ?
				 '/' : '+';
			bptr = cpystr( bptr, designator[
				_mld wvt$b_g_sets[3] ] );
			}
		    }
		    if ( renditions != 0 )
			{
			*bptr++ = '\33';
			*bptr++ = '[';
			*bptr++ = '0';
			*bptr++ = 'm';
			}
		    }
		/*
		 * Terminate the line and send it to the printer port.
		 */

		*bptr++ = terminator;
		*bptr++ = 0x0d;	/* carriage return in ASCII */

		_cld wvt$l_work_string.offset = bptr - _cld wvt$b_work_buffer;
		if ( WVT$PRINT_LINE( ld ) != DECwPrinterReady )
	  	{
    		    _cld wvt$w_print_flags &= ~pf1_m_auto_print_mode;
    		    w->common.printMode = DECwNormalPrintMode;
		    return;
	  	}
		bptr = _cld wvt$b_work_buffer;
		}
	    break;

	}
}

/*
 * lookup_fallback - convert a character into an ASCII equivalent
 *
 * The returned value is the number of characters written to the output buffer.
 * The output buffer must be at least three characters long.
 */

int lookup_fallback( w, code, char_set, ext_char_set, string )
    DECtermWidget w;
    unsigned char code;
    int char_set;
    int ext_char_set;
    char *string;
{
    char *result = NULL, *sptr;

    switch ( ext_char_set )
    {
    case STANDARD_SET:
    if ( char_set == TECHNICAL )
    {
	if ( w->common.v1_encodings || code > 128 )
	    result = technical_fallback[ (code & 127) - 32 ];
	else if ( code >= 96 )
	    result = line_drawing_fallback[ code ];
    } else {
	if ( code >= 128 )
	{
	    if ( w->common.v1_encodings )
		result = supplemental_fallback[ code & 127 ];
	    else
		result = iso_latin_1_fallback[ code & 127 ];
	}
	else if ( code < 32 )
	    result = line_drawing_fallback[ code ];
    }
    break;
    case ONE_BYTE_SET:
    case TWO_BYTE_SET:
    case FOUR_BYTE_SET:
    default:
	break;
    }

    if ( result == NULL )
	{
	string[0] = code;
	return 1;
	}
    else
	{
	sptr = string;
	while ( *result != '\0' )
	    *sptr++ = *result++;
	return (sptr - string);
	}
}

/*
 * send_line_attributes
 *
 * This appends to the print buffer the escape sequences to set the attributes
 * of the current line in the display list.  It returns the address to be
 * used next in the print buffer (i.e. one past the last character appended).
 */

static unsigned char *send_line_attributes( bptr, rendits )
    unsigned char *bptr, rendits;
{
    *bptr++ = '\33';
    *bptr++ = '#';

    switch (rendits)
	{
	case 0:
	default:
	    *bptr++ = '5';	/* single width */
	    break;
	case csa_M_DOUBLE_WIDTH:
	    *bptr++ = '6';	/* double width */
	    break;
	case csa_M_DOUBLE_HIGH:
	    *bptr++ = '3';	/* double height top */
	    break;
	case csa_M_DOUBLE_HIGH | csa_M_DOUBLE_BOTTOM:
	    *bptr++ = '4';
	    break;
	}

    return bptr;
}

/*
 * send_graphic_rendition
 *
 * This appends to the print buffer the escape sequencs to set the graphic
 * rendition of the current character in the display list.  It returns the
 * address to be used next in the print buffer (i.e. one past the last
 * character appended).
 */

static unsigned char *send_graphic_rendition( bptr, char_rend )
    unsigned char *bptr;
    REND char_rend;
{
    *bptr++ = '\33';
    *bptr++ = '[';
    *bptr++ = '0';	/* start with renditions clear */
    if ( char_rend & csa_M_BOLD )
	{
	*bptr++ = ';';
	*bptr++ = '1';
	}
    if ( char_rend & csa_M_UNDERLINE )
	{
	*bptr++ = ';';
	*bptr++ = '4';
	}
    if ( char_rend & csa_M_BLINK )
	{
	*bptr++ = ';';
	*bptr++ = '5';
	}
    if ( char_rend & csa_M_REVERSE )
	{
	*bptr++ = ';';
	*bptr++ = '7';
	}
    *bptr++ = 'm';

    return bptr;
}

/*
 * get_window_image - read the contents of the window as an image
 *
 * In the first version of this routine we'll read directly from the window,
 * but the problem with this is that the window contents won't be correct if
 * the window is occluded.  Eventually we should compose an image based on the
 * contents of the pixmap backing store and the text display list.
 *
 * Instead of reading the entire window we'll read just 6 rows or columns
 * of pixels at a time; this should make things faster by reducing the
 * number of page faults.
 */

static XImage *get_window_image( w, x, y, width, height, pixmap )
    DECtermWidget w;
    int x, y, width, height;
    Pixmap pixmap;
{
    long plane_mask;
    int format;

/*
 * For now, always use the ZPixmap format.
 */

    plane_mask = AllPlanes;
    format = ZPixmap;

    return XGetImage( XtDisplay(w), pixmap, x + X_MARGIN, y + Y_MARGIN,
		      width, height, plane_mask, format );
}

/*
 * print_color_map - output the sixel commands to initialize the color map
 */

print_color_map( w )
    DECtermWidget w;
{
    int color, first_color, num_colors, syntax, cx, cy, cz, color_monitor;
    unsigned long mono;
    char buffer[80];

    color_monitor = ( w->common.visual->class != GrayScale
      && w->common.visual->class != StaticGray
      && w->common.graphics_mode != SINGLE_PLANE );

    first_color = w->common.printBackgroundMode ? 0 : 1;
    num_colors = 1 << w->common.bitPlanes;

    for ( color = first_color; color < num_colors; color++ )
	{
	if ( color_monitor )
	    {
	    if ( w->common.printHLSColorSyntax )
		{
		/*
		 * To define a color in HLS (hue, lightness, saturation) format
		 * we have to convert from X's RGB format to HLS.
		 */
		rgbhls( w->common.color_map[color].red,
			w->common.color_map[color].green,
			w->common.color_map[color].blue,
			&cx, &cy, &cz );
		syntax = 1;
		}
	    else
		{
		/*
		 * To define a color in RGB (red, green, blue) format we have
		 * convert from X's 0..65535 to sixel's 0..100.
		 */
		cx = TRIM_RGB( w->common.color_map[color].red );
		cy = TRIM_RGB( w->common.color_map[color].green );
		cz = TRIM_RGB( w->common.color_map[color].blue );
		syntax = 2;
		}
	    }
	else
	    {
	    /*
	     * On a monochrome monitor we only look at the "mono" component
	     * of each color.
	     */
	    mono = TRIM_RGB( w->common.color_map_mono[color] );
	    if ( w->common.printHLSColorSyntax )
		{
		cx = 0;		/* hue = 0 */
		cy = mono;	/* lightness = mono value, 0 to 100 */
		cz = 0;		/* saturation = 0 */
		syntax = 1;
		}
	    else
		{
		cx = mono;	/* red, green and blue are all mono value */
		cy = mono;
		cz = mono;
		syntax = 2;
		}
	    }
	sprintf( buffer, "#%d;%d;%d;%d;%d", color, syntax, cx, cy, cz );
	send_to_printer( w, buffer );
	}
}

/*
 * print_image_data - print an image in sixel format
 */

print_image_data( w, src_x, src_y, src_width, src_height, x_offset, y_offset,
		  pixmap )
    DECtermWidget w;	/* widget context */
    int src_x, src_y;	/* upper left corner of the rectangle of the window
			   to be printed */
    int src_width, src_height;
			/* width and height of the rectangle of the window
			   to be printed */
    int x_offset, y_offset;
			/* offset of printed data on the output device */
    Pixmap pixmap;      /* pixmap of the window */
{
    wvtp ld = w_to_ld( w );
    XImage *image;
    Boolean single_plane, rotated, color_seen[16], expanded, compressed;
    int start_x, end_x, start_y, end_y, x, y, color, line, output_width;
    int sixel, new_sixel, current_run_length, x1, y1, bitmask, num_colors;
    int start_color, i, rl_min, rl_left[6], rl_color[6], sixel_size, index;
    int new_color;
    Pixel pixel, new_pixel, pixel2, new_pixel2;
    char buffer[80], printable_sixel;
    typedef struct rl_struct	/* run length linked list element */
	{
	struct rl_struct *next;
	int count, color;
	} RL_STRUCT;
    RL_STRUCT *rl_heads[12], *rl_tail, *rl_tail_next, *rl_element[12];
    int net_sixel_size;

    single_plane = ( w->common.graphics_mode == SINGLE_PLANE );
    num_colors = ( 1 << w->common.bitPlanes );
    if ( w->common.printSixelLevel == DECwSixelLevel_1 )
	{
	if ( w->common.printFormat == DECwCompressedPrinting )
	    {
	    compressed = True;
	    expanded = False;
	    }
	else
	    {
	    compressed = False;
	    expanded = True;
	    }
	}
    else
	{
	compressed = False;
	expanded = False;
	}
    if ( compressed )
	sixel_size = 12;
    else
	sixel_size = 6;

/*
 * Validate the input parameters.
 */

    if ( src_x + src_width > w->common.display_width )
	src_width = w->common.display_width - src_x;
    if ( src_y + src_height > w->common.display_height )
	src_height = w->common.display_height - src_y;

/*
 * Decide on the loop conditions for x and y, depending on whether output is
 * rotated or not.  Note that x and y are always in terms of the image, which
 * may be rotated 90 degrees from the way the output is printed.
 *
 * If output is rotated, the top of the image will be printed at the left
 * edge of the page.
 */

    if ( w->common.printFormat == DECwRotatedPrinting )
	{
	rotated = True;
	start_x = src_width - 1;
	end_x = 0;
	}
    else
	{
	rotated = False;
	start_x = 0;
	end_x = src_width - 1;
	}
    start_y = 0;
    end_y = src_height - 1;

/*
 * Output enough graphic line feeds to account for y_offset.
 */
    while ( y_offset >= 20 )
	{
	send_to_printer( w, "~~~~~~~~~~~~~~~~~~~~" );
	y_offset -= 20;
	}
    for ( i = 0; i < y_offset; i++ )
	buffer[i] = '~';
    buffer[y_offset] = '\0';
    send_to_printer( w, buffer );

/*
 * Main loop: for each line of sixel output.  If the output is rotated then
 * we loop on x, and if the output is not rotated we loop on y.  Each pass
 * through the loop we process either 6 columns (if rotated) or six rows
 * (if unrotated) of the image.  This is because a "sixel" is a vertical
 * column of 6 pixels.
 */

    for ( x = start_x, y = start_y; rotated ? ( x >= end_x ) : ( y <= end_y );
		rotated ? ( x -= sixel_size, y = start_y )
			: ( y += sixel_size, x = start_x ) )
	{

	/*
	 * Read the image data for this sixel line from the window.
	 */

	if ( rotated ) {
	    net_sixel_size = x >= sixel_size ? sixel_size : sixel_size - abs(x);

	    image = get_window_image( w,
			x >= sixel_size ? (x - sixel_size + 1 + src_x) : 0,
	      		start_y + src_y, net_sixel_size, src_height, pixmap );
	} else {
	    net_sixel_size = ( y + src_y + sixel_size ) < src_height ?
			       sixel_size : src_height - (y + src_y);

	    image = get_window_image( w, start_x + src_x, y + src_y,
		src_width, net_sixel_size, pixmap );
	}

	if ( !image )
	    {
	    widget_message( w, "XGetImage failed.\n" );
	    break;  /* Don't return here or else pointer won't be ungrabbed*/
	    }

	for ( color = 0; color < num_colors; color++ )
	    color_seen[ color ] = False;

	/*
	 * Build the arrays of run-lengths.  This has the advantage on a
	 * multi-plane system that we only have to make one pass through
	 * the image structure, instead of one pass per color.
	 */

	for ( line = 0;
	      line < net_sixel_size;
	      compressed ? (line += 2) : (line++) )
	    {
	    if ( compressed )
		index = line / 2;
	    else
		index = line;
	    if ( rotated )
		{
		x1 = net_sixel_size - line - 1;
		y1 = 0;
		}
	    else
		{
		x1 = 0;
		y1 = line;
		}

	    /*
	     * Start the first run length to get things going.
	     */

	    rl_heads[ index ] = rl_tail = ALLOC_ARRAY( RL_STRUCT, 1 );
	    pixel = XGetPixel( image, x1, y1 );
	    if ( compressed )
		pixel2 = XGetPixel( image, rotated ? ( x1 - 1 ) : x1,
					   rotated ? y1 : ( y1 + 1 ) );
	    rl_tail->color = pixel_to_color( w, pixel );
	    if ( compressed )
		rl_tail->color |= pixel_to_color( w, pixel2 );
	    color_seen[ rl_tail->color ] = True;
	    rl_tail->count = 0;

	    /*
	     * Scan through the image data and build up the run length array.
	     */

	    for ( ; rotated ? ( y1 <= end_y ) : ( x1 <= end_x ) ;
		    rotated ? ( y1++ )	      : ( x1++ ) )
		{

		new_pixel = XGetPixel( image, x1, y1 );
		if ( compressed )
		    new_pixel2 = XGetPixel( image, rotated ? ( x1 - 1 ) : x1,
						   rotated ? y1 : ( y1 + 1 ) );
		if ( new_pixel == pixel
		  && ( ! compressed || new_pixel2 == pixel2  ) )
		    rl_tail->count++;
		else
		    {

		    /*
		     * The pixel values have changed.  See if the color has
		     * also changed.
		     */

		    new_color = pixel_to_color( w, new_pixel );
		    if ( compressed )
			new_color |= pixel_to_color( w, new_pixel2 );
		    if ( new_color == rl_tail->color )
			rl_tail->count++;
		    else
			{

			/*
			 * The color has changed, so start a new run length.
			 */

			rl_tail->next = ALLOC_ARRAY( RL_STRUCT, 1 );
			rl_tail = rl_tail->next;
			pixel = new_pixel;
			if ( compressed )
			    pixel2 = new_pixel2;
			rl_tail->color = new_color;
			color_seen[ new_color ] = True;
			rl_tail->count = 1;
			}
		    }
		}
	    rl_tail->next = NULL;
	    }

	/*
	 * Now scan through the run length structures for the six scan lines
	 * and output the sixel data for each color.
	 */

	for ( color = ( w->common.printBackgroundMode ? 0 : 1 );
		color < num_colors; color++ )
	    {
	    if ( color_seen[ color ] )
		{
		sprintf( buffer, "#%d", color );
		send_to_printer( w, buffer );
		if ( x_offset > 0 )
		    {
		    sprintf( buffer, "!%d?", x_offset );
		    send_to_printer( w, buffer );
		    }
		if ( rotated )
		    output_width = image->height;
		else
		    output_width = image->width;
		x1 = 0;

		/*
		 * Start the first sixel.
		 */

		sixel = 0;
		/* don't hard code to 6 lines, depend on how many pixel
		 * captured
		 */
		for ( line = 0, bitmask = 1;
		      line < net_sixel_size && line <= index;
		      line++, bitmask <<= 1 )
		    {
		    if  ( rl_heads[ line ]->color == color )
			sixel |= bitmask;
		    rl_element[ line ] = rl_heads[ line ];
		    rl_left[ line ] = rl_element[ line ]->count;
		    rl_color[ line ] = rl_element[ line ]->color;
		    }

		current_run_length = 0;

		while ( x1 < output_width )
		    {

		    /*
		     * Find the shortest remaining run length.
		     */

		    rl_min = rl_left[ 0 ];

		    /* not necessarily 6 lines, depends on net_sixel_size
		     */
		    for ( line = 1;
			  line < net_sixel_size && line <= index;
			  line++ )
			if ( rl_left[ line ] < rl_min )
			    rl_min = rl_left[ line ];

		    /*
		     * Add this into the current run length and subtract it
		     * from each remaining run length.  One or more run
		     * lengths will go to zero.
		     */

		    current_run_length += rl_min;
		    x1 += rl_min;
		    new_sixel = sixel;

		    /* not necessarily 6 lines, depends on the net_sixel_size
		     */
		    for ( line = 0, bitmask = 1;
			  line < net_sixel_size && line <= index;
			  line++, bitmask <<= 1 )
			{
			rl_left[ line ] -= rl_min;
			if ( rl_left[ line ] == 0 )
			    {

			    /*
			     * When a run length goes to zero, advance to
			     * the next element in the linked list.
			     */

			    rl_element[ line ] = rl_element[ line ]->next;
			    if ( rl_element[ line ] == NULL )
				new_sixel &= ~bitmask;
			    else
				{
				rl_left[ line ] = rl_element[ line ]->count;
				rl_color[ line ] = rl_element[ line ]->color;
				if ( rl_color[ line ] == color )
				    new_sixel |= bitmask;
				else
				    new_sixel &= ~bitmask;
				}
			    }

			}

		    /*
		     * If the sixel value changes, output a run length and
		     * printable sixel.
		     */

		    if ( new_sixel != sixel )
			{
			printable_sixel = sixel + '?';
			if ( expanded )
			    current_run_length *= 2;
			if ( current_run_length >= 3 )
			    {
			    sprintf( buffer, "!%d%c", current_run_length,
			      printable_sixel );
			    send_to_printer( w, buffer );
			    }
			else
			    {
			    for ( i = 0; i < current_run_length; i++ )
				buffer[i] = printable_sixel;
			    buffer[ current_run_length ] = '\0';
			    send_to_printer( w, buffer );
			    }
			sixel = new_sixel;
			current_run_length = 0;
			}
		    }

		/*
		 * At the end of a color, output a graphics carriage return.
		 */

		send_to_printer( w, "$" );
		}
	    }

	/*
	 * At the end of a line, output a graphics line feed.
	 */

	send_to_printer( w, "-" );

	/*
	 * Free the memory that we've allocated.
	 */

	for ( line = 0; line < net_sixel_size && line <= index; line++ )
	    for ( rl_tail = rl_heads[ line ]; rl_tail != NULL; )
		{
		rl_tail_next = rl_tail->next;
		XtFree( (char *)rl_tail );
		rl_tail = rl_tail_next;
		}
	XDestroyImage( image );

	}
}

/*
 * send_to_printer - send buffered data to the printer
 *
 * This routine will send data to the printer whenever it has 80 characters
 * of data to send.  The data sent to this routine is assumed to be atomic,
 * i.e. the routine will never insert a carriage return-line feed in the
 * middle of a packet of data.  This is because some printers can't deal
 * with repeat counts etc. split across line boundaries.
 */

send_to_printer( w, data )
    DECtermWidget w;
    char *data;
{
    wvtp ld = w_to_ld( w );
    int len = strlen( data );

    if ( len + _cld wvt$l_work_string.offset > 80 )
	flush_print_buffer( w );
    cpystr( _cld wvt$b_work_buffer + _cld wvt$l_work_string.offset, data );
    _cld wvt$l_work_string.offset += len;
}

/*
 * flush_print_buffer - send a line of data to the printer, terminated with
 * a carriage return and line feed.
 */

flush_print_buffer( w )
    DECtermWidget w;
{
    wvtp ld = w_to_ld( w );

    _cld wvt$b_work_buffer[ _cld wvt$l_work_string.offset++ ] = '\r';
    _cld wvt$b_work_buffer[ _cld wvt$l_work_string.offset++ ] = '\n';
    WVT$PRINT_LINE( w );
}

/*
 * pixel_to_color - convert a hardware pixel value to a sixel color
 *
 * Returned value: the sixel color number, 0 - 15.
 */

int pixel_to_color( w, pixel )
    DECtermWidget w;
    Pixel pixel;
{
    XColor color_struct;
    int color, i, min_diff, min_index, diff;
    int num_colors = ( 1 << w->common.bitPlanes );

#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
    globalref
#else
    extern
#endif
    int ansi_color_entries[];

    if ( w->common.color_map_allocated )
	{
	for ( color = 0; color < num_colors; color++ )
	    {
	    if ( w->common.color_map[color].pixel == pixel )
		return color;
	    }
	}
    else if ( w->manager.foreground == pixel )
	return w->common.text_foreground_index;
    else if ( w->core.background_pixel == pixel )
	return w->common.text_background_index;

    for ( i = 0; i < NUM_TEXT_COLORS; i++ )
	if ( w->output.color_pixel_allocated[i]
	  && w->output.color_pixels[i] == pixel )
	    return ansi_color_entries[i];

/*
 * This isn't any color we know about, so read its RGB value and choose the
 * color in our color table that comes closest to matching it.
 */

    if ( ! w->common.color_map_allocated )
	return 0;

    color_struct.pixel = pixel;
    XQueryColor( XtDisplay(w), w->core.colormap, &color_struct );

    min_diff = 99999;
    min_index = 0;

    for ( color = 0; color < num_colors; color++ )
	{
	diff = abs( color_struct.red - w->common.color_map[color].red )
	  + abs( color_struct.blue - w->common.color_map[color].blue )
	  + abs( color_struct.green - w->common.color_map[color].green );
	if ( diff < min_diff )
	    {
	    min_diff = diff;
	    min_index = color;
	    }
	}
    return min_index;
}

static int abs( n )
    int n;
{
    return ( (n < 0) ? (-n) : (n) );
}
