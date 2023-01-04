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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_API.C*/
/* *15   23-APR-1992 21:04:27 GOSSELIN "conditionalized default type"*/
/* *14   22-APR-1992 21:17:36 BALLENGER "Remove unnecessary header includes"*/
/* *13   22-APR-1992 21:14:29 BALLENGER "Use XtResolvePathname and remove wait cursor support."*/
/* *12   16-APR-1992 18:22:42 BALLENGER "Fix syncronization & add work-in-progress dialog"*/
/* *11    1-APR-1992 13:40:46 GOSSELIN "fixed"*/
/* *10    3-MAR-1992 16:55:48 KARDON "UCXed"*/
/* *9    10-NOV-1991 21:14:42 BALLENGER "Make all atoms part of BkrServerInfo struct."*/
/* *8     8-NOV-1991 09:31:50 GOSSELIN "fixed includes and routine names"*/
/* *7     4-NOV-1991 15:45:42 GOSSELIN "fixed to make use of arglists"*/
/* *6    15-OCT-1991 09:28:53 GOSSELIN "isolated files"*/
/* *5    14-OCT-1991 12:09:25 BALLENGER "Fix synchronization problems."*/
/* *4    25-SEP-1991 20:13:02 BALLENGER "Fix more synchorinzation problems for DECquery FT."*/
/* *3    25-SEP-1991 18:07:54 BALLENGER "Fix bkr_api.h include problems"*/
/* *2    25-SEP-1991 16:24:41 BALLENGER "Fix problems with api requests."*/
/* *1    16-SEP-1991 12:32:51 PARMENTER "Client-Server Support"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_API.C*/
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
**      This module contains routines which enable applications
**	to act as Bookreader clients. 
**
**  FUNCTIONS:
**
**      BkrOpen
**      BkrDisplay
**      BkrClose
**
**
**  AUTHOR:  Dan Gosselin, OSAG
**
**  CREATION DATE:  03-Nov-1991
**
**  MODIFICATION HISTORY:
**
*/


/* include files */
#include "bkr_api.h"
#include "bkr_api_util.h"



int BkrOpen( bkr_client_context, parent, args, argc )
Opaque	*bkr_client_context;
Widget	 parent;
ArgList	 args;
int	 argc;
{
    BkrClientStruct	*client_ctx;
    int			 index;
    
    client_ctx = ( BkrClientStruct * )BKR_MALLOC( sizeof(BkrClientStruct ));
    if (client_ctx == NULL) {
        *((BkrClientStruct **)bkr_client_context) = NULL;
	return( Bkr_Create_Client_Failure );
    }
    memset( client_ctx, 0, sizeof( BkrClientStruct ));
    
    index =  BkrGetArg( args, argc, BkrNfilename );
    if ( index >= 0 )
    {
	client_ctx->default_filename = (char *)args[index].value;
    }
    else
    {
        BKR_FREE(client_ctx);
        *((BkrClientStruct **)bkr_client_context) = NULL;
        return( Bkr_Bad_Filename );
    }

    index =  BkrGetArg( args, argc, BkrNfilePath );
    if ( index >= 0 ) 
    {
        client_ctx->default_path = (char *)args[index].value;
    }
	
    index =  BkrGetArg( args, argc, BkrNfileType );
    if ( index >= 0 )
    {
        client_ctx->default_type = (char *)args[index].value;
    }
    else 
    {
#ifdef VMS
        client_ctx->default_type = "sys$help:";
#else
        client_ctx->default_type = "help";
#endif
    }

    index =  BkrGetArg( args, argc, BkrNfileSuffix );
    if ( index >= 0 )
    {
        client_ctx->default_suffix = (char *)args[index].value;
    }

    /* implied from the memset above: client_ctx->display_status = FALSE */

    client_ctx->request_status  = Waiting_For_Request;
    client_ctx->server.lock_pid = getpid();
    client_ctx->app_context     = XtWidgetToApplicationContext(parent);
    client_ctx->client_widget   = XtCreateWidget( "Bkr Client",
                                    xmLabelWidgetClass, parent, NULL, 0 );
    
    if ( client_ctx->client_widget )
    {
        XtRealizeWidget(client_ctx->client_widget);

        client_ctx->client_window      = XtWindow( client_ctx->client_widget );
        client_ctx->server.display     = XtDisplay( client_ctx->client_widget );
        client_ctx->server.root_window = 
	    DefaultRootWindow( client_ctx->server.display );
        
        BkrCreateAtoms( &client_ctx->server );
        
        XtAddEventHandler( client_ctx->client_widget, NoEventMask, 
                           TRUE, BkrApiClientMessageHandler, 
                           client_ctx );

        /* return local context to user's Opaque pointer */
        
        *((BkrClientStruct **)bkr_client_context) = client_ctx;
        return( Bkr_Success );
    }
    else 
    {
        BKR_FREE(client_ctx);
        *((BkrClientStruct **)bkr_client_context) = NULL;
	return( Bkr_Create_Client_Failure );
    }
}


