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
static char *rcsid = "@(#)$RCSfile: colval.c,v $ $Revision: 1.1.6.3 $ (DEC) $Date: 1993/11/23 21:34:00 $";
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
 * FUNCTIONS: _getcolval, _mbucoll, _mbce_lower
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.4  com/lib/c/str/colval.c, libcstr, bos320, 9125320 6/6/91 18:06:25
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#ifdef stat
#undef stat
#endif

#include <sys/localedef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include "collation.h"

static int FixWeight(_LC_collate_t *hdl, int cv, int wc, int realwc) {

    if (wc != realwc && cv != 0 && cv <= hdl->co_col_max) {
	/*
	 * May need to fix up the weights for this collation point.
	 * Three cases:
	 *	*colval == 0		(don't want to change)
	 *	*colval > co_col_max	(don't change)
	 *	OTHERWISE		add delta
	 */

	int delta = ((cv & 0xff)+realwc - wc)/255;

	cv += (realwc - wc) + delta;
    }

    cv = collation_to_byte_string(cv);

    return (cv);
}

/************************************************************************/
/*  _getcolval - determine nth collation weight of collating symbol	*/
/************************************************************************/

int _getcolval(_LC_collate_t *hdl, wchar_t *colval, wchar_t realwc, const char *str, int order)
{

    int i;
    int count;
    int wc;		/* In case collation table is compressed */
    wchar_t	weight;

    /**********
      check for a compressed collation table
    **********/
    if (realwc > hdl->co_hbound)
        wc = hdl->co_hbound;
    else
        wc = realwc;

    /**********
      get the collation value for the character (wc)
    **********/
    if ( !hdl->co_coltbl ) return 0;

    if (hdl->co_nord < _COLL_WEIGHTS_INLINE)
	weight = hdl->co_coltbl[wc].ct_wgt.n[order];
    else
	weight = hdl->co_coltbl[wc].ct_wgt.p[order];

    *colval = FixWeight(hdl, weight, wc, realwc);

    /**********
      check if there are any collation elements for this character
    **********/
     if (hdl->co_coltbl[wc].ct_collel != (_LC_collel_t *)NULL) {

	 i = 0;
	 while (hdl->co_coltbl[wc].ct_collel[i].ce_sym != NULL) {
	    /**********
	      get the length of the collation element that starts with
	      this character
	    **********/
	    count = strlen((char *)hdl->co_coltbl[wc].ct_collel[i].ce_sym);
	    
	    /**********
	      if there is a match, get the collation elements value and
	      return the number of characters that make up the collation
	      value
	    **********/
	    if (strncmp(str,(char *)hdl->co_coltbl[wc].ct_collel[i].ce_sym, 
			count) == 0) {
		if (hdl->co_nord < _COLL_WEIGHTS_INLINE)
		    weight = hdl->co_coltbl[wc].ct_collel[i].ce_wgt.n[order];
		else
		    weight = hdl->co_coltbl[wc].ct_collel[i].ce_wgt.p[order];

		*colval = FixWeight(hdl, weight, wc, realwc);
		return(count);
	    }
	    
	    /**********
	      This collation element did not match, go to the next
	    **********/
	    i++;
	}
    }
    /**********
      no collation elements, or none that matched, return
      0 additional characters.
    **********/
    return(0);
}

