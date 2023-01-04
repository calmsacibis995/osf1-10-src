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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_FONT.C*/
/* *19    5-MAR-1993 14:31:21 GOSSELIN "fixing default font problem"*/
/* *18    8-JUN-1992 19:11:58 BALLENGER "UCX$CONVERT"*/
/* *17    8-JUN-1992 12:59:43 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *16    2-APR-1992 18:17:51 ROSE "Fixed bugs in returning of default font and improperly cased styles and weights"*/
/* *15    1-APR-1992 14:19:56 KLUM "default for un-matched font"*/
/* *14   31-MAR-1992 16:24:51 ROSE "Return final form font string that gets passed to Postscript interpreter"*/
/* *13   30-MAR-1992 20:29:26 BALLENGER "Fix FONT_NOT_FOUND deallocation problem"*/
/* *12   30-MAR-1992 16:09:54 BALLENGER "run ucx$convert"*/
/* *11   30-MAR-1992 16:07:11 BALLENGER "Fix function prototypes for local routines."*/
/* *10   28-MAR-1992 17:24:11 BALLENGER "Add font support for converted postscript"*/
/* *9    25-MAR-1992 14:08:18 ROSE "Added font mapping"*/
/* *8    20-MAR-1992 15:49:50 BALLENGER "Fix I18N support"*/
/* *7    14-MAR-1992 14:18:16 BALLENGER "Fine tune spacing for topic data."*/
/* *6     8-MAR-1992 19:16:10 BALLENGER "  Add topic data and text line support"*/
/* *5     3-MAR-1992 16:59:06 KARDON "UCXed"*/
/* *4    12-FEB-1992 12:02:43 PARMENTER "Adding i18n support for two byte fonts"*/
/* *3     1-NOV-1991 13:05:06 BALLENGER "reintegrate  memex support"*/
/* *2    17-SEP-1991 20:03:56 BALLENGER "include function prototype headers and change use of major and minor in version number to*/
/*avoid conflict with sys/types.h definition on ULTRIX"*/
/* *1    16-SEP-1991 12:39:23 PARMENTER "Fonts"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_FONT.C*/
#ifndef VMS
 /*
#else
#module BKR_FONT "V03-0001"
#endif
#ifndef VMS
 */
#endif

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
**      Bookreader User Interface (bkr)
**
**  ABSTRACT:
**
**	Font initialization and handling routines.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     29-Jan-1990
**
**  MODIFICATION HISTORY:
**
**	V01-0001    Tom Rose				25-Mar-1992
**  	    	    Add code to map X font string to Adobe font string.
**
**	V03-0001    JAF0001	James A. Ferguson   	29-Jan-1990
**  	    	    Extracted V2 routines and created new module.
**
**--
**/


/*****************
 * INCLUDE FILES *
 *****************/
#include    <ctype.h>	    	    /* for upper_case() */
#include    <string.h>
#include    <X11/Xatom.h>
#include    "br_common_defs.h"  /* common BR #defines */
#include    "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include    "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include    "br_globals.h"      /* BR external variables declared here */
#include    "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include    "bkr_font.h"        /* function prototypes for .c module */
#include    "bkr_error.h"       /* error routines */
#include    "bkr_fetch.h"       /* resource fetching routines */


/*****************************************************
 *  DEFINES AND FORWARD REFERENCES FOR FONT MAPPING  *
 *****************************************************/
#define UPCASE(str) { int i, len; \
		for(i=0, len=strlen(str);i<len;str[i]=toupper(str[i]),i++); }
#ifdef vms
#define SETCASE(str) { int i, len; \
		for(i=0, len=strlen(str);i<len;str[i]=toupper(str[i]),i++); }
