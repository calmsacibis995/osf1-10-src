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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BR_META_DATA.H*/
/* *17   24-FEB-1993 17:38:29 BALLENGER "Add BMD_RECTANGLE data type."*/
/* *16   15-OCT-1992 16:50:23 BALLENGER "Accumulate x,y values before scaling to lessen affects of scaling."*/
/* *15   24-JUL-1992 12:27:56 KLUM "repress print if NO_PRINT alt-prod-name"*/
/* *14   14-JUL-1992 16:48:37 BALLENGER "UXC$CONVERT"*/
/* *13   14-JUL-1992 16:38:42 BALLENGER "Character cell support."*/
/* *12   19-JUN-1992 20:13:47 BALLENGER "Cleanup for Alpha/OSF port"*/
/* *11    8-JUN-1992 18:57:30 BALLENGER "UCX$CONVERT"*/
/* *10    8-JUN-1992 13:39:34 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *9     8-MAR-1992 19:13:59 BALLENGER " Add topic data and text line support"*/
/* *8     6-MAR-1992 17:53:59 ROSE "Done"*/
/* *7     4-MAR-1992 14:22:06 KLUM "removed BMD_MIN_DATA_CHUNK"*/
/* *6     3-MAR-1992 17:12:15 KARDON "UCXed"*/
/* *5    10-JAN-1992 10:51:44 FITZELL "added new chunk types and new topic types for Green"*/
/* *4    10-JAN-1992 10:48:17 FITZELL "Added new chunk types for Green"*/
/* *3     1-NOV-1991 13:10:47 BALLENGER "Reintegrate memex support"*/
/* *2    17-SEP-1991 21:23:20 BALLENGER "change use of major and minor in version number to avoid conflict with sys/types.h*/
/*definition on ULTRIX"*/
/* *1    16-SEP-1991 12:48:35 PARMENTER "API definitions"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BR_META_DATA.H*/
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
**
**
** Facility:
**
**      br_meta_data.h
**
** Abstract:
**
**      Public definitions for users of Book Reader/Writer Interface
**      routines.
**
** Author:
**
**      David L. Ballenger
**
** Date:
**
**      Tue May 30 15:15:32 1989
**
** Revision History:
**
**      19-Feb-1991 David L Ballenger
**                  Add support for BMD_CHUNK_RAGS_NO_FILL for PIC 
**                  graphics support.
**
**      06-Feb-1991 Davd L Ballenger
**                  Portability / C standards compliance cleanup,
**                  change 0 sized arrays in structures to be declared
**                  with one element.  
**
**  	29-Nov-1990 James A. Ferguson
**  	    	    Add external declaration for bri_book_copyright_info.
**
**  	15-Aug-1990 James A. Ferguson
**  	    	    Create new module.
**
**      16-Jan-1990 David L Ballenger
**                  Add latent support for titles and comments in bookshelf 
**                  files.
*/


#ifndef _BR_META_DATA_H
#define _BR_META_DATA_H

#include <X11/Intrinsic.h>
#ifndef _BR_COMMON_DEFS_H
#include "br_common_defs.h"	/* Make sure that we have these defs available. */
#endif 
#include "br_prototype.h"

/* Entry types for bookshelf files.
 */

typedef enum {
    	BMD_C_UNKNOWN = 0,
    	BMD_C_BOOK,
    	BMD_C_SHELF,
    	BMD_C_TITLE,
    	BMD_C_COMMENT
  } BMD_ENTRY_TYPE;

/* Valid object types
*/

typedef enum {
    BMD_C_NO_OBJECT_TYPE = 0,
    BMD_C_BOOK_OBJECT,
    BMD_C_SHELF_OBJECT,
    BMD_C_SHELF_ENTRY_OBJECT,
    BMD_C_DIRECTORY_OBJECT,
    BMD_C_DIRECTORY_ENTRY_OBJECT,
    BMD_C_TOPIC_OBJECT,
    BMD_C_CHUNK_OBJECT,
    BMD_C_SYMBOL_EXREF_OBJECT,
    BMD_C_STATIC_EXREF_OBJECT
} BMD_OBJECT_TYPE;


