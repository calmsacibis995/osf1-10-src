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
**  Abstract: Window Manager Dialog module
**
**  Environment: VMS, Unix, Sun
**
*******************************************************************/

#include "WmGlobal.h"
#include "WmResNames.h"
#include "WmFunction.h"
#include "WmCEvent.h"
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <X11/ShellP.h>
#include <X11/Shell.h>
#include <Xm/DialogSP.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MessageB.h>
#include <Xm/ToggleB.h>
#include <Xm/Protocols.h>
#include <stdio.h>      /* debug */
#include <DXm/DXmColor.h>

#include "WmError.h"
#include "mwm_col.h"
#include "mwm_cust.h"
#include "mwm_internal.h"
#include "mwm_dialog.h"
#include "mwm_dialog_internal.h"
#include "mwm_dialog_forward.h"
#include "mwm_dialog_callbacks.h"
#include "mwm_icons.h"
#include "mwm_util.h"
                                                   
/* Initialization module */

/*******************************************************************
**                                                                    
**  Description: Initialize the customize shell.
**
**  Formal Parameters
**    Input: screen index
**           screen number.
**  
********************************************************************/

int mwm_dialog_shell_init( screen, screen_num )

int screen;
int screen_num;

/* local variables */
{
Arg arg_list[ 8 ];
int n;

/********************************/

    /* Create the shell if necessary. */                                  
    if ( mwm_dialog_shell[ screen ] == NULL )
      {
        n = 0;
        XtSetArg( arg_list[ n ], XtNallowShellResize, (XtArgVal)true ); n++;
        XtSetArg( arg_list[ n ], XmNwaitForWm, (XtArgVal)false ); n++;
        XtSetArg( arg_list[ n ], XtNscreen,
                  (XtArgVal)ScreenOfDisplay( wmGD.dialog_display,
                                             screen_num ) ); n++;
        mwm_dialog_shell[ screen ] =
          XtAppCreateShell( k_mwm_dialog_resource_name,
                            k_mwm_dialog_resource_class,
                            applicationShellWidgetClass,
                            wmGD.dialog_display, arg_list, n );
      }
    return( true );

}

/*******************************************************************
**                                                                    
**  Description: Initialize the dialog shell.
**               This is needed to set certain resources
**               on the dialog shell so that mwm will not
**               wait for geometry requests to itself.
**               Help creates dialog shells that mwm
**               can not access; so this initialize is
**               called.
**
**  Formal Parameters
**    Input: request wid,
**           new widget,
**           arg list,
**           number of args.
**  
********************************************************************/

int mwm_dialog_xtinit( req, new, args, num_args )

Widget req;
XmDialogShellWidgetRec *new;
ArgList args;
Cardinal *num_args;

/* local variables */
{
Cardinal  zero = 0;

/********************************/

    new->wm.wait_for_wm = FALSE;
    new->wm.wm_timeout = 1;
    /* Call the real thing (the real init proc) */
    (*mwm_dialog_xtinitproc)( req, (Widget)new, NULL, &zero );

}

/*******************************************************************
**                                                                    
**  Description: Initialize the UIL file and mrm callbacks.
**
**  Formal Parameters
**  
********************************************************************/

int mwm_dialog_init()

/* local variables */
{

/********************************/

    if ( mwm_cb_init )
        return( true );

    if ( MrmRegisterNames( mwm_dialog_cblist, mwm_dialog_cbnum ) != MrmSUCCESS)
      {
        fprintf( stderr, "The dialog resources could not be registered.\n" );
        return( false );
      }

    if ( !mwm_alloc( (void *)&mwm_icon,
                     wmGD.numScreens * sizeof( Pixmap ) * ( k_mwm_max_did ),
                     "Insufficient memory for icons" ))
        return( false );

    /* Get the initialize field, save it */
    mwm_dialog_xtinitproc = xmDialogShellClassRec.core_class.initialize;
    xmDialogShellClassRec.core_class.initialize = (XtInitProc)mwm_dialog_xtinit;
    return( true );

}

/*******************************************************************/
               
/* Utility module */

/*******************************************************************
**               
**  Description: Get a string from UIL.
**
**  Formal Parameters
**    Input: source,
**    Output: string.
**
********************************************************************/

int mwm_dialog_text_get( source, result )
                  
char *source;
XmString *result;
{
/* local variables */
int string_len;
MrmCode type;
                                               
/********************************/

    if ( !mwm_uil_init() )
        return( false );
    if ( mwm_hierarchy == NULL )
        return( false );
    if ( *result == NULL )
        if ( MrmFetchLiteral( mwm_hierarchy, source, DISPLAY, (caddr_t *)result, &type ) 
          != MrmSUCCESS )
          {
             sprintf( source, "The literal, %s, could not be fetched from UIL\n" );
             return( false );
          };

    return( true );

}

/*******************************************************************
**
**  Description: Bring up an error message.
**
**  Formal Parameters
**    Input: wid,
             did,
**           field in error,
**           contents of string.
**  
********************************************************************/
                                                               
int mwm_dialog_error_up( wid, did, error_fid, field )

Widget wid;
int did;
int error_fid;                           
char *field;
                        
