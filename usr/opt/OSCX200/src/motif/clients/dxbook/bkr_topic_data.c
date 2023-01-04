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
#module BKR_TOPIC_DATA "V03-0001"
#endif
#ifndef VMS
  */
#endif

/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1990  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use,	     **
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
**      Bookreader User Interface (bkr)
**
**  ABSTRACT:
**
**	Routines for accessing topic data
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     26-Feb-1992
**
**  MODIFICATION HISTORY:
**
**
**--
**/

/*
** Include Files
*/
#include <stdio.h>
#include "br_common_defs.h"  /* common BR #defines */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_font.h"
#include "bkr_topic_data.h"  /* function prototypes for .c module */

/*
** Local Defines
*/
#define TEXT_PTR( pkt )	    ( (BMD_TEXT_PKT *) &( (BMD_FTEXT_PKT *) (pkt) )->value[0] )
#define WORD_PTR( txt )	    ( (BMD_WORD_PKT *) &(txt)->data[0] )
#define WORD_START( pkt )   WORD_PTR( TEXT_PTR( (pkt) ) )
#define WORD_END( pkt )	    ( (BMD_WORD_PKT *) &(pkt)->value[(pkt)->len - 2] )


#define APPEND_TO_CHUNK_LIST(end_ptr,chunk_ptr) { *end_ptr = chunk_ptr ; end_ptr = &chunk_ptr->next; }


/*
** Local Data Types
*/
typedef struct _TEXT_FRAGMENT {
    struct _TEXT_FRAGMENT *next_x;
    struct _TEXT_FRAGMENT *next_y;
    struct _TEXT_FRAGMENT *above;  /* fragment at same x position above */
    struct _TEXT_FRAGMENT *below;  /* fragment at same y position above */
    int baseline;
    int unscaled_x;
    int x;
    int y;
    int width;
    int height;
    int font_num;
    BKR_FONT_DATA *font_data;
    Boolean text300;
    BMD_WORD_PKT *words;
    BMD_WORD_PKT *end_of_words;
    BMD_CHUNK *parent;
    BKR_TEXT_LINE *line;
    int first_char;
    int end_char;
} TEXT_FRAGMENT ;


/*
** Function prototypes for local rooutines
*/
static Boolean
fragment_in_hotspot PROTOTYPE((
    BMD_CHUNK *chunk,
    TEXT_FRAGMENT *fragment
));

static void 
calculate_rectangle PROTOTYPE((
    BMD_CHUNK *chunk,
    BMD_VERSION version
));

static void
add_text_packets PROTOTYPE((
    TEXT_FRAGMENT **first_fragment,
    BMD_CHUNK   *chunk,
    BMD_CHUNK   *parent_chunk
));

static void
create_text_lines PROTOTYPE((
    BKR_BOOK_CTX *book,
    TEXT_FRAGMENT *first_fragment,
    BKR_TOPIC_DATA *topic
));

BKR_TOPIC_DATA *
bkr_topic_data_get PARAM_NAMES((book,page_id))
    BKR_BOOK_CTX *book PARAM_SEP
    BMD_OBJECT_ID page_id PARAM_END
/*
 *
 * Function description:
 *
 *      Calls the BRI to read thetopic data from the file and initializes
 *      the topic data structures (chunks, lines, etc.) that aren't window
 *      specific.
 *
 * Arguments:
 *
 *      book - pointer to the book context
 *
 *      page_id - the page_id of the topic
 *
 * Return value:
 *
 *      A pointer to the topic data or NULL if the data can't be read or
 *      initialized.
 *
 * Side effects:
 *
 *      Stale topic data, for topics that aren't currently in use may
 *      be deleted.
 *
 */