#define ISOLATIN1 "_ISOLATIN1"
#define ISOLATIN2 "_ISOLATIN2"
#define ISOLATINARABIC "_ISOLATINARABIC"
#define ISOLATINGREEK "_ISOLATINGREEK"
#define ISOLATINHEBREW "_ISOLATINHEBREW"
#define ISO8859_REGISTRY "ISO8859"
#define DEC_REGISTRY "DEC"
#define	DECMATHSYMBOL	"_DECMATH_SYMBOL"
#define	DECMATHITALIC	"_DECMATH_ITALIC"
#define	DECMATHEXTENSION "_DECMATH_EXTENSION"
#define	DECTECH		"_DECTECH"
#define	DECPUBLISHING	"_DECPUBLISHING"
#define DECMATHSYM_ENC	"DECMATH_SYMBOL"
#define DECMATHITA_ENC	"DECMATH_ITALIC"
#define DECMATHEXT_ENC	"DECMATH_EXTENSION"
#define DECTECH_ENC 	"DECTECH"
#define DECPUB_ENC	"DECPUBLISH"
#define ISO8859_1_ENC	"1"	/* ISO Latin-1 */
#define ISO8859_2_ENC	"2"	/* ISO Latin-2 */
#define ISO8859_6_ENC	"6"	/* ISO Latin Arabic */
#define ISO8859_7_ENC	"7"	/* ISO Latin Greek */
#define ISO8859_8_ENC	"8"	/* ISO Latin Hebrew */
#define NARROW "NARROW"
#define SET_NARROW "_NARROW"
#define TERMINAL_DECTECH "TERMINAL_DECTECH"
#define SYMBOL "SYMBOL"
#define HELVETICA "HELVETICA"
#define NEWCENTURYSCHLBK "NEWCENTURYSCHLBK"
#define DUTCH801 "DUTCH801"
#define PRESENT_BULLETS "PRESENT_BULLETS"
#define ZAPFCHANCERY_MI "ZAPFCHANCERY_MEDIUMITALIC"
#define ZAPFDINGBATS "ZAPFDINGBATS"
#define TIMES "TIMES"
#else
#define SETCASE(str) { int i, len; \
		for(i=0, len=strlen(str);i<len;str[i]=tolower(str[i]),i++); }
#define ISOLATIN1 "_isolatin1"
#define ISOLATIN2 "_isolatin2"
#define ISOLATINARABIC "_isolatinarabic"
#define ISOLATINGREEK "_isolatingreek"
#define ISOLATINHEBREW "_isolatinhebrew"
#define ISO8859_REGISTRY "iso8859"
#define DEC_REGISTRY "dec"
#define	DECMATHSYMBOL	"_decmath_symbol"
#define	DECMATHITALIC	"_decmath_italic"
#define	DECMATHEXTENSION "_decmath_extension"
#define	DECTECH		"_dectech"
#define	DECPUBLISHING	"_decpublishing"
#define DECMATHSYM_ENC	"decmath_symbol"
#define DECMATHITA_ENC	"decmath_italic"
#define DECMATHEXT_ENC	"decmath_extension"
#define DECTECH_ENC 	"dectech"
#define DECPUB_ENC	"decpublish"
#define ISO8859_1_ENC	"1"	/* ISO Latin-1 */
#define ISO8859_2_ENC	"2"	/* ISO Latin-2 */
#define ISO8859_6_ENC	"6"	/* ISO Latin Arabic */
#define ISO8859_7_ENC	"7"	/* ISO Latin Greek */
#define ISO8859_8_ENC	"8"	/* ISO Latin Hebrew */
#define NARROW "narrow"
#define SET_NARROW "_narrow"
#define TERMINAL_DECTECH "terminal_dectech"
#define SYMBOL "symbol"
#define HELVETICA "helvetica"
#define NEWCENTURYSCHLBK "newcenturyschlbk"
#define DUTCH801 "dutch801"
#define PRESENT_BULLETS "present_bullets"
#define ZAPFCHANCERY_MI "zapfchancery_mediumitalic"
#define ZAPFDINGBATS "zapfdingbats"
#define TIMES "times"
#endif


#define NORMAL "Normal"
#define ROMAN "Roman"
#define ITALIC "Italic"
#define OBLIQUE "Oblique"
#define MEDIUM "Medium"
#define BOLD "Bold"
#define LIGHT "Light"
#define BOOK "Book"
#define DEMI "Demi"
#define HYPHEN "-"


#define CHAR_METRICS_ON_KEY "StartCharMetrics"
#define CHAR_METRICS_OFF_KEY "EndCharMetrics"

#define	CHAR_NONE		0
#define	CHAR_ISOLATIN1		1
#define	CHAR_ISOLATIN2		2
#define	CHAR_ISOLATINARABIC	3
#define	CHAR_ISOLATINGREEK	4
#define	CHAR_ISOLATINHEBREW	5
#define	CHAR_DECMATH_SYMBOL	6
#define	CHAR_DECMATH_ITALIC	7
#define	CHAR_DECMATH_EXTENSION	8
#define	CHAR_DECTECH		9
#define	CHAR_DECPUBLISHING	10

	/* Define the X font names */
