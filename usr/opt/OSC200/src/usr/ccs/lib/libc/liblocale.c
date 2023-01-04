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
static char	*sccsid = "@(#)$RCSfile: liblocale.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/07 23:25:25 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/*
 * To provide backwards compatibility to stuff that looks for the
 * old locale structure. These routines all expect a 16-bit wide char.
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif

#include <sys/types.h>

typedef unsigned short wchar16_t;

/*
** The following is the contents of the OSF/1 1.0 <sys/localedef.h>,
** with wchar_t replaced by wchar16_t.
*/

/* _SYS_LOCALEDEF_H_ */

/*
**	LC_COLLATE Tables:
*/
/*
**	Struct for extended collating descriptors.
*/
typedef struct  coldesc  {                     /* descriptor for col */
	short     cd_stroff;                   /* see NLcolval */
	short	  cd_repoff;
	short     cd_cval;
	short	  cd_cuniq;
} coldesc_t;

/*  
**	Struct for collation tables.
*/
typedef struct collation_table {                /* LC_COLLATE */
	short    lc_version;                    /* 1 for now */
	short	 lc_length;			/* length of this table */
	char     *lc_locale_name;		/* pointer to locale name */
	int      len_collate;
	short    *lc_collate;                   /* ptr to coll tbl */
	int      len_coluniq;
	short    *lc_coluniq;                   /* ptr to 2nd wt tbl */
	int      len_coldesc;
	coldesc_t *lc_coldesc;                  /* ptr to coldesc */
	int	 len_strings;			
	wchar16_t *lc_strings;			/* ptr to coldesc strings */
	int	 high_cvalue;			/* largest allocated uniq */
} col_t;

/*
**	LC_CTYPE Table:
**	CHARACTER COLLATING/CLASSIFICATION INFO.
**  	Struct for extended char class & converison tables
*/

typedef struct char_classification_table {      /* LC_CTYPE */
	short      lc_version; 		/* version 1 */
	short	   lc_length;  		/* length of this table */
	short      lc_code_type; 	/* 0 for now */
	short      mb_cur_max;   	/* 2 bytes max for a character */
	short      mb_cur_min;   	/* 1 byte minimum for a character */
	short      lc_dsp_width;                  
	char       *lc_locale_name; 	/* pointer to locale name */
	int        len_caseconv;   	/* table length */
	wchar16_t  *lc_caseconv;   	/* ptr to tbl */
	int	   len_ctype;
	unsigned short *lc_ctype;	/* old ctype */	
} ctype_t;

/*
**	LC_MONETARY Table
**	Struct for Monetary values
*/
typedef	struct lc_monetary_table {
	short  	lc_version;
	short 	lc_length;		/* length of this table */
	char   	*lc_locale_name;	/* pointer to locale name */
	char 	*int_curr_symbol;	/* international currency symbol*/
	char 	*currency_symbol;	/* national currency symbol	*/
	char 	*mon_decimal_point;	/* currency decimal point	*/
	char 	*mon_thousands_sep;	/* currency thousands separator*/
	char 	*mon_grouping;		/* currency digits grouping	*/
	char 	*positive_sign;		/* currency plus sign		*/
	char 	*negative_sign;		/* currency minus sign		*/
	char 	int_frac_digits;	/* internat currency fract digits*/
	char 	frac_digits;		/* currency fractional digits	*/
	char 	p_cs_precedes;		/* currency plus location	*/
	char 	p_sep_by_space;		/* currency plus space ind.	*/
	char 	n_cs_precedes;		/* currency minus location	*/
	char 	n_sep_by_space;		/* currency minus space ind.	*/
	char 	p_sign_posn;		/* currency plus position	*/
	char 	n_sign_posn;		/* currency minus position	*/
} mon_t;

/*  	
**	LC_NUMERIC Table:
**	Struct for numeric editing tables
*/
typedef struct numeric_table {                  /* LC_NUMERIC */
	short	lc_version;
	short	lc_length;		/* length of this table */
	char    *lc_locale_name;	/* pointer to locale name */
	char 	*decimal_point;
	char 	*thousands_sep;
	char	*grouping;
} num_t;

/* 
**	LC_MESSAGES Table:
**	Structure for message support 
*/
typedef struct lc_messages_table {
	short	lc_version;
	short	lc_length;		/* length of this table */
	char    *lc_locale_name;	/* pointer to locale name */
	char 	*messages;		/* Message Catalog name */
	char 	*yes_string;		/* Response string for affirmation */
	char 	*no_string;		/* Response string for negation */
} msg_t;

