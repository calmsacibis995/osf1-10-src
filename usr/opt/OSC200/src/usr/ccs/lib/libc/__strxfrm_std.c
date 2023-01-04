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
static char *rcsid = "@(#)$RCSfile: __strxfrm_std.c,v $ $Revision: 1.1.5.5 $ (DEC) $Date: 1993/11/23 21:33:42 $";
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
 * FUNCTIONS: __strxfrm_std
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.7  com/lib/c/str/__strxfrm_std.c, libcstr, bos320, 9140320 9/24/91 15:46:09
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

static size_t forward_xfrm_std(_LC_collate_t *, const char *, 
			char *, int , int , int );

static size_t forw_pos_xfrm_std(_LC_collate_t *, const char *, 
			char *, int , int , int );


/*
 * FUNCTION: __strxfrm_std
 *
 * PARAMETERS:
 *           _LC_collate_t *coll - unused
 *
 *	     char *str_out - output string of collation weights.
 *	     char *str_int - input string of characters.
 *           size_t n     - max weights to output to s1.
 *
 * RETURN VALUE DESCRIPTIONS: Returns the number of collation weights
 *                            output to str_out.
 */

size_t
__strxfrm_std(char *str_out, const char *str_in, size_t n, _LC_collate_t *hdl)
{
    
    char *str_in_ptr;
    
    int		cur_order;
    char	*str_in_rep=NULL; 	/* Non-NULL when string is malloc-ed */
    int		rev_start;
    int		i;
    char 	save[2*sizeof(wchar_t)]; /* Holds 1 collation-position pair */
    int 	rc;
    size_t	count = 0;
    int 	sort_mod;
    int  	copy_start;
    int  	xfrm_byte_count;
    int  	limit;

    /**********
      loop thru the orders
    **********/
    for (cur_order=0; (cur_order<= hdl->co_nord); cur_order++) {
	
	/**********
	  get the sort modifier for this order
	**********/
	if (hdl->co_nord < _COLL_WEIGHTS_INLINE)
	    sort_mod = hdl->co_sort.n[cur_order];
	else
	    sort_mod = hdl->co_sort.p[cur_order];
	
	/**********
	  the character order is only for use by regcomp() in
	  handling bracket expressions
	**********/
	if (sort_mod & _COLL_CHAR_ORDER_MASK)
	    continue;
	
	/**********
	  if this order uses replacement strings, set them up
	**********/
	if (hdl->co_nsubs && !(sort_mod & _COLL_NOSUBS_MASK)) {
	    (void)free(str_in_rep);
	    str_in_rep = str_in_ptr = __do_replacement(hdl, str_in, cur_order);
	}
	else 
	    str_in_ptr = (char *)str_in;

	/**********
	  check for direction of the collation for this order
	**********/
	/**********
	  CHARACTER
	**********/
	if (sort_mod == 0) {
	    /**********
	      until char collation is defined
	    ***********/
	    count = forward_xfrm_std(hdl, str_in_ptr, str_out, count,
				     n, cur_order);
	    if (count == (size_t)-1)
		return(count);
	}

	/****************
	    backwards COLLATION
	****************/
	else if (sort_mod & _COLL_BACKWARD_MASK) {
	    rev_start = count;
	    if (sort_mod & _COLL_POSITION_MASK) {
	    	count = forw_pos_xfrm_std(hdl, str_in_ptr, str_out, count,
				     n, cur_order);
		xfrm_byte_count = 2*sizeof(wchar_t);
	    }
	    else {
	    	count = forward_xfrm_std(hdl, str_in_ptr, str_out, count,
				     n, cur_order);
		xfrm_byte_count = sizeof(wchar_t);
	    }

	    if (count == (size_t)-1)
		return(count);

	    /**********
	      reverse the collation orders
	    Change:
	      +-------+-------+-------+-------+-------+-------+-------+-------+
	      | a1|a2 | b1|b2 | c1|c2 | d1|d2 | e1|e2 | f1|f2 | g1|g2 |low|low|
	      +-------+-------+-------+-------+-------+-------+-------+-------+
	    To:   
	      +-------+-------+-------+-------+-------+-------+-------+-------+
	      | g1|g2 | f1|f2 | e1|e2 | d1|d2 | c1|c2 | b1|b2 | a1|a2 |low|low|
	      +-------+-------+-------+-------+-------+-------+-------+-------+
	    **********/
	    if (count <= n) {
		limit = (count-rev_start-xfrm_byte_count)/(2*xfrm_byte_count);
		copy_start = count-xfrm_byte_count;
		for (i=0; i<limit && str_out; i++) {
		    copy_start -=  xfrm_byte_count;
		    bcopy(&str_out[rev_start], save, xfrm_byte_count);
		    bcopy(&str_out[copy_start], &str_out[rev_start],
						xfrm_byte_count);
		    bcopy(save, &str_out[copy_start], xfrm_byte_count);
		    rev_start += xfrm_byte_count;
		}
	    }
	    
	}

	/**********
	  forward is the default
	**********/
	else {
	    if (sort_mod & _COLL_POSITION_MASK) 
		count = forw_pos_xfrm_std(hdl, str_in_ptr, str_out, count,
				    n, cur_order);
	    else
		count = forward_xfrm_std(hdl, str_in_ptr, str_out, count,
				    n, cur_order);

	    if (count == (size_t)-1)
		return(count);
	}
    }

    /**********
      special case, forward_xfrm_std does not handle n == 1.
    **********/
    if (n == 1 && str_out)
	*str_out = '\0';
    
    return(count);
}


