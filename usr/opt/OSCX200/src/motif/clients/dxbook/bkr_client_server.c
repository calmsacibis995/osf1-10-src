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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_CLIENT_SERVER.C*/
/* *19   19-JUN-1992 20:19:08 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *18    8-JUN-1992 19:15:27 BALLENGER "UCX$CONVERT"*/
/* *17    8-JUN-1992 11:32:19 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *16    1-MAY-1992 11:45:15 GOSSELIN "fixed I14Y bug"*/
/* *15   24-APR-1992 16:38:42 BALLENGER "Support window positioning through the API."*/
/* *14   16-APR-1992 18:21:38 BALLENGER "Fix client/server synchronization"*/
/* *13    1-APR-1992 13:40:11 GOSSELIN "fixed"*/
/* *12    3-MAR-1992 16:57:14 KARDON "UCXed"*/
/* *11   13-FEB-1992 18:32:12 BALLENGER "Fix problems with error messages for API."*/
/* *10   10-NOV-1991 21:16:47 BALLENGER "Make all atoms part of BkrServerInfo struct."*/
/* *9     8-NOV-1991 11:26:09 GOSSELIN "fixed 2 calls"*/
/* *8     8-NOV-1991 09:46:46 GOSSELIN "changed Bkr_Status to int"*/
/* *7     1-NOV-1991 13:06:33 BALLENGER "Reintegrate  memex support"*/
/* *6    14-OCT-1991 12:10:42 BALLENGER " Fix synchronization problems."*/
/* *5    25-SEP-1991 20:13:18 BALLENGER "Fix more synchorinzation problems for DECquery FT."*/
/* *4    25-SEP-1991 18:08:06 BALLENGER "Fix bkr_api.h include problems"*/
/* *3    25-SEP-1991 16:24:49 BALLENGER "Fix problems with api requests."*/
/* *2    17-SEP-1991 19:28:28 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:38:51 PARMENTER "Client-Server Support"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_CLIENT_SERVER.C*/
#ifndef VMS
 /*
#else
# module BKR_CLIENT_SERVER "V03-0000"
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
**	bkr_client_server.c
**
**  ABSTRACT:
**
**	Routines for dealing with the client/server access through the
**      Bookreader API.
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:    1-July-1991
**
**  MODIFICATION HISTORY:
**
**--
**/


/*
 * INCLUDE FILES
 */
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <Xm/Label.h>
#include <string.h>
#include "br_common_defs.h"
#include "br_meta_data.h"       /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"        /* BR high-level typedefs and #defines */
#include "br_globals.h"         /* BR external variables declared here */
#include "br_malloc.h"          /* BKR_MALLOC, etc defined here */
#include "bkr_api.h"            /* api (help) typedefs and #defines */
#include "bkr_api_util.h"       /* function prototypes for .c module */
#include "bkr_client_server.h"  /* function prototypes for .c module */
#include "bkr_book.h"           /* Book access routines */
#include "bkr_close.h"          /* Close routines */
#include "bkr_object.h"         /* Object Id dispatching routines */
#include "bkr_selection_open.h" /* Selection window open routines */
#include "bkr_shell.h"          /* Shell handling routines */
#include "bkr_window.h"         /* Window handling routines */

static BkrServerStruct *server_ctx;
static Window cold_start_client = NULL;

static int BkrIgnoreXerror
    PROTOTYPE((Display *, XErrorEvent *));

static void server_message_handler
    PROTOTYPE((Widget, 
               XtPointer, 
               XEvent *,
               Boolean * ));

static void get_bkrdata
    PROTOTYPE((BkrServerStruct *,
               Window));

static BKR_CLIENT *get_client
    PROTOTYPE((Window client_id,
               Boolean create_client));

/* 
 * Routine to do server specific cleanup when Bookeader exits. It is
 * as an exit handler, and is only set up as an exit handler if this
 * bookreader is acting as the server.
 */
static void
server_exit_cleanup()
{
    if (server_ctx) {

        /* Delete the server node property.
         */
        XDeleteProperty(server_ctx->server.display,
                        server_ctx->server.root_window,
                        server_ctx->server.node_atom);
    }
}