{
    BMD_CHUNK   *chunk;
    int	    	cknum;
    BKR_TOPIC_DATA **ptr_to_ptr;
    BKR_TOPIC_DATA *data = NULL;
    TEXT_FRAGMENT *first_fragment = NULL;
    BMD_CHUNK **last_graphic_ptr;
    BMD_CHUNK **last_extension_ptr;
    BMD_CHUNK **last_hotspot_ptr;
    unsigned int unscaled_width;
    unsigned int unscaled_height;

    /* Search the open topics to see if the topic for the specified
     * page id is already open.
     */
    ptr_to_ptr = &book->open_topics;

    while (*ptr_to_ptr) {

        BKR_TOPIC_DATA *ptr = *ptr_to_ptr;

        if (ptr->page_id == page_id) {

            /* Found the topic, make sure it's use count is at least 1.
             */
            if (ptr->use_count > 0) {
                ptr->use_count++;
            } else {
                ptr->use_count = 1;
            }
            
            /* Remember the pointer to the data, but keep going through
             * the list so we can delete any stale data.
             */
            data = ptr;
            ptr_to_ptr = &ptr->next;

        } else if (ptr->use_count < 0) {

            /* This data hasn't been used in a while so we can delete it
             */
            *ptr_to_ptr = ptr->next;
            bkr_topic_data_delete(book,ptr);

        } else {

            /* This data isn't being used right now, but let it stay
             * on the list a little longer in case we try to use it
             * again soon.
             */
            if (ptr->use_count == 0) {
                ptr->use_count--;
            }
            ptr_to_ptr = &ptr->next;
        } 
    }

    /* If we already had the topic in memory just return it
     */
    if (data) {
        return data;
    }

    /* Allocate the topic data.
     */
    data = (BKR_TOPIC_DATA *)BKR_MALLOC(sizeof(BKR_TOPIC_DATA));
    if (data == NULL) {
        return data;
    }
    memset(data,0,sizeof(BKR_TOPIC_DATA));

    /* Open the page in the book
     */
    data->type = (BKR_WINDOW_TYPE)bri_page_open(book->book_id,page_id);
    if ((data->type != BKR_STANDARD_TOPIC) && (data->type != BKR_FORMAL_TOPIC)) {
        BKR_FREE(data);
        return NULL;
    }

    /* Put new topic at start of the list.
     */
    data->next = book->open_topics;
    book->open_topics = data;

    data->use_count = 1;
    data->page_id = page_id;

    /* Get the chunks
     */
    data->num_chunks = bri_page_chunk_count(book->book_id,page_id);
    data->chunk_list = (BMD_CHUNK *) BKR_CALLOC( data->num_chunks, sizeof( BMD_CHUNK ) );
    if (data->chunk_list == NULL) {
        BKR_FREE(data);
        return NULL;
    }
    memset( data->chunk_list, 0, data->num_chunks * sizeof( BMD_CHUNK ) );
    bri_page_data(book->book_id, page_id, data->chunk_list );

    /*  
     *  Find the title for the Topic.  First we search the chunks
     *  for the first title we find and if no title is found,
     *  we default to the books title.
     */

    for ( cknum = 0; cknum < data->num_chunks; cknum++ )
    {
	data->title = bri_page_chunk_title( book->book_id, data->chunk_list[cknum].id );
	if ( ( data->title != NULL ) && ( data->title[0] != NULLCHAR ) )
	    break;
    }

    if ( ( ( data->title != NULL ) && ( data->title[0] == NULLCHAR ) ) 
    	    || ( data->title == NULL ) )
	data->title = bri_book_title( book->book_id );

    /* Initialize pointers to special chunk lists
     */
    last_graphic_ptr = &data->graphic_data;
    last_extension_ptr = &data->extensions;
    last_hotspot_ptr = &data->hot_spots;

    /* Figure out where the chunks go and what they are.
     */
    unscaled_width = 0;
    unscaled_height = 0;

    for ( cknum = 0; cknum < data->num_chunks; cknum++ )
    {
	chunk = &data->chunk_list[cknum];

	if ( chunk->parent == 0 )   	/*  main level chunk?  */
	{
            chunk->unscaled_y = unscaled_height;
	    unscaled_width = MAX( unscaled_width, chunk->unscaled_width );
	    unscaled_height += chunk->unscaled_height;

            calculate_rectangle(chunk,book->version);

            /* Add FTEXT packets to the list of packets for the topic
             */
            if (chunk->data_type == BMD_CHUNK_FTEXT)
            {
                add_text_packets(&first_fragment,chunk,chunk);
            }
            APPEND_TO_CHUNK_LIST(last_graphic_ptr,chunk);
	}
	else	    	    	    /*  sub-chunk, x-ref, or extension  */
	{
	    BMD_CHUNK	*pchunk = NULL;
	    int		pcknum;

	    /*  find the parent chunk  */
	    for ( pcknum = 0; pcknum < cknum ; pcknum++ )
	    {
		if ( data->chunk_list[pcknum].id == chunk->parent )
		{
                    pchunk = &data->chunk_list[pcknum];
                    break;
                }
            }
            if (pchunk) 
            {
                chunk->unscaled_x += pchunk->unscaled_x;
                chunk->unscaled_y += pchunk->unscaled_y;
                
                calculate_rectangle(chunk,book->version);

                switch (chunk->chunk_type) 
                {
                    case BMD_DATA_SUBCHUNK:
                    {
                        if (chunk->data_type == BMD_CHUNK_FTEXT) 
                        {
                            /* Add FTEXT packets to the list of packets for the topic
                             */
                            add_text_packets(&first_fragment,chunk,pchunk);
                        } 
                        APPEND_TO_CHUNK_LIST(last_graphic_ptr,chunk);
                        break;
                    }
                    case BMD_REFERENCE_RECT:
                    {
                        APPEND_TO_CHUNK_LIST(last_hotspot_ptr,chunk);
                        break;
                    }
                    case BMD_EXTENSION_RECT:
                    {
                        APPEND_TO_CHUNK_LIST(last_extension_ptr,chunk);
                        break;
                    }
                    case BMD_REFERENCE_POLY:
                    case BMD_EXTENSION_POLY:
                    {
                        int	point;

                        chunk->xpoint_vec = (XPoint *)BKR_CALLOC( chunk->num_points + 1, sizeof(XPoint) );
                        
                        for ( point = 0; point < chunk->num_points; point++ )
                        {
                            int	    scaled_x, scaled_y;
                            
                            chunk->point_vec[point].x 
                                = SCALE_VALUE(chunk->point_vec[point].x,  
                                          book->version.major_num )
                                + pchunk->rect.left;

                            chunk->xpoint_vec[point].x  = chunk->point_vec[point].x 
                                                          - chunk->rect.left ;

                            chunk->point_vec[point].y 
                                = SCALE_VALUE(chunk->point_vec[point].y, 
                                              book->version.major_num)
                                + pchunk->rect.top;

                            chunk->xpoint_vec[point].y  = chunk->point_vec[point].y 
                                - chunk->rect.top ;
                        }

                        /*
                         *  we allocated an extra point to ensure closure of 
                         *  the polygon.  we repeat the first point at the end 
                         *  of the list to ensure this closure.
                         */
                        chunk->xpoint_vec[chunk->num_points].x = chunk->xpoint_vec[0].x;
                        chunk->xpoint_vec[chunk->num_points].y = chunk->xpoint_vec[0].y;
                        
                        chunk->region = XPolygonRegion(chunk->xpoint_vec,
                                                       chunk->num_points+1,
                                                       WindingRule );
                        

                        if ( chunk->chunk_type == BMD_REFERENCE_POLY )
                        {
                            APPEND_TO_CHUNK_LIST(last_hotspot_ptr,chunk);
                        }
                        else 
                        {
                            APPEND_TO_CHUNK_LIST(last_extension_ptr,chunk);
                        }

                        break;
                    }
                } /* end of switch(chunk->chunk_type) */

            } /* end of "if ( pchunk )" */
	}   	/* end of "if ( chunk->parent == 0 )"  */
    }	    /* end of "for: cknum < data->num_chunks"  */

    data->width = SCALE_VALUE( unscaled_width, book->version.major_num ) + 1;
    data->height = SCALE_VALUE( unscaled_height, book->version.major_num ) + 1;

    data->width = MAX( 1, data->width );  /* Avoid Xlib error for
                                             empties */
#ifdef PAD_BOTTOM
    data->height += 20;	    	    /* Pad the bottom */
#endif 

    if (first_fragment) 
    {
        TEXT_FRAGMENT *y_fragment;
        TEXT_FRAGMENT *x_fragment;

        create_text_lines(book,first_fragment,data);

        if (bkrplus_g_charcell_display) 
        {
            chunk = data->hot_spots ;

            while (chunk) 
            {
                Region new_region = (Region)0;

                if (chunk->data_type == BMD_REFERENCE_POLY) {
                    new_region = XCreateRegion();
                }
                chunk->n_segments = 0;
                y_fragment = first_fragment;

                while (y_fragment) 
                {
                    x_fragment = y_fragment;
                    while (x_fragment) 
                    {
                        TEXT_FRAGMENT *first = x_fragment;
                        TEXT_FRAGMENT *last = NULL ;

                        while (x_fragment && fragment_in_hotspot(chunk,x_fragment))
                        {
                            last = x_fragment;
                            x_fragment = x_fragment->next_x;
                        }
                        if (last)
                        { 
                            if (chunk->n_segments == 0) {
                                chunk->segments = (XSegment *)BKR_MALLOC(sizeof(XSegment));
                            }
                            else 
                            {
                                chunk->segments = (XSegment *)BKR_REALLOC(chunk->segments,
                                                                          (sizeof(XSegment)*(chunk->n_segments+1)));
                            }
                            chunk->segments[chunk->n_segments].x1 
                                = (first->first_char * bkr_default_space_width)
                                - chunk->rect.left;

                            chunk->segments[chunk->n_segments].x2 
                                = (last->end_char * bkr_default_space_width)
                                - chunk->rect.left;
                            
                            chunk->segments[chunk->n_segments].y1 
                                = first->line->baseline - chunk->rect.top;
                            chunk->segments[chunk->n_segments].y2  
                                = chunk->segments[chunk->n_segments].y1; 


                            if (chunk->data_type == BMD_REFERENCE_POLY) 
                            {
                                XRectangle rect;

                                rect.x = chunk->segments[chunk->n_segments].x1;
                                rect.y = first->line->y;
                                rect.width = chunk->segments[chunk->n_segments].x2 - rect.x;
                                rect.height = first->line->height;
                                
                                XUnionRectWithRegion(&rect,new_region,new_region);
                            }
                            chunk->n_segments++;
                        }
                        else 
                        {
                            x_fragment = x_fragment->next_x;
                        }
                    }
                    y_fragment = y_fragment->next_y;
                }
                if (chunk->data_type == BMD_REFERENCE_POLY) 
                {
                    if (XEmptyRegion(new_region))
                    {
                        XDestroyRegion(new_region);
                    }
                    else 
                    {
                        XDestroyRegion(chunk->region);
                        chunk->region = new_region;
                    }
                }
                chunk = chunk->next;
            }
        }
        y_fragment = first_fragment;
        while (y_fragment) {
            TEXT_FRAGMENT *next_y;

            next_y = y_fragment->next_y;
            x_fragment = y_fragment;
            while (x_fragment) {
                TEXT_FRAGMENT *next_x;

                next_x = x_fragment->next_x;
                BKR_FREE(x_fragment);
                x_fragment = next_x;
            }
            y_fragment = next_y;
        }
    }

#ifdef DEBUG_TEXT_LINES
    for ( cknum = 0; cknum < data->num_chunks; cknum++ )
    {
	chunk = &data->chunk_list[cknum];

	if ( chunk->parent == 0 )   	/*  main level chunk?  */
	{
            BKR_TEXT_LINE *line = chunk->first_line;
            int i;
            
            fprintf(stderr,"===\n=== Chunk %d\n===\n",chunk->id);
            for ( i = 1 ; i <= chunk->n_lines; i++) {
                if (line == NULL) {
                    fprintf(stderr,
                            "***\n*** Only %d lines, should be %d.\n***\n",
                            i,
                            chunk->n_lines);
                    break;
                }
                if (line->parent_chunk && (line->parent_chunk != chunk)) {
                    fprintf(stderr,
                            "***\n*** This line belongs really belongs to chunk %d\n***\n",
                            line->parent_chunk->id);
                }
                fprintf(stderr,line->chars);
                line = line->next;
            }
        } else if (chunk->first_line || (chunk->n_lines > 0)){
            fprintf(stderr,"***\n*** Subchunk %d has lines\n***\n",chunk->id);
        }
    }
#endif 
    return data;

} /* end bkr_topic_data_get */

