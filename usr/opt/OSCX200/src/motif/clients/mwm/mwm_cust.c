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
**  Abstract: Window Manager Customize
**
**  Environment: VMS, Unix, Sun
**
*******************************************************************/

#include "WmGlobal.h"
#include "WmResNames.h"
#include "WmResParse.h"
#include "WmFunction.h"
#include "WmInitWs.h"
#include <Xm/Xm.h>
#include <X11/ShellP.h>
#include <X11/Shell.h>
#include <Xm/DialogSP.h>
#include <Xm/MessageB.h>
#include <Xm/ToggleB.h>
#include <Mrm/MrmPublic.h>
#include <stdio.h>

extern char   *getenv ();

#include "mwm_cust.h"
#include "mwm_internal.h"
#include "mwm_cust_internal.h"
#include "mwm_cust_forward.h"
#include "mwm_cust_callbacks.h"
#include "mwm_dialog.h"
#include "mwm_util.h"  
#include "mwm_col.h"
                                                   
/* Initialization module */

/*******************************************************************
**                                                                    
**  Description: Initialize customize structures.
**
**  Formal Parameters
**  
********************************************************************/

int mwm_cust_info_init()     

/* local variables */
{
int argc;
int index;
#ifdef MOTIF_ONE_DOT_ONE
char homedir[maxpath];
#else
char *homedir;
#endif

/********************************/

    /* Inited yet ? */
    if ( mwm_cust_init )
        return( true );

    memset( &mwm_cust_info, 0, sizeof( mwm_cust_info_type ) ); 
    memset( &mwm_cust_info_save, 0, sizeof( mwm_cust_info_type ) ); 
    memset( &mwm_cust_info_last, 0, sizeof( mwm_cust_info_type ) ); 
    /* Open another connection to the display */
    argc = 0;
    wmGD.dialog_display = XtOpenDisplay( wmGD.mwmAppContext, NULL,
                                       k_mwm_dialog_resource_name,
                                       k_mwm_dialog_resource_class,
                                       NULL, 0, &argc, NULL );   
    if ( MrmRegisterNames( mwm_cust_cblist, mwm_cust_cbnum ) != MrmSUCCESS)
      {
            fprintf( stderr, "The customize resources could not be registered.\n" );
            return( false );
      }
    
    if ( !mwm_alloc( (void *)&mwm_dialog_shell, 
                     wmGD.numScreens * sizeof( mwm_dialog_shell ),
                     "Insufficient memory for customize screen shells" ))
        return( false );

    /* Allocate the dialog box information */
    if ( !mwm_alloc( (void *)&mwm_screen_did, sizeof( mwm_did_screen_type ) *
                     wmGD.numScreens * ( k_mwm_max_did ),
                     "Error allocating dialog box screen information" ))
        return( false );
                                    
    /* Allocate the field id information */
    if ( !mwm_alloc( (void *)&mwm_fid, 
                     wmGD.numScreens * sizeof( Widget ) * ( k_mwm_fid ),
                     "Error allocating field id information" ))
        return( false );

    /* Allocate the mix information */
    if ( !mwm_alloc( (void *)&mwm_cust_info.mix,
                     wmGD.numScreens * sizeof( color_mix_type ) *
                     ( k_mwm_color_max ),
                     "Error allocating color mix information" ))
        return( false );

    /* Allocate the mix box information */
    if ( !mwm_alloc( (void *)&mwm_cust_info.mix_box,
                     wmGD.numScreens * sizeof( color_mix_box_type ),
                     "Error allocating color mix box information" ))
        return( false );
    /* Initialize the color index */
    for ( index = 0; index < wmGD.numScreens; index ++ )
        mwm_cust_info.mix_box[ index ].index = k_mwm_none;

    /* Allocate the saved mix information */
    if ( !mwm_alloc( (void *)&mwm_cust_info_save.mix,
                     wmGD.numScreens * sizeof( color_mix_type ) *
                     ( k_mwm_color_max ),
                     "Error allocating saved color mix information" ))
        return( false );

    /* Allocate the last mix information */
    if ( !mwm_alloc( (void *)&mwm_cust_info_last.mix,
                     wmGD.numScreens * sizeof( color_mix_type ) *
                     ( k_mwm_color_max ),
                     "Error allocating last color mix information" ))
        return( false );

    /* Get the resource file names */
    WmInitResUserGet( mwm_def_res_name, &mwm_user_def_res_file );
    /* If the user def res file does not exist, then create it */
    if ( mwm_user_def_res_file == NULL )
      {
#ifdef MOTIF_ONE_DOT_ONE
        GetHomeDirName(homedir);
#else
        homedir = (char *)_XmOSGetHomeDirName();
#endif
        /* No, allocate it */        
        if ( ! mwm_alloc( &mwm_user_def_res_file,
                          strlen( homedir ) + strlen( mwm_def_res_name ) + 2,
                          "Error allocating mwm user resource file" ))
            return( false );                          
        strcpy( mwm_user_def_res_file, homedir );
        strcat( mwm_user_def_res_file, "/" );
        strcat( mwm_user_def_res_file, mwm_def_res_name );
      }
    WmInitResSysGet( mwm_def_res_name, &mwm_sys_def_res_file );
    mwm_cust_init = true;
    return( true );

}

/*******************************************************************/

/* Utility module */

/*******************************************************************
**
**  Description: Get the icon box dimensions.
**
**  Formal Parameters
**    Output: string with dimensions,
**            orientation.
**    Return: true if icon box is used.    
**  
********************************************************************/

int mwm_cust_box_geo_get( string, orient )   
        
char *string;
char *orient;

/* local variables */
{
IconBoxData *icon_box;
int x, y, slots_wide, slots_high;
unsigned int height, width, border, depth;
                               
/********************************/

    if ( ACTIVE_PSD->useIconBox )
      {
        icon_box = ACTIVE_WS->pIconBox;
        XGetGeometry( DISPLAY, (Drawable)XtWindow( icon_box->shellWidget),
                      &ACTIVE_PSD->rootWindow, &x, &y, &width, &height, 
                      &border, &depth );
        slots_wide = ( width / icon_box->IPD.iPlaceW );
        slots_high = ( height / icon_box->IPD.iPlaceH );
        if ( orient != NULL )
          {
            if ( slots_wide >= slots_high )
                strcpy( orient, k_mwm_vert_str );
            else strcpy( orient, k_mwm_horiz_str );
          }
        sprintf( string, "%dx%d+%d+%d", slots_wide, slots_high, 
                 icon_box->pCD_iconBox->frameInfo.x, 
                 icon_box->pCD_iconBox->frameInfo.y );
        return( true );
      }
    return( false );

}

/*******************************************************************
**
**  Description: For an option menu, with a text field.
**               Reset the text field based on whether none is set.
**
**  Formal Parameters
**    Input: widget,
**           new field,
**           old field.
**  
********************************************************************/

void mwm_cust_size_none( wid, new_fid, current_fid )

Widget wid;
int new_fid;
int current_fid;

/* local variables */
{
int did;

/********************************/

    /* Did the value change from other to something else ? */
    if ((( new_fid == k_mwm_matte_none_fid ) && ( current_fid != new_fid )) ||
        (( new_fid != k_mwm_matte_none_fid ) && 
         ( current_fid == k_mwm_matte_none_fid )))
      /* Yes, the field changed. */
      {
        XtSetSensitive( (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_fore_label_fid ], 
                        (Boolean)(new_fid != k_mwm_matte_none_fid) );
        XtSetSensitive( (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_back_label_fid ], 
                        (Boolean)(new_fid != k_mwm_matte_none_fid) );
        XtSetSensitive( (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_top_label_fid ], 
                        (Boolean)(new_fid != k_mwm_matte_none_fid ));
        XtSetSensitive( (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_bot_label_fid ], 
                        (Boolean)(new_fid != k_mwm_matte_none_fid ));
        XtSetSensitive( (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_fore_fid ], 
                        (Boolean)(new_fid != k_mwm_matte_none_fid ));
        XtSetSensitive( (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_back_fid ], 
                        (Boolean)(new_fid != k_mwm_matte_none_fid ));
        XtSetSensitive( (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_top_fid ], 
                        (Boolean)(new_fid != k_mwm_matte_none_fid ));
        XtSetSensitive( (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_bot_fid ], 
                        (Boolean)(new_fid != k_mwm_matte_none_fid ));
        XtSetSensitive( (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_auto_shade_fid ], 
                        (Boolean)(new_fid != k_mwm_matte_none_fid ));
        /* Is matte none? */
        if ( new_fid != k_mwm_matte_none_fid )
            /* No, set the auto color fields */
            mwm_col_auto_chec( wid, k_mwm_matte_auto_shade_fid );
        /* remove the color mix dialog box if none */
        else
          {
            did = k_mwm_cust_col_mix_did;
            mwm_dialog_cancel( wid, &did, NULL );
          }
        
      }                 
                                   
}

/*******************************************************************
**
**  Description: For an option menu, with a text field.
**               Reset the text field based on whether other is set.
**
**  Formal Parameters   
**    Input: new field,
**           other field id,
**           old field,
**           label widget,
**           text widget,
**           size,
**    I/O:   size string.
**  
********************************************************************/

void mwm_cust_size_other( new_fid, other_fid, current_fid, label_wid, text_wid,
                          value, size )

int new_fid;
int other_fid;          
int current_fid;
Widget label_wid;
Widget text_wid;
int value;
char *size;

/* local variables */
{
char *string;

/********************************/

    /* Did the value change from other to something else
       or is it a new field ? */
    if (((( new_fid == other_fid ) && ( current_fid != new_fid )) ||
         (( new_fid != other_fid ) && ( current_fid == other_fid ))) ||
         ( current_fid == k_mwm_none ))
      /* Yes, the field changed. */
      {
        /* Is it now other ? */
        if ( new_fid != other_fid )
          /* No, save the old field and clear the field. */
          {
            /* Is the text field sensitive ? */
            if ( XtIsSensitive( text_wid ))
              {
                string = (char *)XmTextGetString( text_wid );
                strcpy( size, string );
                XtFree( string );
                XmTextSetString( text_wid, k_mwm_blank_str );
              }
          }

        XtSetSensitive( label_wid, (Boolean)(new_fid == other_fid) );
        XtSetSensitive( text_wid, (Boolean)(new_fid == other_fid) );
  
        /* Is it now other ? */
        if ( new_fid == other_fid )
          {
            /* Was it set to anything ? */
            if ( strlen( size ) == 0 )
                /* Use the default */
                sprintf( size, "%d", value );
            /* Convert the value to a string */
            XmTextSetString( text_wid, size );
          }
      }

}