#ifdef	vms
noshare                   
#endif	/* vms */
    char
	x_name_0[] = "COURIER",
	x_name_1[] = "HELVETICA",
	x_name_2[] = "TIMES",
	x_name_3[] = "ITC AVANT GARDE GOTHIC",
	x_name_4[] = "ITC LUBALIN GRAPH",
	x_name_5[] = "ITC SOUVENIR",
	x_name_6[] = "NEW CENTURY SCHOOLBOOK",
	x_name_7[] = "TERMINAL",		/* Match to TERMINAL_DECTECH */
	x_name_8[] = "SYMBOL",
	x_name_9[] = "ITC ZAPF DINGBATS",
	x_name_10[] = "ITC ZAPF CHANCERY",
	x_name_11[] = "ITC BOOKMAN",
	x_name_12[] = "PALATINO",
	x_name_13[] = "PRESENT BULLETS",
	x_name_14[] = "NARKISSTAM",		/* ISOLATINHEBREW */
	x_name_15[] = "MIRIAMFIXED",		/* ISOLATINHEBREW */
	x_name_16[] = "MIRIAM",			/* ISOLATINHEBREW */
	x_name_17[] = "GAM",			/* ISOLATINHEBREW */
	x_name_18[] = "FRANKRUHL",		/* ISOLATINHEBREW */
	x_name_19[] = "DAVID",			/* ISOLATINHEBREW */
	x_name_20[] = "DUTCH801",		/* DECMATH_SYMBOL, DECMATH_EXTENSION, DECMATH_ITALIC */
	x_name_21[] = "FIXED";   		/* FIXED font */

#ifdef  vms
noshare
#endif  /* vms */
    char *x_name_table[] = {
	x_name_0,
	x_name_1,
	x_name_2,
	x_name_3,
	x_name_4,
	x_name_5,
	x_name_6,
	x_name_7,
	x_name_8,
	x_name_9,
	x_name_10,
	x_name_11,
	x_name_12,
	x_name_13,
	x_name_14,
	x_name_15,
	x_name_16,
	x_name_17,
	x_name_18,
	x_name_19,
	x_name_20,
	x_name_21
    };


	/* Define the ADOBE font names */
#ifdef	vms
noshare 
#endif	/* vms */
    char
	adobe_name_0[] = "Courier",
	adobe_name_1[] = "Helvetica",
	adobe_name_2[] = "Times",
	adobe_name_3[] = "AvantGarde",
	adobe_name_4[] = "LubalinGraph",
	adobe_name_5[] = "Souvenir",
	adobe_name_6[] = "NewCenturySchlbk",
	adobe_name_7[] = "Terminal_Dectech",
	adobe_name_8[] = "Symbol",
	adobe_name_9[] = "Zapfdingbats",
	adobe_name_10[] = "Zapfchancery_MediumItalic",
	adobe_name_11[] = "Bookman",
	adobe_name_12[] = "Palatino",
	adobe_name_13[] = "Present_Bullets",
	adobe_name_14[] = "Narkisstam",		/* isolatinhebrew */
	adobe_name_15[] = "MiriamFixed",	/* ISOLATINHEBREW */
	adobe_name_16[] = "Miriam",		/* ISOLATINHEBREW */
	adobe_name_17[] = "Gam",	      	/* ISOLATINHEBREW */
	adobe_name_18[] = "Frankruhl",		/* ISOLATINHEBREW */
	adobe_name_19[] = "David",	    	/* ISOLATINHEBREW */
	adobe_name_20[] = "Dutch801",		/* DECMATH_SYMBOL, DECMATH_EXTENSION, DECMATH_ITALIC */
	adobe_name_21[] = "Courier";		/* Map FIXED to COURIER */

#ifdef  vms
noshare
#endif  /* vms */
    char *adobe_name_table[] = {
	adobe_name_0,
	adobe_name_1,
	adobe_name_2,
	adobe_name_3,
	adobe_name_4,
	adobe_name_5,
	adobe_name_6,
	adobe_name_7,
	adobe_name_8,
	adobe_name_9,
	adobe_name_10,
	adobe_name_11,
	adobe_name_12,
	adobe_name_13,
	adobe_name_14,
	adobe_name_15,
	adobe_name_16,
	adobe_name_17,
	adobe_name_18,
	adobe_name_19,
	adobe_name_20,
	adobe_name_21
    };

	/* Define the preferred character set to look for */