int BkrDisplay( bkr_client_context, args, argc )
Opaque	bkr_client_context;
ArgList	args;
int	argc;
{
    BkrClientStruct *client_ctx;
    char	     bkrdata[BKR_DATA_SIZE];
    char	    *ptr;
    long	     x_position;
    long	     y_position;
    char	     x_position_str[16];
    char	     y_position_str[16];
    int		     status;
    BkrServerState   server_state;
    String	     filespec = NULL;
    char	     object[20];
    int		     index;

    client_ctx = ( BkrClientStruct * )bkr_client_context;

    /* Attempt to handle any incoming messages, in case this has been called
     * without any intervening event handling.
     */
    BkrProcessMessages(client_ctx);

    /* if we're still waiting for Boorkeader to start, we can't do anything
     * else right now.
     */
    if (client_ctx->request_status == Startup_Request) 
    {
        return Bkr_Busy ;
    }

    /* See what state the Bookreader server is in.
     */
    server_state = BkrExists(&client_ctx->server);

    switch (server_state) {
        case BkrServerChanged:
        case BkrServerAbsent: 
        { 
            /* The Bookreader that proceesed our last request is no longer
             * active, so the client no longer has any books displayed.
             */
	    client_ctx->display_status = FALSE; 
	    break;
	}
        case BkrServerLocked: 
        { 
            /* Bookreader is starting up or about to be started by some other
             * client, so we can't do anything else right now.
             */
	    return Bkr_Busy; 
	    break; 
	}
        case BkrServerUnchanged: 
        {
            if ( client_ctx->request_status == Processing_Request )
            {
                /* We're still waing for Bookreader to process our last
                 * request, so we can't do anything else right now.
                 */
		return Bkr_Busy;
            }
            break;
        }
    }

    /* Now process the request.
     */
    client_ctx->request_status = Processing_Request;

    filespec = BkrResolvePath(client_ctx,args,argc);
    if (filespec == NULL) 
    {
        return Bkr_Bad_Filename;
    }

    memset( bkrdata, 0, BKR_DATA_SIZE);

    index = BkrGetArg( args, argc, BkrNobject );

    if ( index < 0 )
        return( Bkr_Invalid_Object );  /* "Invalid object" */

    if ( args[index].value == Bkr_Topic )
    {
        /* encode topic request in bkrdata */

	strcpy( bkrdata, "T" );
    }
    else if ( args[index].value == Bkr_Directory )
    {
        /* encode directory request in bkrdata */

        strcpy( bkrdata, "D" );
    }
    else if ( args[index].value == Bkr_Book )
    {
        /* encode book request in bkrdata */

        strcpy( bkrdata, "B" );
    }
    else if ( args[index].value == Bkr_Widget )
    {
        /* encode widget request in bkrdata */

        strcpy( bkrdata, "W" );
    }
    else {
        return( Bkr_Invalid_Object );  /* "Invalid object" */
    }
    ptr = &bkrdata[2];


    /* Set filespec in encoded Bookreader information */ 

    BkrAppendData( &ptr, filespec );


    /* Tell bookreader whether to use the default window or not */

    index = BkrGetArg( args, argc, BkrNwindowUsage);

    if (( index < 0 ) || ( args[index].value != Bkr_New_Window ))
	BkrAppendData( &ptr, "D" );   /* "Bkr_Default_Window" or NULL */
    else	
	BkrAppendData( &ptr, "N" );   /* "Bkr_New_Window" */


    /* encode entry/topic in encoded Bookreader information */

    index = BkrGetArg( args, argc, BkrNobjectName );

    if ( index < 0 )
	BkrAppendData( &ptr, " " );
    else
	BkrAppendData( &ptr, (char *)args[index].value );    


    /* encode x and y window positions in encoded Bookreader information */

    index = BkrGetArg( args, argc, BkrNxPosition );

    if ( index >= 0 )
    {
	x_position = (long)args[index].value;

	index = BkrGetArg( args, argc, BkrNyPosition );

	if ( index >= 0 )
	{
	    y_position = (long)args[index].value;

	    sprintf( x_position_str, "%d", x_position );
	    sprintf( y_position_str, "%d", y_position );

	    BkrAppendData( &ptr, x_position_str );
	    BkrAppendData( &ptr, y_position_str );
	}
    }

    /* Post the Bookreader information as a property to the client window */

    XChangeProperty( client_ctx->server.display, 
		     client_ctx->client_window,
                     client_ctx->server.data_atom, client_ctx->server.data_type_atom,
                     8, PropModeReplace,
                     (unsigned char *)bkrdata, 
		     BKR_DATA_SIZE );

    XtFree(filespec);

    if ( server_state == BkrServerAbsent ) 
    {
        /* We need to start up a bookreader to handle the request.
         */
        client_ctx->request_status = Startup_Request;
        status = BkrColdStart( client_ctx );
    }
    else 
    {
        /* Just send the current bookreader a message to notify it that the
         * client has a request.
         */
        status = BkrApiSendMessage( client_ctx, client_ctx->server.client_request_atom );
    }

    if ( status == Bkr_Success )
    {
        client_ctx->display_status = TRUE;
    }

    return( status );
}


