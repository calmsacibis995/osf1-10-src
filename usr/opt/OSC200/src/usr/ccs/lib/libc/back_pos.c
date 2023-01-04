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
static char *rcsid = "@(#)$RCSfile: back_pos.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/08 01:26:20 $";
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
 *   FUNCTIONS: __back_pos_collate_std
 *		__back_pos_collate_sb
 *
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.1  com/lib/c/str/back_pos.c, libcstr, bos320, 9140320 9/24/91 15:48:29
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

int 
__back_pos_collate_std(_LC_collate_t *hdl, char *str1, char *str2, int order)
{
    wchar_t *str1_colvals;  /* space for collation values for str1 */
    wchar_t *str2_colvals;  /* space for collation values for str1 */

    size_t	len1 = strlen(str1)*sizeof(wchar_t)*2 + 1; /* POS:COLL */
    size_t	len2 = strlen(str2)*sizeof(wchar_t)*2 + 1; /*  for each char */

    size_t	rc;
    size_t	str1_colvals_len, str2_colvals_len;
    off_t	str1_pos = 0;
    off_t	str2_pos = 0;

    wchar_t	wc;

    /**********
      get the space for the collation values.  currently there cannot
      be more collation values than bytes in the string
    *********/
    if ((str1_colvals=malloc(len1)) == NULL ||
	(str2_colvals=malloc(len2)) == NULL) {
	perror("malloc");
	exit(-1);
    }

    /**********
      put all of the colvals in str1_colvals followed by
      their position and keep count
    **********/
    str1_colvals_len = 0;
    str2_colvals_len = 0;

    while (*str1 != '\0') {
	/**********
	  get the collating value for each character.  if it is an invalid
	  character, assume 1 byte and go on
	**********/
	if ((rc = mbtowc(&wc, str1, MB_CUR_MAX)) == -1) {
	    _Seterrno(EINVAL);
	    wc = (wchar_t) *(unsigned char *)str1++;
	}
	else
	    str1 += rc;

	str1_pos++;
	str1 += _getcolval(hdl, &str1_colvals[str1_colvals_len], wc, str1, order);
	if (str1_colvals[str1_colvals_len] != 0) {
	    str1_colvals[str1_colvals_len] =
	      	byte_string_to_collation(str1_colvals[str1_colvals_len]);
	    str1_colvals_len++;
	    str1_colvals[str1_colvals_len++] = str1_pos;
	}
    }

    str1_colvals_len--;		/* Trim */
    
    /**********
      do the same for str2
    **********/
    while (*str2 != '\0') {
	/**********
	  get the collating value for each character.  if it is an invalid
	  character, assume 1 byte and go on
	**********/
	if ((rc = mbtowc(&wc, str2, MB_CUR_MAX)) == -1) {
	    _Seterrno(EINVAL);
	    wc = (wchar_t) *(unsigned char *)str2++;
	}
	else
	    str2 += rc;

	str2_pos++;
	str2 += _getcolval(hdl, &str2_colvals[str2_colvals_len], wc, str2, order);
	if (str2_colvals[str2_colvals_len] != 0) {
	    str2_colvals[str2_colvals_len] =
	              byte_string_to_collation(str2_colvals[str2_colvals_len]);
	    str2_colvals_len++;
	    str2_colvals[str2_colvals_len++] = str2_pos;
	}
    }
    str2_colvals_len--;

    /**********
      start at the end of both string and compare the values
    **********/
    while ((str1_colvals_len>=0) && (str2_colvals_len>=0)) {
	if (str1_colvals[str1_colvals_len] < str2_colvals[str2_colvals_len]) {
	    free(str1_colvals);
	    free(str2_colvals);
	    return(-1);
	}
	else if (str1_colvals[str1_colvals_len] > str2_colvals[str2_colvals_len]) {
	    free(str1_colvals);
	    free(str2_colvals);
	    return(1);
	}
	str1_colvals_len--;
	str2_colvals_len--;
    }
    free(str1_colvals);
    free(str2_colvals);
    /********
      if we are here, they are equal, if str1 is longer than str2, it is
      greater
    **********/
    return(str1_colvals_len - str2_colvals_len);
   
}

int 
__back_pos_collate_sb(_LC_collate_t *hdl, char *str1, char *str2, int order)
{
    wchar_t *str1_colvals;  /* space for collation values for str1 */
    wchar_t *str2_colvals;  /* space for collation values for str1 */

    int rc;
    int str1_colvals_len;
    int str2_colvals_len;
    int str1_pos = 0;
    int str2_pos = 0;
    wchar_t wc;

    /**********
      get the space for the collation values.  currently there cannot
      be more collation values than bytes in the string
    *********/
    if ((str1_colvals=malloc((strlen(str1)*2*(sizeof(wchar_t)))+1)) == NULL ||
	(str2_colvals=malloc((strlen(str2)*2*(sizeof(wchar_t)))+1)) == NULL) {
	perror("malloc");
	exit(-1);
    }

    /**********
      put all of the colvals in str1_colvals and keep count
    **********/
    str1_colvals_len = 0;
    str2_colvals_len = 0;
    while (*str1 != '\0') {
	/**********
	  get the collating value for each character.  if it is an invalid
	  character, assume 1 byte and go on
	**********/
	wc = (wchar_t) *(unsigned char *)str1++;
	str1 += _getcolval(hdl, &str1_colvals[str1_colvals_len], wc, str1, order);
	str1_pos++;
	if (str1_colvals[str1_colvals_len] != 0) {
	    str1_colvals[str1_colvals_len] =
	      	byte_string_to_collation(str1_colvals[str1_colvals_len]);
								      
	    str1_colvals_len++;
	    str1_colvals[str1_colvals_len++] = str1_pos;
	}
    }
    str1_colvals_len--;
    
    /**********
      do the same for str2
    **********/
    while (*str2 != '\0') {
	/**********
	  get the collating value for each character.  if it is an invalid
	  character, assume 1 byte and go on
	**********/
	wc = (wchar_t) *(unsigned char *)str2++;
	str2 += _getcolval(hdl, &str2_colvals[str2_colvals_len], wc, str2, order);
	str2_pos++;
	if (str2_colvals[str2_colvals_len] != 0) {
	    str2_colvals[str2_colvals_len] =
	        byte_string_to_collation(str2_colvals[str2_colvals_len]);

	    str2_colvals_len++;
	    str2_colvals[str2_colvals_len++] = str2_pos;
	}
    }
    str2_colvals_len--;

    /**********
      start at the end of both string and compare the values
    **********/
    while ((str1_colvals_len>=0) && (str2_colvals_len>=0)) {
	if (str1_colvals[str1_colvals_len] < str2_colvals[str2_colvals_len]) {
	    free(str1_colvals);
	    free(str2_colvals);
	    return(-1);
	}
	else if (str1_colvals[str1_colvals_len] > str2_colvals[str2_colvals_len]) {
	    free(str1_colvals);
	    free(str2_colvals);
	    return(1);
	}
	str1_colvals_len--;
	str2_colvals_len--;
    }
    free(str1_colvals);
    free(str2_colvals);
    /********
      if we are here, they are equal, if str1 is longer than str2, it is
      greater
    **********/
    return(str1_colvals_len - str2_colvals_len);
   
}
