/*
 *  Title:	DECterm.h - DECterm Widget public declarations
 *
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © Digital Equipment Corporation, 1988 - 1993 All Rights      |
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
 *	Definitions required by applications to use DECterm widget.
 *
 *  Author:	Tom Porcher
 *
 *  Modification history:
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
 *  Alfred von Campe    25-Mar-1993     V1.2/BL2
 *      - OSF/1 code merge.
 *      - Add F1-F5 key support.
 *
 *  Aston Chan		12-Mar-1993	V1.2/BL2
 *	- Add Turkish/Greek support.
 *
 *  Aston Chan		20-Aug-1992	Post V1.1
 *	- ToLung's fix to make terminal type switchable on DECterm widget
 *	  based on xnllanguage in Japanese TPU.
 *
 *  Alfred von Campe    20-Feb-1992     V3.1
 *      - Add color text support.
 *      - Remove #ifdefs since this is a "public" header file.
 *
 *  Aston Chan		19-Dec-1991	V3.1
 *	- I18n code merge.
 *
 *  Alfred von Campe	13-Nov-1991     V3.1
 *	- Added autoAdjustPosition resource.
 *
 *  Alfred von Campe	06-Oct-1991     Hercules/1 T0.7
 *	- Added F11 key feature from ULTRIX.
 *
 *  Michele Lien    11-Dec-1990	VXT X0.0 BL2
 *  - VXT can only direct printer output to a dedicated printer port, "printer
 *    destination" option in printer customize menu for DECterm V3.0 has been 
 *    taken out.  All the resources associated with printer destination and 
 *    port name can only be conditionally compiled for non-VXT systems.  
 *    Printer destination and printer port name has been hard coded to the 
 *    local printer port.
 *
 *  Bob Messenger	15-Sep-1990	X3.0-7
 *	- Add support for GS font.
 *
 *  Bob Messenger	15-Sep-1990	X3.0-7
 *	- Add support for GS font.
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 *  Bob Messenger	17-Jul-1990	X3.0-5
 *	Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- definitions of Asian terminal IDs
 *	- terminal type resource
 *	- resources specific to Kanji terminal (Tomcat)
 *
 *  Bob Messenger	23-Jun-1990	X3.0-5
 *	- Define resources needed for printer port support.
 *
 *  Mark Woodbury	12-March-1990	X3.0-3M
 *	- Motif conversion: take out decw$include logical.  they are using
 *    c$include to point to the correct directories.  This may change again.
 *
 *  Bob Messenger	 2-May-1990	X3.0-2
 *	- Change DECW$K_MSG_EXIT to DECW$K_MSG_EXIT_NO_FONT, so applications
 *	  using the DECterm widget don't have to link with the DECterm
 *	  message file.
 *
 *  Bob Messenger	24-Apr-1990	X3.0-2
 *	- Add a new internal error code: DECW$K_MSG_EXIT.
 *
 *  Bob Messenger	27-Feb-1990	V2.1
 *	- Support secure keyboard in UWS V4.0.
 *
 *  Bob Messenger	16-May-1989	X2.0-19
 *	- Added DECwNsyncFrequency.
 *
 *  Bob Messenger	14-May-1989	X2.0-10
 *	- Add definitions needed for errorCallback.
 *
 *  Bob Messenger	22-Apr-1989	X2.0-7
 *	- Correct capitalization of Intrinsic.h on Ultrix.
 *
 *  Bob Messenger	11-Apr-1989	X2.0-6
 *	- Added DECwNsaveErasedLines.
 *
 *  Bob Messenger	 8-Apr-1989	X2.0-6
 *	- Choose better names for keyboard remapping resource values.
 *	- Add helpCallback.
 *
 *  Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Convert to new widget bindings (DECw instead of Dwt).
 *	- Add definitions for all resource names.
 *
 *  Bob Messenger	 1-Apr-1989	X2.0-5
 *	- Add fields to support shellValuesCallback.
 *
 *  Bob Messenger	15-Mar-1989	X2.0-3
 *	- Added VT330_id.
 *
 *  Peter Sichel        12-Apr-1988     X0.4-8
 *      - corrected DECterm specific type names to be more meaningful
 *
 *  <modifier's name>	<date>		<ident of revised code>
 *	<description of change and purpose of change>
 *
 */

#ifndef _DECterm_h
#define _DECterm_h