#ifdef	vms
noshare 
#endif	/* vms */
    unsigned long  char_set_match[] = {
	CHAR_ISOLATIN1,		/* "COURIER" */
	CHAR_ISOLATIN1,		/* "HELVETICA" */
	CHAR_ISOLATIN1,		/* "TIMES" */
	CHAR_ISOLATIN1,		/* "ITC AVANT GARDE GOTHIC" */
	CHAR_ISOLATIN1,		/* "ITC LUBALIN GRAPH" */
	CHAR_ISOLATIN1,		/* "ITC SOUVENIR" */
	CHAR_ISOLATIN1,		/* "NEW CENTURY SCHOOLBOOK" */
	CHAR_ISOLATIN1,		/* "TERMINAL" */
	CHAR_ISOLATIN1,		/* "SYMBOL" */
	CHAR_ISOLATIN1,		/* "ITC ZAPF DINGBATS" */
	CHAR_ISOLATIN1,		/* "ITC ZAPF CHANCERY" */
	CHAR_ISOLATIN1,		/* "ITC BOOKMAN" */
	CHAR_ISOLATIN1,		/* "PALATINO" */
	CHAR_ISOLATIN1,		/* "PRESENT BULLETS" */
	CHAR_ISOLATINHEBREW,	/* "NARKISSTAM" */                             
	CHAR_ISOLATINHEBREW,	/* "MIRIAMFIXED" */
	CHAR_ISOLATINHEBREW,	/* "MIRIAM" */
	CHAR_ISOLATINHEBREW,	/* "GAM" */
	CHAR_ISOLATINHEBREW,	/* "FRANKRUHL" */
	CHAR_ISOLATINHEBREW,	/* "DAVID" */
	CHAR_DECMATH_SYMBOL	/* "DUTCH801" */
};

#define	KNOWN_NAME_MAX_INDEX	21

#define TMP_STRING_SIZE 250




/********************
 * FORWARD ROUTINES *
 ********************/
static char *   	    
cvt_to_wild_card_font PROTOTYPE((
    char    *font_name
));

static STATUS	    	    
find_first_substring PROTOTYPE((
    char	*string,
    char	*substring,
    int		*position	/* position of substring returned */
));

static BKR_FONT_DATA_PTR
get_font PROTOTYPE((
    char    	    	*wild_card_font_name,
    unsigned short int 	font_num
));

static char *	           
upper_case PROTOTYPE((
    char    *string
));

static char *		    
ParseFontNextToken PROTOTYPE((
    char **context,	/* Maintains string position */
    char delim,		/* Delimiting character */
    char *str		/* Work string, NULL on calls other than the initial */
));
    


/***********************
 * FORWARD DEFINITIONS *
 ***********************/
static BKR_FONT_DATA *font_list = NULL;
static Window	     window_for_error = 0;
static char    	     *error_string;


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_font_data_close
**
** 	Frees all the data associated with the fonts open in a book.
**
**  FORMAL PARAMETERS:
**
**	book - the book context containing the font info for the book.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	Virtual memory is freed.
**
**--
**/
void 
bkr_font_data_close PARAM_NAMES((book))
    BKR_BOOK_CTX *book PARAM_END
{
    register unsigned      max_font_id = book->max_font_id;
    register int	   i;
                 
    /* 
     *  Unload each font 
     *
     *  NOTE: font_data is a 1-based array, indexed by 
     *	      font_numbers from 0 to max_font_id.
     */
    if (book->font_data && font_list) {
        for ( i = 0; i <= max_font_id; i++ )
        {
            if (book->font_data[i] && (book->font_data[i] != FONT_NOT_FOUND))
            {
                book->font_data[i]->ref_count--;
                if (book->font_data[i]->ref_count == 0) {

                    BKR_FONT_DATA   *list;
                    BKR_FONT_DATA   *prev;

                    for ( list = font_list, prev = NULL; 
                         list != NULL; 
                         prev = list, list = list->next )
                    {
                        if ( list == book->font_data[i] )	/* match! */
                        {
                            if ( prev != NULL )
                            {
                                prev->next = list->next;
                            }
                            else
                            {
                                font_list = list->next;
                            }
                            XFreeFont( bkr_display, list->font_struct);
                            BKR_FREE( list->font_name );
                            BKR_FREE( list );
                            break;
                        }
                    }
                }
            }
        }
        
        BKR_CFREE(book->font_data );

    }
};  /* end of bkr_font_data_close */



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_font_data_init
**
** 	Initializes a font data structure for a given book.
**
**  FORMAL PARAMETERS:
**
**	book - the book context containing the font info for the book.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void 
bkr_font_data_init PARAM_NAMES((book))
    BKR_BOOK_CTX *book PARAM_END

{
    unsigned	    	max_num_entries;

    /*
     *  Allocate the font_data structure
     *
     *  NOTE: font_data is 1-based array of size max_font_id + 1, because
     *	      the array is indexed by font_numbers from 0 to max_font_id.
     */

    max_num_entries = book->max_font_id + 1;

    book->font_data = (BKR_FONT_DATA_PTR *) BKR_CALLOC( max_num_entries, 
    	    	    	    	    	    sizeof( BKR_FONT_DATA_PTR ) );

    memset( book->font_data, 0, ( sizeof( BKR_FONT_DATA_PTR ) * max_num_entries));

};  /* end of bkr_font_data_init */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_font_entry_init
**
** 	Initializes the fields a font data entry.
**
**  FORMAL PARAMETERS:
**
**	font_entry   - pointer the font entry to initialize.
**	font_num     - number of the font to be opened.
**	book         - book context
**	window_id    - id of a window to used to report errors.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns a pointer to the font data.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
BKR_FONT_DATA_PTR
bkr_font_entry_init PARAM_NAMES((book,font_num))
    BKR_BOOK_CTX *book PARAM_SEP
    unsigned short int 	font_num PARAM_END