{                                                        
/* local variables */
int status;
Widget mess_wid;
XmString error_string;
XmString field_string;             
XmString string;                             
XmString title;

/********************************/

    /* Bring up the dialog box. */
    if ( mwm_dialog_get( WID_SCREEN,
                         &(*mwm_screen_did)[ WID_SCREEN ][ did ].error_wid,
                         k_mwm_cust_error_did, 
                         mwm_did[ k_mwm_cust_error_did ].name, 
                         (*mwm_screen_did)[ WID_SCREEN ][ did ].wid ))
      {                                                            

        /* Remove cancel */
        mess_wid = XmMessageBoxGetChild( (*mwm_screen_did)[ WID_SCREEN ][ did ].error_wid,
                                    XmDIALOG_CANCEL_BUTTON );
        XtUnmanageChild( mess_wid );

        /* Set Ok to also be cancel button */
        mess_wid = XmMessageBoxGetChild( (*mwm_screen_did)[ WID_SCREEN ][ did ].error_wid, 
                                    XmDIALOG_OK_BUTTON );
        mwm_set( mess_wid, XmNcancelButton, (void *)mess_wid );

        /* Set the message */
        switch ( error_fid )
          {
            case k_mwm_icon_height_fid:
              mwm_dialog_text_get( mwm_cust_error.icon_height_name, 
                                &mwm_cust_error.icon_height );
              error_string = XmStringCopy( mwm_cust_error.icon_height ); 
              break;
            case k_mwm_icon_width_fid:
              mwm_dialog_text_get( mwm_cust_error.icon_width_name, 
                                &mwm_cust_error.icon_width );
              error_string = XmStringCopy( mwm_cust_error.icon_width ); 
              break;
            case k_mwm_border_size_text_fid:
              mwm_dialog_text_get( mwm_cust_error.border_size_name, 
                                &mwm_cust_error.border_size );
              error_string = XmStringCopy( mwm_cust_error.border_size ); 
              break;
            case k_mwm_matte_size_text_fid:                         
              mwm_dialog_text_get( mwm_cust_error.matte_size_name, 
                                &mwm_cust_error.matte_size );
              error_string = XmStringCopy( mwm_cust_error.matte_size ); 
              break;
            default:                 
              mwm_dialog_text_get( mwm_cust_error.number_name, 
                                &mwm_cust_error.number );
              error_string = XmStringCopy( mwm_cust_error.number ); 
              break;
          }
        field_string = XmStringCreate( field, XmSTRING_DEFAULT_CHARSET );
        string = XmStringConcat( error_string, field_string );            
        /* Set the message text */
        mwm_set( (*mwm_screen_did)[ WID_SCREEN ][ did ].error_wid, 
                 XmNmessageString, (void *)string );                  
        XmStringFree( field_string );
        XmStringFree( error_string );
        XmStringFree( string );
                                   
        /* Set the title */
        mwm_get( (*mwm_screen_did)[ WID_SCREEN ][ did ].wid, 
                 XmNdialogTitle, (void *)(&title) );
        mwm_set( (*mwm_screen_did)[ WID_SCREEN ][ did ].error_wid, 
                 XmNdialogTitle, (void *)(title) );

        mwm_set( (*mwm_screen_did)[ WID_SCREEN ][ did ].error_wid, XtNwindowGroup,
                 (void *)(*mwm_screen_did)[ WID_SCREEN ][ did ].wid );
        status = mwm_dialog_up( (*mwm_screen_did)[ WID_SCREEN ][ did ].error_wid, 
                              k_mwm_cust_error_did,
                              (*mwm_screen_did)[ WID_SCREEN ][ did ].wid );
      } 
    /* Make sure the cursor is reset in case of an error. */
    mwm_watch_set( false );
    return( true );
        
}

/*******************************************************************/

/* Field module */

/*******************************************************************
**
**  Description: Initialize a field.
**
**  Formal Parameters
**    Input: widget,
**           field id.
**  
********************************************************************/

void mwm_dialog_field_init( wid, fid )

Widget wid;
int fid;

