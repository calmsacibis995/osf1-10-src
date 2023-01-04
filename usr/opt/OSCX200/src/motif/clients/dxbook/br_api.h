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
/* DEC/CMS REPLACEMENT HISTORY, Element BR_API.H*/
/* *2     3-MAR-1992 17:11:49 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:48:18 PARMENTER "Client-Server definitions"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BR_API.H*/
/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1991  **
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
/* Public header file for Bkr API */

/* VMS and Ultrix specifics */

#include <Xm/Xm.h>

#ifdef VMS

#include <syidef.h>
#include <descrip.h>
#include <ssdef.h>
#include <lnmdef.h>
#include <clidef.h>

#define DEFAULT_HELP_DIR    "sys$help:"

typedef struct _VMS_ITEM_LIST {
    unsigned short  length;
    unsigned short  itemcode;
    char            *pointer;
    unsigned short  *ret_len;
} VMS_ITEM_LIST ;

#else

#define DEFAULT_HELP_DIR    "/usr/lib/x11/help/"

#endif



/* Enumerated types */

typedef enum { Bkr_Topic, Bkr_Directory, Bkr_Book, Bkr_Widget } Bkr_Object;
typedef enum { Bkr_Default_Window, Bkr_New_Window } Bkr_Window_Usage;
typedef enum { Bkr_Default_Position, Bkr_Set_Position } Bkr_Position_Usage;
typedef enum _Bkr_Status
{ 
    Bkr_Success, Bkr_Send_Event_Failure, Bkr_Startup_Failure, 
    Bkr_Create_Client_Failure, Bkr_Invalid_Object, Bkr_Get_Data_Failure
} Bkr_Status;


/* Other */

#define FILESPEC_SIZE  256
#define BKR_DATA_SIZE 1024
#define NODE_LENGTH     64

Atom	BKR_DATA, BKR_DATA_TYPE, BKR_TOPIC_SUCCESS, 
	BKR_DIRECTORY_SUCCESS, BKR_CLOSE_SUCCESS, 
	BKR_EXIT_SUCCESS, BKR_BOOK_SUCCESS,
	BKR_QUIT_SUCCESS, BKR_WIDGET_SUCCESS,
	BKR_SERVER_FAILURE, BKR_CLIENT_REQUEST; 

typedef enum { Processing_Request, Waiting_For_Request } Request_Status;

typedef struct {
    Widget          client_widget;
    Window          client_window;
    Display         *client_display;
    Window          root_window;
    Window	    server_window;
    Request_Status  request_status;
    char	   *filespec;
    void	   ((*wait_cursor)(Boolean));
    Boolean	    display_status;
    long	    topic_handle;
    long	    directory_handle;
    Atom	    bkr_server_node_atom;
} BkrClientStruct;

typedef struct {
    Widget	    server_widget;
    Window	    server_window;
    Display         *display;
    Window          root_window;
    long	    topic_handle;
    long	    directory_handle;
} BkrServerStruct;