{
    char    	    	*book_font_name = NULL;
    char	    	*font_name_to_find = NULL;
    unsigned	    	status;
    unsigned short      major_version =  book->version.major_num;

    /* Save the window id for signalling errors */

    window_for_error = NULL;

    /* Get the font name from the font number 
     */
    book_font_name = bri_book_font_name( book->book_id, font_num );

    if ( book_font_name == NULL )
    {
        if (font_num == 0) {

            /* Assume this is a ditroff produced book which doesn't have
             * a font number 0.
             */
            book->font_data[font_num] = FONT_NOT_FOUND;
            return FONT_NOT_FOUND;
        } else {
            book_font_name = "fixed";
            error_string = (char *)bkr_fetch_literal("UNDEFINED_FONT_NUMBER",
                                                     ASCIZ );
            if ( error_string != NULL )
            {
                sprintf( errmsg, error_string, font_num );
                bkr_error_modal( errmsg, window_for_error );
                XtFree( error_string );
            }
        }
    }

    /*
     * Convert the hard coded 75 dpi x and y pixel resolutions to wild cards
     * for version 1 books on monitors other than 75 dpi.
     */

    if ((bkr_monitor_resolution != 75) 
        && (major_version == VERSION_V1) 
        && (bkr_char_cell == FALSE)
        )
    {
	font_name_to_find = (char *) cvt_to_wild_card_font( book_font_name );
	if ( font_name_to_find == NULL ) {
            /* Initialize the font entry now 
             */
            book->font_data[font_num] = get_font( book_font_name, font_num );
        } else {
            /* Initialize the font entry now 
             */
            book->font_data[font_num] = get_font( font_name_to_find, font_num );
            BKR_FREE( font_name_to_find );
        }
    } else {
        /* Initialize the font entry now 
         */
        book->font_data[font_num] = get_font( book_font_name, font_num );
    }
    return ( book->font_data[font_num] );

};	/* end bkr_font_entry_init */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	cvt_to_wild_card_font
**
**  	Convert the hard coded 75 dpi x and y pixel resolutions in the
**  	font name we get out of the book to wild cards.  This is done by
**  	looking for the pattern "-75-75-" and replacing it with "-*-*-"
**
**  FORMAL PARAMETERS:
**
**	font_name - pointer to the font name to convert.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns:    - Pointer to converted font name.
**  	    	    - NULL, if an error occurred converting.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static char *
cvt_to_wild_card_font PARAM_NAMES((font_name))
    char    *font_name PARAM_END
{
    char    	*font_name_ptr = NULL,
		*oldstring_ptr = "-75-75-",
		*newstring_ptr = "-*-*-";
    int		oldstring_len = strlen (oldstring_ptr),
		newstring_len = strlen (newstring_ptr);
    unsigned	status;
    int		index;

    status = (unsigned) find_first_substring (font_name, oldstring_ptr, &index);

    /*
     * Match found. Allocate some memory, copy the font name from the 
     * beginning up to the match, copy the replacement string, and then copy 
     * the rest of the font name to the end.
     */

    if ( status )
    {
	font_name_ptr = (char *) BKR_MALLOC( strlen( font_name ) + 1 );
	strncpy( font_name_ptr, font_name, index );
	strncpy( ( font_name_ptr + index ), newstring_ptr, newstring_len );
	strcpy( ( font_name_ptr + index + newstring_len ), 
	    ( font_name + index + oldstring_len ) );	/* copies NULL */
    }	

    return ( font_name_ptr );

};   /* end of cvt_to_wild_card_font */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	find_first_substring
**
**  	Finds the first match of the substring in the string.
**
**  FORMAL PARAMETERS:
**
**  	string	    - pointer to the string to be searched.
**  	substring   - pointer to the substring match.
**  	position    - address of a pointer to return the position.
**  
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns:    1 - if the substring was found in the string.
**  	    	    0 - if no match was found.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static STATUS
find_first_substring PARAM_NAMES((string,substring,position))
    char	*string    PARAM_SEP
    char	*substring PARAM_SEP
    int		*position  PARAM_END /* position of substring returned */
{
    char	*str_ptr;

    str_ptr = strstr( string, substring );
    if ( str_ptr == NULL )
	return ( 0 );	    	/* no match found */
    else
    {
    	/* Found match; return offset into string */

	*position = (int) str_ptr - (int) &string[0];

	return ( 1 );	    	/* substring found! */
    }

};	/* end of find_first_substring */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	get_font
**
** 	Initializes a font entry given the wildcard font name.  The wildcard
**  	font name is used to query the server to find a fully qualified
**  	font name and then the local cached font list is searched.  If
**  	no fully qualified font name was returned the wildcard name
**  	is converted to a fallback font name.
**
**  FORMAL PARAMETERS:
**
**  	wild_card_font_name - pointer to the wild card font name.
**	font_num            - the number of the font in the book
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns:    pointer to the font entry
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static BKR_FONT_DATA_PTR
get_font PARAM_NAMES((wild_card_font_name,font_num))
    char    	    	*wild_card_font_name PARAM_SEP
    unsigned short int 	font_num PARAM_END