{
/* local variables */
int menu_fid;
  
/********************************/

    /* Has the widget been initialized ? */
    if ( wid == NULL )
        /* No, done. */
        return;

    switch ( fid )
      {
        case k_mwm_inactive_sample_fid:
        case k_mwm_active_sample_fid:
        case k_mwm_icon_image_sample_fid:
        case k_mwm_icon_box_sample_fid:
        case k_mwm_matte_sample_fid:
          mwm_col_sample_crea( wid, fid );
          break;
        case k_mwm_use_icon_box_fid:
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], mwm_cust_info.use_icon_box,
                                  true );                                 
          break;
        case k_mwm_wmenu_dclick_fid:
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], mwm_cust_info.wmenu_dclick,
                                  true );
          break;
        case k_mwm_focus_raised_fid:
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], mwm_cust_info.focus_raised,
                                  true );
          break;
        case k_mwm_focus_deicon_fid:
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], mwm_cust_info.focus_deicon,
                                  true );
          break;
        case k_mwm_focus_start_fid:
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], mwm_cust_info.focus_start,
                                  true );
          break;   
        case k_mwm_focus_removed_fid:
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], mwm_cust_info.focus_removed,
                                  true );    
          break;
        case k_mwm_icon_placement_fid:
          mwm_set( (*mwm_fid)[ WID_SCREEN ][ fid ], XmNmenuHistory, 
                   (*mwm_fid)[ WID_SCREEN ][ mwm_cust_info.icon_placement ] );
          /* Set based on use of icon box */
          XtSetSensitive( (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_placement_fid ],
                          !mwm_cust_info.use_icon_box );
          break;
        case k_mwm_menu_col_fid:
          if ( !wmGD.multicolor && ( wmGD.Screens[ 0 ].monitor == k_mwm_bw_type ))
              mwm_cust_info.menu_col = k_mwm_menu_col_none_fid;
          mwm_set( (*mwm_fid)[ WID_SCREEN ][ fid ], XmNmenuHistory, 
                   (*mwm_fid)[ WID_SCREEN ][ mwm_cust_info.menu_col ] );
          if ( !wmGD.multicolor && ( wmGD.Screens[ 0 ].monitor == k_mwm_bw_type ))
              XtSetSensitive( (*mwm_fid)[ WID_SCREEN ][ fid ], false );
          break;
        case k_mwm_icon_size_fid:
          mwm_set( (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_size_fid ], XmNmenuHistory, 
                   (*mwm_fid)[ WID_SCREEN ][ mwm_cust_info.icon_size_fid ] );
          break;
        case k_mwm_icon_width_fid: 
          mwm_cust_size_other( mwm_cust_info.icon_size_fid, 
                               k_mwm_icon_other_fid, k_mwm_none,
                               (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_width_label_fid ],
                               (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_width_fid ],
                               mwm_cust_info.icon_width,
                               mwm_cust_info.icon_width_str );
          break;
        case k_mwm_icon_height_fid: 
          mwm_cust_size_other( mwm_cust_info.icon_size_fid, 
                               k_mwm_icon_other_fid, k_mwm_none,
                               (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_height_label_fid ],
                               (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_height_fid ],
                               mwm_cust_info.icon_height,
                               mwm_cust_info.icon_height_str );
          break;
        case k_mwm_border_size_fid:
          mwm_set( (*mwm_fid)[ WID_SCREEN ][ k_mwm_border_size_fid ], XmNmenuHistory, 
                   (*mwm_fid)[ WID_SCREEN ][ mwm_cust_info.border_size_fid ] );
          break;
        case k_mwm_border_size_text_fid:                       
          mwm_cust_size_other( mwm_cust_info.border_size_fid, 
                               k_mwm_border_other_fid, k_mwm_none,
                               (*mwm_fid)[ WID_SCREEN ][ k_mwm_border_size_label_fid ],
                               (*mwm_fid)[ WID_SCREEN ][ k_mwm_border_size_text_fid ],
                               mwm_cust_info.border_size,
                               mwm_cust_info.border_size_str );
          break;
        case k_mwm_focus_fid:
          if ( mwm_cust_info.implicit_focus )
              menu_fid = k_mwm_implicit_focus_fid;
          else menu_fid = k_mwm_explicit_focus_fid;
          mwm_set( (*mwm_fid)[ WID_SCREEN ][ fid ], XmNmenuHistory, 
                   (*mwm_fid)[ WID_SCREEN ][ menu_fid ] );
          break;
        case k_mwm_move_fid:
          if ( mwm_cust_info.move_window )
              menu_fid = k_mwm_move_window_fid;
          else menu_fid = k_mwm_move_outline_fid;
          mwm_set( (*mwm_fid)[ WID_SCREEN ][ fid ], XmNmenuHistory, 
                   (*mwm_fid)[ WID_SCREEN ][ menu_fid ] );
          break;
        case k_mwm_fore_fid:
        case k_mwm_back_fid:
        case k_mwm_top_fid:
        case k_mwm_bot_fid:
        case k_mwm_active_fore_fid:
        case k_mwm_active_back_fid:
        case k_mwm_active_top_fid:  
        case k_mwm_active_bot_fid:
        case k_mwm_icon_image_fore_fid: 
        case k_mwm_icon_image_back_fid:
        case k_mwm_icon_image_top_fid:             
        case k_mwm_icon_image_bot_fid:
        case k_mwm_icon_box_back_fid:
        case k_mwm_matte_fore_fid: 
        case k_mwm_matte_back_fid:
        case k_mwm_matte_top_fid:             
        case k_mwm_matte_bot_fid:
          /* Update the color */
          XAllocColor( wmGD.dialog_display, XDefaultColormap( DISPLAY, WID_SCREEN_NUM ),
                       &mwm_cust_info.col[ mwm_col_index_get( fid ) ].xcol);          
          /* fill push button with current colors */
          mwm_set( (*mwm_fid)[ WID_SCREEN ][ fid ], XmNbackground, 
                   (void *)(mwm_cust_info.col[ mwm_col_index_get( fid )].xcol.pixel) );
          break;
        case k_mwm_min_fid :
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], mwm_cust_info.min, true );
          break;
        case k_mwm_max_fid :
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], mwm_cust_info.max, true );
          break;
        case k_mwm_title_fid :
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], mwm_cust_info.title, true );
          break;
        case k_mwm_resize_fid :
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], mwm_cust_info.resize, true );
          break;
        case k_mwm_border_fid :
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], mwm_cust_info.border, true );
          break;
        case k_mwm_menu_fid :
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], mwm_cust_info.menu, true );
          break;
        case k_mwm_active_icon_fid :
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], mwm_cust_info.active_icon, true );
          break;
        case k_mwm_icon_label_fid :
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], mwm_cust_info.icon_label, true );
          break;
        case k_mwm_icon_image_fid :
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], mwm_cust_info.icon_image, true );
          break;
        case k_mwm_inactive_auto_shade_fid:                                   
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], 
                                  mwm_cust_info.inactive_auto_shade, true );
          mwm_col_auto_chec( wid, fid );
          break;
        case k_mwm_force_alt_space_fid:
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], 
                                  mwm_cust_info.force_alt_space, true );
          break;
        case k_mwm_fb_move_size_fid:
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], 
                                  mwm_cust_info.fb.resize || 
                                  mwm_cust_info.fb.move, true );
          break;
        case k_mwm_ignore_mods_fid:
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], 
                                  mwm_cust_info.ignore_mods, true );
          break;
        case k_mwm_icon_box_auto_shade_fid:                                   
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], 
                                  mwm_cust_info.icon_box_auto_shade, true );
          mwm_col_auto_chec( wid, fid );
          break;
        case k_mwm_active_auto_shade_fid:
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], 
                                  mwm_cust_info.active_auto_shade, true );     
          mwm_col_auto_chec( wid, fid );
          break;                                              
        case k_mwm_image_auto_shade_fid:
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], mwm_cust_info.image_auto_shade, true );
          mwm_col_auto_chec( wid, fid );
          break;
        case k_mwm_matte_auto_shade_fid:
          XmToggleButtonSetState( (*mwm_fid)[ WID_SCREEN ][ fid ], mwm_cust_info.matte_auto_shade, true );
          /* Is there a Matte ? */                    
          if ( mwm_cust_info.matte_size_fid != k_mwm_matte_none_fid )
            {
              /* Yes, set the auto color fields */
              mwm_col_auto_chec( wid, fid );
              /* Set the sensitivity of the matte fields to True */
              mwm_cust_size_none( wid, mwm_cust_info.matte_size_fid, 
                                  k_mwm_matte_none_fid );
            }
          /* Set the sensitivity of the matte fields to False */
          else mwm_cust_size_none( wid, k_mwm_matte_none_fid, 0 );
          break;
        case k_mwm_matte_size_fid:
          mwm_set( (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_size_fid ], XmNmenuHistory, 
                   (*mwm_fid)[ WID_SCREEN ][ mwm_cust_info.matte_size_fid ] );
          break;
        case k_mwm_matte_size_text_fid:                       
          mwm_cust_size_other( mwm_cust_info.matte_size_fid, 
                               k_mwm_matte_other_fid, k_mwm_none,
                               (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_size_label_fid ],
                               (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_size_text_fid ],
                               mwm_cust_info.matte_size,
                               mwm_cust_info.matte_size_str );
          break;
        case k_mwm_col_mix_fid:
          mwm_col_mix_set( wid );
          break;
        /* Message label for the color mix. */
        case k_mwm_cust_col_mix_label_fid:
          mwm_col_mix_mess_set( wid );
          break;
        default:
          break;
      }

    /* Set sensitivity of focus fields */
    switch ( fid )
      {
        case k_mwm_focus_deicon_fid:
        case k_mwm_focus_start_fid:
        case k_mwm_focus_removed_fid:
          XtSetSensitive( (*mwm_fid)[ WID_SCREEN ][ fid ], 
                          !mwm_cust_info.implicit_focus );
          break;
        default: break;
      }

}