int BkrClose( bkr_client_context, args, argc )
Opaque	bkr_client_context;
ArgList args;
int	argc;
{
    BkrClientStruct *client_ctx;
    char	     bkrdata[BKR_DATA_SIZE];
    char	    *ptr;
    int		     status = Bkr_Busy;
    BkrServerState   server_state;
    char	    *filespec = NULL;
    int		     index;
    Boolean         quitting = FALSE;

    client_ctx = ( BkrClientStruct * )bkr_client_context;

    /* Attempt to handle any incoming messages, in case this has been called
     * without any intervening event handling.
     */
    BkrProcessMessages(client_ctx);


    /* If there is no file name the client is "exiting" or quitting,
     */
    index =  BkrGetArg( args, argc, BkrNfilename );
    if (( index < 0 ) 
	|| ( (char *)args[index].value == NULL )
	|| ( *(char *)args[index].value == '\0' ))
    {	
        quitting = TRUE;
    }

    if ( client_ctx->request_status == Startup_Request )
    {
        /* The Bookreader is still starting up, so just return "busy" 
         */
	return Bkr_Busy ;
    }

    server_state = BkrExists( &client_ctx->server );

    if ((server_state == BkrServerAbsent) 
        || (server_state == BkrServerChanged)) 
    {
        /* The server that proccessed our original requests is no longer
         * alive.  If there is no server active then just release the lock.
         * If there is a new server active we don't need to tell it anything.
         */
        if (server_state == BkrServerAbsent) {
            BkrUnlockServer(&client_ctx->server);
        }
        /* Now just cleanup or delete the client contex based on whether or
         * not the client is quitting.
         */
        if (quitting) 
        {
            BkrDeleteContext(client_ctx);
        } 
        else 
        {
            /* The server isn't displaying anything and we can just wait
             * for further requests.
             */
            client_ctx->display_status = FALSE;
            client_ctx->request_status = Waiting_For_Request;
        }
        return Bkr_Success;
    }
    else if (server_state == BkrServerLocked) 
    {
        /* Another process is accessing the server.
         */
        if (quitting) 
        {
            /* Try to send a message to the server to tell it that we're quitting
             * and delete the client context.
             */
            (void)BkrApiSendMessage( client_ctx, client_ctx->server.client_exit_atom );
            BkrDeleteContext(client_ctx);
            return Bkr_Success;
        }
        else
        {
            /* Just return busy
             */
            return Bkr_Busy;
        }
    }
    else 
    {
        /* Still the same server as before
         */
        if (quitting) 
        {
            /* Just send a message to the server to tell it that we're quitting
             * and delete the client context.
             */
            status = BkrApiSendMessage( client_ctx, client_ctx->server.client_exit_atom );
            BkrDeleteContext(client_ctx);
        }
        else if (client_ctx->request_status == Processing_Request)
        {
            /* Still waiting for a request to complete, so return busy  
             */
            return( Bkr_Busy );
        }
        else if ( client_ctx->display_status == TRUE ) 
        {
            client_ctx->request_status = Processing_Request;
            
            filespec = BkrResolvePath(client_ctx,args,argc);
            if (filespec == NULL) 
            {
                return Bkr_Bad_Filename;
            }
            memset( bkrdata, 0, BKR_DATA_SIZE);
            
            strcpy( bkrdata, "C" );
            ptr = &bkrdata[2];
            BkrAppendData( &ptr, filespec );
            
            XtFree(filespec);
            
            /* Post the Bookreader information as a property to the root window */
            
            XChangeProperty( client_ctx->server.display, 
                            client_ctx->client_window,
                            client_ctx->server.data_atom, client_ctx->server.data_type_atom,
                            8, PropModeReplace,
                            (unsigned char *)bkrdata, 
                            BKR_DATA_SIZE );
            
            /* send message to Bookreader */
            
            status = BkrApiSendMessage( client_ctx, client_ctx->server.client_request_atom );
            
        }
        
        return( status );
    }
}
