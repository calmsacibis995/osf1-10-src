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
static char *rcsid = "@(#)$RCSfile: bkr_api_util.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/24 15:32:14 $";
#endif
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_API_UTIL.C*/
/* *32   25-NOV-1992 16:53:05 GOSSELIN "conditionalizing between ALPHA/OSF and MIPS"*/
/* *31   25-NOV-1992 15:43:08 GOSSELIN "changing execvp to execv using absolute path for dxbook"*/
/* *30   22-JUN-1992 12:59:22 BALLENGER "Remove definition of pid_t."*/
/* *29   19-JUN-1992 13:32:36 BALLENGER "UCX$CONVERT"*/
/* *28   19-JUN-1992 13:30:51 BALLENGER "Clean up for Alpha/OSF port."*/
/* *27    8-JUN-1992 19:20:48 BALLENGER "UCX$CONVERT"*/
/* *26    8-JUN-1992 11:15:55 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *25    4-MAY-1992 10:41:33 GOSSELIN "centered WIP box"*/
/* *24    1-MAY-1992 11:45:54 GOSSELIN "fixed I14Y bug"*/
/* *23   29-APR-1992 16:36:19 GOSSELIN "updated atoms"*/
/* *22   24-APR-1992 08:57:39 GOSSELIN "changing default for LIB$FINDFILE"*/
/* *21   23-APR-1992 22:05:44 GOSSELIN "fixed VMS defaults"*/
/* *20   23-APR-1992 16:15:06 BALLENGER "BkrResolvePath cleanup"*/
/* *19   22-APR-1992 21:14:48 BALLENGER "Use XtResolvePathname and remove wait cursor support."*/
/* *18   17-APR-1992 13:23:39 GOSSELIN "fixed"*/
/* *17   16-APR-1992 18:22:59 BALLENGER "Fix syncronization & add work-in-progress dialog"*/
/* *16    9-APR-1992 11:19:00 ROSE "Default file handling added for Ultrix to resolve filename routine"*/
/* *15    1-APR-1992 13:40:32 GOSSELIN "fixed"*/
/* *14   19-MAR-1992 14:48:25 GOSSELIN "fixed context error"*/
/* *13    3-MAR-1992 16:56:08 KARDON "UCXed"*/
/* *12   12-DEC-1991 10:07:40 GOSSELIN "finsihed changing coldstart from mcr to run sys$system:"*/
/* *11   10-NOV-1991 21:14:46 BALLENGER "Make all atoms part of BkrServerInfo struct."*/
/* *10    8-NOV-1991 09:31:57 GOSSELIN "fixed includes and routine names"*/
/* *9     4-NOV-1991 15:45:54 GOSSELIN "fixed to make use of arglists"*/
/* *8    29-OCT-1991 10:01:00 GOSSELIN "changed property status test"*/
/* *7    15-OCT-1991 09:29:13 GOSSELIN "isolated files"*/
/* *6    14-OCT-1991 16:38:29 GOSSELIN "fixed coldstart problem"*/
/* *5    14-OCT-1991 12:09:33 BALLENGER "Fix synchronization problems."*/
/* *4    25-SEP-1991 20:13:10 BALLENGER "Fix more synchorinzation problems for DECquery FT."*/
/* *3    25-SEP-1991 18:08:02 BALLENGER "Fix bkr_api.h include problems"*/
/* *2    25-SEP-1991 16:24:44 BALLENGER "Fix problems with api requests."*/
/* *1    16-SEP-1991 12:38:39 PARMENTER "Client-Server Support"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_API_UTIL.C*/
/*
*****************************************************************************
**                                                                          *
**                     COPYRIGHT (c) 1990, 1991 BY                          *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
*/

/*
**
**  FACILITY:  Bkr API (Callable Bookreader) routines
**
**  DESCRIPTION:
**
**      This module contains utility routines for the Bkr API
**	(Callable Bookreader) routines 
**
**  FUNCTIONS:  various
**
**  AUTHOR:  Dan Gosselin, OSAG
**
**  CREATION DATE:  03-Nov-1991
**
**  MODIFICATION HISTORY:
**
*/


