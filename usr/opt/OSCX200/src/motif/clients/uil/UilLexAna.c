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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: UilLexAna.c,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/07 00:34:39 $"
#endif
#endif

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */

/*
**++
**  FACILITY:
**
**      User Interface Language Compiler (UIL)
**
**  ABSTRACT:
**
**      This module hold the routines that build tokens for the UIL
**	compiler.
**
**--
**/


/*
**
**  INCLUDE FILES
**
**/

#include <Xm/Xm.h>
/* I think this one should be public too, it's not the case right now,
   and I don't want to include XmP.h here - dd */
extern char *_XmStringGetCurrentCharset ();
#include <Xm/XmosP.h>	/* Need this for _XmOSSetLocale, MB_CUR_MAX */

#include "UilDefI.h"
#include <ctype.h>

#ifdef I18N_MULTIBYTE
#include "UilSymGen.h"
#endif

double atof();

#include <errno.h>	/* needed to support conversion functions */

/*
**
**  TABLE OF CONTENTS
**
*/

/*
** FORWARD DECLARATIONS
*/

typedef struct	_lex_buffer_type
{
    struct _lex_buffer_type	*az_next_buffer;
    unsigned char		c_text[ 1 ];
} lex_buffer_type;


static lex_buffer_type *get_lex_buffer  _ARGUMENTS(( lex_buffer_type *az_current_lex_buffer ));
#if debug_version
static void dump_token  _ARGUMENTS(( lex_buffer_type *az_current_lex_buffer , int l_lex_pos ));
#endif

#ifdef I18N_MULTIBYTE
#define total_no_of_leading_codes 3
#define total_no_of_encoded_charset 3
#define total_no_of_composite_charset 5
#endif

#ifdef I18N_MULTIBYTE
static int charset_len _ARGUMENTS(( char * l_code, int tag ));
static int cset_in_composite_cset _ARGUMENTS(( char * l_code, int tag ));
static int Is_lcode _ARGUMENTS((char *ptr));
static int Is_lcode_valid _ARGUMENTS((int lcode_entry));
static int Is_all_lcode_counter_zero();
static int lcode_count _ARGUMENTS((int lcode_entry));
static char *lcode_ptr _ARGUMENTS((int lcode_entry));
static void increment_lcode_counter _ARGUMENTS((int lcode_entry));
static int lcode_len _ARGUMENTS((int lcode_entry));
static void Reset_lcode_counter();
#endif /* I18N_MULTIBYTE */



/*
**
**  EXTERNAL DEFINITIONS
**
*/


/*    These values are set by Lex initiailize routine or if the charset	    */
/*    option is specified on the module declaration			    */
externaldef(uil_comp_glbl) int			Uil_lex_l_user_default_charset;
externaldef(uil_comp_glbl) sym_value_entry_type *Uil_lex_az_charset_entry;
externaldef(uil_comp_glbl) int 			Uil_lex_l_localized;
     

/*  This is the most recent character set recognized by the grammar.  It    */
/*  allows the use of the CHARACTER_SET function to prefix general string   */
/*  literals.								    */
externaldef(uil_comp_glbl) int			 Uil_lex_l_charset_specified;
externaldef(uil_comp_glbl) int			 Uil_lex_l_literal_charset;
externaldef(uil_comp_glbl) sym_value_entry_type *Uil_lex_az_literal_charset;
/* %COMPLETE */
externaldef(uil_comp_glbl) int Uil_characters_read;

/*    For portability, declare all yy* variables as extern.    */

extern yystype			yylval;
extern yystype			prev_yylval;

/*    This stack entry is used as the result of epsilon productions.   */

extern yystype			gz_yynullval;


/*
 * Retain comments as scanned
 */
#define INITIAL_COMMENT_SIZE 8000
#define INCR_COMMENT_SIZE 2000


externaldef(uil_comp_glbl) char *comment_text;
externaldef(uil_comp_glbl) int comment_size;

static int last_token_seen = 0;



/*
**
**  MACRO DEFINITIONS
**
**/

/* 
**  Each of the 256 possible input characters has an associated class.
**  These defines gives names to the classes.
*/

#define	    class_blank	    0		/* white space */
#define     class_punc	    1		/* punctuation characters */
#define     class_name	    2		/* alphabetics - {eEntvbrf} + {$ _} */
#define     class_e	    3		/* e */
#define     class_escape    4		/* n t v b r f */
#define     class_ff	    5		/* form feed */
#define     class_digit	    6		/* 0..9 */
#define     class_dot	    7		/* . */
#define	    class_sign	    8		/* - + */
#define     class_quote	    9		/* ' */
#define     class_bslash    10		/* \ */
#define     class_slash	    11		/* / */
#define     class_star	    12		/* * */
#define     class_exclam    13		/* ! */
#define     class_eol	    14		/* end of line - ascii nul */
#define     class_rest	    15		/* remaining printable characters */
#define     class_illegal   16		/* remaining non printable characters */
#define     class_pound	    17		/* # */
#define     class_dquote    18		/* " */
#define	    class_langle    19		/* < */
#define	    class_rangle    20		/* > */
#define     class_highbit   21		/* high order bit set */
#define     max_class	    21		

/*
**   states within the token table
*/

#define	    state_initial   0		/* initial state of automaton */
#define	    state_name	    1		/* looking for a name */
#define	    state_integer   2		/* looking for a integer */
#define	    state_real_1    3		/* looking for a real */
#define	    state_real_2    4		/* looking for a real */
#define	    state_exp_1	    5		/* looking for a real with exponent */
#define	    state_exp_2	    6		/* looking for a real with exponent */
#define	    state_exp_3	    7		/* looking for a real with exponent */
#define	    state_str_1     8		/* looking for a string */
#define	    state_str_2     9		/* looking for a string */
#define	    state_str_3     10		/* looking for a string */
#define	    state_comment_1 11		/* looking for a comment */
#define	    state_comment_2 12		/* looking for a comment */
#define	    state_comment_3 13		/* looking for a comment */
#define	    state_comment_4 14		/* looking for a comment */
#define	    state_eat_rest  15		/* eat up rest characters */
#define	    state_gstr_1    16		/* looking for a general string */
#define	    state_gstr_2    17		/* looking for a general string */
#define	    state_gstr_3    18		/* looking for a general string */
#define	    state_gstr_4    19		/* looking for a general string */
#define	    state_gstr_5    20		/* looking for a general string */
#define	    state_langle    21		/* looking for << */
#define	    state_rangle    22		/* looking for >> */
#define	    max_state	    22

/*
**   actions in token table
**       all actions requiring a move of the current character have
**       a negative value
*/

#define	    NEGATIVE 0x40

#define     min_action	    1
#define	    move_advance    (NEGATIVE | 1)	/* save character - goto next state */
#define	    advance	    1		/* goto next state */
#define     move_final      (NEGATIVE | 2)	/* save character - found token */
#define     final           2		/* found token */
#define     move_error      (NEGATIVE | 3)	/* save character - found error */
#define     error           3		/* found error */
#define	    move_special    (NEGATIVE | 4)		/* special action */
#define	    special	    4		/* save character - special action */
#define	    reset	    5		/* reset the analyzer */
#define     final_comment   (NEGATIVE | 6)       /* save whole comment  RAP */

/*
**  errors encoded in the token table
*/

#define	    min_error	    0		
#define	    bad_prefix      0		/* junk starts a token */
#define	    error_max       0

/*
**  final states in the token table
*/

#define     token_min		1
#define	    token_punc		1	/* punctuation character */
#define     token_eol		2	/* end of line */
#define     token_real		3	/* real number */
#define     token_integer	4	/* integer */
#define     token_name		5	/* identifier */
#define     token_ustring	6	/* unterminated string */
#define     token_string	7	/* string */
#define     token_ff		8	/* form feed */
#define     token_ugstr		9	/* unterminated general string */
#define     token_gstr		10	/* general string */
#define     token_punc2		11	/* 2 character punctuation */
#define     token_comment       12      /* comment block RAP */
#define	    token_lstr		13	/* localized general string */
#define     token_max		13


/*
**  special actions that take place
*/

#define	    control_char    1		/* unprintable character in construct */
#define	    start_bslash    2		/* start of \ construct in string */
#define	    ignore_bslash   3		/* \ not followed by correct sequence */
#define	    found_digit	    4		/* digit in \ sequence */
#define	    end_digits	    5		/* end of \digit...\ sequence */
#define	    insert_char	    6		/* end of \x sequence */
#define	    missing_bslash  7		/* \ at end of \digit...\ missing */
#define	    string_wrap	    8		/* string wraps to next line */
#define	    comment_wrap    9		/* comment wraps to next line */
#define	    charset_gstr    10		/* found char set for general string */
#define	    nocharset_gstr  11		/* default char set for general string */
#define	    highbit_char    12		/* default char set for general string */

/*
**  define backup field values of a token table cell
*/

#define	    not_used	    0		/* value not defined for this action */
#define	    backup_0	    0		/* no backup required */
#define	    backup_1	    1		/* 1 character backup needed */
#define	    backup_2	    2		/* 2 character backup needed */
#define	    backup_3	    3		/* 3 character backup needed */

/* 
**  form of each cell in the token table
*/


typedef struct
{
    char	action;			    /* action to be taken */
    char        next_state;		    /* next state */
					    /* terminal found */
					    /* error found */
    char	backup;			    /* number of character to backup */
					    /* action for special cases */
    char	unused;
} cell;

/*
**
**  OWN Storage for TOKEN BUILDER
**
*/

#define l_max_lex_buffer_pos  127

static	lex_buffer_type  *az_first_lex_buffer;

/*
**  the actual token table
*/