/*
 * If SECURE_KEYBOARD is defined, DECterm will support these security features
 * (required on ULTRIX because TCP/IP doesn't allow per-user authorization):
 * secureKeyboard (prevents other programs from grabbing the keyboard),
 * allowSendEvent (lets user prevent other programs from sending fake events),
 * allowQuickCopy (lets user disable the QuickCopy feature, which lets other
 * programs stuff data into the input buffer).
 */

#if !defined(VMS_DECTERM) && !defined(VXT_DECTERM)
#define SECURE_KEYBOARD
#include <X11/Intrinsic.h>
#else
#include "Intrinsic.h"
#endif

/*
 * Definitions of names specific to DECterm widget
 */

#define DECwNinputCallback		"inputCallback"
#define DECwNstopOutputCallback		"stopOutputCallback"
#define DECwNstartOutputCallback	"startOutputCallback"
#define DECwNresizeCallback		"resizeCallback"
#define DECwNshellValuesCallback	"shellValuesCallback"
#define DECwNhelpCallback		XmNhelpCallback
#define DECwNerrorCallback		"errorCallback"
#define DECwNmaxInput			"maxInput"
#define DECwNcolumns			"columns"
#define DECwNrows			"rows"
#define DECwNdisplayWidth		"displayWidth"
#define DECwNdisplayHeight		"displayHeight"
#define DECwNdisplayWidthInc		"displayWidthInc"
#define DECwNdisplayHeightInc		"displayHeightInc"
#define DECwNscrollHorizontal		"scrollHorizontal"
#define DECwNscrollVertical		"scrollVertical"
#define DECwNfontSetSelection		"fontSetSelection"
#define DECwNbigFontSetName		"bigFontSetName"
#define DECwNlittleFontSetName		"littleFontSetName"
#define DECwNgsFontSetName		"gsFontSetName"
#define DECwNfontUsed			"fontUsed"
#define DECwNbigFontSetSelection	"bigFontSetSelection"
#define DECwNbigFontOtherName		"bigFontOtherName"
#define DECwNlittleFontSetSelection	"littleFontSetSelection"
#define DECwNlittleFontOtherName	"littleFontOtherName"
#define DECwNgsFontSetSelection		"gsFontSetSelection"
#define DECwNgsFontOtherName		"gsFontOtherName"
#define DECwNcursorStyle		"cursorStyle"
#define DECwNtextCursorEnable		"textCursorEnable"
#define DECwNcouplingHorizontal		"couplingHorizontal"
#define DECwNcouplingVertical		"couplingVertical"
#define DECwNautoResizeTerminal		"autoResizeTerminal"
#define DECwNautoResizeWindow		"autoResizeWindow"
#define DECwNterminalDriverResize	"terminalDriverResize"
#define DECwNautoAdjustPosition         "autoAdjustPosition"
#define DECwNcondensedFont		"condensedFont"
#define DECwNadjustFontSizes		"adjustFontSizes"
#define DECwNstatusDisplayEnable	"statusDisplayEnable"
#define DECwNlockUDK			"lockUDK"
#define DECwNlockUserFeatures		"lockUserFeatures"
#define DECwNuserPreferenceSet		"userPreferenceSet"
#define DECwNterminalMode		"terminalMode"
#define DECwNresponseDA			"responseDA"
#define DECwNeightBitCharacters		"eightBitCharacters"
#define DECwNmarginBellEnable		"marginBellEnable"
#define DECwNwarningBellEnable		"warningBellEnable"
#define DECwNcontrolQSHold		"controlQSHold"
#define DECwNsaveLinesOffTop		"saveLinesOffTop"
#define DECwNtranscriptSize		"transcriptSize"
#define DECwNnewLineMode		"newLineMode"
#define DECwNscreenMode			"screenMode"
#define DECwNautoWrapEnable		"autoWrapEnable"
#define DECwNcursorBlinkEnable		"cursorBlinkEnable"
#define DECwNautoRepeatEnable		"autoRepeatEnable"
#define DECwNapplicationKeypadMode	"applicationKeypadMode"
#define DECwNapplicationCursorKeyMode	"applicationCursorKeyMode"
#define DECwNbackarrowKey		"backarrowKey"
#define DECwNperiodCommaKeys		"periodCommaKeys"
#if !defined(VXT_DECTERM)
#define DECwNf11EscapeKey		"f11EscapeKey"
#endif
#define DECwNangleBracketsKey		"angleBracketsKey"
#define DECwNopenQuoteTildeKey		"openQuoteTildeKey"
#define DECwNkeyboardDialect		"keyboardDialect"
#define DECwNmacrographReportEnable	"macrographReportEnable"
#define DECwNforeground			"foreground"
#define DECwNbackground			"background"
#define DECwNreverseVideo		"reverseVideo"
#define DECwNbatchScrollCount		"batchScrollCount"
#define DECwNlocalEcho			"localEcho"
#define DECwNconcealAnswerback		"concealAnswerback"
#define DECwNanswerbackMessage		"answerbackMessage"
#define DECwNselectThreshold		"selectThreshold"
#define DECwNdoubleClickDelay		"doubleClickDelay"
#define DECwNwhiteSpaceCharacters	"whiteSpaceCharacters"
#define DECwNshareColormapEntries	"shareColormapEntries"
#define DECwNbitPlanes			"bitPlanes"
#define DECwNbackingStoreEnable		"backingStoreEnable"
#define DECwNdefaultTitle		"defaultTitle"
#define DECwNdefaultIconName		"defaultIconName"
#define DECwNsaveErasedLines		"saveErasedLines"
#define DECwNsyncFrequency		"syncFrequency"
#ifdef SECURE_KEYBOARD
#define DECwNsecureKeyboard		"secureKeyboard"
#define DECwNallowSendEvents		"allowSendEvents"
#define DECwNallowQuickCopy		"allowQuickCopy"
#endif
#define DECwNstartPrintingCallback	"startPrintingCallback"
#define DECwNstopPrintingCallback	"stopPrintingCallback"
#define DECwNstartPrinterToHostCallback	"startPrinterToHost"
#define DECwNstopPrinterToHostCallback	"stopPrinterToHost"
#define DECwNprintLineCallback		"printLineCallback"
#define DECwNprinterStatusCallback	"printerStatusCallback"
#define DECwNprinterStatus		"printerStatus"
#define DECwNprinterPending		"printerPending"
#define DECwNprintingDestination	"printingDestination"
#define DECwNprinterPortName		"printerPortName"
#define DECwNprinterFileName		"printerFileName"
#define DECwNprintMode			"printMode"
#define DECwNprintExtent		"printExtent"
#define DECwNprintFormat		"printFormat"
#define DECwNsendBreakCallback		"sendBreakCallback"
#define DECwNexitCallback		"exitCallback"
#define DECwNprintDataType		"printDataType"
#define DECwNprintFormFeedMode		"printFormFeedMode"
#define DECwNprinterToHostEnabled	"printerToHostEnabled"
#define DECwNgraphicsPrintingEnabled	"graphicsPrintingEnabled"
#define DECwNprintBackgroundMode	"printBackgroundMode"
#define DECwNprintColorMode		"printColorMode"
#define DECwNprint8BitMode		"print8BitMode"
#define DECwNprintHLSColorSyntax	"printHLSColorSyntax"
#define DECwNprintSixelLevel		"printSixelLevel"
#define DECwNterminalType		"terminalType"
#define DECwNfineFontSetName		"fineFontSetName"
#define DECwNjisRomanAsciiMode		"jisRomanAsciiMode"
#define DECwNkanjiKatakanaMode		"kanjiKatakanaMode"
#define DECwNregisScreenMode		"regisScreenMode"
#define DECwNkanji_78_83                "kanji_78_83"
#define DECwNcontrolRepresentationMode  "CRM"
#define DECwNprintDisplayMode		"printDisplayMode"
#define DECwNleadingCodeEnable		"leadingCodeEnable"
#define DECwNksRomanAsciiMode		"ksRomanAsciiMode"