/*  
** 	LC_TIME table:
**	Struct for date/time editing tables
*/
typedef struct lc_time_table {
	short   lc_version;
	short	lc_length;	 /* length of this table */
	char    *lc_locale_name; /* pointer to locale name */
	char    *t_fmt;         /* NLTIME; date %X descriptor */
	char    *d_fmt;         /* NLDATE; date %x descriptor */
	char    *nlldate;       /* NLLDATE  long form         */
	char    *d_t_fmt;       /* NLDATIM, date %c descriptor */
	char    *abday;         /* NLSDAY; date %a descriptor */
	char    *day;           /* NLLDAY; date %A descriptor */
	char    *abmon;         /* NLSMONTH; date %b descriptor */
	char    *mon;           /* NLLMONTH; date %B descriptor */
/* 
**	Posix extensions needed to add capability needed for some 
**	commands like at. This allows for translation into other languages.
*/
	char    *misc;          /* NLTMISC  at;each;every;on;through  */
	char    *tstrs;         /* NLTSTRS */
	char    *tunits;        /* NLTUNITS */
/*
** 	Extended capability to name the year 
*/
	char	*year;		/* Name of the year and the starting time */
	char    *am_pm;         /* am and pm */
} tim_t;

/*
**	A table driven file code to process code mapping is 
**	achieved through this table. (For 3.2 use).
*/

typedef struct wchar_mapping_table {            /* used for wchar_t map */
	short    lc_version;
	short	 lc_length;			/* length of this table */
	char     *lc_identifier;
} map_t;


/*  Struct for runtime locale tables
 */

#define NLCTMAG0	(unsigned char)0x01
#define NLCTMAG1	(unsigned char)0x05

typedef struct localeinfo_table {           
	char     lc_mag0, lc_mag1;      /* magic numbers... */
	short    lc_version;            /* identifier */
	short    lc_code_type;                 
	short	 lc_length;		/* length of this table */
	col_t    *lc_coltbl;		/* LC_COLLATE */
	ctype_t  *lc_chrtbl;		/* LC_CTYPE */
	mon_t    *lc_montbl;		/* LC_MONETARY */
	num_t    *lc_numtbl;		/* LC_NUMERIC */
	tim_t    *lc_timtbl;		/* LC_TIME */
	msg_t    *lc_msgtbl;		/* LC_MESSAGES */
	map_t    *lc_maptbl;		/* For code page work later */
} loc_t;

/* _SYS_LOCALEDEF_H_ */

#include "setlocale.h"

static const char C_locname[] = "C";


static const short C_collate[] = {
	0, 257, 258, 259, 260, 261, 262, 263, 264, 265,
	266, 267, 268, 269, 270, 271, 272, 273, 274, 275,
	276, 277, 278, 279, 280, 281, 282, 283, 284, 285,
	286, 287, 288, 289, 290, 291, 292, 293, 294, 295,
	296, 297, 298, 299, 300, 301, 302, 303, 304, 305,
	306, 307, 308, 309, 310, 311, 312, 313, 314, 315,
	316, 317, 318, 319, 320, 321, 322, 323, 324, 325,
	326, 327, 328, 329, 330, 331, 332, 333, 334, 335,
	336, 337, 338, 339, 340, 341, 342, 343, 344, 345,
	346, 347, 348, 349, 350, 351, 352, 353, 354, 355,
	356, 357, 358, 359, 360, 361, 362, 363, 364, 365,
	366, 367, 368, 369, 370, 371, 372, 373, 374, 375,
	376, 377, 378, 379, 380, 381, 382, 383, 384, 385,
	386, 387, 388, 389, 390, 391, 392, 393, 394, 395,
	396, 397, 398, 399, 400, 401, 402, 403, 404, 405,
	406, 407, 408, 409, 410, 411, 412, 413, 414, 415,
	416, 417, 418, 419, 420, 421, 422, 423, 424, 425,
	426, 427, 428, 429, 430, 431, 432, 433, 434, 435,
	436, 437, 438, 439, 440, 441, 442, 443, 444, 445,
	446, 447, 448, 449, 450, 451, 452, 453, 454, 455,
	456, 457, 458, 459, 460, 461, 462, 463, 464, 465,
	466, 467, 468, 469, 470, 471, 472, 473, 474, 475,
	476, 477, 478, 479, 480, 481, 482, 483, 484, 485,
	486, 487, 488, 489, 490, 491, 492, 493, 494, 495,
	496, 497, 498, 499, 500, 501, 502, 503, 504, 505,
	506, 507, 508, 509, 510, 511, 513, 0, 0x0
};