/* Define a generic pointer type.  
 */

typedef BR_GENERIC_PTR BMD_GENERIC_PTR;
typedef char	BMD_BOOLEAN;

/* Book and shelf ids are actually handles/pointers to private bri data.
 */
typedef BR_HANDLE BMD_BOOK_ID ;
typedef BR_HANDLE BMD_SHELF_ID;

/* Shelf entry ids are just 32 bit values.
 */
typedef BR_UINT_32 BMD_SHELF_ENTRY_ID;



/* Object ids are endcoded 32 bit unsigned values.
 */
typedef BR_UINT_32 BMD_OBJECT_ID;

typedef struct {
    BR_UINT_16 major_num;
    BR_UINT_16 minor_num;
} BMD_VERSION ;


/*
 *	BMD_FTEXT_PKT -- Tag Length Value format for generically coding information
 *
 *	format:
 *              +--------+--------+--------+
 *              | Tag    | Length | Value  |
 *              +--------+--------+--------+
 *
 *	(length includes the tag, length, and value fields)
 *	Constant BMD_FTEXT_PKT_LENGTH equals the total size of the type and length
 *	fields.
 *
 *	Length must be less than 256
 */

#define BMD_FTEXT_PKT_LENGTH 2	    	/* length of header                 */
typedef struct _BMD_FTEXT_PKT {
    BR_UINT_8 tag;                  /* "type" (opcode) identifier       */
    BR_UINT_8 len;                  /* length of total packet           */
    BR_UINT_8 value [1];                      /* start of data (generic)          */
    } BMD_FTEXT_PKT;

/*
 *	Tag values for electrodoc internal data format (BMD_FTEXT_PKTs)
 */

#define BMD_FTEXT_MIN_TAG   1
#define BMD_FTEXT_RULE 	    1	    	/* draw a rule                  */
#define BMD_FTEXT_TEXT300   2           /* draw text stored in 300 dpi  */
#define BMD_FTEXT_TEXT400   3           /* draw text stored in 400 dpi  */
#define BMD_FTEXT_MAX_TAG   3


/*  BMD_RULE_PKT -- packet value contents for RULE
 */

#define BMD_RULE_PKT_LENGTH 8	    	/* length of BMD_RULE_PKT packet    */
typedef struct _BMD_RULE_PKT {
    BR_INT_16 x;                        /* x coord. of upper left corner    */
    BR_INT_16 y;                        /* y coord. of upper left corner    */
    BR_INT_16 width;                    /* width of rule                    */
    BR_INT_16 height;                   /* height of rule                   */
    } BMD_RULE_PKT;


/*
 *  	BMD_TEXT_PKT -- Text packet (text length is obtained by subtracting the
 *		FTEXT$K_LENGTH from the total packet length.
 *
 *		The format of the data field is "text_words", where each
 *		text_word starts with a 1 byte "delta" value which
 *		is the amount of space to put before the word, followed
 *		by a 1 byte count, followed by that many letters,
 */

#define BMD_TEXT_PKT_LENGTH 6           /* length of BMD_TEXT_PKT packet header */
typedef struct _BMD_TEXT_PKT {
    BR_INT_16 x;                        /* x coord. of first char           */
    BR_INT_16 y;                        /* y coord. of baseline             */
    BR_UINT_16 font_num;        /* number assoc w/font by DEFINE_FONT */
    BR_UINT_8 data [1];                      /* start of data                    */
    } BMD_TEXT_PKT;

#define BMD_WORD_PKT_LENGTH 2           /* length of BMD_WORD_PKT packet    */
typedef struct _BMD_WORD_PKT {
    BR_UINT_8 delta;                /* horizontal offset to start of word */
    BR_UINT_8 count;                /* number of char's in word         */
    char chars [1];                      /* start of text                    */
    } BMD_WORD_PKT;