void
bkr_topic_data_free PARAM_NAMES((book,page_id))
    BKR_BOOK_CTX *book PARAM_SEP
    BMD_OBJECT_ID page_id PARAM_END
/*
 *
 * Function description:
 *
 *      Free's the specified topic by decrementing the use count but
 *      doesn't delete the data strucutres.
 *
 * Arguments:
 *
 *      book - pointer to the book context
 *
 *      page_id - the page_id of the topic
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      None
 *
 */

{
    BKR_TOPIC_DATA *data;

    data = book->open_topics;
    while (data) {
        if (data->page_id == page_id) {
            data->use_count--;
            break;
        }
        data = data->next;
    }
} /* end bkr_topic_data_free */

void
bkr_topic_data_delete PARAM_NAMES((book,topic))
    BKR_BOOK_CTX *book PARAM_SEP
    BKR_TOPIC_DATA *topic PARAM_END
/*
 *
 * Function description:
 *
 *      Deletes the data for a topic.  Called by bkr_topic_data_get
 *      when it finds stale data and by bkr_book_free.
 *
 * Arguments:
 *
 *      book - pointer to the book context
 *
 *      page_id - the page_id of the topic
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      The page record is closed.
 *
 */

{
    int cknum;

    if ( topic->chunk_list != NULL )
    {
        for ( cknum = 0; cknum < topic->num_chunks; cknum++ )
        {
            BMD_CHUNK  *chunk = &topic->chunk_list[cknum];
            
            switch ( chunk->data_type ) {
                case BMD_CHUNK_FTEXT: {
                    break;
                }
                
                case BMD_CHUNK_RAGS:
                case BMD_CHUNK_RAGS_NO_FILL: {
#ifndef NO_RAGS
                    if ( ( chunk->handle ) && ( chunk->handle != NODISPLAY ) ) {
                        bkr_rags_close( chunk->handle );
                    }
#endif 
                    break;
                }
                
                case BMD_CHUNK_IMAGE:
                case BMD_CHUNK_IMAGE75: {		/* NOTE: Version 1.0 books ONLY */
                    if ( chunk->handle ) {
                        bkr_image_close( chunk );
                    }
                    break;  
                }
            }
            /* end of switch */
            
            if ( chunk->xpoint_vec != NULL )
            {
                BKR_CFREE( chunk->xpoint_vec );
                
                /*  check in here since point_vec originally points into
                 *  BRI space, then new vec is calloc'd at the same time
                 *	as the xpoint_vec
                 */
                if ( chunk->point_vec != NULL ) {
		    int *npoint = (int *)chunk->point_vec;
		    npoint -= 1;
                    BKR_CFREE( (BMD_POINT *)npoint );
                }
            }
            if ( chunk->region ) {
                XDestroyRegion( chunk->region );
            }
        }   /* end for loop */
        
        BKR_CFREE( topic->chunk_list );
        topic->num_chunks = 0;
        
    }	    /* end "if chunk_list" */

    if (topic->char_buffer) {
        BKR_FREE(topic->char_buffer);
    }
    if (topic->widths_buffer) {
        BKR_CFREE(topic->widths_buffer);
    }
    if (topic->text_item_lists) {
        BKR_CFREE(topic->text_item_lists);
    }
    if (topic->items) {
        BKR_CFREE(topic->items);
    }
    if (topic->font_nums) {
        BKR_CFREE(topic->font_nums);
    }
    if (topic->text_item_lists) {
        BKR_CFREE(topic->item16s);
    }
    while (topic->text_lines) {
        BKR_TEXT_LINE *line = topic->text_lines->next;
        BKR_FREE(topic->text_lines);
        topic->text_lines = line;
    }
    
    /* Close the page, the bri routines keep track of usage,
     * and will only do a real close if this window is the last
     * user.
     */
    bri_page_close( book->book_id, topic->page_id );

    BKR_FREE(topic);
    
} /* end bkr_topic_data_delete */