/* include files */

#ifdef vms
#include <ssdef.h>
#include <starlet.h>
#endif 
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <Xm/MessageB.h>
#include "br_common_defs.h"
#include "bkr_api.h"
#include "bkr_api_util.h"

/***************
 *   DEFINES   *
 ***************/
#define	BACKSLASH	'/'
#define PERIOD		'.'

/* local function PROTOTYPES */ 

static Boolean xerror_occurred;

static int BkrIgnoreXerror
    PROTOTYPE((Display *, XErrorEvent *));

static void get_node_name
    PROTOTYPE((char *, int));

static void check_cold_start
    PROTOTYPE((XtPointer , XtIntervalId *));

static char *state_names[] = {
    "BkrServerAbsent",
    "BkrServerLocked",
    "BkrServerChanged",
    "BkrServerUnchanged"
};



int BkrGetArg( args, argc, name )
ArgList	args;
int	argc;
String	name;
{
    int	i;
    
    for ( i = 0; i < argc;  i++ )
    {
    	if ( strcmp( args[i].name, name ) == 0 )
	    return( i );
    }
    return( -1 );
}


static int BkrIgnoreXerror( display, error_event )
Display	    *display;
XErrorEvent *error_event;
{
    xerror_occurred = TRUE;
    return Success;
}


void
BkrProcessMessages PARAM_NAMES((client_ctx))
    BkrClientStruct *client_ctx PARAM_END
/*
 *
 * Function description:
 *
 *      Routine to process any currently outstanding events for the client window.
 *      This routine is called by BkrDisplay and BkrClose to help ensure that any
 *      pending client messages events are processed before attempting to handle
 *      a new request for the client.  This should reduce the number of cases in
 *      which these routines need to return Bkr_Busy.
 *
 * Arguments:
 *
 *      client_ctx - the client context
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      Flushes the the event output buffers.
 *
 */
{
    XEvent event;

    /* Make sure the events are flushed.
     */
    XSync(client_ctx->server.display,FALSE);

    /* Now see if any events are pending for the client window and process
     * them.
     */
    while (XCheckWindowEvent(client_ctx->server.display,
                             client_ctx->client_window,
                             XtAllEvents,
                             &event
                             )
           ) 
    {
        XtDispatchEvent(&event);
    }
}

int BkrApiSendMessage( client_ctx, message_type )
BkrClientStruct	*client_ctx;
Atom		 message_type;
{
    XClientMessageEvent	event;
    Status		status;
    XErrorHandler	old_handler;
    int			return_status = Bkr_Success ;
	
    event.display      = client_ctx->server.display;
    event.window       = client_ctx->server.window;
    event.type	       = ClientMessage;
    event.format       = 32;
    event.message_type = message_type;
    event.data.l[0]    = ( long )client_ctx->client_window;
    
    event.data.l[1] = 0;  /* not currently used for anything */
    event.data.l[2] = 0;
    event.data.l[3] = 0;
    event.data.l[4] = 0;	    
    
    /* Set a dummy error handler to ignore errors if the windows don't exist */

    xerror_occurred = FALSE;
    old_handler = XSetErrorHandler(BkrIgnoreXerror);
    
    status = XSendEvent(client_ctx->server.display,
			client_ctx->server.window,
			TRUE, NoEventMask, (XEvent *)&event );

    if (status == Success) 
    {
        return_status = Bkr_Send_Event_Failure;
    }

    XSync(client_ctx->server.display,FALSE);

    /* Restore the old error handler */

    (void)XSetErrorHandler(old_handler);

    /* if bad Xerror, remove Processing_Request */

    if (xerror_occurred)
	client_ctx->request_status = Waiting_For_Request;

    return return_status;
}