/*  BMD_IMAGE_PKT -- format for image information
 */

#define BMD_IMAGE_PKT_LENGTH 8          /* length of BMD_IMAGE_PKT packet   */
typedef struct _BMD_IMAGE_PKT {
    BR_INT_16 res_x;                    /* horizontal resolution created at */
    BR_INT_16 res_y;                    /* vertical resolution created at   */
    BR_INT_16 pix_width;                /* width of image in pixels         */
    BR_INT_16 pix_height;               /* height of image in pixels        */
    BR_UINT_8 data [1];                       /* start of image data              */
    } BMD_IMAGE_PKT;


/*  BMD_POINT -- Defines a polygon point (unscaled) below the BRI interface
 */

typedef struct _BMD_POINT {
    BR_INT_32	    x;
    BR_INT_32	    y;
    } BMD_POINT;

/*  BMD_XPOINT -- Defines a polygon point (scaled) above the BRI interface
 */

typedef struct _BMD_XPOINT {
    BR_INT_16	    x;
    BR_INT_16	    y;
    } BMD_XPOINT;

/* BMD_RECTANGLE - Defines a rectangle using top, bottom, left, and right
 */
typedef struct _BMD_RECTANGLE {
    BR_INT_32 top;
    BR_INT_32 bottom;
    BR_INT_32 height;
    BR_INT_32 left;
    BR_INT_32 right;
    BR_INT_32 width;
} BMD_RECTANGLE ;


/*  BMD_CHUNK -- Chunk Block
 */

typedef struct _BMD_CHUNK {
    struct _BMD_CHUNK *next;            /* So we can keep lists of chunks   */
    BR_UINT_16      chunk_type;		/* MAIN_CHUNK, SUB_CHUNK, X_REF	    */
    BR_UINT_32	    id;			/* chunk identifier                 */
    BR_UINT_32	    parent;		/* chunk identifier of parent       */
    BR_INT_32	    unscaled_x;		/* x offset within parent           */
    BR_INT_32	    unscaled_y;		/* Y offset within parent           */
    BR_INT_32	    unscaled_width;     /* width of display                 */
    BR_INT_32	    unscaled_height;	/* height of display                */
    BMD_RECTANGLE   rect;               /* Rectangle within topic           */
    BR_UINT_32	    target;		/* target id of cross reference     */
    BR_UINT_32	    data_type;		/* data type identifier		    */
    BR_UINT_8	    *data_addr;         /* display data                     */
    BR_UINT_32	    data_len;		/* length of display data           */
    BR_HANDLE	    handle;		/* data type specific information   */
					/* used for display id's etc.       */
    BR_UINT_32	    num_points;		/* no. of points for polygon chunks */
    BMD_POINT	    *point_vec;		/* polygon points (unscaled)	    */
    XPoint	    *xpoint_vec;	/* polygon points (scaled)	    */
    BR_INT_32       n_segments;        /* n hotspot segments for char cell */
    XSegment        *segments;          /* line segments for highlighting   *
                                         * hotspots in char cell mode       */
    Region	    region;		/* Region id for polygonal hotspots */
#ifdef MEMEX
    BMD_BOOLEAN	    highlight;          /* Highlight chunk in topic window  */
#endif
    struct _BKR_TEXT_LINE   *first_line;
    BR_INT_32             n_lines;
    }  BMD_CHUNK;

/*  Data type values for electrodoc internal data chunks
 */

