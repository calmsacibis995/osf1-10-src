/*
 *  Title:	DT_resources.h -- DECterm resource list
 *
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © Digital Equipment Corporation, 1988, 1993  All Rights      |
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
 *	This is the resource list for DECterm.
 *
 *  Author:	Tom Porcher  1987
 *
 *  Modification history:
 *
 * Alfred von Campe     18-Dec-1993     BL-E
 *	- Foreground resource is now inherited from XmManager.
 *
 *  Alfred von Campe    08-Nov-1993     BL-E
 *      - Add F11 key feature from dxterm.
 *
 *  Grace Chung         15-Sep-1993     BL-E
 *      - Add 7-bit/8-bit printer support.
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Alfred von Campe    20-Feb-1992     V3.1
 *      - Add color text support.
 *
 *  Aston Chan		08-Jan-1991	V3.1 (Alpha)
 *	- DECC complained about an #ifdef in a macro parameter.
 *
 *  Aston Chan		19-Dec-1991	V3.1
 *	- I18n code merge
 *
 *  Alfred von Campe	13-Nov-1991     V3.1
 *	- Added autoAdjustPosition resource.
 *
 *  Alfred von Campe    06-Oct-1991     Hercules/1 T0.7
 *	- Added F11 key resource from ULTRIX.
 *
 * Alfred von Campe     28-May-1991     V3.0
 *	- Change second DECwNdisplayHeight to DECwNdisplayWidthInc (old typo).
 *
 * Bob Messenger	15-Sep-1990	X3.0-7
 *	- Add support for GS fonts.
 *
 * Bob Messenger	12-Sep-1990	X3.0-7
 *	- Default to sixel level 2 for graphics print screen output.
 *
 *  Michele Lien    11-Dec-1990	VXT X0.0 BL2
 *  - VXT can only direct printer output to a dedicated printer port, "printer
 *    destination" option in printer customize menu for DECterm V3.0 has been 
 *    taken out.  All the resources associated with printer destination and 
 *    port name can only be conditionally compiled for non-VXT systems.  
 *    Printer destination and printer port name has been hard coded to the 
 *    local printer port.
 *
 * Bob Messenger	19-Jul-1990	X3.0-5
 *	- Change the defaults for the big and little font set names to "",
 *	  since they will be calculated at run time.
 *
 * Bob Messenger	17-Jul-1990	X3.0-5
 *	Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- terminal type resource
 *	- resources specific to Japanese terminal (Tomcat)
 *
 * Bob Messenger	17-Jul-1990	X3.0-5
 * 	- Fix declaration of printHLSColorSyntax resource.
 *
 * Bob Messenger	23-Jun-1990	X3.0-5
 *	- Add resource definitions to support the printer port.
 *
 * Mark Woodbury	25-May-1990 X3.0-3M
 * - Motif update
 *
 * Bob Messenger	 9-Apr-1990	X3.0-2
 *	- Merge UWS and VMS changes.
 *
 * Bob Messenger	12-Mar-1990	V2.1 (VMS V5.4)
 *	- Declare bitPlanes before shareColormapEntries and enableBackingStore,
 *	  to prevent a crash when more than one resource is changed in the
 *	  same XtSetValues call.
 *	- Use the right resource name when declaring displayWidthInc.
 *
 * Bob Messenger	27-Feb-1990	V2.1 (UWS V4.0)
 *	- Add resources for secure keyboard in UWS V4.0.
 *
 * Bob Messenger	10-Oct-1989	V2.0 (UWS V2.2)
 *	- Add class name for FontUsed, so the set_value routine will be
 *	  called (avoids malloc/free problem).
 *
 * Bob Messenger	15-Aug-1989	X2.0-15
 *	- Add syncFrequency, so the user can control the trade-off between
 *	  speed and responsiveness.
 *
 * Bob Messenger	31-May-1989	X2.0-8
 *	- Add set_value routine for fontUsed, to prevent it's value from
 *	  being exchanged except from within the DECterm widget.
 *
 * Bob Messenger	14-May-1989	X2.0-8
 *	- Add errorCallback resources, and remove class definitions for
 *	  read-only reasources.
 *
 * Bob Messenger	21-Apr-1989	X2.0-7
 *	- Avoid Ultrix compilation warning by making using (caddr_t) for
 *	  integer initializers.
 *
 * Bob Messenger	11-Apr-1989	X2.0-6
 *	- Added saveErasedLines.
 *
 * Bob Messenger	09-Apr-1989	X2.0-6
 *	- Added helpCallback.
 *	- Use new bindings for keyboard mapping resource values.
 *
 * Bob Messenger	07-Apr-1989	X2.0-6
 *	- Convert to new widget bindings (DECw prefix).
 *
 * Bob Messenger	01-Apr-1989	X2.0-5
 *	- Added shellValuesCallback, defaultTitle and defaultIconName.
 *	- Set default for transcriptSize to 500 lines, since it's
 *	  working now.
 *
 * Bob Messenger	15-Mar-1989	X2.0-3
 *	- Added terminalDriverResize, transcriptSize, shareColormapEntries,
 *	  bitPlanes and backingStore resources.
 *
 * Bob Messenger	15-Mar-1989	X2.0-2
 *	- Added fontUsed resource.
 *
 * Eric Osman		13-Sep-1988	FT2.1
 *	- Use symbolic names from decterm.h instead of 0 for enumerated defaults
 *
 * Tom Porcher		16-Aug-1988	X0.4-43
 *	- Added controlQSHold resource.
 *
 * Tom Porcher		13-Aug-1988	X0.4-43
 *	- Added reverseVideo resource.
 *	- Added callback for tabStops.
 *	- Removed transcript.
 *
 * Eric Osman		18-May-1988	X0.4-27
 *	- Added cursor blink
 *
 * Tom Porcher		 9-May-1988	X0.4-26
 *	- Added "maxInput" resource.
 *
 * Tom Porcher		 5-Apr-1988	X0.4-7
 *	- Changed default for background and foreground to XtMumble.
 *	- Removed "colormap" resource; it is in Core.
 *
 */