BMD_CHUNK *
bkr_topic_chunk_get PARAM_NAMES((book,chunk_id))
    BKR_BOOK_CTX *book PARAM_SEP
    BMD_OBJECT_ID chunk_id PARAM_END
/*
 *
 * Function description:
 *
 *      Makes sure that the specified chunk is in memory and returns a
 *      pointer to it.
 *
 * Arguments:
 *
 *      book - pointer to the book context
 *
 *      chunk_id - id of the chunk to get.
 *
 * Return value:
 *
 *      Pointer to the chunk or NULL if it can't be read in.
 *
 * Side effects:
 *
 *      May result in the topic data being read in.
 *
 */

{
    BKR_TOPIC_DATA *topic;

    /* Lookup the page id for the chunk, call bkr_topic_data_get to
     * get the topic and then find the chunk in the topic.
     */
    topic = bkr_topic_data_get(book,bri_page_chunk_page_id(book->book_id,chunk_id));
    if (topic) {
        int i;
        for (i = 0 ; i < topic->num_chunks; i++) {
            if (topic->chunk_list[i].id == chunk_id) {
                return &topic->chunk_list[i];
            }
        }
    }
    return NULL;
} /* end bkr_topic_chunk_get */

void
bkr_topic_chunk_free PARAM_NAMES((book,chunk_id))
    BKR_BOOK_CTX *book PARAM_SEP
    BMD_OBJECT_ID chunk_id PARAM_END
/*
 *
 * Function description:
 *
 *      Frees the specified chunk.
 *
 * Arguments:
 *
 *      book - pointer to the book context
 *
 *      chunk_id - id of the chunk to get.
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      None
 *
 */