#define BMD_MIN_CHUNK_TYPE   1
#define BMD_CHUNK_ASCII      1   	/* ASCII text                       */
#define BMD_CHUNK_FTEXT      2          /* DOCUMENT Formatted Text          */
#define BMD_CHUNK_RAGS 	     3	    	/* RAGS Graphics Editor format      */
#define BMD_CHUNK_IMAGE75    4          /* Bitmap Image --  75 dpi resolution */
#define BMD_CHUNK_IMAGE      5	    	/* Bitmap Image                     */
#define BMD_NEVER_USED 	     6          /* CHUNK_IMAGE100 -- 100dpi Bitmap Image */
#define BMD_CHUNK_DDIF 	     7          /* DDIF format                      */
#define BMD_CHUNK_POSTSCRIPT 8          /* Postscript format                */
#define BMD_CHUNK_RAGS_NO_FILL 9	/* RAGS data but don't fill the     *
                                         * background with white before     *
                                         * displaying.                      */
#define BMD_CHUNK_SGX	     10
#define BMD_CHUNK_PIXMAP     11
#define BMD_CHUNK_SIXEL      12
#define BMD_CHUNK_TIFF       13
#define BMD_CHUNK_LAUNCH     14
#define BMD_CHUNK_AUDIO      15
#define BMD_CHUNK_PROTOTYPE  16
#define BMD_MAX_CHUNK_TYPE   16

/*
 *  Data chunk types
 */

#define BMD_MIN_DATA_CHUNK  18
#define BMD_DATA_CHUNK	    18
#define BMD_DATA_SUBCHUNK   19
#define BMD_REFERENCE_RECT  20
#define BMD_REFERENCE_POLY  21
#define BMD_EXTENSION_RECT  22
#define BMD_EXTENSION_POLY  23
#define BMD_MAX_DATA_CHUNK  23

/*
 *  Topic types
 */

#define BMD_MIN_TOPIC_TYPE  1
#define BMD_STANDARD 	    1	    	/* mainline topic                   */
#define BMD_REFERENCE 	    2           /* formal reference topic           */
#define BMD_LAUNCH          3           /* launch topic */
#define BMD_MAX_TOPIC_TYPE  3

/*
 *  Directory flags
 */

#define BMD_CONTENTS_MASK     1	    	/* True for Table of Contents       */
#define BMD_INDEX_MASK 	      2         /* True for main Index              */
#define BMD_DEFAULT_MASK      4         /* True for default directory       */
#define BMD_MULTI_VALUED_MASK 8	    	/* True if multiple hits allowed    */
/*  bit mask for standard table of contents  */
#define BMD_TOC_FLAGS 	      5
/*  bitmask for standard index	*/
#define BMD_INDEX_FLAGS       10


/*  
**  BMD_SGX_PKT -- Tag Length Value format for generically coding simple graphics
**	   information
**
**	format:
**            +--------+--------+--------+
**            | Tag    | Length | Value  |
**            +--------+--------+--------+
**
**	(length includes the tag, length, and value fields)
**	Constant BMD_SGX_PKT_LENGTH equals the total size of the type and length
**	fields.
**
**	The object types below are designed to map easily and efficiently
**	to Xlib routine calls at runtime.
**
**	Text is not included in this type--subchunks and a text type 
**	(e.g. FTEXT) should be used to mix text and graphics
**
**	All packets are longword aligned (multiples of a 4 bytes)
**
**	All coordinates and dimensions are on a 400dpi coordinate system
**	Coordinates are all with respect to the chunk origin.
**	Rules for specifying arguments to XLIB routines apply--e.g.
**	angle arguments for arcs are in degrees*64 (360*64 units/revolution.
*/

typedef struct _BMD_SGX_PKT {
    BR_UINT_16 tag;			/* "type" (opcode) identifier       */
    BR_UINT_16 len;			/* length of total packet           */
    char value [1];			/* start of data (generic)          */
    } BMD_SGX_PKT;
#define BMD_SGX_PKT_LENGTH (sizeof (BMD_SGX_PKT) - sizeof(char)) /* length of header  */

/*
**  Tag values for internal data format (BMD_SGX_PKTs)           
**
*/

