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
#ifndef VMS
 /*
#else
#module BKR_BRTOCONVERTPS "V03-0000"
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
**	bkr_brtoconvertps - reads decw$book file and produces converted PS file
**
**
**  AUTHORS:
**
**      F. Klum
**
**  CREATION DATE:     30-Apr-1992
**
**  NOTE: Authoring tool issue:
**        Positioning of bitmap graphics next to text works better if
**        text lines around graphic are in chunk list before
**        graphics, on a topic basis. BR coords are assumed to be 75 dpi-
**        PS coords area at 3600 dpi.
**
**--
**/
#define TRULE 1		/* if true, draw ftext rules */
#define BITMAP_IMAGES 1 /* if true, draw bitmap images */

#include <stdio.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "br_meta_data.h"
#include "br_typedefs.h"
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "br_globals.h"
#include "br_common_defs.h"
#include "br_printextract.h"
#include "bkr_brtoconvertps.h"
#include "bkr_font.h"
#include "bkr_fetch.h"
#include "bkr_topic_data.h"
#include "bkr_image.h"
#include "bkr_imagetops.h"


typedef struct trule_s   /* for positioning ftext rules */
  {
  BMD_CHUNK      *chunk;
  int		 x;
  int		 y;
  int		 w;
  int		 h;
  int            drawn;
  } trule_t;


typedef struct graphic_s   /* for positioning graphics */
  {
  BMD_CHUNK      *chunk;
  int            pix_width;
  int            pix_height;
  int		 top_y;
  int            drawn;
  } graphic_t;


/* to see if a graphic y extent is within text positioned on page */
typedef struct txtline_s
  {
  int            y;
  int            ps_y;
  int            height;
  int            drawn;
  } txtline_t;

/*
** Function prototypes for local routines
*/
static void
copyto7bit PROTOTYPE((
  char     *tp,
  char     *sp));

static void
ps_octal PROTOTYPE((
  int      ch,
  char     *buf));

static int
textline_to_ps PROTOTYPE((
  FILE               *fp,
  BKR_BOOK_CTX       *book,
  BKR_TOPIC_DATA     *topic,
  txtline_t          **textlines,
  BMD_CHUNK          *chunk,
  graphic_t          **b_graph,
  int                n_graphics,
  trule_t	     **b_trule,
  int                n_trules,
  txtline_t          ***txt,
  char               *out_fname));

static int
gen_ps_rule PROTOTYPE((
  FILE               *fp,
  BKR_BOOK_CTX       *book,
  BKR_TOPIC_DATA     *topic,
  txtline_t          **textlines,
  trule_t	     **b_trule,
  int		     n_trules,
  char               *out_fname));

static int
gen_ps_image PROTOTYPE((
  FILE                  *fp,
  BKR_BOOK_CTX          *book,
  BKR_TOPIC_DATA        *topic,
  txtline_t             **textlines,
  BMD_CHUNK             *chunk,
  char                  *out_fname));

static int
begin_page PROTOTYPE((
  FILE                  *fp,
  BKR_BOOK_CTX          *book,
  char                  *out_fname));

static int
end_page PROTOTYPE((
  FILE                  *fp,
  BKR_BOOK_CTX          *book,
  BKR_TOPIC_DATA        *topic,
  txtline_t             **textlines,
  graphic_t             **b_graph,
  int                   n_graphics,
  trule_t               **b_trule,
  int                   n_trules,
  char                  *out_fname));

static void
setup_topic_scaling PROTOTYPE((
  int                   br_width));

#define WRKBUFSIZ 256
#define PSLINE_LENGTH 78  /* ouput ps file line length */
#define MAX_ICON_SIZE 225 /* 3" at 75 dpi - most icon graphics are
                             smaller width than this - used in guessing
                             at page-breaks/graphic positioning */
static int    psbuflen;
static int    pagenum;
static int    active_font = -1;
static char   workbuf[WRKBUFSIZ];
static char   psbuf[WRKBUFSIZ];
static int    in_page = FALSE;
static int    start_page = FALSE;
static int    x,y;
static int    len;
static int    n_octals = 0;
static int    leading = 0;
static int    text_max_y;
static int    text_top_y;
static int    buf_exceeded = FALSE;
static int    char2byte = FALSE;
static Widget parent_w;  /* for error handling */
static char   *prolog_fname;

static int    paperwidth;
static int    paperheight;
static int    paper_x_extent;
static int    left_margin;
static int    right_margin;
static int    top_margin;
static int    bottom_margin;
static int    dflt_delta_y;
static int    img_gutter;
static int    left_gutter;
static int    right_gutter;
static int    img_max_width;
static int    end_page_done = FALSE;
static double reduction;
static double scale;    /* BR(75) to PS(3600) conversion (48 or less, depends
                           on papersize */
static double t_scale;  /* as above, adjusted for topic width */
static double scale_wordspace;  /* scale stretched for wordspace */
static double t_scale_wordspace; /* as above, adjusted for topic width */
static double scale_leading; /* scale compressed for leading */
static double t_scale_leading; /* as above, adjusted for topic width */
static double scale_font_dec; /* medium and small fonts are scaled down */
static double scale_font_inc; /* large fonts are scaled up */
static double scale_down;     /* amount a graphic is scaled down below
                                 papersize scaling */

short int pixel_width, pixel_height; /* for version 1.0 books, 75 dpi
*/

/*--- minimum gutters defined at 75 dpi ---*/
#define TOP_GUTTER 75   /* leave an inch at the top */
#define BOTTOM_GUTTER 50 /* 2/3 inch at the bottom */
#define IMG_GUTTER 30   /* advance .4 inch after graphic */

#define BKR_KF  48.0  /* converts BR to PS coord system, default papersize */
#define BKR_K   48    /* converts BR to PS coord system, default papersize */
#define Y_ADJ   0.83  /* squeeze leading for appearance */
#define W_ADJ   1.10  /* stretch word-space for appearance */
#define FONT_INC 1.05 /* make large fonts a little bigger */
#define FONT_DEC 0.87 /* make most fonts smaller */
#define IMG_ADJ  4
#define IMG_FIT_K 40  /* (44)magic constant used in scaling down images
                         which exceed width */
#define DFLT_DELTA_Y 10  /* leading value used between topics */
#define ADJUST_Y_IMAGE 1 /* if true, try to correct image next to text
                            placement */
/*#define PS_DIAG 1        * if defined, put diagnostic comments into PS */

