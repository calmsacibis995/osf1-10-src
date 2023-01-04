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
#include "br_common_defs.h"  /* common BR #defines */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_topic_create.h" /* function prototypes for .c module */
#include "bkr_error.h"       /* Error reporting */
#include "bkr_fetch.h"       /* Fetch resource literals */
#include "bkr_menu_create.h" /* Menu creation */
#include "bkr_resource.h"    /* Resource routines */
#include "bkr_window.h"      /* function prototypes for .c module */
#ifdef MEMEX
#include "bmi_topic.h"
#endif
#include  <X11/StringDefs.h>
#include  <Xm/Protocols.h>
#include  <DXm/DECspecific.h>
#ifdef DECWORLD
#include "bkr_decworld.h"
#endif
#include <stdio.h>         

void bkr_decworld_reset();

XtIntervalId bkr_decworld_timer_id;

static Widget gLit_widg;

FILE *OUTFILE;
char *out_fname="irs.txt";

typedef struct fullfill {
    Widget gName_widg;
    Widget gdecworld_widget;
    BKR_WINDOW *window;
    char *name;
    };

struct fullfill ff[10];

#define DECCIE 499999
#define MAX_IRS 10

void bkr_decworld_check()
{
    int index;
/* make sure we don't leave anything hanging around */
    for(index=0;index < MAX_IRS; index++) {
	if(ff[index].gdecworld_widget) 
	    XtUnmanageChild(ff[index].gdecworld_widget);
	    
	}
}

void
bkr_decworld_widg_save(widg,tag,data)
Widget               widg;
int                  *tag;
XmAnyCallbackStruct  *data;
{
  int open_slot;

    bkr_decworld_reset_timer = TRUE;	

    switch (*tag) {
	case 1: 
	    gLit_widg = widg;
	    break;

	case 2:
	    for(open_slot = 0; open_slot < MAX_IRS; open_slot++) {
		if(ff[open_slot].window == NULL) {
		    ff[open_slot].gName_widg = widg;
		    break;
		    }
		}
	    break;
	    
        default:
	    break;

	}

}


void 
bkr_decworld_data_save(widg,tag,data)
Widget               widg;
int                  *tag;
XmPushButtonCallbackStruct  *data;
{

    BKR_WINDOW              *window = NULL;
    BMD_CHUNK		    *chunk;
    BMD_BOOK_ID		    book_id;
    char *string;
    char *symbol;
    int	 i, index;
    unsigned big_brother;
    long    tloc,rounded_time;
    static Boolean first_time = TRUE;
    Widget  drawing_window;
    Window  window_id;
    char filename[255];
    char  customer_is_fullfilled[255];
    
    bkr_decworld_reset_timer = TRUE;	

    if(first_time) {
	first_time = FALSE;
	time(&tloc);
	rounded_time = tloc/60*60;
	sprintf(filename,"%s",ctime(&rounded_time));

	OUTFILE = fopen(out_fname, "w");
	if(OUTFILE == NULL) {
	    /* need to notify somebody thatthings are amiss */
	    printf("FILE NOT OPENED\n");
	    }
	    
	}	     

    window = bkr_window_find( widg );
    if (window == NULL)
        return;

    drawing_window = window->widgets[W_MAIN_WINDOW];
    window_id = XtWindow(drawing_window);
    
    for(index=0;index < MAX_IRS; index++) {
	if(window == ff[index].window)
	    break;
	}

    if(index > MAX_IRS - 1)
	return; 
   
    switch (*tag) {
        case 1:
	    string = XmTextGetString(ff[index].gName_widg);
	    if (string && strlen(string)) { 
		ff[index].name = malloc(strlen(string)+ 1);
	        strcpy(ff[index].name, string);

                XtFree(string);
                }
	    else {
		if(string)
		    XtFree(string);
		return;
	    }
	    
	    /* need to clear input field */
	    XmTextReplace(ff[index].gName_widg,0,
			      (XmTextGetLastPosition(ff[index].gName_widg)),"");
	    XtUnmanageChild(ff[index].gdecworld_widget);

	    book_id = window->shell->book->book_id;

	    if ( window->u.topic.selected_chunk != NULL ) {
		chunk = window->u.topic.selected_chunk;
		symbol = bri_get_object_symbol(book_id, chunk->target);
		}
	    else {
		for( i = 0; i < window->u.topic.num_chunks; i++) {
		    chunk = &window->u.topic.chunk_list[i];
		    symbol = bri_get_object_symbol(book_id,chunk->id);
		    if( symbol != NULL)
			break;
		    }
		}
				

	    big_brother = (unsigned) atoi(ff[index].name);

	    if(big_brother > DECCIE) {
		if(symbol != NULL) {
		    if( (symbol[0] == 'z') || (symbol[0] == 'Z'))
			symbol = NULL;
		    }

		if(( i == window->u.topic.num_chunks) || (symbol == NULL)) {
		    sprintf( errmsg, "NO INFORMATION PACKETS AVAILABLE ");
		    bkr_error_modal( errmsg, window_id );
		    }
		else {
		    		
		    sprintf(customer_is_fullfilled,"%s\t%s\n",ff[index].name ,symbol);

		    fprintf(OUTFILE, customer_is_fullfilled);
		    printf("%s\t%s",ff[index].name,symbol);
		    sprintf(errmsg,"The information you have requested will be\nmailed to your DECWORLD registration address");
		    bkr_error_modal( errmsg, window_id );
		    }
		}
	    else {
		sprintf( errmsg, "INFORMATION PACKETS NOT AVAILABLE TO DIGITAL EMPLOYEES");
		bkr_error_modal( errmsg, window_id );
		}

	    free(ff[index].name);

	    ff[index].gName_widg = 0;
	    ff[index].gdecworld_widget = 0;
	    ff[index].name = NULL;
	    ff[index].window = NULL;
            break;

        case 2:

            /* need to clear input field */
            XmTextReplace(ff[index].gName_widg,0,
			      (XmTextGetLastPosition(ff[index].gName_widg)),"");
	
	    XtUnmanageChild(ff[index].gdecworld_widget);
            /* cancel everthing */
	    ff[index].gName_widg = 0;
	    ff[index].gdecworld_widget = 0;
	    ff[index].name = NULL;
	    ff[index].window = NULL;
            break;
	}

}

