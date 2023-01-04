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
/* xkeycaps, Copyright (c) 1991 Jamie Zawinski <jwz@lucid.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */


#include <X11/keysym.h>
#ifndef XK_Sys_Req
#define XK_Sys_Req      0xFF15
#endif
#ifndef DXK_macron
#define DXK_macron  0x1000feaf
#endif
#include "kbspecial.h"	/* for special data declarations */
#include "belgian.h"
#include "denmark.h"
#include "swiss_german.h"
#include "swiss_french.h"
#include "finnish.h"
#include "france.h"
#include "french_canadian.h"
#include "germany.h"
#include "italy.h"
#include "netherlands.h"
#include "norway.h"
#include "portugal.h"
#include "spain.h"
#include "sweden.h"
#include "uk_at.h"
#include "us_at.h"
#include "lkswiss_german.h"
#include "lkswiss_french.h"
#include "lknetherlands.h"
#include "lkfinnish.h"
#include "lkfrench_canadian.h"
#include "lkbelgian.h"
#include "lkdenmark.h"
#include "lkfrance.h"
#include "lkgermany.h"
#include "lkitaly.h"
#include "lknorway.h"
#include "lkportugal.h"
#include "lkspain.h"
#include "lksweden.h"
#include "lkuk_at.h"
#include "lkus_at.h"
/*
**      ANSI--lk401
*/
#include "austrian_german_lk401ag.h"
#include "belgian_french_lk401ap_dp.h"
#include "canadian_french_lk401ac_dp.h"
#include "danish_lk401ad_dp.h"
#include "dutch_us_lk401ah.h"
#include "finnish_lk401af_dp.h"
#include "flemish_lk401ab_dp.h"
#include "italian_lk401ai_dp.h"
#include "norwegian_lk401an_dp.h"
#include "portuguese_lk401av.h"
#include "spanish_lk401as_dp.h"
#include "swedish_lk401am_dp.h"
#include "swiss_french_lk401ak_dp.h"
#include "swiss_german_lk401al_dp.h"
#include "uk_lk401aa.h"

/*
**      ANSI--lk201
*/
#include "austrian_german_lk201ng_tw.h" /* germany */
#include "british_lk201le_tw.h"         /* great britain */
#include "canadian_french_lk201lc_tw.h" /* canada */
#include "danish_lk201rd_tw.h"          /* denmark */
#include "dutch_lk201lh_tw.h"           /* netherland */
#include "flemish_lk201lb_tw.h"         /* belgium */
#include "belgian_french_lk201lp_tw.h"  /* france */
#include "italian_lk201li_tw.h"         /* italy */
#include "norwegian_lk201rn_tw.h"       /* norway */
#include "portuguese_lk201lv.h"         /* portugal */
#include "spanish_lk201ls_tw.h"         /* spain */
#include "swedish_lk201nm_tw.h"         /* sweden */
#include "icelandic_lk201lu_tw.h"       /* finland */
#include "swiss_french_lk201lk_tw.h"    /* switzerland/french */
#include "swiss_german_lk201ll_tw.h"    /* switzerland/german */

#include "kbd-ncd-n101.h"
#include "kbd-ncd-n102.h"
#include "kbd-ncd-n102sf.h"
#include "kbd-ncd-n108.h"
#include "kbd-ncd-n97.h"
#include "kbd-ncd-vt220.h"
#include "kbd-dec-lk201.h"
#include "kbd-dec-lk401.h"
#include "kbd-dec-lk421.h"