{
    char    	    	*valid_fontname = NULL;
    char    	    	*uppercase_name = NULL;
    char    	    	*fallback_fontname = NULL;
    char    	    	*fontname_to_match = NULL;
    char	    	**font_name_rtn;    	/* font name returned  */
    int		    	num_fonts_found;
    BKR_FONT_DATA   	*font_entry;

    /* Get the full font name and information */

    fontname_to_match = wild_card_font_name;
    while ( TRUE )
    {
    	font_name_rtn = (char **) XListFonts(
	    bkr_display,
	    fontname_to_match,	    	/* font pattern to match */
	    1,				/* match ONLY 1 font     */
	    &num_fonts_found);		/* # of fonts returned   */
    	if ( num_fonts_found != 0 )
    	{
    	    valid_fontname = font_name_rtn[0];
    	    break;
    	}

    	/* No mapping found on server, find a fallback font instead */

    	fallback_fontname = (char *) DXmFindFontFallback( fontname_to_match );
    	if ( fallback_fontname == NULL )
    	{
    	    error_string = (char *) bkr_fetch_literal( "OPEN_FONT_ERROR", ASCIZ );
    	    if ( error_string != NULL )
    	    {
	    	sprintf( errmsg, error_string, wild_card_font_name );
	    	bkr_error_modal( errmsg, window_for_error );
	    	XtFree( error_string );
    	    }
    	    return ( FONT_NOT_FOUND );
    	}
    	fontname_to_match = fallback_fontname;

    }	/* end while */

    /* Uppercase the font name
     */
    uppercase_name = (char *) upper_case( valid_fontname );

     /* Try to match the font
      */
    font_entry = font_list;
    while (font_entry && ( strcmp( font_entry->font_name, uppercase_name ) != 0 )) {
        font_entry = font_entry->next;
    }

    if ( font_entry ) 
    {
    	font_entry->ref_count++;

    } else {

        /*  
         *  Font not found in global list, so add the font to the list.
         *  NOTE: we store the font name in upper case for compares.
         */

        font_entry = (BKR_FONT_DATA_PTR)BKR_MALLOC(sizeof(BKR_FONT_DATA));
        font_entry->font_name = (char *) BKR_MALLOC( strlen( uppercase_name ) + 1 );
        strcpy( font_entry->font_name, uppercase_name );
        
        /* Load the font 
         */
        font_entry->font_struct = XLoadQueryFont( bkr_display, uppercase_name );
        if ( font_entry->font_struct == NULL )
        {
            error_string = (char *) bkr_fetch_literal( "LOAD_FONT_ERROR", ASCIZ );
            if ( error_string != NULL )
            {              
                sprintf( errmsg, error_string, uppercase_name );
                bkr_error_modal( errmsg, window_for_error );
                XtFree( error_string );
            }
            font_entry->font_struct = bkr_default_font;
        }
        if (XGetFontProperty(font_entry->font_struct,
                             XA_NORM_SPACE,
                             &font_entry->min_space)
            == FALSE) 
        {
            font_entry->min_space = font_entry->font_struct->min_bounds.width;
        }

#ifdef I18N_MULTIBYTE
        /*
         ** Check if it is 2-byte font or not
         */
        font_entry->font_2byte = FALSE;
        if ((font_entry->font_struct->min_byte1 != 0) ||
            (font_entry->font_struct->max_byte1 !=0)) 
        {
            font_entry->font_2byte = TRUE;
        }
#endif
        font_entry->ref_count = 1;
        font_entry->next = font_list;
        font_list = font_entry;
        
    }

    BKR_FREE( uppercase_name );

    if ( fallback_fontname != NULL )
    	XtFree( fallback_fontname );
    return ( font_entry );

};  /* end of get_font */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	upper_case
**
** 	Upper cases a string.
**
**  FORMAL PARAMETERS:
**
**	string - pointer to the string to upper case.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns:    Pointer to the upper cased string.  The calling routine
**  	    	    must use BKR_FREE to free the memory.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static char *
upper_case PARAM_NAMES((string))
    char    *string PARAM_END