/*******************************************************************
**
**  Description: Create a new field.
**
**  Formal Parameters
**    Input: widget id,               
**           resource,
**           reason.
**  
********************************************************************/

void mwm_dialog_field_crea( wid, tag, reason )

Widget wid;
int *tag;
unsigned int *reason;

/* local variables */
{
int fid = *tag;
  
/********************************/

    (*mwm_fid)[ WID_SCREEN ][ fid ] = wid;
    mwm_dialog_field_init( wid, fid );

}

/*******************************************************************
**
**  Description: An option menu field was set.
**
**  Formal Parameters
**    Input: widget id,               
**           resource,
**           reason.
**  
********************************************************************/

void mwm_dialog_field_set( wid, tag, reason )

Widget wid;
int *tag;
unsigned int *reason;

/* local variables */
{
int fid = *tag;
  
/********************************/

    switch ( fid )
      {
        case k_mwm_icon_box_auto_shade_fid:  
        case k_mwm_inactive_auto_shade_fid:  
        case k_mwm_active_auto_shade_fid:
        case k_mwm_image_auto_shade_fid:
          /* Set the auto color fields */
          mwm_col_auto_chec( wid, fid );
          break;
        case k_mwm_matte_auto_shade_fid:
          /* Is matte none? */
          if ( mwm_cust_info.matte_size_fid != k_mwm_matte_none_fid )
              /* No, set the auto color fields */
              mwm_col_auto_chec( wid, fid );
          break;
        case k_mwm_menu_col_active_fid:
        case k_mwm_menu_col_inactive_fid:
        case k_mwm_menu_col_none_fid:
          mwm_cust_info.menu_col = fid;
          break;
        case k_mwm_icon_top_left_fid:
        case k_mwm_icon_top_right_fid:
        case k_mwm_icon_bot_left_fid:
        case k_mwm_icon_bot_right_fid:
        case k_mwm_icon_left_top_fid:
        case k_mwm_icon_left_bot_fid:
        case k_mwm_icon_right_top_fid:
        case k_mwm_icon_right_bot_fid:
          mwm_cust_info.icon_placement = fid;
          break;
        case k_mwm_move_window_fid:
        case k_mwm_move_outline_fid:
          mwm_cust_info.move_window = fid == k_mwm_move_window_fid;
          break;   
        case k_mwm_implicit_focus_fid:
        case k_mwm_explicit_focus_fid:
          mwm_cust_info.implicit_focus = fid == k_mwm_implicit_focus_fid;
          XtSetSensitive( (*mwm_fid)[ WID_SCREEN ][ k_mwm_focus_deicon_fid ], 
                          !mwm_cust_info.implicit_focus );
          XtSetSensitive( (*mwm_fid)[ WID_SCREEN ][ k_mwm_focus_start_fid ], 
                          !mwm_cust_info.implicit_focus );
          XtSetSensitive( (*mwm_fid)[ WID_SCREEN ][ k_mwm_focus_removed_fid ], 
                          !mwm_cust_info.implicit_focus );
          break;
        case k_mwm_icon_small_fid: 
        case k_mwm_icon_medium_fid: 
        case k_mwm_icon_large_fid: 
        case k_mwm_icon_wide_fid: 
        case k_mwm_icon_other_fid:
          mwm_cust_size_other( fid, k_mwm_icon_other_fid, 
                               mwm_cust_info.icon_size_fid,
                               (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_width_label_fid ],
                               (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_width_fid ],
                               mwm_cust_info.icon_width,
                               mwm_cust_info.icon_width_str );
          mwm_cust_size_other( fid, k_mwm_icon_other_fid, 
                               mwm_cust_info.icon_size_fid, 
                               (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_height_label_fid ],
                               (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_height_fid ],
                               mwm_cust_info.icon_height,
                               mwm_cust_info.icon_height_str );
          mwm_cust_info.icon_size_fid = fid;
          break;
        case k_mwm_border_small_fid: 
        case k_mwm_border_medium_fid: 
        case k_mwm_border_large_fid: 
        case k_mwm_border_other_fid:
          mwm_cust_size_other( fid, k_mwm_border_other_fid, 
                               mwm_cust_info.border_size_fid,
                               (*mwm_fid)[ WID_SCREEN ][ k_mwm_border_size_label_fid ],
                               (*mwm_fid)[ WID_SCREEN ][ k_mwm_border_size_text_fid ],
                               mwm_cust_info.border_size,
                               mwm_cust_info.border_size_str );
          mwm_cust_info.border_size_fid = fid;
          break;
        case k_mwm_matte_none_fid: 
        case k_mwm_matte_small_fid: 
        case k_mwm_matte_medium_fid: 
        case k_mwm_matte_large_fid: 
        case k_mwm_matte_other_fid:
          mwm_cust_size_other( fid, k_mwm_matte_other_fid, 
                               mwm_cust_info.matte_size_fid,
                               (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_size_label_fid ],
                               (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_size_text_fid ],
                               mwm_cust_info.matte_size,
                               mwm_cust_info.matte_size_str );
          /* Check if the matte was changed to none or from none. */
          mwm_cust_size_none( wid, fid, mwm_cust_info.matte_size_fid );          
          mwm_cust_info.matte_size_fid = fid;
          break;
        case k_mwm_use_icon_box_fid:
          /* Reset icon placement if use box was changed. */
          XtSetSensitive( (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_placement_fid ],
                          !XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_use_icon_box_fid ] ));
          break;
        default:
          break;
      }
}

