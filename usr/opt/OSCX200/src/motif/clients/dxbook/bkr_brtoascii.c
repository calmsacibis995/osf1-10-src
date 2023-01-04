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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_BRTOASCII.C*/
/* *16   26-MAR-1993 15:36:30 BALLENGER "Fix compilation problems for VAX ULTRIX."*/
/* *15   14-OCT-1992 13:51:23 KLUM "fixes to BKR_ERROR_DIALOG"*/
/* *14   20-SEP-1992 17:59:44 BALLENGER "Clean up parent widget references."*/
/* *13   25-AUG-1992 16:21:12 KLUM "fixup freeing"*/
/* *12   18-JUN-1992 16:06:30 KLUM "bubble up print format"*/
/* *11    7-MAY-1992 14:23:02 KLUM "better error handling"*/
/* *10   24-MAR-1992 10:23:54 KLUM "clean-up"*/
/* *9    24-MAR-1992 09:53:06 KLUM "add include for bkr_topic_data.h"*/
/* *8    23-MAR-1992 19:45:42 BALLENGER "run ucx$convert"*/
/* *7    19-MAR-1992 15:02:34 KLUM "uses new topic data structures"*/
/* *6     3-MAR-1992 16:56:23 KARDON "UCXed"*/
/* *5     7-FEB-1992 16:26:55 KLUM "change names to bkr_book_ascii, bkr_topic_ascii"*/
/* *4     2-JAN-1992 13:34:43 KLUM "missed this one on integration"*/
/* *3    13-NOV-1991 13:41:14 KLUM "Green development work"*/
/* *2    18-SEP-1991 20:03:23 BALLENGER "add cast for mallocs and callocs"*/
/* *1    16-SEP-1991 12:38:44 PARMENTER "Reads decw$book file and produces ascii output"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_BRTOASCII.C*/
#ifndef VMS
 /*
#else
#module BKR_BRTOASCII "V03-0000"
#endif
#ifndef VMS
  */
#endif

#ifdef PRINT
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


/*
**++
**  FACILITY:
**
**      Bookreader User Interface ( bkr )
**
**  ABSTRACT:
**
**	bkr_brtoascii - reads a decw$book file and produces ascii text file
**
**
**  AUTHORS:
**
**      F. Klum
**
**  CREATION DATE:     27-Feb-1992
**
**--
**/

#include <stdio.h>
#include "br_meta_data.h"
#include "br_typedefs.h"
#include "br_common_defs.h"
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "br_globals.h"
#include "br_printextract.h"
#include "bkr_brtoascii.h"
#include "bkr_topic_data.h"

/*------------------------------------------------------------
| Extracts entire book for ascii print and/or save to file.
+------------------------------------------------------------*/
extern int 
bkr_book_ascii PARAM_NAMES((book,out_fname,fp,parent_w))
  BKR_BOOK_CTX        *book PARAM_SEP                  /* book context */
  char                *out_fname PARAM_SEP
  FILE                *fp PARAM_SEP                    /* ascii output file ptr */
  Widget              parent_w PARAM_END
  {
  BMD_OBJECT_ID	      page_id;                /* current topic */
  BKR_TOPIC_DATA      *topic;
  BKR_TEXT_LINE       *line;
  BMD_OBJECT_ID       chunk_id;
  char                *error_string;

  if(!(page_id = book->first_page))
    {
    /* printf("\n no book->first_page\n"); */
    if(!(chunk_id = bri_book_copyright_chunk(book->book_id)))
      chunk_id = 1;
    page_id = bri_page_chunk_page_id(book->book_id,chunk_id);
    }

  /* display each page until there are is no next page */
  for( ; page_id; page_id = bri_page_next(book->book_id,page_id))
    {
    if(topic = bkr_topic_data_get(book, page_id))
      {
      for(line=topic->text_lines; line; line=line->next)
        {
        F_PUTS(line->chars);
        }
      }
    bkr_topic_data_free(book, page_id);
    }

  return(0);
  }

/*------------------------------------------------------------
| Extracts a topic for ascii print and/or save to file.
+------------------------------------------------------------*/
extern int 
bkr_topic_ascii PARAM_NAMES((book,page_id,out_fname,fp,parent_w))
  BKR_BOOK_CTX       *book PARAM_SEP         /* book to extract toic from */
  BMD_OBJECT_ID	     page_id PARAM_SEP       /* page id for this topic */
  char               *out_fname PARAM_SEP
  FILE               *fp PARAM_SEP           /* ascii output file ptr */
  Widget             parent_w PARAM_END
  {
  BKR_TOPIC_DATA      *topic;
  BKR_TEXT_LINE       *line;
  char                *error_string;

  if(topic = bkr_topic_data_get(book, page_id))
    {
    for(line=topic->text_lines; line; line=line->next)
      {
      F_PUTS(line->chars);
      }
    bkr_topic_data_free(book, page_id);
    }
  return(0);
  }

#endif /* PRINT */