/*
 * DECterm_resources.h -- DECterm resource list
 *
 * Parameters to RESOURCE macro:
 *
 *    #1-#3:	Elements 1,2,3 of XtResource structure:
 *                  name, class, representation-type.
 *    #4:	C type of resource.
 *    #5:	Name of element in private part of Widget data record.
 *    #6-#7:	Elements 6,7 of XtResource structure:
 *		    default-type, default.
 *    #8:       Name of routine to call when value is changed or NULL.
 *
 * Parameters to C_RESOURCE macro are the same except:
 *    #5:	Name of element in any part of Widget data record.
 */

/* C_RESOURCE( "translations", "Translations", XtRStringTable,
	XtTranslations, core.translations,
	XtRStringTable, (caddr_t)&default_translations,
	NULL )
			/* translation table for user events to
			   terminal codes and actions */

RESOURCE( DECwNinputCallback, XtCCallback, XtRCallback,
	XtCallbackList, inputCallback,
	XtRCallback, NULL ,
	i_set_value_inputCallback )
			/* callback to receive ANSI keyboard and report data */

RESOURCE( DECwNstopOutputCallback, XtCCallback, XtRCallback,
	XtCallbackList, stopOutputCallback,
	XtRCallback, NULL ,
	c_set_value_callbacks )
			/* callback to stop ANSI data to widget */

RESOURCE( DECwNstartOutputCallback, XtCCallback, XtRCallback,
	XtCallbackList, startOutputCallback,
	XtRCallback, NULL ,
	c_set_value_callbacks )
			/* callback to resume ANSI data to widget */

RESOURCE( DECwNresizeCallback, XtCCallback, XtRCallback,
	XtCallbackList, resizeCallback,
	XtRCallback, NULL ,
	c_set_value_callbacks )
			/* callback to request a resize */

RESOURCE( DECwNshellValuesCallback, XtCCallback, XtRCallback,
	XtCallbackList, shellValuesCallback,
	XtRCallback, NULL,
	c_set_value_callbacks )
			/* callback to request SetValues on shell */

RESOURCE( DECwNhelpCallback, XtCCallback, XtRCallback,
	XtCallbackList, helpCallback,
	XtRCallback, NULL,
	c_set_value_callbacks )
			/* callback for context sensitive help */

RESOURCE( DECwNerrorCallback, XtCCallback, XtRCallback,
	XtCallbackList, errorCallback,
	XtRCallback, NULL,
	c_set_value_callbacks )
			/* callback for error/informational messages */

RESOURCE( DECwNmaxInput, "MaxInput", XtRInt,
	int, maxInput,
	XtRImmediate, (caddr_t) 32767,
	NULL )
			/* max number of characters that can be input */

RESOURCE( DECwNcolumns, "Columns", XtRInt,
	int, columns,
	XtRImmediate, (caddr_t) 80,
	s_set_value_columns_rows )
			/* width of logical display in columns */

RESOURCE( DECwNrows, "Rows", XtRInt,
	int, rows,
	XtRImmediate, (caddr_t) 24,
	s_set_value_columns_rows )
			/* height of logical display in rows */

RESOURCE( DECwNdisplayWidth, "", XtRInt,
	int, displayWidth,
	XtRImmediate, (caddr_t) 0,
	NULL )
			/* width of logical display including scroll bars
			   and other decorationin pixels; read-only */

RESOURCE( DECwNdisplayHeight, "", XtRInt,
	int, displayHeight,
	XtRImmediate, (caddr_t) 0,
	NULL )
			/* height of logical display including scroll bars
			   and other decorationin pixels; read-only */

RESOURCE( DECwNdisplayWidthInc, "", XtRInt,
	int, displayWidthInc,
	XtRImmediate, (caddr_t) 0,
	NULL )
			/* size increment for width of logical display
			   in pixels; read-only */

RESOURCE( DECwNdisplayHeightInc, "", XtRInt,
	int, displayHeightInc,
	XtRImmediate, (caddr_t) 0,
	NULL )
			/* size increment for height of logical display
			   in pixels; read-only */