static const short C_coluniq[] = {
	0, 257, 258, 259, 260, 261, 262, 263, 264, 265,
	266, 267, 268, 269, 270, 271, 272, 273, 274, 275,
	276, 277, 278, 279, 280, 281, 282, 283, 284, 285,
	286, 287, 288, 289, 290, 291, 292, 293, 294, 295,
	296, 297, 298, 299, 300, 301, 302, 303, 304, 305,
	306, 307, 308, 309, 310, 311, 312, 313, 314, 315,
	316, 317, 318, 319, 320, 321, 322, 323, 324, 325,
	326, 327, 328, 329, 330, 331, 332, 333, 334, 335,
	336, 337, 338, 339, 340, 341, 342, 343, 344, 345,
	346, 347, 348, 349, 350, 351, 352, 353, 354, 355,
	356, 357, 358, 359, 360, 361, 362, 363, 364, 365,
	366, 367, 368, 369, 370, 371, 372, 373, 374, 375,
	376, 377, 378, 379, 380, 381, 382, 383, 384, 385,
	386, 387, 388, 389, 390, 391, 392, 393, 394, 395,
	396, 397, 398, 399, 400, 401, 402, 403, 404, 405,
	406, 407, 408, 409, 410, 411, 412, 413, 414, 415,
	416, 417, 418, 419, 420, 421, 422, 423, 424, 425,
	426, 427, 428, 429, 430, 431, 432, 433, 434, 435,
	436, 437, 438, 439, 440, 441, 442, 443, 444, 445,
	446, 447, 448, 449, 450, 451, 452, 453, 454, 455,
	456, 457, 458, 459, 460, 461, 462, 463, 464, 465,
	466, 467, 468, 469, 470, 471, 472, 473, 474, 475,
	476, 477, 478, 479, 480, 481, 482, 483, 484, 485,
	486, 487, 488, 489, 490, 491, 492, 493, 494, 495,
	496, 497, 498, 499, 500, 501, 502, 503, 504, 505,
	506, 507, 508, 509, 510, 511, 513, 0, 0x0
};


static const coldesc_t C_coldesc[] = {
	0x0
};


static const wchar16_t C_strings[] = {
	0, 0x0
};

static const col_t C_col_t = {
	(short)1,
	(short)88,
	(char *)C_locname,
	(int)516,
	(short *)C_collate,
	(int)516,
	(short *)C_coluniq,
	(int)0,
	(coldesc_t *)C_coldesc,
	(long)2,
	(wchar16_t *)C_strings,
	(long)514
};


static const wchar16_t C_caseconv[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
	39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
	49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
	59, 60, 61, 62, 63, 64, 97, 98, 99, 100,
	101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
	111, 112, 113, 114, 115, 116, 117, 118, 119, 120,
	121, 122, 91, 92, 93, 94, 95, 96, 65, 66,
	67, 68, 69, 70, 71, 72, 73, 74, 75, 76,
	77, 78, 79, 80, 81, 82, 83, 84, 85, 86,
	87, 88, 89, 90, 123, 124, 125, 126, 127, 128,
	129, 130, 131, 132, 133, 134, 135, 136, 137, 138,
	139, 140, 141, 142, 143, 144, 145, 146, 147, 148,
	149, 150, 151, 152, 153, 154, 155, 156, 157, 158,
	159, 160, 161, 162, 163, 164, 165, 166, 167, 168,
	169, 170, 171, 172, 173, 174, 175, 176, 177, 178,
	179, 180, 181, 182, 183, 184, 185, 186, 187, 188,
	189, 190, 191, 192, 193, 194, 195, 196, 197, 198,
	199, 200, 201, 202, 203, 204, 205, 206, 207, 208,
	209, 210, 211, 212, 213, 214, 215, 216, 217, 218,
	219, 220, 221, 222, 223, 224, 225, 226, 227, 228,
	229, 230, 231, 232, 233, 234, 235, 236, 237, 238,
	239, 240, 241, 242, 243, 244, 245, 246, 247, 248,
	249, 250, 251, 252, 253, 254, 255, 0, 0x0
};