/*******************************************************************/

/* Get module */

/*******************************************************************
**
**  Description: Get the workspace resources.
**
**  Formal Parameters
**    Input: database.
**  
********************************************************************/

int mwm_cust_ws_get( database )

XrmDatabase database;

/* local variables */
{
char string[ 256 ];
                               
/********************************/

    /* Has it already been initialized ? */
    if ( mwm_cust_info.init[ k_mwm_cust_ws_did ] )
        return( false );

    /* Keyboard focus policy */
    mwm_res_get( ACTIVE_PSD, database, WmNkeyboardFocusPolicy, k_mwm_res_string,
                 string, k_mwm_explicit_str );
    mwm_cust_info.implicit_focus = mwm_str_eq( string, k_mwm_implicit_str );
 
    /* Auto focus ? */
    mwm_res_get( ACTIVE_PSD, database, WmNfocusAutoRaise, k_mwm_res_boolean, 
                 &mwm_cust_info.focus_raised, (void *)true );
    /* deicon focus ? */      
    mwm_res_get( ACTIVE_PSD, database, WmNdeiconifyKeyFocus, k_mwm_res_boolean,
                 &mwm_cust_info.focus_deicon, (void *)true );  
    /* startup focus ? */                
    mwm_res_get( ACTIVE_PSD, database, WmNstartupKeyFocus, k_mwm_res_boolean,
                 &mwm_cust_info.focus_start, (void *)true );   
    /* removed focus ? */
    mwm_res_get( ACTIVE_PSD, database, WmNautoKeyFocus, k_mwm_res_boolean,
                 &mwm_cust_info.focus_removed, (void *)true );
    /* alt space ? */
    mwm_res_get( ACTIVE_PSD, database, WmNforceAltSpace, k_mwm_res_boolean,
                 &mwm_cust_info.force_alt_space, (void *)false );
    /* move and size feedback ? */
    mwm_res_get( ACTIVE_PSD, database, WmNshowFeedback, k_mwm_res_string,
                 string, k_mwm_all_str );
    /* No feedback ? */
    if ( !mwm_str_eq( string, k_mwm_none_str ))
      /* There is some feedback. */
      {
        /* Is everything set ? */
        if ( mwm_str_eq( string, k_mwm_all_str ))
          /* Yup. */
          {                     
            mwm_cust_info.fb.behavior = true;
            mwm_cust_info.fb.kill = true;
            mwm_cust_info.fb.move = true;
            mwm_cust_info.fb.resize = true;
            mwm_cust_info.fb.quit = true;
            mwm_cust_info.fb.restart = true;
            mwm_cust_info.fb.placement = true;
          }
        /* Are they individually or implicitly set ? */
        else
          {
            mwm_cust_info.fb.kill = mwm_str_find( string, k_mwm_kill_str );
            mwm_cust_info.fb.move = mwm_str_find( string, k_mwm_move_str );
            mwm_cust_info.fb.resize = mwm_str_find( string, k_mwm_resize_str );
            mwm_cust_info.fb.quit = mwm_str_find( string, k_mwm_quit_str );
            mwm_cust_info.fb.restart = mwm_str_find( string, k_mwm_restart_str );
            mwm_cust_info.fb.placement = mwm_str_find( string, k_mwm_placement_str );
            mwm_cust_info.fb.behavior = mwm_str_find( string, k_mwm_behavior_str );
          }
      }
    /* Move opaque */
    mwm_res_get( ACTIVE_PSD, database, WmNmoveOpaque, k_mwm_res_boolean,
                 &mwm_cust_info.move_window, (void *)false );
    /* Intl mod keys */
    mwm_res_get( ACTIVE_PSD, database, WmNignoreModKeys, k_mwm_res_boolean,
                 &mwm_cust_info.ignore_mods, (void *)false );
    return( true );

}

/*******************************************************************
**
**  Description: Get the border resources.
**
**  Formal Parameters
**    Input: database.
**  
********************************************************************/

int mwm_cust_border_get( database )

XrmDatabase database;

/* local variables */
{
char string[ 256 ];
int fid;
                               
/********************************/

    /* Has it already been initialized ? */
    if ( mwm_cust_info.init[ k_mwm_cust_border_did ] )
        return( false );

    /* Border Decoration */
    strcpy( string, k_mwm_all_str );
    mwm_res_get( ACTIVE_PSD, database, WmNclientDecoration, k_mwm_res_string,
                      string, k_mwm_all_str );

    /* No border decorations ? */
    if ( !mwm_str_eq( string, k_mwm_none_str ))
      /* There are some border decorations. */
      {
        /* Are they all explicitly or implicitly set ? */
        if ( mwm_str_eq( string, k_mwm_all_str ))
          /* Yup. */
          {
            mwm_cust_info.min = true;
            mwm_cust_info.max = true;
            mwm_cust_info.title = true;
            mwm_cust_info.border = true;
            mwm_cust_info.menu = true;
            mwm_cust_info.resize = true;
          }
        /* Are they individually or implicitly set ? */
        else
          {
            mwm_cust_info.max = mwm_str_find( string, k_mwm_max_str );
            mwm_cust_info.min = mwm_str_find( string, k_mwm_min_str );      
            mwm_cust_info.resize = mwm_str_find( string, k_mwm_resizeh_str );
            mwm_cust_info.title = mwm_str_find( string, k_mwm_title_str );
            mwm_cust_info.border = mwm_str_find( string, k_mwm_border_str );
            mwm_cust_info.menu = mwm_str_find( string, k_mwm_menu_str );
          }
      }
                                     
    /* Menu click2 */
    mwm_res_get( ACTIVE_PSD, database, WmNsystemButtonClick2, k_mwm_res_boolean,
                      &mwm_cust_info.wmenu_dclick, (void *)false );            
    /* Border width */
    mwm_res_get( ACTIVE_PSD, database, WmNresizeBorderWidth, k_mwm_res_int,
                      &mwm_cust_info.border_size, (void *)(k_mwm_border_medium_fid) );
    /* Save the border size */
    switch ( mwm_cust_info.border_size )
      {
        case k_mwm_border_small_size:          
          mwm_cust_info.border_size_fid = k_mwm_border_small_fid;
          break;
        case k_mwm_border_medium_size:          
          mwm_cust_info.border_size_fid = k_mwm_border_medium_fid;
          break;
        case k_mwm_border_large_size:          
          mwm_cust_info.border_size_fid = k_mwm_border_large_fid;
          break;
        default:
          mwm_cust_info.border_size_fid = k_mwm_border_other_fid;
          break;
      }

    return( true );

}

/*******************************************************************
**
**  Description: Get the border color resources.
**
**  Formal Parameters
**    Input: database.
**  
********************************************************************/

int mwm_cust_border_col_get( database )

XrmDatabase database;

/* local variables */
{
int fid, index, value;
                               
/********************************/

    /* Has it already been initialized ? */
    if ( mwm_cust_info.init[ k_mwm_cust_border_col_did ] )
        return( false );

    /* Border colors */
    mwm_res_get( ACTIVE_PSD, database, WmNforeground, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_inactive_fore_x ], 0 );
    mwm_res_get( ACTIVE_PSD, database, WmNbackground, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_inactive_back_x ], 0 );
    mwm_res_get( ACTIVE_PSD, database, WmNtopShadowColor, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_inactive_top_x ], 0 );
    mwm_res_get( ACTIVE_PSD, database, WmNbottomShadowColor, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_inactive_bot_x ], 0 );
    /* Autoshade */
    if ( !mwm_res_get( ACTIVE_PSD, database, k_mwm_inactive_auto_shade_str, k_mwm_res_boolean, 
                            &mwm_cust_info.inactive_auto_shade, (void *)false )) 
        mwm_cust_info.inactive_auto_shade = false;

    /* Active border colors */
    mwm_res_get( ACTIVE_PSD, database, WmNactiveForeground, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_active_fore_x ], 0 );
    mwm_res_get( ACTIVE_PSD, database, WmNactiveBackground, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_active_back_x ], 0 );
    mwm_res_get( ACTIVE_PSD, database, WmNactiveTopShadowColor, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_active_top_x ], 0 );
    mwm_res_get( ACTIVE_PSD, database, WmNactiveBottomShadowColor, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_active_bot_x ], 0 );
    /* Autoshade */
    if ( !mwm_res_get( ACTIVE_PSD, database, k_mwm_active_auto_shade_str, k_mwm_res_boolean, 
                            &mwm_cust_info.active_auto_shade, (void *)false ))
        mwm_cust_info.active_auto_shade = false;

    /* Get the menu colors */
    mwm_res_get( ACTIVE_PSD, database, WmNmatchMenuColors, k_mwm_res_int,
                 &value, (void *)k_mwm_menu_col_active );
    /* Save the fid. */
    switch ( value ) 
      {
        case k_mwm_menu_col_active:
          mwm_cust_info.menu_col = k_mwm_menu_col_active_fid;
          break;
        case k_mwm_menu_col_inactive:
          mwm_cust_info.menu_col = k_mwm_menu_col_inactive_fid;
          break;
        default:
        case k_mwm_menu_col_none:
          mwm_cust_info.menu_col = k_mwm_menu_col_none_fid;
          break;
      }

    return( true );

}

/*******************************************************************
**
**  Description: Get the icon resources.
**
**  Formal Parameters
**    Input: database.
**  
********************************************************************/

int mwm_cust_icons_get( database )

XrmDatabase database;