#define DECwNrightToLeft		"rightToLeft"
#define DECwNredisplay7bit		"redisplay7bit"
#define DECwNselectionRtoL		"selectionRtoL"
#define DECwNdisableXSME		"disableXSME"

#define DECwNuseBoldFont                "useBoldFont"

/*
 * Callback reasons
 */

#define DECwCRInput		100001
#define DECwCRStopOutput	100002
#define DECwCRStartOutput	100003
#define DECwCRResize		100004
#define DECwCRShellValues	100005
#define DECwCRError		100006
#define DECwCRStartPrintScreen	100007
#define DECwCRStopPrintScreen	100008
#define DECwCREnterAutoPrintMode		100009
#define DECwCRExitAutoPrintMode			100010
#define DECwCREnterPrinterControllerMode	100011
#define DECwCRExitPrinterControllerMode		100012
#define DECwCRPrintLine				100013
#define DECwCRPrinterStatus			100014

/*
 * Callback structures
 */

typedef struct {	/* DECwTermInputCallbackStruct */
    int		reason;	/* reason code */
    char	*data;	/* pointer to data */
    int		count;	/* count of characters; this may be reduced by
			   callback proc if not all characters accepted */
} DECwTermInputCallbackStruct;

typedef struct {	/* DECwTermArgCallbackStruct */
    int		reason;		/* reason code */
    Arg		*arglist;	/* list of arguments to change */
    int		num_args;	/* number of arguments in arglist */

} DECwTermArgCallbackStruct;