/*
 * Routine to attempt to initialize Bookreader as a server.  It will
 * only do so if no other Bookreader is the server for this node.
 */
void
bkr_server_initialize()
{
    BkrServerState   server_state;
    unsigned long val;

    server_ctx = ( BkrServerStruct * ) BKR_MALLOC( sizeof( BkrServerStruct ));
    memset( server_ctx, 0, sizeof( BkrServerStruct ));

    /* create unmanaged label widget
     */
    server_ctx->server_widget = XtCreateWidget("Bkr Server", 
                                               xmLabelWidgetClass, 
                                               bkr_library->appl_shell_id, 
                                               NULL, 0 );

    XtRealizeWidget(server_ctx->server_widget);

    server_ctx->window = XtWindow(server_ctx->server_widget);
    server_ctx->server.window = server_ctx->window;
    server_ctx->server.display = XtDisplay(server_ctx->server_widget);
    server_ctx->server.root_window = DefaultRootWindow(server_ctx->server.display);
        
    BkrCreateAtoms( &server_ctx->server );

    server_ctx->server.lock_pid = getppid();
    server_ctx->server.pid = getpid();

    /* check to see if bkr_server_node already exists as a property 
     * on the root window 
     */
    server_state = BkrExists(&server_ctx->server);

    if ((server_state == BkrServerAbsent) || (cold_start_client))
    {

        /* Setup the server cleanup routine to be called when the program
         * exits.
         */
        (void)atexit(server_exit_cleanup);

        /* Set up the event handler to receive events from clients
         */
        XtAddEventHandler( server_ctx->server_widget, NoEventMask,
                          TRUE, server_message_handler,
                          server_ctx);

	/* If NULL, post bkr_server_node as a property to the root window 
         */
	val = server_ctx->server.pid;
	XChangeProperty(server_ctx->server.display,
                        server_ctx->window,
                        server_ctx->server.node_atom,
                        XA_INTEGER,
                        32, PropModeReplace,
                        (unsigned char *)&val, 
                        (sizeof(pid_t) / sizeof(BR_INT_32)) );

	/* If NULL, post bkr_server_node as a property to the root window
         */
	XChangeProperty(server_ctx->server.display,
                        server_ctx->server.root_window,
                        server_ctx->server.node_atom,
                        XA_WINDOW,
                        32, PropModeReplace,
                        (unsigned char *)&server_ctx->window, 
                        (sizeof(Window) / sizeof(BR_INT_32)) );

        XSync(server_ctx->server.display,FALSE);

        /* Delete the lock atom.
         */
        if (server_state == BkrServerAbsent) {
            BkrUnlockServer(&server_ctx->server);
        }

        if (cold_start_client) {
            get_bkrdata(server_ctx,cold_start_client);
        }
    }
    else 
    {
        BKR_FREE(server_ctx);
    }        
}

Boolean
bkr_server_coldstart()
{
    char *trans_name;
    char *logical_name = "BKR_CLIENT_INITIALIZE";

    /* check to see if logical/env. var. BKR_CLIENT_INITIALIZE is set
     */
    
    trans_name = bri_logical(logical_name);

    /* if so, then call translate back to Window ID and get bkrdata */
    
    if ( strcmp( trans_name, logical_name )) {
        
        cold_start_client = ( Window )(atol(trans_name));
        
        return TRUE;
    }
    
    return FALSE;
}

static void server_message_handler 
    PARAM_NAMES(( bkr_server, server_data, xevent, continue_to_dispatch ))
    Widget     bkr_server PARAM_SEP
    XtPointer  server_data PARAM_SEP
    XEvent     *xevent PARAM_SEP
    Boolean    *continue_to_dispatch PARAM_END
{
    XClientMessageEvent  *event = (XClientMessageEvent  *)xevent;
    BkrServerStruct *server_ctx = (BkrServerStruct *)server_data;

    if ( event->type != ClientMessage ) return;
    if ( event->message_type == server_ctx->server.client_request_atom )
    {
	get_bkrdata(server_ctx,(Window)event->data.l[0]);
    }
    else if (event->message_type == server_ctx->server.client_exit_atom)
    {
        /* Handle exit/quit request
         */
        close_client((Window)event->data.l[0],NULL);
    }
}