{
    /* Just translate the chunk id to a page id and free the topic
     */
    bkr_topic_data_free(book,bri_page_chunk_page_id(book->book_id,chunk_id));

} /* end bkr_topic_chunk_free */

static void 
calculate_rectangle PARAM_NAMES((chunk,version))
    BMD_CHUNK *chunk PARAM_SEP
    BMD_VERSION version PARAM_END
    
{
    chunk->rect.top    = SCALE_VALUE(chunk->unscaled_y,version.major_num);
    chunk->rect.bottom = SCALE_VALUE((chunk->unscaled_y + chunk->unscaled_height),version.major_num) + 1;
    chunk->rect.height = chunk->rect.bottom - chunk->rect.top;

    chunk->rect.left = SCALE_VALUE(chunk->unscaled_x,version.major_num);
    chunk->rect.right = SCALE_VALUE((chunk->unscaled_x + chunk->unscaled_width),version.major_num) + 1;
    chunk->rect.width = chunk->rect.right - chunk->rect.left;
}

static void
add_text_packets PARAM_NAMES((first_fragment,chunk,parent_chunk))
    TEXT_FRAGMENT **first_fragment PARAM_SEP
    BMD_CHUNK *chunk PARAM_SEP
    BMD_CHUNK *parent_chunk PARAM_END

/*
 *
 * Function description:
 *
 *      Adds the FTEXT packets for a chunk to a list of packets for
 *      the topic sorted by their x and y coordinates.
 *
 * Arguments:
 *
 *	first_fragment - Address of the pointer to the first packet in 
 *                     the list.
 *
 *	chunk        - pointer to the chunk or subchunk that the ftext
 *	               packet comes from.
 *
 *	parent_chunk - if chunk is a subchunk then this is a pointer
 *                     to its parent chunk, otherwise chunk and
 *                     parent_chunk are the same.
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      Memory allocation
 *
 */

{
    BMD_FTEXT_PKT  *packet;
    BMD_FTEXT_PKT  *data_end;

    if (chunk->data_addr == NULL) {
        return;
    }
    
    packet = (BMD_FTEXT_PKT *)chunk->data_addr;
    data_end = (BMD_FTEXT_PKT *)&chunk->data_addr[chunk->data_len];

    while (packet < data_end) {

        if ((packet->tag == BMD_FTEXT_TEXT300) 
            || (packet->tag == BMD_FTEXT_TEXT400)
            ) 
        {
            BMD_TEXT_PKT  text_pkt;
            BMD_TEXT_PKT  *text = &text_pkt;
            TEXT_FRAGMENT *new_fragment;
            TEXT_FRAGMENT *fragment;
            TEXT_FRAGMENT *previous_x;
            TEXT_FRAGMENT *previous_y;


            /* Attempt to allocate a text fragment.
             */
            new_fragment = (TEXT_FRAGMENT *)BKR_MALLOC(sizeof(TEXT_FRAGMENT));
            if (new_fragment == NULL) {
                break;
            }

            /* begin deconstructing the packet, so we can sort it */
#ifdef VMS
            text = TEXT_PTR(packet);
#else
            memcpy(&text_pkt,TEXT_PTR(packet),sizeof( text_pkt ));
#endif
            /* Use the text packet to initialize the text fragment
             */
            new_fragment->unscaled_x = text->x + chunk->unscaled_x;
            if (packet->tag == BMD_FTEXT_TEXT300) 
            {
                new_fragment->text300 = TRUE;
                new_fragment->x = SCALE_VALUE300(new_fragment->unscaled_x);
                new_fragment->baseline = SCALE_VALUE300((text->y + chunk->unscaled_y));
            }
            else 
            {
                new_fragment->text300 = FALSE;
                new_fragment->x = SCALE_VALUE400(new_fragment->unscaled_x);
                new_fragment->baseline = SCALE_VALUE400((text->y + chunk->unscaled_y));
            }
            new_fragment->font_num = text->font_num;
            new_fragment->words = WORD_START(packet);
            new_fragment->end_of_words = WORD_END(packet);
            new_fragment->parent = parent_chunk;
            new_fragment->line = NULL;
            
            /* Sort the text fragment with the previous fragments.
             */
            new_fragment->next_x = NULL;
            new_fragment->next_y = NULL;

            fragment = *first_fragment;
            previous_x = NULL;
            previous_y = NULL;

            while (fragment && (fragment->baseline <= new_fragment->baseline)) {
                if (fragment->baseline == new_fragment->baseline) {
                    while (fragment && (fragment->x <= new_fragment->x)) {
                        previous_x = fragment;
                        fragment = fragment->next_x;
                    }
                    break;
                } else {
                    previous_y = fragment;
                    fragment = fragment->next_y;
                }
            }
            
            /* Found where it goes in the list, now insert the packet.
             */
            if (previous_x) {
                new_fragment->next_x = fragment;
                previous_x->next_x = new_fragment;
            } else {
                if (previous_y) {
                    previous_y->next_y = new_fragment;
                } else {
                    *first_fragment = new_fragment;
                }
                if (fragment && (new_fragment->baseline == fragment->baseline)) {
                    new_fragment->next_y = fragment->next_y;
                    new_fragment->next_x = fragment;
                } else {
                    new_fragment->next_y = fragment;
                }
            }
        }
        packet = (BMD_FTEXT_PKT *)&packet->value[packet->len - 2];
        if (packet->len == 0) {
            break;
        }
    }
} /* end add_text_packets */