/* local variables */
{
char string[ 256 ];
int fid;
char *result;
                               
/********************************/

    /* Has it already been initialized ? */
    if ( mwm_cust_info.init[ k_mwm_cust_icons_did ] )
        return( false );

    /* Use icon box ? */
    mwm_res_get( ACTIVE_PSD, database, WmNuseIconBox, k_mwm_res_boolean,
                      &mwm_cust_info.use_icon_box, (void *)false );

    /* Icon Decoration */
    mwm_res_get( ACTIVE_PSD, database, WmNiconDecoration, k_mwm_res_string,
                      string, k_mwm_all_str );
 
    /* No icon decorations ? */
    if ( !mwm_str_eq( string, k_mwm_none_str ))
      /* There are some icon decorations. */
      {
        /* Are they all explicitly or implicitly set ? */
        if ( mwm_str_eq( string, k_mwm_all_str ))
          /* Yup. */
          {
            mwm_cust_info.icon_image = true;
            mwm_cust_info.icon_label = true;
            mwm_cust_info.active_icon = true;
          }
        /* Are they all individually or implicitly set ? */
        else
          {
            mwm_cust_info.icon_image = mwm_str_find( string, k_mwm_icon_image_str );
            mwm_cust_info.icon_label = mwm_str_find( string, k_mwm_icon_label_str );
            mwm_cust_info.active_icon  = mwm_str_find( string, k_mwm_active_icon_str );
          }
      }

    /* Icon Placement */
    mwm_res_get( ACTIVE_PSD, database, WmNiconPlacement, k_mwm_res_string,
                      string, k_mwm_bot_right_str );
  
    if ( mwm_str_find( string, k_mwm_left_top_str ))
        mwm_cust_info.icon_placement = k_mwm_icon_left_top_fid;
    else if ( mwm_str_find( string, k_mwm_right_top_str ))
        mwm_cust_info.icon_placement = k_mwm_icon_right_top_fid;
    else if ( mwm_str_find( string, k_mwm_left_bot_str ))
        mwm_cust_info.icon_placement = k_mwm_icon_left_bot_fid;
    else if ( mwm_str_find( string, k_mwm_right_bot_str ))
        mwm_cust_info.icon_placement = k_mwm_icon_right_bot_fid;
    else if ( mwm_str_find( string, k_mwm_top_right_str ))
        mwm_cust_info.icon_placement = k_mwm_icon_top_right_fid;
    else if ( mwm_str_find( string, k_mwm_top_left_str ))
        mwm_cust_info.icon_placement = k_mwm_icon_top_left_fid;
    else if ( mwm_str_find( string, k_mwm_bot_right_str ))
        mwm_cust_info.icon_placement = k_mwm_icon_bot_right_fid;
    else if ( mwm_str_find( string, k_mwm_bot_left_str ))
        mwm_cust_info.icon_placement = k_mwm_icon_bot_left_fid;
    else mwm_cust_info.icon_placement = k_mwm_icon_bot_right_fid;

    /* Icon size */
    mwm_res_get( ACTIVE_PSD, database, WmNiconImageMaximum, k_mwm_res_string,
                      string, "");               
    result = mwm_str_get( string, "x" );
    if (( result == NULL ) || ( strlen( result ) == 0 ))
      {
        /* Set to the default */
        mwm_cust_info.icon_width = mwm_cust_info.icon_height =
          k_mwm_icon_medium_size;
        mwm_cust_info.icon_size_fid = k_mwm_icon_medium_fid;
      }
    else
      {
        /* Extract the width and height from Width x height */
        result++;               
        mwm_cust_info.icon_height = atol( result );
        string[ result - string - 1 ] = '\0';       
        mwm_cust_info.icon_width = atol( string );

        /* Set the field id based on the width */
        switch ( mwm_cust_info.icon_width )
          {
            case k_mwm_icon_small_size:          
              mwm_cust_info.icon_size_fid = k_mwm_icon_small_fid;
              break;
            case k_mwm_icon_medium_size:
              mwm_cust_info.icon_size_fid = k_mwm_icon_medium_fid;
              break;
            case k_mwm_icon_large_size:
              mwm_cust_info.icon_size_fid = k_mwm_icon_large_fid;
              break;
            case k_mwm_icon_wide_width:
              /* It's wide, is the height correct ? */
              if ( mwm_cust_info.icon_height == k_mwm_icon_wide_height )
                  /* Yes */
                  mwm_cust_info.icon_size_fid = k_mwm_icon_wide_fid;
              /* No, set to other */
              else mwm_cust_info.icon_size_fid = k_mwm_icon_other_fid;
              break;
            default:
              mwm_cust_info.icon_size_fid = k_mwm_icon_other_fid;
              break;
          }

        /* Make sure small, medium, and other height = width */
        switch ( mwm_cust_info.icon_width )     
          {
            case k_mwm_icon_small_size:          
            case k_mwm_icon_medium_size:
            case k_mwm_icon_large_size:
              if ( mwm_cust_info.icon_width != mwm_cust_info.icon_height )
                mwm_cust_info.icon_size_fid = k_mwm_icon_other_fid;
            default:
              break;
          }
      }

    return( true );

}

/*******************************************************************
**
**  Description: Get the icon color resources.
**
**  Formal Parameters
**    Input: database.
**  
********************************************************************/

int mwm_cust_icon_col_get( database )

XrmDatabase database;
                      
/* local variables */
{
int fid, index;
char *result;
                               
/********************************/

    /* Has it already been initialized ? */
    if ( mwm_cust_info.init[ k_mwm_cust_icon_col_did ] )
        return( false );

    /* Icon colors */
    mwm_res_get( ACTIVE_PSD, database, WmNiconImageForeground, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_icon_image_fore_x ], 0 );
    mwm_res_get( ACTIVE_PSD, database, WmNiconImageBackground, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_icon_image_back_x ], 0 );
    mwm_res_get( ACTIVE_PSD, database, WmNiconImageTopShadowColor, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_icon_image_top_x ], 0 );
    mwm_res_get( ACTIVE_PSD, database, WmNiconImageBottomShadowColor, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_icon_image_bot_x ], 0 );
    /* Autoshade */
    if ( !mwm_res_get( ACTIVE_PSD, database, k_mwm_image_auto_shade_str, k_mwm_res_boolean, 
                            &mwm_cust_info.image_auto_shade, (void *)false ))
        mwm_cust_info.image_auto_shade = false;

    /* Icon box colors */
    mwm_cust_info.icon_box_auto_shade =
      !mwm_res_get( ACTIVE_PSD, database, WmNiconBoxHBackground, k_mwm_res_col, 
                         &mwm_cust_info.col[ k_mwm_icon_box_back_x ], 0 );

    /* Is auto shade set ? */
    if ( mwm_cust_info.icon_box_auto_shade )   
      /* Get the derived color from the widget */
      {
        /* Is there an icon box ? */

        if ( ACTIVE_PSD->useIconBox && ACTIVE_WS->pIconBox )
          /* Yes, get the color */
          {
            mwm_get( ACTIVE_WS->pIconBox->hScrollBar, XmNtroughColor, 
                     &mwm_cust_info.col[ k_mwm_icon_box_back_x ] );
            /* Get the color */
            XQueryColor( wmGD.dialog_display, ACTIVE_PSD->lastInstalledColormap,
                         (void *)&mwm_cust_info.col[ k_mwm_icon_box_back_x ].xcol );
          }
      }

    return( true );

}

/*******************************************************************
**
**  Description: Get the matte resources.
**
**  Formal Parameters
**    Input: database.
**  
********************************************************************/

int mwm_cust_matte_get( database )

XrmDatabase database;

/* local variables */
{
char string[ 256 ];
int fid, index;
                               
/********************************/

    /* Has it already been initialized ? */
    if ( mwm_cust_info.init[ k_mwm_cust_matte_did ] )
        return( false );

    /* Matte width */
    mwm_res_get( ACTIVE_PSD, database, WmNmatteWidth, k_mwm_res_int,
                      &mwm_cust_info.matte_size, k_mwm_matte_none_size );
    /* Save the matte size */
    switch ( mwm_cust_info.matte_size )
      {
        case k_mwm_matte_small_size:          
          mwm_cust_info.matte_size_fid = k_mwm_matte_small_fid;
          break;
        case k_mwm_matte_medium_size:          
          mwm_cust_info.matte_size_fid = k_mwm_matte_medium_fid;
          break;
        case k_mwm_matte_large_size:          
          mwm_cust_info.matte_size_fid = k_mwm_matte_large_fid;
          break;
        case k_mwm_matte_none_size:          
          mwm_cust_info.matte_size_fid = k_mwm_matte_none_fid;
          break;
        default:
          mwm_cust_info.matte_size_fid = k_mwm_matte_other_fid;
          break;
      }
                                          
    mwm_res_get( ACTIVE_PSD, database, WmNmatteForeground, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_matte_fore_x ], 0 );
    mwm_res_get( ACTIVE_PSD, database, WmNmatteBackground, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_matte_back_x ], 0 );
    mwm_res_get( ACTIVE_PSD, database, WmNmatteTopShadowColor, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_matte_top_x ], 0 );
    mwm_res_get( ACTIVE_PSD, database, WmNmatteBottomShadowColor, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_matte_bot_x ], 0 );
    /* Autoshade */
    if ( !mwm_res_get( ACTIVE_PSD, database, k_mwm_matte_auto_shade_str, k_mwm_res_boolean, 
                            &mwm_cust_info.matte_auto_shade, false ))
        mwm_cust_info.matte_auto_shade = false;

    return( true );

}

/*******************************************************************
**
**  Description: Get the resources.                         
**
**  Formal Parameters
**    Input: dialog id.
**                      
********************************************************************/
                        
void mwm_cust_get( did ) 

int did;
                      
/* local variables */
{                
XrmDatabase database;
                               
/********************************/

    database = XtDatabase( DISPLAY );
    if ( database == NULL )
        fprintf( stderr, "mwm: Opening %s \n", mwm_user_def_res_file );

    /* Get the information */
    switch ( did )
      {
        case k_mwm_cust_ws_did: 
          if ( mwm_cust_ws_get( database ))
            {
              mwm_cust_ws_copy( &mwm_cust_info_save, &mwm_cust_info );
              mwm_cust_ws_copy( &mwm_cust_info_last, &mwm_cust_info );
            }
          break;
        case k_mwm_cust_border_did: 
          if ( mwm_cust_border_get( database ))
            {
              mwm_cust_border_copy( &mwm_cust_info_save, &mwm_cust_info );
              mwm_cust_border_copy( &mwm_cust_info_last, &mwm_cust_info );
            }  
          break;
        case k_mwm_cust_icons_did: 
          if ( mwm_cust_icons_get( database ))
            {
              mwm_cust_icons_copy( &mwm_cust_info_save, &mwm_cust_info );
              mwm_cust_icons_copy( &mwm_cust_info_last, &mwm_cust_info );
            }
          break;
        case k_mwm_cust_border_col_did: 
          if ( mwm_cust_border_col_get( database ))
            {
              mwm_cust_border_col_copy( &mwm_cust_info_save, &mwm_cust_info );
              mwm_cust_border_col_copy( &mwm_cust_info_last, &mwm_cust_info );
            }
          break;
        case k_mwm_cust_icon_col_did: 
          if ( mwm_cust_icon_col_get( database ))
            {
              mwm_cust_icon_col_copy( &mwm_cust_info_save, &mwm_cust_info );
              mwm_cust_icon_col_copy( &mwm_cust_info_last, &mwm_cust_info );
            }
          break;
        case k_mwm_cust_matte_did: 
          if ( mwm_cust_matte_get( database ))
            {
              mwm_cust_matte_copy( &mwm_cust_info_save, &mwm_cust_info );
              mwm_cust_matte_copy( &mwm_cust_info_last, &mwm_cust_info );
            }
          break;
        default: return;
      }
    mwm_cust_info.init[ did ] = true;
                        
}

