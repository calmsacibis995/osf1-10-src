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
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1991  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use, 	     **
**  duplication or dissemination of the software and	     **
**  media is authorized only pursuant to a valid written     **
**  license from Digital Equipment Corporation.	    	     **
**  	    	    	    	    	    	    	     **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	     **
**  disclosure by the U.S. Government is subject to 	     **
**  restrictions as set forth in Subparagraph (c)(1)(ii)     **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as  	     **
**  applicable.	    	    	    	    	    	     **
***************************************************************
*/



/*======================== bkr_pr_readpscomments.c =========================
File Description: Bookreader Print: Reads structured PS comments.
----------------

Modules: readpscomments
-------

History: 11-apr-91 f.klum; created
-------
*/
#include <stdio.h>
#include <ctype.h>
#include <Xm/Xm.h>
#include "br_common_defs.h"
#include "br_meta_data.h"
#include "br_typedefs.h"
#include "br_printextract.h"
#include "br_malloc.h"
#include "bkr_pr_readpscomments.h"
#include "bkr_pr_util.h"

externaldef(bkrdata) unsigned long ex_offset;


/*----------------------- readpscomments ------------------------
Purpose: Read structured comments from ps file and form linked
-------  lists for page and topic structs.

Description:
-----------

--------------------------------------------------------------*/
extern int readpscomments(in_fp,cdp,first_page,first_topic)
  FILE      *in_fp;        /* ptr to assoc ps file for read */
  cdp_t     *cdp;          /* demand print file info */
  pglist_t  **first_page;  /* ptr ptr to first page struct */
  topic_t   **first_topic; /* ptr ptr to first topic struct */
  {
  pglist_t  *page;         /* ptr to a page struct */
  pglist_t  **page_ptr;    /* ptr ptr to a page struct, for link */
  topic_t   *topic;        /* ptr to a topic struct */
  topic_t   **topic_ptr;   /* ptr ptr to a topic struct, for link */
  unsigned long offset;    /* byte offset where page starts */
  char  comment[COMENTSIZ]; /* buffer to read a filtered comment into */
  char  filebytes[COMENTSIZ]; /* buffer to read straight file bytes into */
  char  foliobuf[LINESIZ]; /* buffer to hold current folio string */
  char  *folio;            /* ptr into comment string to folio string */
  int   ordinal;           /* ordinal page number value from comment */
  char  *argbuf;           /* ptr to first arg of structured comment */
  unsigned long eof_offset;
  int   coment_type;       /* code for type of comment */
  int   eof;               /* true if eof */
  int   error;             /* error return value */
  int   n_pages;           /* total number of %%Page comments found */
  int   i;
  int   nested_doc;
  FILE  *out_fp;           /* dummy file ptr */

  foliobuf[0] = '\0';
  nested_doc = 0;
  cdp->prolog_offset = 0L;
  page = (pglist_t *)0;

  /*--- loop until eof reading comments and building lists ---*/
  for(n_pages = error = 0, eof = FALSE; !eof;)
    {
    /* read until a %% comment is found */
    if(error = ioparse(in_fp, out_fp, filebytes, comment, &argbuf, FALSE))
      {
      eof_offset = ex_offset;
      if((error == EOF) && *comment)
        eof = TRUE;    /* eof on a comment line, process comment */
      else
        break;  /* eof or error encountered */
      }
    /*----------
    if(*comment == '+')
      continue; /* continuation -----*/

    if(str_equal(comment,"BeginDocument"))
      nested_doc++;
    if(str_equal(comment,"EndDocument"))
      nested_doc--;
    if(nested_doc > 0)
      continue;
    else
      nested_doc = 0;

    if(str_equal(comment,"Page"))
      coment_type = PAGE;
    else if(str_equal(comment,"EC$BKR_TOPIC"))
      coment_type = TOPIC;
    else if(str_equal(comment,"EC$BKR_ENDTOPIC"))
      coment_type = ENDTOPIC;
    else if(str_equal(comment,"Trailer"))
      cdp->trail_offset = ex_offset;
    else if((cdp->prolog_offset == 0L) && str_equal(comment,"EndSetup"))
      cdp->prolog_offset = (unsigned long)ftell(in_fp);
    else
      continue;
/*    coment_type = DONT_CARE;  */

    if(!argbuf)  /* no arg, never mind */
      continue;

    if(coment_type == PAGE)
      {
      ++n_pages;
      /* get ordinal page number and ptr to folio string */
      ordinal = 0;
      foliobuf[0] = '\0';
      getpageordfolio(argbuf,&folio,&ordinal);
      if(!ordinal)
        ordinal = n_pages;
      folio[LINESIZ-1] = '\0';
      strcpy(foliobuf,folio);
      if(!foliobuf[0])
        sprintf(foliobuf,"%d",ordinal);
        
      /* setup link */

      if(*first_page)
        page_ptr = &page->next;
      else
        page_ptr = first_page;

      /* get memory for struct and string in struct */
      if(!(page = *page_ptr = (pglist_t *)BKR_CALLOC(1,sizeof(pglist_t))))
        return(ERROR);

      if(!(page->folio = (char *)BKR_CALLOC(1,strlen(foliobuf)+1)))
        return(ERROR);

      strcpy(page->folio,foliobuf);
      page->ordinal = ordinal;
      page->offset = ex_offset;

      }   /* end if found Page comment */

    else if((coment_type == TOPIC) || (coment_type == ENDTOPIC))
      {    /*--- if comment topic, look at it ---*/

      argbuf[LINESIZ-1] = '\0';

      /* setup link */
      if(*first_topic)
        topic_ptr = &topic->next;
      else
        topic_ptr = first_topic;

      /* get memory for struct and string in struct */
      if(!(topic = *topic_ptr = (topic_t *)BKR_CALLOC(1,sizeof(topic_t))))
        return(ERROR);

      if(!(topic->string = (char *)BKR_CALLOC(1,strlen(argbuf)+1)))
        return(ERROR);

      strcpy(topic->string,argbuf);
      topic->page = page;
      topic->end = (coment_type == ENDTOPIC)? TRUE: FALSE;

      }   /* end if found topic comment */
    }   /* end loop on reading comments */        

  cdp->max_ordinal = (ordinal > n_pages)? ordinal: n_pages;

  return(error);
 }