/*------------------------------------------------------------
| Sets up scaling related to papersize
+------------------------------------------------------------*/
extern void setup_paper_scaling(width,height)
  int          width;
  int          height;
  {
  FILE   *fp;
  int    min_height;

  paperwidth = width;
  paperheight = height;

  /*--- compute gutters/margins and take care of ridiculous paper sizes.
        Also set up scale, which at default papersize, is 48. This converts
        the 75 dpi bookreader coords to 3600 dpi PS coords. scale is
        less than 48 for smaller papersizes, but never greater.
        This is because it is better to partially populate large
        paper widths with reasonable font/leading sizes than to have
        very large fonts.
  ---*/

  if(paperwidth <= 570)
    {
    left_gutter = 55;
    right_gutter = 15;
    if(paperwidth < 150)
      paperwidth = 150;
    reduction = (double)paperwidth/570.0;
    scale = reduction * BKR_KF;
    }
  else
    {
    left_gutter = 55 + (paperwidth - 570)/4;
    right_gutter = 15;
    reduction = 1.0;
    scale = BKR_KF;
    }
  min_height = TOP_GUTTER + BOTTOM_GUTTER + 75;
  if(paperheight < min_height)
    paperheight = min_height;

  top_margin = TOP_GUTTER * BKR_K;
  bottom_margin = (paperheight - BOTTOM_GUTTER) * BKR_K;
  left_margin = left_gutter * BKR_K;
  right_margin = BKR_K * paperwidth - left_margin;
  paper_x_extent = paperwidth - (left_gutter + right_gutter);
  img_max_width = (int)(0.9 * paper_x_extent + 0.5);
  img_gutter = (int)(scale * IMG_GUTTER + 0.5);

  /* medium and small font sizes are scaled down further than "scale" */
  scale_font_dec = scale * FONT_DEC;
  /* large fonts are scaled up slightly */
  scale_font_inc = scale * FONT_INC;
  /* wordspace is stretched slightly for appearance */
  scale_wordspace = scale * W_ADJ;
  /* leading is compressed for appearance */
  scale_leading = scale * Y_ADJ;

  /****
  printf("\n p_width = %d, p_height = %d\n\
 left_margin = %d right_margin = %d\n\
 top = %d, bottom = %d  scale = %d\n",
 paperwidth,paperheight,left_margin,right_margin,
 top_margin,bottom_margin,bkr_scale);
 ****/

  return;
  }

/*------------------------------------------------------------
| Further adjust scaling based on topic width
+------------------------------------------------------------*/
static void setup_topic_scaling(br_width)
  int       br_width;
  {

  if(br_width <= paper_x_extent)  /* topic fits, no need to scale down */
    {
    t_scale = scale;
    t_scale_wordspace = scale_wordspace;
    t_scale_leading = scale_leading;
    }
  else  /* topic is wide, scale down to fit */
    {
    scale_down = (double)paper_x_extent / (double)br_width;
    t_scale = scale * scale_down;
    t_scale_wordspace = scale_wordspace * scale_down;
    t_scale_leading = scale_leading * scale_down;
    }

  return;
  }

/*------------------------------------------------------------
| Extracts entire book for PS print and/or save to file.
+------------------------------------------------------------*/
extern int bkr_book_convertps PARAM_NAMES((book,out_fname,fp,parent_widget))
  BKR_BOOK_CTX        *book PARAM_SEP                  /* book context */
  char                *out_fname PARAM_SEP
  FILE                *fp PARAM_SEP                    /* PS output file ptr */
  Widget              parent_widget PARAM_END
  {
  BKR_TOPIC_DATA      *topic;
  BMD_OBJECT_ID	      page_id;                /* current topic */
  BMD_OBJECT_ID       chunk_id;
  char                *error_string;
  int                 err_return;
  int                 i;

  char2byte = buf_exceeded = FALSE;
  parent_w = parent_widget;

  if(!(page_id = book->first_page))
    {
    /* printf("\n no book->first_page\n"); */
    if(!(chunk_id = bri_book_copyright_chunk(book->book_id)))
      chunk_id = 1;
    page_id = bri_page_chunk_page_id(book->book_id,chunk_id);
    }

  /* loop through all the pages of the book */
  for( ; page_id; page_id = bri_page_next(book->book_id,page_id))
    {
    if(topic = bkr_topic_data_get(book, page_id))
      {
      err_return = bkr_topic_convertps(book,page_id,out_fname,
                                   fp,TRUE,parent_widget,topic);
      bkr_topic_data_free(book, page_id);
      if(err_return)
        return(err_return);
      }
    }

  if(buf_exceeded)
    {
    BKR_ERROR_DIALOG("PE_WRKBUF_EXCEEDED",parent_w);
    buf_exceeded = FALSE;
    }
  if(char2byte)
    {
    BKR_ERROR_DIALOG("PE_2BYTE_CHAR",parent_w);
    char2byte = FALSE;
    }

  return(0);
  }