static size_t
forward_xfrm_std(_LC_collate_t *hdl, const char *str_in, char *str_out,
		    int count, int n, int order)
{
    wchar_t 	wc;
    size_t 	rc;
    size_t	mbc = MB_CUR_MAX;
    int 	nn = n-1;
    wchar_t 	colval;


    if ( str_out && n &&		/* There is an output buffer */
	(count < nn))			/*  and it still has room */
      str_out = &str_out[count]; 	/* Advance to fill point */
    else
      str_out = NULL;			/* Not filling */

    
    /**********
      go thru all of the characters until a null is hit
    **********/
    while (*str_in != '\0') {
	/**********
	  get the collating value for each character
	  if mbtowc fails, set errno and return.
	**********/
	if ((rc = mbtowc(&wc, str_in, mbc)) == -1) {
	    _Seterrno(EINVAL);
	    return(rc);
	}
	str_in += rc;

	str_in += _getcolval(hdl, &colval, wc, str_in, order);
	
	/**********
	  if this character has collation, put it in the output string
	**********/
	if (colval != 0) {

	    count += sizeof(wchar_t);

	    if ( (count < nn) && str_out) {
		bcopy((char *)&colval, str_out, sizeof(wchar_t));
		str_out += sizeof(wchar_t);
	    } else {
	        str_out = NULL;	/* No more space */
	    }
	}

    }

    /**********
	add the low weight
    **********/

    count += sizeof(wchar_t);

    /*
     * since this is the last, we need to check count against
     * n and not nn.
     */
    if ((count < n) && str_out) {
	/*
	 * Weights from collation table must be converted to network byte
	 * order for inclusion into string comparisons.
	 */
	colval = collation_to_byte_string(hdl->co_col_min);
	bcopy((char *)&colval, str_out, sizeof(wchar_t));
	str_out += sizeof(wchar_t);

	/*********
	  always add a null to the end.  If this was not the last order, it will
	  be overwritten on the next pass
	**********/
	*str_out = '\0';
    }
    
    return(count);
}

static size_t
forw_pos_xfrm_std(_LC_collate_t *hdl, const char *str_in, char *str_out,
		    int count, int n, int order)
{
    wchar_t 	wc;
    size_t 	rc;
    size_t	mbc = MB_CUR_MAX;
    int 	nn = n-1;
    off_t	str_pos;
    wchar_t 	colval;

    if ( str_out && n &&		/* There is an output buffer */
	(count < nn))			/*  and it still has room */
      str_out = &str_out[count]; 	/* Advance to fill point */
    else
      str_out = NULL;			/* Not filling */


    str_pos = hdl->co_col_min;		/* Min Weight is free of zero bytes */

    /**********
      go thru all of the characters until a null is hit
    **********/
    while (*str_in != '\0') {
	/**********
	  get the collating value for each character
	  if mbtowc fails, set errno and return.
	**********/
	if ((rc = mbtowc(&wc, str_in, mbc)) == -1) {
	    _Seterrno(EINVAL);
	    return(rc);
	}
	str_in += rc;
	str_in += _getcolval(hdl, &colval, wc, str_in, order);

	str_pos++;		/* Count character position in string */
	
	/**********
	  if this character has collation, put its position and its weight in 
	  the output string
	**********/
	if (colval != 0) {

	    count += 2*sizeof(wchar_t);		/* POSITION + WEIGHT */

	    if (count >= nn)
	        str_out = NULL; /* Turn off filling */

	    if ( str_out) {
		wchar_t pos = collation_to_byte_string(str_pos);
		char *p = (char *)&pos;

		/*
		 * Look for NUL byte in 'pos' string, and replace with 0x01
		 * to keep compares of string from getting confused.
		 */
		while( p<(char *)(&pos+1)) {
		    if (*p++ == 0) p[-1] = 1;
		}

		bcopy((char *)&pos, str_out, sizeof(wchar_t)); 	/* POSITION */
		str_out += sizeof(wchar_t);
		bcopy((char *)&colval, str_out, sizeof(wchar_t)); /* WEIGHT */
		str_out += sizeof(wchar_t);
	    }
	}
    }

    /**********
	add the low weight (as POSITION and COLLATION)
    **********/

    count += 2*sizeof(wchar_t);	
    if (count >= nn)
        str_out = NULL;

    if ( str_out ) {
	colval = collation_to_byte_string(hdl->co_col_min);

	bcopy((char *)&colval, str_out, sizeof(wchar_t));
	str_out += sizeof(wchar_t);
	bcopy((char *)&colval, str_out, sizeof(wchar_t));
	str_out += sizeof(wchar_t);

	/*********
	  always add a null to the end.  If this was not the last order, it will
	  be overwritten on the next pass
	**********/
	*str_out = '\0';
    }

    return(count);
}