void BkrApiClientMessageHandler 
    PARAM_NAMES((bkr_client, client_data, event, continue_to_dispatch))
    Widget    bkr_client PARAM_SEP
    XtPointer client_data PARAM_SEP
    XEvent  *event PARAM_SEP
    Boolean *continue_to_dispatch PARAM_END
{
    BkrClientStruct *client_ctx = (BkrClientStruct *)client_data;
    Window          server_window = (Window)event->xclient.data.l[0];
    pid_t           server_pid = (pid_t)event->xclient.data.l[1];


    if ( event->type != ClientMessage ) return;

    /* If the server_pid from the event is not 0 then it should match
     * the server.pid from the last time the client checked to see if the
     * server exists.
     */
     if ((server_pid != 0) && (client_ctx->server.pid != server_pid))
     {
         /* It doesn't match so we assume that the message is from a server
          * that has died after sending the message.  That means we just
          * ignore the message.  If we don't ignore the message and the client
          * is starting a Bookreader, this stale message will make it think
          * that the new Bookreader has completed startup when it hasn't.
          *
          * For the all other request_statuses, BkrExists has already noted
          * that Boorkeader has changed, and reset the status to
          * Waiting_For_request.
          */
         return;
     }
    
    /* Now make sure the message type is one we recognize and handle the
     * event.
     */
    if (( event->xclient.message_type == client_ctx->server.topic_success_atom )
	|| ( event->xclient.message_type == client_ctx->server.widget_success_atom )
        || ( event->xclient.message_type == client_ctx->server.directory_success_atom )
	|| ( event->xclient.message_type == client_ctx->server.book_success_atom )
        || ( event->xclient.message_type == client_ctx->server.close_success_atom ) 
        || ( event->xclient.message_type == client_ctx->server.server_failure_atom ) 
        )
    {
        /* Update the server window if the previous request was a successful
         * "display" of something.
         */
        if (( event->xclient.message_type != client_ctx->server.close_success_atom ) 
            && ( event->xclient.message_type != client_ctx->server.server_failure_atom ) 
            ) 
        {
            client_ctx->server.window  = server_window;
        }
        
        if (client_ctx->request_status == Startup_Request) 
        {
            /* Remove the work-in-progress dialog if it is active. */
             
            if (client_ctx->wip_active)
            {
                BkrRemoveWipDialog( client_ctx->wip_widget,
				    (caddr_t)client_ctx, NULL);
            }
            
            /* If we were in the process of starting Bookreader, make sure the
             * timeout procedure is removed.
             */
            if (client_ctx->coldstart_timer != NULL) 
            {
                XtRemoveTimeOut(client_ctx->coldstart_timer);
            }
        }
        
        /* The api is now just waiting for a request.
         */
        client_ctx->request_status = Waiting_For_Request;
    }
}

void BkrDeleteContext PARAM_NAMES((client_ctx))
    BkrClientStruct *client_ctx PARAM_END
{
    /* We're deleting the client context, so we need to remove the 
     * message handler.  
     */

    XtRemoveEventHandler( client_ctx->client_widget, NoEventMask, 
                          TRUE, BkrApiClientMessageHandler, 
                          client_ctx);


    /* If we were in the process of starting Bookreader, make sure the
     * timeout procedure is removed.
     */
    if (client_ctx->coldstart_timer != NULL) {
        XtRemoveTimeOut(client_ctx->coldstart_timer);
    }

    BKR_FREE( client_ctx );
}


static void get_node_name( name, length )
char *name;
int   length;
{
#ifdef VMS
    char     node[NODE_LENGTH];
    unsigned long status;
    int	     item_code = SYI$_NODENAME;
    struct   dsc$descriptor_s res_str;
    unsigned short node_len;
    int	     dummy[2];

    res_str.dsc$w_length  = sizeof(node);
    res_str.dsc$b_dtype   = DSC$K_DTYPE_T;
    res_str.dsc$b_class   = DSC$K_CLASS_S;
    res_str.dsc$a_pointer = node;

    status = lib$getsyi( &item_code, &dummy, &res_str, 
			 &node_len, NULL, NULL );
    node[node_len] = 0;

    strcpy( name, node );
#else
    gethostname( name, length );
#endif
}