RESOURCE( DECwNscrollHorizontal, "Scroll", XtRBoolean,
	Boolean, scrollHorizontal,
	XtRImmediate, (caddr_t) FALSE,
	o_set_value_scrollHorizontal )
			/* Horizontal scroll bar enable */

RESOURCE( DECwNscrollVertical, "Scroll", XtRBoolean,
	Boolean, scrollVertical,
	XtRImmediate, (caddr_t) TRUE,
	o_set_value_scrollVertical )
			/* Vertical scroll bar enable */

RESOURCE( DECwNfontSetSelection, "FontSetSelection", XtRInt,
	DECwFontSetSelection, fontSetSelection,
	XtRImmediate, (caddr_t) 1,		/* Little_Font */
     	o_set_value_fontSetSelection )
			/* current set of fonts */

RESOURCE( DECwNbigFontSetName, "BigFontSetName", XtRString,
	String, bigFontSetName,
	XtRString, "",
     	o_set_value_bigFontSetName )
			/* Big Font set name (default chosen at run time) */

RESOURCE( DECwNlittleFontSetName, "LittleFontSetName", XtRString,
	String, littleFontSetName,
	XtRString, "",
     	o_set_value_littleFontSetName )
			/* Little Font set name (default chosen at run time) */

RESOURCE( DECwNgsFontSetName, "GSFontSetName", XtRString,
	String, gsFontSetName,
	XtRString, DEFAULT_GS_FONT_SET_NAME,
	o_set_value_gsFontSetName )
			/* German Standard Font set name */

RESOURCE( DECwNfontUsed, "FontUsed", XtRString,
	String, fontUsed,
	XtRString, NULL,
	o_set_value_fontUsed )
	    		/* read-only: name of base font */

RESOURCE( DECwNbigFontSetSelection, "BigFontSetSelection", XtRBoolean,
	Boolean,bigFontSetSelection,
	XtRImmediate, (caddr_t) False,		/* Default Big Font */
	NULL)
			/* current set of big font */

RESOURCE( DECwNbigFontOtherName, "BigFontOtherName", XtRString,
	String, bigFontOtherName,
	XtRString, "",
     	o_set_value_bigFontOtherName )
			/* Big Font other name */

RESOURCE( DECwNlittleFontSetSelection, "LittleFontSetSelection", XtRBoolean,
	Boolean,littleFontSetSelection,
	XtRImmediate, (caddr_t) False,		/* Default Little Font */
	NULL)
			/* current set of little font */

RESOURCE( DECwNlittleFontOtherName, "LittleFontOtherName", XtRString,
	String, littleFontOtherName,
	XtRString, "",
     	o_set_value_littleFontOtherName )
			/* Little Font other name */

RESOURCE( DECwNgsFontSetSelection, "GsFontSetSelection", XtRBoolean,
	Boolean,gsFontSetSelection,
	XtRImmediate, (caddr_t) False,		/* Default Gs Font */
	NULL)
			/* current set of gs font */

RESOURCE( DECwNgsFontOtherName, "GsFontOtherName", XtRString,
	String, gsFontOtherName,
	XtRString, "",
     	o_set_value_gsFontOtherName )
			/* Gs Font other name */

RESOURCE( DECwNcursorStyle, "cursorStyle", XtRInt,
	DECwCursorStyle, cursorStyle,
	XtRImmediate, (caddr_t) DECwCursorBlock,
	o_set_value_cursorStyle )
			/* Cursor style, one of:
			   DECwCursorBlock or DECwCursorUnderline */

RESOURCE( DECwNtextCursorEnable, "TextCursorEnable", XtRBoolean,
	Boolean, textCursorEnable,
	XtRImmediate, (caddr_t) TRUE,
	s_set_value_textCursorEnable )
			/* Text Cursor enable */

RESOURCE( DECwNcouplingHorizontal, "CouplingHorizontal", XtRBoolean,
	Boolean, couplingHorizontal,
	XtRImmediate, (caddr_t) FALSE,
	NULL )
			/* Horizontal cursor coupling */

RESOURCE( DECwNcouplingVertical, "CouplingVertical", XtRBoolean,
	Boolean, couplingVertical,
	XtRImmediate, (caddr_t) TRUE,
	NULL )                  
			/* Horizontal cursor coupling */

RESOURCE( DECwNautoResizeTerminal, "AutoResizeTerminal", XtRBoolean,
	Boolean, autoResizeTerminal,
	XtRImmediate, (caddr_t) FALSE,
	NULL )
			/* Automatically match terminal to window size */

RESOURCE( DECwNautoResizeWindow, "AutoResizeWindow", XtRBoolean,
	Boolean, autoResizeWindow,
	XtRImmediate, (caddr_t) TRUE,
	NULL )
			/* Automatically match window to logical display size */

RESOURCE( DECwNterminalDriverResize, "TerminalDriverResize", XtRBoolean,
	Boolean, terminalDriverResize,
	XtRImmediate, (caddr_t) TRUE,
	NULL )
			/* Automatically match terminal to size in driver,
			   and vice-versa */