static cell XmConst token_table[ max_state+1][ max_class+1] =
  { 
    { /* state_initial */
	/* class_blank   */ { reset,		state_initial,	    not_used },
	/* class_punc    */ { move_final,	token_punc,	    backup_0 },
	/* class_name    */ { move_advance,	state_name,	    not_used },
	/* class_e       */ { move_advance,	state_name,	    not_used },
	/* class_escape  */ { move_advance,	state_name,	    not_used },
	/* class_ff      */ { final,		token_ff,	    backup_0 },
	/* class_digit   */ { move_advance,	state_integer,	    not_used },
	/* class_dot     */ { move_advance,	state_real_1,	    not_used },
	/* class_sign    */ { move_final,	token_punc,	    backup_0 },
	/* class_quote   */ { advance,		state_str_1,	    not_used },
	/* class_bslash  */ { move_advance,	state_eat_rest,	    not_used },
	/* class_slash   */ { move_advance,	state_comment_1,    not_used },
	/* class_star    */ { move_final,  	token_punc,    	    backup_0 },
	/* class_exclam  */ { move_advance,	state_comment_4,    not_used },
	/* class_eol     */ { final,		token_eol,	    backup_0 },
	/* class_rest    */ { move_advance,	state_eat_rest,	    not_used },
	/* class_illegal */ { special,		state_initial,	control_char },
	/* class_pound	 */ { move_final,	token_punc,	    backup_0 },
	/* class_dquote  */ { special,		state_gstr_3, nocharset_gstr },
	/* class_langle  */ { move_advance,	state_langle,       not_used },
	/* class_rangle  */ { move_advance,	state_rangle,       not_used },
	/* class_highbit */ { move_advance,	state_eat_rest,	    not_used },
    },
    { /* state_name */
	/* class_blank   */ { final,		token_name,	    backup_0 },
	/* class_punc    */ { move_final,	token_name,	    backup_1 },
	/* class_name    */ { move_advance,	state_name,	    not_used },
	/* class_e       */ { move_advance,	state_name,	    not_used },
	/* class_escape  */ { move_advance,	state_name,	    not_used },
	/* class_ff      */ { move_final,	token_name,	    backup_1 },
	/* class_digit   */ { move_advance,	state_name,	    not_used },
	/* class_dot     */ { move_final,	token_name,	    backup_1 },
	/* class_sign    */ { move_final,	token_name,	    backup_1 },
	/* class_quote   */ { move_final,	token_name,	    backup_1 },
	/* class_bslash  */ { move_final,	token_name,	    backup_1 },
	/* class_slash   */ { move_final,	token_name,	    backup_1 },
	/* class_star    */ { move_final,	token_name,	    backup_1 },
	/* class_exclam  */ { move_final,	token_name,	    backup_1 },
	/* class_eol     */ { move_final,	token_name,	    backup_1 },
	/* class_rest    */ { move_final,	token_name,	    backup_1 },
	/* class_illegal */ { move_final,	token_name,	    backup_1 },
	/* class_pound   */ { move_final,	token_name,	    backup_1 },
	/* class_dquote  */ { move_final,	token_name,	    backup_1 },
	/* class_langle  */ { move_final,  	token_name,         backup_1 },
	/* class_rangle  */ { move_final,  	token_name,         backup_1 },
	/* class_highbit */ { move_final,	token_name,	    backup_1 },
    },
    { /* state_integer */
	/* class_blank   */ { final,		token_integer,	    backup_0 },
	/* class_punc    */ { move_final,	token_integer,	    backup_1 },
	/* class_name    */ { move_final,	token_integer,	    backup_1 },
	/* class_e       */ { move_final,	token_integer,	    backup_1 },
	/* class_escape  */ { move_final,	token_integer,	    backup_1 },
	/* class_ff      */ { move_final,	token_integer,	    backup_1 },
	/* class_digit   */ { move_advance,	state_integer,	    not_used },
	/* class_dot     */ { move_advance,	state_real_2,	    not_used },
	/* class_sign    */ { move_final,	token_integer,	    backup_1 },
	/* class_quote   */ { move_final,	token_integer,	    backup_1 },
	/* class_bslash  */ { move_final,	token_integer,	    backup_1 },
	/* class_slash   */ { move_final,	token_integer,	    backup_1 },
	/* class_star    */ { move_final,	token_integer,	    backup_1 },
	/* class_exclam  */ { move_final,	token_integer,	    backup_1 },
	/* class_eol     */ { move_final,	token_integer,	    backup_1 },
	/* class_rest    */ { move_final,	token_integer,	    backup_1 },
	/* class_illegal */ { move_final,	token_integer,	    backup_1 },
	/* class_pound   */ { move_final,	token_integer,	    backup_1 },
	/* class_dquote  */ { move_final,	token_integer,	    backup_1 },
	/* class_langle  */ { move_final,	token_integer,	    backup_1 },
	/* class_rangle  */ { move_final,	token_integer,	    backup_1 },
	/* class_highbit */ { move_final,	token_integer,	    backup_1 },
    },
    { /* state_real_1 */
	/* class_blank   */ { move_advance,	state_eat_rest,	    not_used },
	/* class_punc    */ { move_error,	bad_prefix,	    backup_1 },
	/* class_name    */ { move_error,	bad_prefix,	    backup_1 },
	/* class_e       */ { move_error,	bad_prefix,	    backup_1 },
	/* class_escape  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_ff      */ { special,		state_real_1,	control_char },
	/* class_digit   */ { move_advance,	state_real_2,	    not_used },
	/* class_dot     */ { move_error,	bad_prefix,	    backup_1 },
	/* class_sign    */ { move_error,	bad_prefix,	    backup_1 },
	/* class_quote   */ { move_error,	bad_prefix,	    backup_1 },
	/* class_bslash  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_slash   */ { move_error,	bad_prefix,	    backup_1 },
	/* class_star    */ { move_error,	bad_prefix,	    backup_1 },
	/* class_exclam  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_eol     */ { move_error,	bad_prefix,	    backup_1 },
	/* class_rest    */ { move_advance,	state_eat_rest,	    not_used },
	/* class_illegal */ { special,		state_real_1,	control_char },
	/* class_pound	 */ { move_error,	bad_prefix,	    backup_1 },
	/* class_dquote  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_langle  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_rangle  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_highbit */ { move_advance,	state_eat_rest,	    not_used },
    },
    { /* state_real_2 */
	/* class_blank   */ { final,		token_real,	    backup_0 },
	/* class_punc    */ { move_final,	token_real,	    backup_1 },
	/* class_name    */ { move_final,	token_real,	    backup_1 },
	/* class_e       */ { move_advance,	state_exp_1,	    not_used },
	/* class_escape  */ { move_final,	token_real,	    backup_1 },
	/* class_ff      */ { move_final,	token_real,	    backup_1 },
	/* class_digit   */ { move_advance,	state_real_2,	    not_used },
	/* class_dot     */ { move_final,	token_real,	    backup_1 },
	/* class_sign    */ { move_final,	token_real,	    backup_1 },
	/* class_quote   */ { move_final,	token_real,	    backup_1 },
	/* class_bslash  */ { move_final,	token_real,	    backup_1 },
	/* class_slash   */ { move_final,	token_real,	    backup_1 },
	/* class_star    */ { move_final,	token_real,	    backup_1 },
	/* class_exclam  */ { move_final,	token_real,	    backup_1 },
	/* class_eol     */ { move_final,	token_real,	    backup_1 },
	/* class_rest    */ { move_final,	token_real,	    backup_1 },
	/* class_illegal */ { move_final,	token_real,	    backup_1 },
	/* class_pound   */ { move_final,	token_real,	    backup_1 },
	/* class_dquote  */ { move_final,	token_real,	    backup_1 },
	/* class_langle  */ { move_final,	token_real,	    backup_1 },
	/* class_rangle  */ { move_final,	token_real,	    backup_1 },
	/* class_highbit */ { move_final,	token_real,	    backup_1 },
    },
    { /* state_exp_1 */
	/* class_blank   */ { move_final,	token_real,	    backup_2 },
	/* class_punc    */ { move_final,	token_real,	    backup_2 },
	/* class_name    */ { move_final,	token_real,	    backup_2 },
	/* class_e       */ { move_final,	token_real,	    backup_2 },
	/* class_escape  */ { move_final,	token_real,	    backup_2 },
	/* class_ff      */ { move_final,	token_real,	    backup_2 },
	/* class_digit   */ { move_advance,	state_exp_3,	    not_used },
	/* class_dot     */ { move_final,	token_real,	    backup_2 },
	/* class_sign    */ { move_advance,	state_exp_2,	    not_used },
	/* class_quote   */ { move_final,	token_real,	    backup_2 },
	/* class_bslash  */ { move_final,	token_real,	    backup_2 },
	/* class_slash   */ { move_final,	token_real,	    backup_2 },
	/* class_star    */ { move_final,	token_real,	    backup_2 },
	/* class_exclam  */ { move_final,	token_real,	    backup_2 },
	/* class_eol     */ { move_final,	token_real,	    backup_2 },
	/* class_rest    */ { move_final,	token_real,	    backup_2 },
	/* class_illegal */ { move_final,	token_real,	    backup_2 },
	/* class_pound   */ { move_final,	token_real,	    backup_2 },
	/* class_dquote  */ { move_final,	token_real,	    backup_2 },
	/* class_langle  */ { move_final,	token_real,	    backup_2 },
	/* class_rangle  */ { move_final,	token_real,	    backup_2 },
	/* class_highbit */ { move_final,	token_real,	    backup_2 },
    },
    { /* state_exp_2 */
	/* class_blank   */ { move_final,	token_real,	    backup_3 },
	/* class_punc    */ { move_final,	token_real,	    backup_3 },
	/* class_name    */ { move_final,	token_real,	    backup_3 },
	/* class_e       */ { move_final,	token_real,	    backup_3 },
	/* class_escape  */ { move_final,	token_real,	    backup_3 },
	/* class_ff      */ { move_final,	token_real,	    backup_3 },
	/* class_digit   */ { move_advance,	state_exp_3,	    not_used },
	/* class_dot     */ { move_final,	token_real,	    backup_3 },
	/* class_sign    */ { move_final,	token_real,	    backup_3 },
	/* class_quote   */ { move_final,	token_real,	    backup_3 },
	/* class_bslash  */ { move_final,	token_real,	    backup_3 },
	/* class_slash   */ { move_final,	token_real,	    backup_3 },
	/* class_star    */ { move_final,	token_real,	    backup_3 },
	/* class_exclam  */ { move_final,	token_real,	    backup_3 },
	/* class_eol     */ { move_final,	token_real,	    backup_3 },
	/* class_rest    */ { move_final,	token_real,	    backup_3 },
	/* class_illegal */ { move_final,	token_real,	    backup_3 },
	/* class_pound   */ { move_final,	token_real,	    backup_3 },
	/* class_dquote  */ { move_final,	token_real,	    backup_3 },
	/* class_langle  */ { move_final,	token_real,	    backup_3 },
	/* class_rangle  */ { move_final,	token_real,	    backup_3 },
	/* class_highbit */ { move_final,	token_real,	    backup_3 },
    },
    { /* state_exp_3 */
	/* class_blank   */ { final,		token_real,	    backup_0 },
	/* class_punc    */ { move_final,	token_real,	    backup_1 },
	/* class_name    */ { move_final,	token_real,	    backup_1 },
	/* class_e       */ { move_final,	token_real,	    backup_1 },
	/* class_escape  */ { move_final,	token_real,	    backup_1 },
	/* class_ff      */ { move_final,	token_real,	    backup_1 },
	/* class_digit   */ { move_advance,	state_exp_3,	    not_used },
	/* class_dot     */ { move_final,	token_real,	    backup_1 },
	/* class_sign    */ { move_final,	token_real,	    backup_1 },
	/* class_quote   */ { move_final,	token_real,	    backup_1 },
	/* class_bslash  */ { move_final,	token_real,	    backup_1 },
	/* class_slash   */ { move_final,	token_real,	    backup_1 },
	/* class_star    */ { move_final,	token_real,	    backup_1 },
	/* class_exclam  */ { move_final,	token_real,	    backup_1 },
	/* class_eol     */ { move_final,	token_real,	    backup_1 },
	/* class_rest    */ { move_final,	token_real,	    backup_1 },
	/* class_illegal */ { move_final,	token_real,	    backup_1 },
	/* class_pound   */ { move_final,	token_real,	    backup_1 },
	/* class_dquote  */ { move_final,	token_real,	    backup_1 },
	/* class_langle  */ { move_final,	token_real,	    backup_1 },
	/* class_rangle  */ { move_final,	token_real,	    backup_1 },
	/* class_highbit */ { move_final,	token_real,	    backup_1 },
    },
    { /* state_str_1 */
	/* class_blank   */ { move_advance,	state_str_1,	    not_used },
	/* class_punc    */ { move_advance,	state_str_1,	    not_used },
	/* class_name    */ { move_advance,	state_str_1,	    not_used },
	/* class_e       */ { move_advance,	state_str_1,	    not_used },
	/* class_escape  */ { move_advance,	state_str_1,	    not_used },
	/* class_ff      */ { special,		state_str_1,	control_char },
	/* class_digit   */ { move_advance,	state_str_1,	    not_used },
	/* class_dot     */ { move_advance,	state_str_1,	    not_used },
	/* class_sign    */ { move_advance,	state_str_1,	    not_used },
	/* class_quote   */ { final,		token_string,	    backup_0 },
	/* class_bslash  */ { special,		state_str_2,	start_bslash },
	/* class_slash   */ { move_advance,	state_str_1,	    not_used },
	/* class_star    */ { move_advance,	state_str_1,	    not_used },
	/* class_exclam  */ { move_advance,	state_str_1,	    not_used },
	/* class_eol     */ { move_final,	token_ustring,	    backup_1 },
	/* class_rest    */ { move_advance,	state_str_1,	    not_used },
	/* class_illegal */ { special,		state_str_1,	control_char },
	/* class_pound   */ { move_advance,	state_str_1,	    not_used },
	/* class_dquote  */ { move_advance,	state_str_1,	    not_used },
	/* class_langle  */ { move_advance,	state_str_1,	    not_used },
	/* class_rangle  */ { move_advance,	state_str_1,	    not_used },
	/* class_highbit */ { move_advance,	state_str_1,	    not_used },
    },
    { /* state_str_2 */
	/* class_blank   */ { move_special,	state_str_1,   ignore_bslash },
	/* class_punc    */ { move_special,	state_str_1,   ignore_bslash },
	/* class_name    */ { move_special,	state_str_1,   ignore_bslash },
	/* class_e       */ { move_special,	state_str_1,   ignore_bslash },
	/* class_escape  */ { special,		state_str_1,     insert_char },
	/* class_ff      */ { special,		state_str_1,    control_char },
	/* class_digit   */ { special,		state_str_3,	 found_digit },
	/* class_dot     */ { move_special,	state_str_1,   ignore_bslash },
	/* class_sign    */ { move_special,	state_str_1,   ignore_bslash },
	/* class_quote   */ { special,		state_str_1,     insert_char },
	/* class_bslash  */ { special,		state_str_1,     insert_char },
	/* class_slash   */ { move_special,	state_str_1,   ignore_bslash },
	/* class_star    */ { move_special,	state_str_1,   ignore_bslash },
	/* class_exclam  */ { move_special,	state_str_1,   ignore_bslash },
	/* class_eol     */ { special,		state_str_1,     string_wrap },
	/* class_rest    */ { move_special,	state_str_1,   ignore_bslash },
	/* class_illegal */ { special,		state_str_1,    control_char },
	/* class_pound   */ { move_special,	state_str_1,   ignore_bslash },
	/* class_dquote  */ { special,		state_str_1,     insert_char },
	/* class_langle  */ { move_special,	state_str_1,   ignore_bslash },
	/* class_rangle  */ { move_special,	state_str_1,   ignore_bslash },
	/* class_highbit */ { move_special,	state_str_1,   ignore_bslash },
    },
    { /* state_str_3 */
	/* class_blank   */ { move_special,	state_str_1,  missing_bslash },
	/* class_punc    */ { move_special,	state_str_1,  missing_bslash },
	/* class_name    */ { move_special,	state_str_1,  missing_bslash },
	/* class_e       */ { move_special,	state_str_1,  missing_bslash },
	/* class_escape  */ { move_special,	state_str_1,  missing_bslash },
	/* class_ff      */ { special,		state_str_1,    control_char },
	/* class_digit   */ { special,		state_str_3,	 found_digit },
	/* class_dot     */ { move_special,	state_str_1,  missing_bslash },
	/* class_sign    */ { move_special,	state_str_1,  missing_bslash },
	/* class_quote   */ { move_special,	state_str_1,  missing_bslash },
	/* class_bslash  */ { special,		state_str_1,	  end_digits },
	/* class_slash   */ { move_special,	state_str_1,  missing_bslash },
	/* class_star    */ { move_special,	state_str_1,  missing_bslash },
	/* class_exclam  */ { move_special,	state_str_1,  missing_bslash },
	/* class_eol     */ { move_final,       token_ustring,	    backup_1 },
	/* class_rest    */ { move_special,	state_str_1,  missing_bslash },
	/* class_illegal */ { special,		state_str_3,    control_char },
	/* class_pound   */ { move_special,	state_str_1,  missing_bslash },
	/* class_dquote  */ { move_special,	state_str_1,  missing_bslash },
	/* class_langle  */ { move_special,	state_str_1,  missing_bslash },
	/* class_rangle  */ { move_special,	state_str_1,  missing_bslash },
	/* class_highbit */ { move_special,	state_str_1,  missing_bslash },
    },
    { /* state_comment_1 */ 
	/* class_blank   */ { move_final,	token_punc,	    backup_0 },
	/* class_punc    */ { move_final,	token_punc,	    backup_1 },
	/* class_name    */ { move_final,	token_punc,	    backup_1 },
	/* class_e       */ { move_final,	token_punc,	    backup_1 },
	/* class_escape  */ { move_final,	token_punc,	    backup_1 },
	/* class_ff      */ { special,		state_comment_1,control_char },
	/* class_digit   */ { move_final,	token_punc,	    backup_1 },
	/* class_dot     */ { move_final,	token_punc,	    backup_1 },
	/* class_sign    */ { move_final,	token_punc,	    backup_1 },
	/* class_quote   */ { move_final,	token_punc,	    backup_1 },
	/* class_bslash  */ { move_final,	token_punc,	    backup_1 },
	/* class_slash   */ { move_final,	token_punc,	    backup_1 },
	/* class_star    */ { move_advance,	state_comment_2,    not_used },
	/* class_exclam  */ { move_final,	token_punc,	    backup_1 },
	/* class_eol     */ { move_final,	token_punc,	    backup_1 },
	/* class_rest    */ { move_final,  	token_punc,    	    backup_1 },
	/* class_illegal */ { special,		state_comment_1,control_char },
	/* class_pound	 */ { move_final,	token_punc,	    backup_1 },
	/* class_dquote  */ { move_final,	token_punc,	    backup_1 },
	/* class_langle  */ { move_final,	token_punc,	    backup_1 },
	/* class_rangle  */ { move_final,	token_punc,	    backup_1 },
	/* class_highbit */ { move_final,  	token_punc,    	    backup_1 },
    },
    { /* state_comment_2 */ 
	/* class_blank   */ { move_advance,	state_comment_2,    not_used },
	/* class_punc    */ { move_advance,	state_comment_2,    not_used },
	/* class_name    */ { move_advance,	state_comment_2,    not_used },
	/* class_e       */ { move_advance,	state_comment_2,    not_used },
	/* class_escape  */ { move_advance,	state_comment_2,    not_used },
	/* class_ff      */ { move_special,	state_comment_2,control_char },
	/* class_digit   */ { move_advance,	state_comment_2,    not_used },
	/* class_dot     */ { move_advance,	state_comment_2,    not_used },
	/* class_sign    */ { move_advance,	state_comment_2,    not_used },
	/* class_quote   */ { move_advance,	state_comment_2,    not_used },
	/* class_bslash  */ { move_advance,	state_comment_2,    not_used },
	/* class_slash   */ { move_advance,	state_comment_2,    not_used },
	/* class_star    */ { move_advance,	state_comment_3,    not_used },
	/* class_exclam  */ { move_advance,	state_comment_2,    not_used },
	/* class_eol     */ { move_special,	state_comment_2,comment_wrap },
	/* class_rest    */ { move_advance,	state_comment_2,    not_used },
	/* class_illegal */ { move_special,	state_comment_2,control_char },
	/* class_pound   */ { move_advance,	state_comment_2,    not_used },
	/* class_dquote  */ { move_advance,	state_comment_2,    not_used },
	/* class_langle  */ { move_advance,	state_comment_2,    not_used },
	/* class_rangle  */ { move_advance,	state_comment_2,    not_used },
	/* class_highbit */ { move_advance,	state_comment_2,    not_used },
    },
    { /* state_comment_3 */ 
	/* class_blank   */ { move_advance,	state_comment_2,    not_used },
	/* class_punc    */ { move_advance,	state_comment_2,    not_used },
	/* class_name    */ { move_advance,	state_comment_2,    not_used },
	/* class_e       */ { move_advance,	state_comment_2,    not_used },
	/* class_escape  */ { move_advance,	state_comment_2,    not_used },
	/* class_ff      */ { move_special,	state_comment_2,control_char },
	/* class_digit   */ { move_advance,	state_comment_2,    not_used },
	/* class_dot     */ { move_advance,	state_comment_2,    not_used },
	/* class_sign    */ { move_advance,	state_comment_2,    not_used },
	/* class_quote   */ { move_advance,	state_comment_2,    not_used },
	/* class_bslash  */ { move_advance,	state_comment_2,    not_used },
	/* class_slash   */ { final_comment,	token_comment,      not_used },
	/* class_star    */ { move_advance,	state_comment_3,    not_used },
	/* class_exclam  */ { move_advance,	state_comment_2,    not_used },
	/* class_eol     */ { move_special,	state_comment_2,comment_wrap },
	/* class_rest    */ { move_advance,	state_comment_2,    not_used },
	/* class_illegal */ { move_special,	state_comment_2,control_char },
	/* class_pound   */ { move_advance,	state_comment_2,    not_used },
	/* class_dquote  */ { move_advance,	state_comment_2,    not_used },
	/* class_langle  */ { move_advance,	state_comment_2,    not_used },
	/* class_rangle  */ { move_advance,	state_comment_2,    not_used },
	/* class_highbit */ { move_advance,	state_comment_2,    not_used },
    },
    { /* state_comment_4 */ 
	/* class_blank   */ { move_advance,	state_comment_4,    not_used },
	/* class_punc    */ { move_advance,	state_comment_4,    not_used },
	/* class_name    */ { move_advance,	state_comment_4,    not_used },
	/* class_e       */ { move_advance,	state_comment_4,    not_used },
	/* class_escape  */ { move_advance,	state_comment_4,    not_used },
	/* class_ff      */ { move_special,	state_comment_4,control_char },
	/* class_digit   */ { move_advance,	state_comment_4,    not_used },
	/* class_dot     */ { move_advance,	state_comment_4,    not_used },
	/* class_sign    */ { move_advance,	state_comment_4,    not_used },
	/* class_quote   */ { move_advance,	state_comment_4,    not_used },
	/* class_bslash  */ { move_advance,	state_comment_4,    not_used },
	/* class_slash   */ { move_advance,	state_comment_4,    not_used },
	/* class_star    */ { move_advance,	state_comment_4,    not_used },
	/* class_exclam  */ { move_advance,	state_comment_4,    not_used },
	/* class_eol     */ { final_comment,	token_comment,	    backup_1 },
	/* class_rest    */ { move_advance,	state_comment_4,    not_used },
	/* class_illegal */ { move_special,	state_comment_4,control_char },
	/* class_pound   */ { move_advance,	state_comment_4,    not_used },
	/* class_dquote  */ { move_advance,	state_comment_4,    not_used },
	/* class_langle  */ { move_advance,	state_comment_4,    not_used },
	/* class_rangle  */ { move_advance,	state_comment_4,    not_used },
	/* class_highbit */ { move_advance,	state_comment_4,    not_used },
    },
    { /* state_eat_rest */ 
	/* class_blank   */ { move_advance,	state_eat_rest,     not_used },
	/* class_punc    */ { move_error,	bad_prefix,	    backup_1 },
	/* class_name    */ { move_error,	bad_prefix,	    backup_1 },
	/* class_e       */ { move_error,	bad_prefix,	    backup_1 },
	/* class_escape  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_ff      */ { special,		state_eat_rest, control_char },
	/* class_digit   */ { move_error,	bad_prefix,	    backup_1 },
	/* class_dot     */ { move_error,	bad_prefix,	    backup_1 },
	/* class_sign    */ { move_error,	bad_prefix,	    backup_1 },
	/* class_quote   */ { move_error,	bad_prefix,	    backup_1 },
	/* class_bslash  */ { move_advance,	state_eat_rest,     not_used },
	/* class_slash   */ { move_error,	bad_prefix,	    backup_1 },
	/* class_star    */ { move_advance,	state_eat_rest,     not_used },
	/* class_exclam  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_eol     */ { move_error,	bad_prefix,	    backup_1 },
	/* class_rest    */ { move_advance,	state_eat_rest,     not_used },
	/* class_illegal */ { special,		state_eat_rest, control_char },
	/* class_pound   */ { move_error,	bad_prefix,	    backup_1 },
	/* class_dquote  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_langle  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_rangle  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_highbit */ { move_advance,	state_eat_rest,     not_used },
    },
    { /* state_gstr_1 */ 
	/* class_blank   */ { move_error,	bad_prefix,         backup_1 },
	/* class_punc    */ { move_error,	bad_prefix,	    backup_1 },
	/* class_name    */ { move_advance,	state_gstr_2,	    not_used },
	/* class_e       */ { move_advance,	state_gstr_2,	    not_used },
	/* class_escape  */ { move_advance,	state_gstr_2,	    not_used },
	/* class_ff      */ { special,		state_gstr_1,   control_char },
	/* class_digit   */ { move_error,	bad_prefix,	    backup_1 },
	/* class_dot     */ { move_error,	bad_prefix,	    backup_1 },
	/* class_sign    */ { move_error,	bad_prefix,	    backup_1 },
	/* class_quote   */ { move_error,	bad_prefix,	    backup_1 },
	/* class_bslash  */ { move_advance,	state_eat_rest,     not_used },
	/* class_slash   */ { move_error,	bad_prefix,	    backup_1 },
	/* class_star    */ { move_advance,	state_eat_rest,     not_used },
	/* class_exclam  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_eol     */ { move_error,	bad_prefix,	    backup_1 },
	/* class_rest    */ { move_advance,	state_eat_rest,     not_used },
	/* class_illegal */ { special,		state_eat_rest, control_char },
	/* class_pound   */ { move_error,	bad_prefix,	    backup_1 },
	/* class_dquote  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_langle  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_rangle  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_highbit */ { move_advance,	state_eat_rest,     not_used },
    },
    { /* state_gstr_2 */ 
	/* class_blank   */ { move_error,	bad_prefix,         backup_1 },
	/* class_punc    */ { move_error,	bad_prefix,	    backup_1 },
	/* class_name    */ { move_advance,	state_gstr_2,	    not_used },
	/* class_e       */ { move_advance,	state_gstr_2,	    not_used },
	/* class_escape  */ { move_advance,	state_gstr_2,	    not_used },
	/* class_ff      */ { special,		state_gstr_2,   control_char },
	/* class_digit   */ { move_advance,	state_gstr_2,	    not_used },
	/* class_dot     */ { move_error,	bad_prefix,	    backup_1 },
	/* class_sign    */ { move_error,	bad_prefix,	    backup_1 },
	/* class_quote   */ { move_error,	bad_prefix,	    backup_1 },
	/* class_bslash  */ { move_advance,	state_eat_rest,     not_used },
	/* class_slash   */ { move_error,	bad_prefix,	    backup_1 },
	/* class_star    */ { move_advance,	state_eat_rest,     not_used },
	/* class_exclam  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_eol     */ { move_error,	bad_prefix,	    backup_1 },
	/* class_rest    */ { move_advance,	state_eat_rest,     not_used },
	/* class_illegal */ { special,		state_eat_rest, control_char },
	/* class_pound   */ { move_error,	bad_prefix,	    backup_1 },
	/* class_dquote  */ { special,		state_gstr_3,   charset_gstr },
	/* class_langle  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_rangle  */ { move_error,	bad_prefix,	    backup_1 },
	/* class_highbit */ { move_advance,	state_eat_rest,     not_used },
    },
    { /* state_gstr_3 */
	/* class_blank   */ { move_advance,	state_gstr_3,	    not_used },
	/* class_punc    */ { move_advance,	state_gstr_3,	    not_used },
	/* class_name    */ { move_advance,	state_gstr_3,	    not_used },
	/* class_e       */ { move_advance,	state_gstr_3,	    not_used },
	/* class_escape  */ { move_advance,	state_gstr_3,	    not_used },
	/* class_ff      */ { special,		state_gstr_3,	control_char },
	/* class_digit   */ { move_advance,	state_gstr_3,	    not_used },
	/* class_dot     */ { move_advance,	state_gstr_3,	    not_used },
	/* class_sign    */ { move_advance,	state_gstr_3,	    not_used },
	/* class_quote   */ { move_advance,	state_gstr_3,	    not_used },
	/* class_bslash  */ { special,		state_gstr_4,	start_bslash },
	/* class_slash   */ { move_advance,	state_gstr_3,	    not_used },
	/* class_star    */ { move_advance,	state_gstr_3,	    not_used },
	/* class_exclam  */ { move_advance,	state_gstr_3,	    not_used },
	/* class_eol     */ { move_final,	token_ugstr,	    backup_1 },
	/* class_rest    */ { move_advance,	state_gstr_3,	    not_used },
	/* class_illegal */ { special,		state_gstr_3,	control_char },
	/* class_pound   */ { move_advance,	state_gstr_3,	    not_used },
	/* class_dquote  */ { final,		token_gstr,	    backup_0 },
	/* class_langle  */ { move_advance,	state_gstr_3,	    not_used },
	/* class_rangle  */ { move_advance,	state_gstr_3,	    not_used },
	/* class_highbit */ { move_special,	state_gstr_3,	highbit_char },
    },
    { /* state_gstr_4 */
	/* class_blank   */ { move_special,	state_gstr_3,  ignore_bslash },
	/* class_punc    */ { move_special,	state_gstr_3,  ignore_bslash },
	/* class_name    */ { move_special,	state_gstr_3,  ignore_bslash },
	/* class_e       */ { move_special,	state_gstr_3,  ignore_bslash },
	/* class_escape  */ { special,		state_gstr_3,    insert_char },
	/* class_ff      */ { special,		state_gstr_3,   control_char },
	/* class_digit   */ { special,		state_gstr_5,	 found_digit },
	/* class_dot     */ { move_special,	state_gstr_3,  ignore_bslash },
	/* class_sign    */ { move_special,	state_gstr_3,  ignore_bslash },
	/* class_quote   */ { special,		state_gstr_3,    insert_char },
	/* class_bslash  */ { special,		state_gstr_3,    insert_char },
	/* class_slash   */ { move_special,	state_gstr_3,  ignore_bslash },
	/* class_star    */ { move_special,	state_gstr_3,  ignore_bslash },
	/* class_exclam  */ { move_special,	state_gstr_3,  ignore_bslash },
	/* class_eol     */ { move_final,	token_ugstr,	    backup_1 },
	/* class_rest    */ { move_special,	state_gstr_3,  ignore_bslash },
	/* class_illegal */ { special,		state_gstr_3,   control_char },
	/* class_pound   */ { move_special,	state_gstr_3,  ignore_bslash },
	/* class_dquote  */ { special,		state_gstr_3,    insert_char },
	/* class_langle  */ { move_special,	state_gstr_3,  ignore_bslash },
	/* class_rangle  */ { move_special,	state_gstr_3,  ignore_bslash },
	/* class_highbit */ { move_special,	state_gstr_3,  ignore_bslash },
    },
    { /* state_gstr_5 */
	/* class_blank   */ { move_special,	state_gstr_3, missing_bslash },
	/* class_punc    */ { move_special,	state_gstr_3, missing_bslash },
	/* class_name    */ { move_special,	state_gstr_3, missing_bslash },
	/* class_e       */ { move_special,	state_gstr_3, missing_bslash },
	/* class_escape  */ { move_special,	state_gstr_3, missing_bslash },
	/* class_ff      */ { special,		state_gstr_3,   control_char },
	/* class_digit   */ { special,		state_gstr_5,	 found_digit },
	/* class_dot     */ { move_special,	state_gstr_3, missing_bslash },
	/* class_sign    */ { move_special,	state_gstr_3, missing_bslash },
	/* class_quote   */ { move_special,	state_gstr_3, missing_bslash },
	/* class_bslash  */ { special,		state_gstr_3,	  end_digits },
	/* class_slash   */ { move_special,	state_gstr_3, missing_bslash },
	/* class_star    */ { move_special,	state_gstr_3, missing_bslash },
	/* class_exclam  */ { move_special,	state_gstr_3, missing_bslash },
	/* class_eol     */ { move_final,	token_ugstr,	    backup_1 },
	/* class_rest    */ { move_special,	state_gstr_3, missing_bslash },
	/* class_illegal */ { special,		state_str_3,    control_char },
	/* class_pound   */ { move_special,	state_gstr_3, missing_bslash },
	/* class_dquote  */ { move_special,	state_gstr_3, missing_bslash },
	/* class_langle  */ { move_special,	state_gstr_3, missing_bslash },
	/* class_rangle  */ { move_special,	state_gstr_3, missing_bslash },
	/* class_highbit */ { move_special,	state_gstr_3, missing_bslash },
    },
    { /* state_langle */
	/* class_blank   */ { move_error,  	bad_prefix,         backup_1 },
	/* class_punc    */ { move_error,  	bad_prefix,         backup_1 },
	/* class_name    */ { move_error,  	bad_prefix,         backup_1 },
	/* class_e       */ { move_error,  	bad_prefix,         backup_1 },
	/* class_escape  */ { move_error,  	bad_prefix,         backup_1 },
	/* class_ff      */ { special,		state_langle,   control_char },
	/* class_digit   */ { move_error,       bad_prefix, 	    backup_1 },
	/* class_dot     */ { move_error,  	bad_prefix,         backup_1 },
	/* class_sign    */ { move_error,  	bad_prefix,         backup_1 },
	/* class_quote   */ { move_error,  	bad_prefix,         backup_1 },
	/* class_bslash  */ { move_error,       bad_prefix,  	    backup_1 },
	/* class_slash   */ { move_error,  	bad_prefix,         backup_1 },
	/* class_star    */ { move_error,  	bad_prefix,         backup_1 },
	/* class_exclam  */ { move_error,  	bad_prefix,         backup_1 },
	/* class_eol     */ { move_final,	token_ugstr,	    backup_1 },
	/* class_rest    */ { move_advance,  	state_eat_rest,     not_used },
	/* class_illegal */ { special,		state_langle,   control_char },
	/* class_pound   */ { move_error,  	bad_prefix,         backup_1 },
	/* class_dquote  */ { move_error,  	bad_prefix,         backup_1 },
	/* class_langle  */ { move_final,  	token_punc2,        backup_0 },
	/* class_rangle  */ { move_error,  	bad_prefix,         backup_1 },
	/* class_highbit */ { move_error,  	bad_prefix,         backup_1 },
    },
    { /* state_rangle */
	/* class_blank   */ { move_error,  	bad_prefix,         backup_1 },
	/* class_punc    */ { move_error,  	bad_prefix,         backup_1 },
	/* class_name    */ { move_error,  	bad_prefix,         backup_1 },
	/* class_e       */ { move_error,  	bad_prefix,         backup_1 },
	/* class_escape  */ { move_error,  	bad_prefix,         backup_1 },
	/* class_ff      */ { special,		state_langle,   control_char },
	/* class_digit   */ { move_error,       bad_prefix, 	    backup_1 },
	/* class_dot     */ { move_error,  	bad_prefix,         backup_1 },
	/* class_sign    */ { move_error,  	bad_prefix,         backup_1 },
	/* class_quote   */ { move_error,  	bad_prefix,         backup_1 },
	/* class_bslash  */ { move_error,       bad_prefix,  	    backup_1 },
	/* class_slash   */ { move_error,  	bad_prefix,         backup_1 },
	/* class_star    */ { move_error,  	bad_prefix,         backup_1 },
	/* class_exclam  */ { move_error,  	bad_prefix,         backup_1 },
	/* class_eol     */ { move_final,	token_ugstr,	    backup_1 },
	/* class_rest    */ { move_advance,  	state_eat_rest,     not_used },
	/* class_illegal */ { special,		state_langle,   control_char },
	/* class_pound   */ { move_error,  	bad_prefix,         backup_1 },
	/* class_dquote  */ { move_error,  	bad_prefix,         backup_1 },
	/* class_langle  */ { move_error,  	bad_prefix,         backup_1 },
	/* class_rangle  */ { move_final,  	token_punc2,        backup_0 },
	/* class_highbit */ { move_error,  	bad_prefix,         backup_1 },
    },
  };