{
    int	    cnt;
    char    *string_ptr;
    int	    string_len = strlen( string );

    string_ptr = (char *) BKR_MALLOC( string_len + 1 );

    for ( cnt = 0; cnt < string_len; cnt++ ) 
    {
    	if ( islower( string[cnt] ) )
	    string_ptr[cnt] = _toupper( string[cnt] );
    	else
    	    string_ptr[cnt] = string[cnt];  /* just copy char */
    }

    string_ptr[ string_len ] = NULLCHAR;

    return ( string_ptr );

};  /* end of upper_case */




/*
**++
**  FUNCTIONAL DESCRIPTION:
**      Parse an X font name and return the corresponding family, style,
**	weight, and size in Adobe terms.
**
**  FORMAL PARAMETERS:
**      See Below
**
**  FUNCTION VALUE:
**      None
**
**  SIDE EFFECTS:
**      None
**
**  DESIGN:
**	o   Separate the font name into tokens (delimited by "-")
**
**	o   The meaning of a token depends upon its position.
**
**	o   Translate any coded token to its full name
**--
*/
void bkr_font_parse_name PARAM_NAMES((font_name,family,style,weight,
                                      set_width_name,size,char_set,
                                      ps_font_name))
    char *font_name PARAM_SEP
    char *family PARAM_SEP     
    char *style PARAM_SEP
    char *weight PARAM_SEP
    char *set_width_name PARAM_SEP
    long int *size PARAM_SEP
    unsigned long *char_set PARAM_SEP
    char *ps_font_name PARAM_END
{
    int token_num, i, found_map, string_length, save_index = -1;
    char *token, *work_str, *context;
    char	tmp_family[256],
		tmp_registry[80],
		tmp_encoding[80];

    /* Copy the input font_name to a font_name that can be */
    /* trashed by ParseFontNextToken */
    work_str = (char *) malloc( (int)(strlen(font_name)+1) );
    strcpy( work_str, font_name );

    /* Default the character set to ISOLATIN1 */
    *char_set = CHAR_ISOLATIN1;

    /* Parse the font name */
    token_num = 0;
    context = work_str;
    while (token = ParseFontNextToken( &context, '-', (token_num?0:work_str))){
        switch (token_num)
        {
            case 2: {	    /* Typeface family */ 
		strcpy( tmp_family, token );
		UPCASE(tmp_family);

		    /* Attempt to match the X family name to an Adobe family name */
		found_map = 0;
		i = 0;
		while ((found_map == 0) && (i <= KNOWN_NAME_MAX_INDEX))  {
		    if  (strcmp( tmp_family, x_name_table[i]) == 0) {
			strcpy(family, adobe_name_table[i]);
			*char_set = char_set_match[i];		/* Store the default character set for the font family */
			save_index = i;
			found_map = 1;
		    }
		    i++;
		}	

		/* If we didn't have a name mapping, then use the name that was given for X */
		if (found_map == 0)  
		    strcpy (family, tmp_family);

		break;
	    }
            case 3: {	    /* Weight */ 
		strcpy( weight, token );
		weight[0] = toupper (weight[0]);
		string_length = strlen (weight);
		for (i = 1; i < string_length; weight[i] = tolower (weight[i]), i++);
		break;
	    }
            case 4: {	    /* Style */ 
		strcpy( style, token );
		UPCASE(style);
		switch (style[0]){
		    case 'R':
			strcpy( style, ROMAN);
			break;
		    case 'I':
			strcpy( style, ITALIC);
			break;
		    case 'O':
			strcpy( style, OBLIQUE);
			break;
		}			
		break;
	    }
            case 5: {	    /* Set Width Name */ 
		strcpy(set_width_name, token);
		SETCASE(set_width_name);
		break;
	    }
            case 8: {	    /* Point Size */ 
		*size = atoi(token);
		break;
	    }
            case 13: {	    /* Character set registry */ 
		strcpy(tmp_registry, token);
		SETCASE(tmp_registry);
		break;
	    }
            case 14: {	    /* Character set encoding */ 
		strcpy(tmp_encoding, token);
		SETCASE(tmp_encoding);
		break;
	    }
        }
	token_num++;
    }

	/* Now check the character set registry and encoding fields */
	/* If there is information present, then use that info to */
	/* set the character set field */
    if  ((strlen(tmp_registry) > 0) && (strcmp(tmp_registry,"*")))  {
	    /* If the registry was DEC, then check the encodings for ones that we recognize */
	if  (!strcmp(tmp_registry,DEC_REGISTRY)) {
	    if  (!strcmp(tmp_encoding,DECMATHSYM_ENC))  *char_set = CHAR_DECMATH_SYMBOL;
	    if  (!strcmp(tmp_encoding,DECMATHITA_ENC))  *char_set = CHAR_DECMATH_ITALIC;
	    if  (!strcmp(tmp_encoding,DECMATHEXT_ENC))  *char_set = CHAR_DECMATH_EXTENSION;
	    if  (!strcmp(tmp_encoding,DECTECH_ENC))  *char_set = CHAR_DECTECH;
	    if  (!strcmp(tmp_encoding,DECPUB_ENC))  *char_set = CHAR_DECPUBLISHING;
	}           

	    /* If the registry was ISO8859, then check the encodings for ones that we recognize */
	if  (!strcmp(tmp_registry,ISO8859_REGISTRY)) {
	    if  (!strcmp(tmp_encoding,ISO8859_1_ENC))  *char_set = CHAR_ISOLATIN1;
	    if  (!strcmp(tmp_encoding,ISO8859_2_ENC))  *char_set = CHAR_ISOLATIN2;
	    if  (!strcmp(tmp_encoding,ISO8859_6_ENC))  *char_set = CHAR_ISOLATINARABIC;
	    if  (!strcmp(tmp_encoding,ISO8859_7_ENC))  *char_set = CHAR_ISOLATINGREEK;
	    if  (!strcmp(tmp_encoding,ISO8859_8_ENC))  *char_set = CHAR_ISOLATINHEBREW;
	}
    }

    free( work_str );

    /* If no match was found, return Courier 12 by default */
    if (save_index == -1) {
	save_index = 1;
	strcpy (family, adobe_name_0);
	strcpy (weight, MEDIUM);
	strcpy (style, NORMAL);
        *size = 120;   
    }

    /* All Postscript names start with the Family */
    strcpy (ps_font_name, family);

    /* Special case to create proper Postscript name.  There are 3 major
       cases: Courier and Helvetica make up #1, Times and New Century
       Schoolbook make up #2, and Avant Garde, Lubalin Graph, and Souvenir make
       up #3 */ 
    switch (save_index) {
	case 0:
	case 1:
	    if (strcmp (weight, BOLD) == 0) {
		strcat (ps_font_name, HYPHEN);
		strcat (ps_font_name, BOLD);
		if (strcmp (style, OBLIQUE) == 0)
		    strcat (ps_font_name, OBLIQUE);
	    }
	    else if (strcmp (style, OBLIQUE) == 0) {
		strcat (ps_font_name, HYPHEN);
		strcat (ps_font_name, OBLIQUE);
	    }
	    break;

	case 2:
	case 6:
	    strcat (ps_font_name, HYPHEN);
	    if (strcmp (weight, BOLD) == 0) {
		strcat (ps_font_name, BOLD);
		if (strcmp (style, ITALIC) == 0)
		    strcat (ps_font_name, ITALIC);
	    }
	    else if (strcmp (style, ITALIC) == 0)
		strcat (ps_font_name, ITALIC);
	    else
		strcat (ps_font_name, ROMAN);
	    break;

	case 3:
	case 4:
	case 5:
	    /* All will have the hyphen */
	    strcat (ps_font_name, HYPHEN);

	    /* All will have the weight, but make sure only the first letter is
	       capitalized */
	    strcat (ps_font_name, weight);

	    /* Add the style, which is already the proper case, only if it is
	       italic or oblique */ 
	    if ((strcmp (style, ITALIC) == 0) ||
		(strcmp (style, OBLIQUE) == 0))
		strcat (ps_font_name, style);

	    break;
    }
}




/*
**++
**  FUNCTIONAL DESCRIPTION:
**      Return a pointer to the next token.  This routine differs from
**	strtok in that the delimiter is limited to a single character,
**	and each occurence of a delimiter indicates a separate token.
**
**  FORMAL PARAMETERS:
**      See Below
**
**  FUNCTION VALUE:
**      token - pointer to the next token in str
**
**  SIDE EFFECTS:
**      None
**
**  DESIGN:
**	o   context points to the next token
**
**	o   starting at context search for the next delimiter and replace it
**	    with a 0.
**
**	o   set context to the character position after the 0.
**
**	o   return the previous value of context as the token pointer.
**--
*/
static char * ParseFontNextToken PARAM_NAMES((context,delim,str))
    char **context PARAM_SEP	/* Maintains string position */
    char delim PARAM_SEP		/* Delimiting character */
    char *str PARAM_END	/* Work string, NULL on calls other than the initial */

{
    char *p;

    if (str)
	*context = str;

    p = *context;
    if (p)
	*context = strchr( *context, delim );
    if (*context){
	**context = 0;
	(*context)++;
    }    
    	
    return p;
}


