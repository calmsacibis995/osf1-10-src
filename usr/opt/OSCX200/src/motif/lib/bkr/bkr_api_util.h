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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_API_UTIL.H*/
/* *14   19-JUN-1992 13:32:42 BALLENGER "UCX$CONVERT"*/
/* *13   19-JUN-1992 13:30:57 BALLENGER "Clean up for Alpha/OSF port."*/
/* *12    1-MAY-1992 11:46:01 GOSSELIN "fixed I14Y bug"*/
/* *11   22-APR-1992 21:14:59 BALLENGER "Use XtResolvePathname and remove wait cursor support."*/
/* *10   17-APR-1992 16:14:09 GOSSELIN "fixed filename"*/
/* *9    16-APR-1992 18:23:12 BALLENGER "Fix syncronization & add work-in-progress dialog"*/
/* *8     3-MAR-1992 16:56:13 KARDON "UCXed"*/
/* *7    10-NOV-1991 21:14:50 BALLENGER "Make all atoms part of BkrServerInfo struct."*/
/* *6     8-NOV-1991 09:47:57 GOSSELIN "fixed PROTO defs"*/
/* *5     8-NOV-1991 09:32:50 GOSSELIN "added function protos and fixed ifB"*/
/* *4     4-NOV-1991 15:46:03 GOSSELIN "fixed to make use of arglists"*/
/* *3    15-OCT-1991 09:29:20 GOSSELIN "isolated files"*/
/* *2    14-OCT-1991 12:09:38 BALLENGER "Fix synchronization problems."*/
/* *1    16-SEP-1991 12:44:39 PARMENTER "Function Prototypes for bkr_api_util.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_API_UTIL.H*/
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
**  DESCRIPTION:  header file for utility routines
**
**  AUTHOR:  Dan Gosselin, OSAG
**
**  CREATION DATE:  03-Nov-1991
**
**  MODIFICATION HISTORY:
**
*/

#ifndef BKR_API_UTIL_H
#define BKR_API_UTIL_H

/* include files */
#include <stdlib.h>
#include "br_prototype.h"

#ifndef VMS
#include <sys/types.h>
#include <sys/wait.h>
#endif 

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Mrm/MrmPublic.h>
#include <string.h>
#include <signal.h>

#ifdef VMS

#include <syidef.h>
#include <descrip.h>
#include <ssdef.h>
#include <lnmdef.h>
#include <clidef.h>

#ifndef BKR_MALLOC
#define BKR_MALLOC( size ) lib$vm_malloc( ( size ) )
#endif

#ifndef BKR_FREE
#define BKR_FREE( ptr )   \
  {                         \
    lib$vm_free( ( ptr ) ); \
    ( ptr ) = NULL;         \
  }
#endif

typedef struct _VMS_ITEM_LIST {
    unsigned short  length;
    unsigned short  itemcode;
    char            *pointer;
    unsigned short  *ret_len;
} VMS_ITEM_LIST ;

/* Define a pid_t for VMS
 */
typedef int pid_t;

#else

#ifndef BKR_MALLOC
#define BKR_MALLOC( size ) malloc( ( size ) )
#endif

#ifndef BKR_FREE
#define BKR_FREE( ptr )    \
  {                         \
    free( ( ptr ) );        \
    ( ptr ) = NULL;         \
  }
#endif

#endif


/* Other */

#define FILESPEC_SIZE  255
#define BKR_DATA_SIZE 1024
#define NODE_LENGTH     64

#ifdef vms
#define BKR_API_UID_FILE "DECW$BOOKREADER"
#else
#define BKR_API_UID_FILE "DXBookreader"
#endif

/* Typedefs */

typedef enum { 
    Processing_Request, 
    Waiting_For_Request, 
    Startup_Request 
} Request_Status;

typedef struct {
    Atom	    node_atom;
    Atom            lock_atom;
    Atom            data_atom;
    Atom            data_type_atom;
    Atom            topic_success_atom;
    Atom            directory_success_atom;
    Atom            book_success_atom;
    Atom            widget_success_atom;
    Atom            close_success_atom;
    Atom            server_failure_atom;
    Atom            client_request_atom;
    Atom            client_exit_atom;
    pid_t           lock_pid;    
    pid_t           pid;
    Window	    window;
    Window          root_window;
    Display         *display;
} BkrServerInfo ;

typedef struct {
    BkrServerInfo   server;
    XtAppContext    app_context;
    Widget          client_widget;
    Window          client_window;
    Request_Status  request_status;
    String	    default_filename;
    String          default_type;
    String          default_path;
    String          default_suffix;
    Boolean	    display_status;
    Boolean         wip_active;
    Widget          wip_widget;               /* Work-in-progress widget */
    XmString        wip_message;
    XtIntervalId    coldstart_timer;
} BkrClientStruct;

typedef struct {
    BkrServerInfo   server;
    Widget	    server_widget;
    Window	    window;
    long	    topic_handle;
    long	    directory_handle;
} BkrServerStruct;

typedef enum {
    BkrServerAbsent,
    BkrServerLocked,
    BkrServerChanged,
    BkrServerUnchanged
} BkrServerState;


/* Function PROTOTYPES */

extern int BkrGetArg
    PROTOTYPE((ArgList, int, String));

extern int BkrApiSendMessage
    PROTOTYPE((BkrClientStruct *, Atom));

extern int BkrApiProcessMessages
    PROTOTYPE((BkrClientStruct *));

extern void BkrApiClientMessageHandler
    PROTOTYPE((Widget, XtPointer, XEvent *, Boolean *));

extern void BkrDeleteContext
    PROTOTYPE((BkrClientStruct *));

extern void BkrCreateAtoms
    PROTOTYPE((BkrServerInfo *));

extern Boolean BkrLockServer
    PROTOTYPE((BkrServerInfo *));

extern void BkrUnlockServer
    PROTOTYPE((BkrServerInfo *));

extern BkrServerState BkrExists
    PROTOTYPE((BkrServerInfo *));

extern void BkrAppendData
    PROTOTYPE((char **, char *));

extern String BkrResolvePath
    PROTOTYPE((BkrClientStruct	* , ArgList , int));

extern int BkrColdStart
    PROTOTYPE((BkrClientStruct *));

extern void BkrRemoveWipDialog
    PROTOTYPE((Widget widget,
               caddr_t user_data,
               XmAnyCallbackStruct *callback_data 
               ));

extern void BkrCreateWipDialog
    PROTOTYPE((BkrClientStruct *client));



#endif /* BKR_API_UTIL_H */