static Boolean
fragment_in_hotspot PARAM_NAMES((chunk,fragment))
    BMD_CHUNK *chunk PARAM_SEP
    TEXT_FRAGMENT *fragment PARAM_END
{
    Boolean status = FALSE;
    int x = fragment->x + 1;
    int y = fragment->baseline - 1;

    switch (chunk->chunk_type) 
    {
        case BMD_REFERENCE_POLY:
        {
            return XPointInRegion(chunk->region,(x - chunk->rect.left),(y - chunk->rect.top));
        }
        case BMD_REFERENCE_RECT:
        {
            if ((x >= chunk->rect.left) && (x <= chunk->rect.right)
                && (y >= chunk->rect.top) && (y <= chunk->rect.bottom)
                ) 
            {
                return TRUE; 
            }
            break;
        }
    }
    return FALSE;
}

static 
insert_spaces PARAM_NAMES((line,delta,min_interword_spacing))
    BKR_TEXT_LINE *line PARAM_SEP
    int delta PARAM_SEP
    unsigned long min_interword_spacing PARAM_END

/*
 *
 * Function description:
 *
 *      Inserts a number of space characters into a line, based on the
 * 	width of the white space.
 *
 * Arguments:
 *
 *      line - the line
 *      
 *	delta - width of the white space
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      None
 *
 */

{
    if (delta >= bkr_default_space_width) {
        while (delta >= bkr_default_space_width) {
            line->chars[line->n_bytes] = ' ';
            line->char_widths[line->n_bytes] = bkr_default_space_width;
            line->n_bytes++;
            delta -= bkr_default_space_width;
        }
        if (bkrplus_g_charcell_display == 0) {
            line->char_widths[line->n_bytes - 1] += delta;
        }
    } else if (delta >= min_interword_spacing) {
        line->chars[line->n_bytes] = ' ';
        if (bkrplus_g_charcell_display == 0) {
            line->char_widths[line->n_bytes] = delta;
        } else {
            line->char_widths[line->n_bytes] = bkr_default_space_width;
        }
        line->n_bytes++;
    }

} /* end insert_spaces */


static void
create_text_lines PARAM_NAMES((book,first_fragment,topic))
    BKR_BOOK_CTX *book PARAM_SEP
    TEXT_FRAGMENT *first_fragment PARAM_SEP
    BKR_TOPIC_DATA *topic PARAM_END

/*
 *
 * Function description:
 *
 *      Creates the text lines for a the topic data structure.
 *
 * Arguments:
 *
 *      book - the book the topic data belongs to
 *
 * 	first_fragment - pointer to the first of the sorted text fragments
 *
 * 	topic - pointer to the topic data
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      Memory allocation
 *
 */