void BkrCreateAtoms( server ) 
BkrServerInfo *server;
{
    char    node_name[NODE_LENGTH];
    char    atom_name[20+NODE_LENGTH];
    char    lock_name[25+NODE_LENGTH];

    get_node_name( node_name, NODE_LENGTH );

    strcpy( atom_name, "_DEC_BKR_SERVER_FOR_" );
    strcat( atom_name, node_name );
    strcpy( lock_name, "_DEC_BKR_SERVER_LOCK_FOR_");
    strcat( lock_name, node_name );

    server->node_atom =	             XInternAtom(server->display, 
				       atom_name, FALSE );
    server->lock_atom =	             XInternAtom(server->display, 
				       lock_name, FALSE );

    server->data_atom =	             XInternAtom(server->display, 
				       "_DEC_BKR_DATA", FALSE );
    server->data_type_atom =	     XInternAtom(server->display, 
				       "_DEC_BKR_DATA_TYPE", FALSE );

    server->topic_success_atom =     XInternAtom(server->display, 
				       "_DEC_BKR_TOPIC_SUCCESS", FALSE );
    server->directory_success_atom = XInternAtom(server->display, 
				       "_DEC_BKR_DIRECTORY_SUCCESS", FALSE );
    server->book_success_atom =	     XInternAtom(server->display, 
				       "_DEC_BKR_BOOK_SUCCESS", FALSE );
    server->widget_success_atom =    XInternAtom(server->display, 
				       "_DEC_BKR_WIDGET_SUCCESS", FALSE );
    server->close_success_atom =     XInternAtom(server->display, 
				       "_DEC_BKR_CLOSE_SUCCESS", FALSE );
    server->server_failure_atom =    XInternAtom(server->display, 
				       "_DEC_BKR_SERVER_FAILURE", FALSE );
    server->client_request_atom =    XInternAtom(server->display, 
				       "_DEC_BKR_CLIENT_REQUEST", FALSE );
}

Boolean process_exists PARAM_NAMES((pid))
    pid_t pid PARAM_END
{
#ifdef vms
    long iosb[2];
    long null_item_list = 0;
    long status;
    
    /* Call sys$getjpiw with a null item list. We're just interested in
     * seeing if the process exists.
     */

    status = sys$getjpiw(0,&pid,NULL,&null_item_list,&iosb,0,0);
    if (status != SS$_NORMAL) {
        return FALSE;
    } else if (iosb[0] != SS$_NORMAL) {
        return FALSE;
    } else {
        return TRUE;
    }
#else
    /* Call kill() with a signal number of 0, which just checks
     * to see if the specified process exists.
     */

    return (kill(pid,0) == 0);
#endif 
}

Boolean BkrLockServer( server )
BkrServerInfo	*server;
{
    unsigned char *data = NULL;
    Atom          type;
    int           format;
    unsigned long nitems, left;
    int           status;
    long          offset = 0;
    Boolean       got_lock = FALSE;
    Boolean       get_lock = TRUE;

    XChangeProperty(server->display,
                    server->root_window,
                    server->lock_atom,
                    XA_INTEGER,32,
                    PropModeAppend,
                    (unsigned char *)&server->lock_pid,
                    (sizeof(pid_t)/sizeof(BR_INT_32)));

    /* Did we get it? */

    while (get_lock) {

        status = XGetWindowProperty(server->display, 
                                    server->root_window,
                                    server->lock_atom, 
                                    offset++, 1L, 
                                    FALSE, XA_INTEGER, &type, 
                                    &format, &nitems, &left, &data );
        
        /* Did we get it? */
        
        if (status == Success)
        {
            if (type == XA_INTEGER) 
            {
                pid_t pid = *(pid_t *)data;
                if (server->lock_pid == pid)
                {
                    got_lock = TRUE;
                    get_lock = FALSE;
                }
                else if (process_exists(pid))
                {
                    get_lock = FALSE;
                }
                else if (left < sizeof(BR_INT_32))
                {
                    get_lock = FALSE;
                }
            }
            else if (type == None) 
            {
                /* The property was deleted while we were trying to
                 * get it. Create it and try again.
                 */
                XChangeProperty(server->display,
                                server->root_window,
                                server->lock_atom,XA_INTEGER,32,
                                PropModeAppend,
                                (unsigned char *)&server->lock_pid,
                                (sizeof(pid_t)/sizeof(BR_INT_32)));
            } 
            else 
            {
                get_lock = FALSE;
            }
        } 
        else 
        {
            get_lock = FALSE;
        }

        if (data) {
            XFree(data);
        }
    }

    return got_lock;
}


