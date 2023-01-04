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
**	Internationalization RTL (I18n RTL)
**
**  ABSTRACT:
**
**	Internationalization Services header file for DECwindows 
**
**
**  MODIFICATION HISTORY:
**
**	
**
**--
**/

#ifndef I18n_RTL
#define I18n_RTL
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include "I18nConverter.h"

/*
 * definitions and data structure needed for the font fallback routine
 */

#define I18NTKMAJOR         3
#define I18NTKMINOR         0
#define I18NXFLDMAJOR       1
#define I18NXFLDMINOR       2

#define NUMOFI18NFIELDS		14
#define I18NFOUNDRY 		0
#define I18NFAMILY		1
#define I18NWEIGHT		2
#define I18NSLANT		3
#define I18NSETWIDTH		4
#define I18NADDSTYLE		5
#define I18NPIXELSIZE		6
#define I18NPOINTSIZE		7
#define I18NXRES		8
#define I18NYRES		9
#define I18NSPACING		10
#define I18NAVGWIDTH		11
#define I18NCHARSET		12
#define I18NENCODING		13

typedef void (*VoidProc) ();

typedef struct _I18NFontName {
	short	XFLDmajor;
	short	XFLDminor;
	short	TKmajor;
	short	TKminor;
	char	*fields[NUMOFI18NFIELDS];
} I18NFontNameRec, *I18NFontNamePtr;


/*
 * I18N layer context block
 */

typedef struct _I18nContextRec {
	char * locale;
	Boolean use_mrm_hierarchy;
	Opaque mrm_hierarchy_id;
	Opaque widget_class;
} I18nContextRec;

/*
 *  I18n fontlist record structure
 */

typedef struct _I18nFontListRec {
	XFontStruct	*font;
	char		*character_set;
} I18nFontListRec;

/*
 * I18n Xlib Buffer record structure
 */

typedef struct _I18nXlibBufferRec {
	Position	x;		/* not used by I18n */
	Dimension 	width;		/* not used by I18n */
	short		font_index;
	Opaque		text;
	short		char_count;
} I18nXlibBufferRec;

#include <X11/DECwI18n.h>

/*
 * forward declarations of public entry points
 */

#ifdef _NO_PROTO

extern Boolean I18nRemapFontname ( );
extern Boolean I18nGetLocaleString ( );
extern unsigned long I18nGetLocaleMnemonic ( );
extern char *I18nGetLocaleCharset ( );
extern char **I18nGetLocaleCharsets ( );
extern Boolean I18nParseFC ( );
extern Boolean I18nConstructFC ( );
extern Boolean I18nBeginConstructFC ( );
extern Opaque I18nEndConstructFC ( );
extern void I18nSegmentMeasure ( );
extern void I18nSegmentLength ( );
extern void I18nCreateXlibBuffers ( );
extern I18nContext I18nGetGlobalContext ( );
extern char *I18nCreateDefaultFontList ( );
extern void I18nMapSegment ( );
extern Boolean I18nHasSubstring ( );
extern void I18nSubStringPosition ( );
extern Boolean I18nIsWhiteSpace ( );
extern Boolean I18nIsScanBreak ( );
extern void I18nCSConstructInit ( );
extern void I18nCSConstructEnd ( );
extern void I18nCSConstructSegment ( );
extern void I18nCSConstructLine ( );
extern void I18nCvtCStoFCInit ( );
extern void I18nCvtCStoFCEnd ( );
extern void I18nCvtCStoFCSegment ( );
extern void I18nCvtCStoFCLine ( );
extern void I18nCvtCStoOSInit ( );
extern void I18nCvtCStoOSEnd ( );
extern void I18nCvtCStoOSSegment ( );
extern void I18nCvtCStoOSLine ( );
extern void I18nCvtFCtoCS ( );
extern void I18nCvtOStoCS ( );
extern void I18nLoadShareable ( );

#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern Boolean I18nRemapFontname ( I18NFontNamePtr old , I18NFontNamePtr new );
extern Boolean I18nGetLocaleString ( char *context , I18nCvtContext cvtcontext , char *ascii, I18nWordType word_type );
extern unsigned long I18nGetLocaleMnemonic ( I18nContext context , char *widget_class , char *mnemonic , char *charset , char **returned_mnemonic );
extern char *I18nGetLocaleCharset ( char *context );
extern char **I18nGetLocaleCharsets ( char *context );
extern Boolean I18nParseFC ( char *fc , char **text , char **charset , unsigned char *direction , long *byte_count , long *status );
extern Boolean I18nConstructFC ( char **local_context , char *text , char *charset , unsigned char direction , char separator , long *byte_count , long *status );
extern Boolean I18nBeginConstructFC ( char **local_context );
extern Opaque I18nEndConstructFC ( char **local_context , long *byte_count , long *status );
extern void I18nSegmentMeasure ( char *context , char *charset , char *text , int character_offset , int number_characters , int *byte_offset_start , int *byte_offset_end );
extern void I18nSegmentLength ( Opaque context , char *charset , Opaque text , int byte_length , int *char_length );
extern void I18nCreateXlibBuffers ( I18nContext context , I18nFontList *font_list , char *charset , 
int direction , Opaque text , short character_count , 
int first_char_position , VoidProc need_more_callback , 
Opaque need_more_context , VoidProc no_font_callback , 
Opaque no_font_context , I18nXlibBuffers *buffers , short *buffer_count );
extern I18nContext I18nGetGlobalContext ( void );
extern char *I18nCreateDefaultFontList ( I18nContext context , char *widget_class_name , char *resource_name );
extern void I18nMapSegment ( I18nContext context , char *input_charset , Opaque input_text , short input_char_count , short input_byte_count , char **output_charset , Opaque *output_text , short *output_char_count , short *output_byte_count );
extern Boolean I18nHasSubstring ( I18nContext context , char *character_set , Opaque *string , Opaque *sub_string , int string_char_count , int sub_char_count );
extern void I18nSubStringPosition ( I18nContext context , XFontStruct *font , char *character_set , Opaque *string , Opaque *sub_string , int string_char_count , int sub_char_count , Position x , Dimension *under_begin , Dimension *under_end );
extern Boolean I18nIsWhiteSpace ( I18nContext context , char *this_charset , Opaque *this_char );
extern Boolean I18nIsScanBreak ( I18nContext context , char *prev_charset , Opaque *prev_char , char *this_charset , Opaque *this_char , char *next_charset , Opaque *next_char , int scan_direction , I18nScanType scan_type );
extern void I18nCSConstructInit ( I18nCvtContext context );
extern void I18nCSConstructEnd ( I18nCvtContext context );
extern void I18nCSConstructSegment ( I18nCvtContext context );
extern void I18nCSConstructLine ( I18nCvtContext context );
extern void I18nCvtCStoFCInit ( I18nCvtContext context );
extern void I18nCvtCStoFCEnd ( I18nCvtContext context );
extern void I18nCvtCStoFCSegment ( I18nCvtContext context );
extern void I18nCvtCStoFCLine ( I18nCvtContext context );
extern void I18nCvtCStoOSInit ( I18nCvtContext context );
extern void I18nCvtCStoOSEnd ( I18nCvtContext context );
extern void I18nCvtCStoOSSegment ( I18nCvtContext context );
extern void I18nCvtCStoOSLine ( I18nCvtContext context );
extern void I18nCvtFCtoCS ( I18nCvtContext context , int *status );
extern void I18nCvtOStoCS ( I18nCvtContext context , int *status );
extern void I18nLoadShareable ( void );

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */


#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* I18n_RTL */