/*******************************************************************/

/* Copy module */

/*******************************************************************
**
**  Description: Copy a color.
**
**  Formal Parameters
**    Input: current info,
**           saved info,
**           index.
**  
********************************************************************/

void mwm_cust_col_copy( info, info_save, index )

mwm_cust_info_type *info;
mwm_cust_info_type *info_save;
int index;

/* local variables */
{
                               
/********************************/

    /* Copy the rgb value */
    info->col[ index ].xcol = info_save->col[ index ].xcol;
    /* Is there a named color ? */
    if ( info_save->col[ index ].name != NULL )
      /* Yup */
      {
        /* Is there a destination ? */
        if ( info->col[ index ].name == NULL )
          {
            /* No, allocate the color name field */        
            if ( ! mwm_alloc( &info->col[ index ].name, 
                              k_mwm_max_color_name_size,          
                              "Error allocating color name data" ))
                return;
          }
        strcpy( info->col[ index ].name, info_save->col[ index ].name );
      }
      
}

/*******************************************************************
**
**  Description: Copy the workspace resources.
**
**  Formal Parameters
**    Input: current info,
**           saved info.
**  
********************************************************************/

void mwm_cust_ws_copy( info, info_save )

mwm_cust_info_type *info;
mwm_cust_info_type *info_save;

/* local variables */
{
                               
/********************************/

    /* Copy the current information for workspace */
    info->implicit_focus = info_save->implicit_focus;
    info->focus_raised = info_save->focus_raised;
    info->focus_deicon = info_save->focus_deicon;
    info->focus_start = info_save->focus_start;
    info->focus_removed = info_save->focus_removed;
    info->force_alt_space = info_save->force_alt_space;
    info->ignore_mods = info_save->ignore_mods;
    info->move_window = info_save->move_window;
    memcpy( &info->fb, &info_save->fb, sizeof( info->fb ));

}

/*******************************************************************
**
**  Description: Copy the border resources.
**
**  Formal Parameters
**    Input: current info,
**           saved info.
**  
********************************************************************/

void mwm_cust_border_copy( info, info_save )

mwm_cust_info_type *info;
mwm_cust_info_type *info_save;

/* local variables */
{
                                
/********************************/

    info->max = info_save->max;
    info->min = info_save->min;
    info->resize = info_save->resize;
    info->title = info_save->title;
    info->border = info_save->border;
    info->menu = info_save->menu;
    info->wmenu_dclick = info_save->wmenu_dclick;
    info->border_size = info_save->border_size;
    info->border_size_fid = info_save->border_size_fid;
    strcpy( info->border_size_str, info_save->border_size_str);

}

/*******************************************************************
**
**  Description: Copy the border color resources.
**
**  Formal Parameters
**    Input: current info,
**           saved info.
**  
********************************************************************/

void mwm_cust_border_col_copy( info, info_save )

mwm_cust_info_type *info;
mwm_cust_info_type *info_save;

/* local variables */
{
int index;

/********************************/
                                                                      
    /* Copy the current settings */
    for ( index = k_mwm_active_fore_x; index <= k_mwm_inactive_bot_x; index++ )
      {
        mwm_cust_col_copy( info, info_save, index );
      }          
    info->active_auto_shade = info_save->active_auto_shade;
    info->inactive_auto_shade = info_save->inactive_auto_shade;
    info->menu_col = info_save->menu_col;

}

/*******************************************************************
**
**  Description: Copy the icon resources.
**
**  Formal Parameters
**    Input: current info,
**           saved info.
**  
********************************************************************/
                                      
void mwm_cust_icons_copy( info, info_save )

mwm_cust_info_type *info;
mwm_cust_info_type *info_save;

/* local variables */
{

/********************************/

    /* Copy the current information */
    info->use_icon_box = info_save->use_icon_box;
    info->icon_image = info_save->icon_image;
    info->icon_label = info_save->icon_label;
    info->active_icon = info_save->active_icon;
    info->icon_placement = info_save->icon_placement;
    info->icon_width = info_save->icon_width;
    info->icon_size_fid = info_save->icon_size_fid;
    strcpy( info->icon_height_str, info_save->icon_height_str);
    strcpy( info->icon_width_str, info_save->icon_width_str);
    
}

/*******************************************************************
**
**  Description: Copy the icon color resources.
**
**  Formal Parameters
**    Input: current info,
**           saved info.
**  
********************************************************************/

void mwm_cust_icon_col_copy( info, info_save )

mwm_cust_info_type *info;
mwm_cust_info_type *info_save;

/* local variables */
{
int index;
                               
/********************************/

    /* Copy the current settings */
    for ( index = k_mwm_icon_image_fore_x; index <= k_mwm_icon_image_bot_x; index++ )
      {
        mwm_cust_col_copy( info, info_save, index );
      }
    info->col[ k_mwm_icon_box_back_x ] =
      info_save->col[ k_mwm_icon_box_back_x ];
    info->image_auto_shade = info_save->image_auto_shade;
    info->icon_box_auto_shade = info_save->icon_box_auto_shade;

}

/*******************************************************************
**
**  Description: Copy the matte resources.
**
**  Formal Parameters
**    Input: current info,
**           saved info.
**  
********************************************************************/

void mwm_cust_matte_copy( info, info_save )

mwm_cust_info_type *info;
mwm_cust_info_type *info_save;

/* local variables */
{
int index;
                               
/********************************/

    /* Copy the current settings */
    for ( index = k_mwm_matte_fore_x; index <= k_mwm_matte_bot_x; index++ )
      {
        mwm_cust_col_copy( info, info_save, index );
      }
    info->matte_auto_shade = info_save->matte_auto_shade;
    info->matte_size = info_save->matte_size;      
    info->matte_size_fid = info_save->matte_size_fid;
    strcpy( info->matte_size_str, info_save->matte_size_str);

}

/*******************************************************************/

/* Save module */

/*******************************************************************
**
**  Description: Save all the current settings.
**
**  Formal Parameters
**    Input: database.
**  
********************************************************************/

void mwm_cust_ws_save( database )

XrmDatabase *database;   

/* local variables */
{               
char string[ 256 ];
                               
/********************************/

    if ( !mwm_cust_info.mod[ k_mwm_cust_ws_did ] )
        return;

    /* Focus policy */
    if ( mwm_cust_info.implicit_focus )
        strcpy( string, k_mwm_pointer_str );        
    else strcpy( string, k_mwm_explicit_str );
    /* Keyboard focus policy */
    mwm_res_set( database, WmNkeyboardFocusPolicy, k_mwm_res_string,
                 string );

    /* Auto focus ? */
    mwm_res_set( database, WmNfocusAutoRaise, k_mwm_res_boolean, 
                 &mwm_cust_info.focus_raised );
    /* deicon focus ? */      
    mwm_res_set( database, WmNdeiconifyKeyFocus, k_mwm_res_boolean,
                 &mwm_cust_info.focus_deicon );
    /* startup focus ? */
    mwm_res_set( database, WmNstartupKeyFocus, k_mwm_res_boolean,
                 &mwm_cust_info.focus_start );
    /* removed focus ? */
    mwm_res_set( database, WmNautoKeyFocus, k_mwm_res_boolean,
                 &mwm_cust_info.focus_removed );
    /* force alt space ? */
    mwm_res_set( database, WmNforceAltSpace, k_mwm_res_boolean,
                 &mwm_cust_info.force_alt_space );
    /* ignore mod keys ? */
    mwm_res_set( database, WmNignoreModKeys, k_mwm_res_boolean,
                 &mwm_cust_info.ignore_mods );
    /* move window ? */
    mwm_res_set( database, WmNmoveOpaque, k_mwm_res_boolean,
                 &mwm_cust_info.move_window );

    /* Feedback */
    strcpy( string, "" );
    if ( mwm_cust_info.fb.behavior )
      {                            
        strcat( string, k_mwm_behavior_str );
        strcat( string, k_mwm_space_str );
      }
    if ( mwm_cust_info.fb.move )
      {                            
        strcat( string, k_mwm_move_str );
        strcat( string, k_mwm_space_str );
      }
    if ( mwm_cust_info.fb.resize )
      {                            
        strcat( string, k_mwm_resize_str );
        strcat( string, k_mwm_space_str );
      }
    if ( mwm_cust_info.fb.placement )
      {                            
        strcat( string, k_mwm_placement_str );
        strcat( string, k_mwm_space_str );
      }
    if ( mwm_cust_info.fb.quit )
      {                            
        strcat( string, k_mwm_quit_str );
        strcat( string, k_mwm_space_str );
      }
    if ( mwm_cust_info.fb.restart )
      {                            
        strcat( string, k_mwm_restart_str );
        strcat( string, k_mwm_space_str );
      }
    if ( mwm_cust_info.fb.kill )
      {                            
        strcat( string, k_mwm_kill_str );
        strcat( string, k_mwm_space_str );
      }

    /* Feedback */
    mwm_res_set( database, WmNshowFeedback, k_mwm_res_string,
                 string );            

 
}

/*******************************************************************
**
**  Description: Save all the current settings.
**
**  Formal Parameters
**    Input: database.
**  
********************************************************************/

void mwm_cust_border_save( database )

XrmDatabase *database;

/* local variables */
{
int size;
char string[ 256 ];
                               
/********************************/

    if ( !mwm_cust_info.mod[ k_mwm_cust_border_did ] )
        return;

    /* Save the border size */
    switch ( mwm_cust_info.border_size_fid )
      {
        case k_mwm_border_small_fid:          
          size = k_mwm_border_small_size;
          break;
        case k_mwm_border_medium_fid:          
          size = k_mwm_border_medium_size;
          break;
        case k_mwm_border_large_fid:          
          size = k_mwm_border_large_size;
          break;
        default:
          size = mwm_cust_info.border_size;
          break;
      }
                                          
    /* Border width */
    mwm_res_set( database, WmNresizeBorderWidth, k_mwm_res_int, &size );
    
    strcpy( string, "" );
    if ( mwm_cust_info.min )
      {
        strcat( string, k_mwm_min_str );
        strcat( string, k_mwm_space_str );
      }
    if ( mwm_cust_info.max )
      {
        strcat( string, k_mwm_max_str );
        strcat( string, k_mwm_space_str );
      }
    if ( mwm_cust_info.title )
      {
        strcat( string, k_mwm_title_str );
        strcat( string, k_mwm_space_str );
      }
    if ( mwm_cust_info.border )
      {
        strcat( string, k_mwm_border_str );
        strcat( string, k_mwm_space_str );
      }
    if ( mwm_cust_info.resize )
      {
        strcat( string, k_mwm_resizeh_str );
        strcat( string, k_mwm_space_str );
      }
    if ( mwm_cust_info.menu )
        strcat( string, k_mwm_menu_str );

    /* None ? */
    if ( strlen( string ) == 0 )
        strcat( string, k_mwm_none_str );
    
    /* Border Decorations */
    mwm_res_set( database, WmNclientDecoration, k_mwm_res_string,
                      string );            

    /* Menu click2 */
    mwm_res_set( database, WmNsystemButtonClick2, k_mwm_res_boolean,
                      &mwm_cust_info.wmenu_dclick );            

}