static const unsigned short C_ctype[] = {
	0, 32, 32, 32, 32, 32, 32, 32, 32, 32,
	40, 40, 40, 40, 40, 32, 32, 32, 32, 32,
	32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
	32, 32, 32, 72, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 132,
	132, 132, 132, 132, 132, 132, 132, 132, 132, 16,
	16, 16, 16, 16, 16, 16, 385, 385, 385, 385,
	385, 385, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 257, 257, 257, 257, 257, 257, 257, 257,
	257, 257, 16, 16, 16, 16, 16, 16, 386, 386,
	386, 386, 386, 386, 258, 258, 258, 258, 258, 258,
	258, 258, 258, 258, 258, 258, 258, 258, 258, 258,
	258, 258, 258, 258, 16, 16, 16, 16, 32, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0x0
};

static const ctype_t C_ctype_t = {
	(short)1,
	(short)56,
	(short)1,
	(short)1,
	(short)1,
	(short)1,
	(char *)C_locname,
	(int)516,
	(wchar16_t *)C_caseconv,
	(long)516,
	(unsigned short *)C_ctype
};


static const unsigned char C_mon_int_curr_symbol[] = {
	0x0
};


static const unsigned char C_mon_currency_symbol[] = {
	0x0
};


static const unsigned char C_mon_decimal_point[] = {
	0x0
};


static const unsigned char C_mon_thousands_sep[] = {
	0x0
};


static const unsigned char C_mon_grouping[] = {
	0x0
};


static const unsigned char C_mon_positive_sign[] = {
	0x0
};


static const unsigned char C_mon_negative_sign[] = {
	0x0
};

static const mon_t C_mon_t = {
	(short)0,
	(short)0,
	(char *)C_locname,
	(char *)C_mon_int_curr_symbol,
	(char *)C_mon_currency_symbol,
	(char *)C_mon_decimal_point,
	(char *)C_mon_thousands_sep,
	(char *)C_mon_grouping,
	(char *)C_mon_positive_sign,
	(char *)C_mon_negative_sign,
	0x7f,
	0x7f,
	0x7f,
	0x7f,
	0x7f,
	0x7f,
	0x7f,
	0x7f
};


static const unsigned char C_num_decimal_point[] = {
	46, 0x0
};


static const unsigned char C_num_thousands_sep[] = {
	0x0
};


static const unsigned char C_num_grouping[] = {
	0x0
};

static const num_t C_num_t = {
	(short)0,
	(short)0,
	(char *)C_locname,
	(char *)C_num_decimal_point,
	(char *)C_num_thousands_sep,
	(char *)C_num_grouping
};


static const unsigned char C_t_fmt[] = {
	37, 72, 58, 37, 77, 58, 37, 83, 0x0
};


static const unsigned char C_d_fmt[] = {
	37, 109, 47, 37, 100, 47, 37, 121, 0x0
};


static const unsigned char C_nlldate[] = {
	37, 98, 32, 37, 100, 32, 37, 89, 0x0
};


static const unsigned char C_d_t_fmt[] = {
	37, 97, 32, 37, 98, 32, 37, 100, 32, 37,
	72, 58, 37, 77, 58, 37, 83, 32, 37, 89, 0x0
};


static const unsigned char C_abday[] = {
	83, 117, 110, 58, 77, 111, 110, 58, 84, 117,
	101, 58, 87, 101, 100, 58, 84, 104, 117, 58,
	70, 114, 105, 58, 83, 97, 116, 0x0
};


static const unsigned char C_day[] = {
	83, 117, 110, 100, 97, 121, 58, 77, 111, 110,
	100, 97, 121, 58, 84, 117, 101, 115, 100, 97,
	121, 58, 87, 101, 100, 110, 101, 115, 100, 97,
	121, 58, 84, 104, 117, 114, 115, 100, 97, 121,
	58, 70, 114, 105, 100, 97, 121, 58, 83, 97,
	116, 117, 114, 100, 97, 121, 0x0
};


static const unsigned char C_abmon[] = {
	74, 97, 110, 58, 70, 101, 98, 58, 77, 97,
	114, 58, 65, 112, 114, 58, 77, 97, 121, 58,
	74, 117, 110, 58, 74, 117, 108, 58, 65, 117,
	103, 58, 83, 101, 112, 58, 79, 99, 116, 58,
	78, 111, 118, 58, 68, 101, 99, 0x0
};