void BkrUnlockServer( server )
BkrServerInfo	*server;
{
    /* Delete the lock atom */

    XDeleteProperty( server->display, server->root_window, server->lock_atom );
    XSync(server->display,FALSE);

}


BkrServerState BkrExists( server )
BkrServerInfo *server;
{
    unsigned char   *data = NULL;
    Atom            type;
    int             format;
    unsigned long   nitems, left;
    int		    status;
    BkrServerState  server_state = BkrServerAbsent;
    XErrorHandler   old_handler;

    if ( BkrLockServer(server) == FALSE )
        return BkrServerLocked;
    
    /* Set a dummy error handler to ignore errors if the windows don't exist */

    xerror_occurred = FALSE;
    old_handler = XSetErrorHandler(BkrIgnoreXerror);
    
    /* Attempt to get the property sepcified by the atom from
       the root window. The property should be the window id
       of the Bookreader acting as a server  */

    status = XGetWindowProperty( server->display, 
                                 server->root_window,
                                 server->node_atom, 0L, 1L, FALSE, XA_WINDOW,
                                 &type, &format, &nitems, &left, &data );
    
    /* Did we get it? */

    if ( (status == Success) && (type == XA_WINDOW) )
    {
        /* Save the id and discard the data buffer */

        server->window = *(Window *)data;
        XFree(data);
        data = NULL;
        
        /* We got a window id, now make sure it is a Bookreader window
           acting as a server by checking to see if that window has the
           server node property, but containing the process id of the server */

        status = XGetWindowProperty( server->display, 
                                     server->window,
                                     server->node_atom, 
                                     0L, 1L, FALSE, XA_INTEGER,
                                     &type, &format, &nitems, &left,
                                     &data );

        if ( (status == Success) && (type == XA_INTEGER) )
        {
            if ( server->pid == *(int *)data ) {
                server_state = BkrServerUnchanged;
            }
            else 
            {
                server->pid  = *(int *)data;
                server_state = BkrServerChanged;
            }
        } 

    }
    
    if (data)
        XFree(data);  /* Free the data buffer */

    /* If the server is absent then we will keep the lock and attempt
     * a coldstart of Bookreader.  This will prevent other clients from
     * doing the same thing. The lock will be removed in one of two cases:
     *
     * o When Bookreader starts up and is ready to receive messages it will
     *   delete the lock.
     *
     * o If Bookreader doesn't start successfully, this client will delete
     *   the lock.
     */
    if ( server_state != BkrServerAbsent )
    {
        BkrUnlockServer(server);
    }

    /* Do an XSync here so we flush the event queue and catch/ignore 
       any errors (i.e the server window is not a valid window) */

    XSync( server->display, FALSE );
    
    /* Restore the old error handler */

    (void)XSetErrorHandler(old_handler);

    /* if bad Xerror, remove Processing_Request */

    if (xerror_occurred)
	server_state = BkrServerAbsent;

    /* Return the server state */
    return( server_state );
}


void BkrAppendData( buffer, data )
char **buffer;
char  *data;
{
    char *ptr = *buffer;

    strcpy( ptr, data );
    *buffer = &ptr[strlen(ptr)+1];
}