/*******************************************************************
**
**  Description: Save the border colors.
**
**  Formal Parameters
**    Input: database.                     
**  
********************************************************************/

void mwm_cust_border_col_save( database )

XrmDatabase *database;

/* local variables */
{ 
int value, fore_x, back_x, top_x, bot_x;
                               
/********************************/

    if ( !mwm_cust_info.mod[ k_mwm_cust_border_col_did ] )
        return;

    /* Inactive colors */
    mwm_res_set( database, WmNforeground, k_mwm_res_col, 
                 &mwm_cust_info.col[ k_mwm_inactive_fore_x ] );
    mwm_res_set( database, WmNbackground, k_mwm_res_col, 
                 &mwm_cust_info.col[ k_mwm_inactive_back_x ] );
    mwm_res_set( database, WmNtopShadowColor, k_mwm_res_col, 
                 &mwm_cust_info.col[ k_mwm_inactive_top_x ] );
    mwm_res_set( database, WmNbottomShadowColor, k_mwm_res_col, 
                 &mwm_cust_info.col[ k_mwm_inactive_bot_x ] );
    mwm_res_set( database, k_mwm_inactive_auto_shade_str, k_mwm_res_boolean, 
                 &mwm_cust_info.inactive_auto_shade );

    /* Active border */
    mwm_res_set( database, WmNactiveForeground, k_mwm_res_col, 
                 &mwm_cust_info.col[ k_mwm_active_fore_x ] );
    mwm_res_set( database, WmNactiveBackground, k_mwm_res_col, 
                 &mwm_cust_info.col[ k_mwm_active_back_x ] );
    mwm_res_set( database, WmNactiveTopShadowColor, k_mwm_res_col, 
                 &mwm_cust_info.col[ k_mwm_active_top_x ] );
    mwm_res_set( database, WmNactiveBottomShadowColor, k_mwm_res_col, 
                 &mwm_cust_info.col[ k_mwm_active_bot_x ] );
    mwm_res_set( database, k_mwm_active_auto_shade_str, k_mwm_res_boolean, 
                 &mwm_cust_info.active_auto_shade );


}

/*******************************************************************
**
**  Description: Save the menu colors.
**
**  Formal Parameters
**    Input: database.                     
**  
********************************************************************/

void mwm_cust_menu_col_save( database )

XrmDatabase *database;

/* local variables */
{ 
int value, fore_x, back_x, top_x, bot_x;
                               
/********************************/

    if ( !mwm_cust_info.mod[ k_mwm_cust_border_col_did ] )
        return;

    /* Is this a B&W screen ? */
    if ( wmGD.Screens[ ACTIVE_SCREEN ].monitor != k_mwm_bw_type )
      /* No, save the menu colors */
      {
        /* Match the menu color ? */
        switch ( mwm_cust_info.menu_col )
          {
            case k_mwm_menu_col_active_fid:
              fore_x = k_mwm_active_fore_x;
              back_x = k_mwm_active_back_x;
              top_x = k_mwm_active_top_x;
              bot_x = k_mwm_active_bot_x;
              value = k_mwm_menu_col_active;
              break;
            case k_mwm_menu_col_inactive_fid:
              fore_x = k_mwm_inactive_fore_x;
              back_x = k_mwm_inactive_back_x;
              top_x = k_mwm_inactive_top_x;
              bot_x = k_mwm_inactive_bot_x;
              value = k_mwm_menu_col_inactive;    
              break;
            default:
            case k_mwm_menu_col_none_fid:
              value = k_mwm_menu_col_none;
              break;
          }
        /* Save the menu color resource */
        mwm_res_set( database, WmNmatchMenuColors, k_mwm_res_int, &value );
        /* Save the menu colors */
        if ( value != k_mwm_menu_col_none )
          {
            /* Menu border */
            mwm_res_set( database, WmNmenuForeground, k_mwm_res_col, 
                         &mwm_cust_info.col[ fore_x ] );
            mwm_res_set( database, WmNmenuBackground, k_mwm_res_col, 
                         &mwm_cust_info.col[ back_x ] );
            mwm_res_set( database, WmNmenuTopShadowColor, k_mwm_res_col, 
                         &mwm_cust_info.col[ top_x ] );
            mwm_res_set( database, WmNmenuBottomShadowColor, k_mwm_res_col, 
                         &mwm_cust_info.col[ bot_x ] );
          }
    }

}

/*******************************************************************
**
**  Description: Save all the current settings.
**
**  Formal Parameters
**    Input: database.
**  
********************************************************************/

void mwm_cust_matte_save( database )

XrmDatabase *database;

/* local variables */
{
int size;
char string[ 256 ];
                               
/********************************/

    if ( !mwm_cust_info.mod[ k_mwm_cust_matte_did ] )
        return;

    /* Save the matte size */
    switch ( mwm_cust_info.matte_size_fid )
      {
        case k_mwm_matte_small_fid:          
          size = k_mwm_matte_small_size;
          break;
        case k_mwm_matte_medium_fid:          
          size = k_mwm_matte_medium_size;
          break;
        case k_mwm_matte_large_fid:          
          size = k_mwm_matte_large_size;
          break;
        case k_mwm_matte_none_fid:          
          size = k_mwm_matte_none_size;
          break;
        default:
          size = mwm_cust_info.matte_size;
          break;
      }
                                          
    /* Matte width */
    mwm_res_set( database, WmNmatteWidth, k_mwm_res_int, &size );

}

/*******************************************************************
**
**  Description: Save all the current settings.
**
**  Formal Parameters
**    Input: database.
**  
********************************************************************/

void mwm_cust_matte_col_save( database )
                             
XrmDatabase *database;

/* local variables */
{
                               
/********************************/

    if ( !mwm_cust_info.mod[ k_mwm_cust_matte_did ] )
        return;

    /* colors */                               
    mwm_res_set( database, WmNmatteForeground, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_matte_fore_x ] );
    mwm_res_set( database, WmNmatteBackground, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_matte_back_x ] );
    mwm_res_set( database, WmNmatteTopShadowColor, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_matte_top_x ] );
    mwm_res_set( database, WmNmatteBottomShadowColor, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_matte_bot_x ] );
    mwm_res_set( database, k_mwm_matte_auto_shade_str, k_mwm_res_boolean, 
                      &mwm_cust_info.matte_auto_shade );

}

/*******************************************************************
**
**  Description: Save all the current settings.
**                       
**  Formal Parameters
**    Input: database.
**  
********************************************************************/

void mwm_cust_icons_save( database )
        
XrmDatabase *database;

/* local variables */
{
char string[ 256 ];
int height, width;
char orient[ k_mwm_max_str_len ];
                               
/********************************/

    /* Check the icon box geometry. */
    if ( mwm_cust_box_geo_get( string, orient ))
      {
        /* Set the geometry */
        XrmPutStringResource( database, k_mwm_icon_geo_str, string );
        /* Set the display policy */
        mwm_res_set( database, WmNiconBoxSBDisplayPolicy, k_mwm_res_string,
                     orient );
      }

    if ( !mwm_cust_info.mod[ k_mwm_cust_icons_did ] )
        return;

    /* Use icon box ? */
    mwm_res_set( database, WmNuseIconBox, k_mwm_res_boolean,
                      &mwm_cust_info.use_icon_box );

    /* Icon Decoration */
    strcpy( string, "" );
    if ( mwm_cust_info.active_icon ) 
      {                                                 
        strcat( string, k_mwm_active_icon_str );
        strcat( string, k_mwm_space_str );
      }
    if ( mwm_cust_info.icon_label )                                       
      { 
        strcat( string, k_mwm_icon_label_str );
        strcat( string, k_mwm_space_str );
      }
    if ( mwm_cust_info.icon_image ) 
        strcat( string, k_mwm_icon_image_str );

    mwm_res_set( database, WmNiconDecoration, k_mwm_res_string, string );

    /* Icon Placement */
    strcpy( string, "" );
    switch ( mwm_cust_info.icon_placement )
      {
        case k_mwm_icon_top_left_fid:
          strcat( string, k_mwm_top_left_str );
          break;
        case k_mwm_icon_top_right_fid:
          strcat( string, k_mwm_top_right_str );
          break;
        case k_mwm_icon_bot_left_fid:         
          strcat( string, k_mwm_bot_left_str );
          break;                        
        case k_mwm_icon_bot_right_fid:          
          strcat( string, k_mwm_bot_right_str );
          break;
        case k_mwm_icon_left_top_fid:               
          strcat( string, k_mwm_left_top_str );
          break;
        case k_mwm_icon_left_bot_fid:
          strcat( string, k_mwm_left_bot_str );
          break;
        case k_mwm_icon_right_top_fid:             
          strcat( string, k_mwm_right_top_str );
          break;                       
        case k_mwm_icon_right_bot_fid:        
          strcat( string, k_mwm_right_bot_str );
          break;
        default:
          break;
      }
      
    mwm_res_set( database, WmNiconPlacement, k_mwm_res_string, string );

    /* Save the icon size */
    switch ( mwm_cust_info.icon_size_fid )
      {
        case k_mwm_icon_small_fid:          
          height = width = k_mwm_icon_small_size;
          break;
        case k_mwm_icon_medium_fid:          
          height = width = k_mwm_icon_medium_size;
          break;
        case k_mwm_icon_large_fid:          
          height = width = k_mwm_icon_large_size;
          break;
        case k_mwm_icon_wide_fid:          
          height = k_mwm_icon_wide_height;
          width = k_mwm_icon_wide_width;
          break;
        default:
          height = mwm_cust_info.icon_height;
          width = mwm_cust_info.icon_width;
          break;
      }
                                          
    sprintf( string, "%dx%d", mwm_cust_info.icon_width,
             mwm_cust_info.icon_height );
    /* Icon size */
    mwm_res_set( database, WmNiconImageMaximum, k_mwm_res_string,
                      string );
    
}

