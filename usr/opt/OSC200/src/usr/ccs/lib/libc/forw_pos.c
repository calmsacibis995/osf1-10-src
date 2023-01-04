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
static char *rcsid = "@(#)$RCSfile: forw_pos.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/07 22:53:13 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 *   COMPONENT_NAME: LIBCSTR
 *
 *   FUNCTIONS: __forw_pos_collate_std, __forw_pos_collate_sb
 *
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.1  com/lib/c/str/forw_pos.c, libcstr, bos320, 9140320 9/24/91 15:48:32
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

/**********************************************************
  FUNCTION: forw_pos collate

  PURPOSE: collates in string order in the forw_pos direction
**********************************************************/

int 
__forw_pos_collate_std(_LC_collate_t *hdl, char *str1, char *str2, int order)
{
    wchar_t wc;
    int     rc;
    wchar_t str1_colval;
    wchar_t str2_colval;
    int str1_pos = 0;
    int str2_pos = 0;
    int mbc = MB_CUR_MAX;
    int dir;

    /**********
      go thru all of the characters until a null is hit
    **********/
    while ((*str1 != '\0') && (*str2 != '\0')) {
	/**********
	  convert to a wc, if it is an invalid wc, assume a length of 1
	  and continue. If the collating value is 0 (non-collating) get the
	  collating value of the next character
	***********/
	do {
	    if ((rc = (mbtowc(&wc, str1, mbc))) == -1) {
		_Seterrno(EINVAL);
		wc = (wchar_t) *(unsigned char *)str1++;
	    }
	    else
		str1 += rc;
	    str1_pos++;
	    str1 += _getcolval(hdl, &str1_colval, wc, str1, order);
	} while ((str1_colval == 0) && (*str1 != '\0'));
	
	do {
	    if ((rc = (mbtowc(&wc, str2, mbc))) == -1) {
		_Seterrno(EINVAL);
		wc = (wchar_t) *(unsigned char *)str2++;
	    }
	    else
		str2 += rc;
	    str2_pos++;
	    str2 += _getcolval(hdl, &str2_colval, wc, str2, order);
	} while ((str2_colval == 0) && (*str2 != '\0'));
	
	/**********
	  If the collating values are not zero, check the position
	  in the string. 
	**********/
	if (str1_colval !=0 && str2_colval != 0) {
	    if (str1_pos != str2_pos)
		return (str1_pos - str2_pos);
	}

	/**********
	  if the collation values are not equal, then we have gone far
	  enough and may return
	**********/
	dir = memcmp(&str1_colval, &str2_colval, sizeof(wchar_t));
	if (dir)
	    return (dir);
    }
    
    /*********
      to get here, str1 and/or str2 are NULL and they have been equal so far.
      If one of them is non-null, check if the remaining characters are
      non-collating. If they are, then the strings are equal for this order.
    *********/
    /**********
      If str1 is non-null and there are collating characters left, then
      str1 is greater than str2
    **********/
    while (*str1 != '\0') {
	if ((rc = (mbtowc(&wc, str1, mbc))) == -1) {
	    _Seterrno(EINVAL);
	    wc = (wchar_t) *(unsigned char *)str1++;
	  }
	else
	  str1 += rc;
	str1 += _getcolval(hdl, &str1_colval, wc, str1, order);

	if (str1_colval)
	  return (1);
    }
    
    
    /**********
      if str2 is non-null and there are collating characters left, then
      str1 is less than str2
    **********/
    while (*str2 != '\0') {
	if ((rc = (mbtowc(&wc, str2, mbc))) == -1) {
	    _Seterrno(EINVAL);
	    wc = (wchar_t) *(unsigned char *)str2++;
	}
	else
	  str2 += rc;
	str2 += _getcolval(hdl, &str2_colval, wc, str2, order);
    
	if (str2_colval != 0)
	    return(-1);
    }

    /**********
      if we get to here, they are equal
    **********/
    return(0);
}

int 
__forw_pos_collate_sb(_LC_collate_t *hdl, char *str1, char *str2, int order)
{
    wchar_t	wc;
    wchar_t	str1_colval;
    wchar_t	str2_colval;
    int		str1_pos = 0;
    int 	str2_pos = 0;
    int 	dir;

    /**********
      go thru all of the characters until a null is hit
    **********/
    while ((*str1 != '\0') && (*str2 != '\0')) {
	/**********
	  convert to a wc, if it is an invalid wc, assume a length of 1
	  and continue. If the collating value is 0 (non-collating) get the
	  collating value of the next character
	***********/
	do {
	    wc = (wchar_t) *(unsigned char *)str1++;
	    str1 += _getcolval(hdl, &str1_colval, wc, str1, order);
	    str1_pos++;
	} while ((str1_colval == 0) && (*str1 != '\0'));
	
	do {
	    wc = (wchar_t) *(unsigned char *)str2++;
	    str2 += _getcolval(hdl, &str2_colval, wc, str2, order);
	    str2_pos++;
	} while ((str2_colval == 0) && (*str2 != '\0'));

	/**********
	  If the collating values are not zero, check the position
	  in the string. 
	**********/
	if (str1_colval !=0 && str2_colval != 0) {
	    if (str1_pos != str2_pos)
		return (str1_pos - str2_pos);
	}
	
	/**********
	  if the collation values are not equal, then we have gone far
	  enough and may return
	**********/
	dir = memcmp( &str1_colval, &str2_colval, sizeof(wchar_t));
	if (dir)
	    return (dir);
    }
    
    /*********
      to get here, str1 and/or str2 are NULL and they have been equal so far.
      If one of them is non-null, check if the remaining characters are
      non-collating. If they are, then the strings are equal for this order.
    *********/
    /**********
      If str1 is non-null and there are collating characters left, then
      str1 is greater than str2
    **********/
    while (*str1 != '\0') {
	wc = (wchar_t) *(unsigned char *)str1++;
	str1 += _getcolval(hdl, &str1_colval, wc, str1, order);

	if (str1_colval != 0)
	    return(1);
    }
    
    
    /**********
      if str2 is non-null and there are collating characters left, then
      str1 is less than str2
    **********/
    while (*str2 != '\0') {
	wc = (wchar_t) *(unsigned char *)str2++;
	str2 += _getcolval(hdl, &str2_colval, wc, str2, order);

	if (str2_colval != 0)
	    return(-1);
    }

    /**********
      if we get to here, they are equal
    **********/
    return(0);
}