char *BkrResolveFilespec PARAM_NAMES((filename))
    char *filename PARAM_END
{
    char		*result_filename;
#ifdef VMS
    char		defaults[FILESPEC_SIZE];
    int			status;
    struct		dsc$descriptor_s d_input_filespec;
    struct		dsc$descriptor_s d_output_filespec;
    struct		dsc$descriptor_s d_default;
    unsigned long 	context = 0;
#else
    char		*work_filename, *c_ptr;
#endif

    result_filename = (char * ) XtMalloc( FILESPEC_SIZE+1 );
    memset( result_filename, 0 , FILESPEC_SIZE+1 );

#ifdef VMS
    strcpy( defaults, "SYS$HELP:*.DECW$BOOK" );

    d_input_filespec.dsc$w_length  = strlen(filename);
    d_input_filespec.dsc$b_dtype   = DSC$K_DTYPE_T;
    d_input_filespec.dsc$b_class   = DSC$K_CLASS_S;
    d_input_filespec.dsc$a_pointer = filename;

    d_output_filespec.dsc$w_length  = FILESPEC_SIZE;
    d_output_filespec.dsc$b_dtype   = DSC$K_DTYPE_T;
    d_output_filespec.dsc$b_class   = DSC$K_CLASS_S;
    d_output_filespec.dsc$a_pointer = result_filename;

    d_default.dsc$w_length  = strlen(defaults);
    d_default.dsc$b_dtype   = DSC$K_DTYPE_T;
    d_default.dsc$b_class   = DSC$K_CLASS_S;
    d_default.dsc$a_pointer = defaults;

    status = lib$find_file( &d_input_filespec, &d_output_filespec, 
			    &context, &d_default, NULL, NULL, NULL );

    status = lib$find_file_end( &context );

#else

    /* First, see if the file name is an environment variable */
    work_filename = (char *) getenv (filename);

    /* If not an environment variable, point the work file name at the file
       name and begin parsing */ 
    if (work_filename == NULL)
	work_filename = filename;

    /* Just return the work file name, we'll let Bookreader handle default
     * directories, etc.
     */
    strcat (result_filename, work_filename);

#endif

    return (result_filename);
}

static String suffix_list[] = { ".decw$book", ".decw_book", ".bkb"};

String
BkrResolvePath PARAM_NAMES((client_ctx,args,argc))
    BkrClientStruct *client_ctx PARAM_SEP 
    ArgList args PARAM_SEP
    int argc PARAM_END
{
    int                 index;
    String		filename = client_ctx->default_filename;
    String		type     = client_ctx->default_type;
    String              path     = client_ctx->default_path;
    String		suffix   = client_ctx->default_suffix;
    String              file_path = NULL;
    

    index =  BkrGetArg( args, argc, BkrNfilename );
    if ( index >= 0 )
    {
	filename = (char *)args[index].value;
    }

    index =  BkrGetArg( args, argc, BkrNfilePath );
    if ( index >= 0 ) 
    {
        path = (char *)args[index].value;
    }
	
    index =  BkrGetArg( args, argc, BkrNfileType );
    if ( index >= 0 )
    {
        type = (char *)args[index].value;
    }

    index =  BkrGetArg( args, argc, BkrNfileSuffix );
    if ( index >= 0 )
    {
        suffix = (char *)args[index].value;
    }


    if (suffix) 
    {
        file_path = XtResolvePathname(client_ctx->server.display,
                                      type,
                                      filename,
                                      suffix,
                                      path,
                                      NULL,
                                      0,
                                      NULL);
    }
    else 
    {
        index = XtNumber(suffix_list);
        while ((file_path == NULL) && (index > 0))
        {
            file_path = XtResolvePathname(client_ctx->server.display,
                                          type,
                                          filename,
                                          suffix_list[--index],
                                          path,
                                          NULL,
                                          0,
                                          NULL);
        }
    }

    if (file_path == NULL) {
        file_path = BkrResolveFilespec(filename);
    }

    return file_path;
}