static const unsigned char C_mon[] = {
	74, 97, 110, 117, 97, 114, 121, 58, 70, 101,
	98, 114, 117, 97, 114, 121, 58, 77, 97, 114,
	99, 104, 58, 65, 112, 114, 105, 108, 58, 77,
	97, 121, 58, 74, 117, 110, 101, 58, 74, 117,
	108, 121, 58, 65, 117, 103, 117, 115, 116, 58,
	83, 101, 112, 116, 101, 109, 98, 101, 114, 58,
	79, 99, 116, 111, 98, 101, 114, 58, 78, 111,
	118, 101, 109, 98, 101, 114, 58, 68, 101, 99,
	101, 109, 98, 101, 114, 0x0
};


static const unsigned char C_misc[] = {
	97, 116, 58, 101, 97, 99, 104, 58, 101, 118,
	101, 114, 121, 58, 111, 110, 58, 116, 104, 114,
	111, 117, 103, 104, 58, 97, 109, 58, 112, 109,
	58, 122, 117, 108, 117, 0x0
};


static const unsigned char C_tstrs[] = {
	110, 111, 119, 58, 121, 101, 115, 116, 101, 114,
	100, 97, 121, 58, 116, 111, 109, 111, 114, 114,
	111, 119, 58, 110, 111, 111, 110, 58, 109, 105,
	100, 110, 105, 103, 104, 116, 58, 110, 101, 120,
	116, 58, 119, 101, 101, 107, 100, 97, 121, 115,
	58, 119, 101, 101, 107, 101, 110, 100, 58, 116,
	111, 100, 97, 121, 0x0
};


static const unsigned char C_tunits[] = {
	109, 105, 110, 117, 116, 101, 58, 109, 105, 110,
	117, 116, 101, 115, 58, 104, 111, 117, 114, 58,
	104, 111, 117, 114, 115, 58, 100, 97, 121, 58,
	100, 97, 121, 115, 58, 119, 101, 101, 107, 58,
	119, 101, 101, 107, 115, 58, 109, 111, 110, 116,
	104, 58, 109, 111, 110, 116, 104, 115, 58, 121,
	101, 97, 114, 58, 121, 101, 97, 114, 115, 58,
	109, 105, 110, 58, 109, 105, 110, 115, 0x0
};


static const unsigned char C_year[] = {
	49, 57, 56, 57, 48, 49, 48, 56, 44, 72,
	101, 105, 115, 101, 105, 58, 49, 57, 50, 54,
	49, 50, 50, 53, 44, 83, 104, 111, 119, 97,
	58, 0x0
};


static const unsigned char C_am_pm[] = {
	65, 77, 58, 80, 77, 0x0
};

static const tim_t C_tim_t = {
	(short)0,
	(short)0,
	(char *)C_locname,
	(char *)C_t_fmt,
	(char *)C_d_fmt,
	(char *)C_nlldate,
	(char *)C_d_t_fmt,
	(char *)C_abday,
	(char *)C_day,
	(char *)C_abmon,
	(char *)C_mon,
	(char *)C_misc,
	(char *)C_tstrs,
	(char *)C_tunits,
	(char *)C_year,
	(char *)C_am_pm
};


static const unsigned char C_messages[] = {
	0x0
};


static const unsigned char C_yes_string[] = {
	121, 101, 115, 58, 121, 58, 89, 0x0
};


static const unsigned char C_no_string[] = {
	110, 111, 58, 110, 58, 78, 0x0
};

static const msg_t C_msg_t = {
	(short)0,
	(short)0,
	(char *)C_locname,
	(char *)C_messages,
	(char *)C_yes_string,
	(char *)C_no_string
};

static const char C_msg_identifier[] = "";

static const map_t C_map_t = {
	(short)0,
	(short)0,
	(char *)C_msg_identifier
};

static const env_t C_env = {
	"C",
	"C",
	NULL,
	(env_t *)NULL,
	{
		1,
		5,
		(short)1,
		(short)0,
		(short)64,
		(col_t *)&C_col_t,
		(ctype_t *)&C_ctype_t,
		(mon_t *)&C_mon_t,
		(num_t *)&C_num_t,
		(tim_t *)&C_tim_t,
		(msg_t *)&C_msg_t,
		(map_t *)&C_map_t
	}
};

env_t	*_envp = (env_t *)&C_env;

static loc_t	cur_loc = {
	1,
	5,
	(short)1,
	(short)0,
	(short)64,
	(col_t *)&C_col_t,
	(ctype_t *)&C_ctype_t,
	(mon_t *)&C_mon_t,
	(num_t *)&C_num_t,
	(tim_t *)&C_tim_t,
	(msg_t *)&C_msg_t,
	(map_t *)&C_map_t
};

loc_t		*_locp = &cur_loc;