#define BMD_SGX_MIN_TAG	    1
#define BMD_SGX_SET	    1		/* set drawing attributes	    */
#define BMD_SGX_LINE	    2		/* draw a rule                      */
#define BMD_SGX_RECT	    3		/* draw a rectangle		    */
#define BMD_SGX_RECT_FILL   4		/* draw a filled rectangle	    */
#define BMD_SGX_ARC	    5		/* draw an arc			    */
#define BMD_SGX_ARC_FILL    6		/* draw a filled arc		    */
#define BMD_SGX_POLY	    7		/* draw connected lines 	    */
#define BMD_SGX_POLY_FILL   8		/* draw a filled polygon	    */
#define BMD_SGX_MAX_TAG	    8

/*
**  BMD_SGX_SET_PKT -- packet value contents for BMD_SGX_SET (XChangeGC)
**
**	This is used to set Graphics Context (GC) attributes.
**	The only supported attribute is line width.  
**	All other attributes use the default GC values.
*/
typedef struct _BMD_SGX_SET_PKT {
    unsigned long attribute;		/* attribute to set		    */
    unsigned long value;		/* generic value		    */
    } BMD_SGX_SET_PKT;
#define BMD_SGX_SET_PKT_LENGTH sizeof (BMD_SGX_SET_PKT) 
#define BMD_SGX_PKT_C_LINE_WIDTH    (1L<<4) /* set line width--must agree with  */
					    /* GCLineWidth definede in x.h      */

/*
**  BMD_SGX_LINE_PKT -- packet value contents for BMD_SGX_LINE (XDrawLine)
*/

typedef struct _BMD_SGX_LINE_PKT {
    BR_INT_16 x1;			/* x coord. of endpoint 1	    */
    BR_INT_16 y1;			/* y coord. of endpoint 1	    */
    BR_INT_16 x2;			/* x coord. of endpoint 2	    */
    BR_INT_16 y2;			/* y coord. of endpoint 2	    */
    } BMD_SGX_LINE_PKT;
#define BMD_SGX_LINE_PKT_LENGTH sizeof (BMD_SGX_LINE_PKT)


/*
**  BMD_SGX_RECT_PKT -- packet value contents for BMD_SGX_RECT and BMD_SGX_FILL_RECT 
**		(XDrawRectangle, XFillRectangle)
*/

typedef struct _BMD_SGX_RECT_PKT {
    BR_INT_16 x;                        /* x coord. of upper left corner    */
    BR_INT_16 y;                        /* y coord. of upper left corner    */
    BR_UINT_16 width;           /* width of rectangle		    */
    BR_UINT_16 height;          /* height of rectangle		    */
    } BMD_SGX_RECT_PKT;
#define BMD_SGX_RECT_PKT_LENGTH sizeof (BMD_SGX_RECT_PKT)   

/*
**  BMD_SGX_ARC_PKT -- packet value contents for BMD_SGX_ARC and BMD_SGX_FILL_ARC
**	       (XDrawArc, XFillArc)
*/

typedef struct _BMD_SGX_ARC_PKT {
    BR_INT_16 x;                        /* x coord. of upper left corner    */
    BR_INT_16 y;                        /* y coord. of upper left corner    */
    BR_UINT_16 width;		/* length of horizontal axis	    */
    BR_UINT_16 height;		/* length of vertical axis	    */
    BR_INT_16 start_angle;		/* start of arc (degrees * 64)	    */
    BR_INT_16 extent;			/* path and extent (degrees * 64)   */
    } BMD_SGX_ARC_PKT;
#define BMD_SGX_ARC_PKT_LENGTH sizeof (BMD_SGX_ARC_PKT)

/*
**  BMD_SGX_POLY_PKT -- packet value contents for BMD_SGX_POLY and BMD_SGX_FILL_POLY
**		(XDrawLines, XFillPolygon)
*/

