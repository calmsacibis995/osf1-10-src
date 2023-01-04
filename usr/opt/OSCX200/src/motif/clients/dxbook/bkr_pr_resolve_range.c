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


/* Copyright Digital Equipment Corporation */
/* Maynard, MA, USA.  1991 */
/*======================== bkr_pr_resolve_range.c =========================
File Description: Bookreader Print: Resolve a topic or page-range.
----------------

Modules: resolve_range, condenseranges
-------

History: 25-feb-91 f.klum; created
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
#include "bkr_pr_resolve_range.h"
#include "bkr_pr_util.h"

/*----------------------- resolve_range ----------------------
Purpose: Build new range struct element and resolve values.
-------
Description: Input is start and end strings. Object is to build new
-----------  range str onto end of current range struct list and
             resolve other values (esp ordinal page and offset).
             If range can't be resolved, report error msg.
             If start/end are out of order, return SWAP.
--------------------------------------------------------------*/
extern range_t *resolve_range(cdp, first_range, first_page, first_topic,
                              start_string, end_string, type, status)
  cdp_t     *cdp;
  range_t   **first_range;
  pglist_t  *first_page;
  topic_t   *first_topic;
  char      *start_string;  /* start range string */
  char      *end_string;    /* end range string */
  int       type;           /* FOLIO, ORDINAL, or TOPIC */
  int       *status;
  {
  range_t   *range;         /* ptr to a page-range info struct in list */
  range_t   **range_ptr;    /* ptr to a page-range info struct in list */
  pglist_t  *page;          /* ptr to a page struct from list */
  pglist_t  *page_start;    /* ptr to a page struct from list */
  pglist_t  *page_end;      /* ptr to a page struct from list */
  topic_t   *topic;         /* ptr to a topic struct from list */
  char      *topic_str_start;
  char      *topic_str_end;
  int       i_start;
  int       i_end;
  char      *start;
  char      *end;
  char      *cp;
  char      *s_e_cp;
  int       i;

  *status = 0;

  if((type != ORDINAL) && (type != FOLIO) && (type != TOPIC))
    {
    /* printf("\nUnknown range type, ignored\n"); */
    *status = ERROR;
    return((range_t *)0);
    }

  /*--- skip leading whitespace ---*/
  for(start=start_string; isspace(*start); ++start);
  for(end=end_string; isspace(*end); ++end);

  /*--- nul trailing whitespace ---*/  
  for(cp=start+strlen(start)-1; isspace(*cp); *cp-- = '\0');
  for(cp=end+strlen(end)-1; isspace(*cp); *cp-- = '\0');

  /*--- check for blank strings ---*/
  if(!*start_string || !*end_string)
    {
    /*printf("\n bad range, both start and end ranges must be non-nul.\n"); */
    *status = ERROR;
    return((range_t *)0);
    }

  topic_str_start = topic_str_end = (char *)0;
  page_start = page_end = (pglist_t *)0;

  if(type != TOPIC)
    {
    if(str_equal_nocase(start_string,"FIRST"))
      page_start = first_page;
    else if(str_equal_nocase(start_string,"LAST"))
      {
      for(page=first_page; page->next; page=page->next);
      page_start = page;
      }
    if(str_equal_nocase(end_string,"FIRST"))
      page_end = first_page;
    else if(str_equal_nocase(end_string,"LAST"))
      {
      for(page=first_page; page->next; page=page->next);
      page_end = page;
      }
    }

  if(type == ORDINAL)
    {
    /* check syntax */
    for(i=2, s_e_cp = start_string, page=page_start;
          i; --i, s_e_cp = end_string, page=page_end)
      {
      if(!page)  /* don't check if already resolved from "First" or "Last" */
        {
        for(cp=s_e_cp; *cp; ++cp)
          {
          if(!isdigit(*cp))
            {
            /* printf("\n bad range, in ordinal mode must specify digits only\n"); */
            *status = ERROR;
            return((range_t *)0);
            }
          }
        }
      }

    i_start = (page_start)? page_start->ordinal: atoi(start_string);
    i_end = (page_end)? page_end->ordinal: atoi(end_string);

    if(i_start > cdp->max_ordinal)
      i_start = cdp->max_ordinal;
    if(i_start < 1)
      i_start = 1;
    if(i_end > cdp->max_ordinal)
      i_end = cdp->max_ordinal;
    if(i_end < 1)
      i_end = 1;

    for(page = first_page; page; page = page->next)
      {
      if(i_start == page->ordinal)
        page_start = page;
      if(i_end == page->ordinal)
        page_end = page;
      if(page_start && page_end)
        break;
      }
    }

  else if(type == FOLIO)
    {
    /* loop on page comments */
    for(page = first_page; page; page = page->next)
      {
      if(str_equal(start_string,page->folio)) /* matched folio_start */
        page_start = page;
      if(str_equal(end_string,page->folio)) /* matched folio_end */
        page_end = page;
      if(page_start && page_end)
        break;
      }
    }   /* end if type == FOLIO */

  else if(type == TOPIC)
    {
    for(topic = first_topic; topic; topic = topic->next)
      {   /* loop on topic comments */
      if(!topic->end && str_equal(start_string,topic->string))
        {
        page_start = topic->page;
        topic_str_start = topic->string;
        }
      if(topic->end && str_equal(end_string,topic->string))
        { /* matched topic_end */
        page_end = topic->page;
        topic_str_end = topic->string;
        }
      if(page_start && page_end)
        break;
      }   /* end loop on topic comments */
    }   /* end if type == TOPIC */

  if(!page_start || !page_end)
    {    /* bad range, complain and eliminate */
    /* printf("\n Bad Range:\n"); */
    if(!page_start)
      ; /* printf(" Could not find %s\n",start_string); */
    if(!page_end)
      ; /* printf(" Could not find %s\n",end_string); */
    *status = ERROR;
    return((range_t *)0);
    }

  /* determine ptr where this range struct will attach to list */
  if(!*first_range)
    range_ptr = first_range;
  else
    {
    for(range = *first_range; range->next; range = range->next);
    range_ptr = &(range->next);
    }

  /* get memory for new range struct */
  if(!(range = *range_ptr = (range_t *)BKR_CALLOC(1,sizeof(range_t))))
    {
    *status = ERROR;
    return((range_t *)0);
    }

  /* swap start and end if out of order */
  if(page_start->ordinal > page_end->ordinal)
    {
    page = page_start;
    page_start = page_end;
    page_end = page;
    cp = topic_str_start;
    topic_str_start = topic_str_end;
    topic_str_end = cp;
    }

  range->type = type;
  range->ord_start = page_start->ordinal;
  range->ord_end = page_end->ordinal;
  range->folio_start = page_start->folio;
  range->folio_end = page_end->folio;
  range->topic_start = topic_str_start;
  range->topic_end = topic_str_end;
  range->offset_start = page_start->offset;
  range->offset_end = (page_end->next)? page_end->next->offset:
                                        cdp->trail_offset;

  /* advance page counters */
  ++cdp->n_ranges;
  range->n_pages = range->ord_end - range->ord_start + 1;
  cdp->n_pages += range->n_pages;

  return(range);
  }