static void check_cold_start PARAM_NAMES(( client_data, tid ))
    XtPointer    client_data PARAM_SEP
    XtIntervalId *tid PARAM_END
{
    BkrClientStruct *client_ctx = (BkrClientStruct *)client_data;
    int stat;

    /* Check to see if a server for this node exists */

    if (client_ctx->request_status == Startup_Request)
    {
        /* Nothing has identified itself as a server. See if the
           server process that was coldstarted is still running */

        if (process_exists(client_ctx->server.pid))
        {
            /* Set another time out to give it a chance to become the
             * server
             */
            client_ctx->coldstart_timer = XtAppAddTimeOut(client_ctx->app_context, 
                                                          1000L,
                                                          check_cold_start, 
                                                          client_ctx );
        }
        else 
        {
            /* The server died, delete the lock atom and let a subsequent
             * request attempt to start one.
             */
            BkrUnlockServer( &client_ctx->server );


            /* Remove the work-in-progress dialog if it is active. */
             
            if (client_ctx->wip_active)
            {
                BkrRemoveWipDialog( client_ctx->wip_widget,
				    (caddr_t)client_ctx, NULL);
            }

            /* Indicate that the timeout is no longer active.
             */
            client_ctx->coldstart_timer = NULL;

            /* Now just waiting for the next request.
             */
            client_ctx->request_status = Waiting_For_Request;

        }
    }
}

int BkrColdStart( client_ctx )
BkrClientStruct *client_ctx;
{
#ifdef VMS
    int	    status;
    int	    flags;
    struct  dsc$descriptor_s d_logical;
    struct  dsc$descriptor_s d_value;
    struct  dsc$descriptor_s d_command;
    char    value[16];
    char    logical[25];
    char    command[35];

    /* Put up the work-in-progress dialog */

    BkrCreateWipDialog(client_ctx);

    /* set logical name */

    sprintf( value, "%ld", (long)client_ctx->client_window );

    d_value.dsc$w_length  = strlen(value);
    d_value.dsc$b_dtype   = DSC$K_DTYPE_T;
    d_value.dsc$b_class   = DSC$K_CLASS_S;
    d_value.dsc$a_pointer = value;

    strcpy( logical, "BKR_CLIENT_INITIALIZE" );

    d_logical.dsc$w_length  = strlen(logical);
    d_logical.dsc$b_dtype   = DSC$K_DTYPE_T;
    d_logical.dsc$b_class   = DSC$K_CLASS_S;
    d_logical.dsc$a_pointer = logical;

    status = lib$set_logical( &d_logical, &d_value, 0, 0, 0 );


    /* do lib$spawn of Bookreader */

    strcpy( command, "run sys$system:decw$bookreader" );

    flags = CLI$M_NOWAIT;

    d_command.dsc$w_length  = strlen( command );
    d_command.dsc$b_dtype   = DSC$K_DTYPE_T;
    d_command.dsc$b_class   = DSC$K_CLASS_S;
    d_command.dsc$a_pointer = command;

    status = lib$spawn( &d_command, 0, 0, &flags, 0, 
	&client_ctx->server.pid, 0, 0, 0, 0, 0, 0 );

    if ( (status & 1) == 0 )
	return( Bkr_Startup_Failure );
    else
    {
        client_ctx->coldstart_timer = XtAppAddTimeOut(client_ctx->app_context, 
                                                      1000L,
                                                      check_cold_start, 
                                                      client_ctx );
	return( Bkr_Success );
    }
#else
    char     env[40];
    char    *argv[2];

    /* Put up the work-in-progress dialog */

    BkrCreateWipDialog(client_ctx);

    /* set environment variable */

    sprintf( env, "BKR_CLIENT_INITIALIZE=%ld", 
        ( long )client_ctx->client_window );

    putenv( env );

    /* do fork and exec */

#ifdef ALPHA
    argv[0] = "usr/bin/X11/dxbook";
#else
    argv[0] = "usr/bin/dxbook";
#endif
    argv[1] = NULL;

    client_ctx->server.pid = vfork();
    if (client_ctx->server.pid == 0) 
    {
        if ( setpgrp(0,getpid()) )
        {
            (void)signal( SIGHUP,SIG_IGN );
            (void)signal( SIGTERM,SIG_IGN );
        }
        execv( argv[0], argv );
        exit( EXIT_FAILURE );
    }
    else
    {
        client_ctx->coldstart_timer = XtAppAddTimeOut(client_ctx->app_context, 
                                                      1000L,
                                                      check_cold_start, 
                                                      client_ctx );
	return( Bkr_Success );
    }
#endif
}
void BkrRemoveWipDialog PARAM_NAMES((widget,user_data,callback_data))
    Widget widget PARAM_SEP
    caddr_t user_data PARAM_SEP
    XmAnyCallbackStruct *callback_data PARAM_END
{
    BkrClientStruct *client = (BkrClientStruct *)user_data;

    if (client->wip_active) {

        if (XtIsManaged(widget))
        {
            XtUnmanageChild(widget);
        }
        XtUnmapWidget(widget);
        XWithdrawWindow(client->server.display,
                        XtWindow(widget),
                        XDefaultScreen(client->server.display)
                        );
        client->wip_active = FALSE;
    }
}