typedef struct _BMD_SGX_POLY_PKT {
    BR_UINT_32 num_points;		/* number of points max=BMD_SGX_MAX_PTS */
    BMD_XPOINT points [1];   	    	/* vector of vertices		    */
    } BMD_SGX_POLY_PKT;
#define BMD_SGX_POLY_PKT_LENGTH (sizeof (BMD_SGX_POLY_PKT) - sizeof(BMD_XPOINT))
#define BMD_SGX_MAX_PTS 255			/* limit # points in a polygon	    */



#define BRI_LOCAL_OBJECT_NUMBER(obj_id)  \
    	    ( (BMD_OBJECT_ID) ((BR_UINT_32)(obj_id) & 0x00FFFFFF) )
#define BRI_DIRECTORY_ENTRY_NUMBER(obj_id)  \
    	    ( (BMD_OBJECT_ID) ((BR_UINT_32)(obj_id) & 0x00FFFFFF) )
#define BRI_DIRECTORY_ID(obj_id)    \
    	    ( (BMD_OBJECT_ID) ((BR_UINT_32)(obj_id) & 0xFF000000) )



/* Book access routines
 */
extern void           	bri_book_close PROTOTYPE((BMD_BOOK_ID bkid));
extern BMD_OBJECT_ID   	bri_book_copyright_chunk PROTOTYPE((BMD_BOOK_ID bkid));
extern char *           bri_book_copyright_info PROTOTYPE((BMD_BOOK_ID bkid));
extern BMD_OBJECT_ID    bri_book_first_page();
extern BR_UINT_32       bri_book_page_count PROTOTYPE((BMD_BOOK_ID bkid));
extern BR_UINT_32    	bri_book_directory_count PROTOTYPE((BMD_BOOK_ID bkid));
extern BMD_OBJECT_ID	bri_book_directory_contents PROTOTYPE((BMD_BOOK_ID bkid));
extern BMD_OBJECT_ID	bri_book_directory_default PROTOTYPE((BMD_BOOK_ID bkid));
extern BMD_OBJECT_ID	bri_book_directory_index PROTOTYPE((BMD_BOOK_ID bkid));
extern BR_UINT_32       bri_book_font_count PROTOTYPE((BMD_BOOK_ID bkid));
extern BR_UINT_32       bri_book_font_max_id PROTOTYPE((BMD_BOOK_ID bkid));
extern char *         	bri_book_font_name PROTOTYPE((BMD_BOOK_ID bkid, 
                                                      BR_UINT_16 font_id));
extern void	    	bri_book_timestamp PROTOTYPE((BMD_BOOK_ID bkid,
                                                      BR_UINT_32 timestamp_rtn[2]));
extern BMD_BOOK_ID      bri_book_open_file PROTOTYPE((char *file_name, char *home_dir));
extern BMD_BOOK_ID    	bri_book_open PROTOTYPE((BMD_SHELF_ID shelf_id, 
                                                 BMD_SHELF_ENTRY_ID entry_id));
extern char *         	bri_book_title PROTOTYPE((BMD_BOOK_ID bkid));
extern char *         	bri_book_file_name PROTOTYPE((BMD_BOOK_ID bkid));
extern char *         	bri_book_found_file_spec PROTOTYPE((BMD_BOOK_ID bkid));
extern void 	    	bri_book_version PROTOTYPE((BMD_BOOK_ID bkid, 
                                                    BMD_VERSION *version));
extern BR_UINT_32       bri_book_no_print PROTOTYPE((BMD_BOOK_ID bkid));
extern BR_UINT_32       bri_book_chunk_count PROTOTYPE((BMD_BOOK_ID bkid));
extern char *           bri_logical PROTOTYPE((char *name));


/* Page access routines
 */
extern BR_UINT_32       bri_page_chunk_count PROTOTYPE((BMD_BOOK_ID bkid,
                                                        BMD_OBJECT_ID pgid));