/*------------------------------------------------------------
| Extracts a topic for converted PS print and/or save to file.
+------------------------------------------------------------*/
extern int 
bkr_topic_convertps PARAM_NAMES((book,page_id,out_fname,fp,entire_book,parent_widget,topic))
  BKR_BOOK_CTX       *book PARAM_SEP         /* book to extract toic from */
  BMD_OBJECT_ID	     page_id PARAM_SEP       /* page id for this topic */
  char               *out_fname PARAM_SEP
  FILE               *fp PARAM_SEP        /* PS output file ptr */
  int                entire_book PARAM_SEP   /* true if printing entire book */
  Widget             parent_widget PARAM_SEP
  BKR_TOPIC_DATA     *topic PARAM_END
  {
  BMD_OBJECT_ID	     target_page_id; /* page id for this topic */
  BMD_CHUNK 	     *chunk;         /* ptr into chunk_list */
  BMD_CHUNK 	     *chk;           /* ptr into chunk_list */
  BMD_CHUNK 	     *parent_chk;    /* ptr into chunk_list */
  int                n_chunks;       /* how many chunks in topic */
  int                i,j;
  BMD_CHUNK          *chunk_list;
  int                id;
  BMD_IMAGE_PKT	     *image;
  txtline_t          **textlines;
  graphic_t          *graphic;
  graphic_t          **b_graph;
  graphic_t          **gr;
  int                n_graphics;
  txtline_t          *txtline;
  txtline_t          **txt;
  int                pg_brk;
  int                y_after_image;
  int		     save_text_top_y = text_top_y;
  int		     save_text_max_y = text_max_y;
  int		     save_buf_exceeded = buf_exceeded;
  BKR_TOPIC_DATA     *target_topic;
  char               *error_string;
  int		    n_trules = 0;
  trule_t	    *trule;
  trule_t	    **b_trule;
  trule_t	    **tr;
  BMD_FTEXT_PKT	    *packet;
  BR_UINT_8	    *data_addr;
  BR_UINT_8	    *data_end;
  BMD_RULE_PKT	    rule;
  BMD_RULE_PKT	    *rule_ptr = &rule;

  parent_w = parent_widget;

  setup_topic_scaling(topic->width);

  /* to compensate for page break part-way through topic... */
  buf_exceeded = FALSE;
  text_top_y = 0;
  text_max_y = -10000;

#ifdef PS_DIAG
sprintf(workbuf,"%%NEWTOPIC: y = %d",y);
FLUSH_LINE;
ADD_TO_LINE(workbuf);
FLUSH_LINE;
/*active_font = -1;*/
#endif

#ifdef BITMAP_IMAGES
  /* traverse the chunk list to get number of graphics */
  for(i=n_graphics=0, chunk = topic->chunk_list;
       i < topic->num_chunks; ++i, ++chunk)
    {
    if((chunk->data_type == BMD_CHUNK_IMAGE75) ||
       (chunk->data_type == BMD_CHUNK_IMAGE))
      ++n_graphics;
#ifdef TRULE
    else if(chunk->chunk_type == BMD_DATA_CHUNK && chunk->data_addr)
      {
      packet = (BMD_FTEXT_PKT *)chunk->data_addr;
      data_addr = chunk->data_addr;
      data_end = &data_addr[chunk->data_len];

      while (packet < (BMD_FTEXT_PKT *)data_end)
	{
	if (packet->tag == BMD_FTEXT_RULE)
	  ++n_trules;

	packet = (BMD_FTEXT_PKT *) &packet->value[packet->len - 2];
	if (packet->len == 0)
	  break;
	}
      }
#endif
    }

  /*--- allocate memory for graphics and textline extent arrays ---*/
  if(n_graphics)
    {
    b_graph = (graphic_t **)BKR_CALLOC(1,n_graphics * sizeof(graphic_t *));
    }
#ifdef TRULE
  if(n_trules)
    {
    b_trule = (trule_t **)BKR_CALLOC(1,n_trules * sizeof(trule_t *));
    }
  tr=b_trule;
#endif
  if(topic->n_lines)
    {
    textlines = (txtline_t **)
                BKR_CALLOC(1,topic->n_lines * sizeof(txtline_t *));
    }
  /*--- allocate textline extents - will set values as we draw ---*/
  for(i=0, txt=textlines; i < topic->n_lines; ++i, ++txt)
    {
    *txt = (txtline_t *)BKR_CALLOC(1,sizeof(txtline_t));
    (*txt)->drawn = FALSE;
    }

  /*--- allocate graphic extents and set values ---*/
  for(i=0, gr=b_graph, chunk = topic->chunk_list;
       i < topic->num_chunks; ++i, ++chunk)
    {
    if((chunk->data_type == BMD_CHUNK_IMAGE75) ||
       (chunk->data_type == BMD_CHUNK_IMAGE))
      {
      image = (BMD_IMAGE_PKT *)chunk->data_addr;
      *gr++ = graphic = (graphic_t *)BKR_CALLOC(1,sizeof(graphic_t));
      graphic->chunk = chunk;
      graphic->pix_width = (int)image->pix_width;
      graphic->pix_height = (int)image->pix_height;
      graphic->top_y = chunk->rect.top;
      graphic->drawn = FALSE;
      }
#ifdef TRULE
    else if(chunk->chunk_type == BMD_DATA_CHUNK && chunk->data_addr)
      {
      packet = (BMD_FTEXT_PKT *)chunk->data_addr;
      data_addr = chunk->data_addr;
      data_end = &data_addr[chunk->data_len];

      while (packet < (BMD_FTEXT_PKT *)data_end)
	{
	if (packet->tag == BMD_FTEXT_RULE)
	  {
	  memcpy (&rule, &packet->value[0], sizeof(BMD_RULE_PKT));
	  *tr++ = trule = (trule_t *)BKR_CALLOC(1,sizeof(trule_t));
	  trule->chunk = chunk;
	  trule->x = rule.x;
	  trule->y = rule.y;
	  trule->w = rule.width;
	  trule->h = rule.height;
	  trule->drawn = FALSE;
	  }

	packet = (BMD_FTEXT_PKT *) &packet->value[packet->len - 2];
	if (packet->len == 0)
	  break;
	}
      }
#endif
    }
#endif

  /*--- loop on chunk list drawing text and graphics ---*/
  for(i=0, pg_brk=FALSE,gr=b_graph, chunk = topic->chunk_list, txt=textlines;
      i < topic->num_chunks; ++i, ++chunk)
    {
#ifdef PS_DIAG
FLUSH_LINE;
sprintf(workbuf,"%%newchunk %d, chunk->y = %d, y = %d",i,chunk->y,y);
ADD_TO_LINE(workbuf);
FLUSH_LINE;
#endif
       if ((chunk->data_type == BMD_CHUNK_IMAGE75) ||
             (chunk->data_type == BMD_CHUNK_IMAGE))
        {
           /*--- bitmap image data ---*/
#ifdef BITMAP_IMAGES
          /* loop on graphics for this topic */
      /*    for(j=0, pg_brk=FALSE, gr=b_graph; j < n_graphics; ++j, ++gr)
            {
      */
      if (!in_page)
         begin_page( fp, book, out_fname);
            graphic = *gr;
            if(!graphic->drawn)
              { /* check if graphic goes with text that's already drawn */
              if(((graphic->top_y + graphic->pix_height) <= text_max_y) &&
                 (graphic->pix_width < MAX_ICON_SIZE))
                {
                /* draw the graphic */
                if(gen_ps_image(fp,book,topic,textlines,
                                graphic->chunk,out_fname))
                  return(1);
                graphic->drawn = TRUE;
                }
              else
                {  /* check to see if one or more graphics break the page */
                /* compute PS y value to test pagebreak */
                y_after_image = y + (int)(graphic->pix_height * t_scale + 0.5);
                if(y_after_image >= bottom_margin)
		{
                  pg_brk = TRUE;
		  if(end_page_done == FALSE)
              	  {
		   if(end_page(fp,book,topic,textlines,b_graph,n_graphics,
				b_trule, n_trules,out_fname))
                          return(1);
                   if(begin_page(fp,book,out_fname))
                        return(1);
                  }
            	end_page_done = FALSE;
                }
		if(gen_ps_image(fp,book,topic,textlines,graphic->chunk,
                              out_fname))
                return(1);
              graphic->drawn = TRUE;
              }
            } /* if(!graphic->drawn) */

          /* all graphics for this topic below already drawn text are
             treated together - if any of them broke page, start new page */
	/*
          if(pg_brk)
            {
            if(end_page_done == FALSE)
              {

      		if(end_page(fp,book,topic,textlines,b_graph,n_graphics,b_trule,
         		n_trules,out_fname))
                return(1);
              if(begin_page(fp,book,out_fname))
		return(1);
              }
            end_page_done = FALSE;
            }
	*/

 /* draw the rest of the graphics for this topic (possibly on a new
page) */
	/*
           for(j=0, pg_brk=FALSE, gr=b_graph; j < n_graphics; ++j, ++gr)
            {
            graphic = *gr;
            if(!graphic->drawn)
              {
              if(gen_ps_image(fp,book,topic,textlines,graphic->chunk,
                              out_fname))
                return(1);
              graphic->drawn = TRUE;
              }
            }
	*/
	  
	++gr;
#endif
   } /* if chunk->data_type == BMD_CHUNK_IMAGE* */



    /*--- if this is a text chunk, process ---*/

    else if((chunk->chunk_type == BMD_DATA_CHUNK) && !chunk->parent)
      {    /*??? if(chunk->data_type == BMD_CHUNK_FTEXT) ???*/
      /*--- draw the text, checking for page-break ---*/
      if(textline_to_ps(fp,book,topic,textlines,chunk,
                     b_graph,n_graphics,b_trule,n_trules,&txt,out_fname))
        return(1);
      continue;
      }
    else if((chunk->chunk_type == BMD_REFERENCE_RECT) ||
       (chunk->chunk_type == BMD_REFERENCE_POLY))

      {  /*--- formal "popup" graphic - recursively process ---*/

      target_page_id = bri_page_chunk_page_id(book->book_id,chunk->target);
      if(!(target_topic = bkr_topic_data_get(book, target_page_id)))
        continue;
      if(target_topic->type == BKR_FORMAL_TOPIC)
        {
	  /* If whole topic won't fit on rest of page
	   * break to new page first
	   */
	  y_after_image = y + (int)(target_topic->height * t_scale + 0.5);
	  if(y_after_image >= bottom_margin)
            {
            if(end_page(fp,book,topic,textlines,b_graph,n_graphics,b_trule,n_trules,out_fname))
              return(1);
            if(begin_page(fp,book,out_fname))
              return(1);
	    end_page_done = TRUE;
            }

        if(bkr_topic_convertps(book,target_page_id,out_fname,fp,
                            TRUE,parent_widget,target_topic))
          return(1);
	end_page_done = FALSE;
        }
      bkr_topic_data_free(book, target_page_id);
      continue;
      }
    else if((chunk->chunk_type == BMD_EXTENSION_RECT) ||
       (chunk->chunk_type == BMD_EXTENSION_POLY))
      {  /*--- extension area ---*/
	/* Ignore for now - could overlay a pattern to simulate
	 * Bookreader's display of this area
	 */
      continue;
      }
    else
      {  /*--- else see if graphic or other interesting type ---*/
      switch(chunk->data_type)
        {
        case BMD_CHUNK_POSTSCRIPT:
          /* printf("\n BMD_CHUNK_POSTSCRIPT\n"); */
          break;
        case BMD_CHUNK_RAGS:
          /* printf("\n BMD_CHUNK_RAGS\n"); */
          break;
        case BMD_CHUNK_RAGS_NO_FILL:
          /* printf("\n BMD_CHUNK_RAGS_NO_FILL\n"); */
          break;
        default: 
          break;
        }
      }  /* end else other intersesting type */
    }  /* end loop on chunk list */

#ifdef TRULE
  /* draw undrawn trules for this topic */
  if (topic->height > text_max_y)
    text_max_y = topic->height;
  if(gen_ps_rule(fp,book,topic,textlines,b_trule,n_trules,out_fname))
    return(1);
#endif

#ifdef BITMAP_IMAGES
  /*--- free graphics and text extent structs ---*/
  if(n_graphics)
    {
    for(i=0, gr=b_graph; i < n_graphics; ++i,++gr)
      {
      BKR_FREE(*gr);
      }
    BKR_FREE(b_graph);
    }
#ifdef TRULE
  if(n_trules)
    {
    for(i=0, tr=b_trule; i < n_trules; ++i,++tr)
      {
      BKR_FREE(*tr);
      }
    BKR_FREE(b_trule);
    }
#endif
  if(topic->n_lines)
    {
    for(i=0, txt=textlines; i < topic->n_lines; ++i,++txt)
      {
      BKR_FREE(*txt);
      }
    BKR_FREE(textlines);
    }
#endif

  if(!entire_book)
    {
    if(buf_exceeded)
      {
      BKR_ERROR_DIALOG("PE_WRKBUF_EXCEEDED",parent_w);
      buf_exceeded = FALSE;
      }
    if(char2byte)
      {
      BKR_ERROR_DIALOG("PE_2BYTE_CHAR",parent_w);
      char2byte = FALSE;
      }
    }
  text_top_y = save_text_top_y;
  text_max_y = save_text_max_y;
  buf_exceeded = save_buf_exceeded;
  return(0);
  }