RESOURCE( DECwNautoAdjustPosition, "AutoAdjustPosition", XtRBoolean,
	Boolean, autoAdjustPosition,
	XtRImmediate, (caddr_t) TRUE,
	NULL )
			/* Automatically adjust position to fit on screen */

RESOURCE( DECwNcondensedFont, "CondensedFont", XtRBoolean,
	Boolean, condensedFont,
	XtRImmediate, (caddr_t) FALSE,
	o_set_value_condensedFont )
			/* use 132 column condensed font */

RESOURCE( DECwNadjustFontSizes, "AdjustFontSizes", XtRBoolean,
	Boolean, adjustFontSizes,
	XtRImmediate, (caddr_t) TRUE,
	NULL )
			/* used 132 column font when DECCOLM is set */

RESOURCE( DECwNstatusDisplayEnable, "StatusDisplayEnable", XtRBoolean,
	Boolean, statusDisplayEnable,
	XtRImmediate, (caddr_t) FALSE,
	s_set_value_statusDisplayEnable )
			/* Enable host-writeable status display */

RESOURCE( DECwNlockUDK, "LockUDK", XtRBoolean,
	Boolean, lockUDK,
	XtRImmediate, (caddr_t) FALSE,
	s_set_value_misc )
			/* Prevent changes to UDK definitions */

RESOURCE( DECwNlockUserFeatures, "LockUserFeatures", XtRBoolean,
	Boolean, lockUserFeatures,
	XtRImmediate, (caddr_t) FALSE,
	s_set_value_misc )
			/* Prevent changes to user features */

RESOURCE( DECwNuserPreferenceSet, "UserPreferenceSet", XtRInt,
	DECwUserPreferenceSet, userPreferenceSet,
	XtRImmediate, (caddr_t) DECwDEC_Supplemental,
	s_set_value_userPreferenceSet )
			/* UserPreferenceSet, one of:
			   DECwDEC_Supplemental, DECwISO_Supplemental */

RESOURCE( DECwNterminalMode, "TerminalMode", XtRInt,
	DECwTerminalMode, terminalMode,
	XtRImmediate, (caddr_t) DECwVT300_7_BitMode,
	s_set_value_terminalMode )
			/* Terminal operating mode, one of:
			   DECwVT52_Mode, DECwVT100_Mode,
			   DECwVT300_7_BitMode, DECwVT300_8_BitMode */

RESOURCE( DECwNresponseDA, "ResponseDA", XtRInt,
	DECwResponseDA, responseDA,
	XtRImmediate, (caddr_t) DECwDECtermID,
	s_set_value_responseDA )
			/* Response to Device Attributes (terminal ID)
			   request, one of:
			     DECwVT100_ID, DECwVT101_ID, DECwVT102_ID,
			     DECwVT125_ID, DECwVT220_ID, DECwVT240_ID,
			     DECwVT320_ID, DECwVT340_ID, DECwDECtermID,
			     DECwVT330_ID */

RESOURCE( DECwNeightBitCharacters, "EightBitCharacters", XtRBoolean,
	Boolean, eightBitCharacters,
	XtRImmediate, (caddr_t) TRUE,
	s_set_value_eightBitCharacters )
			/* 8 bit characters (multinational mode) */

RESOURCE( DECwNmarginBellEnable, "MarginBellEnable", XtRBoolean,
	Boolean, marginBellEnable,
	XtRImmediate, (caddr_t) FALSE,
	s_set_value_misc )
			/* Margin Bell on */

RESOURCE( DECwNwarningBellEnable, "WarningBellEnable", XtRBoolean,
	Boolean, warningBellEnable,
	XtRImmediate, (caddr_t) TRUE,
	s_set_value_misc )
			/* Warning Bell on */

RESOURCE( DECwNcontrolQSHold, "ControlQSHold", XtRBoolean,
	Boolean, controlQSHold,
	XtRImmediate, (caddr_t) TRUE,
	NULL )
			/* TRUE means ^S and ^Q hold/unhold screen  */

RESOURCE( DECwNsaveLinesOffTop, "SaveLinesOffTop", XtRBoolean,
	Boolean, saveLinesOffTop,
	XtRImmediate, (caddr_t) TRUE,
	s_set_value_saveLinesOffTop )
			/* Save transcript of lines off top */

RESOURCE( DECwNtranscriptSize, "TranscriptSize", XtRInt,
	int, transcriptSize,
	XtRImmediate, (caddr_t) 500,
	s_set_value_transcriptSize )
			/* number of lines saved off top */

RESOURCE( DECwNnewLineMode, "NewLineMode", XtRBoolean,
	Boolean, newLineMode,
	XtRImmediate, (caddr_t) FALSE,
	s_set_value_misc )
			/* (LNM) Line-feed/Newline mode,
			   true = Newline, false = Line Feed */

RESOURCE( DECwNscreenMode, "screenMode", XtRBoolean,
	Boolean, screenMode,
	XtRImmediate, (caddr_t) TRUE,
	s_set_value_screenMode )
			/* (DECSCM) Screen mode,
			   true = dark on light, false = light on dark */

