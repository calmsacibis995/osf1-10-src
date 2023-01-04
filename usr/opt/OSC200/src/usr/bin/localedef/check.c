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
static char     *sccsid = "@(#)$RCSfile: check.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/09/30 20:36:40 $";
#endif
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
 * FUNCTIONS: check_upper, check_lower, check_alpha, check_space,
 *	      check_cntl, check_print, check_graph, check_punct
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.2  com/cmd/nls/check.c, cmdnls, bos320, 9132320m 8/9/91 15:04:53
 */
#include <sys/types.h>
#include <sys/localedef.h>
#include <ctype.h>
#include "semstack.h"
#include "symtab.h"
#include "err.h"

/* All these functions follow the same logic. For 0 through max_wchar_enc
   check the mask of the char. If the character has the characteristic
   being tested, check further to make sure it has none of the characteristic's
   that are invalid. (ie. an upper can not be a control, a punct, a digit, 
   or a space). These are all POSIX checks (from Draft 11). 

*/ 

void
check_upper()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;
	int upper=FALSE;

	for (i = 0 ; i <= max_wchar_enc; i++){
	    if (ctype._mask[i] & _ISUPPER) {
		upper=TRUE;
		if ((ctype._mask[i] & _ISCNTRL) ||
		    (ctype._mask[i] & _ISDIGIT) ||
		    (ctype._mask[i] & _ISPUNCT) ||
		    (ctype._mask[i] & _ISSPACE))
		    fail=TRUE;
	    }
	}
	if (fail)
	    diag_error(ERR_INVALID_UPPER);

	/* no upper specified, define default set */
	fail=FALSE;
	if (!upper)
	  for (i = 'A'; i <= 'Z'; i++) {
	    ctype._mask[i] |= _ISUPPER;
	    if ((ctype._mask[i] & _ISCNTRL) ||
		(ctype._mask[i] & _ISDIGIT) ||
		(ctype._mask[i] & _ISPUNCT) ||
		(ctype._mask[i] & _ISSPACE))
		fail=TRUE;
	  }
	if (fail)
	    diag_error(ERR_INVALID_UPPER);	
	return;
}

void
check_lower()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;
	int lower=FALSE;

	for (i = 0 ; i <= max_wchar_enc; i++){
	    if (ctype._mask[i] & _ISLOWER) {
	        lower=TRUE;
		if ((ctype._mask[i] & _ISCNTRL) ||
		    (ctype._mask[i] & _ISDIGIT) ||
		    (ctype._mask[i] & _ISPUNCT) ||
		    (ctype._mask[i] & _ISSPACE))
		    fail=TRUE;
	    }
	}
	if (fail)
 	    diag_error(ERR_INVALID_LOWER);

	/* no lower specified, define default set */
	fail=FALSE;
	if (!lower)
	  for (i='a'; i <= 'z'; i++) {
	    ctype._mask[i] |= _ISLOWER;
	    if ((ctype._mask[i] & _ISCNTRL) ||
		(ctype._mask[i] & _ISDIGIT) ||
		(ctype._mask[i] & _ISPUNCT) ||
	        (ctype._mask[i] & _ISSPACE))
		fail=TRUE;
	  }
	if (fail)
 	    diag_error(ERR_INVALID_LOWER);
	return;
}

void
check_alpha()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;

	for (i = 0 ; i <= max_wchar_enc; i++){
	    if (ctype._mask[i] & _ISALPHA) 
		if ((ctype._mask[i] & _ISCNTRL) ||
		    (ctype._mask[i] & _ISDIGIT) ||
		    (ctype._mask[i] & _ISPUNCT) ||
		    (ctype._mask[i] & _ISSPACE))
		    fail=TRUE;
	}
	if (fail)
	    diag_error(ERR_INVALID_ALPHA);
	return;
}

void
check_space()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;
	int space=FALSE;
	/* 
	 * default values for tab, newline, verticle tab, form
	 * feed, carriage return, and space 
	 */
	int def_space[6] = {'\t','\n',11,12,13,' '};


	for (i = 0 ; i <= max_wchar_enc; i++){
	    if (ctype._mask[i] & _ISSPACE) {
	        space=TRUE;
		if ((ctype._mask[i] & _ISDIGIT) ||
		    (ctype._mask[i] & _ISUPPER) ||
		    (ctype._mask[i] & _ISLOWER) ||
		    (ctype._mask[i] & _ISALPHA) ||
		    (ctype._mask[i] & _ISGRAPH) ||
		    (ctype._mask[i] & _ISXDIGIT))
		    fail=TRUE;
	    }
	}
	if (fail)
	    diag_error(ERR_INVALID_SPACE);

	/* no space specified, define default values */
	fail=FALSE;
	if (!space)
	  for (i = 0; i < (sizeof(def_space)/sizeof(def_space[0])); i++) {
	    ctype._mask[def_space[i]] |= _ISSPACE;
	    if ((ctype._mask[def_space[i]] & _ISDIGIT) ||
		(ctype._mask[def_space[i]] & _ISUPPER) ||
		(ctype._mask[def_space[i]] & _ISLOWER) ||
		(ctype._mask[def_space[i]] & _ISALPHA) ||
		(ctype._mask[def_space[i]] & _ISGRAPH) ||
		(ctype._mask[def_space[i]] & _ISXDIGIT))
		fail=TRUE;
	  }
	if (fail)
	    diag_error(ERR_INVALID_SPACE);
	return;
}

