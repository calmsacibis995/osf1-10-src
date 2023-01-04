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


/*===================== bkr_pr_createpsfile.c ===========================
File Description: Customer Demand Print: non-edm ps file processing
----------------

Modules: createpsfile
-------

History: 25-nov-91 f.klum; created
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
#include "bkr_pr_createpsfile.h"

/*----------------------- createpsfile -------------------------
Purpose: Create the page-range output ps file.
-------
--------------------------------------------------------------*/

#define OPEN_IN  \
        if(!(in_fp = fopen(cdp->ps_fname,"r")))  \
          {  \
	  printf("\nCouldn't open %s for read\n", cdp->ps_fname); \
          perror("   Open");  \
          return(ERROR);  \
          }

#define OPEN_OUT  \
        if(!(out_fp = fopen(cdp->cdp_fname,"w")))  \
          {  \
	  printf("\nCouldn't open %s for write\n", cdp->cdp_fname); \
          perror("   Open");  \
          return(ERROR);  \
          }

extern int createpsfile(cdp,first_range)
  cdp_t     *cdp;         /* demand print file info */
  range_t   *first_range; /* linked list of ranges */
  {
  FILE      *in_fp;       /* ptr to assoc ps file for read */
  FILE      *out_fp;      /* ptr to cdp ps file for write */
  range_t   *range;       /* ptr to a page-range info struct in list */
  unsigned long il;
  int       eof;
  int       i_ch;
  char      ch;
  int       output;
  int       i;

  output = TRUE;

  /* Open the input ps file for read */
  OPEN_IN
  
  /* Open the output ps file for write */
  OPEN_OUT

  /* printf("\n prolog_offset = %ld\n",cdp->prolog_offset); */

  /* Copy the ps prolog */
  for(eof = FALSE; !eof; )
    {
    READ
    if(eof)
      break;
    WRITE
    if(ch == '\n')
      {
      READ
      if(eof)
        break;
      if(ch == '%')
        {
        if((unsigned long)ftell(in_fp) == cdp->prolog_offset)
          break;
        /*************
        il = (unsigned long)ftell(in_fp);
        i = (int)il - (int)cdp->prolog_offset;
        if((i < 20) && (i > -20))
          printf("\n ftell = %ld\n",il);
        ****************/
        }
      WRITE
      }
    }

  /*--- loop through ranges positioning and copying ---*/
  for(range = first_range; range; range = range->next)
    {
    /* print message ----------
    if(range->type == TOPIC)
      {
      printf("\nExtracting topics %s through %s (%d - %d)\n", 
      range->topic_start,range->topic_end,range->ord_start,range->ord_end);
      }
    else if(range->type == FOLIO)
      {
      printf("\nExtracting pages %s through %s (%d - %d)\n", 
      range->folio_start,range->folio_end,range->ord_start,range->ord_end);
      }
    else
      {
      printf("\nExtracting ordinal pages %d through %d\n", 
      range->ord_start,range->ord_end);
      }
    ---------*/
/*printf("offset_start = %ld, end = %ld\n",range->offset_start,
range->offset_end);*/

    /*-- position and copy bytes for this page-range --*/
    if(fseek(in_fp,range->offset_start,FROM_BOF))
      {
      printf("\n fseek error, start = %ld end = %ld\n",range->offset_start,
             range->offset_end);
      return(ERROR);
      }
    for(eof = FALSE; ; )
      {
      READ
      if(eof)
        break;
      WRITE
      if(ch == '\n')
        {
        READ
        if(eof)
          break;
        if(ch == '%')
          {
          if((unsigned long)ftell(in_fp) == range->offset_end)
            break;
          }
        WRITE
        }
      }
    }

  /*-- position and copy Trailer bytes --*/
  if(fseek(in_fp,cdp->trail_offset,FROM_BOF))
    {
    printf("\n fseek error, trailer = %ld\n",cdp->trail_offset);
    return(ERROR);
    }
  for(eof=FALSE; ; )
    {
    READ
    if(eof)
      break;
    WRITE
    }

  fclose(in_fp);
  fclose(out_fp);

  /* printf("\n%d page(s) extracted.\n",cdp->n_pages); */

  return(0);
  }



/*----------------------- free_range_mem -------------------------
Purpose: Free mem for structs from creating linked PS file.
-------
--------------------------------------------------------------*/
extern int free_range_mem(first_range,first_page,first_topic,list_w)
  range_t   **first_range; /* linked list of ranges */
  pglist_t  **first_page;  /* just to free */
  topic_t   **first_topic; /* just to free */
  Widget    list_w;        /* list widget */
  {
  range_t   *range;       /* ptr to a page-range info struct in list */
  range_t   *free_range;  /* ptr to a page-range info struct to free */
  pglist_t  *page;        /* just to free */
  pglist_t  *free_page;   /* just to free */
  topic_t   *topic;       /* just to free */
  topic_t   *free_topic;  /* just to free */

  /*--- free up the memory for all this stuff ---*/
  for(range = *first_range; range; )
    {
    if(list_w)  /* long_form */
      XmListDeleteItem(list_w,range->xstring);
    free_range = range;
    range = range->next;
    if(list_w)  /* long_form */
      XmStringFree(free_range->xstring);
    BKR_CFREE(free_range);
    }
  *first_range = (range_t *)0;

  for(page = *first_page; page; )
    {
    if(page->folio)
      {
      BKR_CFREE(page->folio);
      }
    free_page = page;
    page = page->next;
    BKR_CFREE(free_page);
    }
  *first_page = (pglist_t *)0;

  for(topic = *first_topic; topic; )
    {
    if(topic->string)
      {
      BKR_CFREE(topic->string);
      }
    free_topic = topic;
    topic = topic->next;
    BKR_CFREE(free_topic);
    }
  *first_topic = (topic_t *)0;

  return(0);
  }