RESOURCE( DECwNautoWrapEnable, "AutoWrapEnable", XtRBoolean,
	Boolean, autoWrapEnable,
	XtRImmediate, (caddr_t) FALSE,
	s_set_value_misc )
			/* (DECAWM) Auto-wrap mode */

RESOURCE( DECwNcursorBlinkEnable, "CursorBlinkEnable", XtRBoolean,
	Boolean, cursorBlinkEnable,
	XtRImmediate, (caddr_t) TRUE,
	s_set_value_cursor_blink )
			/* (DECCBK) Cursor-blink mode */

RESOURCE( DECwNautoRepeatEnable, "AutoRepeatEnable", XtRBoolean,
	Boolean, autoRepeatEnable,
	XtRImmediate, (caddr_t) TRUE,
	s_set_value_autoRepeatEnable )
			/* (DECARM) Keyboard autorepeat */

RESOURCE( DECwNapplicationKeypadMode, "ApplicationKeyMode", XtRBoolean,
	Boolean, applicationKeypadMode,
	XtRImmediate, (caddr_t) FALSE,
	s_set_value_misc )
			/* Application Keypad mode */
           
RESOURCE( DECwNapplicationCursorKeyMode, "ApplicationKeyMode", XtRBoolean,
	Boolean, applicationCursorKeyMode,
	XtRImmediate, (caddr_t) FALSE,
	s_set_value_misc )
			/* Application Cursor Key mode */

RESOURCE( DECwNbackarrowKey, "BackarrowKey", XtRInt,
	DECwBackarrowKey, backarrowKey,
	XtRImmediate, (caddr_t) DECwBackarrowDelete,
	NULL )
			/* Backarrow key code, one of:
			     DECwBackarrowDelete, DECwBackarrowBackspace */

RESOURCE( DECwNperiodCommaKeys, "PeriodCommaKeys", XtRInt,
	DECwPeriodCommaKeys, periodCommaKeys,
	XtRImmediate, (caddr_t) DECwCommaPeriodComma,
	NULL )
			/* Period/Comma keys shifted, one of:
			      DECwCommaPeriodComma, DECwCommaAngleBrackets */

#if !defined(VXT_DECTERM)
RESOURCE( DECwNf11EscapeKey, "F11EscapeKey", XtRInt,
	DECwF11EscapeKey, f11EscapeKey,
	XtRImmediate, (caddr_t) DECwF11F11,
	NULL )
			/* F11 function key sends one of:
			      DECwF11F11, DECwF11Escape */
#endif

RESOURCE( DECwNangleBracketsKey, "AngleBracketsKey", XtRInt,
	DECwAngleBracketsKey, angleBracketsKey,
	XtRImmediate, (caddr_t) DECwAngleAngleBrackets,
	NULL )
			/* Angle Brackets key, one of:
			      DECwAngleAngleBrackets, DECwAngleOpenQuoteTilde */

RESOURCE( DECwNopenQuoteTildeKey, "OpenQuoteTildeKey", XtRInt,
	DECwOpenQuoteTildeKey, openQuoteTildeKey,
	XtRImmediate, (caddr_t) DECwTildeOpenQuoteTilde,
	NULL )
			/* OpenQuote/TildeKey, one of:
			      DECwTildeOpenQuoteTilde, DECwTildeEscape */

RESOURCE( DECwNkeyboardDialect, "KeyboardDialect", XtRInt,
	DECwKeyboardDialect, keyboardDialect,
	XtRImmediate, (caddr_t) DECwNorthAmericanDialect,
	s_set_value_keyboardDialect )
			/* Keyboard dialect (7-Bit NRCS), one of:
				DECwNorthAmericanDialect,
				DECwFlemishDialect,
				DECwCanadianFrenchDialect,
				DECwBritishDialect,
				DECwDanishDialect,
				DECwFinnishDialect,
				DECwAustrianGermanDialect,
				DECwDutchDialect,
				DECwItalianDialect,
				DECwSwissFrenchDialect,
				DECwSwissGermanDialect,
				DECwSwedishDialect,
				DECwNorwegianDialect,
				DECwBelgianFrenchDialect,
				DECwSpanishDialect,
				DECwPortugueseDialect
				*/

RESOURCE( DECwNmacrographReportEnable, "MacrographReportEnable", XtRBoolean,
	Boolean, macrographReportEnable,
	XtRImmediate, (caddr_t) FALSE,
	NULL )
			/* Allows ReGIS macrograph report command
			   [ R(M(...) ] to work */

C_RESOURCE( DECwNforeground, "Foreground", XtRPixel,
	Pixel, manager.foreground,
	XtRString, "XtDefaultForeground",
	o_set_value_foreground )
			/* default text foreground color */

C_RESOURCE( DECwNbackground, "Background", XtRPixel,
	Pixel, core.background_pixel,
	XtRString, "XtDefaultBackground",
	o_set_value_background )
			/* default text background color */

/**** reverseVideo must follow foreground & background!! ****/

