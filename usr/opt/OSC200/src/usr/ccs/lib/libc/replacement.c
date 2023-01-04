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
static char *rcsid = "@(#)$RCSfile: replacement.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/07 22:19:10 $";
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
 *   FUNCTIONS: __do_replacement
 *
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.3  com/lib/c/str/replacement.c, libcstr, bos320, 9137320a 9/4/91 13:52:17
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <sys/localedef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <regex.h>
#include "collation.h"

/*******************************************************
  FUNCTION: __do_replacement
  PURPOSE: does replacement strings for collation
*******************************************************/
char *
__do_replacement(_LC_collate_t *hdl, const char *str, int order)
{
    const char *replacement_ptr;
    char *outbuf_ptr;
    char *outbuf[2];
    char *str_ptr;
    char *ptr;
    char temp[2];
    int i;
    int space_available;
    int space_used;
    int j;
    int subs;
    unsigned char backref;
    unsigned char buffer;
    
    int status;
    int called_regcomp = FALSE;
    regex_t cmp_reg;
    regmatch_t match_reg[10];
    short order_value;

    /**********
      set up the pointers to the original string and
      the return string
    **********/
    str_ptr = (char *)str;
    buffer = 0;
    outbuf[0] = (char *)NULL;
    outbuf[1] = (char *)NULL;

    space_available = strlen(str)*sizeof(wchar_t);

    /*********
      for each sub string, compile the pattern and try to match it in str
    **********/
    for (subs=0; subs<hdl->co_nsubs; subs++) {

	/**********
	  check if this sub string is used in this order
	**********/
	if (hdl->co_nord < _COLL_WEIGHTS_INLINE)
	    order_value = hdl->co_subs[subs].ss_act.n[order];
	else 
	    order_value = hdl->co_subs[subs].ss_act.p[order];
	
	if (!(order_value & _SUBS_ACTIVE))
	    continue;

	/**********
	  determine which buffer to put the output in
	**********/
	buffer = subs % 2;
	if (outbuf[buffer] == (char *)NULL){
	    if (((outbuf[buffer] = (char *)malloc(space_available+1))) ==
		(char *)NULL){
		perror("malloc");
		/**********
		  free the other buffer
		**********/
		(void) free(outbuf[(subs+1) % 2]);
		return(NULL);
	    }
	}
	outbuf_ptr = outbuf[buffer];


	/**********
	  if this is not a regular-expressions, just do a sub
	**********/
	if (!(order_value & _SUBS_REGEXP)) {
	    while((ptr=strstr(str_ptr,hdl->co_subs[subs].ss_src)) !=(char *)NULL){
		strncpy(outbuf_ptr, str_ptr, (ptr - str_ptr));
		outbuf_ptr += ptr-str_ptr;
		strcpy(outbuf_ptr, hdl->co_subs[subs].ss_tgt);
		outbuf_ptr += strlen(hdl->co_subs[subs].ss_tgt);
		str_ptr = ptr + strlen(hdl->co_subs[subs].ss_src);
	    }
	}
	/**********
	  could be a regular expression
	**********/
	else {
	    /**********
	      compile the pattern, if it fails, return the string
	      **********/
	    if (regcomp(&cmp_reg, hdl->co_subs[subs].ss_src, 0) != 0) {
		/**********
		  free the outbuffers
		  **********/
		(void) regfree(&cmp_reg);
		strcpy(outbuf[0], str);
		(void) free(outbuf[0]);
		return(outbuf[0]);
	    }
	    called_regcomp = TRUE;
	    
	    /**********
	      go thru the string searching for matches.
	      **********/
	    status = regexec(&cmp_reg, str_ptr, 10, &match_reg[0], 0);
	    while (status == 0) {
		/**********
		  place everything before the match in the sub string
		  *********/
		for (i=0; i<match_reg[0].rm_so; i++)
		    *outbuf_ptr++ = str_ptr[i];
		
		/*********
		  we are sitting at the part to be replaced
		  **********/
		for (replacement_ptr=hdl->co_subs[subs].ss_tgt;
		     *replacement_ptr;
		     replacement_ptr++) {
		    /**********
		      if the replacement string contains \1-\9, then
		      keep the original part of the string there
		      *********/
		    if ((*replacement_ptr == '\\') &&
			isdigit(*(replacement_ptr+1))) {
			replacement_ptr++;
			temp[0] = *replacement_ptr;
			backref = atoi(temp);
			for (j=match_reg[backref].rm_so;
			     j<match_reg[backref].rm_eo; j++)
			    *outbuf_ptr++ = str_ptr[j];
		    }
		    /**********
		      otherwise just put in the replacement string
		      **********/
		    else
			*outbuf_ptr++ = *replacement_ptr;
		}
		/**********
		  look for another match after 
		  **********/
		str_ptr = &str_ptr[match_reg[0].rm_eo];
		status = regexec(&cmp_reg, str_ptr, 10, &match_reg[0], REG_NOTBOL);
	    }
	}
	    
	/**********
	  put everything after the matches back in the string
	  *********/
	strcpy(outbuf_ptr, str_ptr);

	/**********
	  for the next time around set str_ptr equal to the last
	  out buffer
	*********/
	str_ptr = outbuf[buffer];

    }

    /**********
      if the buffer is null, then none of the replacement strings 
      were active for this order, get some space and copy the original
      string into it, and return
    **********/
    if (outbuf[buffer] == (char *)NULL){
        if (((outbuf[buffer] = (char *)malloc(space_available+1))) ==
        (char *)NULL)
	    perror("malloc");

	strcpy(outbuf[buffer], str);
    }
    /*********
      otherwise free the unneeded outbuf and reg struct
    ********/
    else {
    	if(buffer)
		(void)free(outbuf[0]);
    	else
		(void)free(outbuf[1]);
    
	if (called_regcomp)
	  (void) regfree(&cmp_reg);
    }

    /**********
      return the new string
    **********/
    return(outbuf[buffer]);
}