/*
**	build the all_kbds list based on which of the table
**	entries provided here are reasonable for the keyboard
**	actually hooked up to the server. This means we have 
**	base keyboards (your basic lk.. and 101-, 102-key pc
**	style keyboards), with a separate list of international
**	keyboard entries (so the internationals get their own
**	sub-submenu) -bg
*/
struct keyboard **all_kbds = (struct keyboard **) 0;
struct keyboard *base_kbds [] = {
	&us_at,
	&DEC_LK401,
	&DEC_LK201,
	&lkus_at,
	&DEC_LK421,
	&NCD_N101,	/* this position makes *THIS* default NCD */
	&NCD_N102,
	&NCD_N102sf,
	&NCD_N108,
	&NCD_VT220,
	&NCD_N97,
0
};
struct keyboard *pc102s [] = {
	&denmark,		/* 0 DANSK -- danish */
	&germany,		/* 1 DEUTSCH -- austrian-german */
	&swiss_german,		/* 2 SCHWEIZ -- swiss-german */
	/*
	**      English/American is in base_kbds[]...
	*/
	&uk_at,			/* 3 BRITISH -- uk */
	&spain,			/* 4 ESPANOL -- spanish */
	&france,		/* 5 FRANCAIS -- french */
	&french_canadian, 	/* 6 CANADIEN -- french-canadian */
	&swiss_french,		/* 7 SUISSEROMANDE -- swiss-french */
	&italy,			/* 8 ITALIANO -- italian */
	&netherlands,		/* 9 NEDERLANDS -- dutch */
	&norway,		/* 10 NORSK -- norwegian */
	&portugal,		/* 11 PORTUGUES -- portguese */
	&finnish,		/* 12 SUOMI -- finland */
	&sweden,		/* 13 SVENSKA  -- swedish */
	&belgian,		/* 14 VLAAMS -- flemish */
	/*
	**      ANSI--LK401 family
	*/
	&danish_lk401ad_dp,     /* 0 DANSK -- danish */
	&austrian_german_lk401ag,/* 1 DEUTSCH -- austrian-german */
	&swiss_german_lk401al_dp,/* 2 SCHWEIZ -- swiss-german */
	/*
	**      English/American is in base_kbds[]...
	*/
	&uk_lk401aa,	    		/* 3 BRITISH -- uk */
	&spanish_lk401as_dp,    	/* 4 ESPANOL -- spanish */
	&belgian_french_lk401ap_dp,     /* 5 FRANCAIS -- french */
	&canadian_french_lk401ac_dp,	/* 6 CANADIEN -- french-canadian */
	&swiss_french_lk401ak_dp,       /* 7 SUISSEROMANDE -- swiss-french */
	&italian_lk401ai_dp,    	/* 8 ITALIANO -- italian */
	&dutch_us_lk401ah,	      	/* 9 NEDERLANDS -- dutch */
	&norwegian_lk401an_dp,  	/* 10 NORSK -- norwegian */
	&portuguese_lk401av,    	/* 11 PORTUGUES -- portguese */
	&finnish_lk401af_dp,    	/* 12 SUOMI -- finland */
	&swedish_lk401am_dp,    	/* 13 SVENSKA  -- swedish */
	&flemish_lk401ab_dp,    	/* 14 VLAAMS -- flemish */
	/*
	**      ANSI--LK201 family
	*/
	&danish_lk201rd_tw,     	/* 0 DANSK -- danish */
	&austrian_german_lk201ng_tw,	/* 1 DEUTSCH -- austrian-german */
       	&swiss_german_lk201ll_tw,      	/* 2 SCHWEIZ -- swiss-german */
       /*
	**      English/American is in base_kbds[]...
	*/
	&british_lk201le_tw,    	/* 3 BRITISH -- uk */
	&spanish_lk201ls_tw,    	/* 4 ESPANOL -- spanish */
	&belgian_french_lk201lp_tw,     /* 5 FRANCAIS -- french */
	&canadian_french_lk201lc_tw,	/* 6 CANADIEN -- french-canadian */
	&swiss_french_lk201lk_tw,       /* 7 SUISSEROMANDE -- swiss-french */
	&italian_lk201li_tw,    	/* 8 ITALIANO -- italian */
	&dutch_lk201lh_tw,      	/* 9 NEDERLANDS -- dutch */
	&norwegian_lk201rn_tw,  	/* 10 NORSK -- norwegian */
	&portuguese_lk201lv,    	/* 11 PORTUGUES -- portguese */
	&icelandic_lk201lu_tw,  	/* 12 SUOMI -- finland */
	&swedish_lk201nm_tw,    	/* 13 SVENSKA  -- swedish */
	&flemish_lk201lb_tw,    	/* 14 VLAAMS -- flemish */
	/*
	**      lk443/lk444 keymap tables
	**      must be in the same order as
	**      the pcxal keyboards... -bg
	*/
	&lkdenmark,	     		/* 0 DANSK -- danish */
	&lkgermany,	     		/* 1 DEUTSCH -- austrian-german */
	&lkswiss_german,		/* 2 SCHWEIZ -- swiss-german */
	/*
	**      English/American is in base_kbds[]...
	*/
	&lkuk_at,	       		/* 3 BRITISH -- uk */
	&lkspain,	       		/* 4 ESPANOL -- spanish */
	&lkfrance,	      		/* 5 FRANCAIS -- french */
	&lkfrench_canadian,     	/* 6 CANADIEN -- french-canadian */
	&lkswiss_french,		/* 7 SUISSEROMANDE -- swiss-french */
	&lkitaly,	       		/* 8 ITALIANO -- italian */
	&lknetherlands,	 		/* 9 NEDERLANDS -- dutch */
	&lknorway,	      		/* 10 NORSK -- norwegian */
	&lkportugal,	    		/* 11 PORTUGUES -- portguese */
	&lkfinnish,	     		/* 12 SUOMI -- finland */
	&lksweden,	      		/* 13 SVENSKA  -- swedish */
	&lkbelgian,	     		/* 14 VLAAMS -- flemish */
  0
};

/* When we have to guess, and haven't a clue.
   The Imakefile can define this to be 0, to make there be no default.
 */
#ifndef DEFAULT_KBD_NAME
#define DEFAULT_KBD_NAME	"LK401"
#endif