RESOURCE( DECwNreverseVideo, "ReverseVideo", XtRBoolean,
	Boolean, reverseVideo,
	XtRImmediate, (caddr_t) FALSE,
	o_set_value_reverseVideo )
			/* reverseVideo (setting TRUE swaps fg & bg) */

RESOURCE( DECwNbatchScrollCount, "BatchScrollCount", XtRInt,
	int, batchScrollCount,
	XtRImmediate, (caddr_t) 0,
	s_set_value_batchScrollCount )
			/* number of lines to scroll at once */

RESOURCE( DECwNlocalEcho, "LocalEcho", XtRBoolean,
	Boolean, localEcho,
	XtRImmediate, (caddr_t) FALSE,
	s_set_value_localEcho )        
			/* local echo  */

RESOURCE( DECwNconcealAnswerback, "ConcealAnswerback", XtRBoolean,
	Boolean, concealAnswerback,
	XtRImmediate, (caddr_t) FALSE,
	s_set_value_concealAnswerback )        
			/* conceal answerback message */

RESOURCE( DECwNanswerbackMessage, "AnswerbackMessage", XtRString,
	String, answerbackMessage,
	XtRString, "",
	s_set_value_answerbackMessage )
			/* answerback message text */

RESOURCE( DECwNselectThreshold, "SelectThreshold", XtRInt,
	int, selectThreshold,
	XtRImmediate, (caddr_t) 5,
	NULL )
			/* number of pixels of movement needed for selection
			   to occur */

RESOURCE( DECwNdoubleClickDelay, "DoubleClickDelay", XtRInt,
	int, doubleClickDelay,
	XtRImmediate, (caddr_t) 250,
	NULL )
			/* number of ms to wait for double/triple click */

RESOURCE( DECwNwhiteSpaceCharacters, "WhiteSpaceCharacters", XtRString,
	String, whiteSpaceCharacters,
	XtRString, " \t\r\n",
	i_set_value_whiteSpace )
			/* break characters for word select */

RESOURCE( DECwNshareColormapEntries, "ShareColormapEntries", XtRBoolean,
	Boolean, shareColormapEntries,
	XtRImmediate, (caddr_t) FALSE,
	regis_set_value_shareColormap )
			/* TRUE means use virtual colormap */

RESOURCE( DECwNbitPlanes, "BitPlanes", XtRInt,
	int, bitPlanes,
	XtRImmediate, (caddr_t) 0,
	regis_set_value_bitPlanes )
			/* number of planes used for graphics */

RESOURCE( DECwNbackingStoreEnable, "BackingStoreEnable", XtRBoolean,
	Boolean, backingStoreEnable,
	XtRImmediate, (caddr_t) TRUE,
	regis_set_value_backingStore )
			/* TRUE means keep pixmap backing store for graphics */

RESOURCE( DECwNdefaultTitle, "", XtRString,
	String, defaultTitle,
	XtRString, (caddr_t) NULL,
	s_set_value_defaultTitle )
			/* default shell window title */

RESOURCE( DECwNdefaultIconName, "", XtRString,
	String, defaultIconName,
	XtRString, (caddr_t) NULL,
	s_set_value_defaultIconName )
			/* default shell icon name */

RESOURCE( DECwNsaveErasedLines, "SaveErasedLines", XtRBoolean,
	Boolean, saveErasedLines,
	XtRImmediate, (caddr_t) TRUE,
	NULL )		/* save erased lines in the transcript */

RESOURCE( DECwNsyncFrequency, "SyncFrequency", XtRInt,
	int, syncFrequency,
	XtRImmediate, (caddr_t) 10,
	NULL )
			/* number of lines scrolled before synchronizing */

#ifdef SECURE_KEYBOARD
RESOURCE( DECwNsecureKeyboard, "SecureKeyboard", XtRBoolean,
	Boolean, secureKeyboard,
	XtRImmediate, (caddr_t) FALSE,
	i_set_value_secureKeyboard )
			/* secureKeyboard (does XGrabKeyboard) */

RESOURCE( DECwNallowSendEvents, "AllowSendEvents", XtRBoolean,
	Boolean, allowSendEvents,
	XtRImmediate, (caddr_t) FALSE,
	NULL )
			/* allowSendEvents (to keyboard) */

RESOURCE( DECwNallowQuickCopy, "AllowQuickCopy", XtRBoolean,
	Boolean, allowQuickCopy,
	XtRImmediate, (caddr_t) TRUE,
	NULL )
			/* allowQuickCopy (to keyboard) */
#endif

RESOURCE( DECwNstartPrintingCallback, XtCCallback, XtRCallback,
	XtCallbackList, startPrintingCallback,
	XtRCallback, NULL,
	c_set_value_callbacks )
			/* callback called when printing starts */

RESOURCE( DECwNstopPrintingCallback, XtCCallback, XtRCallback,
	XtCallbackList, stopPrintingCallback,
	XtRCallback, NULL,
	c_set_value_callbacks )
			/* callback called when printing stops */