/*-----------------------------------------------------------
| draw a text line, checking for page-break
+----------------------------------------------------------*/
static int 
textline_to_ps
PARAM_NAMES((fp,book,topic,textlines,chunk,b_graph,n_graphics,
             b_trule,n_trules,txt,out_fname))
  FILE               *fp PARAM_SEP
  BKR_BOOK_CTX       *book PARAM_SEP
  BKR_TOPIC_DATA     *topic PARAM_SEP
  txtline_t          **textlines PARAM_SEP
  BMD_CHUNK          *chunk PARAM_SEP
  graphic_t          **b_graph PARAM_SEP
  int                n_graphics PARAM_SEP
  trule_t            **b_trule PARAM_SEP
  int                n_trules PARAM_SEP
  txtline_t          ***txt PARAM_SEP
  char               *out_fname PARAM_END
  {
  BKR_TEXT_LINE      *line;
  BKR_TEXT_LINE      *prev_line;
  BKR_TEXT_LINE      *next_tline;
  BKR_TEXT_ITEM_LIST *item_list;
  XTextItem          *item;
  unsigned short     *font_num_ptr;
  int                word_space;
  int                x_val,y_val;
  int                nchars;
  int                use_abs_pos;
  int                i,j,k,m;
  char               *cp,*w_ptr;
  char               *error_string;
  static int         delta_y;

  if(!chunk->first_line)
    return(0);

  /*--- loop on text lines for this chunk ---*/
  for(i = 0, line = chunk->first_line; line && (i < chunk->n_lines); ++i)
    {

    if(!in_page)
      begin_page(fp,book,out_fname);

    /*--- set y extent for textline ---*/
    (**txt)->ps_y = y;  /* topic->n_lines better be right */ 
    (**txt)->y = line->y;
    (**txt)->height = line->height;
    (**txt)->drawn = TRUE;
    if((line->y + line->height) > text_max_y) /* lines are increasing in y, but check anyway */
      text_max_y = line->y + line->height;
    ++(*txt);

    /*--- loop on item_lists in this line ---*/
    for(j = 0, item_list = line->item_lists;
        item_list && (j < line->n_item_lists);
        ++j, item_list = item_list->next)
      {
      if(item_list->is2byte) /*--- warn and skip for now ---*/
        {
        /* printf("\n bkr_brtoconvertps.c: 2 byte chars encountered\n"); */
        char2byte = TRUE;
        continue;
        }

      x = item_list->x * t_scale + left_margin;
      use_abs_pos = TRUE;

      font_num_ptr = item_list->font_nums;

      /*--- loop to copy items into workbuf ---*/
      for(k = 0, item = item_list->u.items; 
           item && (k < item_list->n_items); ++k, ++item)
        {
        /* disregard if empty line */
        if(!item->nchars)
          continue;

        /* output abs. position before first word on line */
        if(use_abs_pos)
          {
          FLUSH_LINE;
          sprintf(workbuf,"%d %d XY",x,y);
          ADD_TO_LINE(workbuf);
          use_abs_pos = FALSE;
          }
        else
          {
          /* add word space if non-zero */
          if(item->delta && k)
            {
            word_space = item->delta * t_scale_wordspace;
            sprintf(workbuf," %d x",word_space);
            ADD_TO_LINE(workbuf);
            }
          }

        /*--- set font if changed or page start ---*/
        if((*font_num_ptr != active_font) || start_page)
          /* font change at this item */
          {
          start_page = FALSE;
          ADD_TO_LINE(" ");
          sprintf(workbuf,"F%d",*font_num_ptr);
          ADD_TO_LINE(workbuf);
          active_font = *font_num_ptr;
          }
        font_num_ptr++;

        /*--- init workbuf for word string ---*/
        strcpy(workbuf," (");
        w_ptr = workbuf + strlen(" (");

        /*--- loop to copy chars for a word ---*/
        for(m=0, nchars = item->nchars, cp = item->chars;
             m < nchars; )
          {
          if(m >= (WRKBUFSIZ - strlen(" (") - 1))
            {
            /* printf("\nbkr_brtoconvertps.c: workbuf size exceeded \
, string truncated\n"); */
            buf_exceeded = TRUE;
            break;
            }
#if FALSE
          if(!isprint(*cp) && ((unsigned char)*cp != 173))
            { /* add small hyphen(173) to isprint macro */
            printf("\n non-printable char");
            n_octals++;
            ps_octal((int)*cp,w_ptr);
            w_ptr += 4; /* it takes four chars to represent an octal char */
            m += 4;
            nchars += 4;
            ++cp;
            continue;
            }
#endif
          if((*cp == '\\') || (*cp == '(') || (*cp == ')'))
            {
            *w_ptr++ = '\\';  /* precede special chars with backslash */
            ++m;
            ++nchars;
            }
          *w_ptr++ = *cp++;
          ++m;
          }

        /*--- close word string ---*/
        *w_ptr = '\0';
        strcat(workbuf,")S");

        ADD_TO_LINE(workbuf);
        }       /* end loop on items */
      }       /* end loop on item_lists */


    /*--- advance line, see how we moved ---*/
    prev_line = line;
    line = (i < (chunk->n_lines - 1))? line->next: (BKR_TEXT_LINE *)0;
    delta_y = dflt_delta_y;
    if(line)
      {
      if(line->ftext_baseline > prev_line->ftext_baseline)
        delta_y = line->ftext_baseline - prev_line->ftext_baseline;
      else
        {
        delta_y = prev_line->height + dflt_delta_y;
        }
      }
    else
      {
      delta_y = prev_line->height + dflt_delta_y;
      }

    /*--- stretch leading above 14 pts ---*/
    if(delta_y > 14)
      delta_y += delta_y - 14;

    leading = (int)(delta_y * t_scale_leading + 0.5);
    y += leading;

    if(y > bottom_margin)  /* see if time to eject this page */
      {
      end_page(fp,book,topic,textlines,b_graph,n_graphics,b_trule,n_trules,out_fname);
      begin_page(fp,book,out_fname);
      }

    }   /* end loop on lines */

  /*** if(i < n_lines)
    {
    printf("\n bkr_brtoconvertps: i < n_lines, i = %d, n_lines = %d\n",
           i,n_lines);
    } ***/

  FLUSH_LINE;

  return(0);
  }


