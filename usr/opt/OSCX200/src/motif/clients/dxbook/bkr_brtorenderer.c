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
#module BKR_BRTORENDERER "V03-0000"
#endif
#ifndef VMS
  */
#endif


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
**        graphics, on a topic basis.
**
**--
**/

#include <stdio.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "br_meta_data.h"
#include "br_typedefs.h"
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "br_globals.h"
#include "br_common_defs.h"
#include "bkr_cursor.h"
#include "br_printextract.h"
#include "bkr_brtoconvertps.h"
#include "bkr_font.h"
#include "bkr_topic_data.h"
#include "bkr_brtorenderer.h"

void
bkr_topic_render(Widget widget,
                 XtPointer window_ptr,
                 XmAnyCallbackStruct callback_data)

{
    BKR_WINDOW         *window = (BKR_WINDOW *)window_ptr;
    BKR_BOOK_CTX       *book = window->shell->book;
    BKR_TOPIC_DATA     *topic = window->u.topic.data;
    char               out_filename[500];
    char               *ptr;
    FILE               *file;
    BKR_TEXT_LINE      *line;
    char            font_family[80];
    char            font_style[80];
    char            font_weight[80];
    char            font_set_width_name[80];
    char            ps_font_string[80];
    long            point_size;
    unsigned long   char_set;
    BKR_FONT_DATA   *font;
    int             font_num;
    

    bkr_cursor_display_wait(TRUE);

#ifdef vms
    ptr = strrchr(book->filespec,']');
#else
    ptr = strrchr(book->filespec,'/');
#endif 
    if (ptr == NULL) {
        ptr = book->filespec;
    } 
    else 
    {
        ptr++;
    }
    strcpy(out_filename,ptr);
    ptr = strrchr(out_filename,'.');
    if (ptr == NULL) {
        ptr = &out_filename[strlen(out_filename)];
    }
    strcpy(ptr,".renderer_data");

    file = fopen(out_filename,"w");
    if (file == NULL) {
        bkr_error_modal("Can't open renderer data file",XtWindow(window->appl_shell_id));
        bkr_cursor_display_wait(FALSE);
        return ;
    }

    fprintf(file,"%d\n",(book->max_font_id + 1));
    
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
                font = FONT_NOT_FOUND;
            }
            else 
            {
                font = bkr_font_entry_init(book,font_num);
            }
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
        
        fprintf(file,"%s:%s:%d\n",font_family,font_style,point_size);
    }


    fprintf(file,"%d\n",topic->n_lines);
    fprintf(file,"%d\n",topic->width);
    fprintf(file,"%d\n",topic->height);
    fprintf(file,"%s\n",topic->title);

    line = topic->text_lines;
    while (line) 
    {
        BKR_TEXT_ITEM_LIST *item_list = line->item_lists;

        fprintf(file,"%d\n",line->baseline);
        fprintf(file,"%d\n",line->x);
        fprintf(file,"%d\n",line->y);
        fprintf(file,"%d\n",line->width);
        fprintf(file,"%d\n",line->height);
        fprintf(file,"%d\n",line->n_item_lists);

        while (item_list) 
        {
            int item_num;

            fprintf(file,"%d\n",item_list->x);
            fprintf(file,"%d\n",item_list->n_items);
            for (item_num = 0 ;item_num < item_list->n_items ; item_num++)
            {
                fprintf(file,"%d\n",item_list->u.items[item_num].delta);
                fprintf(file,"%d\n",item_list->font_nums[item_num]);
                fprintf(file,"%d\n",item_list->u.items[item_num].nchars);
                fprintf(file,"%*.*s\n",
                        item_list->u.items[item_num].nchars,
                        item_list->u.items[item_num].nchars,
                        item_list->u.items[item_num].chars);
            }
            
            item_list = item_list->next;
        }

        line = line->next;
    }

    fclose(file);

    bkr_cursor_display_wait(FALSE);
}