void BkrCreateWipDialog PARAM_NAMES((client))
    BkrClientStruct *client PARAM_END
{
    Arg		arglist[5];
    int		ac,scrwidth,scrheight;
    Position	dix, diy;

    static char	*uid_file[] = { BKR_API_UID_FILE };

    /*
    ** Fetch the Box.
    */
    if (client->wip_widget == NULL) {

        MrmHierarchy hierarchy;
        Widget shell;
        XtCallbackRec ok_callback[2];
        MrmType *dummy_class;
        int status;

        /*
         **  Open the Mrm hierarchy
         */
        status = MrmOpenHierarchy(1,uid_file,NULL,&hierarchy);
        if (status != MrmSUCCESS) {
            return;
        }

        ac = 0;
        XtSetArg(arglist[ac], XmNwidth, 5); ac++;
        XtSetArg(arglist[ac], XmNheight, 5); ac++;

	scrwidth = XDisplayWidth(client->server.display, 
				 XDefaultScreen(client->server.display));
	scrheight= XDisplayHeight(client->server.display, 
				  XDefaultScreen(client->server.display));

	dix = ((scrwidth/2) - (2)); /* subtract half of XmNwidth, XmNheight */
	diy = ((scrheight/2) - (2));

        XtSetArg(arglist[ac], XmNx, dix); ac++;
        XtSetArg(arglist[ac], XmNy, diy); ac++;
        
        shell = XtAppCreateShell("WIPBox", "WIPBox", 
                                 topLevelShellWidgetClass,
                                 client->server.display, 
                                 (Arg *) arglist, (int) ac);
        
        ok_callback[0].callback = (XtCallbackProc) BkrRemoveWipDialog;
        ok_callback[0].closure = (XtPointer) client;
        ok_callback[1].callback = NULL;
        
        ac = 0;
        XtSetArg(arglist[ac], XmNokCallback, ok_callback); ac++;
        
        status = MrmFetchWidgetOverride(hierarchy, 
                                        "bkr_api_wip_window", 
                                        shell,
                                        NULL,
                                        arglist, (int) ac, 
                                        &client->wip_widget,
                                        &dummy_class
                                        );
        if (status != MrmSUCCESS) {
            (void)MrmCloseHierarchy(hierarchy);
            return;
        }
        /*  Close the hierarchy.
         */

        (void)MrmCloseHierarchy(hierarchy);

        /*
         ** Remove the Cancel and Help buttons
         */
        
        XtUnmanageChild(XmMessageBoxGetChild(client->wip_widget, XmDIALOG_CANCEL_BUTTON));
        XtUnmanageChild(XmMessageBoxGetChild(client->wip_widget, XmDIALOG_HELP_BUTTON));

        XtSetMappedWhenManaged(shell, FALSE);
        XtRealizeWidget(shell);
        XtRealizeWidget(client->wip_widget);

    }
    client->wip_active = TRUE;

    XtManageChild(client->wip_widget);
    XMapRaised(client->server.display,XtWindow(client->wip_widget));

    return;
}