/*----------------------------------------------------------------------
* gen_ps_rule() - generate PS for a rule
*----------------------------------------------------------------------*/
static int gen_ps_rule(fp,book,topic,textlines,b_trule,n_trules,out_fname)
  FILE		    *fp;
  BKR_BOOK_CTX	    *book;
  BKR_TOPIC_DATA    *topic;
  txtline_t	    **textlines;
  trule_t	    **b_trule;
  int		    n_trules;
  char              *out_fname;
  { 
  int		     rx,ry,psy;
  int		     rw,rh;
  int                diff,abs_diff,min_diff,r_diff;
  int                i,j;
  txtline_t          **txt;
  char               *error_string;
  trule_t	     *trule;
  trule_t	     **tr;

#ifdef TRULE
  /* loop on trules */
  for(i=0, tr=b_trule; i < n_trules; ++i, ++tr)
    {
    trule = *tr;
    if(!trule->drawn)
      {
      ry = trule->chunk->rect.top + SCALE_VALUE(trule->y,  book->version.major_num);
      if ((ry > text_top_y) && (ry <= text_max_y))
	{
	/* find the text PS y value for line that is closest to rule */
	for(j=0, txt=textlines, min_diff=10000; j < topic->n_lines; ++j, ++txt)
	  {
	  if((*txt)->drawn == FALSE) /* only compare w/ lines drawn on this page */
	    continue;
	  diff = ry - (*txt)->y;
	  abs_diff = (diff >= 0)? diff: -(diff);
	  if(abs_diff < min_diff)
	    {
	    min_diff = abs_diff;
	    r_diff = diff;
	    psy = (*txt)->ps_y -
		    (int)((*txt)->height * t_scale_leading + 0.5);
	    }
	  }

	rw = MAX(1, SCALE_VALUE(trule->w,  book->version.major_num));
	rw = (int)(rw * t_scale + 0.5);

	rh = MAX(1, SCALE_VALUE(trule->h, book->version.major_num));
	rh = (int)(rh * t_scale_leading + 0.5);

	rx = trule->chunk->rect.left + SCALE_VALUE(trule->x,  book->version.major_num);
	rx = (int)(rx * t_scale + 0.5) + left_margin;

	diff = (int)(r_diff * t_scale_leading + 0.5);
	ry = psy + diff + rh;

	FLUSH_LINE;
	sprintf(workbuf,"gsave %d setlinewidth newpath %d %d XY %d 0 rlineto stroke grestore",
		    rh, rx, ry, rw);
	ADD_TO_LINE(workbuf);
	FLUSH_LINE;
	trule->drawn = TRUE;
	}
      }
    }
#endif
  return(0);
  }