/*----------------------- condense_ranges -------------------------
Purpose: Condense contiguous ranges
-------
----------------------------------------------------------------*/
extern void condense_ranges(cdp,first_range)
  cdp_t    *cdp;
  range_t  **first_range; /* ptr to first range struct in list */
  {
  range_t  *range;
  range_t  *nextrange;
  range_t  *free_range;
  XmString xstring;

  for(range = *first_range; range; range=range->next)
    {
    for(nextrange = range->next; nextrange; )
      {
      if((range->ord_end+1) >= nextrange->ord_start)
        {
        range->ord_end = nextrange->ord_end;
        range->folio_end = nextrange->folio_end;
        range->topic_end = nextrange->topic_end;
        range->offset_end = nextrange->offset_end;
        free_range = nextrange;
        nextrange = nextrange->next;
        if(free_range == *first_range)
          *first_range = (free_range->next)? free_range->next: (range_t *)0;
        else
          range->next = (free_range->next)? free_range->next: (range_t *)0;
        if(free_range->xstring)
          XmStringFree(free_range->xstring);
        BKR_CFREE(free_range);
        }
      else
        nextrange = nextrange->next;
      }
    }

  for(range = *first_range, cdp->n_pages=0; range; range=range->next)
    {
    range->n_pages = range->ord_end - range->ord_start + 1;
    cdp->n_pages += range->n_pages;
    }
  return;
  }


/*----------------------- delete_range -------------------------
Purpose: Delete selected range from list.
-------
--------------------------------------------------------------*/
extern void delete_range(cdp,first_range,xstring)
  cdp_t     *cdp;
  range_t   **first_range; /* ptr to first range struct in list */
  XmString  xstring;       /* string identifies range struct to delete */
  {
  range_t *range;         /* ptr to a page-range info struct in list */
  range_t *prev_range;    /* ptr to a page-range info struct in list */

  for(range = *first_range; range; range = range->next)
    {
    if(XmStringCompare(range->xstring,xstring))
      {
      if(range == *first_range)
        *first_range = (range->next)? range->next: (range_t *)0;
      else
        prev_range->next = (range->next)? range->next: (range_t *)0;
      cdp->n_pages -= range->n_pages;
      --cdp->n_ranges;
      XmStringFree(range->xstring);
      BKR_CFREE(range);
      break;
      }
    prev_range = range;
    }
  return;
  }