static BKR_CLIENT *
get_client PARAM_NAMES((client_window,create_client))
    Window client_window PARAM_SEP
    Boolean create_client PARAM_END

{
    BKR_CLIENT *client;

    client = bkr_client_context;
    while (client) {
        if (client->id == client_window) {
            return client;
        }
        client = client->next;
    }

    if (create_client) {
        client = (BKR_CLIENT *)BKR_MALLOC(sizeof(BKR_CLIENT));
        if (client) {
            client->next = bkr_client_context;
            bkr_client_context = client;
            client->id = client_window;
            client->shells = NULL;
        }
    }
    return client;
}

BKR_WINDOW *
display_client_topic(client_window,filespec,topic_symbol,open_in_default)
    Window client_window;
    char *filespec;
    char *topic_symbol;
    Boolean open_in_default;
{
    BKR_CLIENT *client;
    
    client = get_client(client_window,TRUE);
    if (client) 
    {
        BKR_BOOK_CTX *book;

        book = bkr_book_get(filespec,NULL,NULL);
        if (book) 
        {
            BKR_SHELL *shell = bkr_shell_get(book,client,FALSE);

            if (shell)
            {
                BMD_OBJECT_ID chunk_id; 
                BKR_WINDOW *window = NULL;
                
                if (open_in_default) {
                    window = shell->default_topic;
                }

                chunk_id = bri_symbol_to_chunk(book->book_id,topic_symbol);
                if (chunk_id) {
                    return bkr_object_id_dispatch(shell,window,chunk_id);
                } else {
                    char *errmsg = (char*)bkr_fetch_literal("TOPIC_NOT_FOUND", ASCIZ );
                    if (errmsg) {
                        char buffer[512];
                        sprintf(buffer,errmsg,topic_symbol,book->title,book->filename);
                        bkr_error_modal(buffer,NULL);
                        XtFree(errmsg);
                    }
                }
            }
        }
    }
    return NULL;
}

BKR_WINDOW *
display_client_directory(client_window,filespec,directory_name)
    Window client_window;
    char *filespec;
    char *directory_name;
{
    BKR_CLIENT *client;
    
    client = get_client(client_window,TRUE);
    if (client) 
    {
        BKR_BOOK_CTX *book;

        book = bkr_book_get(filespec,NULL,NULL);
        if (book) 
        {
            BKR_SHELL *shell = bkr_shell_get(book,client,FALSE);

            if (shell)
            {
                BMD_OBJECT_ID dir_id;
                dir_id = bri_directory_find(book->book_id,directory_name);
                if (dir_id) {
                    return bkr_object_id_dispatch(shell,NULL,dir_id);
                } else {
                    char *errmsg = (char*)bkr_fetch_literal("DIRECTORY_NOT_FOUND", ASCIZ );
                    if (errmsg) {
                        char buffer[512];
                        sprintf(buffer,errmsg,directory_name,book->title,book->filename);
                        bkr_error_modal(buffer,NULL);
                        XtFree(errmsg);
                    }
                }
            }
        }
    }
    return NULL;
}

BKR_WINDOW *
display_client_widget(client_window,filespec,widget_name)
    Window client_window;
    char *filespec;
    char *widget_name;
{
    BKR_CLIENT *client;
    
    client = get_client(client_window,TRUE);
    if (client) 
    {
        BKR_BOOK_CTX *book;

        book = bkr_book_get(filespec,NULL,NULL);
        if (book) 
        {
            BKR_SHELL *shell = bkr_shell_get(book,client,FALSE);

            if (shell)
            {
                
            }
        }
    }
    return NULL;
}

BKR_WINDOW *
display_client_book(client_window,filespec,open_in_default)
    Window client_window;
    char *filespec;
    Boolean open_in_default;
{
    BKR_CLIENT *client;
    BKR_WINDOW *default_window = NULL;
    
    client = get_client(client_window,TRUE);
    if (client) 
    {
        BKR_BOOK_CTX *book;

        book = bkr_book_get(filespec,NULL,NULL);
        if (book) 
        {
            BKR_SHELL *shell = bkr_shell_get(book,client,FALSE);

            if (shell)
            {
                BMD_OBJECT_ID object = book->default_directory->object_id;
                if (object == (BMD_OBJECT_ID)0) 
                {
                    object = book->first_page;
                    if (open_in_default)
                    {
                        default_window = shell->default_topic;
                    }
                }
                return bkr_object_id_dispatch(shell,default_window,object);
            }
        }
    }
    return NULL;
}