/*----------------------------------------------------------------------
* gen_ps_image() - generate PS for a bitmap image
*----------------------------------------------------------------------*/
static int gen_ps_image(fp,book,topic,textlines,chunk,out_fname)
  FILE               *fp;
  BKR_BOOK_CTX       *book;
  BKR_TOPIC_DATA     *topic;
  txtline_t          **textlines;
  BMD_CHUNK          *chunk;
  char               *out_fname;
  {
  BMD_IMAGE_PKT      *image;
  BMD_CHUNK          *chk;
  BMD_CHUNK          *parent_chk;
  int                diff,abs_diff,min_diff,g_diff;
  int                i;
  txtline_t          **txt;
  int                x_image, y_image;
  int                y_after_image;
  int                y_off;
  int                psy;	/* the PS y value for of text nearest
                                   this graphic chunk */
  int                near_text; /* true if graphic is placed near text */
  int                max_extent;
  int                image_scale;
  XColor             colortable[1];
  char               *error_string;
/* ELLEN */
  int		    n_graphics = 0;
  int		    n_trules = 0;
  graphic_t	    **b_graph = NULL;
  trule_t	    **b_trule = NULL;
  int                z;

#ifdef BITMAP_IMAGES
  /* printf("\n BMD_CHUNK_IMAGE(75)\n"); */

  if (chunk->data_type == BMD_CHUNK_IMAGE)
  {
   /* Bitmap Image -- screen resolution */
     image = (BMD_IMAGE_PKT *) chunk->data_addr;

   /* Need these so we dont have to differentiate between
	BMD_CHUNK_IMAGE75 and BMD_CHUNK_IMAGE */

   pixel_width = image->pix_width;
   pixel_height = image->pix_height;
   chunk->handle = bkr_image_init(
          image->data, chunk->data_len,
          image->pix_width, image->pix_height,
          image->res_x, image->res_y,
          chunk->data_type );
   if(!chunk->handle)
          return(0);
  }
  else if (chunk->data_type == BMD_CHUNK_IMAGE75)
  {
      /* NOTE: Version 1.0 books ONLY */
         pixel_width  = (short int) chunk->rect.width - 1;
         pixel_height = (short int) chunk->rect.height - 1;

         /*
          * Scale the width and height to 75dpi coordinates because
          * for Version 1.0 books all graphics were created at 75dpi
          * and stored in the database at 300dpi coordinates.
          * We must do this because the width and height have already
          * been scaled from 300dpi (in bkr_page.c) to the output
          * resolution which is different.
          * NOTE: we do no rounding because this causes problems!
          */

         if ( bkr_monitor_resolution != 75 )
         {

          pixel_width = SCALE_GRAPHIC_VALUE75( pixel_width );
          pixel_height = SCALE_GRAPHIC_VALUE75( pixel_height );

         }
         chunk->handle = bkr_image_init(
          chunk->data_addr, chunk->data_len,
          pixel_width, pixel_height,       /* in pixels */
          75, 75,                    /* created resolution */
          chunk->data_type );

      if ( ! chunk->handle )
     return(0);
    }

  near_text = FALSE;

  y_off = chunk->rect.top;

  /* see if bottom of graphic less than text drawn */
  if((y_off <= text_max_y) &&
     (pixel_width < MAX_ICON_SIZE))
    {
    near_text = TRUE;
    /* find the text PS y value for line that is closest to graphic */
    for(i=0, txt=textlines, min_diff=10000; i < topic->n_lines; ++i, ++txt)
      {
      if((*txt)->drawn == FALSE) /* only compare w/ lines drawn on this page */
        continue;
      diff = y_off - (*txt)->y;
      abs_diff = (diff >= 0)? diff: -(diff);
      if(abs_diff < min_diff)
        {
        min_diff = abs_diff;
        g_diff = diff;
	psy = (*txt)->ps_y -
		(int)((*txt)->height * t_scale_leading + 0.5);
        }
      }
    /*--- if graphic wasn't real near any text or graphic is fairly wide,
          position it based on offset rather than proximity to text
    ---*/
    if(min_diff > 75)
      near_text = FALSE;
    }

  if(near_text) /* set y value according to closest text-line y */
    {
    y_after_image = y_image = psy + (int)(pixel_height * t_scale + 0.5);
#ifdef ADJUST_Y_IMAGE
    y_after_image += (int)(g_diff * t_scale_leading + 0.5); 
    y_image = y_after_image;
#endif
    }
  else if(y_off <= text_max_y) /* graphic before text already drawn? */
    {
    psy = chunk->rect.top + pixel_height - text_top_y;
    y_image = (int)(psy * t_scale + 0.5);
    y_after_image = y_image + img_gutter;
    }
  else  /* graphic isn't next to drawn text */
    {   /* position based on chunk offset */
    psy = chunk->rect.top  + pixel_height ;
    y_image =  y + (int)(psy * t_scale + 0.5);
    y_after_image = y_image + img_gutter;
    }

  /* if graphic has increased our y, so be it */
  if(y_after_image > y)
    y = y_after_image;

/**** ELLEN */
  if(y_after_image >= bottom_margin)
  {
    if(end_page_done == FALSE)
    {
	if(end_page(fp,book,topic,textlines,b_graph,n_graphics,
                    b_trule, n_trules,out_fname))
            return(1);
        if(begin_page(fp,book,out_fname))
            return(1);
	psy = chunk->rect.top ;
        y_image =  (int)(psy * t_scale + 0.5);
	y_after_image =  y_image + img_gutter;
    }
    end_page_done = FALSE;
   }


#ifdef PS_DIAG
sprintf(workbuf,"%%IMAGE: chunk->y = %d, image->pix_height = %d, y_off = %d",chunk->y,image->pix_height,y_off);
FLUSH_LINE;
ADD_TO_LINE(workbuf);
FLUSH_LINE;
/*active_font = -1;*/
#endif

  max_extent = (pixel_height > pixel_width)?
                pixel_height: pixel_width;

  image_scale = (max_extent > img_max_width)?
                (IMG_FIT_K * paper_x_extent)/max_extent: 50;

  if (near_text)
  {
     image_scale = (int)((image_scale * reduction)/2 );
  }
  else
  {
     image_scale = (int)(image_scale * reduction + 0.5);
  }

  if(near_text)
    x_image = (int) ((chunk->rect.left - 15) * t_scale + left_margin + 0.5);
  else
    x_image = (int) (chunk->rect.left * t_scale + left_margin + 0.5);

  /*--- generate PS hex bitmap for image ---*/
  if(bkr_imagetops(
                (XImage *)chunk->handle, 
		fp,
		colortable,
		x_image,
		y_image,
                (int) BKR_K * paperheight,
		image_scale, out_fname, parent_w))
    return(1);

#ifdef PS_DIAG
z = y_image - (image->pix_height * t_scale + 0.5);
sprintf(workbuf,"%d %d XY FCPRT (x)S %d %d XY (z)S",x_image,y_image,x_image,z);
FLUSH_LINE;
ADD_TO_LINE(workbuf);
FLUSH_LINE;
#endif


#endif
  return(0);
  }