/*******************************************************************/

/* Dialog module */
                       
/*******************************************************************
**
**  Description: A dialog box was created.
**               Initialize the user data area.
**
**  Formal Parameters
**    Input: widget id,               
**           resource,
**           reason.
**  
********************************************************************/

void mwm_dialog_crea( wid, tag, reason )

Widget wid;
int *tag;
unsigned int *reason;

{
/* local variables */
int did = *tag;

/********************************/

}
    
/*******************************************************************
**
**  Description: Get the customize dialog box.
**
**  Formal Parameters
**    Input: wid,
**           dialog id, 
**           name,
**           parent wid.
**
********************************************************************/

int mwm_dialog_get( screen, wid_ptr, did, name, parent_wid )

int screen;
Widget *wid_ptr;
int did; 
char *name;
Widget parent_wid;
 
/* local variables */
{
MrmType classid;
Arg arg_list[ 8 ];
int x, y, n, status;
Dimension width, height;
Widget dialog_shell;
Widget wid;
Atom delete_window_atom;
int *value;                                                 

/********************************/
         
    if ( mwm_hierarchy == NULL )
        return( false );

    /* Is this color mix or help ? */
    if (( did == k_mwm_cust_col_mix_did ) || ( did == k_mwm_help_did ))
      /* Yes, these can be up on both screens at the same time. */
      {
        /* Is it up on this screen. */
        if ( (*mwm_screen_did)[ screen ][ did ].up )
            /* Yes, done here */
            return( true );
      }
    /* Is it already up on a screen ? */
    else if ( mwm_did[ did ].up )
      {
        /* Yes, set to true if it's up on this screen.
                false if it's up on another_screen. */
        return( mwm_did[ did ].screen == (screen) );
      }

    /* Save the screen. */
    mwm_did[ did ].screen = screen;
    /* Has it already been fetched ? */
    if ( *wid_ptr == NULL )
      {
        /* Get the customization information. */
        mwm_cust_get( did );
        mwm_watch_set( true );
        dialog_shell = parent_wid;
             
        /* Fetch customize widget */
        status = MrmFetchWidget( mwm_hierarchy, name,
                                 dialog_shell, wid_ptr, &classid );
        if ( status != MrmSUCCESS )
          { 
            Warning( "Could not fetch Customize widget" );
            mwm_watch_set( false );
            return( false );  
          }
        /* Create the atom for the close */
        delete_window_atom = XmInternAtom( XtDisplay( XtParent( *wid_ptr ) ), 
                                           "WM_DELETE_WINDOW", FALSE );
        value = NULL;
        if ( mwm_alloc( (char **)&value, sizeof( did ),
                        "Error allocating did for close" ))
          {
            *value = did;
            /* Establish a callback for delete */
            XmAddWMProtocolCallback( XtParent( *wid_ptr ), delete_window_atom,
                                     (XtCallbackProc)mwm_dialog_cancel, 
                                     (caddr_t)value );
            /* Activate the delete protocol */
            XmActivateWMProtocol( XtParent( *wid_ptr ), delete_window_atom );
            /* Make sure that the window manager does nothing on a close.
               If this is not set, the window manager will delete the shell. */
            mwm_set( XtParent( *wid_ptr ), XmNdeleteResponse, (void *)XmDO_NOTHING );
          }
      }
    /* Set the dialog box fields. */
    else 
      {
        /* Assign the wid (to get the screen using WID_SCREEN below) */
        wid = *wid_ptr;
        mwm_dialog_rese( did, mwm_fid_list[ did ] );
      }
/*     else mwm_dialog_col_rese( screen, mwm_fid_res_list[ did ] ); */

    return( true );

}    