{
    TEXT_FRAGMENT *y_fragment;
    BKR_TEXT_LINE *line;
    BKR_TEXT_LINE *prev_line = NULL;
    BKR_TEXT_LINE *blank_line;
    BKR_TEXT_ITEM_LIST *item_list;
    int bottom_of_previous_line = 0;
    int char_buffer_offset = 0;
    int char_buffer_size = 0;
    int max_lines = 0;
    int max_item_lists = 0;
    int max_items = 0;
    int max_item16s = 0;
    XTextItem *item;
    unsigned short *font_num_ptr;

    /* First go throught the fragments, allocating the BKR_TEXT_LINE data
     * structures and determining how many XTextItems,
     * BKR_TEXT_ITEM_LISTS will be needed and how big the charachter
     * and char widths buffers will need to be.
     */
    y_fragment = first_fragment;
    while (y_fragment) {

        TEXT_FRAGMENT *x_fragment = y_fragment;
        int max_ascent = 0;
        int max_descent = 0;
        int last_x = 0;

        max_lines++;

        /* Allocate the BKR_TEXT_LINE.
         */
        line = (BKR_TEXT_LINE *)BKR_MALLOC(sizeof(BKR_TEXT_LINE));
        if (line == NULL) {
            break;
        }
        memset(line,0,sizeof(BKR_TEXT_LINE));

        /* Figure out the number of XTextItems and item lists.
         */
        x_fragment = y_fragment;
        do {

            BMD_WORD_PKT *word = x_fragment->words;
            int x_delta;

            x_fragment->font_data = book->font_data[x_fragment->font_num];
            if (x_fragment->font_data == NULL) {
                x_fragment->font_data = bkr_font_entry_init(book,x_fragment->font_num);
            }
            x_fragment->y = x_fragment->baseline 
                          - x_fragment->font_data->font_struct->max_bounds.ascent;
            x_fragment->height = x_fragment->font_data->font_struct->max_bounds.ascent
                               + x_fragment->font_data->font_struct->max_bounds.descent;
            max_ascent = MAX(max_ascent,x_fragment->font_data->font_struct->max_bounds.ascent);
            max_descent = MAX(max_descent,x_fragment->font_data->font_struct->max_bounds.descent);

            x_delta = x_fragment->x - last_x;
            char_buffer_size += MAX(0,x_delta) / bkr_default_space_width;
            last_x = x_fragment->x;

            max_item_lists++;

            while (word < x_fragment->end_of_words) {

                char_buffer_size += (word->count
                                     + ((word->delta) / bkr_default_space_width) 
                                     + 1 
                                     );
                max_items++;

                word = (BMD_WORD_PKT *)&word->chars[word->count];
            }
            x_fragment = x_fragment->next_x;
        } while (x_fragment);

        char_buffer_size+=2;

        /* We now know the parent chunk for the line and the line
         * dimensions and coordinates.
         */
        line->parent_chunk = y_fragment->parent;
        line->ftext_baseline = y_fragment->baseline;
        line->baseline = y_fragment->baseline;
        line->y = line->baseline - max_ascent;
        line->height = max_ascent + max_descent;

        /* See if we need to insert any blank lines before this line.
         *
         * NOTE: This needs some fine tuning
         *
         */
        while ((line->y - bottom_of_previous_line) >= line->height) {
            blank_line = (BKR_TEXT_LINE *)BKR_MALLOC(sizeof(BKR_TEXT_LINE));
            if (blank_line == NULL) {
                break;
            }
            memset(blank_line,0,sizeof(BKR_TEXT_LINE));
            blank_line->prev = prev_line;
            if (prev_line) {
                prev_line->next = blank_line;
            } else {
                topic->text_lines = blank_line;
            }
            blank_line->y = bottom_of_previous_line;
            blank_line->ftext_baseline = blank_line->y + bkr_default_font->ascent;
            blank_line->baseline = blank_line->ftext_baseline;
            blank_line->height = line->height;
            blank_line->parent_chunk = line->parent_chunk;
            prev_line = blank_line;
            bottom_of_previous_line += line->height;
            char_buffer_size+=2;
            max_lines++;
        }
        if ((line->y - bottom_of_previous_line) >= bkr_default_font->descent) {
            blank_line = (BKR_TEXT_LINE *)BKR_MALLOC(sizeof(BKR_TEXT_LINE));
            if (blank_line == NULL) {
                break;
            }
            memset(blank_line,0,sizeof(BKR_TEXT_LINE));
            blank_line->prev = prev_line;
            if (prev_line) {
                prev_line->next = blank_line;
            } else {
                topic->text_lines = blank_line;
            }
            blank_line->y = bottom_of_previous_line;
            blank_line->height += (bottom_of_previous_line < line->y);
            blank_line->ftext_baseline = blank_line->y + (blank_line->height/2);
            blank_line->baseline = blank_line->ftext_baseline;
            blank_line->parent_chunk = line->parent_chunk;
            prev_line = blank_line;
            char_buffer_size+=2;
            max_lines++;
        }

        /* Now insert the line in the list of lines.
         */
        line->prev = prev_line;
        if (prev_line) {
            prev_line->next = line;
        } else {
            topic->text_lines = line;
        }
        prev_line = line;
        bottom_of_previous_line = line->y + line->height;
        y_fragment = y_fragment->next_y;
    }

    if (max_lines == 0) {
        return;
    }

    /* Allocate the character buffer
     */
    topic->char_buffer = (unsigned char *)BKR_MALLOC(char_buffer_size);
    if (topic->char_buffer == NULL) {
        BKR_FREE(topic->text_lines);
        return;
    }
    
    /* Allocate the character widths buffer.
     */
    topic->widths_buffer = (short *)BKR_CALLOC(char_buffer_size,sizeof(short));
    if (topic->char_buffer == NULL) {
        BKR_FREE(topic->char_buffer);
        BKR_FREE(topic->text_lines);
        return;
    }
    
    /* Alocate the item lists
     */
    item_list = (BKR_TEXT_ITEM_LIST *)BKR_CALLOC(max_item_lists,
                                                 sizeof(BKR_TEXT_ITEM_LIST));
    if (item_list == NULL) {
        BKR_FREE(topic->widths_buffer);
        BKR_FREE(topic->char_buffer);
        BKR_FREE(topic->text_lines);
        return;
    }
    memset(item_list,0,(max_item_lists*sizeof(BKR_TEXT_ITEM_LIST)));
    topic->text_item_lists = item_list;

    /* Allocate the text items
     */
    topic->items = (XTextItem *)BKR_CALLOC(max_items,sizeof(XTextItem));
    if (topic->items == NULL) {
        BKR_FREE(topic->text_item_lists);
        BKR_FREE(topic->widths_buffer);
        BKR_FREE(topic->char_buffer);
        BKR_FREE(topic->text_lines);
        return;
    }
    memset(topic->items,0,(max_items * sizeof(XTextItem)));
    item = topic->items;

    /* Allocate the font numbers
     */
    font_num_ptr = (unsigned short *)BKR_CALLOC(max_items,
                                                sizeof(unsigned short));
    if (font_num_ptr == NULL) {
        BKR_FREE(topic->items);
        BKR_FREE(topic->text_item_lists);
        BKR_FREE(topic->widths_buffer);
        BKR_FREE(topic->char_buffer);
        BKR_FREE(topic->text_lines);
        return;
    }
    memset(font_num_ptr,0,(max_items * sizeof(unsigned short)));
    topic->font_nums = font_num_ptr;


    /* Now make another pass through the fragments filling setting up the
     * data strucutres for each line.  We'll also delete the fragments
     * as we go.
     */
    line = topic->text_lines;
    y_fragment = first_fragment;

    while (line && y_fragment) {

        /* Start the line
         */
        line->n_bytes = 0;
        line->chars = &topic->char_buffer[char_buffer_offset];
        line->char_widths = &topic->widths_buffer[char_buffer_offset];

        /* Update the pointer to the first line and number of
         * lines in the parent chunk.
         */
        if (line->parent_chunk->first_line == NULL) {
            line->parent_chunk->first_line = line;
            line->parent_chunk->n_lines = 0;
        }
        line->parent_chunk->n_lines++;


        /* If the baseline of the line matches the y of the fragment
         * then ths is not a blank line, so set up the line based on the
         * x_fragments.
         */
        if (y_fragment->baseline == line->baseline) {

            TEXT_FRAGMENT *x_fragment;
            int end_of_last_fragment;

            line->x = 0;

            /* NOTE:  Only one item list per line now. This needs to be
             * fixed for I18N support.
             */
            line->n_item_lists = 1;
            line->item_lists = item_list;
            line->item_lists->is2byte = FALSE;
            line->item_lists->x = y_fragment->x;
            line->item_lists->u.items = item;
            line->item_lists->font_nums = font_num_ptr;

            x_fragment = y_fragment;
            y_fragment = y_fragment->next_y;

            end_of_last_fragment = x_fragment->x;
            
            /* Insert any need spaces at the beginning of the line
             * based on the the x offset of the line.
             */
            insert_spaces(line,x_fragment->x,bkr_default_space_width);

            /* Now loop through the the text packets and the words in
             * each packet setting up the data structures.
             */
            do {

                TEXT_FRAGMENT *next_x_fragment;
                BMD_WORD_PKT *word = x_fragment->words;
                int delta;
                int i;
                
                delta = x_fragment->x - end_of_last_fragment;

                x_fragment->line = line;
                x_fragment->first_char = line->n_bytes;
                x_fragment->width = 0;

                while (word < x_fragment->end_of_words) {

                    if (x_fragment->text300) {
                        item->delta = delta + SCALE_VALUE300(word->delta);
                    } else {
                        item->delta = delta + SCALE_VALUE400(word->delta);
                    }

                    /* See if we need to insert any spaces before
                     * inserting the charachters in the word.
                     */
                    if (item->delta > 0) {
                        insert_spaces(line,item->delta,x_fragment->font_data->min_space);
                        end_of_last_fragment += item->delta;
                    } else if (delta < 0) {
                        line->chars[line->n_bytes] = ' ';
                        line->char_widths[line->n_bytes] = 0;
                        line->n_bytes++;
                        item->delta = 0;
                    }
                    delta = 0;
                    
                    /* Finish setting up the XTextItem.
                     */
                    item->chars = (char *)&line->chars[line->n_bytes];
                    item->nchars = word->count;
                    item->font = x_fragment->font_data->font_struct->fid;
                    
                    /* Now put the characters and widths into their
                     * respective buffers.
                     */
                    if (bkrplus_g_charcell_display) 
                    {
                        XCharStruct text_extents;
                        int unused;
                    
                        for (i = 0; i < word->count; i++) 
                        {
                            if (word->chars[i] == '\255') 
                            {
                                /* This is a dash in most fonts, so convert it to
                                 * a '-' for character cell, otherwise it will should 
                                 * up as a backwards question mark.
                                 */
                                line->chars[line->n_bytes] = '-';
                            }
                            else 
                            {
                                /* Just pass all the other characters through.
                                 */
                                line->chars[line->n_bytes] = word->chars[i];
                            }
                            /* All characters have the same width on a character
                             * cell display
                             */
                            line->char_widths[line->n_bytes] = bkr_default_space_width;
                            line->n_bytes++;
                        }
                        /* Even though all characters have the same width on a CC terminal,
                         * we still have to deterimne the width of this word to determine
                         * interword spacing.
                         */
                        XTextExtents(x_fragment->font_data->font_struct,
                                     word->chars,
                                     word->count,
                                     &unused,&unused,&unused,
                                     &text_extents
                                     );
                        x_fragment->width += text_extents.rbearing - text_extents.lbearing;
                        end_of_last_fragment += text_extents.rbearing - text_extents.lbearing;
                    }
                    else 
                    {
                        for (i = 0; i < word->count; i++) 
                        {
                            int width;

                            line->chars[line->n_bytes] = word->chars[i];
                            
                            /* Use the real width of the character
                             */
                            width = XTextWidth(x_fragment->font_data->font_struct,
                                               &word->chars[i],1);
                            line->char_widths[line->n_bytes] = width;
                            line->n_bytes++;
                            x_fragment->width += width;
                            end_of_last_fragment += width;
                        }
                    }
                    word = (BMD_WORD_PKT *)&word->chars[word->count];

                    *font_num_ptr++ = x_fragment->font_num;

                    item_list->n_items++;
                    item++;
                }


                while (line->chars[x_fragment->first_char] == ' ') 
                {
                    x_fragment->first_char++;
                }
                x_fragment->end_char = line->n_bytes;

                next_x_fragment = x_fragment->next_x;
                x_fragment = next_x_fragment;

            } while (x_fragment);
            
            item_list++;
            
            line->width = end_of_last_fragment - line->x;
        }

        /* Each line, even blank lines are terminated with a newline
         * and a NULL character.  Neither contribut to the width of
         * the line, and only the newline adds to the number of characters.
         */
        line->chars[line->n_bytes] = '\n';
        line->char_widths[line->n_bytes] = 0;
        line->n_bytes++;
        line->chars[line->n_bytes] = '\000';
        line->char_widths[line->n_bytes] = 0;
        char_buffer_offset += line->n_bytes + 1;
        line = line->next;
        topic->n_lines++;
    }

    if (bkrplus_g_charcell_display) 
    {
        int total_height = 0;
        int max_bytes = 0;

        line = topic->text_lines;
        while (line) 
        {
            line->y = total_height;
            line->baseline = line->y + bkr_default_font->ascent;
            line->height = bkr_default_line_height;
            total_height += bkr_default_line_height;

            max_bytes = MAX(max_bytes,line->n_bytes);

            line = line->next;
        }

        topic->height = total_height ;
        topic->width = bkr_default_space_width * (max_bytes-1);
    }

} /* end create_text_lines */