/*******************************************************************
**
**  Description: Save the icon colors.
**
**  Formal Parameters
**    Input: database.
**  
********************************************************************/

void mwm_cust_icon_col_save( database ) 
                                                       
XrmDatabase *database;

/* local variables */
{
                               
/********************************/

    if ( !mwm_cust_info.mod[ k_mwm_cust_icon_col_did ] )
        return;

    /* Icon cols */
    mwm_res_set( database, WmNiconImageForeground, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_icon_image_fore_x ] );
    mwm_res_set( database, WmNiconImageBackground, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_icon_image_back_x ] );
    mwm_res_set( database, WmNiconImageTopShadowColor, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_icon_image_top_x ] );
    mwm_res_set( database, WmNiconImageBottomShadowColor, k_mwm_res_col, 
                      &mwm_cust_info.col[ k_mwm_icon_image_bot_x ] );
    mwm_res_set( database, k_mwm_image_auto_shade_str, k_mwm_res_boolean, 
                      &mwm_cust_info.image_auto_shade );

    /* Icon box */
    if ( mwm_cust_info.icon_box_auto_shade )
      {
        /* For V1.1 compatibility */
        mwm_res_set( database, WmNiconBoxHBackground, k_mwm_res_string,
                           "" );
        mwm_res_set( database, WmNiconBoxVBackground, k_mwm_res_string,
                           "" );   
        /* For V1.2, Mwm*iconbox.background is the correct resource.
           Write this out too.  The troughColor does not work in V1.2. */
        mwm_res_set( database, WmNiconBoxBackground, k_mwm_res_string,
                           "" );   
        /* If it is monochrome, reset the background pixmap to 50_foreground */
        if ( wmGD.Screens[ 0 ].monitor == k_mwm_bw_type )
            mwm_res_set( database, WmNiconBoxBackgroundPixmap, k_mwm_res_string,
                         k_mwm_50_foreground_str );
      }
    else 
      {
        mwm_res_set( database, WmNiconBoxVBackground, k_mwm_res_col, 
                     &mwm_cust_info.col[ k_mwm_icon_box_back_x ] );
        mwm_res_set( database, WmNiconBoxHBackground, k_mwm_res_col, 
                     &mwm_cust_info.col[ k_mwm_icon_box_back_x ] );
        /* For V1.2, Mwm*iconbox.background is the correct resource.
           Write this out too.  The troughColor does not work in V1.2. */
        mwm_res_set( database, WmNiconBoxBackground, k_mwm_res_col, 
                     &mwm_cust_info.col[ k_mwm_icon_box_back_x ] );
        /* If it is monochrome, reset the background pixmap to none */
        if ( wmGD.Screens[ 0 ].monitor == k_mwm_bw_type )
            mwm_res_set( database, WmNiconBoxBackgroundPixmap, k_mwm_res_string,
                         k_mwm_unspec_pixmap_str );
      }

}                                           

/*******************************************************************
**
**  Description: Save all the current settings.          
**               For multicolor systems, don't save the
**               color information if screen
**
**  Formal Parameters
**    Input: screen,
**           apply flag.
**  
********************************************************************/

void mwm_cust_apply_all( screen, apply )

int screen;
Boolean apply;
                                      
/* local variables */
{
XrmDatabase database;
XrmDatabase nondef_database;                               
int nondef_file;
                                                                  
/********************************/
    
    /* Save the information ? */
    if ( apply )
      {
        mwm_watch_set( true );
        database = XrmGetFileDatabase( mwm_user_def_res_file );

        /* What's the color of the main monitor ?
           A current limitation is to only save the colors for
           color or gray-scale systems for multicolor systems. */
        switch ( wmGD.main_monitor )
          {
            case k_mwm_bw_type:
              nondef_database = XrmGetFileDatabase( mwm_user_bw_res_file );
              nondef_file = true;
              break;
            case k_mwm_gray_type: 
              nondef_database = XrmGetFileDatabase( mwm_user_gray_res_file );
              nondef_file = true;
              break;                                          
            case k_mwm_color_type:
            default:             
              nondef_file = false;
              break;
          }
    
        mwm_cust_ws_save( &database );
        mwm_cust_icons_save( &database );
        mwm_cust_border_save( &database );
        mwm_cust_matte_save( &database );

        /* Default color resource file ? */
        if ( !nondef_file )
          {
            mwm_cust_icon_col_save( &database );
            mwm_cust_border_col_save( &database );
            mwm_cust_menu_col_save( &database );
            mwm_cust_matte_col_save( &database );
          }
        else
          {
            mwm_cust_icon_col_save( &nondef_database );
            mwm_cust_border_col_save( &nondef_database );
            /* Is this a B&W system ? */
            if ( wmGD.main_monitor != k_mwm_bw_type )
                /* No, save the menu colors */
                mwm_cust_menu_col_save( &nondef_database );
            mwm_cust_matte_col_save( &nondef_database );
          }
        /* Update the saved information for a future reset. */
        memcpy( &mwm_cust_info_save, &mwm_cust_info, sizeof( mwm_cust_info_type ));

        /* Write resources back out to file */
        XrmPutFileDatabase( database, mwm_user_def_res_file );
        if ( nondef_file )
            if ( wmGD.main_monitor == k_mwm_bw_type )
                XrmPutFileDatabase( nondef_database, mwm_user_bw_res_file );
            else XrmPutFileDatabase( nondef_database, mwm_user_gray_res_file );

        /* Destroy the database */
        XrmDestroyDatabase( database );
        if ( nondef_file )
            XrmDestroyDatabase( nondef_database );
        mwm_watch_set( false );
      }    

    /* Remove the dialog box if it is up. */
    if ( mwm_did[ k_mwm_cust_apply_did ].up )
        XtPopdown( XtParent( (*mwm_screen_did)[ screen ][ k_mwm_cust_apply_did ].wid ));
    else if ( mwm_did[ k_mwm_cust_restart_did ].up )
        XtUnmanageChild( (*mwm_screen_did)[ screen ][ k_mwm_cust_restart_did ].wid );

#ifdef DEBUG
    exit(0);
#else
    /* Restart the window manager */
    RestartWm( MWM_INFO_STARTUP_CUSTOM );
#endif

}

/*******************************************************************
**
**  Description: Save all the current settings.
**
**    Input: widget id,               
**           resource,
**           reason.
**  
********************************************************************/

void mwm_cust_apply( wid, tag, reason )

Widget wid;
int *tag;
unsigned int *reason;

/* local variables */
{
XrmDatabase database;
int apply = *tag;
                               
/********************************/

    mwm_cust_apply_all( WID_SCREEN, apply == k_mwm_yes_id );

}

/*******************************************************************
**
**  Description: Bring up a message dialog box.
**
**  Formal Parameters:
**    Input: screen index, 
**           screen num,                                                  
**           true if this is restart; false if apply.
**  
********************************************************************/

int mwm_cust_apply_up( screen, screen_num, restart )

int screen;
int screen_num;
int restart;

/* local variables */
{
Widget wid;
int modified, did, other_did;
char string[ 256 ];
                               
/********************************/
                                                        
    /* Was anything modified ? */
    modified = false;
    if ( mwm_cust_init )
        /* Set to true if anything was modified. */
        modified = mwm_cust_info.mod[ k_mwm_cust_ws_did ] ||
                   mwm_cust_info.mod[ k_mwm_cust_icons_did ] ||
                   mwm_cust_info.mod[ k_mwm_cust_icon_col_did ] ||
                   mwm_cust_info.mod[ k_mwm_cust_border_did ] ||
                   mwm_cust_info.mod[ k_mwm_cust_border_col_did ] ||
                   mwm_cust_info.mod[ k_mwm_cust_matte_did ];
    /* If this is called from customize apply settings and
       the customization has not been modified, then check the icon box. */
    if ( ! modified && ! restart ) 
      {
        /* Get the current icon box geometry */
        if ( mwm_cust_box_geo_get( string, NULL ))
          {
            /* Did the geometry change ? */
            if ( !mwm_str_eq( string, ACTIVE_PSD->iconBoxGeometry ))
                /* Yes */
                modified = true;
          }  
      }

    if ( restart )
      {
        did = k_mwm_cust_restart_did;
        other_did = k_mwm_cust_apply_did;
      }
    else  
      {
        did = k_mwm_cust_apply_did;
        other_did = k_mwm_cust_restart_did;
      }
    if ( mwm_dialog_apply_wid != (Widget)NULL )
      {
        /* Bring down the other one if it's up. */
        mwm_dialog_cancel( mwm_dialog_apply_wid, &other_did, NULL );
        mwm_dialog_cancel( mwm_dialog_apply_wid, &did, NULL );
      }

    if ( ! modified )
        return ( false );

    /* Make sure Mwm is initialized on this screen. */
    mwm_init( screen, screen_num );               

    /* Bring up the dialog box. */
    if ( mwm_dialog_get( screen, &(*mwm_screen_did)[ screen ][ did ].wid, did,
                         mwm_did[ did ].name,
                         mwm_dialog_shell[ screen ] ))
      {                                                            
        mwm_dialog_up( (*mwm_screen_did)[ screen ][ did ].wid, did,
                       mwm_dialog_shell[ screen ] );
        mwm_dialog_apply_wid =  (*mwm_screen_did)[ screen ][ did ].wid;
        /* Make sure the cursor is reset in case of an error. */
        mwm_watch_set( false );
        return( true );
      }
    /* Make sure the cursor is reset in case of an error. */
    mwm_watch_set( false );
    return( false );
                       
}

/*******************************************************************/

/* OK module */

/*******************************************************************
**
**  Description: The user OKed customize workspace.
**
**  Formal Parameters
**    Input: widget.
**  
********************************************************************/

int mwm_cust_ws_ok( wid )     
                       
Widget wid;

{
/* local variables */

/********************************/

    mwm_cust_info.focus_raised = 
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_focus_raised_fid ] );
    mwm_cust_info.focus_removed =                             
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_focus_removed_fid ] );
    mwm_cust_info.focus_start = 
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_focus_start_fid ] );
    mwm_cust_info.focus_deicon =                             
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_focus_deicon_fid ] );
    mwm_cust_info.force_alt_space =                             
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_force_alt_space_fid ] );
    mwm_cust_info.fb.move =
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_fb_move_size_fid ] );
    mwm_cust_info.fb.resize = mwm_cust_info.fb.move;
    mwm_cust_info.ignore_mods =                             
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_ignore_mods_fid ] );
    /* Save the info in last */
    mwm_cust_ws_copy( &mwm_cust_info_last, &mwm_cust_info );
    return( true );

}