/*----------------------- bkr_psdocsetup ----------------------------
| DESCRIPTION: writes converted PS document setup area, font defs
--------------------------------------------------------------------*/
extern int 
bkr_psdocsetup PARAM_NAMES((book,out_fname,fp,parent_widget))
  BKR_BOOK_CTX    *book PARAM_SEP
  char            *out_fname PARAM_SEP
  FILE            *fp PARAM_SEP
  Widget          parent_widget PARAM_END
  {
  char            font_family[80];
  char            ps_font_string[80];
  char            font_style[80];
  char            font_weight[80];
  char            font_set_width_name[80];
  long            point_size;
  unsigned long   char_set;
  BKR_FONT_DATA   *font;
  int             ps_pointsize;
  double          convert;
  char            *error_string;
  int             font_num;
  int             fnt_size;

  parent_w = parent_widget;

  F_PUTS("%%BeginSetup\n");
  F_PUTS("/PxlResolution 300 def\n");
  F_PUTS("/Resolution 3600 def\n");
  F_PUTS("/RasterScaleFactor PxlResolution Resolution div def\n");
  F_PUTS("/PortraitMode true def\n");
  F_PUTS("/PaperWidth 8.500 def\n");
  F_PUTS("/PaperHeight 11.000 def\n");
  F_PUTS("72 Resolution div dup scale\n%\n");

  F_PUTS("%Spot color array\n");
  F_PUTS("/ColorPalette [\n");
  F_PUTS("0.0 0.0 0.0 1.0 (BLACK) findcmykcustomcolor\n");
  F_PUTS("] def\n%\n");

  for(font_num = 0; font_num <= book->max_font_id; font_num++)
    {
    font = book->font_data[font_num];

    if(!font)
      {
      /* Some authoring tools do not generate contiguous font numbers,
       * so just skip over font numbers that don't translate to a font
       * name.
       */
      if(bri_book_font_name(book->book_id,font_num) == NULL) 
        {
        continue;
        }
      font = bkr_font_entry_init(book,font_num);
      }

    if(font == FONT_NOT_FOUND)
      {
      strcpy(font_family,"courier");
      strcpy(font_style,"roman");
      strcpy(ps_font_string,"Courier");
      point_size = 12;
      }
     else
      {
      bkr_font_parse_name(font->font_name, font_family, font_style,
                          font_weight, font_set_width_name, &point_size,
                          &char_set, ps_font_string);
      point_size /= 10;

      }
    if(!strcmp(ps_font_string,"Helvetica") && (point_size > 12))
      strcpy(ps_font_string,"Helvetica-Bold");
    sprintf(workbuf,"%% DefineFont:F%d Pointsize:%d\n", font_num, point_size);
    F_PUTS(workbuf);

    sprintf(workbuf,"/%s /%s@DOCPSE DOCPSE ReENCODE\n",ps_font_string,
            ps_font_string);
    F_PUTS(workbuf);
    convert = (point_size > 14)? scale_font_inc: scale_font_dec;
    ps_pointsize = (int)(point_size * convert + 0.5);
    sprintf(workbuf,"/F%d %d.0 /%s@DOCPSE DPSF\n",font_num,ps_pointsize,
            ps_font_string);
    F_PUTS(workbuf);
    }

  F_PUTS("%% DefineFont:FCPRT Pointsize:11\n");
  F_PUTS("/Times-Roman /Times-Roman@DOCPSE DOCPSE ReENCODE\n");
  fnt_size = (int)(550.0 * reduction + 0.5);
  sprintf(workbuf,"%d.0",fnt_size);
  F_PUTS("/FCPRT ");
  F_PUTS(workbuf);
  F_PUTS(" /Times-Roman@DOCPSE DPSF\n");

  F_PUTS("/DVC$PSFonts save def\n");
  F_PUTS("%%EndSetup\n%\n");

  return(0);
  }


/*----------------------- copyto7bit ---------------------------
Purpose: Copy string converting 8 bit chars to 7 bit postscript
-------  format.
--------------------------------------------------------------*/
static void copyto7bit(tp, sp)
  char     *tp;  /* target/destination buffer */
  char     *sp;  /* source string */
  {
  while(*sp)
    {
    if (*sp & 128)
      {
      ps_octal((int)*sp, tp);
      tp += 4;
      sp++;
      }
    else
      *tp++ = *sp++;
    }
  *tp = '\0';
  return;
  }


/*----------------------- ps_octal ---------------------------
Purpose: Translate char into octal, return in buf.
-------
--------------------------------------------------------------*/
static void ps_octal(ch, buf)
  int      ch;    /* char to translate into octal */
  char     *buf;  /* buffer to put char into */
  {
  ch = ch & 255;
  buf[3] = (char)(ch % 8 + 48);
  ch = ch / 8;
  buf[0] = '\\';
  buf[1] = (char)(ch / 8 + 48);
  buf[2] = (char)(ch % 8 + 48);
  buf[4] = '\0';
  return;
  }

/*-----------------------------------------------------------
| begin_page - start a PS page, if first generate copyright
+----------------------------------------------------------*/
static int
begin_page(fp,book,out_fname)
  FILE          *fp;
  BKR_BOOK_CTX  *book;
  char          *out_fname;
  {
  char          *copyright;
  int           copyrt_from_book;
  int           folio;
  char          *error_string;
  char		tempbuf[WRKBUFSIZ];
  static char   *copyright_notice = (char *)0;

  in_page = TRUE;
  y = top_margin;
  active_font = -1;
  start_page = TRUE;
  text_top_y = text_max_y;
  text_max_y = -10000;

  psbuf[0] = '\0';
  folio = pagenum + 1;
  sprintf(workbuf,"%%%%Page: %d %d\n",folio,folio);
  F_PUTS(workbuf);
  F_PUTS("PaperHeight PaperWidth PM 0 SC\n");

  if(!pagenum)  /* set font and write copyright string */
    {
    copyrt_from_book = TRUE;
    if(!(copyright = bri_book_copyright_info(book->book_id)))
      {
      if(!copyright_notice)
        copyright_notice = (char *)bkr_fetch_literal("copyright",MrmRtypeChar8);
      copyright = copyright_notice;
      copyrt_from_book = FALSE;
      }

    /* Convert any 8-bit chars to /xxx.
     */
    copyto7bit(tempbuf,copyright);

    sprintf(workbuf,"FCPRT %d %d XY\n (%s)S\n",left_margin,top_margin,
            tempbuf);

    F_PUTS(workbuf);
    if(copyrt_from_book)
      BriFree(copyright,book->book_id);
    }

  return(0);
  }