typedef struct {	/* DECwTermErrorCallbackStruct */
    int		reason;	/* reason code */
    int		code;	/* DECterm-specific error code */
    int		status;	/* system error status code */
    char	*text;	/* pointer to null-terminated data */
} DECwTermErrorCallbackStruct;

/*
 * DECterm-specific error codes
 */

#define DECW$K_MSG_INFORMATIONAL	0
#define DECW$K_MSG_SYSTEM_ERROR		1
#define DECW$K_MSG_CANT_FIND_FONT	2
#define DECW$K_MSG_EXIT_NO_FONT		3
#define DECW$K_MSG_NO_INPUT_METHOD	4

#define DECTERM$K_ISO_LATIN1	"ISO8859-1"
#define DECTERM$K_ISO_HEBREW	"ISO8859-8"
#define DECTERM$K_DEC_TECH	"DEC-TECH"
/*
 * DECterm-specific types
 */

typedef enum {		/* DECwCursorStyle */
    DECwCursorBlock, DECwCursorUnderline
    } DECwCursorStyle;

typedef enum {		/* DECwTerminalMode */
    DECwVT52_Mode, DECwVT100_Mode, DECwVT300_7_BitMode, DECwVT300_8_BitMode
    } DECwTerminalMode;
               
typedef enum {		/* DECwResponseDA */
    DECwVT100_ID, DECwVT101_ID, DECwVT102_ID, DECwVT125_ID,
    DECwVT220_ID, DECwVT240_ID, DECwVT320_ID, DECwVT340_ID,
    DECwDECtermID, DECwVT330_ID,
    DECwVT80_ID, DECwVT100J_ID, DECwVT102J_ID, DECwVT220J_ID, 
    DECwVT282_ID, DECwVT284_ID, DECwVT286_ID, DECwVT382_ID,
    DECwVT382CB_ID, DECwVT382K_ID, DECwVT382D_ID 
    } DECwResponseDA;

typedef enum {		/* DECwUserPreferenceSet */
    DECwDEC_Supplemental, DECwISO_Latin1_Supplemental,
    DECwDEC_Hebrew_Supplemental, DECwISO_Latin8_Supplemental,
    DECwDEC_Turkish_Supplemental, DECwISO_Latin5_Supplemental,
    DECwDEC_Greek_Supplemental, DECwISO_Latin7_Supplemental,
    } DECwUserPreferenceSet;

typedef enum {		/* DECwBackarrowKey */
    DECwBackarrowBackspace,	/* delete key sends backspace */
    DECwBackarrowDelete		/* delete key sends delete */
    } DECwBackarrowKey;

typedef enum {	  	/* DECwPeriodCommaKeys */
    DECwCommaPeriodComma,	/* shifted "," and "." send "," and "." */
    DECwCommaAngleBrackets	/* shifted "," and "." send "<" and ">" */
    } DECwPeriodCommaKeys;

#if !defined(VXT_DECTERM)
typedef enum {   	/* DECwF11EscapeKey */
    DECwF11F11,			/* F11 function key sends F11 keycode */
    DECwF11Escape		/* F11 function key sends escape */
    } DECwF11EscapeKey;
#endif

typedef enum {	  	/* DECwAngleBracketsKey */
    DECwAngleAngleBrackets,	/* "<>" key sends "<" unshifted, ">" shifted */
    DECwAngleOpenQuoteTilde	/* "<>" key sends "`" unshifted, "~" shifted */
    } DECwAngleBracketsKey;

typedef enum {		/* DECwOpenQuoteTildeKey */
    DECwTildeOpenQuoteTilde,	/* "`~" key sends "`" unshifted, "~" shifted */
    DECwTildeEscape		/* "`~" key sends escape */
    } DECwOpenQuoteTildeKey;

