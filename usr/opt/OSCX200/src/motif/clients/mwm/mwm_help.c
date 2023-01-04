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
/*
 */
/*
 * HISTORY
 */
/******************************************************************
**
**                         Copyright (C) 1990, 1993 by
**             Digital Equipment Corporation, Maynard, Mass.
**                         All Rights Reserved
**
**  This software is furnished under a license and may be used and  copied
**  only  in  accordance  with  the  terms  of  such  license and with the
**  inclusion of the above copyright notice.  this software or  any  other
**  copies  thereof may not be provided or otherwise made available to any
**  other person.  no title to and ownership of  the  software  is  hereby
**  transferred.
**
**  The information in this software is subject to change  without  notice
**  and  should  not  be  construed  as  a commitment by digital equipment
**  corporation.
**
**  Digital assumes no responsibility for the use or  reliability  of  its
**  software on equipment which is not supplied by digital.
**
*******************************************************************
**  
**  Facility: Mwm
**
**  Abstract: Window Manager Help module
**
**  Environment: VMS, Unix, Sun
**
*******************************************************************/

#include "WmGlobal.h"
#include <Xm/Xm.h>
#include <Mrm/MrmPublic.h>
#include <stdio.h>
#include <DXm/DXmHelpB.h>
#include <X11/ShellP.h>
#include <DXm/DXmHelpSP.h>
#include <DXm/DECspecific.h>

#include "mwm_cust.h"
#include "mwm_internal.h"
#include "mwm_dialog.h"
#include "mwm_help_internal.h"
#include "mwm_help_forward.h"
#include "mwm_help_callbacks.h"
#include "mwm_util.h"

                                                   
#ifndef HYPERHELP
/*******************************************************************
**                                                                    
**  Description: Initialize the help shell.
**               This is needed to set certain resources
**               on the help shell so that mwm will not
**               wait for geometry requests to itself.
**
**  Formal Parameters
**    Input: request wid,
**           new widget,
**           arg list,
**           number of args.
**  
********************************************************************/

int mwm_help_xtinit( req, new, args, num_args )

Widget req;
DXmHelpShellWidgetRec *new;
ArgList args;
Cardinal *num_args;

/* local variables */
{
Cardinal zero = 0;

/********************************/

    new->wm.wait_for_wm = FALSE;
    new->wm.wm_timeout = 1;
    /* Call the real thing (the real init proc) */
    (*mwm_help_xtinitproc)( req, (Widget)new, NULL, &zero );

}
#endif
/*******************************************************************
**                                                                    
**  Description: Initialize the help callbacks.
**
**  Formal Parameters
**  
********************************************************************/

int mwm_help_init()

/* local variables */
{

/********************************/

    if ( ! mwm_cb_init )
      {
        if ( MrmRegisterNames( mwm_help_cblist, mwm_help_cbnum ) != MrmSUCCESS)
          {
            fprintf( stderr, "The help resources could not be registered.\n" );
            return( false );
          }
#ifdef HYPERHELP
        /* Allocate the context */
        if ( !mwm_alloc( (void *)&mwm_hyperhelp_context,
                         wmGD.numScreens * sizeof( Opaque ),
                         "Error allocating hyperhelp context" ))
        return( false );
#else                                     
        mwm_help_xtinitproc = dxmHelpShellClassRec.core_class.initialize;
        dxmHelpShellClassRec.core_class.initialize = (XtInitProc)mwm_help_xtinit;
#endif    
      }
    return( true );

}

/*******************************************************************/

/* Help module */

#ifdef HYPERHELP

/*******************************************************************
**
**  Description: Help error.
**
**  Formal Parameters
**    Input: string,
**           status.
**                                    
********************************************************************/

void mwm_help_error( string, status )

char *string;
int status;

/* local variables */
{

/********************************/

    printf( "%s, %x\b", string, status );

}

/*******************************************************************
**
**  Description: Help close (on restart or exit).
**
**  Formal Parameters
**                                    
********************************************************************/

void mwm_help_exit()

/* local variables */
{

/* local variables */
int num;

/********************************/

    /* Was help initialized ? */
    if ( ! mwm_cb_init )
        /* Nope, no cleanup necessary */
        return;
    /* Close help on all active screens. */
    for ( num = 0; num < wmGD.numScreens; num++ )
      {
        if ( mwm_hyperhelp_context[ num ] != (Opaque)NULL )
            DXmHelpSystemClose( mwm_hyperhelp_context[ num ], 
                                mwm_help_error, "Help system error" );
      }

}

#endif /* HYPERHELP */

/*******************************************************************
**
**  Description: Help was selected for this screen.
**
**  Formal Parameters
**    Input: screen,
**           topic.
**                                    
********************************************************************/

void mwm_help_screen_up( screen, topic )

int screen;
char *topic;       