/*******************************************************************
**
**  Description: The user OKed customize icons.
**
**  Formal Parameters
**    Input: widget.
**  
********************************************************************/

int mwm_cust_icons_ok( wid )     
                       
Widget wid;

{
/* local variables */
int ok = true;

/********************************/                               
                       
    /* Save the current variables */
    /* Use icon box */
    mwm_cust_info.use_icon_box =
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_use_icon_box_fid ] );

    /* Icon decorations */
    mwm_cust_info.active_icon =
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_active_icon_fid ] );
    mwm_cust_info.icon_image =
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_image_fid ] );
    mwm_cust_info.icon_label =
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_label_fid ] );

    /* Icon size */
    if ( mwm_cust_info.icon_size_fid == k_mwm_icon_other_fid )
      {              
        ok = mwm_num_get( wid, k_mwm_cust_icons_did, k_mwm_icon_height_fid,
                          &mwm_cust_info.icon_height,
                          mwm_cust_info.icon_height_str );
        if ( ok )
        /* Verify the icon other height and size */
            ok = mwm_num_get( wid, k_mwm_cust_icons_did,
                              k_mwm_icon_width_fid, &mwm_cust_info.icon_width,
                              mwm_cust_info.icon_width_str );
      }

    switch ( mwm_cust_info.icon_size_fid )
      {
        case k_mwm_icon_small_fid:
          mwm_cust_info.icon_width = mwm_cust_info.icon_height = 
            k_mwm_icon_small_size;
          break;
        case k_mwm_icon_medium_fid:
          mwm_cust_info.icon_width = mwm_cust_info.icon_height = 
            k_mwm_icon_medium_size;
          break;
        case k_mwm_icon_large_fid:
          mwm_cust_info.icon_width = mwm_cust_info.icon_height = 
            k_mwm_icon_large_size;
          break;
        case k_mwm_icon_wide_fid:
          mwm_cust_info.icon_width = k_mwm_icon_wide_width;
          mwm_cust_info.icon_height = k_mwm_icon_wide_height;
          break;
        case k_mwm_icon_other_fid:
          mwm_cust_info.icon_width = mwm_cust_info.icon_width;
          mwm_cust_info.icon_height = mwm_cust_info.icon_height;
          break;
        default:
          break;                
      }
    if ( ok )
        /* Save the info in last */
        mwm_cust_icons_copy( &mwm_cust_info_last, &mwm_cust_info );
        
    return( ok );

}

/*******************************************************************
**
**  Description: The user OKed customize icon colors.
**
**  Formal Parameters
**    Input: widget.
**  
********************************************************************/
                                                     
int mwm_cust_icon_col_ok( wid )     
                       
Widget wid;

{
/* local variables */

/********************************/
                                
    /* Save the current variables */
                     
    /* Icon image colors */
    mwm_col_get( (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_image_fore_fid ], XmNbackground,
      &mwm_cust_info.col[ mwm_col_index_get( k_mwm_icon_image_fore_fid ) ].xcol);
    mwm_col_get( (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_image_back_fid ], XmNbackground,
      &mwm_cust_info.col[ mwm_col_index_get( k_mwm_icon_image_back_fid ) ].xcol);
    mwm_col_get( (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_image_top_fid ], XmNbackground,
      &mwm_cust_info.col[ mwm_col_index_get( k_mwm_icon_image_top_fid ) ].xcol);     
    mwm_col_get( (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_image_bot_fid ], XmNbackground,
      &mwm_cust_info.col[ mwm_col_index_get( k_mwm_icon_image_bot_fid )].xcol);   
    mwm_cust_info.image_auto_shade =              
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_image_auto_shade_fid ] );
    mwm_cust_info.icon_box_auto_shade =              
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_box_auto_shade_fid ] );

    /* Icon box cols */
    mwm_col_get( (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_box_back_fid ], XmNbackground, 
      &mwm_cust_info.col[ mwm_col_index_get( k_mwm_icon_box_back_fid ) ].xcol);

    /* Save the info in last */
    mwm_cust_icon_col_copy( &mwm_cust_info_last, &mwm_cust_info );
    return( true );

}

/*******************************************************************
**
**  Description: The user OKed customize border.
**
**  Formal Parameters                             
**    Input: widget.
**  
********************************************************************/

int mwm_cust_border_ok( wid )     
                       
Widget wid;

{
/* local variables */
int ok = true;

/********************************/

    mwm_cust_info.min =
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_min_fid ] );
    mwm_cust_info.max =
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_max_fid ] );
    mwm_cust_info.resize =
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_resize_fid ] );
    mwm_cust_info.title =
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_title_fid ] );
    mwm_cust_info.border =
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_border_fid ] );
    mwm_cust_info.menu =
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_menu_fid ] );
    mwm_cust_info.wmenu_dclick =
      XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ k_mwm_wmenu_dclick_fid ] );

    /* Border size */
    if ( mwm_cust_info.border_size_fid == k_mwm_border_other_fid )
      {
        /* Verify the border other height and size */
        ok = mwm_num_get( wid, k_mwm_cust_border_did, k_mwm_border_size_text_fid,
                          &mwm_cust_info.border_size,   
                          mwm_cust_info.border_size_str );
      }

    if ( ok )
        /* Save the info in last */
        mwm_cust_border_copy( &mwm_cust_info_last, &mwm_cust_info );

    return( ok );

}

/*******************************************************************
**
**  Description: The user OKed customize border colors.
**
**  Formal Parameters                             
**    Input: widget.
**  
********************************************************************/

int mwm_cust_border_col_ok( wid )     
                       
Widget wid;       

{
/* local variables */
int screen;

/********************************/

    mwm_col_get( (*mwm_fid)[ WID_SCREEN ][ k_mwm_fore_fid ], XmNforeground, 
      &mwm_cust_info.col[ mwm_col_index_get( k_mwm_fore_fid ) ].xcol);
    mwm_col_get( (*mwm_fid)[ WID_SCREEN ][ k_mwm_back_fid ], XmNbackground,                
      &mwm_cust_info.col[ mwm_col_index_get( k_mwm_back_fid ) ].xcol);
    mwm_col_get( (*mwm_fid)[ WID_SCREEN ][ k_mwm_top_fid ], XmNbackground,                          
      &mwm_cust_info.col[ mwm_col_index_get( k_mwm_top_fid ) ].xcol);
    mwm_col_get( (*mwm_fid)[ WID_SCREEN ][ k_mwm_bot_fid ], XmNbackground,               
      &mwm_cust_info.col[ mwm_col_index_get( k_mwm_bot_fid ) ].xcol);  
    mwm_col_get( (*mwm_fid)[ WID_SCREEN ][ k_mwm_active_fore_fid ], XmNbackground, 
      &mwm_cust_info.col[ mwm_col_index_get( k_mwm_active_fore_fid ) ].xcol);
    mwm_col_get( (*mwm_fid)[ WID_SCREEN ][ k_mwm_active_back_fid ], XmNbackground,               
      &mwm_cust_info.col[ mwm_col_index_get( k_mwm_active_back_fid ) ].xcol);  
    mwm_col_get( (*mwm_fid)[ WID_SCREEN ][ k_mwm_active_top_fid ], XmNbackground,            
      &mwm_cust_info.col[ mwm_col_index_get( k_mwm_active_top_fid ) ].xcol);   
    mwm_col_get( (*mwm_fid)[ WID_SCREEN ][ k_mwm_active_bot_fid ], XmNbackground, 
      &mwm_cust_info.col[ mwm_col_index_get( k_mwm_active_bot_fid ) ].xcol);  

    /* Save the info in last */
    mwm_cust_border_col_copy( &mwm_cust_info_last, &mwm_cust_info );
 
    /* Reset the icon box colors */
    /* Is customize icon colors up ? */
    if ( mwm_did[ k_mwm_cust_icon_col_did ].up )
      /* Yes, reset the icon box colors if necessary since
         if autoshadow is set, then they are based on the
         border background color. */
        mwm_col_auto_chec( (*mwm_screen_did)[ mwm_did[ k_mwm_cust_icon_col_did ].screen ][ k_mwm_cust_icon_col_did ].wid,
                           k_mwm_icon_box_auto_shade_fid );

    return( true );

}

/*******************************************************************
**
**  Description: The user OKed customize matte.
**
**  Formal Parameters                             
**    Input: widget.
**  
********************************************************************/

int mwm_cust_matte_ok( wid )     
                       
Widget wid;       

{
/* local variables */
int ok = true;

/********************************/

    /* Matte size */
    if ( mwm_cust_info.matte_size_fid == k_mwm_matte_other_fid )
      {
        /* Verify the matte other height and size */
        ok = mwm_num_get( wid, k_mwm_cust_matte_did, k_mwm_matte_size_text_fid, 
                          &mwm_cust_info.matte_size,
                          mwm_cust_info.matte_size_str );
      }
    mwm_col_get( (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_fore_fid ], XmNbackground, 
      &mwm_cust_info.col[ mwm_col_index_get( k_mwm_matte_fore_fid ) ].xcol);           
    mwm_col_get( (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_back_fid ], XmNbackground, 
      &mwm_cust_info.col[ mwm_col_index_get( k_mwm_matte_back_fid ) ].xcol);
    mwm_col_get( (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_top_fid ], XmNbackground,                          
      &mwm_cust_info.col[ mwm_col_index_get( k_mwm_matte_top_fid ) ].xcol);
    mwm_col_get( (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_bot_fid ], XmNbackground,               
      &mwm_cust_info.col[ mwm_col_index_get( k_mwm_matte_bot_fid ) ].xcol);  

    if ( ok )
        /* Save the info in last */
        mwm_cust_matte_copy( &mwm_cust_info_last, &mwm_cust_info );

    return( ok );

}

/*******************************************************************/

/* Reset module */

/*******************************************************************
**
**  Description: Reset the customize settings.
**               Reset to initial or last settings.
**
**  Formal Parameters
**    Input: widget id,               
**           did,
**           cust info.
**  
********************************************************************/

void mwm_cust_info_rese( wid, did, cust_info_ptr )

Widget wid;
int did;
mwm_cust_info_type *cust_info_ptr;