RESOURCE( DECwNstartPrinterToHostCallback, XtCCallback, XtRCallback,
	XtCallbackList, startPrinterToHostCallback,
	XtRCallback, NULL,
	c_set_value_callbacks )
			/* callback called when starting to read printer */

RESOURCE( DECwNstopPrinterToHostCallback, XtCCallback, XtRCallback,
	XtCallbackList, stopPrinterToHostCallback,
	XtRCallback, NULL,
	c_set_value_callbacks )
			/* callback called when stopping reading printer */

RESOURCE( DECwNprintLineCallback, XtCCallback, XtRCallback,
	XtCallbackList, printLineCallback,
	XtRCallback, NULL,
	c_set_value_callbacks )
			/* callback called for each line that is printed */

RESOURCE( DECwNprinterStatusCallback, XtCCallback, XtRCallback,
	XtCallbackList, printerStatusCallback,
	XtRCallback, NULL,
	c_set_value_callbacks )
			/* callback to request printerStatus to be updated */

RESOURCE( DECwNprinterStatus, "PrinterStatus", XtRInt,
	int, printerStatus,
	XtRImmediate, (caddr_t) 13,
	NULL )
			/* printer status, one of:
				10	printer ready
				11	printer not ready
				13	no printer */

RESOURCE( DECwNprinterPending, "PrinterPending", XtRInt,
	int, printerPending,
	XtRImmediate, (caddr_t) 0,
	NULL )		/* number of print requests pending per DECterm widget*/

RESOURCE( DECwNprintingDestination, "PrintingDestination",  XtRInt,
	DECwPrintingDestination, printingDestination,
	XtRImmediate, (caddr_t) DECwDestinationNone,
	NULL )
			/* printing destination, one of:
				DECwDestinationQueue, DECwDestinationPort,
				DECwDestinationFile, DECwDestinationNone */

RESOURCE( DECwNprinterPortName, "PrinterPortName", XtRString,
	String, printerPortName,
	XtRString, "",
	s_set_value_printerPortName )
			/* printer port device name */

RESOURCE( DECwNprinterFileName, "PrinterFileName", XtRString,
	String, printerFileName,
	XtRString, "",
	s_set_value_printerFileName )
			/* name of file for printer port output */

RESOURCE( DECwNprintMode, "PrintMode", XtRInt,
	DECwPrintMode, printMode,
	XtRImmediate, (caddr_t) DECwNormalPrintMode,
	s_set_value_printMode )
			/* print mode, one of:
				DECwNormalPrintMode, DECwPrintControllerMode,
				DECwAutoPrintMode */

RESOURCE( DECwNprintExtent, "PrintExtent", XtRInt,
	DECwPrintExtent, printExtent,
	XtRImmediate, (caddr_t) DECwFullPage,
	s_set_value_printExtent )
			/* print extent, one of:
				DECwFullPage, DECwFullPagePlusTranscript,
				DECwScrollRegionOnly, DECwSelectionOnly */

RESOURCE( DECwNprintFormat, "PrintFormat", XtRInt,
	DECwPrintFormat, printFormat,
	XtRImmediate, (caddr_t) DECwCompressedPrinting,
	NULL )
			/* print format, one of:
				DECwCompressedPrinting, DECwExpandedPrinting,
				DECwRotatedPrinting */

RESOURCE( DECwNprintDataType, "PrintDataType", XtRInt,
	DECwPrintDataType, printDataType,
	XtRImmediate, (caddr_t) DECwPrintAllCharacters,
	NULL )
			/* data type for text printing, one of:
				DECwNationalOnly, DECwNationalPlusLineDrawing,
				DECwPrintAllCharacters */

RESOURCE( DECwNprintFormFeedMode, "PrintFormFeedMode", XtRBoolean,
	Boolean, printFormFeedMode,
	XtRImmediate, (caddr_t) True,
	s_set_value_misc )
			/* equivalent to DECPFF */

RESOURCE( DECwNprinterToHostEnabled, "PrinterToHostEnabled", XtRBoolean,
	Boolean, printerToHostEnabled,
	XtRImmediate, (caddr_t) False,
	s_set_value_prt_to_host )
			/* allows printer reports to be sent to host */

RESOURCE( DECwNgraphicsPrintingEnabled, "GraphicsPrintingEnabled", XtRBoolean,
	Boolean, graphicsPrintingEnabled,
	XtRImmediate, (caddr_t) True,
	NULL )
			/* allows graphics (sixel) printing */

RESOURCE( DECwNprintBackgroundMode, "PrintBackgroundMode", XtRBoolean,
	Boolean, printBackgroundMode,
	XtRImmediate, (caddr_t) True,
	NULL )
			/* equivalent to DECGPBM */

RESOURCE( DECwNprintColorMode, "PrintColorMode", XtRBoolean,
	Boolean, printColorMode,
	XtRImmediate, (caddr_t) True,
	NULL )
			/* equivalent to DECGPCM */

RESOURCE( DECwNprint8BitMode, "Print8BitMode", XtRBoolean,
	Boolean, print8BitMode,
	XtRImmediate, (caddr_t) True,
	s_set_value_misc )