void
check_cntl()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;

	for (i = 0 ; i <= max_wchar_enc; i++){
	    if (ctype._mask[i] & _ISCNTRL) 
		if ((ctype._mask[i] & _ISUPPER) ||
		    (ctype._mask[i] & _ISLOWER) ||
		    (ctype._mask[i] & _ISALPHA) ||
		    (ctype._mask[i] & _ISDIGIT) ||
		    (ctype._mask[i] & _ISGRAPH) ||
		    (ctype._mask[i] & _ISPUNCT) ||
		    (ctype._mask[i] & _ISPRINT) ||
		    (ctype._mask[i] & _ISXDIGIT))
		    fail=TRUE;
	}
	if (fail)
	    diag_error(ERR_INVALID_CNTRL);
	return;
}

void
check_punct()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;

	for (i = 0 ; i <= max_wchar_enc; i++){
	    if (ctype._mask[i] & _ISPUNCT) 
		if ((ctype._mask[i] & _ISUPPER) ||
		    (ctype._mask[i] & _ISLOWER) ||
		    (ctype._mask[i] & _ISALPHA) ||
		    (ctype._mask[i] & _ISDIGIT) ||
		    (ctype._mask[i] & _ISCNTRL) ||
		    (ctype._mask[i] & _ISXDIGIT) ||
		    (ctype._mask[i] & _ISSPACE))
		    fail=TRUE;
	}
	if (fail)
	    diag_error(ERR_INVALID_PUNCT);
	return;
}
void
check_graph()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;

	for (i = 0 ; i <= max_wchar_enc; i++){
	    if (ctype._mask[i] & _ISGRAPH) 
		if (ctype._mask[i] & _ISCNTRL) 
		    fail=TRUE;
	}
	if (fail)
	    diag_error(ERR_INVALID_GRAPH);
	return;
}

void
check_print()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;

	for (i = 0 ; i <= max_wchar_enc; i++){
	    if (ctype._mask[i] & _ISPRINT) 
		if (ctype._mask[i] & _ISCNTRL) 
		    fail=TRUE;
	}
	if (fail)
	    diag_error(ERR_INVALID_PRINT);
	return;
}

void
check_digits()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;
	int digit=FALSE;
	
	/*
	 * if digit is specified 0-9 must be specified as digits
	 */
	for (i = 0; i <= max_wchar_enc; i++) {
	    if (ctype._mask[i] & _ISDIGIT)
		digit=TRUE;
	    if ((i >= '0') && (i <= '9'))
	        if (!(ctype._mask[i] & _ISDIGIT))
		    fail = TRUE;
	}
	        
	if (fail && digit)
	    diag_error(ERR_INV_DIGIT);

	/* no digit specified, defined default set */
	if (!digit)
	  for (i = '0'; i <= '9'; i++)
	    ctype._mask[i] |= _ISDIGIT;
	return;
}

void
check_xdigit()
{
	extern _LC_ctype_t ctype;
	extern wchar_t max_wchar_enc;   /* the maximum wchar encoding for 
						             this codeset */
	int i;
	int fail=FALSE;
	int xdigit=FALSE;
	
	/*
	 * if xdigit is specified, 0-9 must be specified as xdigits
	 */
	for (i = 0; i <= max_wchar_enc; i++) {
	    if (ctype._mask[i] & _ISXDIGIT)
	        xdigit=TRUE;
	    if ((i >= '0') && (i <= '9'))  {
		if (!(ctype._mask[i] & _ISXDIGIT))
		    fail=TRUE;
	    }	
	}
        if (fail && xdigit)
	    diag_error(ERR_INV_XDIGIT);

	/* xdigit not specified, define default values */
	if (!xdigit) {
	  for (i = '0'; i <= '9'; i++)
	    ctype._mask[i] |= _ISXDIGIT;
	  for (i = 'a'; i <= 'f'; i++)
	    ctype._mask[i] |= _ISXDIGIT;
	  for (i = 'A'; i <= 'F'; i++)
	    ctype._mask[i] |= _ISXDIGIT;
	}
     	return;
}	