/* local variables */
{
XmString string;
Widget parent;
Arg args[ 1 ];
int arg_num;
  
/********************************/

    mwm_watch_set( true );

#ifdef HYPERHELP
    /* Has help been loaded on this screen yet ? */
    if ( mwm_hyperhelp_context[ screen ] == (Opaque)NULL )
      {
        /* Does this match the main screen number ? */
        if ( screen == XScreenNumberOfScreen( XtScreen( wmGD.topLevelW )))
            /* Yes, we already have the parent. */
            parent = wmGD.topLevelW;
        /* No, create the parent */
        else
          {
            arg_num = 0;
            XtSetArg( args[ arg_num ], XtNscreen, 
                      XScreenOfDisplay( DISPLAY, screen )); arg_num++;
            parent = XtAppCreateShell( WM_RESOURCE_NAME, WM_RESOURCE_CLASS,
                                       applicationShellWidgetClass,
                                       DISPLAY, args, arg_num );
          }
        DXmHelpSystemOpen( &mwm_hyperhelp_context[ screen ], parent, 
                           k_mwm_hyperhelp_path, mwm_help_error, 
                           "Help system error" );
      }
    DXmHelpSystemDisplay( mwm_hyperhelp_context[ screen ], k_mwm_hyperhelp_path,
                          "topic", topic, mwm_help_error, "Help system error" );
#else
    /* Has help been loaded on this screen yet ? */
    if ( (*mwm_screen_did)[ screen ][ k_mwm_help_did ].wid == NULL )
      {
        /* Bring up the help widget with the appropriate topic string */
        if ( !mwm_dialog_get( screen, 
                              &(*mwm_screen_did)[ screen ][ k_mwm_help_did ].wid,
                              k_mwm_help_did,
                              mwm_did[ k_mwm_help_did ].name, 
                              mwm_dialog_shell[ screen ] ))
          {
            mwm_watch_set( false );
            return;
          }
        /* Set the help library */
        mwm_help_lib = XmStringCreate( mwm_help_path, XmSTRING_DEFAULT_CHARSET );
        mwm_set( (*mwm_screen_did)[ screen ][ k_mwm_help_did ].wid,
                 DXmNlibrarySpec, mwm_help_lib );
      }
    /* Set the topic */
    string = XmStringCreate( topic, XmSTRING_DEFAULT_CHARSET );
    mwm_set( (*mwm_screen_did)[ screen ][ k_mwm_help_did ].wid,
             DXmNfirstTopic, string );

    /* Bring up the help dialog box */
    mwm_dialog_up( (*mwm_screen_did)[ screen ][ k_mwm_help_did ].wid, 
                   k_mwm_help_did, mwm_dialog_shell[ screen ] );
    XmStringFree( string );

#endif /* HYPERHELP */
    mwm_watch_set( false );

}

/*******************************************************************
**
**  Description: Help was selected from a dialog box.
**           
**  Formal Parameters
**    Input: widget id,               
**           resource,
**           reason.
**  
********************************************************************/

void mwm_help_up( wid, tag, reason )

Widget wid;
int *tag;
unsigned int *reason;

/* local variables */
{
char *topic = (char *)tag;
char *temp;
  
/********************************/

#ifdef HYPERHELP
    /* Strip off any hierarchical help topic information. */
    temp = topic;                      
    while ( temp != NULL )
      {
        temp = strstr( topic, k_mwm_space_str );
        /* Was a space found ? */
        if ( temp != NULL )
            /* Yes, continue */
            temp++;
        /* No, done */
        else break;
        topic = temp;
      }
#endif /* HYPERHELP */

    mwm_help_screen_up( XScreenNumberOfScreen( XtScreen( XtParent( wid ))),
                        topic );

}

/*******************************************************************
**
**  Description: The user selected help from a menu.
**
**  Formal Parameters
**    Input: arg list,
**           ptr to Client,
**           widget.
**  
********************************************************************/

int mwm_help( arg_list, pCD, wid )

String arg_list;                      
ClientData *pCD;
Widget wid;

/* local variables */
{
char topic[ 128 ];

/********************************/

    strcpy( topic, "" );

    if ( mwm_init( WID_SCREEN, WID_SCREEN_NUM ) )
      {
        /* Help on window manager ? */
        if ( mwm_str_eq( arg_list, k_mwm_help_wm_arg ))
            strcpy( topic, k_mwm_help_wm_topic );
        /* Help on version ? */               
        else if ( mwm_str_eq( arg_list, k_mwm_help_version_arg ))
            strcpy( topic, k_mwm_help_version_topic );
        /* Help on terms ? */               
        else if ( mwm_str_eq( arg_list, k_mwm_help_terms_arg ))
            strcpy( topic, k_mwm_help_terms_topic );
        /* Help on shortcuts ? */               
        else if ( mwm_str_eq( arg_list, k_mwm_help_shortcuts_arg ))
            strcpy( topic, k_mwm_help_shortcuts_topic );
        /* copy the argument */               
        else strcpy( topic, arg_list );
      }

    /* Bring up help */
    if ( strlen( topic ) > 0 )
        mwm_help_screen_up( WID_SCREEN, (char *)topic );
                          
    /* Make sure the cursor is reset in case of an error. */
    mwm_watch_set( false );

}    

/*******************************************************************/