RESOURCE( DECwNprintHLSColorSyntax, "PrintHLSColorSyntax", XtRBoolean,
	Boolean, printHLSColorSyntax,
	XtRImmediate, (caddr_t) True,
	NULL )
			/* equivalent to DECGPCS */

RESOURCE( DECwNprintSixelLevel, "PrintSixelLevel", XtRInt,
	DECwSixelLevel, printSixelLevel,
	XtRImmediate, (caddr_t) DECwSixelLevel_2,
	NULL )
			/* controls the format of sixel output */

RESOURCE( DECwNsendBreakCallback, XtCCallback, XtRCallback,
	XtCallbackList, sendBreakCallback,
	XtRCallback, NULL,
	c_set_value_callbacks )
			/* callback called when user hits BREAK key */

RESOURCE( DECwNexitCallback, XtCCallback, XtRCallback,
	XtCallbackList, exitCallback,
	XtRCallback, NULL,
	c_set_value_callbacks )
			/* callback to exit the decterm */

RESOURCE( DECwNterminalType, "TerminalType", XtRInt,
	DECwTerminalType, terminalType,
	XtRImmediate, (caddr_t) DECwStandard,
     	s_set_value_terminalType )
			/* terminal type */

RESOURCE( DECwNfineFontSetName, "FineFontSetName", XtRString,
	String, fineFontSetName,
	XtRString, "",
     	o_set_value_fineFontSetName )
			/* current set of fonts */

RESOURCE( DECwNjisRomanAsciiMode, "JisRomanAsciiMode", XtRInt,
	DECwJisRomanAsciiMode, jisRomanAsciiMode,
	XtRImmediate, (caddr_t) DECwJisRomanMode,
	s_set_value_jisRomanAsciiMode )
			/* JIS-Roman/Ascii character sets to G0 */

RESOURCE( DECwNkanjiKatakanaMode, "KanjiKatakanaMode", XtRInt,
	DECwKanjiKatakanaMode, kanjiKatakanaMode,
	XtRImmediate, (caddr_t) DECwKanjiMode,
	s_set_value_kanjiKatakanaMode )
			/* Kanji/Katakana terminal emulation mode */

RESOURCE( DECwNregisScreenMode, "RegisScreenMode", XtRBoolean,
	Boolean, regisScreenMode,
	XtRImmediate, (caddr_t) FALSE,
	regis_set_value_regisScreenMode )
			/* regis screen mode (keep VT286/4's aspect ratio) */

RESOURCE( DECwNcontrolRepresentationMode, "ControlRepresentationMode",
	XtRBoolean,Boolean, controlRepresentationMode,
	XtRImmediate, (caddr_t) FALSE,
	s_set_value_crm )
			/* (DECCRM) Control Representation Mode */

RESOURCE( DECwNprintDisplayMode, "PrintDisplayMode", XtRInt,
	DECwDisplayMode, printDisplayMode,
	XtRImmediate, (caddr_t) DECwMainDisplay24,
	s_set_value_printDisplayMode )
			/* select to print ststus line */

RESOURCE( DECwNkanji_78_83, "Kanji_78_83", XtRInt,
	DECwKanji_78_83, kanji_78_83,
	XtRImmediate, (caddr_t) DECwKanji_83,
	s_set_value_kanji_78_83 )

RESOURCE( DECwNleadingCodeEnable, "LeadingCodeEnable", XtRBoolean,
	Boolean, leadingCodeEnable,
	XtRImmediate, (caddr_t) TRUE,
	s_set_value_misc )
			/* (DECLCSM) Leading-code mode */

RESOURCE( DECwNksRomanAsciiMode, "KsRomanAsciiMode", XtRInt,
	DECwKsRomanAsciiMode, ksRomanAsciiMode,
	XtRImmediate, (caddr_t) DECwKsRomanMode,
	s_set_value_ksRomanAsciiMode )
			/* KS-Roman/Ascii character sets to G0 */

RESOURCE( DECwNrightToLeft, "RightToLeft", XtRBoolean,
	Boolean, rightToLeft,
	XtRImmediate, (caddr_t) FALSE,
	s_set_value_rToL )

RESOURCE( DECwNredisplay7bit, "Redisplay7bit", XtRBoolean,
	Boolean, redisplay7bit,
	XtRImmediate, (caddr_t) FALSE,
	NULL )

RESOURCE( DECwNselectionRtoL, "SelectionRtoL", XtRBoolean,
	Boolean, selectionRtoL,
	XtRImmediate, (caddr_t) FALSE,
	s_set_value_copyDir )

RESOURCE( DECwNdisableXSME, "DisableXSME", XtRBoolean,
	Boolean, disableXSME,
	XtRImmediate, (caddr_t) FALSE,
	NULL )

RESOURCE( DECwNuseBoldFont, "UseBoldFont", XtRBoolean,
        Boolean, useBoldFont,
        XtRImmediate, (caddr_t) TRUE,
        o_set_value_useBoldFont )
                        /* use a special font for bold (FALSE means use
                           a different color) */