/*******************************************************************
**
**  Description: The user OKed one of the customize dialogs.
**                           
**  Formal Parameters
**    Input: widget id,               
**           resource,
**           reason.
**  
********************************************************************/

void mwm_dialog_ok( wid, tag, reason )

Widget wid;
int *tag;
unsigned int *reason;
{                                                          
/* local variables */
int did = *tag;
int ok;
int mix_did;

/********************************/

    /* It's been modified */
    switch ( did )
      {
        case k_mwm_cust_ws_did:
          ok = mwm_cust_ws_ok( wid );
          break;
        case k_mwm_cust_border_did:
          ok = mwm_cust_border_ok( wid );    
          break;  
        case k_mwm_cust_border_col_did:
          ok = mwm_cust_border_col_ok( wid );
          break;
        case k_mwm_cust_icons_did:        
          ok = mwm_cust_icons_ok( wid );                
          break;
        case k_mwm_cust_icon_col_did:
          ok = mwm_cust_icon_col_ok( wid );
          break;                     
        case k_mwm_cust_matte_did:
          ok = mwm_cust_matte_ok( wid );     
          break;
        case k_mwm_cust_col_mix_did:
          ok = mwm_col_mix_ok( wid, (int *)tag, (unsigned int *)reason );
          break;
        default:
          break;                  
      }

    if ( ok && (( did != k_mwm_cust_restart_did ) &&
                ( *reason != XmCR_APPLY ) &&
                ( did != k_mwm_cust_error_did )))
      {
        XtUnmanageChild( (Widget)(*mwm_screen_did)[ WID_SCREEN ][ did ].wid );
        /* Color mix can be up on several screens. */
        if (( did != k_mwm_cust_col_mix_did ) && 
            ( did != k_mwm_help_did ))
            mwm_did[ did ].up = false;
        else (*mwm_screen_did)[ WID_SCREEN ][ did ].up = false;
        mwm_cust_info.mod[ did ] = true;
      }
    /* Unmanage the open mix box */
    /* Is there an open mix box ? */
    if ( mwm_col_did_get( mwm_cust_info.mix_box[ WID_SCREEN ].index  ) == did )
      /* Yes, unmanage it */
      {
        mix_did = k_mwm_cust_col_mix_did;
        XtUnmanageChild( (Widget)(*mwm_screen_did)[ WID_SCREEN ][ mix_did ].wid );
        mwm_cust_info.mod[ mix_did ] = true;
        (*mwm_screen_did)[ WID_SCREEN ][ mix_did ].up = false;
      }                      

}

/*******************************************************************
**
**  Description: The user canceled one of the customize dialogs.
**
**  Formal Parameters
**    Input: widget id,               
**           resource,
**           reason.
**  
********************************************************************/

void mwm_dialog_cancel( wid, tag, reason )

Widget wid;
int *tag;
unsigned int *reason;

/* local variables */           
{                    
int did = *tag;
int mix_did;
ShellWidget shell;

/********************************/

    switch ( did )
      {
        case k_mwm_cust_matte_did:
        case k_mwm_cust_border_did:
        case k_mwm_cust_icons_did:
        case k_mwm_cust_border_col_did:
        case k_mwm_cust_ws_did:   
        case k_mwm_cust_icon_col_did:
        case k_mwm_cust_apply_did:
          if ( mwm_did[ did ].up )
            {
              XtUnmanageChild( (*mwm_screen_did)[WID_SCREEN ][ did ].wid );
              /* Reset to last OKed settings */
              mwm_did[ did ].up = false;
              mwm_cust_info_rese( (*mwm_screen_did)[ WID_SCREEN ][ did ].wid,
                                  did, &mwm_cust_info_last );
            }                        
          else return;
          break;
        /* Customize colors and help can be up on several screens. */
        case k_mwm_cust_col_mix_did:
        case k_mwm_help_did:
          if ( (*mwm_screen_did)[ WID_SCREEN ][ did ].up )
            {
              XtUnmanageChild( (*mwm_screen_did)[ WID_SCREEN ][ did ].wid );
              (*mwm_screen_did)[ WID_SCREEN ][ did ].up = false;
            }
          break;
        case k_mwm_cust_error_did:
          XtUnmanageChild( XtParent( wid ));
          break;                        
        case k_mwm_cust_restart_did:
          if ( mwm_did[ did ].up )
            {
              XtUnmanageChild( (*mwm_screen_did)[ WID_SCREEN ][ did ].wid );
              mwm_did[ did ].up = false;
            }
          else return;
          break;                        
        default:            
          break;
      }

    /* Unmanage the open mix box */

    /* Is there an open mix box ? */
    if ( mwm_col_did_get( mwm_cust_info.mix_box[ WID_SCREEN ].index ) == did )
      /* Yes, unmanage it */
      {
        mix_did = k_mwm_cust_col_mix_did;
        mwm_dialog_cancel( wid, &mix_did, reason );
      }                      

    /* Unmanage the message box */
    if ( (*mwm_screen_did)[ WID_SCREEN ][ did ].error_wid != NULL )
      {
        if ( XtIsManaged( (*mwm_screen_did)[ WID_SCREEN ][ did ].error_wid ))
            XtUnmanageChild( (*mwm_screen_did)[ WID_SCREEN ][ did ].error_wid );
            /* Don't save message boxes */
        XtDestroyWidget( XtParent( (*mwm_screen_did)[ WID_SCREEN ][ did ].error_wid ));
        (*mwm_screen_did)[ WID_SCREEN ][ did ].error_wid = (Widget)NULL;
      }

}