void
bkr_decworld_popup(widg,window,data)
Widget               widg;
BKR_WINDOW           *window;
XmPushButtonCallbackStruct  *data;
{
    Widget	decworld_widget;
    MrmType     dummy_class;
    unsigned    status;
    char        *topic_popup_menu_index = "bkr_decworld_dialog_box";
    int		open_space;

    bkr_decworld_reset_timer = TRUE;	

    status = MrmFetchWidget(
                    bkr_hierarchy_id,
                    topic_popup_menu_index,              /* index          */
                    window->widgets[W_MAIN_WINDOW],/* parent widget  */
                    &decworld_widget, /* widget fetched */
                    &dummy_class );                      /* unused class   */

    if ( status != MrmSUCCESS )
    {
        char    *error_string;
        error_string = (char *) bkr_fetch_literal( "FETCH_WIDGET_ERROR",ASCIZ );
        if ( error_string != NULL )
        {
            sprintf( errmsg, error_string, topic_popup_menu_index );
            bkr_error_modal( errmsg, NULL );
            XtFree( error_string );
        }
    }

    for(open_space = 0; open_space < MAX_IRS; open_space++) {
	if(ff[open_space].window == NULL) {
	    ff[open_space].gdecworld_widget = decworld_widget;
	    ff[open_space].window = window;
	    break;
	    }
	}
	
     if(open_space != MAX_IRS)
	XtManageChild(decworld_widget);

}

void bkr_decworld_reset( widget, id )
Widget	widget;
XtIntervalId	id;
{
    static int flag = 0;
    static Widget   window_check;
    BKR_WINDOW  *window;
    BKR_SHELL *parent;

    if(flag) {
	if(window_check != widget)
	    return;

	if(bkr_decworld_reset_timer == FALSE) {
	    window = bkr_window_find( widget );
	    if(!window) {
		bkr_decworld_start_timer = TRUE;
		flag = 0;
		return;
		}
	    parent = window->shell;

	    bkr_object_id_dispatch(parent,window,1);
	    }
	bkr_decworld_reset_timer = FALSE;	
        }    
    else {
	bkr_decworld_start_timer = FALSE;
	bkr_decworld_reset_timer = FALSE;
	window_check = widget;
	flag = 1;
	}

    bkr_decworld_timer_id =
	    XtAppAddTimeOut(bkr_app_context,60000,bkr_decworld_reset, widget);
}

