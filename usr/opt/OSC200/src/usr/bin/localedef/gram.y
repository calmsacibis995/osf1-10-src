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
 * @(#)$RCSfile: gram.y,v $ $Revision: 1.1.5.6 $ (DEC) $Date: 1993/12/20 21:30:54 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.10  com/cmd/nls/gram.y, cmdnls, bos320, 9137320a 9/4/91 13:41:56
 */

/* ----------------------------------------------------------------------
** Tokens for keywords
** ----------------------------------------------------------------------*/

/* keywords identifying the beginning and end of a locale category */
%token KW_END
%token KW_CHARMAP
%token KW_CHARSETID
%token KW_LC_COLLATE
%token KW_LC_CTYPE
%token KW_LC_MONETARY
%token KW_LC_NUMERIC
%token KW_LC_MSG
%token KW_LC_TIME
%token KW_METHODS
%token KW_DISPWID
%token KW_COPY

/* keywords supporting the LC_METHODS category */
%token KW_MBLEN
%token KW_MBTOWC
%token KW_MBSTOWCS
%token KW_WCTOMB
%token KW_WCSTOMBS
%token KW_WCWIDTH
%token KW_WCSWIDTH
%token KW_MBTOPC
%token KW_MBSTOPCS
%token KW_PCTOMB
%token KW_PCSTOMBS
%token KW_CSID
%token KW_WCSID
%token KW_TOWUPPER
%token KW_TOWLOWER
%token KW_WCTYPE
%token KW_ISWCTYPE
%token KW_STRCOLL
%token KW_STRXFRM
%token KW_WCSCOLL
%token KW_WCSXFRM
%token KW_REGCOMP
%token KW_REGEXEC
%token KW_REGFREE
%token KW_REGERROR
%token KW_STRFMON
%token KW_STRFTIME
%token KW_STRPTIME
%token KW_WCSFTIME

/* keywords support LC_COLLATE category */
%token KW_COLLATING_ELEMENT
%token KW_COLLATING_SYMBOL
%token KW_ORDER_START
%token KW_ORDER_END
%token KW_FORWARD
%token KW_BACKWARD
%token KW_NO_SUBSTITUTE
%token KW_POSITION
%token KW_WITH
%token KW_FROM
%token KW_SUBSTITUTE

/* keywords supporting LC_CTYPE category */
%token KW_ELLIPSES
%token KW_TOUPPER
%token KW_TOLOWER
%token KW_CHARCLASS

/* keywords supporting the LC_MONETARY category */
%token KW_INT_CURR_SYMBOL
%token KW_CURRENCY_SYMBOL
%token KW_MON_DECIMAL_POINT
%token KW_MON_THOUSANDS_SEP
%token KW_MON_GROUPING
%token KW_POSITIVE_SIGN
%token KW_NEGATIVE_SIGN
%token KW_INT_FRAC_DIGITS
%token KW_FRAC_DIGITS
%token KW_P_CS_PRECEDES
%token KW_P_SEP_BY_SPACE
%token KW_N_CS_PRECEDES
%token KW_N_SEP_BY_SPACE
%token KW_P_SIGN_POSN
%token KW_N_SIGN_POSN
%token KW_DEBIT_SIGN
%token KW_CREDIT_SIGN
%token KW_LEFT_PARENTHESIS
%token KW_RIGHT_PARENTHESIS

/* keywords supporting the LC_NUMERIC category */
%token KW_DECIMAL_POINT
%token KW_THOUSANDS_SEP
%token KW_GROUPING

/* keywords supporting the LC_TIME category */
%token KW_ABDAY
%token KW_DAY
%token KW_ABMON
%token KW_MON
%token KW_D_T_FMT
%token KW_D_FMT
%token KW_T_FMT
%token KW_AM_PM
%token KW_ERA
%token KW_ERA_YEAR
%token KW_ERA_D_FMT
%token KW_ERA_T_FMT
%token KW_ERA_D_T_FMT
%token KW_ALT_DIGITS
%token KW_T_FMT_AMPM
%token KW_M_D_RECENT	/* OSF extension */
%token KW_M_D_OLD	/* OSF extension */

/* keywords for the LC_MSG category */
%token KW_YESEXPR
%token KW_NOEXPR
%token KW_YESSTR
%token KW_NOSTR

/* tokens for meta-symbols */
%token KW_CODESET
%token KW_ESC_CHAR
%token KW_MB_CUR_MAX
%token KW_MB_CUR_MIN
%token KW_COMMENT_CHAR

/* tokens for user defined symbols, integer constants, etc... */
%token SYMBOL
%token STRING
%token INT_CONST
%token DEC_CONST
%token OCT_CONST
%token HEX_CONST
%token CHAR_CONST
%token LOC_NAME

%{
#include <locale.h>
#include <sys/localedef.h>
#include <sys/method.h>
#include "err.h"
#include "symtab.h"
#include "semstack.h"
#include "locdef.h"

symtab_t cm_symtab;

extern _LC_charmap_t  charmap;
extern _LC_collate_t  collate;
extern _LC_ctype_t    ctype;
extern _LC_monetary_t monetary;
extern _LC_numeric_t  numeric; 
extern _LC_time_t     lc_time;
extern _LC_resp_t     resp;

extern int next_bit;	    /* sem_ctype.c : holds next bitmask for ctype properties */

extern symbol_t *sem_get_coll_tgt();

extern char yytext[];
extern char sym_text[];
extern int  skip_to_EOL;

int method_class=SB_CODESET;
int mb_cur_max=0;
int max_disp_width=0;
int sort_mask = 0;
int cur_order = 0;

/* Flags for determining if the category was empty when it was defined and
   if it has been defined before */

static int lc_time_flag = 0;
static int lc_monetary_flag = 0;
static int lc_ctype_flag = 0;
static int lc_message_flag = 0;
static int lc_numeric_flag = 0;
static int arblen;
static int user_defined = 0;

_LC_weight_t weights;
symbol_t *coll_tgt;
%}

%%
/* ----------------------------------------------------------------------
** GRAMMAR for files parsed by the localedef utility.  This grammar 
** supports both the CHARMAP and the LOCSRC definitions.  The 
** implementation will call yyparse() twice.  Once to parse the CHARMAP 
** file, and once to parse the LOCSRC file.
** ----------------------------------------------------------------------*/

file    : charmap
    	| category_list
        | method_def
	;

category_list :
   	category_list category
	| category
	;

/* ----------------------------------------------------------------------
** CHARMAP GRAMMAR 
**
** This grammar parses the charmap file as specified by POSIX 1003.2.
** ----------------------------------------------------------------------*/
charmap : charmap charmap_sect
	| charmap_sect
	;

charmap_sect : 
	metasymbol_assign
	| KW_CHARMAP '\n' charmap_stat_list KW_END KW_CHARMAP '\n'
	{
	    check_digit_values();
	}
        | charsets_def
	;

charmap_stat_list : 
        charmap_stat_list charmap_stat
        | charmap_stat
	;

charmap_stat :
	symbol_def
  	| symbol_range_def
	;

symbol_range_def :
	symbol KW_ELLIPSES symbol byte_list {skip_to_EOL++;} '\n'
	{
	    sem_symbol_range_def();
	}
	;

symbol_def :
	symbol byte_list {skip_to_EOL++;} '\n'
	{
	    sem_symbol_def();
	}
	;

metasymbol_assign : 
	KW_MB_CUR_MAX number '\n'
  	{
	    item_t *it;
	  
	    it = sem_pop();
	    if (it->type != SK_INT)
		INTERNAL_ERROR;

	    mb_cur_max = it->value.int_no;

	    sem_push(it);

	    /* If the user supplied his own methods, check_methods()
	       has already been called. */

	    if (method_class != USR_CODESET) {
		check_methods();	/* Insure that required methods are present */
	    }
	    sem_set_sym_val("<mb_cur_max>", SK_INT);
	}
	| KW_MB_CUR_MIN number '\n'
  	{
	    item_t *it;

	    it = sem_pop();
	    if (it->type != SK_INT)
	       INTERNAL_ERROR;
	    if (it->value.int_no != 1) {
		diag_error(ERR_INV_MB_CUR_MIN,it->value.int_no);
		destroy_item(it);
	    }
	    else {
	        sem_push(it);
	        sem_set_sym_val("<mb_cur_min>", SK_INT);
	    }

	}
	| KW_CODESET text '\n'
  	{
	    item_t *it;
	    int i;

	    /* The code set name must consist of character in the PCS -
	       which is analagous to doing an isgraph in the C locale */

	    it = sem_pop();
	    if (it->type != SK_STR)
	       INTERNAL_ERROR;
	    for (i=0; yytext[i] != '\0'; i++){
	        if (!isgraph(yytext[i]))
		   error(ERR_INV_CODE_SET_NAME,yytext);
	    }
	    sem_push(it);
	    sem_set_sym_val("<code_set_name>", SK_STR);
	    charmap.cm_csname = MALLOC(char,strlen(yytext)+1);
	    strcpy(charmap.cm_csname, yytext);
	}
	;

/* ----------------------------------------------------------------------
** LOCSRC GRAMMAR 
**
** This grammar parses the LOCSRC file as specified by POSIX 1003.2.
** ----------------------------------------------------------------------*/
category : regular_category 
	{
	    if (user_defined)
		diag_error(ERR_USER_DEF);
	}
	| non_reg_category
	;

regular_category : '\n' 
	| lc_collate
	| lc_ctype
	| lc_monetary
	| lc_numeric
	| lc_msg
	| lc_time
  	;

non_reg_category : unrecognized_cat
	;

/* ----------------------------------------------------------------------
** LC_COLLATE
**
** This section parses the LC_COLLATE category section of the LOCSRC
** file.
** ----------------------------------------------------------------------*/

lc_collate : 
	coll_sect_hdr coll_stats order_spec KW_END KW_LC_COLLATE '\n'
	{ 
	    sem_collate(); 
	}
        | coll_sect_hdr order_spec KW_END KW_LC_COLLATE '\n'
	{ 
	    sem_collate(); 
	}
	| coll_sect_hdr	KW_COPY locale_name '\n' KW_END KW_LC_COLLATE '\n'
	{
	    copy_locale(LC_COLLATE);
	}
	;

coll_sect_hdr :
    	KW_LC_COLLATE '\n'
	{
	  sem_init_colltbl();
	}
	;

coll_stats :
  	coll_stats coll_stat
	| coll_stat
	;

coll_stat : '\n'
        | KW_COLLATING_ELEMENT symbol KW_FROM string '\n'
    	{
	    sem_def_collel();
	}
	| KW_COLLATING_SYMBOL symbol '\n'
	{
	    sem_spec_collsym();
	}
	| KW_SUBSTITUTE string KW_WITH string '\n'
	{
	    sem_def_substr();
	}
	;

order_spec : 
	KW_ORDER_START sort_sect coll_spec_list KW_ORDER_END
	| KW_ORDER_START sort_sect coll_spec_list KW_ORDER_END white_space
  	;

white_space : white_space '\n'
        | '\n'
        ;

sort_sect : '\n'
	{
	    item_t *i;

	    i = create_item(SK_INT, _COLL_FORWARD_MASK);
	    sem_push(i);

	    collate.co_nord++;

	    sem_sort_spec();
	    if (collate.co_nsubs > 0)
	        setup_substr();
	}
	| sort_modifier_spec '\n'
	{
	    sem_sort_spec();
	    if (collate.co_nsubs > 0)
	        setup_substr();
	}
	;


sort_modifier_spec :
	sort_modifier_spec ';' sort_modifier_list
	{
	    if (collate.co_nord == COLL_WEIGHTS_MAX) 
		diag_error(ERR_COLL_WEIGHTS);
	    collate.co_nord ++;
	}
	| sort_modifier_list
	{
	    if (collate.co_nord == COLL_WEIGHTS_MAX) 
		diag_error(ERR_COLL_WEIGHTS);
	    collate.co_nord ++;
	}
	;

sort_modifier_list :
	sort_modifier_list ',' sort_modifier
	{
	    item_t *i;
	
	    /* The forward and backward mask are mutually exclusive */
	    /* Ignore the second mask and continue processing       */

	    i = sem_pop();
	    if (((i->value.int_no & _COLL_FORWARD_MASK) 
	           && (sort_mask == _COLL_BACKWARD_MASK)) 
	       ||
	       ((i->value.int_no & _COLL_BACKWARD_MASK)
	          && (sort_mask == _COLL_FORWARD_MASK))) {
		diag_error(ERR_FORWARD_BACKWARD);
		sem_push(i);
	    }
	    else {
	        i->value.int_no |= sort_mask;
	        sem_push(i);
	    }
	}
	| sort_modifier
	{
	    item_t *i;

	    i = create_item(SK_INT, sort_mask);
	    sem_push(i);
	}
	;

sort_modifier :
	KW_FORWARD             { sort_mask = _COLL_FORWARD_MASK;  }
	| KW_BACKWARD          { sort_mask = _COLL_BACKWARD_MASK; }
	| KW_NO_SUBSTITUTE     { sort_mask = _COLL_NOSUBS_MASK;   }
	| KW_POSITION          { sort_mask = _COLL_POSITION_MASK; }
	;

coll_spec_list :
        coll_spec_list coll_symbol_ref '\n'
  	{
	    sem_set_dflt_collwgt();
	}
	| coll_symbol_ref '\n'
  	{
	    sem_set_dflt_collwgt();
	}
	| coll_spec_list char_coll_range
	| char_coll_range
        | coll_spec_list coll_ell_spec
        | coll_ell_spec
        | coll_spec_list '\n'
        | '\n'
        ;

char_coll_range :
	coll_symbol_ref '\n' KW_ELLIPSES '\n' coll_symbol_ref '\n'
	{
	    sem_set_dflt_collwgt_range();
	}
        ;

coll_ell_spec :
       	coll_symbol_ref { coll_tgt = sem_get_coll_tgt(); } coll_rhs_list '\n'
    	{
	    sem_set_collwgt(&weights, cur_order);
	}
	;

coll_rhs_list :
     	coll_rhs_list ';' coll_ell_list
	{
	    sem_collel_list(&weights, coll_tgt, ++cur_order);
	}
	| coll_ell_list
	{
	    sem_collel_list(&weights, coll_tgt, cur_order=0);
	}
	;

coll_ell_list :
	coll_ell_list coll_symbol_ref
	{
	    item_t *i;

	    i = sem_pop();
	    if (i==NULL || i->type != SK_INT)
	  	INTERNAL_ERROR;
	    i->value.int_no++;
	    sem_push_collel();
	    sem_push(i);
	}
	| coll_symbol_ref
	{
	    item_t *i;

	    i = create_item(SK_INT, 1);
	    sem_push_collel();
	    sem_push(i);
	}
	;

coll_symbol_ref : 
        char_symbol_ref
        {   
	    sem_coll_sym_ref();
        }
	| byte_list
	{
	    sem_coll_literal_ref();
	    sem_coll_sym_ref();
	}
        ;
        
/* -----------------------------------------------------------------------
** LC_CTYPE
**
** This section parses the LC_CTYPE category section of the LOCSRC
** file.
** ----------------------------------------------------------------------*/

lc_ctype :
	KW_LC_CTYPE '\n' 
	{ 
	    /* The LC_CTYPE category can only be defined once in a file */

	    if (lc_ctype_flag)
	        diag_error(ERR_DUP_CATEGORY,"LC_CTYPE");
        } 
        lc_ctype_spec_list KW_END KW_LC_CTYPE '\n'
	{
	    /* A category with no text is an error (POSIX) */

	    if (!lc_ctype_flag)
	        diag_error(ERR_EMPTY_CAT,"LC_CTYPE");
	    else {
		check_upper();
		check_lower();
		check_alpha();
		check_space();
		check_cntl();
		check_punct();
		check_graph();
		check_print();
		check_digits();
		check_xdigit();
	    }
	}
	| KW_LC_CTYPE '\n' KW_COPY locale_name '\n' KW_END KW_LC_CTYPE '\n'
	{
	    copy_locale(LC_CTYPE);
	}
	| KW_LC_CTYPE '\n' KW_END KW_LC_CTYPE '\n'
	{
	    lc_ctype_flag++;

	    /* A category with no text is an error (POSIX) */

	    diag_error(ERR_EMPTY_CAT,"LC_CTYPE");

	}
	;

lc_ctype_spec_list :
	lc_ctype_spec_list lc_ctype_spec
	| lc_ctype_spec
	;

lc_ctype_spec : '\n'
        | symbol  char_range_list '\n'
	{
	    lc_ctype_flag++;
	    add_ctype(&ctype);
	}
	| KW_CHARCLASS ctype_proplist '\n'
        | KW_TOUPPER char_pair_list '\n'
        {
	    lc_ctype_flag++;
	    add_upper(&ctype);
        }
        | KW_TOLOWER char_pair_list '\n'
        {
	    lc_ctype_flag++;
	    add_lower(&ctype);
        }  
	;

ctype_proplist : ctype_proplist ';' ctype_prop
	| ctype_prop
	;

ctype_prop : STRING
	{
	    add_predef_classname(yytext, next_bit);
	    next_bit <<= 1;
	}
	| SYMBOL
	{
	    add_predef_classname(sym_text, next_bit);
	    next_bit <<= 1;
	}
	;

char_pair_list : char_pair_list ';' char_pair
       	| char_pair
	;

char_pair : '(' char_ref ',' char_ref ')'
        {
	    sem_push_xlat();
        }
	;

/* ----------------------------------------------------------------------
** LC_MONETARY
**
** This section parses the LC_MONETARY category section of the LOCSRC
** file.
** ----------------------------------------------------------------------*/

lc_monetary :
	KW_LC_MONETARY '\n' 
	{
	    /*  The LC_MONETARY category can only be defined once in a */
	    /*  locale						       */

	    if (lc_monetary_flag)
		diag_error(ERR_DUP_CATEGORY,"LC_MONETARY");
	 
	}
        lc_monetary_spec_list KW_END KW_LC_MONETARY '\n'
	{
	    /* A category must have at least one line of text (POSIX) */

	    if (!lc_monetary_flag)
	        diag_error(ERR_EMPTY_CAT,"LC_MONETARY");
	}
	| KW_LC_MONETARY '\n' KW_COPY locale_name '\n' KW_END KW_LC_MONETARY '\n'
	{
	    copy_locale(LC_MONETARY);
	}
	| KW_LC_MONETARY '\n' KW_END KW_LC_MONETARY '\n'
	{
	    lc_monetary_flag++;

	    /* A category must have at least one line of text (POSIX  */

	    diag_error(ERR_EMPTY_CAT,"LC_MONETARY");

	}
	;


lc_monetary_spec_list :
	lc_monetary_spec_list lc_monetary_spec
        | lc_monetary_spec_list '\n'
	{
	    lc_monetary_flag++;
	}
	| lc_monetary_spec
	{
	    lc_monetary_flag++;
	}
        | '\n'
	;

lc_monetary_spec :
  	KW_INT_CURR_SYMBOL string '\n'
	{
	    sem_set_str(&monetary.int_curr_symbol);
	}
	| KW_CURRENCY_SYMBOL string '\n'
	{
	    sem_set_str(&monetary.currency_symbol);
        }
	| KW_MON_DECIMAL_POINT string '\n'
	{ 
	    sem_set_str(&monetary.mon_decimal_point); 
	}
	| KW_MON_THOUSANDS_SEP string '\n'  
	{
	    sem_set_str(&monetary.mon_thousands_sep);
	}
	| KW_CREDIT_SIGN string '\n'
	{
	    sem_set_str(&monetary.credit_sign);
	}
	| KW_DEBIT_SIGN string '\n' 
        {
	    sem_set_str(&monetary.debit_sign);
	}
	| KW_POSITIVE_SIGN string '\n'
	{
	    sem_set_str(&monetary.positive_sign);
	}
	| KW_NEGATIVE_SIGN string '\n'
	{
	    sem_set_str(&monetary.negative_sign);
	}
	| KW_LEFT_PARENTHESIS string '\n'
	{
	    sem_set_str(&monetary.left_parenthesis);
	}
	| KW_RIGHT_PARENTHESIS string '\n'
	{
	    sem_set_str(&monetary.right_parenthesis);
	}
	| KW_MON_GROUPING digit_list '\n'
	{
	    sem_set_diglist(&monetary.mon_grouping);
	}
	| KW_INT_FRAC_DIGITS number '\n'
	{
	    sem_set_int(&monetary.int_frac_digits);
	}
	| KW_FRAC_DIGITS number '\n'
	{
	    sem_set_int(&monetary.frac_digits);
	}
	| KW_P_CS_PRECEDES number '\n'
	{
	    sem_set_int(&monetary.p_cs_precedes);
	}
	| KW_P_SEP_BY_SPACE number '\n'
	{
	    sem_set_int(&monetary.p_sep_by_space);
	}
	| KW_N_CS_PRECEDES number '\n'
	{
	    sem_set_int(&monetary.n_cs_precedes);
	}
	| KW_N_SEP_BY_SPACE number '\n'
	{
	    sem_set_int(&monetary.n_sep_by_space);
	}
	| KW_P_SIGN_POSN number '\n'
	{
	    sem_set_int(&monetary.p_sign_posn);
	}
	| KW_N_SIGN_POSN number '\n'
	{
	    sem_set_int(&monetary.n_sign_posn);
	}
	;

/* ----------------------------------------------------------------------
** LC_MSG
**
** This section parses the LC_MSG category section of the LOCSRC
** file.
** ----------------------------------------------------------------------*/

lc_msg :
	KW_LC_MSG '\n' 
	{
	    if (lc_message_flag)
	        diag_error(ERR_DUP_CATEGORY,"LC_MESSAGE");

	}
	lc_msg_spec_list KW_END KW_LC_MSG '\n'
	{
	    if (!lc_message_flag)
		diag_error(ERR_EMPTY_CAT,"LC_MESSAGE");
	}
	| KW_LC_MSG '\n' KW_COPY locale_name '\n' KW_END KW_LC_MSG '\n'
	{
	    copy_locale(LC_MESSAGES);
	}
	| KW_LC_MSG '\n' KW_END KW_LC_MSG '\n'
	{
	    lc_message_flag++;

	    diag_error(ERR_EMPTY_CAT,"LC_MESSAGE");

	}
	;

lc_msg_spec_list :
	lc_msg_spec_list lc_msg_spec
	| lc_msg_spec_list '\n'
	{
	    lc_message_flag++;
	}
	| lc_msg_spec
	{
	    lc_message_flag++;
	}
        | '\n'
	;

lc_msg_spec :
	KW_YESEXPR string '\n'
	{
	    sem_set_str(&resp.yesexpr);
	}
	| KW_NOEXPR string '\n'
        {
	    sem_set_str(&resp.noexpr);
	}
        | KW_YESSTR string '\n'
        {
	    sem_set_str(&resp.yesstr);
	}
	| KW_NOSTR string '\n'
	{
	    sem_set_str(&resp.nostr);
	}
	;

/* ----------------------------------------------------------------------
** LC_NUMERIC
**
** This section parses the LC_NUMERIC category section of the LOCSRC
** file.
** ----------------------------------------------------------------------*/

lc_numeric :
	KW_LC_NUMERIC '\n' 
	{
	    if (lc_numeric_flag)
		diag_error(ERR_DUP_CATEGORY,"LC_NUMERIC");

	}
	lc_numeric_spec_list KW_END KW_LC_NUMERIC '\n'
	{
	    if (!lc_numeric_flag)
		diag_error(ERR_EMPTY_CAT,"LC_NUMERIC");
	}
	| KW_LC_NUMERIC '\n' KW_COPY locale_name '\n' KW_END KW_LC_NUMERIC '\n'
	{
	    copy_locale(LC_NUMERIC);
	}
	| KW_LC_NUMERIC '\n' KW_END KW_LC_NUMERIC '\n'
	{
	    lc_numeric_flag++;
	    diag_error(ERR_EMPTY_CAT,"LC_NUMERIC");
	}
	;

lc_numeric_spec_list :
	lc_numeric_spec_list lc_numeric_spec
	| lc_numeric_spec
	{
	    lc_numeric_flag++;
	}
        | lc_numeric_spec_list '\n'
	{
	    lc_numeric_flag++;
	}
        | '\n'
	;


lc_numeric_spec :
	KW_DECIMAL_POINT string '\n'
	{
	    sem_set_str(&numeric.decimal_point);
	    if (numeric.decimal_point == NULL) {
	      numeric.decimal_point = "";
	      diag_error(ERR_ILL_DEC_CONST, "");
	    }
	}
	| KW_THOUSANDS_SEP string '\n'
        {
	    sem_set_str(&numeric.thousands_sep);
	}
	| KW_GROUPING digit_list '\n'
        {
	    sem_set_diglist(&numeric.grouping);
	}
	;

/* ----------------------------------------------------------------------
** LC_TIME
**
** This section parses the LC_TIME category section of the LOCSRC
** file.
** ----------------------------------------------------------------------*/

lc_time :
	KW_LC_TIME '\n' 
	{
	    if (lc_time_flag)
	 	diag_error(ERR_DUP_CATEGORY,"LC_TIME");

	}
	lc_time_spec_list KW_END KW_LC_TIME '\n'
	{
	    if (!lc_time_flag)
		diag_error(ERR_EMPTY_CAT,"LC_TIME");
	}
	| KW_LC_TIME '\n' KW_COPY locale_name '\n' KW_END KW_LC_TIME '\n'
	{
	    copy_locale(LC_TIME);
	}
	| KW_LC_TIME '\n' KW_END KW_LC_TIME '\n'
	{
	    lc_time_flag++;

	    diag_error(ERR_EMPTY_CAT,"LC_TIME");

	}
	;

lc_time_spec_list :
	lc_time_spec_list lc_time_spec
	{
	    lc_time_flag++;
	}
	| lc_time_spec
	{
	    lc_time_flag++;
	}
        | lc_time_spec_list '\n'
	{
	    lc_time_flag++;
	}
        | '\n'
	;

lc_time_spec :
	KW_ABDAY string_list '\n'
        {
	    sem_set_str_lst(lc_time.abday,7);
	}
	| KW_DAY string_list '\n'
	{
	    sem_set_str_lst(lc_time.day,7);
	}
	| KW_ABMON string_list '\n'
	{
	    sem_set_str_lst(lc_time.abmon,12);
	}
	| KW_MON string_list '\n'
	{
	    sem_set_str_lst(lc_time.mon,12);
	}
	| KW_D_T_FMT string '\n'
	{
	    sem_set_str(&lc_time.d_t_fmt);
	}
	| KW_D_FMT string '\n'
	{
	    sem_set_str(&lc_time.d_fmt);
	}
	| KW_T_FMT string '\n'
	{
	    sem_set_str(&lc_time.t_fmt);
	}
	| KW_AM_PM string_list '\n'
	{
	    sem_set_str_lst(lc_time.am_pm,2);
	}
	| KW_T_FMT_AMPM string '\n'
	{
	    sem_set_str(&lc_time.t_fmt_ampm);
	}
	| KW_ERA {arblen=0;} arblist '\n'
	{
	    char **arbp = MALLOC(char*, arblen+1);

	    sem_set_str_lst(arbp,arblen);
	    arbp[arblen] = NULL;
	    lc_time.era = arbp;
	}
	| KW_ERA_YEAR string '\n'
	{
	    sem_set_str(&lc_time.era_year);
	}
	| KW_ERA_D_FMT string '\n' 
	{
	    sem_set_str(&lc_time.era_d_fmt);
	}
	| KW_ERA_D_T_FMT string '\n'
	{
	    sem_set_str(&lc_time.era_d_t_fmt);
	}
	| KW_ERA_T_FMT string '\n'
	{
	    sem_set_str(&lc_time.era_t_fmt);
	}
	| KW_ALT_DIGITS {arblen=0;} arblist '\n'
      	{
	    char **arbp = MALLOC(char*, arblen+1);
	    int i;

	    sem_set_str_lst(arbp,arblen);
	    arbp[arblen] = NULL;
	    if (arblen > 0) {
		sem_conv_str(&lc_time.alt_digits, arbp, arblen);
	        for (i = 0; i < arblen; i++)
		    free(arbp[i]);
	    } else
		lc_time.alt_digits = "";
	    free(arbp);
	}
	| KW_M_D_RECENT string '\n'
	{
	    sem_set_str(&lc_time.m_d_recent);
	}
	| KW_M_D_OLD string '\n'
	{
	    sem_set_str(&lc_time.m_d_old);
	}
	;

arblist :
	arblist ';' string
	{
	    arblen++;
	}
	| string
	{
	    arblen++;
	}
	;


unrecognized_cat :
	SYMBOL '\n'
	{
	    int token;

	    user_defined++;
	    while ((token = yylex()) != KW_END);	    
	}
	SYMBOL '\n'
	{
	    diag_error(ERR_UNDEF_CAT,yytext);
	}
	;

/* ----------------------------------------------------------------------
** METHODS
**
** This section defines the grammar which parses the methods file.
** ----------------------------------------------------------------------*/

method_def : 
  	KW_METHODS '\n' 
	{
	    method_class = USR_CODESET;
	}
	method_assign_list KW_END KW_METHODS '\n'
	{
	    check_methods();		/* Load the required methods NOW! */
	}
	;

method_assign_list : 
	method_assign_list method_assign
	| method_assign_list '\n'
	| method_assign
        | '\n'
	;

method_assign : 
	KW_MBLEN string '\n'
	{
	    set_method(CHARMAP_MBLEN,1);
	}
	| KW_MBLEN string string '\n'
	{
	    set_method(CHARMAP_MBLEN,2);
	}
	| KW_MBLEN string string string '\n'
	{
	    set_method(CHARMAP_MBLEN,3);
	}
	| KW_MBTOWC string '\n'
	{
	    set_method(CHARMAP_MBTOWC,1);
	}
	| KW_MBTOWC string string '\n'
	{
	    set_method(CHARMAP_MBTOWC,2);
	}
	| KW_MBTOWC string string string '\n'
	{
	    set_method(CHARMAP_MBTOWC,3);
	}
	| KW_MBSTOWCS string '\n'
	{
	    set_method(CHARMAP_MBSTOWCS,1);
	}
	| KW_MBSTOWCS string string '\n'
	{
	    set_method(CHARMAP_MBSTOWCS,2);
	}
	| KW_MBSTOWCS string string string '\n'
	{
	    set_method(CHARMAP_MBSTOWCS,3);
	}
	| KW_WCTOMB string '\n'
	{
	    set_method(CHARMAP_WCTOMB,1);
	}
	| KW_WCTOMB string string '\n'
	{
	    set_method(CHARMAP_WCTOMB,2);
	}
	| KW_WCTOMB string string string '\n'
	{
	    set_method(CHARMAP_WCTOMB,3);
	}
	| KW_WCSTOMBS string '\n'
	{
	    set_method(CHARMAP_WCSTOMBS,1);
	}
	| KW_WCSTOMBS string string '\n'
	{
	    set_method(CHARMAP_WCSTOMBS,2);
	}
	| KW_WCSTOMBS string string string '\n'
	{
	    set_method(CHARMAP_WCSTOMBS,3);
	}
	| KW_WCWIDTH string '\n'
	{
	    set_method(CHARMAP_WCWIDTH,1);
	}
	| KW_WCWIDTH string string '\n'
	{
	    set_method(CHARMAP_WCWIDTH,2);
	}
	| KW_WCWIDTH string string string '\n'
	{
	    set_method(CHARMAP_WCWIDTH,3);
	}
	| KW_WCSWIDTH string '\n'
	{
	    set_method(CHARMAP_WCSWIDTH,1);
	}
	| KW_WCSWIDTH string string '\n'
	{
	    set_method(CHARMAP_WCSWIDTH,2);
	}
	| KW_WCSWIDTH string string string '\n'
	{
	    set_method(CHARMAP_WCSWIDTH,3);
	}
	| KW_MBTOPC string '\n'
	{
	    set_method(CHARMAP___MBTOPC,1);
	}
	| KW_MBTOPC string string '\n'
	{
	    set_method(CHARMAP___MBTOPC,2);
	}
	| KW_MBTOPC string string string '\n'
	{
	    set_method(CHARMAP___MBTOPC,3);
	}
	| KW_MBSTOPCS string '\n'
	{
	    set_method(CHARMAP___MBSTOPCS,1);
	}
	| KW_MBSTOPCS string string '\n'
	{
	    set_method(CHARMAP___MBSTOPCS,2);
	}
	| KW_MBSTOPCS string string string '\n'
	{
	    set_method(CHARMAP___MBSTOPCS,3);
	}
	| KW_PCTOMB string '\n'
	{
	    set_method(CHARMAP___PCTOMB,1);
	}
	| KW_PCTOMB string string '\n'
	{
	    set_method(CHARMAP___PCTOMB,2);
	}
	| KW_PCTOMB string string string '\n'
	{
	    set_method(CHARMAP___PCTOMB,3);
	}
	| KW_PCSTOMBS string '\n'
	{
	    set_method(CHARMAP___PCSTOMBS,1);
	}
	| KW_PCSTOMBS string string '\n'
	{
	    set_method(CHARMAP___PCSTOMBS,2);
	}
	| KW_PCSTOMBS string string string '\n'
	{
	    set_method(CHARMAP___PCSTOMBS,3);
	}
	| KW_TOWUPPER string '\n'
	{
	    set_method(CTYPE_TOWUPPER,1);
	}
	| KW_TOWUPPER string string '\n'
	{
	    set_method(CTYPE_TOWUPPER,2);
	}
	| KW_TOWUPPER string string string '\n'
	{
	    set_method(CTYPE_TOWUPPER,3);
	}
	| KW_TOWLOWER string '\n'
	{
	    set_method(CTYPE_TOWLOWER,1);
	}
	| KW_TOWLOWER string string '\n'
	{
	    set_method(CTYPE_TOWLOWER,2);
	}
	| KW_TOWLOWER string string string '\n'
	{
	    set_method(CTYPE_TOWLOWER,3);
	}
	| KW_WCTYPE string '\n'
	{
	    set_method(CTYPE_WCTYPE,1);
	}
	| KW_WCTYPE string string '\n'
	{
	    set_method(CTYPE_WCTYPE,2);
	}
	| KW_WCTYPE string string string '\n'
	{
	    set_method(CTYPE_WCTYPE,3);
	}
	| KW_ISWCTYPE string '\n'
	{
	    set_method(CTYPE_ISWCTYPE,1);
	}
	| KW_ISWCTYPE string string '\n'
	{
	    set_method(CTYPE_ISWCTYPE,2);
	}
	| KW_ISWCTYPE string string string '\n'
	{
	    set_method(CTYPE_ISWCTYPE,3);
	}
	| KW_STRCOLL string '\n'
	{
	    set_method(COLLATE_STRCOLL,1);
	}
	| KW_STRCOLL string string '\n'
	{
	    set_method(COLLATE_STRCOLL,2);
	}
	| KW_STRCOLL string string string '\n'
	{
	    set_method(COLLATE_STRCOLL,3);
	}
	| KW_STRXFRM string '\n'
	{
	    set_method(COLLATE_STRXFRM,1);
	}
	| KW_STRXFRM string string '\n'
	{
	    set_method(COLLATE_STRXFRM,2);
	}
	| KW_STRXFRM string string string '\n'
	{
	    set_method(COLLATE_STRXFRM,3);
	}
	| KW_WCSCOLL string '\n'
	{
	    set_method(COLLATE_WCSCOLL,1);
	}
	| KW_WCSCOLL string string '\n'
	{
	    set_method(COLLATE_WCSCOLL,2);
	}
	| KW_WCSCOLL string string string '\n'
	{
	    set_method(COLLATE_WCSCOLL,3);
	}
	| KW_WCSXFRM string '\n'
	{
	    set_method(COLLATE_WCSXFRM,1);
	}
	| KW_WCSXFRM string string '\n'
	{
	    set_method(COLLATE_WCSXFRM,2);
	}
	| KW_WCSXFRM string string string '\n'
	{
	    set_method(COLLATE_WCSXFRM,3);
	}
	| KW_REGCOMP string '\n'
	{
	    set_method(COLLATE_REGCOMP,1);
	}
	| KW_REGCOMP string string '\n'
	{
	    set_method(COLLATE_REGCOMP,2);
	}
	| KW_REGCOMP string string string '\n'
	{
	    set_method(COLLATE_REGCOMP,3);
	}
	| KW_REGEXEC string '\n'
	{
	    set_method(COLLATE_REGEXEC,1);
	}
	| KW_REGEXEC string string '\n'
	{
	    set_method(COLLATE_REGEXEC,2);
	}
	| KW_REGEXEC string string string '\n'
	{
	    set_method(COLLATE_REGEXEC,3);
	}
	| KW_REGFREE string '\n'
	{
	    set_method(COLLATE_REGFREE,1);
	}
	| KW_REGFREE string string '\n'
	{
	    set_method(COLLATE_REGFREE,2);
	}
	| KW_REGFREE string string string '\n'
	{
	    set_method(COLLATE_REGFREE,3);
	}
	| KW_REGERROR string '\n'
	{
	    set_method(COLLATE_REGERROR,1);
	}
	| KW_REGERROR string string '\n'
	{
	    set_method(COLLATE_REGERROR,2);
	}
	| KW_REGERROR string string string '\n'
	{
	    set_method(COLLATE_REGERROR,3);
	}
	| KW_STRFMON string '\n'
	{
	    set_method(MONETARY_STRFMON,1);
	}
	| KW_STRFMON string string '\n'
	{
	    set_method(MONETARY_STRFMON,2);
	}
	| KW_STRFMON string string string '\n'
	{
	    set_method(MONETARY_STRFMON,3);
	}
	| KW_STRFTIME string '\n'
	{
	    set_method(TIME_STRFTIME,1);
	}
	| KW_STRFTIME string string '\n'
	{
	    set_method(TIME_STRFTIME,2);
	}
	| KW_STRFTIME string string string '\n'
	{
	    set_method(TIME_STRFTIME,3);
	}
	| KW_STRPTIME string '\n'
	{
	    set_method(TIME_STRPTIME,1);
	}
	| KW_STRPTIME string string '\n'
	{
	    set_method(TIME_STRPTIME,2);
	}
	| KW_STRPTIME string string string '\n'
	{
	    set_method(TIME_STRPTIME,3);
	}
	| KW_WCSFTIME string '\n'
	{
	    set_method(TIME_WCSFTIME,1);
	}
	| KW_WCSFTIME string string '\n'
	{
	    set_method(TIME_WCSFTIME,2);
	}
	| KW_WCSFTIME string string string '\n'
	{
	    set_method(TIME_WCSFTIME,3);
	}
        ;   

/* ----------------------------------------------------------------------
** CHARSETID
**
** This section defines the grammar which parses the character set id
** classification of characters.
**
**	THIS IS NOT SUPPORTED or USED in OSF/1!!!
** ----------------------------------------------------------------------*/

charsets_def : 
  	KW_CHARSETID '\n' charset_assign_list KW_END KW_CHARSETID '\n'
	{
	    diag_error(ERR_UNDEF_CAT,"CHARSETID");
	}
	;

charset_assign_list : 
	charset_assign_list charset_assign
	| charset_assign_list '\n'
	| charset_assign
        | '\n'
	;


charset_assign : 
	charset_range_assign 
	{
	    /* sem_charset_range_def(&charmap); */
	}
	| charset_simple_assign
	{
	    /* sem_charset_def(&charmap); */
	}
	;

charset_range_assign :
	char_symbol_ref KW_ELLIPSES char_symbol_ref const '\n'
	;

charset_simple_assign :
	char_symbol_ref const '\n'
	;

/* ----------------------------------------------------------------------
** GENERAL
**
** This section parses the syntatic elements shared by one or more of
** the above.
** ----------------------------------------------------------------------*/

digit_list : digit_list ';' number
    	{
	    /* add the new number to the digit list */
	    sem_digit_list();
	}
	| number
	{
	    item_t *i;

	    /* create count and push on top of stack */
	    i = create_item(SK_INT, 1);
	    sem_push(i);
	}
        ;

char_range_list : char_range_list ';' ctype_symbol
        | char_range_list ';' KW_ELLIPSES ';' char_ref
        {
	    push_char_range();
	}  
        | ctype_symbol
	;

ctype_symbol : char_ref
        {
	    push_char_sym();
	}
        ;

char_ref : char_symbol_ref
	{
	    sem_char_ref();
	}
	| const
	| byte_list
	;

char_symbol_ref : SYMBOL
    	{
	    sem_existing_symbol(sym_text);
	}
	;

symbol  : SYMBOL
        {
	    sem_symbol(sym_text);
	}
	;

const   : int_const
	;

string_list : string_list ';' string
	| string
	;

text	: string
	| SYMBOL
	{
	    item_t *i = create_item(SK_STR, sym_text);
	    sem_push(i);
	}
	;

string	: STRING
	{
	    item_t *i;
	    
	    i = create_item(SK_STR, yytext);
	    sem_push(i);
	}
	;

int_const : INT_CONST
        {
	    item_t *i;
	    char *junk;
	    
	    i = create_item(SK_INT, strtol(yytext, &junk, 10));
	    sem_push(i);
	}
        ;
	
byte_list : CHAR_CONST
        {
	    extern int value;
	    item_t *it;
	    
	    it = create_item(SK_INT, value);
	    sem_push(it);
        } 
        ;
number	: byte_list
	{
	    item_t	*it,*ct;
	    int		c;

	    ct = sem_pop();
	    if (ct->type != SK_INT)
		INTERNAL_ERROR;

	    c = ct->value.int_no;
	    if (c > '9' || c < '0') {
		char s[16];
		if (isprint(c)) {
		    s[0] = c;
		    s[1] = '\0';
		} else
		    sprintf(s,"\\x%2x",c);

	      diag_error(ERR_ILL_DEC_CONST, s);
	    }

	    destroy_item(ct);
	    it = create_item(SK_INT, c-'0');
	    sem_push(it);
	}
	| string
	{
	    item_t *it,*st;
	    int i;
	    char *ep;

	    st = sem_pop();
	    if (st->type != SK_STR)
		INTERNAL_ERROR;

	    i = strtol(st->value.str, &ep, 10);
	    if (st->value.str == ep)
		diag_error(ERR_ILL_DEC_CONST, ep);

	    it = create_item(SK_INT, i);
	    destroy_item(st);
	    sem_push(it);
	}
	;

locale_name : LOC_NAME
	{
	    item_t *i;
	    
	    i = create_item(SK_STR, yytext);
	    sem_push(i);
		
	}
	;
%%

void	initgram(void) {
}