/*******************************************************************
**
**  Description: Reset the colors for multiscreen systems.
**
**  Formal Parameters
**    Input: screen,
**           field list.
**  
********************************************************************/

void mwm_dialog_col_rese( screen, fid_list )

int screen;
int fid_list[];                 

/* local variables */
{
int fid, index;
 
/********************************/

    /* Is there no list to reset ? */                               
    if ( fid_list == NULL )
        /* No, don't reset it. */
        return;

    /* Reinitialize all the fields */
    fid = 0;                      
    while ( fid_list[ fid ] != k_mwm_last_fid )
      {                                                                        
        mwm_dialog_field_init( (*mwm_fid)[ screen ][ fid_list[ fid ]], 
                               fid_list[ fid ] );
        fid++;
      }

}

/*******************************************************************
**
**  Description: Reset the dialog settings.
**
**  Formal Parameters
**    Input: did,
**           field list.
**  
********************************************************************/

void mwm_dialog_rese( did, fid_list )

int did;
int fid_list[];                 

/* local variables */
{
int fid, index, screen;
 
/********************************/

    /* It there a list to reset ? */
    if ( fid_list == NULL )
        /* No, don't reset it. */                                 
        return;

    /* Reinitialize all the fields */
    fid = 0;                      
    screen = mwm_did[ did ].screen;
    while ( fid_list[ fid ] != k_mwm_last_fid )
      {                                                                        
        mwm_dialog_field_init( (*mwm_fid)[ screen ][ fid_list[ fid ]], 
                        fid_list[ fid ] );
        fid++;
      }

}

/*******************************************************************
**
**  Description: Bring up a customize dialog box.
**
**  Formal Parameters
**    Input: widget,
**           dialog id.
**  
********************************************************************/

int mwm_dialog_icon_set( wid, did )

Widget wid;
int did;
        
/* local variables */
{                                         
MrmType classid;
int junk;
Window junk_win;                                            
int index, size;
char *bits;
ClientData *pCD;
int error, up;
PropMwmHints hints;
Arg arg_list[ 8 ];
int n;
          
/********************************/

    /* Is this is an error box ? */
    if ( did == k_mwm_cust_error_did )
        /* Yes, no icon */
        return;

    /* Has the icon been created ? */
    if ( (*mwm_icon)[ WID_SCREEN ][ did ] == (Pixmap)NULL )
      {
        /* Set the icon size */
        /* Large icon ? */
        if (( ACTIVE_PSD->iconImageMaximum.width >= mwm_matte_large_width ) &&
            ( ACTIVE_PSD->iconImageMaximum.height >= mwm_matte_large_height ))
          {
            /* wide ? */
            if ( ACTIVE_PSD->iconImageMaximum.width >= mwm_matte_wide_width )
                /* yes */
                size = mwm_matte_wide_width;
            else size = mwm_matte_large_width;
          }
        /* medium icon ? */
        else if (( ACTIVE_PSD->iconImageMaximum.width >= mwm_matte_width ) &&
                 ( ACTIVE_PSD->iconImageMaximum.height >= mwm_matte_height ))   
            size = mwm_matte_width;
        /* Small icon ! */
        else size = mwm_matte_small_width;
               /* Set the icon */
        switch ( did )
          {
            case k_mwm_cust_matte_did: 
              /* Large icon ? */
              if (( ACTIVE_PSD->iconImageMaximum.width >= mwm_matte_large_width ) &&
                  ( ACTIVE_PSD->iconImageMaximum.height >= mwm_matte_large_height ))
                {
                  /* Is it large or wide ? */
                  if ( ACTIVE_PSD->iconImageMaximum.width >= mwm_matte_wide_width )
                      bits = mwm_matte_wide_bits;
                  else bits = mwm_matte_large_bits;
                }
              /* medium icon ? */
              else if (( ACTIVE_PSD->iconImageMaximum.width >= mwm_matte_width ) &&
                       ( ACTIVE_PSD->iconImageMaximum.height >= mwm_matte_height ))   
                  bits = mwm_matte_bits;
              /* Small icon ! */
              else bits = mwm_matte_small_bits;    
              break;
            case k_mwm_cust_border_did:
              /* Large icon ? */
              if (( ACTIVE_PSD->iconImageMaximum.width >= mwm_border_large_width ) &&
                  ( ACTIVE_PSD->iconImageMaximum.height >= mwm_border_large_height ))
                {
                  /* Is it large or wide ? */
                  if ( ACTIVE_PSD->iconImageMaximum.width >= mwm_border_wide_width )
                      bits = mwm_border_wide_bits;
                  else bits = mwm_border_large_bits;
                } 
              /* medium icon ? */
              else if (( ACTIVE_PSD->iconImageMaximum.width >= mwm_border_width ) &&
                       ( ACTIVE_PSD->iconImageMaximum.height >= mwm_border_height ))   
              bits = mwm_border_bits;
              /* Small icon ! */
              else bits = mwm_border_small_bits;    
              break;
            case k_mwm_cust_icons_did:
            case k_mwm_cust_icon_col_did:
              /* Large icon ? */
              if (( ACTIVE_PSD->iconImageMaximum.width >= mwm_icons_large_width ) &&
                  ( ACTIVE_PSD->iconImageMaximum.height >= mwm_icons_large_height ))
                {
                  /* Is it large or wide ? */
                  if ( ACTIVE_PSD->iconImageMaximum.width >= mwm_icons_wide_width )
                      bits = mwm_icons_wide_bits;
                  else bits = mwm_icons_large_bits;
                }
              /* medium icon ? */
              else if (( ACTIVE_PSD->iconImageMaximum.width >= mwm_icons_width ) &&
                       ( ACTIVE_PSD->iconImageMaximum.height >= mwm_icons_height ))   
                  bits = mwm_icons_bits;
              /* Small icon ! */
              else bits = mwm_icons_small_bits;    
              break;
            case k_mwm_cust_restart_did:
            case k_mwm_cust_apply_did:
            case k_mwm_cust_ws_did:
            case k_mwm_help_did:
            case k_mwm_cust_border_col_did:
            default:
              /* Large icon ? */
              if (( ACTIVE_PSD->iconImageMaximum.width >= mwm_ws_large_width ) &&
                  ( ACTIVE_PSD->iconImageMaximum.height >= mwm_ws_large_height ))
               {
                  /* Is it large or wide ? */
                  if ( ACTIVE_PSD->iconImageMaximum.width >= mwm_ws_wide_width )
                      bits = mwm_ws_wide_bits;
                  else bits = mwm_ws_large_bits;
               }
              /* medium icon ? */
              else if (( ACTIVE_PSD->iconImageMaximum.width >= mwm_ws_width ) &&
                       ( ACTIVE_PSD->iconImageMaximum.height >= mwm_ws_height ))   
                  bits = mwm_ws_bits;
              /* Small icon ! */
              else bits = mwm_ws_small_bits;    
              break;
          }
        /* Create the icons */
        (*mwm_icon)[ WID_SCREEN ][ did ] = 
            XCreateBitmapFromData( 
                DISPLAY, 
                XRootWindow( wmGD.dialog_display, WID_SCREEN_NUM ),
                bits, size, size );
        mwm_set( (Widget)XtParent( wid ), XtNiconPixmap, 
                 (void *)(*mwm_icon)[ WID_SCREEN ][ did ] );
      }

}

