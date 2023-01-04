/* DLFD.h - DLFD C definitions & enumerated types */


/*
    Source: HUMAN::FONT_ARCH$:DLFD.h
    Author: Jim FLOWERS, Font Architect, Corporate Font Program
    Create date: 11 Jan 1988


________________________________________________________________________
			-- REVISION HISTORY --
 Date		Author	Description of modifications
________________________________________________________________________
30 Jul 93	Eric Osman	- Merge vxt and vms sources
25 May 88	Tom Porcher	- Make it compile on Ultrix pcc.

25 Feb 88	JRF	- Removed REPRESENTATION from the font name enumerated
			type definition DLFD_EFNFIELD to conforms with V1.1 of
			DLFD spec
			- renamed MAX_FN_FIELDS to DLFD_MAX_FN_FIELDS
			- decremented DLFD_MAX_FN_FIELDS
26 Apr 88	JRF	- renamed "DLFD$" to "DLFD"
________________________________________________________________________
*/

/*************** these should go into separate file? */

typedef unsigned short SHORT;
typedef char * PBYTE;
typedef char * PSTR;

#define	max(a,b) ((a) > (b) ? (a) : (b))
#define	min(a,b) ((a) < (b) ? (a) : (b))

#ifndef NULL
#define NULL	(int) 0
#endif

#define CR	(char) 0x0A
#define HT	(char) 0x0B
#define FF	(char) 0x0C
#define LF	(char) 0x0D

/*************************************/

#define MAX_FONT_NAME_SIZE 80

#define DLFD_FN_DELIMITER (char) '-'	/* font name field delimiter */
#define DLFD_MAX_FN_FIELDS 14		/* number of fields in name */

typedef enum {				/* Font name string field ids */

    DLFD_REGISTRY 		= 0,	/* field position in string */
    DLFD_FOUNDRY		= 1,
    DLFD_FAMILY_NAME		= 2,
    DLFD_WEIGHT_NAME		= 3,
    DLFD_SLANT			= 4,
    DLFD_SETWIDTH_NAME		= 5,
    DLFD_ADD_STYLE_NAME		= 6,
    DLFD_PIXEL_SIZE		= 7,
    DLFD_POINT_SIZE		= 8,
    DLFD_RESOLUTION_X		= 9,
    DLFD_RESOLUTION_Y		= 10,
    DLFD_SPACING		= 11,
    DLFD_AVERAGE_WIDTH		= 12,
    DLFD_CHARSET_REGISTRY 	= 13,
    DLFD_CHARSET_ENCODING 	= 14

} DLFD_EFNFIELD;


typedef enum {				/* Slant field enumerated type defns */

    ROMAN,
    ITALIC,
    OBLIQUE,
    REVERSE_OBLIQUE,
    REVERSE_ITALIC

} DLFD_ESLANT;


typedef enum {				/* Spacing field enums type defns */

    PROPORTIONAL,
    MONOSPACED,
    CHAR_CELL

} DLFD_ESPACING;


typedef struct {			/* Font name data structure */

    PSTR	pstrFontNameRegistry;
    PSTR	pstrFoundry;
    PSTR	pstrFamilyName;
    PSTR	pstrWeightName;
    DLFD_ESLANT	eSlant;
    PSTR	pstrSetwidthName;
    PSTR	pstrAddStyleName;
    int		PixelSize;
    int		PointSize;
    int		ResolutionX;
    int		ResolutionY;
    DLFD_ESPACING eSpacing;
    int		AverageWidth;
    PSTR	pstrCharsetRegistry;
    PSTR	pstrCharsetEncoding;

} DLFD_FONTNAMESTRUCT;

/*

DIGITAL FONT PROPERTIES

The font properties will generally include the font name fields, plus other
useful global font information such as the height of capitals (CAP_HEIGHT),
calculated weight and setwidth (WEIGHT and SETWIDTH), etc. The DLFD spec
will specify a full list of possible DEC font properties, their
interpretation and fallbacks if certain properties are not defined for a
font. 

Refer to the Digital Logical Font Description (DLFD) for a complete list of font
properties, in HUMAN::FONT_ARCH$.

*/


/* DLFD routine definitions for type checking */


extern PSTR
/*	DLFDGetFontNameField(DLFD_EFNFIELD, PSTR, int *);	*/
	DLFDGetFontNameField();
extern PSTR
/*	DLFDSetFontNameField(DLFD_EFNFIELD, PSTR, int, PSTR);	*/
	DLFDSetFontNameField();

extern DLFD_FONTNAMESTRUCT *
/*	DLFDSetFontNameStruct(PSTR, DLFD_FONTNAMESTRUCT);	*/
	DLFDSetFontNameStruct();

extern PSTR
/*	DLFDSetFontNameString(PSTR, DLFD_FONTNAMESTRUCT);	*/
	DLFDSetFontNameString();


/* end of "DLFD.h" */