/* local variables */
{
 
/********************************/

    /* Is this reset is for one dialog box or for all the dialog boxes?
       If the wid is the screen's, then it's all dialogs, and 
       don't reset the cursor.*/
    if ( wid != mwm_dialog_shell[ WID_SCREEN ] )
        mwm_watch_set( true );
                                                
    /* Copy the saved information back. */       
    switch ( did )
      {
        case k_mwm_cust_ws_did: mwm_cust_ws_copy( &mwm_cust_info, cust_info_ptr ); break;
        case k_mwm_cust_border_did: mwm_cust_border_copy( &mwm_cust_info, cust_info_ptr ); break;
        case k_mwm_cust_border_col_did: mwm_cust_border_col_copy( &mwm_cust_info, cust_info_ptr ); break;
        case k_mwm_cust_icons_did: mwm_cust_icons_copy( &mwm_cust_info, cust_info_ptr ); break;
        case k_mwm_cust_icon_col_did: mwm_cust_icon_col_copy( &mwm_cust_info, cust_info_ptr ); break;
        case k_mwm_cust_matte_did: mwm_cust_matte_copy( &mwm_cust_info, cust_info_ptr ); break;
        default:
          if ( wid != mwm_dialog_shell[ WID_SCREEN ] )
            mwm_watch_set( false );
          return;
      }

    /* Reset the dialog box if it is up. */
    if ( mwm_did[ did ].up )
        mwm_dialog_rese( did, mwm_fid_list[ did ] );
    /* Is this reset is for one dialog box or for all the dialog boxes?
       If the wid is the screen's, then it's all dialogs, and 
       don't reset the cursor.*/
    if ( wid != mwm_dialog_shell[ WID_SCREEN ] )
        mwm_watch_set( false );
    
}                                                                   
                                  
/*******************************************************************
**
**  Description: Reset callback for the customize settings.
**
**  Formal Parameters
**    Input: widget id,               
**           resource,                   
**           reason.
**  
********************************************************************/

void mwm_cust_rese( wid, tag, reason )

Widget wid;
int *tag;
unsigned int *reason;

/* local variables */
{
int did = *tag;
 
/********************************/

    /* Reset to the last saved (initial) settings */
    mwm_cust_info_rese( wid, did, &mwm_cust_info_save );
    
}                                                                   
                                  
/*******************************************************************
**
**  Description: Reset all the customize settings to the system
**               default.
**               This is called with the wid of the dialog box
**               that reset was selected or 
**               with the shell's wid if its reset all.
**
**  Formal Parameters
**    Input: widget id,               
**           resource,                   
**           reason.
**  
********************************************************************/

void mwm_cust_defa( wid, tag, reason )

Widget wid;
int *tag;
unsigned int *reason;

/* local variables */
{                
int did = *tag;
XrmDatabase database;
XrmDatabase nondef_database;                               
Boolean nondef_file;
int screen_num;
 
/********************************/
                                                 
    screen_num =  WID_SCREEN;
    database = XrmGetFileDatabase( mwm_sys_def_res_file );
    if ( database == NULL )
      {
        fprintf( stderr, "mwm: Opening %s \n", mwm_sys_def_res_file );
        return;
      }

    /* Get the database based on the main monitor type */
    switch ( wmGD.main_monitor )
      {
        case k_mwm_bw_type:
          nondef_database = XrmGetFileDatabase( mwm_sys_bw_res_file );
          if ( nondef_database == NULL )
            {
              fprintf( stderr, "mwm: Opening %s \n", mwm_sys_bw_res_file );
              return;
            }
          nondef_file = true;
          break;
        case k_mwm_gray_type: 
          nondef_database = XrmGetFileDatabase( mwm_sys_gray_res_file );
          if ( nondef_database == NULL )
            {
              fprintf( stderr, "mwm: Opening %s \n", mwm_sys_gray_res_file );
              return;
            }
          nondef_file = true;
          break;                                          
        case k_mwm_color_type:
        default:             
          nondef_file = false;
          break;
      }
    

    /* Is this set to default for one dialog box or for all the dialog boxes.  
       If the wid is screen, then its all dialogs, and don't reset the cursor.*/
    if ( wid != mwm_dialog_shell[ WID_SCREEN ] )
        mwm_watch_set( true );

    /* It needs to be initialized. */
    mwm_cust_info.init[ did ] = false;
    switch ( did )
      {
        case k_mwm_cust_ws_did: 
          mwm_cust_ws_get( database ); 
          break;
        case k_mwm_cust_border_did: 
          mwm_cust_border_get( database );
          break;
        case k_mwm_cust_icons_did:
          mwm_cust_icons_get( database );
          break;
        case k_mwm_cust_border_col_did:
          if ( !nondef_file )
              mwm_cust_border_col_get( database );
          else mwm_cust_border_col_get( nondef_database );
          break;
        case k_mwm_cust_icon_col_did: 
          if ( !nondef_file )
              mwm_cust_icon_col_get( database );
          else mwm_cust_icon_col_get( database );
          break;
        case k_mwm_cust_matte_did: 
          if ( !nondef_file )
              mwm_cust_matte_get( database );
          else mwm_cust_matte_get( database );
          break;
        default: break;    
      }          
    mwm_cust_info.init[ did ] = true;
    XrmDestroyDatabase( database );
    if ( nondef_file )
        XrmDestroyDatabase( nondef_database );
    /* Reset the dialog box if it is up. */
    if ( mwm_did[ did ].up )
        mwm_dialog_rese( did, mwm_fid_list[ did ] );
    /* Is this set to default for one dialog box or for all the dialog boxes.  
       If the wid is screen, then its all dialogs, and don't reset the cursor.*/
   if ( wid != mwm_dialog_shell[ WID_SCREEN ] )
       mwm_watch_set( false );
    
}                                                                   
                            
/*******************************************************************
**
**  Description: Reset all the customize settings.
**
**  Formal Parameters
**    Input: widget.
**  
********************************************************************/
             
void mwm_cust_all_rese( wid )     
                       
Widget wid;

/* local variables */
{
int did;
 
/********************************/

    mwm_watch_set( true );

    for ( did = k_mwm_cust_ws_did; did <= k_mwm_last_main_did; did++ )
      {
         /* Has the customization info been read ? */
         if ( mwm_cust_info.init[ did ] )
           /* Yes, reset it. */
           {
             /* Reset the data. */
             mwm_cust_rese( wid, &did, NULL );
             /* Now updated internally (reset) since last modify. */
             mwm_cust_info.mod[ did ] = false;
           }
      }

    mwm_watch_set( false );

}                                                                   

/*******************************************************************
**
**  Description: Reset all the customize settings to the default.
**
**  Formal Parameters
**    Input: widget.
**  
********************************************************************/
            
void mwm_cust_all_defa( wid )     
                       
Widget wid;

/* local variables */
{
int did;
 
/********************************/

    mwm_watch_set( true );

    for ( did = k_mwm_cust_ws_did; did <= k_mwm_last_main_did; did++ )
      {
         /* Get the customization information? */
         mwm_cust_get( did );
         mwm_cust_defa( wid, &did, NULL );
         /* Now modified but not updated */
         mwm_cust_info.mod[ did ] = true;
      }

    mwm_watch_set( false );

}                                                                   

/*******************************************************************/

/* Customize module */

/*******************************************************************
**
**  Description: The user selected customize.
**
**  Formal Parameters
**    Input: arg list,
**           ptr to Client,
**           event.                
**  
********************************************************************/

int mwm_cust( arg_list, pCD, wid )

String arg_list;                      
ClientData *pCD;
Widget wid;

/* local variables */
{
int did, ok;
Widget shell;

/********************************/

#ifdef DEBUG
/*    XSynchronize( wmGD.dialog_display, true );
*/
#endif

    /* Make sure that the uil files are initialized. */
    did = k_mwm_none;
    ok = true;
    if ( mwm_init( WID_SCREEN, WID_SCREEN_NUM ) )
      {
        /* Customize the workspace ? */
        if ( mwm_str_eq( arg_list, k_mwm_cust_ws_arg ))
            did = k_mwm_cust_ws_did;
        /* Customize border ? */               
        else if ( mwm_str_eq( arg_list, k_mwm_cust_border_arg ))
            did = k_mwm_cust_border_did;          
        /* Customize icons ? */
        else if ( mwm_str_eq( arg_list, k_mwm_cust_icons_arg ))
            did = k_mwm_cust_icons_did;
        /* Customize border color ? */               
        else if ( mwm_str_eq( arg_list, k_mwm_cust_border_col_arg ))
            did = k_mwm_cust_border_col_did;                           
        /* Customize icons color ? */                              
        else if ( mwm_str_eq( arg_list, k_mwm_cust_icon_col_arg ))
            did = k_mwm_cust_icon_col_did;
        /* Customize matte ? */
        else if ( mwm_str_eq( arg_list, k_mwm_cust_matte_arg ))
            did = k_mwm_cust_matte_did;
        /* Customize apply ? */
        else if ( mwm_str_eq( arg_list, k_mwm_cust_apply_arg ))
          {
            /* Show feedback ? */
            if ( wmGD.showFeedback & WM_SHOW_FB_RESTART )
                mwm_cust_apply_up( WID_SCREEN, WID_SCREEN_NUM, false );
            else mwm_cust_apply_all( WID_SCREEN, true );      
          }
        /* Customize reset ? */
        else if ( mwm_str_eq( arg_list, k_mwm_cust_reset_arg ))
            mwm_cust_all_rese( mwm_dialog_shell[ WID_SCREEN ] );
        /* Customize default ? */
        else if ( mwm_str_eq( arg_list, k_mwm_cust_default_arg ))
            mwm_cust_all_defa( mwm_dialog_shell[ WID_SCREEN ] );
      }
                          
    switch ( did )
      {
        /* If it's a dual head monitor, then don't bring colors
           up on a b&w or gray-scale system. */
        case k_mwm_cust_border_col_did:
        case k_mwm_cust_icon_col_did:
        case k_mwm_cust_matte_did:
          /* Multicolor system ? */
          if ( mwm_cust_info.multicolor )
              /* OK only if same as the main monitor color */
              ok = wmGD.Screens[ WID_SCREEN ].monitor == wmGD.main_monitor;
        case k_mwm_cust_ws_did:
        case k_mwm_cust_border_did:
        case k_mwm_cust_icons_did:
          if ( ok )
              /* Manage and bring up the dialog box. */
              if ( mwm_dialog_get( WID_SCREEN,
                                   &(*mwm_screen_did)[ WID_SCREEN ][ did ].wid, 
                                   did, 
                                   mwm_did[ did ].name, 
                                   mwm_dialog_shell[ WID_SCREEN ] ))
                  mwm_dialog_up( (*mwm_screen_did)[ WID_SCREEN ][ did ].wid, 
                                 did, mwm_dialog_shell[ WID_SCREEN ] );
          break;

        default:
          break;
      }
                          
    /* Make sure the cursor is reset in case of an error. */
    mwm_watch_set( false );

}    

/*******************************************************************/