static char class_table[ 256 ] =
  {
/* 00 */    class_eol,      class_illegal,  class_illegal,  class_illegal,
/* 04 */    class_illegal,  class_illegal,  class_illegal,  class_illegal,
/* 08 */    class_illegal,  class_blank,    class_illegal,  class_illegal,
/* 0C */    class_ff,	    class_illegal,  class_illegal,  class_illegal,
/* 10 */    class_illegal,  class_illegal,  class_illegal,  class_illegal,
/* 14 */    class_illegal,  class_illegal,  class_illegal,  class_illegal,
/* 18 */    class_illegal,  class_illegal,  class_illegal,  class_illegal,
/* 1C */    class_illegal,  class_illegal,  class_illegal,  class_illegal,
/* 20 */    class_blank,    class_exclam,   class_dquote,   class_pound,
/* 24 */    class_name,	    class_rest,     class_punc,	    class_quote,
/* 28 */    class_punc,	    class_punc,     class_star,     class_sign,
/* 2C */    class_punc,     class_sign,     class_dot,      class_slash,
/* 30 */    class_digit,    class_digit,    class_digit,    class_digit,
/* 34 */    class_digit,    class_digit,    class_digit,    class_digit,
/* 38 */    class_digit,    class_digit,    class_punc,     class_punc,
/* 3C */    class_langle,   class_punc,     class_rangle,   class_rest,
/* 40 */    class_rest,     class_name,     class_name,     class_name, 
/* 44 */    class_name,     class_e,	    class_name,     class_name, 
/* 48 */    class_name,     class_name,     class_name,     class_name, 
/* 4C */    class_name,     class_name,     class_name,	    class_name, 
/* 50 */    class_name,     class_name,     class_name,     class_name,
/* 54 */    class_name,     class_name,     class_name,     class_name, 
/* 58 */    class_name,     class_name,     class_name,     class_rest,
/* 5C */    class_bslash,   class_rest,     class_punc,     class_name,
/* 60 */    class_rest,     class_name,     class_escape,   class_name, 
/* 64 */    class_name,     class_e,        class_escape,   class_name, 
/* 68 */    class_name,     class_name,     class_name,     class_name, 
/* 6C */    class_name,     class_name,     class_escape,   class_name, 
/* 70 */    class_name,     class_name,     class_escape,   class_name,
/* 74 */    class_escape,   class_name,     class_escape,   class_name, 
/* 78 */    class_name,     class_name,     class_name,     class_punc,
/* 7C */    class_punc,	    class_punc,	    class_punc,     class_illegal,
/* 80 */    class_illegal,  class_illegal,  class_illegal,  class_illegal,
/* 84 */    class_illegal,  class_illegal,  class_illegal,  class_illegal,
/* 88 */    class_illegal,  class_illegal,  class_illegal,  class_illegal,
#ifdef I18N_MULTIBYTE
/* 8C */    class_illegal,  class_illegal,  class_highbit,  class_highbit,
#else
/* 8C */    class_illegal,  class_illegal,  class_illegal,  class_illegal,
#endif
/* 90 */    class_illegal,  class_illegal,  class_illegal,  class_illegal,
/* 94 */    class_illegal,  class_illegal,  class_illegal,  class_illegal,
/* 98 */    class_illegal,  class_illegal,  class_illegal,  class_illegal,
/* 9C */    class_illegal,  class_illegal,  class_illegal,  class_illegal,
/* A0 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* A4 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* A8 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* AC */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* B0 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* B4 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* B8 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* BC */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* C0 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* C4 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* C8 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* CC */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* D0 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* D4 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* D8 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* DC */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* E0 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* E4 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* E8 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* EC */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* F0 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* F4 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* F8 */    class_highbit,  class_highbit,  class_highbit,  class_highbit,
/* FC */    class_highbit,  class_highbit,  class_highbit,  class_highbit

  };


