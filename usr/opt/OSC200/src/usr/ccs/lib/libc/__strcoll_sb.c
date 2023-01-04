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
static char *rcsid = "@(#)$RCSfile: __strcoll_sb.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/08 01:22:13 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: __strcoll_sb
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.4  com/lib/c/str/__strcoll_sb.c, libcstr, bos320, 9140320 9/24/91 15:45:42
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <sys/localedef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include "collation.h"

/*
 * FUNCTION: Compares the strings pointed to by str1 and str2, returning an
 *	     integer as follows:
 *
 *		Less than 0	If str1 is less than str2
 *		Equal to 0	If str1 is equal to str2
 *		Greater than 0	If str1 is greater than str2.
 *
 *	     The comparison is based on the collating sequence specified
 *	     by the locale category LC_COLLATE affected by the setlocale
 *	     function.
 *
 * NOTES:    The ANSI Programming Language C standard requires this routine.
 *
 * PARAMETERS: (Uses file codes )
 *	     char *str1 - first string
 *	     char *str2 - second string
 *
 * RETURN VALUE DESCRIPTIONS: Returns a negative, zero, or positive value
 *	     as described above.
 */

int 
__strcoll_sb( const char *str1, const char *str2, _LC_collate_t *hdl)
{
    
    char *str1_rep=(char *)NULL;
    char *str2_rep=(char *)NULL;

    char *str1_ptr; /* ptr to string to collate, either str1 or replacement */
    char *str2_ptr; /* ptr to string to collate, either str2 or replacement */

    int cur_order;  /* current order being collated */
    short sort_mod; /* the current order's modification params */

    int rc;         /* generic return code */
    
    /**********
      see if str1 and str2 are the same string
    **********/
    if (str1 == str2)
	return(0);
    
    /**********
      if str1 and str2 are null, they are equal
    **********/
    if (*str1 == '\0' && *str2 == '\0')
        return(0);

    for (cur_order=0; cur_order<= hdl->co_nord; cur_order++) {
	/**********
          get the sort modifier for this order
	**********/
        if (hdl->co_nord < _COLL_WEIGHTS_INLINE)
            sort_mod = hdl->co_sort.n[cur_order];
        else
            sort_mod = hdl->co_sort.p[cur_order];
	
	/**********
          if this order uses replacement strings, set them up
	**********/
        if (hdl->co_nsubs && !(sort_mod & _COLL_NOSUBS_MASK)) {
            (void)free (str1_rep);
            str1_rep = str1_ptr = __do_replacement(hdl, str1, cur_order);

            (void)free (str2_rep);
            str2_rep = str2_ptr = __do_replacement(hdl, str2, cur_order);
	}

        /**********
          otherwise use the strings as they came in
        **********/
        else {
            str1_ptr = (char *)str1;
            str2_ptr = (char *)str2;
	}
        /**********
          check for direction of collation for this order.
          If neither forward nor backward are specified, then
          this is to be done by character.
        **********/
        /**********
          CHARACTER: if it is character collation, return the
          value from char_collate.  It does all of the orders
        **********/
        if (sort_mod == 0) {
            rc = __char_collate_sb(hdl, str1_ptr, str2_ptr);
            (void)free(str1_rep);
            (void)free(str2_rep);
            return(rc);
	}

        /**********
          backwards
        **********/
	else if (sort_mod & _COLL_BACKWARD_MASK) {
	    if (sort_mod & _COLL_POSITION_MASK)
		rc = __back_pos_collate_sb(hdl, str1_ptr, str2_ptr, cur_order);
	    else
		rc = __backward_collate_sb(hdl, str1_ptr, str2_ptr, cur_order);
	}

        /**********
          or forwards (the default if sort_mod is non-zero)
        **********/
	else {
	    if (sort_mod & _COLL_POSITION_MASK)
		rc = __forw_pos_collate_sb(hdl, str1_ptr, str2_ptr, cur_order);
	    else
		rc = __forward_collate_sb(hdl, str1_ptr, str2_ptr, cur_order);
	}

        /**********
          if the strings are not equal, we can leave
          otherwise continue on to next order
        **********/
        if (rc != 0) {
            (void)free(str1_rep);
            (void)free(str2_rep);
            return(rc);
        }
    }
    /**********
      must be equal, return 0
    **********/
    (void)free(str1_rep);
    (void)free(str2_rep);
    return(0);
}
