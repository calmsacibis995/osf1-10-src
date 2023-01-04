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
static char     *sccsid = "@(#)$RCSfile: copy.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/10 22:07:17 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

#include <locale.h>
#include "locdef.h"
#include "semstack.h"
#include "err.h"

int 	copying_collate = 0;	/* are we copying a collation table */
int 	copying_ctype = 0;	/* are we copying a ctype table */
int	copying = 0;		/* general flag to indicate copying */

extern _LC_collate_t	*collate_ptr;
extern _LC_ctype_t	*ctype_ptr;
extern _LC_monetary_t	*monetary_ptr;
extern _LC_numeric_t	*numeric_ptr;
extern _LC_time_t	*lc_time_ptr;
extern _LC_resp_t	*resp_ptr;

/*
 * Copy_locale  - routine to copy section of locale input files
 * 		   from an existing, installed, locale.
 *
 * 	  We reassign pointers so gen() will use the existing structures.
 */

void
copy_locale(int category)
{
	char *ret;
	item_t	*it;
	char *source;		/* user provided locale to copy from */
	char *orig_loc;		/* orginal locale */

	it = sem_pop();
	if (it->type != SK_STR)
		INTERNAL_ERROR;
	source = it->value.str;

	orig_loc = setlocale(category, NULL);
	if ((ret = setlocale(category, source)) == NULL) 
		error(CANT_LOAD_LOCALE, source);

	copying = 1;			/* make sure gen() puts out C code */
	switch(category) {

		case LC_COLLATE:
		collate_ptr = __lc_collate;
		copying_collate = 1;		/* to avoid re-compressing */
		break;

		case LC_CTYPE:
		ctype_ptr = __lc_ctype;
		copying_ctype = 1;		/* to use max_{upper,lower} */
		break;

		case LC_MONETARY:
		monetary_ptr = __lc_monetary;
		break;

		case LC_NUMERIC:
		numeric_ptr = __lc_numeric;
		break;

		case LC_TIME:
		lc_time_ptr = __lc_time;
		break;

		case LC_MESSAGES:
		resp_ptr = __lc_resp;
		break;
	}

	ret = setlocale(category, orig_loc);
}