/*    Tables to correlate token numbers and ASCII values for
      punctuation characters.  Used by yylex and lex_issue_error.     */

#define tok_punc_token_num	17
static unsigned char	punc_char[tok_punc_token_num] = {
			'{', '}', '=', ';', '(', ')', ':', '+', '-', 
			',', '&', '~', '*', '/', '^', '|', '#'};
static int	punc_token[tok_punc_token_num] =
		    {	LEFT_BRACE,
			RIGHT_BRACE,
			EQUAL_SIGN,
			SEMICOLON,
			LEFT_PAREN,
			RIGHT_PAREN,
			COLON,
			PLUS,
			MINUS,
			COMMA,
			AND,
			NOT,
			MULTIPLY, 
			DIVIDE,
			XOR,
			OR,
			POUND };

/*    Tables to correlate token numbers and ASCII value pairs for
      punctuation characters.  Used by yylex and lex_issue_error.     */

static unsigned char	punc2_char[2] = {'<', '>'};
static int	punc2_token[2] =
		    {	LEFT_SHIFT,
			RIGHT_SHIFT };


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function returns the next token to be built in the UIL source
**	program being read by the compiler.
**
**  FORMAL PARAMETERS:
**
**      none
**
**  IMPLICIT INPUTS:
**
**      
**
**  IMPLICIT OUTPUTS:
**
**      
**
**  FUNCTION VALUE:
**
**      number of the token generated
**
**  SIDE EFFECTS:
**
**      
**
**
**--
**/
int	yylex()
{
    unsigned char c_char;	    /* current character */
    int		l_class;	    /* current character's class */
    int		l_state;	    /* current token_table state */
    int		l_lex_pos;	    /* next available position in c_lex_buffer*/
    cell	z_cell;		    /* local copy of current token_table state*/
    int		l_bslash_value;	    /* current value of \digit...\ construct */
    int		l_start_src_pos;    /* starting source position of a token */
    int		l_charset;	    /* character set for strings */
    int		l_write_direction;	    /* writing direction */
    int		l_parse_direction;	    /* parsing direction */
    int		l_charset_sixteen_bit = FALSE; /* true if charset is 16-bit */
    sym_value_entry_type
		*az_charset_entry;  /* value entry for the current charset */
    int		l_16bit_char_count = 0; /* for strings, count 16-bit chars */
    int		l_16bit_chars_only; /* True if the gstr we are */
				    /* processing only has 16-bit chars */
    src_source_record_type
		*az_start_src_record;/* starting source record of a token */
    lex_buffer_type  
		*az_current_lex_buffer; /* current lexical buffer */
    extern int errno;

	az_charset_entry = (sym_value_entry_type *) 0;

    /*
    **  Call the Status callback routine to report our progress.
    */
    /* %COMPLETE  (between 0-50) */
    Uil_percent_complete =
      CEIL((int)( .5 * ((float)Uil_characters_read/(float)Uil_file_size))*100, 50);
    if (Uil_cmd_z_command.status_cb != (Uil_continue_type(*)())NULL)
	diag_report_status();    


initialize_token_builder:

    /* initialize the lexical analyzer by
     *	    saving starting source position of the token
     *	    resetting the lexical buffer
     *	    putting the analyser in its initial state */

    az_start_src_record = src_az_current_source_record;
    l_start_src_pos = src_az_current_source_buffer->w_current_position;

    az_current_lex_buffer = az_first_lex_buffer;
    l_lex_pos = 0;
    l_charset = lex_k_default_charset;
    l_16bit_chars_only = FALSE;
    l_state = state_initial;
    
    /* start looking for the token */

continue_in_next_state:

    for (;;)
    {
						    /* get next input char */
						    /* advance source too */
	c_char = src_az_current_source_buffer->c_text
		    [ src_az_current_source_buffer->w_current_position++ ];

	/* %COMPLETE */
	Uil_characters_read++;

	l_class = class_table[ c_char ];	    /* determine its class */
	z_cell = token_table[ l_state][l_class ];   /* load state cell */

	/* pick up the next state, or terminal, or error */

	l_state = z_cell.next_state;		    

	/* l_state is negative for action states requiring the current 
	 * character be saved in the current lexical buffer */

	if (z_cell.action & NEGATIVE)
	{
#ifdef DEC_MOTIF_BUG_FIX
            if (l_lex_pos > (l_max_lex_buffer_pos - 1 ))
#else
	    if (l_lex_pos > l_max_lex_buffer_pos )
#endif
	    {
		az_current_lex_buffer = get_lex_buffer( az_current_lex_buffer );
	        l_lex_pos = 0;
	    }
	    az_current_lex_buffer->c_text[ l_lex_pos ] = c_char;
	    l_lex_pos++;
	}

	/* next step is based on action */

	switch (z_cell.action)
	{
	case move_advance:
	case advance:
	    if (l_16bit_chars_only) goto found_16bit_char;
	    continue;

	case reset:
	    
	    goto initialize_token_builder;

	case move_final: 
	case final:
        case final_comment:  /* RAP retain comments */

	    goto found_token;

	case move_error: 
	case error:

	    goto found_error;

	case move_special:
	case special:

	    goto special_processing;

	default:

	    _assert( FALSE, "unknown token_table action" );

	}
    }

    /* process special actions */

special_processing:

    switch (z_cell.backup)	/* backup holds special processing code */
    {
    case control_char:		/* encountered a control char in a string or
				 * comment - issue a diagnotic and continue */
	issue_control_char_diagnostic( c_char );
	break;

    case start_bslash:		/* start of a \ construct in a string */

	l_bslash_value = 0;	    /* initialize collection cell */
	break;

    case found_digit:		/* next digit in a \digit...\ sequence */

	if (l_bslash_value < 256 )  /* if still in range add in next digit */
	    l_bslash_value = l_bslash_value * 10 + ( c_char - 48 );
	break;

    case end_digits:		/* end of \digit...\ sequence */

	if (l_bslash_value >= 256 )  /* issue a diagnostic */
	{   diag_issue_diagnostic
		( d_out_range, 
		  src_az_current_source_record,
		  src_az_current_source_buffer->w_current_position - 1,
		  "\\digit...\\ sequence",
		  "0-255" );
	    l_bslash_value = lex_k_unprint_sub;
	}

	if (l_bslash_value == 0 )  /* issue a diagnostic */
	{   diag_issue_diagnostic
		( d_null,
		  src_az_current_source_record,
		  src_az_current_source_buffer->w_current_position - 1 );
	}

	if (l_lex_pos > l_max_lex_buffer_pos )
	{
	    az_current_lex_buffer = get_lex_buffer( az_current_lex_buffer );
	    l_lex_pos = 0;
	}
	az_current_lex_buffer->c_text[ l_lex_pos++ ] = l_bslash_value;
	break;

    case insert_char:		/* place special character in lex buffer */
    {
	
	static unsigned char c_bslash_char[10]  = {
		'\\', '\'', 'n', 't', 'v', 'b', 'r', 'f', '"'};
	static unsigned char ab_bslash_value[9] =
		 { 0x5C, 0x27, 0x0A, 0x09, 0x0B, 0x08, 0x0D, 0x0C, 0x22 };

	if (l_lex_pos > l_max_lex_buffer_pos )
	{
	    az_current_lex_buffer = get_lex_buffer( az_current_lex_buffer );
	    l_lex_pos = 0;
	}
	az_current_lex_buffer->c_text[ l_lex_pos++ ] = 
	    ab_bslash_value
		[ _index( c_char, c_bslash_char, sizeof( c_bslash_char )-1 )];
	break;
    }

    case missing_bslash:	/* \digit...\ sequence not terminated */

	diag_issue_diagnostic
		( d_unterm_seq, 
		  src_az_current_source_record,
		  src_az_current_source_buffer->w_current_position - 1,
		  "\\digit...\\ sequence",
		  "with \\" );
	break;

    case ignore_bslash:		/* \ not followed by valid character */

	diag_issue_diagnostic
		( d_backslash_ignored, 
		  src_az_current_source_record,
		  src_az_current_source_buffer->w_current_position - 1,
		  c_char );
	break;

    case string_wrap:

	if (src_get_source_line( ) == src_k_end_source)
	{
	    diag_issue_diagnostic
		    ( d_unterm_seq, 
		      src_az_current_source_record,
		      src_az_current_source_buffer->w_current_position - 1,
		      "character string",
		      "before end of source" );

	    src_az_current_source_buffer->w_current_position--;
	}

	break;

    case comment_wrap:

	if (src_get_source_line( ) == src_k_end_source)
	{
	    diag_issue_diagnostic
		    ( d_unterm_seq, 
		      src_az_current_source_record,
		      src_az_current_source_buffer->w_current_position - 1,
		      "comment",
		      "before end of source" );

	    src_az_current_source_buffer->w_current_position--;
	    return UILEOF;
	}
	az_current_lex_buffer->
	    c_text[strlen((char *)az_current_lex_buffer->c_text)] = '\n';
	break;

    case highbit_char:		/* check if must accept extra chars */
found_16bit_char:
    {
	unsigned char    next_char;
	

	/*
	**  If the current character set allows 16-bit characters, then
	**  process them specially.
	*/
#ifdef I18N_MULTIBYTE
        int    char_size;
        char   *ptr = &src_az_current_source_buffer->c_text
                        [src_az_current_source_buffer->w_current_position-1];

        int    lcode_entry = Is_lcode(ptr);

        if (l_charset_sixteen_bit || lcode_entry >=0)
#else
 	if ( l_charset_sixteen_bit )
#endif
	{
	    /* need to:
	    **	1) get next input char and advance the source 
	    **	2) check that the next is not a control character
	    **	3) place the next character in the lex buffer
	    */

	    next_char = src_az_current_source_buffer->c_text
		    [ src_az_current_source_buffer->w_current_position++ ];

	    switch (class_table[ next_char ])
	    {

	    case class_eol:
		src_az_current_source_buffer->w_current_position--;
	    case class_illegal:
		issue_control_char_diagnostic( next_char );
		break;
	    default:
		break;
	    }

	    if (l_lex_pos > l_max_lex_buffer_pos )
	    {
		az_current_lex_buffer = get_lex_buffer( az_current_lex_buffer );
		l_lex_pos = 0;
	    }

	    az_current_lex_buffer->c_text[ l_lex_pos++ ] = next_char;
#ifdef I18N_MULTIBYTE
            if (lcode_entry>=0) {
               if ((char_size = charset_len(ptr,l_charset)) == 0) {
                  if (!Is_lcode_valid(lcode_entry)) {
                     diag_issue_diagnostic
                        ( d_unterm_seq,
                          src_az_current_source_record,
                          src_az_current_source_buffer->w_current_position-2,
                          "wrong leading code sequence");
                  }
               } else {
                 int i;

                 increment_lcode_counter(lcode_entry);
                 for ( i = 0; i < lcode_len(lcode_entry)+char_size-2; i++)
                 {
                     next_char = src_az_current_source_buffer->c_text
                          [ src_az_current_source_buffer->w_current_position++
 ];
                     switch (class_table[ next_char ])
                     {
                        case class_eol:
                            src_az_current_source_buffer->w_current_position--;
                        case class_illegal:
                            issue_control_char_diagnostic( next_char );
                            break;
                        default:
                            break;
                     }
                     if (l_lex_pos > l_max_lex_buffer_pos )
                     {
                        az_current_lex_buffer = get_lex_buffer(
az_current_lex_buffer );
                        l_lex_pos = 0;
                     }
                     az_current_lex_buffer->c_text[ l_lex_pos++ ] = next_char;
                 }
               }
             } else {
               l_16bit_char_count ++;
             }
#else
	    l_16bit_char_count ++;
#endif
	}

	break;
    }

    case charset_gstr:		/* set l_charset with the char set */
    {
	_assert( FALSE, "Should never get to charset_gstr" );
	break;
    }

    case nocharset_gstr:
	if (Uil_lex_l_charset_specified) {
	    /* use the specified charset*/
	    l_charset = Uil_lex_l_literal_charset;
	    az_charset_entry = Uil_lex_az_literal_charset;
	    }
	else if (Uil_lex_l_localized) goto found_localized_string;
	else {		
	    /* No charset specified, use the default */
	    l_charset = Uil_lex_l_user_default_charset;
	    az_charset_entry = Uil_lex_az_charset_entry;
	    }



	/* Get the charset information */
	sem_charset_info
	    (l_charset,
	     az_charset_entry,
	     &l_write_direction,
	     &l_parse_direction,
	     &l_charset_sixteen_bit);

	/* reset 16 bit character count to 0 */

	l_16bit_char_count = 0;
#ifdef I18N_MULTIBYTE
        Reset_lcode_counter();
#endif /* I18N_MULTIBYTE */


	/*
	**  if this is a user-defined, 16-bit charset then treat all
	**  as 16-bit.
	*/
	if ((l_charset_sixteen_bit) && (l_charset == lex_k_userdefined_charset))
	    l_16bit_chars_only = TRUE;

	break;

    default:
	_assert( FALSE, "unknown token_table special action" );

    }

    /* Next state of the token builder is should already be in l_state.
     * Continue at this point */

    goto continue_in_next_state;


found_localized_string:
    {
      /* Local variables */
      int 		mb_len, i;
      unsigned char	mb_byte;
      
      /* Should be looking at the first byte of the string. */
      /* Localize... */
      _XmOSSetLocale("");
    
      /* Parse the string. */
      while (TRUE)
	{
	  mb_len = mblen((char *)&src_az_current_source_buffer->c_text
			 [src_az_current_source_buffer->w_current_position],
			 MB_CUR_MAX);
      
	  mb_byte = src_az_current_source_buffer->c_text
	    [src_az_current_source_buffer->w_current_position];

	  if (mb_len == 1)
	    switch (class_table[mb_byte])
	      {
	      case class_eol:
		z_cell.backup = backup_0;
		l_state = token_ugstr;

		if (l_lex_pos > l_max_lex_buffer_pos )
		  {
		    az_current_lex_buffer = 
		      get_lex_buffer( az_current_lex_buffer );
		    l_lex_pos = 0;
		  }
		az_current_lex_buffer->c_text[ l_lex_pos++ ] = mb_byte;
		_XmOSSetLocale("C");
		goto found_token;

	      case class_dquote:
		z_cell.backup = backup_0;
		l_state = token_lstr;
		src_az_current_source_buffer->w_current_position++;
		_XmOSSetLocale("C");
		goto found_token;

	      default:
		break;
	      }
      
	  if (l_lex_pos > l_max_lex_buffer_pos )
	    {
	      az_current_lex_buffer = get_lex_buffer( az_current_lex_buffer );
	      l_lex_pos = 0;
	    }

	  for (i = 0; i < mb_len; i++)
	    {
	      if (l_lex_pos > l_max_lex_buffer_pos )
		{
		  az_current_lex_buffer = get_lex_buffer(az_current_lex_buffer);
		  l_lex_pos = 0;
		}
	      az_current_lex_buffer->c_text[l_lex_pos++] = 
		src_az_current_source_buffer->c_text
		  [src_az_current_source_buffer->w_current_position++];
	    }
	}
    }
      
found_token:

    /* do any backup of the source buffer position and lex buffer */

    src_az_current_source_buffer->w_current_position -= z_cell.backup;
    l_lex_pos -= z_cell.backup;

    /* put a null at the end of the current lex buffer */

    az_current_lex_buffer->c_text[ l_lex_pos ] = 0;

    /* case on the token found */

    switch (l_state)		/* l_state holds the token found */
    {
    case token_name:
    {

	key_keytable_entry_type	*az_keyword;

	/* check that the length of the name is in range */

	if (l_lex_pos > key_k_keyword_max_length)
	{
	    l_lex_pos = key_k_keyword_max_length;
	    az_current_lex_buffer->c_text[ l_lex_pos ] = 0;
	    diag_issue_diagnostic
		( d_name_too_long,
		  az_start_src_record,
		  l_start_src_pos,
		  az_current_lex_buffer->c_text );
	}

	/* check the case sensitivity flag and change case if necessary */

	if (! uil_v_case_sensitive) 
	{
	    char	* ptr;
	    for ( ptr = (char *)(az_current_lex_buffer->c_text);
		  (* ptr) != 0;
		  (* ptr) = _upper (* ptr), ptr++)
	    {}
	}

	/* check if the name is a keyword */

	az_keyword = 
	    key_find_keyword( l_lex_pos, (char *)az_current_lex_buffer->c_text );
	if( az_keyword != NULL)
	    {
	    yylval.value.az_keyword_entry = az_keyword;
	    yylval.b_type = az_keyword->b_token;
	    break;
	    }

	/* process the name as an identifier */

	yylval.value.az_symbol_entry =
	    (sym_entry_type *) sym_insert_name( l_lex_pos, (char *)az_current_lex_buffer->c_text );

	yylval.b_type = NAME;
	break;
    }

    case token_punc:
    {
	int		l_token;

	/* found a punctuation mark - look up its token number in a table */

	l_token = punc_token
		    [ _index( az_current_lex_buffer->c_text[ 0 ],
			      punc_char, sizeof( punc_char )) ];
	yylval.b_type = l_token;

        break;
    }

    case token_punc2:
    {
	int		l_token;

	/* found a punctuation mark - look up its token number in a table */

	l_token = punc2_token
		    [ _index( az_current_lex_buffer->c_text[ 0 ],
			      punc2_char, sizeof( punc2_char )) ];
	yylval.b_type = l_token;

        break;
    }

    case token_eol:
	
	/* if there is no more source
	 *   then return an end of file
	 *   otherwise go look for the next token */

	if (src_get_source_line( ) == src_k_end_source)
	    return UILEOF;

	goto initialize_token_builder;

    case token_integer:
    {
	long			l_integer;

	yylval.b_type = UNS_INT_LITERAL;

	/* convert the text to binary
	 *    the sign of the number is applied as part of semantic
	 *    analysis; thus we only handle integers in range 0..2**31-1
	 */

        errno = 0;
	l_integer = cvt_ascii_to_long(az_current_lex_buffer->c_text);

	if (errno != 0)
	    diag_issue_diagnostic
		( d_out_range, 
		  az_start_src_record,
		  l_start_src_pos,
		  "integer",
		  " " );

	yylval.value.az_symbol_entry = 
		(sym_entry_type *)sem_create_value_entry
		    ( (char *)&l_integer, sizeof( long ), sym_k_integer_value );
	break;
    }	

    case token_ustring:

	diag_issue_diagnostic
		( d_unterm_seq, 
		  src_az_current_source_record,
		  src_az_current_source_buffer->w_current_position - 1,
		  "character string",
		  "before end of line" );

  case token_comment:       /* RAP preserve comments */
    {
      int size;

      if (last_token_seen != token_comment)
	comment_text[0]=0;

      size = strlen((char *)az_current_lex_buffer->c_text)+1;
      if ((size  + strlen (comment_text)) >= comment_size)
	{
	  comment_text = XtRealloc(comment_text, INCR_COMMENT_SIZE + strlen(comment_text));
	  comment_size = INCR_COMMENT_SIZE + strlen (comment_text);
	}
	  
      strcat (comment_text, (char *)az_current_lex_buffer->c_text);
      strcat (comment_text, "\n");
      last_token_seen = token_comment;
	    goto initialize_token_builder;
    }


    case token_string:
#ifdef I18N_MULTIBYTE
        /*
         * Initialization has not being done for single quoted string.
         * Don't care about encoded charset in single quoted string.
         */
        Reset_lcode_counter();
#endif /* I18N_MULTIBYTE */
found_primitive_string:
    {
	int			l_length;
	lex_buffer_type		*az_lex_buffer;
	sym_value_entry_type	*az_value;
	unsigned char		*c_char;

#ifdef I18N_MULTIBYTE
      /*
       * Handle encoded charset in a general string of only one charset
       */
      int                       lcode_entry;
      if (!Is_all_lcode_counter_zero()) {
          int count, total;
          /*
           * Count no of bytes excluding Leading codes
           */
          total = 0;
          for (az_lex_buffer = az_first_lex_buffer;
               az_lex_buffer != az_current_lex_buffer;
               az_lex_buffer = az_lex_buffer->az_next_buffer){
              count = 0;
              while (count < l_max_lex_buffer_pos + 1) {
                 if ((lcode_entry=Is_lcode(&az_lex_buffer->c_text[count]))>=0){
                    if (lcode_count(lcode_entry)) {
                      count += lcode_len(lcode_entry);
                    } else {
                      total ++;
                      count ++;
                    }
                 } else {
                   total ++;
                   count ++;
                 }
             }
          }
          count = 0;
          while ( count < l_lex_pos){
             if ((lcode_entry=Is_lcode(&az_lex_buffer->c_text[count]))>=0){
                if (lcode_count(lcode_entry)) {
                   count += lcode_len(lcode_entry);
                } else {
                   total ++;
                   count ++;
                }
             } else {
                total ++;
                count ++;
             }
          }
          /*
           * create cs of length tatal
           */
          az_value = create_str_entry( total, l_charset, az_charset_entry );

          c_char = (unsigned char *)az_value->value.c_value;

          /*
           * copy byte by byte excluding leading codes
           */
          for (az_lex_buffer = az_first_lex_buffer;
               az_lex_buffer != az_current_lex_buffer;
               az_lex_buffer = az_lex_buffer->az_next_buffer){
              count = 0;
              while (count < l_max_lex_buffer_pos + 1) {
                 if ((lcode_entry=Is_lcode(&az_lex_buffer->c_text[count]))>=0){
                    if (lcode_count(lcode_entry)) {
                      count += lcode_len(lcode_entry);
                    } else {
                      *c_char = az_lex_buffer->c_text[count++];
                      c_char++;
                    }
                 } else {
                    *c_char = az_lex_buffer->c_text[count++];
                    c_char++;
                 }
             }
          }
          count = 0;
          while (count < l_lex_pos) {
             if ((lcode_entry=Is_lcode(&az_lex_buffer->c_text[count]))>=0){
                if (lcode_count(lcode_entry)) {
                   count += lcode_len(lcode_entry);
                } else {
                   *c_char = az_lex_buffer->c_text[count++];
                   c_char++;
                }
             } else {
                *c_char = az_lex_buffer->c_text[count++];
                c_char++;
             }
          }
      } else {
        l_length = l_lex_pos;

        for (az_lex_buffer = az_first_lex_buffer;
             az_lex_buffer != az_current_lex_buffer;
             az_lex_buffer = az_lex_buffer->az_next_buffer)
            l_length = l_length + l_max_lex_buffer_pos + 1;

        az_value = create_str_entry( l_length, l_charset, az_charset_entry );

        c_char = (unsigned char *)az_value->value.c_value;

        for (az_lex_buffer = az_first_lex_buffer;
             az_lex_buffer != az_current_lex_buffer;
             az_lex_buffer = az_lex_buffer->az_next_buffer)
        {
            _move( c_char, az_lex_buffer->c_text, l_max_lex_buffer_pos + 1);
            c_char = c_char + l_max_lex_buffer_pos + 1;
        }

        _move( c_char, az_lex_buffer->c_text, l_lex_pos );
      }
#else

	l_length = l_lex_pos;

	for (az_lex_buffer = az_first_lex_buffer;  
	     az_lex_buffer != az_current_lex_buffer;  
	     az_lex_buffer = az_lex_buffer->az_next_buffer)
	    l_length = l_length + l_max_lex_buffer_pos + 1;

	az_value = create_str_entry( l_length, l_charset, az_charset_entry );

	c_char = (unsigned char *)az_value->value.c_value;

	for (az_lex_buffer = az_first_lex_buffer;
	     az_lex_buffer != az_current_lex_buffer;  
	     az_lex_buffer = az_lex_buffer->az_next_buffer)
	{
	    _move( c_char, az_lex_buffer->c_text, l_max_lex_buffer_pos + 1);
	    c_char = c_char + l_max_lex_buffer_pos + 1;
	}

	_move( c_char, az_lex_buffer->c_text, l_lex_pos );
#endif

    	yylval.value.az_symbol_entry = (sym_entry_type *) az_value;
    	yylval.b_type = CHAR_8_LITERAL;
#ifdef DEC_MOTIF_BUG_FIX
        sar_assoc_comment((sym_obj_entry_type *)
            yylval.value.az_symbol_entry);
#endif


	break;
    }

    case token_real:
    {
	double	d_real;

	yylval.b_type = UNS_FLOAT_LITERAL;

        errno = 0;
	d_real = atof((char *)az_current_lex_buffer->c_text);

	if (errno != 0)
	    diag_issue_diagnostic
		( d_out_range, 
		  az_start_src_record,
		  l_start_src_pos,
		  "real",
		  " " );

	yylval.value.az_symbol_entry = 
		(sym_entry_type *)sem_create_value_entry
		    ( (char *)&d_real, sizeof( double ), sym_k_float_value );
	break;
    }
	
    case token_ff:

	if (l_start_src_pos != 0)
	    issue_control_char_diagnostic( c_char );

        src_az_current_source_record->b_flags |= src_m_form_feed; 

	goto initialize_token_builder;

    case token_ugstr:

	diag_issue_diagnostic
		( d_unterm_seq, 
		  src_az_current_source_record,
		  src_az_current_source_buffer->w_current_position - 1,
		  "character string",
		  "before end of line" );

    case token_gstr:

	/*
	** Some general strings require special processing.  Those
	** that do not can go thru the normal string code.
	*/
        if ( l_parse_direction == XmSTRING_DIRECTION_R_TO_L )
	{
	    int		    i,j;
	    unsigned char   tmp1;
	    unsigned char   tmp2;

	    /* assuming the string is confined to a single lex buffer.	    */
	    /* just flip the characters around.  16-bit characters need to  */
	    /* be done in groups of two bytes				    */
	    if (l_charset_sixteen_bit != TRUE)
		/*
		**  Just reverse the bytes from the first to last
		*/
		for (i=0, j=l_lex_pos-1;  i < (l_lex_pos>>1);  i++,j--)
		{
		    tmp1 = az_current_lex_buffer->c_text[ i ];
		    az_current_lex_buffer->c_text[ i ] = 
			az_current_lex_buffer->c_text[ j ];
		    az_current_lex_buffer->c_text[ j ] = tmp1;
		}

	    /*
	    **  Don't reverse the string if have less than 2 characters (4 bytes)
	    */
	    else if ((l_lex_pos>>1) > 1)
		/*
		**  This reversing doesn't work for mixed 8/16-bit character
		**  sets, but only built-in character sets allow mixing and
		**  they are not right-to-left.  We do the same copying as in
		**  the 8-bit case above, but we move two bytes at a time and
		**  reverse the order as we copy so they end up correct.
		*/
		for (i=0, j=l_lex_pos-1;  i < (l_lex_pos>>1);  i+=2,j-=2)
		{
		    tmp1 = az_current_lex_buffer->c_text[ i ];
		    tmp2 = az_current_lex_buffer->c_text[ i + 1 ];
		    az_current_lex_buffer->c_text[ i ] = 
			az_current_lex_buffer->c_text[ j - 1 ];
		    az_current_lex_buffer->c_text[ i + 1 ] = 
			az_current_lex_buffer->c_text[ j ];
		    az_current_lex_buffer->c_text[ j ] = tmp2;
		    az_current_lex_buffer->c_text[ j - 1 ] = tmp1;
		}
	}


	/*
	**  If the string isn't 16-bit or it is userdefined and thus
	**  cannot be mixed 8/16-bit then we can just make a primitive
	**  string.
	*/
	if ((l_charset_sixteen_bit != TRUE) ||
	    (l_charset == lex_k_userdefined_charset))
	    goto found_primitive_string;
	else
	{
	
	    sym_value_entry_type    *cstr_entry;
	    sym_value_entry_type    *str_entry;
	    int			    a_off, off;
#ifdef I18N_MULTIBYTE
            char                    *buffer, *lc_ptr;
            int                     count, char_size, i, lc_len;

            /*
            **  If string consists solely of 8-bit ascii characters,
            **  all leading code counters
            **  will be zero.
            **  if string consists solely of one character set,
            **  (number of chars of that charset) * (length of that charset)
            **  will equal l_lex_pos.
            **  In either of these cases, the result is still a
            **  primitive string.
            */
#else



	    /*
	    **	if string consists solely of 8-bit ascii characters,
	    **  l_16bit_char_count will be zero.
	    **	if string consists solely of 16 bit characters,
	    **  l_16bit_char_count*2 will equal l_lex_pos.
	    **  In either of these cases, the result is still a
	    **	primitive string.
	    */
#endif

	    /*
	    **  For KANJI and HANZI treat 8-bit characters as ISO_LATIN1.  
	    */
#ifdef I18N_MULTIBYTE
            if (l_16bit_char_count == 0 && Is_all_lcode_counter_zero())
#else
	    if (l_16bit_char_count == 0)
#endif
	    {
		l_charset = uil_sym_isolatin1_charset;
		goto found_primitive_string;
	    }

	    /*
	    **  If the string only contains 16-bit characters,
	    **  it still can be stored as a primitive string.
	    */
	    if ((l_16bit_char_count<<1) == l_lex_pos)
		goto found_primitive_string;

#ifdef I18N_MULTIBYTE
            /*
            **  If the string only contains characters from encoded charset,
            **  it still can be stored as a primitive string.
            */

            for (count = 0; count < total_no_of_leading_codes; count++) {
               if ( (lcode_count(count)) && ((lcode_count(count) *
                  ( charset_len(lcode_ptr(count),l_charset) +
                   lcode_len(count))) == l_lex_pos )){
                  l_charset = cset_in_composite_cset(lcode_ptr(count), l_charse
                  goto found_primitive_string;
               }
            }
#endif /* I18N_MULTIBYTE */

#ifdef I18N_MULTIBYTE
            /*
            **  Lex buffer is a mix of at least two kinds from
            **  1 byte, 2 byte, and other encoded charset characters.
            **  need to build a compound string.
            */
#else


	    /*
	    **	lex buffer is a mix of 8 and 16 bit characters.
	    **	need to build a compound string.
	    */

#endif
	    cstr_entry = sem_create_cstr();
#ifdef I18N_MULTIBYTE
            /*
             * Buffer for removing leading codes
             */
            if (!Is_all_lcode_counter_zero())
              buffer = _get_memory ( l_lex_pos );
#endif /* I18N_MULTIBYTE */

	    for ( a_off = 0,
		  off = 0;
		  off < l_lex_pos;
		)

	    {

#ifdef I18N_MULTIBYTE
/*
 * Create encoded charset string with the appropriate charset
 */

                for (i = 0; i < total_no_of_leading_codes; i++) {
                   if (lcode_count(i)) {
                      count = 0;
                      lc_len = lcode_len(i);
                      lc_ptr = lcode_ptr(i);
                      char_size = charset_len (lc_ptr,l_charset );
                      while ( strncmp(&az_current_lex_buffer->c_text[a_off],
                             lc_ptr,lc_len)==0
                             && a_off < l_lex_pos ){
                         _move ( &buffer[ count ],
                                &az_current_lex_buffer->c_text[a_off+lc_len],
                                char_size );
                         count += char_size;
                         a_off += (char_size + lc_len);
                      }
                      if (count ){
                          str_entry = create_str_entry
                             (count, cset_in_composite_cset(lc_ptr,l_charset)
                              , az_charset_entry );
                          _move( str_entry->value.c_value, buffer, count );
                          sem_append_str_to_cstr( cstr_entry, str_entry, TRUE );
                      }
                   }
               }

#endif /* I18N_MULTIBYTE */

#ifdef I18N_MULTIBYTE
                for (off = a_off;  off < l_lex_pos;  off++){
                    if (az_current_lex_buffer->c_text[ off ] > 0x97 )
                        break;
                    if ((i = Is_lcode(&az_current_lex_buffer->c_text[off]))>=0)
                       if (lcode_count(i))
                          break;
                }

#else
		for (off = a_off;  off < l_lex_pos;  off++)
		    if (az_current_lex_buffer->c_text[ off ] > 0x97)
			break;
#endif

/*
 * Create the 8 bit string with iso_latin1
 */

		if (off > a_off)
		{
		    str_entry = create_str_entry
			( off - a_off,
			 uil_sym_isolatin1_charset,
			 az_charset_entry );

		    _move( str_entry->value.c_value,
			   &az_current_lex_buffer->c_text[ a_off ],
			   off-a_off );

		    sem_append_str_to_cstr( cstr_entry, str_entry, TRUE );
		}

#ifdef I18N_MULTIBYTE
                for (a_off = off; a_off < l_lex_pos; a_off +=2) {
                   if (az_current_lex_buffer->c_text[a_off] <= 0x97)
                      break;
                   if ((i = Is_lcode(&az_current_lex_buffer->c_text[a_off]))>=0)
                      if (lcode_count(i))
                        break;
                }
#else
		for (a_off = off;  a_off < l_lex_pos;  a_off += 2)
		    if (az_current_lex_buffer->c_text[ a_off ] <= 0x97)
			break;
#endif

/*
 * Create the 16 bit string with its charset
 */

		if (a_off > off)
		{
		    str_entry = 
			create_str_entry( a_off - off, l_charset, az_charset_entry );

		    _move( str_entry->value.c_value,
			   &az_current_lex_buffer->c_text[ off ],
			   a_off-off );

		    sem_append_str_to_cstr( cstr_entry, str_entry, TRUE );
		}

	    }

	    yylval.value.az_symbol_entry = (sym_entry_type *)cstr_entry;
	    yylval.b_type = COMP_STRING;

#ifdef I18N_MULTIBYTE
            if (!Is_all_lcode_counter_zero())
              _free_memory ( buffer );
#endif /* I18N_MULTIBYTE */

	}
	break;

  case token_lstr:
    {
      int			l_length = 0;
      lex_buffer_type		*az_lex_buffer;
      sym_value_entry_type	*str_entry;
      unsigned char		*c_char;

      l_length = l_lex_pos;

      for (az_lex_buffer = az_first_lex_buffer;  
	   az_lex_buffer != az_current_lex_buffer;  
	   az_lex_buffer = az_lex_buffer->az_next_buffer)
	l_length = l_length + l_max_lex_buffer_pos + 1;

      str_entry = create_str_entry(l_length, lex_k_fontlist_default_tag, 
				  az_charset_entry );

      c_char = (unsigned char *)str_entry->value.c_value;

      for (az_lex_buffer = az_first_lex_buffer;
	   az_lex_buffer != az_current_lex_buffer;  
	   az_lex_buffer = az_lex_buffer->az_next_buffer)
	{
	  _move( c_char, az_lex_buffer->c_text, l_max_lex_buffer_pos + 1);
	  c_char = c_char + l_max_lex_buffer_pos + 1;
	}

      _move( c_char, az_lex_buffer->c_text, l_lex_pos );
      
      yylval.value.az_symbol_entry = (sym_entry_type *)str_entry;
      yylval.b_type = LOC_STRING;

      break;
    }

    default:
	_assert( FALSE, "unknown token table final state" );
  }

/* RAP we want to keep track of whether we are appending sequential comments */
    last_token_seen = l_state;

    /* set position information in token value */

    yylval.az_source_record = az_start_src_record;
    yylval.b_source_pos = l_start_src_pos;
    yylval.b_source_end = src_az_current_source_buffer->w_current_position;  /*  was "l_start_src_pos + l_lex_pos;" */
    yylval.b_tag = sar_k_token_frame;

    /* dump the token if requested */

#if debug_version
    if (uil_v_dump_tokens)
	dump_token( az_current_lex_buffer, l_lex_pos );
#endif

    /*
    ** save this token
    */

    prev_yylval = yylval;

    /* return the token generated */

    return yylval.b_type;

found_error:

    /* do any backup of the source buffer position and lex buffer */

    src_az_current_source_buffer->w_current_position -= z_cell.backup;
    l_lex_pos -= z_cell.backup;

    /* put a null at the end of the current lex buffer */

    az_current_lex_buffer->c_text[ l_lex_pos ] = 0;

    /* case on the type of error */

    switch (l_state)		/* contains the type of error */
    {
    case bad_prefix:

	/* printable characters that are not part of a token were found */

	diag_issue_diagnostic
		( d_unknown_seq, 
		  az_start_src_record,
		  l_start_src_pos,
		  az_current_lex_buffer->c_text );

	break;

    default:
	_assert( FALSE, "unknown token table error state" );
	break;
      }

    goto initialize_token_builder;

  }


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the lexical analyzer.
**
**  FORMAL PARAMETERS:
**
**      none
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      az_first_lex_buffer
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      lexical buffer is allocated
**
**--
**/

#define UNSCHAR_MINUS_ONE (unsigned char) 255;

void  lex_initialize_analyzer( )

{
String language;

/* RAP preserve comments */
comment_text = (char *) _get_memory(INITIAL_COMMENT_SIZE);
comment_size = INITIAL_COMMENT_SIZE;


az_first_lex_buffer = 
    (lex_buffer_type *) _get_memory (l_max_lex_buffer_pos + 1 +
				     sizeof(lex_buffer_type *));
az_first_lex_buffer->az_next_buffer = NULL;

/*   Initialize the stack frame entry for epsilon productions.   */

gz_yynullval.b_tag = sar_k_null_frame;

/*   Initialize the default character set  */

language = (char *) _XmStringGetCurrentCharset();
if ( language == NULL )
    Uil_lex_l_user_default_charset = lex_k_default_charset;
else
    {
    Uil_lex_l_user_default_charset = sem_charset_lang_name (language);
    if (Uil_lex_l_user_default_charset == sym_k_error_charset)
	{
	diag_issue_diagnostic
	    ( d_bad_lang_value, 
	     diag_k_no_source,
	     diag_k_no_column);
	Uil_lex_l_user_default_charset = lex_k_default_charset;
	}		
    }
Uil_lex_az_charset_entry = NULL;

/* Determine if localized strings are possible */
if (Uil_cmd_z_command.v_use_setlocale == FALSE)
  Uil_lex_l_localized = FALSE;
else
  {
    Uil_lex_l_localized = TRUE;
    _XmOSSetLocale("C");
  }

/*   Initialize the current character set */
Uil_lex_l_charset_specified = FALSE;

/*    Initialize the source position and record */

prev_yylval.b_source_end = UNSCHAR_MINUS_ONE;
prev_yylval.az_source_record = src_az_current_source_record;

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function performs the cleanup processing of the lexical analyzer.
**
**  FORMAL PARAMETERS:
**
**      none
**
**  IMPLICIT INPUTS:
**
**      az_first_lex_buffer
**
**  IMPLICIT OUTPUTS:
**
**      az_first_lex_buffer
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      lexical buffer is freed
**
**--
**/

void  Uil_lex_cleanup_analyzer( )

{
    /*	pointer to next buffer to free	*/
    lex_buffer_type  *az_buffer_to_free;

    /* Loop through the list of buffers freeing them all */
    while (az_first_lex_buffer != NULL) {
	az_buffer_to_free = az_first_lex_buffer;
	az_first_lex_buffer = az_first_lex_buffer->az_next_buffer;
	_free_memory((char*)az_buffer_to_free);
	}
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function issues a syntax error.  It is called from the
**	error handling mechanism in the parser.
**
**  FORMAL PARAMETERS:
**
**      restart_token		the token number for the punctuation
**				character where parsing will resume after
**				this error is issued.
**
**  IMPLICIT INPUTS:
**
**      current lex buffer
**	punc_token and punc_char tables
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      issue a diagnostic
**
**--
**/

void  lex_issue_error( restart_token )

int		restart_token;

{

    int		    i, token_num;
    unsigned char   c_char = '.';
    char	    * tok_name;

/*    Find the token number for the restart character in the table.
      It should be there.  Get the corresponding character for this
      token.    */

    for ( i = 0 ; i<tok_punc_token_num ; i++ )
	{
	if (restart_token == punc_token [i])
	    {
	    c_char = punc_char [i];
	    break;
	    }
	}

/*    Get the text of the token name which caused the error.    */

    token_num = yylval.b_type;
    if ( (token_num < 0) || (token_num > tok_num_tokens) )
	tok_name = "UNKNOWN_TOKEN";
    else
	tok_name = tok_token_name_table[token_num];

/*    Issue the error.    */

    diag_issue_diagnostic
	(d_syntax, 
	 yylval.az_source_record,
	 yylval.b_source_pos,
	 tok_name,
	 c_char);

}



/*
**
**  LOCAL FUNCTIONS
**
*/


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Issue an error for an illegal control character.
**
**  FORMAL PARAMETERS:
**
**      c_char
**
**  IMPLICIT INPUTS:
**
**      current source position
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      issue a diagnostic
**
**--
**/

void issue_control_char_diagnostic

#ifndef _NO_PROTO
	(unsigned char c_char )
#else
	(c_char)

unsigned char	c_char;
#endif

{

    diag_issue_diagnostic
	( d_control_char, 
	  src_az_current_source_record,
	  src_az_current_source_buffer->w_current_position - 1,
	  c_char );

    src_az_current_source_record->b_flags |= src_m_unprintable_chars; 

    return;

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function obtains another lexical buffer.
**
**  FORMAL PARAMETERS:
**
**      az_current_lex_buffer
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      address of a new lexical buffer
**
**  SIDE EFFECTS:
**
**      another lexical buffer may be allocated
**
**--
**/

static lex_buffer_type *get_lex_buffer( az_current_lex_buffer )

lex_buffer_type *az_current_lex_buffer;

{
    lex_buffer_type *az_lex_buffer;

    /* check to see if another buffer is available - if not allocate one */

    az_lex_buffer = az_current_lex_buffer->az_next_buffer;

    if (az_lex_buffer == NULL)
    {
	az_lex_buffer = 
	    (lex_buffer_type *)_get_memory( l_max_lex_buffer_pos + 1 +
					   sizeof(lex_buffer_type *));
	az_current_lex_buffer->az_next_buffer = az_lex_buffer;
	az_lex_buffer->az_next_buffer = NULL;
    }

    return az_lex_buffer;

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This procedure will change all the unprintable characters in
**	a buffer to lex_k_unprint_sub.
**
**  FORMAL PARAMETERS:
**
**      buffer		buffer to be checked
**	length		length of the buffer
**	flags		lex_m_filter_xxx flags to indicate if additional
**			characters should be filtered.
**
**  IMPLICIT INPUTS:
**
**      class_table	gives the unprintable characters
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

void	lex_filter_unprintable_chars( buffer, length, flags )

char	*buffer;
int	length;
unsigned long	flags;

{
    int		    i;
	
    for (i=0;  i<length;  i++)
    {
	if ((class_table[ buffer[ i ] ] == class_illegal) ||
	    (buffer[ i ] == 12) ||  /* form feed */
	    (buffer[ i ] == 0) ||    /* null */
	    ( (flags & lex_m_filter_tab)
		&& buffer[ i ] == 9 )/* horizontal tab */
	   )
	    buffer[ i ] = lex_k_unprint_sub;
    }

}


#if debug_version
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This procedure will dump a token.
**
**  FORMAL PARAMETERS:
**
**
**  IMPLICIT INPUTS:
**
**
**  IMPLICIT OUTPUTS:
**
**
**  SIDE EFFECTS:
**
**
**--
**/

static void	dump_token( az_current_lex_buffer,
		    l_lex_pos)

lex_buffer_type	*az_current_lex_buffer;
int		l_lex_pos;
{
    unsigned char    c_buffer[l_max_lex_buffer_pos +2];
    lex_buffer_type *az_lex_buffer;
    int		    i;
    int		    last;
    int		    last_buffer;
    unsigned char   c_char;


    _debug_output
	 ("token: %d start: %d, %d  end: %d, %d \n", 
	   yylval.b_type,
	   yylval.az_source_record->w_line_number,
	   yylval.b_source_pos,
	   src_az_current_source_record->w_line_number,
	   src_az_current_source_buffer->w_current_position );

    for (az_lex_buffer = az_first_lex_buffer;  
	 ;  
	 az_lex_buffer = az_lex_buffer->az_next_buffer)
    {
	last_buffer = ( az_lex_buffer == az_current_lex_buffer );
	if (last_buffer)
	    last = l_lex_pos;
	else
	    last = l_max_lex_buffer_pos+1;

	_move( c_buffer, az_lex_buffer->c_text, last );
	
	lex_filter_unprintable_chars (c_buffer, last, 0);

	c_buffer[ last ] = 0;
	_debug_output("%s \n", c_buffer);
    
        if (last_buffer)
	    return;
    }
}
#endif

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function converts a null terminated string to a 
**	longword integer in the range 0..2**31-1.  If the ascii value is
**	outside that range, the external variable errno is set to ERANGE
**	and the value returned is 2**31-1 
**
**  FORMAL PARAMETERS:
**
**      c_text		null terminate string holding integer in ascii
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      errno		set if overflow occurs
**
**  FUNCTION VALUE:
**
**      long 		integer value of c_text
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

#define k_max_int 2147483647
#define k_max_div_10 214748364

long		cvt_ascii_to_long(c_text)
unsigned char XmConst	*c_text;
{
    unsigned long   	l_value;
    int			pos;

    l_value = 0;
    for (pos = 0;  c_text[ pos ] != 0;  pos++)
    {
	if (l_value >= k_max_div_10)
	    goto potential_overflow;
	l_value = (l_value * 10) + c_text[ pos ] - '0';
    }

    return l_value;

potential_overflow:
    if (l_value > k_max_div_10)
    {
	errno = ERANGE;
	return k_max_int;
    }

    l_value = (l_value * 10) + c_text[ pos ] - '0';

    if (l_value > k_max_int)
    {
	errno = ERANGE;
	return k_max_int;
    }

    return l_value;
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function creates a symbol entry for a primitive string.
**
**  FORMAL PARAMETERS:
**
**      l_size	    number of bytes to allocate
**	l_charset   charset of the string (token value)
**	az_charset_entry   charset of the string (symbol table value entry)
**
**  IMPLICIT INPUTS:
**
**      yylval
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      symbol node created
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

sym_value_entry_type *create_str_entry (l_size, l_charset, az_charset_entry)

    int				l_size;
    int				l_charset;
    sym_value_entry_type	*az_charset_entry;
			

{

    sym_value_entry_type	*node;
    int				charset;	/* from sym_k_..._charset */
    unsigned char	     	direction;	/* writing direction */

    /*
     *	Determine character set and writing direction
     */
    if (l_charset != lex_k_userdefined_charset)
	{
	charset = sem_map_subclass_to_charset( l_charset );
	direction = charset_writing_direction_table[charset];
	}
    else
	{
	charset = sym_k_userdefined_charset;
	direction = az_charset_entry->b_direction;
	}
	

    /* size of entry
     *	sym_k_value_entry for common part of a value entry
     *	l_size for the string
     *	1 for the null on string
     */

    node = (sym_value_entry_type *)
	    sem_allocate_node
		( sym_k_value_entry,
		  sym_k_value_entry_size );

    node->value.c_value = XtCalloc(1, l_size + 1);

    node->header.az_src_rec = yylval.az_source_record;
    node->header.b_src_pos = yylval.b_source_pos;
    node->header.b_end_pos = yylval.b_source_end;

    node->b_type = sym_k_char_8_value;
    node->w_length = l_size;
    node->b_charset = charset;
    node->b_direction = direction;
    node->az_charset_value = az_charset_entry;
    node->obj_header.b_flags = sym_m_private | sym_m_builtin;

    return node;

    }

#ifdef I18N_MULTIBYTE
/*
 *
 *  Open-code support:
 *
 *  Aim : to support any codeset with different encoding scheme in uil
 *
 *  The data-structure is designed so as to
 *      1)  independent of the length and contents of leading code
 *      2)  independent of the length of an encoded charset
 *
 *  Composite-charset Table                   Encoded charset Table
 *
 *  -----------------------------            _________________________
 *  |  no of encoded charset    |      ----->| character length       |
 *  -----------------------------      |     | (exclude leading code) |
 *  | encoded charset tbl entry-|-------     --------------------------
 *  -----------------------------            |     charset            |
 *  |         :                 |            --------------------------
 *  |         :                 |-----       |  leading code tbl entry|------
 *  -----------------------------    |       --------------------------     |
 *  |    charset                |    |                   :                  |
 *  -----------------------------    |        --------------------------    |
 *             :                     -------->|          :             |    |
 *             :                                                            |
 *                                                                          |
 *                                                                          |
 *                                                                          |
 *                                                                          |
 *                       Leading code Table                                 |
 *                                                                          |
 *                      ----------------------                              |
 *         "   "   <----|-* leading code ptr |     <------------------------|
 *                      ----------------------
 *                      |    byte length     |
 *                      ----------------------
 *                      |      counter       |
 *                      ----------------------
 *                      |   valid   flag     |
 *                      ----------------------
 *                                 :
 *
 */


        /*
         * EUJIS
         * 0xxxxxxx                     (ASCII)
         * 1xxxxxxx xxxxxxx             (JISX0208)
         * SS2      1xxxxxxx            (JISX0201)
         * SS3      1xxxxxxx 1xxxxxxx   (JISX0212)
         */

        /* DEC_HANYU
         * 0xxxxxxx                                (ASCII)
         * 1xxxxxxx xxxxxxx                        (DEC.CNS11643)
         * C2       CB       1xxxxxxx   1xxxxxxx   (DEC.DTSCS)
         */
<FF>

static char SS2_l_code[2] = {0x8e,0};
static char SS3_l_code[2] = {0x8f,0};
static char C2CB_l_code[3] = {0xC2,0xCB,0};

typedef struct  _leading_code_type
{
    char *leading_code_ptr;    /* null terminated string for leading code eg.SS
    int  len;                   /* no of bytes in leading code */
    int  counter;                /* count the no of leading code char */
    int  valid_seq;             /* whether the leading code is a valid sequence
} leading_code_type;

static leading_code_type leading_code_table[total_no_of_leading_codes] =
{
/* 0 */  { SS2_l_code,  1, 0, 0},
/* 1 */  { SS3_l_code,  1, 0, 0},
/* 2 */  { C2CB_l_code, 2, 0, 1}
};

/*
 * Encoded charset is charset that is encoded by some leading code
 */

typedef struct _encoded_charset_type
{
   int  leading_code_table_entry;         /* char ptr to leading code */
   int  charset_len;           /* no of bytes following leading code */
   int  charset;               /* charset info */
} encoded_charset_type;

static encoded_charset_type encoded_charset_table[total_no_of_encoded_charset]=
{
/* 0 */  { 0, 1, sym_k_jis_katakana_charset },
/* 1 */  { 1, 2, sym_k_jis_suppl_kanji_charset },
/* 2 */  { 2, 2, sym_k_dtscs_hanyu_gr_charset}
};

/*
 * Composite charset is charset that is composed of 1 or more encoded charset
 */

typedef struct _composite_charset_type
{
   int   nof_encoded_cset_supported;
   int   *encoded_cset_table_entry;
   int   charset;
} composite_charset_type;

static int   csets_in_sym_k_jis_katakana_charset[] = {0};
static int   csets_in_sym_k_jis_suppl_kanji_charset[] = {1};
static int   csets_in_sym_k_dtscs_hanyu_gr_charset[] = {2};
static int   csets_in_sym_k_jis_kanji_gr_charset[] = {0, 1};
static int   csets_in_sym_k_cns_hanyu_gr_charset[] = {2};

static composite_charset_type
composite_charset_table[total_no_of_composite_charset]=
{
   {1,  csets_in_sym_k_jis_katakana_charset, sym_k_jis_katakana_charset},
   {1,  csets_in_sym_k_jis_suppl_kanji_charset, sym_k_jis_suppl_kanji_charset},
   {1,  csets_in_sym_k_dtscs_hanyu_gr_charset, sym_k_dtscs_hanyu_gr_charset},
   {2,  csets_in_sym_k_jis_kanji_gr_charset, sym_k_jis_kanji_gr_charset},
   {1,  csets_in_sym_k_cns_hanyu_gr_charset, sym_k_cns_hanyu_gr_charset}
};
<FF>
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      charset_len() return the byte length of one character of an encocoded
**                     charset
**
**  FORMAL PARAMETERS:
**
**      l_code  ptr to leading code
**      tag     charset tag for the string in uil
**
**  IMPLICIT INPUTS:
**
**      static composite_charset_type composite_charset_table[]
**      static encoded_charset_type   encoded_charset_table[]
**      static leading_code_type      leading_code_table[]
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      Byte length of one character
**      Return 0 if this charset doesn't support l_code as leading code
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

static int
charset_len( l_code, tag )
char *l_code;
int  tag;
{
   int i,j;
   composite_charset_type cc;
   encoded_charset_type ec;
   leading_code_type lc;

   for (i=0; i<total_no_of_composite_charset; i++)
   {
      cc = composite_charset_table[i];
      if (cc.charset == tag)
      {
         for (j=0; j < cc.nof_encoded_cset_supported; j++)
         {
            ec = encoded_charset_table[cc.encoded_cset_table_entry[j]];
            if (l_code != 0) {
               lc = leading_code_table[ec.leading_code_table_entry];
               if (strncmp(l_code,lc.leading_code_ptr, lc.len)==0)
                  return (ec.charset_len);
            }
         }
      }
    }

    return(0);  /* no leading code supported in this charset tag */
}

<FF>
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      cset_in_composite_cset() return one of the encoded charset in a
**                               composite charset when a leading code
**                               is specified
**
**  FORMAL PARAMETERS:
**
**      l_code  ptr to leading code
**      tag     charset tag for the string in uil
**
**  IMPLICIT INPUTS:
**
**      static composite_charset_type composite_charset_table[]
**      static encoded_charset_type   encoded_charset_table[]
**      static leading_code_type      leading_code_table[]
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      The encoded charset symbol
**      Return 0 if this charset doesn't support l_code as leading code
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

static int
cset_in_composite_cset( l_code, tag )
char *l_code;
int  tag;
{
   int i,j;
   composite_charset_type cc;
   encoded_charset_type ec;
   leading_code_type lc;

   for (i=0; i<total_no_of_composite_charset; i++)
   {
      cc = composite_charset_table[i];
      if (cc.charset == tag)
      {
         for (j=0; j < cc.nof_encoded_cset_supported; j++)
         {
            ec = encoded_charset_table[cc.encoded_cset_table_entry[j]];
            if (l_code != 0) {
               lc = leading_code_table[ec.leading_code_table_entry];
               if (strncmp(l_code,lc.leading_code_ptr, lc.len)==0)
                  return (ec.charset);
            }
         }
      }
    }

    return(NULL);       /* no leading code supported in this charset tag */
}

<FF>
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      lcode_count () return the counter value of the leading code
**
**  FORMAL PARAMETERS:
**
**      entry   no in the leading code table
**
**  IMPLICIT INPUTS:
**
**      static leading_code_type      leading_code_table[]
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      Counter value of a leading code
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

static int lcode_count(i)
int i;
{
   if (i < total_no_of_leading_codes )
     return(leading_code_table[i].counter);
   else
     return 0;
}

<FF>
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      lcode_ptr () return the ptr value of the leading code
**
**  FORMAL PARAMETERS:
**
**      entry   no in the leading code table
**
**  IMPLICIT INPUTS:
**
**      static leading_code_type      leading_code_table[]
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      char ptr for the leading code
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

static char *lcode_ptr(i)
int i;
{
   if (i < total_no_of_leading_codes )
     return(leading_code_table[i].leading_code_ptr);
   else
     return 0;
}

<FF>
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      lcode_len () return the byte length of the leading code
**
**  FORMAL PARAMETERS:
**
**      entry   no in the leading code table
**
**  IMPLICIT INPUTS:
**
**      static leading_code_type      leading_code_table[]
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      Byte Length for the leading code
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

static int
lcode_len(i)
int i;
{
   if (i < total_no_of_leading_codes )
     return(leading_code_table[i].len);
   else
     return 0;
}

<FF>
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Is_lcode_valid () return whether the leading code is a valid sequence
**                        for other charsets
**                        eg SS2, SS3 are illegal for other charset
**                           C2CB is legal sequence
**
**  FORMAL PARAMETERS:
**
**      entry   no in the leading code table
**
**  IMPLICIT INPUTS:
**
**      static leading_code_type      leading_code_table[]
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      True or false depends on valid byte
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

static int
Is_lcode_valid(i)
int i;
{
   if (i < total_no_of_leading_codes )
     return(leading_code_table[i].valid_seq);
   else
     return 0;
}

<FF>
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Is_all_lcode_counter_zero () return true if all counter values
**      in the leading code table are zeros
**
**  FORMAL PARAMETERS:
**
**      none
**
**  IMPLICIT INPUTS:
**
**      static leading_code_type      leading_code_table[]
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      True if all Counter values of the leading code table is zero
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

static int
Is_all_lcode_counter_zero()
{
   int i;

   for (i = 0; i < total_no_of_leading_codes; i++ )
      if  (lcode_count(i)) return 0;
   return 1;
}

<FF>
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Is_lcode () checks whether the input string starts with one of
**                  the leading code in the leading code table
**
**  FORMAL PARAMETERS:
**
**      ptr   char ptr to be matched with the leading code table
**
**  IMPLICIT INPUTS:
**
**      static leading_code_type      leading_code_table[]
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      -1 if none is matched
**      the leading code table entry otherwise
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

static int
Is_lcode(ptr)
char *ptr;
{
   int i;

   for (i = 0; i < total_no_of_leading_codes; i++ )
      if ( strncmp (ptr, lcode_ptr(i),lcode_len(i)) == 0) return i;
   return -1;
}

<FF>
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      increment_lcode_counter () increments the counter for a certain
**                                 leading code
**
**  FORMAL PARAMETERS:
**
**      entry   no in the leading code table
**
**  IMPLICIT INPUTS:
**
**      static leading_code_type      leading_code_table[]
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

static void
increment_lcode_counter(i)
int i;
{
   if (i < total_no_of_leading_codes )
     leading_code_table[i].counter++;
}

<FF>
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Reset_lcode_counter () Reset all leading code counters to zero
**
**  FORMAL PARAMETERS:
**
**      entry   no in the leading code table
**
**  IMPLICIT INPUTS:
**
**      static leading_code_type      leading_code_table[]
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

static void
Reset_lcode_counter()
{
   int i;

   for (i=0 ; i < total_no_of_leading_codes ; i++)
      leading_code_table[i].counter = 0;
}
#endif /* I18N_MULTIBYTE */