/*******************************************************************
**
**  Description: Bring up a customize dialog box.
**
**  Formal Parameters
**    Input: widget,
**           dialog id,
**           parent widget.
**  
********************************************************************/

int mwm_dialog_up( wid, did, parent_wid )     

Widget wid;
int did;
Widget parent_wid;
        
/* local variables */
{                                         
MrmType classid;
int junk;
Window junk_win;                                            
int x, y, is_managed, index, size;
char *bits;
ClientData *pCD;
int error, up;
PropMwmHints hints;
Arg arg_list[ 8 ];
int n;
          
/********************************/
                           
    mwm_watch_set( true );
    up = false;
    /* Is this color mix or help ? */
    if (( did == k_mwm_cust_col_mix_did ) ||
        ( did == k_mwm_help_did ))
        /* Yes, is it up on this screen ? */
        up = (*mwm_screen_did)[ WID_SCREEN ][ did ].up;
    else if ( mwm_did[ did ].up )
      {
        up = true;
        /* Is it up on another screen ? */
        if ( mwm_did[ did ].screen != WID_SCREEN )
          {
            /* Yes, don't bring it up on another screen. */
            return( false );
          }                                            
        }

    /* Is it up  ? */
    if ( up )
      /* Make sure its de-iconized. */
      {
        error = XFindContext( DISPLAY, XtWindow( XtParent( wid )),
                              wmGD.windowContextType, (caddr_t *)&pCD );
        if ( !error )
          {
            /* Make sure that the active screen is set. */
            if ( wmGD.numScreens > 1 )
                SetActiveScreen( pCD->pSD );
            F_Normalize_And_Raise( (String)NULL, pCD, (XEvent *)NULL );
          }
        return( true );
      }

    /* Only set "up" for top dialog boxes.  This needs to
       be set before resetting the dialog box since
       that only works for dialog boxes that are up. */
    if ( did != k_mwm_cust_error_did )
      {
        mwm_did[ did ].up = true;
      }
    if (( did == k_mwm_cust_col_mix_did ) || ( did == k_mwm_help_did ))
        (*mwm_screen_did)[ WID_SCREEN ][ did ].up = true;
        
    if ( !XtIsManaged( wid ) )
      {
        /* Get the current location */
        XQueryPointer( DISPLAY, XRootWindow( DISPLAY, WID_SCREEN_NUM ),
                       &junk_win, &junk_win, 
                       &x, &y, &junk, &junk, (unsigned int *)&junk );
        n = 0;
        XtSetArg( arg_list[ n ], XmNwaitForWm, (XtArgVal)false ); n++;
        XtSetArg( arg_list[ n ], XtNwindowGroup,
                  (void*)XtUnspecifiedWindowGroup ); n++;
        XtSetArg( arg_list[ n ], XtNx, (void *)x ); n++;
        XtSetArg( arg_list[ n ], XtNy, (void *)y ); n++;
        XtSetValues( (Widget)XtParent( wid ), arg_list, n );
        /* Set the icon. */
        mwm_dialog_icon_set( wid, did );
        /* Reset all the fields to their current values. */
        hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_MOVE | MWM_FUNC_MINIMIZE | MWM_FUNC_CLOSE;
        hints.decorations = MWM_DECOR_BORDER | MWM_DECOR_TITLE | MWM_DECOR_MENU |
                            MWM_DECOR_MINIMIZE;
        XChangeProperty( DISPLAY, XtWindow( XtParent( wid )), wmGD.xa_MWM_HINTS,
                         wmGD.xa_MWM_HINTS, 32, PropModeReplace, 
                         (unsigned char *)&hints, PROP_MWM_HINTS_ELEMENTS );
        XtManageChild( wid );
      }
    mwm_watch_set( false );

    return( true );

}    
                                    
/*******************************************************************/