typedef enum {		/* DECwKeyboardDialect */
    DECwNorthAmericanDialect,	DECwFlemishDialect,
    DECwCanadianFrenchDialect,	DECwBritishDialect,
    DECwDanishDialect,		DECwFinnishDialect,
    DECwAustrianGermanDialect,	DECwDutchDialect,
    DECwItalianDialect,		DECwSwissFrenchDialect,
    DECwSwissGermanDialect,	DECwSwedishDialect,
    DECwNorwegianDialect,	DECwBelgianFrenchDialect,
    DECwSpanishDialect,		DECwPortugueseDialect,
    DECwHebrewDialect,		DECwTurkishDialect,
    DECwGreekDialect
    } DECwKeyboardDialect;

typedef enum {		/* DECwFontSetSelection */
    DECwBigFont, DECwLittleFont, DECwGSFont, DECwFineFont
    } DECwFontSetSelection;

/*  - VXT can only direct printer output to a dedicated printer port, "printer
 *    destination" option in printer customize menu for DECterm V3.0 has been 
 *    taken out.  All the resources associated with printer destination and 
 *    port name can only be conditionally compiled for non-VXT systems.  
 *    Printer destination and printer port name has been hard coded to the 
 *    local printer port.
 */
typedef enum {		/* DECwPrintingDestination */
    DECwDestinationQueue,	DECwDestinationPort,
    DECwDestinationFile,	DECwDestinationNone
    } DECwPrintingDestination;

typedef enum {		/* DECwPrintMode */
    DECwNormalPrintMode,	DECwPrintControllerMode,
    DECwAutoPrintMode
    } DECwPrintMode;

typedef enum {		/* DECwPrintExtent */
    DECwFullPage,		DECwFullPagePlusTranscript,
    DECwScrollRegionOnly,	DECwSelectionOnly
    } DECwPrintExtent;

typedef enum {		/* DECwPrintFormat */
    DECwCompressedPrinting,	DECwExpandedPrinting,
    DECwRotatedPrinting
    } DECwPrintFormat;

typedef enum {		/* DECwPrintDataType */
    DECwNationalOnly,		DECwNationalPlusLineDrawing,
    DECwPrintAllCharacters
    } DECwPrintDataType;

typedef enum {		/* DECwSixelLevel */
    DECwSixelLevel_1,		DECwSixelLevel_2,
    DECwSixelLevel_LA210
    } DECwSixelLevel;

/* values of printerStatus */

#define DECwPrinterReady	10
#define DECwPrinterNotReady	11
#define DECwNoPrinter		13

typedef enum {		/* DECwTerminalType */
    DECwStandard, DECwKanji, DECwHanzi, DECwHangul, DECwHanyu, DECwHebrew,
    DECwMulti=99, DECwStandard2, DECwGreek, DECwTurkish
    } DECwTerminalType;	/* read only in case of non-DECwMulti */

typedef enum {		/* DECwJisRomanAsciiMode */
    DECwJisRomanMode, DECwAsciiMode
    } DECwJisRomanAsciiMode;

typedef enum {		/* DECwKanjiKatakanaMode */
    DECwKanjiMode, DECwKatakanaMode
    } DECwKanjiKatakanaMode;

typedef enum {		/* DECwDisplayMode */
    DECwStatusDisplay25, DECwMainDisplay24
    } DECwDisplayMode;

typedef enum {          /* DECwKanji_78_83 */
    DECwKanji_78, DECwKanji_83
    } DECwKanji_78_83;

typedef enum {		/* DECwKsRomanAsciiMode */
    DECwKsRomanMode, DECwKsAsciiMode
    } DECwKsRomanAsciiMode;

typedef enum {		/* DECwFunctionKeyMode */
    DECwFactoryDefault, DECwLocalFunction,
    DECwSendKeySequence, DECwDisableKey
    } DECwFunctionKeyMode;

/* define some macro to make use of TerminalType */
#define IsAsianTerminalType(t)    ((t) == DECwKanji  || (t) == DECwHanzi || \
				   (t) == DECwHangul || (t) == DECwHanyu)
#define IsAsianOrHebrewTermType(t) (IsAsianTerminalType(t) || (t) == DECwHebrew)

#endif _DECterm_h
/* End of DECterm.h */