/**********
  get collation value for wchar_t
**********/
int _wc_getcolval(_LC_collate_t *hdl, wchar_t *colval, wchar_t realwc, const wchar_t *wcs, int order)
{
    
    char *str;          /* mulit-byte string for wcs */
    char *str_ptr;
    int i;
    int j;
    int rc;
    int count;
    wchar_t	wc;
    wchar_t	weight;

    /**********
      check for a compressed collation table
    **********/
    if (realwc > hdl->co_hbound)
        wc = hdl->co_hbound;
    else
        wc = realwc;

    /**********
      get the collation value for the character (wc)
    **********/
    if ( !hdl->co_coltbl ) return 0; 		/* No COLLATION entry */

    if (hdl->co_nord < _COLL_WEIGHTS_INLINE)
	weight = hdl->co_coltbl[wc].ct_wgt.n[order];
    else
	weight = hdl->co_coltbl[wc].ct_wgt.p[order];

    *colval = FixWeight(hdl, weight, wc, realwc);

    /**********
      check if there are any collation elements for this character
    **********/
     if (hdl->co_coltbl[wc].ct_collel != (_LC_collel_t *)NULL) {

	 i = 0;
	 while (hdl->co_coltbl[wc].ct_collel[i].ce_sym != NULL) {
	    /**********
	      get the length of the collation element that starts with
	      this character
	    **********/
	    count = strlen((char *)hdl->co_coltbl[wc].ct_collel[i].ce_sym);

	    /**********
	      get the space needed to convert 'count' number of characters
	      of wcs to a mb-string for the comparison
	    **********/
	    str = (char *) malloc((count * MB_CUR_MAX)  + 1);
	    if (str == NULL) {
		return(-1);
	    }
	    str_ptr = str;
	    for (j=0; j<count; j++) {
		rc = wctomb(str_ptr, *wcs);
		/***********
		  Hit a null
		**********/
		if (rc == 0)
		    continue;
		/**********
		  for an invalid character, assume it is one
		  character and continue
		**********/
		else if (rc == -1)
		    *str_ptr++ = (char)*wcs;
		else
		    str_ptr += rc;
		wcs++;
	    }
	    /**********
	      if there is a match, get the collation elements value and
	      return the number of characters that make up the collation
	      value
	    **********/
	    if (strncmp(str,(char *)hdl->co_coltbl[wc].ct_collel[i].ce_sym, 
			count) == 0) {
		if (hdl->co_nord < _COLL_WEIGHTS_INLINE)
		    weight = hdl->co_coltbl[wc].ct_collel[i].ce_wgt.n[order];
		else
		    weight = hdl->co_coltbl[wc].ct_collel[i].ce_wgt.p[order];

		*colval = FixWeight(hdl, weight, wc, realwc);

		(void)free(str);
		return(count);
	    }
	    
	    /**********
	      This collation element did not match, go to the next
	    **********/
	    (void)free(str);
	    i++;
	}
    }
    /**********
      no collation elements, or none that matched, return
      0 additional characters.
    **********/
    return(0);
}

/************************************************************************/
/*  _mbucoll - determine unique collating weight of collating symbol	*/
/************************************************************************/

wchar_t _mbucoll(_LC_collate_t *phdl, char *str, char **next_char)
{
	wchar_t	ucoll;		/* collating symbol unique weight	*/
	int	wclen;		/* # bytes in first character		*/
	wchar_t	wc;		/* first character process code		*/

	wclen = mbtowc(&wc, str, MB_CUR_MAX);
	if (wclen < 0)
		wc = (unsigned char) *str++;
	else
		str += wclen;

	*next_char = str + _getcolval(phdl, &ucoll, wc, str, (int)phdl->co_nord);
	return (ucoll);
}


/************************************************************************/
/* _mbce_lower    - convert multibyte collating element to lowercase	*/
/*                - return status indicating if old/new are different	*/
/*									*/
/*                - for each character in collating element		*/
/*                - convert from file code to proces code		*/
/*		  - convert process code to lower case			*/
/*		  - convert lower case process code back to file code	*/
/*		  - set status if upper/lower process code different	*/
/*                - add terminating NUL					*/
/************************************************************************/

int _mbce_lower(char *pstr, size_t n, char *plstr)
{
	char	*pend;		/* ptr to end of conversion string	*/
	int	stat;		/* return status			*/
	wchar_t	wc;		/* original string process code		*/
	wchar_t	wcl;		/* lowercase string process code	*/

	stat = 0;
	for (pend = pstr + n; pstr < pend;)
		{
		pstr += mbtowc(&wc, pstr, MB_CUR_MAX);
		wcl = towlower((wint_t)wc);
		plstr += wctomb(plstr, wcl);
		if (wcl != wc)
			stat++;
		}
	*plstr = '\0';
	return (stat);
}