static int BkrIgnoreXerror( display, error_event )
Display	    *display;
XErrorEvent *error_event;
{
    return Success;  
}

static void 
get_bkrdata PARAM_NAMES((server_ctx,client_window))
    BkrServerStruct *server_ctx PARAM_SEP
    Window client_window PARAM_END

{
    Widget        bkr_server;
    Atom          type;
    int           format;
    unsigned long nitems, left;
    unsigned char *property_data = NULL;
    unsigned char *bkr_data;
    unsigned char request_type;
    unsigned char *filespec;
    unsigned char *entry;
    Position	  x_position;
    Position	  y_position;
    Boolean       has_x = FALSE;
    Boolean       has_y = FALSE;
    int		  i;
    Boolean       use_default_window;
    Status	  status;
    int		  bkr_status;
    XClientMessageEvent event;
    XErrorHandler old_handler;
    BKR_WINDOW   *opened_window = NULL;
    
    if (client_window == NULL) 
    {
        return;
    }

    /* Assuming successful status for now
     */
    bkr_status = Bkr_Success;
    
    /* Set a dummy error handler to ignore errors if the windows
     * don't exist.
     */
    
    old_handler = XSetErrorHandler(BkrIgnoreXerror);
    
    /* get Bkr data 
     */
    if (XGetWindowProperty(server_ctx->server.display,
                           client_window,
                           server_ctx->server.data_atom, 
                           0L,(long)BKR_DATA_SIZE, 
                           TRUE, 
                           server_ctx->server.data_type_atom,
                           &type, &format, &nitems, &left,
                           &property_data ) == Success 
        && type == server_ctx->server.data_type_atom ) 
    {
        
        bkr_data = property_data;

	/* decode Bkr data 
         */
	request_type =  *bkr_data;
	bkr_data = &bkr_data[strlen( bkr_data ) + 1];
	
	filespec = bkr_data;
	bkr_data = &bkr_data[strlen( bkr_data ) + 1];
        
        use_default_window = (*bkr_data == 'D') ? TRUE : FALSE ;
	bkr_data = &bkr_data[strlen( bkr_data ) + 1];
        
	entry = bkr_data;
	bkr_data = &bkr_data[strlen( bkr_data ) + 1];
        
        has_x = (strlen(bkr_data) > 0);
        if (has_x) {
            x_position = atol( bkr_data );
        }
	bkr_data = &bkr_data[strlen( bkr_data ) + 1];
        
        has_y = (strlen(bkr_data) > 0);
        if (has_y) {
            y_position = atol( bkr_data );
        }
        
	/* call Bkr routines to display decoded information and return 
	 * success status if all goes well.  We all so set the window
         * to be the parent for error messages to be the root window
         * to make sure they are visible.
         */
        
        bkr_error_set_parent_shell(NULL);

        switch (request_type) {
            case 'T': {
                /* Handle display topic request
                 */
                opened_window = display_client_topic(client_window,
                                                     filespec,entry,
                                                     use_default_window);
                break;
            }
            case 'D': {
                /* Handle display data reuest
                 */
                opened_window = display_client_directory(client_window,filespec,entry);
                break;
            }
            case 'B': {
                /* Handle display book request
                 */
                opened_window = display_client_book(client_window,filespec);
                break;
            }
            case 'W': {
                /* Handle display widget request
                 */
                opened_window = display_client_widget(client_window,filespec,entry);
                break;
            }
            case 'C': {
                /* Handle close request
                 */
                close_client(client_window,filespec);
                break;
            }
        }
        
        if (opened_window && (has_x || has_y)) {
            Arg           arglist[10];
            int           argcnt = 0;
            int           diff;
            Dimension     height, width;
        

            if (has_x == FALSE) {
                SET_ARG(XtNx,&x_position);
            }
            if (has_y == FALSE) {
                SET_ARG(XtNy,&y_position);
            }
            SET_ARG(XtNheight,&height);
            SET_ARG(XtNwidth,&width);
            XtGetValues( opened_window->appl_shell_id, arglist, argcnt );

            /* Make sure the window fits horizontally 
             */
            diff = (int) height + (int) y_position - bkr_display_height; 
            if ( diff > 0 )
            {
                if ( diff <= (int) y_position )    /* Height ok, adjust Y position */
                {
                    y_position = (Position)( (int) y_position - diff );
                }
                else	    /* Window too tall, adjust both */
                {
                    y_position = 0;
                    height =(Dimension)( (int) height - (diff - (int) y_position) );
                }
            }
            
            /* Make sure the window fits horizontally 
             */
            diff = (int) width + (int) x_position - bkr_display_width;   /* portion off side */
            if ( diff > 0 )
            {
                if ( diff <= (int) x_position )    /* Width ok, adjust X position */
                {
                    x_position = (Position)( (int) x_position - diff );
                }
                else	    /* Window too wide, adjust both */
                {
                    x_position = 0;
                    width = (Dimension)( (int) width - (diff - (int) x_position) );
                }
            }

            /* Set the new positon
             */
            argcnt = 0;
            SET_ARG(XtNx,x_position);
            SET_ARG(XtNy,y_position);
            SET_ARG(XtNheight,height);
            SET_ARG(XtNwidth,width);
            XtSetValues( opened_window->appl_shell_id, arglist, argcnt );
        }

	/* set values for message to send back to client */
        
	event.display = server_ctx->server.display;
	event.window  = client_window;
	event.type    = ClientMessage;
	event.format  = 32;
	event.data.l[0] = ( long )server_ctx->window;
        
	/* depending on request type, send client message back 
           indicating success */
        
	if ( bkr_status == Bkr_Success ) 
        {
            switch (request_type) {
                case 'T': {
                    event.message_type = server_ctx->server.topic_success_atom;
                    break;
                }
                case 'D': {
                    event.message_type = server_ctx->server.directory_success_atom;
                    break;
                }
                case 'B': {
                    event.message_type = server_ctx->server.book_success_atom;
                    break;
                }
                case 'W': {
                    event.message_type = server_ctx->server.widget_success_atom;
                    break;
                }
                case 'C': {
                    event.message_type = server_ctx->server.close_success_atom;
                    break;
                }
            }
	} 
        else 
        {
            event.message_type = server_ctx->server.server_failure_atom;
        }
	
	event.data.l[1] = (long)server_ctx->server.pid;
	event.data.l[2] = 0;        /* not currently used for anything */
	event.data.l[3] = 0;
	event.data.l[4] = 0;

	status = XSendEvent(server_ctx->server.display,
                            client_window,
                            TRUE, NoEventMask, 
                            (XEvent *)&event );
        
	XSync(server_ctx->server.display,FALSE);
    }
    if (property_data)
    {
        XFree(property_data);
    }
    
    /* Restore the old error handler
     */
    (void)XSetErrorHandler(old_handler);

}
close_client(client_window,filespec)
    Window client_window;
    char *filespec;
{
    BKR_CLIENT *client;



    client = get_client(client_window,FALSE);
    if (client) 
    {
        Boolean delete_all = FALSE;
        BKR_SHELL *shell = client->shells;

        if ((filespec == NULL) || (filespec[0] == (char)0) ) 
        {
            delete_all = TRUE;
        }

        while (shell) 
        {
            BKR_SHELL *next_shell = shell->client_shells;
            if (delete_all 
                || (strcmp(shell->book->filename,filespec) == 0)
                )
            {
                bkr_close_shell(shell,TRUE);
            }
            shell = next_shell;
        }
        if (client->shells == NULL) 
        {
            BKR_CLIENT_PTR *ptr_to_ptr;
            
            ptr_to_ptr = &bkr_client_context;
            while (*ptr_to_ptr) 
            {
                BKR_CLIENT     *ptr = *ptr_to_ptr;

                if (ptr == client) 
                {
                    *ptr_to_ptr = ptr->next;
                    BKR_FREE(client);
                    break;
                }
                ptr_to_ptr = &ptr->next;
            }
        }
    }
}