/*-----------------------------------------------------------
| end_page - end PS page, check for graphics
+----------------------------------------------------------*/
static int
end_page(fp, book, topic, textlines, b_graph, n_graphics, b_trule, n_trules, out_fname)
  FILE            *fp;
  BKR_BOOK_CTX    *book;
  BKR_TOPIC_DATA  *topic;
  txtline_t       **textlines;
  graphic_t       **b_graph;
  int             n_graphics;
  trule_t	  **b_trule;
  int             n_trules;
  char            *out_fname;
  {
  BMD_CHUNK       *chunk;
  graphic_t       *graphic;
  graphic_t       **gr;
  txtline_t       **txt;
  char            *error_string;
  int             pagenum_x,pagenum_y;
  int             i;

#ifdef BITMAP_IMAGES
  if(topic && book)
    {
#ifdef TRULE
    /* draw undrawn trules for this topic */
    if(gen_ps_rule(fp,book,topic,textlines,b_trule,n_trules,out_fname))
      return(1);
#endif

    /* loop on graphics for this topic */
    for(i=0, gr=b_graph; i < n_graphics; ++i, ++gr)
      {
      graphic = *gr;
      if(!graphic->drawn)
        { /* check if graphic goes with text that's already drawn */
        if(((graphic->top_y + graphic->pix_height) <= text_max_y) &&
            (graphic->pix_width < MAX_ICON_SIZE))
          {
          /* draw the graphic */
          if(gen_ps_image(fp,book,topic,textlines,graphic->chunk,out_fname))
            return(1);
          graphic->drawn = TRUE;
          }
        }
      }
    }
#endif

  in_page = FALSE;
  ++pagenum;
  if(topic)
    {
    for(i=0, txt=textlines; i < topic->n_lines; ++i, ++txt)
      (*txt)->drawn = FALSE;
    }
  FLUSH_LINE;
  pagenum_x = paperwidth * (int)BKR_K - 2700;
  pagenum_y = paperheight  * (int)BKR_K - 1000;
  sprintf(workbuf,"%d %d XY FCPRT (%d)S", pagenum_x, pagenum_y, pagenum);
  ADD_TO_LINE(workbuf);
  FLUSH_LINE;
  /* sprintf(workbuf,"%d PP EP",pagenum); */
  strcpy(workbuf,"EP PP");
  ADD_TO_LINE(workbuf);
  FLUSH_LINE;
  ADD_TO_LINE("%%PageTrailer");
  FLUSH_LINE;

  return(0);
  }


/*-----------------------------------------------------------
| bkr_ps_begindoc
+----------------------------------------------------------*/
extern int bkr_ps_begindoc(out_fname, fp, parent_widget)
  char       *out_fname;
  FILE       *fp;
  Widget     parent_widget;
  {
  char       *error_string;

  parent_w = parent_widget;

  pagenum = 0;

  return(0);
  }


/*-----------------------------------------------------------
| bkr_ps_enddoc
+----------------------------------------------------------*/
extern int bkr_ps_enddoc(out_fname, fp, parent_widget)
  char       *out_fname;
  FILE       *fp;
  Widget     parent_widget;
  {
  char       *error_string;

  parent_w = parent_widget;

  if(in_page)
    end_page(fp,(BKR_BOOK_CTX *)0,(BKR_TOPIC_DATA *)0,(txtline_t **)0,
             (graphic_t **)0,0,(trule_t **)0,0,out_fname);

  FLUSH_LINE;
  F_PUTS("%%Trailer\n");
  sprintf(workbuf,"%%%%Pages: %d\n",pagenum);
  F_PUTS(workbuf);
  F_PUTS("DVC$PSJob restore\n");
  F_PUTS("end %DEC_DVC$dict\n");
  F_PUTS("end %Color5044\n%\n");
  F_PUTS("%%EOF\n");

  return(0);
  }
/*---------------------------------------------------------*/


#define OPEN_DFLT "dna="

/*----------------------- includeprolog -----------------------
Purpose: Open and write contents of infile to fp. Returns
-------  ERROR if failed, 0 otherwise.
--------------------------------------------------------------*/
extern int includeprolog(in_area,in_fnam,out_fname,fp,parent_widget)
  char   *in_area;              /* ptr to default area */
  char   *in_fnam;              /* name of file to copy */
  char   *out_fname;
  FILE   *fp;               /* ptr to file to copy to */
  Widget parent_widget;
  {
  FILE   *in_fp;                /* ptr to file to copy from */
  int    dna_len;               /* default name length */
  char   *dna_str;              /* default name string */
  char   *error_string;
  int    ch;                    /* char read from input, written */

  dna_str = (char *) 0;

  parent_w = parent_widget;

#ifdef VMS
  /* settup default name and area if specified */
  if(in_area)
    {
    if(*in_area)
      {
      dna_len = strlen(in_area) + strlen(OPEN_DFLT) + 1;
      if(!(dna_str = calloc(1,dna_len)))
        return(ERROR);
      strcpy(dna_str,OPEN_DFLT);
      strcat(dna_str,in_area);
      }
    }
#endif

  /* open input file for read */

  if(dna_str)
    in_fp = fopen(in_fnam,"r",dna_str);
  else
    in_fp = fopen(in_fnam,"r");

  if(!in_fp)

    {
    if(error_string = (char *) bkr_fetch_literal("PE_CANT_OPEN_READ",ASCIZ))
      {
      sprintf( errmsg, error_string, in_fnam );
      bkr_error_modal( errmsg,  XtWindow(parent_w));
      XtFree( error_string );
      }
      /* printf("\n could not open %s for read\n",psfname); */
    return(ERROR);
    }

  /*--- write introductory info to start prolog ---*/
  F_PUTS("%!PS-Adobe-3.0\n");
  sprintf(workbuf,"%%%%Title: %s\n",out_fname);
  F_PUTS(workbuf);
  sprintf(workbuf,"%%%%Creator: %s\n","Bookreader PS Converter");
  F_PUTS(workbuf);

  for(;;)
    {
    GET1BYTE

    if(feof(in_fp) && (ch == EOF))
      break;

    PUT1BYTE
    }

  /*--- close input file ---*/
  fclose(in_fp);

  return(0);
  }


#endif /* PRINT */