extern BMD_OBJECT_ID	bri_page_chunk_page_id PROTOTYPE((BMD_BOOK_ID bkid,
                                                          BMD_OBJECT_ID ckid));
extern char *	    	bri_get_object_symbol PROTOTYPE((BMD_BOOK_ID bkid,
                                                         BMD_OBJECT_ID pgid));
extern BMD_OBJECT_ID	bri_get_symbol_object PROTOTYPE((BMD_BOOK_ID bkid,
                                                         char *symbol));
extern BMD_OBJECT_TYPE	bri_get_exref_info PROTOTYPE((BMD_BOOK_ID bkid,
                                                      BMD_OBJECT_ID exref_id,
                                                      char	 **book_name,
                                                      char	 **book_title,
                                                      BR_UINT_32 **book_timestamp,
                                                      char	**object_name,
                                                      BMD_OBJECT_ID *object_id));
extern char *	    	bri_page_chunk_title PROTOTYPE((BMD_BOOK_ID bkid,
                                                        BMD_OBJECT_ID ckid));
extern void 	    	bri_page_close PROTOTYPE((BMD_BOOK_ID bkid,
                                                  BMD_OBJECT_ID pgid));
extern void  	    	bri_page_data PROTOTYPE((BMD_BOOK_ID bkid,
                                                 BMD_OBJECT_ID pgid,
                                                 BMD_CHUNK *chunk_list
                                                 ));
extern BMD_OBJECT_ID    bri_page_next PROTOTYPE((BMD_BOOK_ID bkid,
                                                 BMD_OBJECT_ID pgid));
extern BR_UINT_32       bri_page_open PROTOTYPE((BMD_BOOK_ID bkid,
                                                 BMD_OBJECT_ID pgid));
extern BMD_OBJECT_ID    bri_page_previous PROTOTYPE((BMD_BOOK_ID bkid,
                                                     BMD_OBJECT_ID pgid));


/* Public bookshelf handling routines
 */
extern void          bri_shelf_close PROTOTYPE((BMD_SHELF_ID shelf_id));
extern BR_UINT_32    bri_shelf_entry_count PROTOTYPE((BMD_SHELF_ID shelf_id));
extern char *        bri_shelf_home_file PROTOTYPE((BMD_SHELF_ID shelf_id,
                                                    BMD_SHELF_ENTRY_ID entry_id));
extern char *        bri_shelf_target_file PROTOTYPE((BMD_SHELF_ID shelf_id,
                                                      BMD_SHELF_ENTRY_ID entry_id
                                                      ));
extern char *	     bri_shelf_file_spec PROTOTYPE((BMD_SHELF_ID shelf_id));
extern BMD_SHELF_ID  bri_shelf_open_file PROTOTYPE((char *file_name,
                                                    char *home_dir,
                                                    BR_INT_32 *entry_count
                                                    ));
extern BMD_SHELF_ID  bri_shelf_open PROTOTYPE((BMD_SHELF_ID parent,
                                               BMD_SHELF_ENTRY_ID entry_id,
                                               BR_INT_32 *entry_count
                                               ));
extern void          bri_shelf_entry PROTOTYPE((BMD_SHELF_ID shelf_id,
                                                BMD_SHELF_ENTRY_ID entry_id,
                                                BR_UINT_32    *entry_type,
                                                char        **file_name,
                                                BR_UINT_32    *width,
                                                BR_UINT_32    *height,
                                                BR_UINT_32    *data_addr,
                                                BR_UINT_32    *data_len,
                                                BR_UINT_32    *data_type,
                                                char	**title
                                                ));
extern BMD_SHELF_ID bri_shelf_openlib PROTOTYPE((char *file_name,
                                                 BR_INT_32 *entry_count,
                                                 char *default_title));

extern BMD_ENTRY_TYPE	    bri_file_type PROTOTYPE((char *file_name));

#endif 	/* _BR_META_DATA_H */

/* DONT ADD STUFF AFTER THIS #endif */

