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
**  Abstract: Window Manager Color customization
**
**  Environment: VMS, Unix, Sun                     
**
*******************************************************************/

#include "WmGlobal.h"
#include "WmResNames.h"
#include "WmFunction.h"
#include <Xm/Xm.h>
#include <X11/ShellP.h>
#include <X11/Shell.h>
#include <Xm/DialogSP.h>
#include <Xm/ToggleB.h>
#include <Mrm/MrmPublic.h>
#include <stdio.h>
#include <DXm/DXmColor.h>

#include "mwm_cust.h"
#include "mwm_internal.h"
#include "mwm_col_internal.h"
#include "mwm_col_forward.h"
#include "mwm_col_callbacks.h"
#include "mwm_dialog.h"
#include "mwm_cust.h"
#include "mwm_util.h"
                                                                         
/* Initialization module */

/*******************************************************************
**                                                                    
**  Description: Initialize the callbacks.
**
**  Formal Parameters
**  
********************************************************************/

int mwm_col_init()

/* local variables */
{

/********************************/

    /* Inited yet ? */
    if ( mwm_cb_init )
        return( true );

    if ( MrmRegisterNames( mwm_col_cblist, mwm_col_cbnum ) != MrmSUCCESS)
      {
        fprintf( stderr, "The color resources could not be registered.\n" );
        return( false );
      }
    
    /* Allocate the sample information */
    if ( !mwm_alloc( (void *)&mwm_col_pix, sizeof( Pixmap ) *
                     wmGD.numScreens * ( k_mwm_sample ),       
                     "Error allocating sample pix" ))
        return( false );
    if ( !mwm_alloc( (void *)&mwm_col_gc, sizeof( GC ) *
                     wmGD.numScreens * ( k_mwm_sample ),       
                     "Error allocating sample gc" ))
        return( false );
    return( true );

}

/*******************************************************************/

/* Utility module */

/*******************************************************************
**
**  Description: Color range.
**
**  Formal Parameters
**    Input: Red, Green, Blue,
**    Output: Min, Max.
**  
********************************************************************/

void mwm_col_range( r,g,b,min,max )
                      
double r,g,b,*min,*max;

{
/********************************/

    *min = *max = r;
    if ( g < *min )
        *min = g;
    else if ( g > *max )
	*max = g;

    if (b < *min)
   	*min = b;
    else if ( b > *max )
	*max = b;

}

/*******************************************************************
**
**  Description: return color value;
**
**  Formal Parameters
**  
********************************************************************/

double mwm_col_value( n1, n2, hue )

double n1,n2,hue;

{
/* local variables */
double val;

/********************************/

    if (hue > 360.0)  
	hue = hue - 360.0;

    if (hue < 0.0)  
	hue = hue + 360.0;

    if (hue < 60.0)
	val = n1+(n2-n1)*hue/60.0;
    else if (hue < 180.0)
	val = n2;
    else if (hue < 240.0)
	val = n1+(n2-n1)*(240.0-hue)/60.0;
    else val = n1;

    return ( val );

}

/*******************************************************************
**
**  Description: Convert RGB to HLS from Foley and VanDam.
**
**  Formal Parameters
**    Input: Red, Green, Blue,
**    Output: Hue, lightness, saturation.
**  
********************************************************************/

void mwm_rgbtohls( red, green, blue, hue, light, sat )

double red, green, blue, *hue, *light, *sat;

{
/* local variables */
double min,max;
double rc,gc,bc;

/********************************/

    /* convert from X11 to standard RGB */
    red   = red/65535;
    green = green/65535;
    blue  = blue/65535;

    mwm_col_range( red, green, blue, &min, &max );

    /* lightness */
    *light = (max + min)/2;

    /* calculate hue and saturation */
    if (max == min)
      {
	/* r=g=b -- achromatic case */
	*sat = 0;
	*hue = 0;  /* undefined? */
      }
    else
      {  /* chromatic case */
	if ( *light <= 0.5 )
	   *sat = ( max - min ) / (max + min );
      	else *sat = ( max - min ) / ( 2 - max - min );

	/* calculate hue */   
	rc = ( max - red ) / ( max - min );
	gc = ( max - green ) / ( max - min );
	bc = ( max - blue )  / ( max - min );

	if ( red == max )
	    *hue = bc - gc;
	else if ( green == max )
	    *hue = 2 + rc - bc;
	else if ( blue == max )
	    *hue = 4 + gc - rc;

	*hue = *hue * 60.0;

	if ( *hue < 0.0 )
	    *hue = *hue + 360;

      }  /* chromatic case */
   
}

/*******************************************************************
**
**  Description: Convert HLS to RGB.
**
**  Formal Parameters
**    Input: Hue, lightness, saturation.
**    Output: rgb.
**  
********************************************************************/

void mwm_hlstorgb( dhue, dlight, dsat, red, green, blue )

double dhue, dlight, dsat;
unsigned short *red, *green, *blue;

{
/* local variables */
double dred, dgreen, dblue;
double m1,m2;

/********************************/

    if ( dlight < 0.5 ) 
	m2 = (dlight)*(1+dsat);
    else m2 = dlight + dsat- (dlight)*(dsat);

    m1 = (2.0*dlight) - m2;

    if ( dsat == 0 )
      {  /* gray shade (ignore hue?) */ 
	(dred)=(dgreen)=(dblue)=(dlight); 
      }
    else 
      {  /* calculate rgb values */
	dred   =mwm_col_value(m1,m2,(double)(dhue+120.0));
	dgreen =mwm_col_value(m1,m2,dhue);
	dblue  =mwm_col_value(m1,m2,(double)(dhue-120.0));
      }

    /* rgb in range of 0.0 to 1.0 - convert to x11 0 to 65535 */
    dred   = dred   * 65535;
    dgreen = dgreen * 65535;
    dblue  = dblue  * 65535;

    *red   = dred;
    *green = dgreen;
    *blue  = dblue;

}

/*******************************************************************
**
**  Description: Set the shadow color.
**               Convert the color to HLS and then vary the 
**               lightness by some percent.
**
**  Formal Parameters
**    Input: wid,
**           top index,
**           bottom index,
**           back index,
**           top fid,
**           bot fid,
**           sample.
**  
********************************************************************/

int mwm_col_auto_set( wid, top_x, bot_x, back_x, top_fid, bot_fid, sample )

Widget wid;
int top_x;
int bot_x;
int back_x;
int top_fid;
int bot_fid;
int sample;

/* local variables */
{
int index, sample_fid;
double hue, lightness, saturation, top_lightness, top_sat, bot_lightness,
bot_sat; 
XColor border_col;             

/********************************/

    /* Is this the icon box ? */
    if ( sample == k_mwm_icon_box_sample )
      /* Yes, get the derived color from the toolkit. */
      {
        /* Make sure the border color is loaded */
        mwm_cust_border_col_get( XtDatabase( DISPLAY ) );
        /* Get the derived background color. */

        /* Note that the border color options dialog box may be on a different
           screen than the customize icon colors. */
        /* Save the border colors */
        border_col = mwm_cust_info_last.col[ k_mwm_inactive_back_x ].xcol;
        /* Update the color for this screen. */
        XAllocColor( wmGD.dialog_display, 
                     XDefaultColormap( DISPLAY, WID_SCREEN_NUM ), &border_col);
        /* Get the derived value */
        XmGetColors( XtScreen ( wid ), XDefaultColormap( DISPLAY, WID_SCREEN_NUM ),
                     border_col.pixel, (Pixel *)NULL, (Pixel *)NULL, (Pixel *)NULL,
                     (Pixel *)&mwm_cust_info.col[ k_mwm_icon_box_back_x ].xcol.pixel );
        /* Get the color */
        XQueryColor( wmGD.dialog_display, XDefaultColormap( DISPLAY, WID_SCREEN_NUM ),
                     (void *)&mwm_cust_info.col[ k_mwm_icon_box_back_x ].xcol );
        /* Set the button and sample */
        mwm_set( (*mwm_fid)[ WID_SCREEN ][ bot_fid ], XmNbackground, 
                 (void *)mwm_cust_info.col[ bot_x ].xcol.pixel );
        mwm_col_reg( (*mwm_fid)[ WID_SCREEN ][ mwm_col_sample_fid( sample ) ], 
                     sample, bot_fid, mwm_cust_info.col[ bot_x ].xcol.pixel );
      }    
    else
      {

    /* Get the Hue, Lightness, and Saturation */
    mwm_rgbtohls( (double)mwm_cust_info.col[ back_x ].xcol.red,
                  (double)mwm_cust_info.col[ back_x ].xcol.green,
                  (double)mwm_cust_info.col[ back_x ].xcol.blue,
                  &hue, &lightness, &saturation );

    /* Compute top and bottom shadows based on the lightness
       of the background */
    if ( lightness > 0.7 )
      {
        top_lightness = MIN( lightness + (0.12*lightness), 1 );
        bot_lightness = MAX( lightness - (0.30*lightness), 0 );    
        top_sat = MIN( saturation + (0.30*saturation), 1 );
        bot_sat = MAX( saturation - (0.30*saturation), 0 );
      }                      
    else if ( lightness > 0.5 )
      {
        top_lightness = MIN( lightness + (0.10*lightness), 1 );
        bot_lightness = MAX( lightness - (0.30*lightness), 0 );    
        top_sat = MIN( saturation + (0.25*saturation), 1 );
        bot_sat = MAX( saturation - (0.25*saturation), 0 );
      }                      
    else if ( lightness > 0.3 )
      {
        top_lightness = MIN( lightness + (0.40*lightness), 1 );
        bot_lightness = MAX( lightness - (0.40*lightness), 0 );
        top_sat = MIN( saturation + (0.20*saturation), 1 );
        bot_sat = MAX( saturation - (0.20*saturation), 0 );
      }
    else /* lightness <= 0.30  */
      {
        top_lightness = MIN( lightness + (0.75*lightness), 1 );
        bot_lightness = MAX( lightness - (0.50*lightness), 0 );
        top_sat = MIN( saturation + (0.75*saturation), 1 );
        bot_sat = MAX( saturation - (0.20*saturation), 0 );
      }
    /* Convert back to Rgb */
    mwm_hlstorgb( hue, top_lightness, top_sat,
                  &mwm_cust_info.col[ top_x ].xcol.red,
                  &mwm_cust_info.col[ top_x ].xcol.green,
                  &mwm_cust_info.col[ top_x ].xcol.blue );
    /* Convert back to Rgb */
    mwm_hlstorgb( hue, bot_lightness, bot_sat,
                  &mwm_cust_info.col[ bot_x ].xcol.red,
                  &mwm_cust_info.col[ bot_x ].xcol.green,
                  &mwm_cust_info.col[ bot_x ].xcol.blue );

    sample_fid = mwm_col_sample_fid( sample );
    /* Get the new colors */
    XAllocColor( wmGD.dialog_display, XDefaultColormap( DISPLAY, WID_SCREEN_NUM ),
                 (void *)&mwm_cust_info.col[ top_x ].xcol );
    XAllocColor( wmGD.dialog_display, XDefaultColormap( DISPLAY, WID_SCREEN_NUM ),
                 (void *)&mwm_cust_info.col[ bot_x ].xcol );
    /* Fill in the top and bottom buttons and sample */
    mwm_set( (*mwm_fid)[ WID_SCREEN ][ top_fid ], XmNbackground, 
             (void *)mwm_cust_info.col[ top_x ].xcol.pixel );
    mwm_set( (*mwm_fid)[ WID_SCREEN ][ bot_fid ], XmNbackground, 
             (void *)mwm_cust_info.col[ bot_x ].xcol.pixel );
    mwm_col_reg( (*mwm_fid)[ WID_SCREEN ][ sample_fid ], 
                 sample, top_fid, mwm_cust_info.col[ top_x ].xcol.pixel );
    mwm_col_reg( (*mwm_fid)[ WID_SCREEN ][ sample_fid ], 
                 sample, bot_fid, mwm_cust_info.col[ bot_x ].xcol.pixel );
      }

}

/*******************************************************************
**
**  Description: Return the color index for a field id.
**
**  Formal Parameters
**    Input: field id,
**    Return: color index.
**  
********************************************************************/

int mwm_col_index_get( fid )

int fid;                       

{
/********************************/

    switch ( fid )
      {
        case k_mwm_fore_fid:    
          return( k_mwm_inactive_fore_x );
        case k_mwm_back_fid:
          return( k_mwm_inactive_back_x );
        case k_mwm_top_fid:
          return( k_mwm_inactive_top_x );
        case k_mwm_bot_fid:
          return( k_mwm_inactive_bot_x );
        case k_mwm_active_fore_fid:
          return( k_mwm_active_fore_x );
        case k_mwm_active_back_fid:
          return( k_mwm_active_back_x );                  
        case k_mwm_active_top_fid:  
          return( k_mwm_active_top_x );
        case k_mwm_active_bot_fid:
          return( k_mwm_active_bot_x );   
        case k_mwm_icon_image_fore_fid: 
          return( k_mwm_icon_image_fore_x );
        case k_mwm_icon_image_back_fid:
          return( k_mwm_icon_image_back_x );
        case k_mwm_icon_image_top_fid:
          return( k_mwm_icon_image_top_x );
        case k_mwm_icon_image_bot_fid:
          return( k_mwm_icon_image_bot_x );
        case k_mwm_icon_box_back_fid:
          return( k_mwm_icon_box_back_x );
        case k_mwm_matte_fore_fid: 
          return( k_mwm_matte_fore_x );
        case k_mwm_matte_back_fid:
          return( k_mwm_matte_back_x );
        case k_mwm_matte_top_fid:
          return( k_mwm_matte_top_x );
        case k_mwm_matte_bot_fid:
          return( k_mwm_matte_bot_x );
        default:
          return( k_mwm_active_fore_x );
          break;
      }

}

/*******************************************************************
**
**  Description: Return the did for the fid.
**
**  Formal Parameters
**    Input: index.
**    Return: did.
**  
********************************************************************/

int mwm_col_did_get( index )

int index;

{
/********************************/

    switch ( index )
      {
        case k_mwm_inactive_fore_x:
        case k_mwm_inactive_back_x:
        case k_mwm_inactive_top_x:
        case k_mwm_inactive_bot_x:
        case k_mwm_active_fore_x:
        case k_mwm_active_back_x:
        case k_mwm_active_top_x:  
        case k_mwm_active_bot_x: return( k_mwm_cust_border_col_did );
        case k_mwm_icon_image_fore_x: 
        case k_mwm_icon_image_back_x:
        case k_mwm_icon_image_top_x:
        case k_mwm_icon_image_bot_x:
        case k_mwm_icon_box_back_x: return( k_mwm_cust_icon_col_did );
        case k_mwm_matte_fore_x: 
        case k_mwm_matte_back_x:
        case k_mwm_matte_top_x:                       
        case k_mwm_matte_bot_x: return( k_mwm_cust_matte_did );
        default: return( k_mwm_none );
      }

}
   
/*******************************************************************
**
**  Description: Get a color resource value.
**
**  Formal Parameters
**    Input: widget id,               
**           resource,
**           address of value.
**  
********************************************************************/

void mwm_col_get( wid, resource, value )       

Widget wid;                      
char *resource;
XColor *value;

{
/* local variables */
Arg arg_list[ 1 ];

/********************************/

    if ( wid != NULL )
      {
        XtSetArg( arg_list[ 0 ], resource, value );
        XtGetValues( wid, arg_list, 1 );
      }

}

/*******************************************************************
**
**  Description:  Bring down color mix if auto shadow is turned off
**                and the color mix box is up for something that is
**                being turned off.
**
**  Formal Parameters
**    Input: wid,
**           fid.
**
********************************************************************/

void mwm_col_mix_auto_chec( wid, fid )

Widget wid;
int fid;

/* local variables */
{
int cancel, did;

/********************************/

    /* Which color mix box is up ? */
    switch ( mwm_cust_info.mix_box[ WID_SCREEN ].index )
      /* Check if it matches the auto shadow toggle that is now off. */
      {
        case k_mwm_inactive_top_x:
        case k_mwm_inactive_bot_x: 
          cancel = fid == k_mwm_inactive_auto_shade_fid;
          break;
        case k_mwm_active_top_x:  
        case k_mwm_active_bot_x:
          cancel = fid == k_mwm_active_auto_shade_fid;
          break;
        case k_mwm_icon_image_top_x:
        case k_mwm_icon_image_bot_x:
          cancel = fid == k_mwm_image_auto_shade_fid;
          break;
        case k_mwm_icon_box_back_x:
          cancel = fid == k_mwm_icon_box_auto_shade_fid;
          break;
        case k_mwm_matte_top_x:                       
        case k_mwm_matte_bot_x:
          cancel = fid == k_mwm_matte_auto_shade_fid;
          break;
        default: 
          cancel = false;
          break;
      }
    /* Cancel it ? */
    if ( cancel )
      /* Yup */
      {
        did = k_mwm_cust_col_mix_did;
        mwm_dialog_cancel( wid, &did, NULL );
      }

}

/*******************************************************************
**
**  Description: The auto shadow information has been toggled.
**               Reset the fields.
**
**  Formal Parameters
**    Input: wid,
**           fid.
**
********************************************************************/
                                 
void mwm_col_auto_chec( wid, fid )

Widget wid;
int fid;

/* local variables */
{
Widget top_wid, bot_wid, top_label_wid, bot_label_wid, button_wid;
int state, sample;

/********************************/

    /* Reset the state */
    switch ( fid )
      {
        case k_mwm_inactive_sample_fid:
          fid = k_mwm_inactive_auto_shade_fid;
        case k_mwm_inactive_auto_shade_fid:
          mwm_cust_info.inactive_auto_shade =  
            XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ fid ] );
          state = mwm_cust_info.inactive_auto_shade;
          top_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_top_fid ];
          bot_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_bot_fid ];
          top_label_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_top_label_fid ];
          bot_label_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_bot_label_fid ];
          if ( state )
              mwm_col_auto_set( wid, k_mwm_inactive_top_x, k_mwm_inactive_bot_x,
                                k_mwm_inactive_back_x, k_mwm_top_fid, 
                                k_mwm_bot_fid, k_mwm_inactive_sample );
          break;
        case k_mwm_icon_box_auto_shade_fid:     
          mwm_cust_info.icon_box_auto_shade =            
            XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ fid ] );
          state = mwm_cust_info.icon_box_auto_shade;
          top_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_box_back_fid ];
          top_label_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_box_back_label_fid ];
          if ( state )
              mwm_col_auto_set( wid, k_mwm_icon_box_back_x, k_mwm_icon_box_back_x,
                                k_mwm_icon_box_back_x, k_mwm_icon_box_back_fid,
                                k_mwm_icon_box_back_fid, k_mwm_icon_box_sample );
          break;
        case k_mwm_active_sample_fid:
          fid = k_mwm_active_auto_shade_fid;
        case k_mwm_active_auto_shade_fid:
          mwm_cust_info.active_auto_shade =  
            XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ fid ] );
          state = mwm_cust_info.active_auto_shade;
          top_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_active_top_fid ];
          bot_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_active_bot_fid ];
          top_label_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_active_top_label_fid ];
          bot_label_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_active_bot_label_fid ];
          if ( state )
              mwm_col_auto_set( wid, k_mwm_active_top_x, k_mwm_active_bot_x,
                                k_mwm_active_back_x, k_mwm_active_top_fid, 
                                k_mwm_active_bot_fid, k_mwm_active_sample );
          break;
        case k_mwm_icon_image_sample_fid:
          fid = k_mwm_image_auto_shade_fid;
        case k_mwm_image_auto_shade_fid:
          mwm_cust_info.image_auto_shade =  
            XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ fid ] );
          state = mwm_cust_info.image_auto_shade;
          top_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_image_top_fid ];
          bot_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_image_bot_fid ];
          top_label_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_image_top_label_fid ];
          bot_label_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_icon_image_bot_label_fid ];
          if ( state )
              mwm_col_auto_set( wid, k_mwm_icon_image_top_x, k_mwm_icon_image_bot_x,
                                k_mwm_icon_image_back_x, k_mwm_icon_image_top_fid, 
                                k_mwm_icon_image_bot_fid, k_mwm_icon_image_sample );
          break;
        case k_mwm_matte_sample_fid:
          fid = k_mwm_matte_auto_shade_fid;
        case k_mwm_matte_auto_shade_fid:
          mwm_cust_info.matte_auto_shade =  
            XmToggleButtonGetState( (*mwm_fid)[ WID_SCREEN ][ fid ] );
          state = mwm_cust_info.matte_auto_shade;
          top_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_top_fid ];
          bot_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_bot_fid ];
          top_label_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_top_label_fid ];
          bot_label_wid = (*mwm_fid)[ WID_SCREEN ][ k_mwm_matte_bot_label_fid ];
          if ( state )
              mwm_col_auto_set( wid, k_mwm_matte_top_x, k_mwm_matte_bot_x,
                                k_mwm_matte_back_x, k_mwm_matte_top_fid, 
                                k_mwm_matte_bot_fid, k_mwm_matte_sample );
          break;
        default:
          break;
      }
                                            
    /* Bring down color mix if auto shadow is turned off
       and the color mix box is up for something that is
       being turned off. */
    /* Was auto shadow set ? */
    if ( state )                          
        mwm_col_mix_auto_chec( wid, fid );

    /* Reset the fields to sensitive or not sensitive */
    XtSetSensitive( top_wid, (Boolean)(!state) );
    XtSetSensitive( top_label_wid, (Boolean)(!state) );
    if ( fid != k_mwm_icon_box_auto_shade_fid )
      {
        XtSetSensitive( bot_wid, (Boolean)(!state) );     
        XtSetSensitive( bot_label_wid, (Boolean)(!state) );
      }                    

}
             
/*******************************************************************/

/* Sample module */

/*******************************************************************
**
**  Description: Color a sample region.
**
**  Formal Parameters
**    Input: widget id,               
**           sample id,
**           field id,
**           color.
**  
********************************************************************/

void mwm_col_reg( wid, sample, fid, col )

Widget wid;
int sample;
int fid;
Pixel col;

{
/* local variables */
XPoint *points;
int n_points;
char fore_text[ 31 ];
XTextItem text;
int x, y, sample_delta;                   
Region region;
XGCValues values;

/********************************/

    switch ( fid ) 
      {
        case k_mwm_top_fid:
        case k_mwm_matte_top_fid:
        case k_mwm_active_top_fid:
        case k_mwm_icon_image_top_fid:
          points = &top_points[ 0 ];
          n_points = 6;
          break;
        case k_mwm_back_fid:
        case k_mwm_matte_back_fid:
        case k_mwm_active_back_fid:
        case k_mwm_icon_image_back_fid:
          points = &back_points[ 0 ];
          n_points = 4;
          break;
        case k_mwm_icon_box_back_fid:  
          points = &full_points[ 0 ];
          n_points = 4;
          break;
        case k_mwm_bot_fid:
        case k_mwm_matte_bot_fid:
        case k_mwm_active_bot_fid:
        case k_mwm_icon_image_bot_fid:
 	  points = &bot_points[ 0 ];
          n_points = 6;
          break;
        /* Draw the foreground text. */
        case k_mwm_active_fore_fid:
          return;
/*          strcpy( fore_text, "Title" );
          text.chars = fore_text;
          text.nchars = strlen( fore_text );
          text.delta = 0;
          text.font = None;
          XSetForeground( wmGD.dialog_display, mwm_col_gc[ sample ], col );
          sample_delta = mwm_col_sample_width / 10;   
          x = sample_delta *2;
          y = sample_delta * 2;
          XDrawText( wmGD.dialog_display, XtWindow( wid ),
                     (*mwm_col_gc)[ WID_SCREEN ][ sample ], x, y, &text, 1 );
          return;
*/        default:
          return;
      }
    
    region = XPolygonRegion( points, n_points, EvenOddRule );
    /* Does the gc exist ? */
    if ( !(*mwm_col_gc)[ WID_SCREEN ][ sample ] ) 
      {
        values.line_width = 3;
        /* No, create it */    
        (*mwm_col_gc)[ WID_SCREEN ][ sample ] = 
            XCreateGC( wmGD.dialog_display, 
                       XRootWindow( wmGD.dialog_display, WID_SCREEN_NUM ),
                       GCLineWidth, &values );
      }
    /* Set the color */
    XSetForeground( wmGD.dialog_display, (*mwm_col_gc)[ WID_SCREEN ][ sample ], col );

    /* Create it. */
    XSetRegion( wmGD.dialog_display, (*mwm_col_gc)[ WID_SCREEN ][ sample ], region );

    /* Does the pixmap exist ? */
    if ( !(*mwm_col_pix)[ WID_SCREEN ][ sample ] )
      {
        (*mwm_col_pix)[ WID_SCREEN ][ sample ] =  
            XCreatePixmap( wmGD.dialog_display, 
                           XRootWindow( wmGD.dialog_display, WID_SCREEN_NUM ),
                           mwm_col_sample_width, mwm_col_sample_height, 
                           XDefaultDepth( wmGD.dialog_display, WID_SCREEN_NUM ));
      }
    /* Fill and copy it */
    XFillRectangle( wmGD.dialog_display, (*mwm_col_pix)[ WID_SCREEN ][ sample ], 
                    (*mwm_col_gc)[ WID_SCREEN ][ sample ], 0, 0, 
           	     mwm_col_sample_width, mwm_col_sample_height );
    XCopyArea( wmGD.dialog_display, (*mwm_col_pix)[ WID_SCREEN ][ sample ], XtWindow( wid ),
               XDefaultGC( wmGD.dialog_display, WID_SCREEN_NUM ), 0, 0,
       	       mwm_col_sample_width, mwm_col_sample_height, 0, 0 );
    /* Free the region */
    XDestroyRegion( region );

}

/*******************************************************************
**
**  Description: Initialize the sample size.
**
**  Formal Parameters
**    Input: widget id.
**  
********************************************************************/

void mwm_col_sample_init( wid )

Widget wid;

/* local variables */
{
Arg arg_list[ 2 ];
int n;
int sample_delta;                                                          

/********************************/

    /* Has the sample information been initialized ? */
    if ( mwm_col_sample_height != 0 )
        /* Yes return */
        return;

    /* Get the height and width of the sample area */
    n = 0;
    XtSetArg( arg_list[ n ], XtNwidth, &mwm_col_sample_width ); n++;
    XtSetArg( arg_list[ n ], XtNheight, &mwm_col_sample_height ); n++;
    XtGetValues( wid, arg_list, n );
    mwm_col_sample_width = XmConvertUnits( wid, XmHORIZONTAL, 
                                                Xm100TH_FONT_UNITS,
                                                mwm_col_sample_width, XmPIXELS );
    mwm_col_sample_height = XmConvertUnits( wid, XmVERTICAL,
                                                 Xm100TH_FONT_UNITS,
                                                 mwm_col_sample_height, XmPIXELS );
    sample_delta = mwm_col_sample_width / 10;   

    /* Fill in the top points */
    top_points[ 0 ].y = mwm_col_sample_height;
    top_points[ 2 ].x = mwm_col_sample_width;
    top_points[ 3 ].x = mwm_col_sample_width - sample_delta;
    top_points[ 3 ].y = sample_delta;
    top_points[ 4 ].x = sample_delta;
    top_points[ 4 ].y = sample_delta;
    top_points[ 5 ].x = sample_delta;
    top_points[ 5 ].y = mwm_col_sample_height - sample_delta;

    /* Fill in the bottom points */
    bot_points[ 0 ].y = mwm_col_sample_height;
    bot_points[ 1 ].x = sample_delta;
    bot_points[ 1 ].y = mwm_col_sample_height - sample_delta;
    bot_points[ 2 ].x = mwm_col_sample_width - sample_delta;
    bot_points[ 2 ].y = mwm_col_sample_height - sample_delta;
    bot_points[ 3 ].x = mwm_col_sample_width - sample_delta;
    bot_points[ 3 ].y = sample_delta;
    bot_points[ 4 ].x = mwm_col_sample_width;
    bot_points[ 5 ].x = mwm_col_sample_width;
    bot_points[ 5 ].y = mwm_col_sample_height;

    /* Fill in the full points */
    full_points[ 1 ].x = mwm_col_sample_width + sample_delta*2;
    full_points[ 2 ].x = mwm_col_sample_width + sample_delta*2;
    full_points[ 2 ].y = mwm_col_sample_height + sample_delta*2;
    full_points[ 3 ].y = mwm_col_sample_height + sample_delta*2;

    /* Fill in the background points */
    back_points[ 0 ].x = sample_delta;
    back_points[ 0 ].y = sample_delta;
    back_points[ 1 ].x = mwm_col_sample_width - sample_delta;
    back_points[ 1 ].y = sample_delta;
    back_points[ 2 ].x = mwm_col_sample_width - sample_delta;
    back_points[ 2 ].y = mwm_col_sample_height - sample_delta;
    back_points[ 3 ].x = sample_delta;
    back_points[ 3 ].y = mwm_col_sample_height - sample_delta;

}

/*******************************************************************
**
**  Description: Create a sample region.
**
**  Formal Parameters
**    Input: widget id,               
**           fid.
**  
********************************************************************/

void mwm_col_sample_crea( wid, fid )

Widget wid;
int fid;

/* local variables */
{
int fore, back, top, bot, sample;

/********************************/
     
    (*mwm_fid)[ WID_SCREEN ][ fid ] = wid;

    /* Initialize the sample information */
    mwm_col_sample_init( wid );
    switch ( fid )
      {
        case k_mwm_active_sample_fid:
          fore = k_mwm_active_fore_fid;
          back = k_mwm_active_back_fid;
          top = k_mwm_active_top_fid;
          bot = k_mwm_active_bot_fid;
          sample = k_mwm_active_sample;
          break;
        case k_mwm_inactive_sample_fid:
          fore = k_mwm_fore_fid;
          back = k_mwm_back_fid;
          top = k_mwm_top_fid;
          bot = k_mwm_bot_fid;
          sample = k_mwm_inactive_sample;
          break;
        case k_mwm_matte_sample_fid:
          fore = k_mwm_matte_fore_fid;
          back = k_mwm_matte_back_fid;
          top = k_mwm_matte_top_fid;
          bot = k_mwm_matte_bot_fid;
          sample = k_mwm_matte_sample;
          break;
        case k_mwm_icon_box_sample_fid:
          back = k_mwm_icon_box_back_fid;
          sample = k_mwm_icon_box_sample;
          break;
        case k_mwm_icon_image_sample_fid:
          fore = k_mwm_icon_image_fore_fid;
          back = k_mwm_icon_image_back_fid;
          top = k_mwm_icon_image_top_fid;
          bot = k_mwm_icon_image_bot_fid;
          sample = k_mwm_icon_image_sample;
          break;
        default:
          return;
          break;                                             
      }
        
    /* Color each area. */
    /* If there is more than one string, get the color for
       this colormap */
    XAllocColor( wmGD.dialog_display, XDefaultColormap( DISPLAY, WID_SCREEN_NUM ),
                 &mwm_cust_info.col[ mwm_col_index_get( back ) ].xcol);
    mwm_col_reg( wid, sample, back,
                 (Pixel)mwm_cust_info.col[ mwm_col_index_get( back )].xcol.pixel );
    /* Is this the icon box ? */
    if ( fid != k_mwm_icon_box_sample_fid )    
      /* No, color the rest of the areas. */
      {
        XAllocColor( wmGD.dialog_display, XDefaultColormap( DISPLAY, WID_SCREEN_NUM ),
                     &mwm_cust_info.col[ mwm_col_index_get( fore ) ].xcol);
        mwm_col_reg( wid, sample, fore,
                     (Pixel)mwm_cust_info.col[ mwm_col_index_get( fore ) ].xcol.pixel );
        XAllocColor( wmGD.dialog_display, XDefaultColormap( DISPLAY, WID_SCREEN_NUM ),
                     &mwm_cust_info.col[ mwm_col_index_get( top ) ].xcol);
        mwm_col_reg( wid, sample, top, 
                     (Pixel)mwm_cust_info.col[ mwm_col_index_get( top ) ].xcol.pixel );
        XAllocColor( wmGD.dialog_display, XDefaultColormap( DISPLAY, WID_SCREEN_NUM ),
                     &mwm_cust_info.col[ mwm_col_index_get( bot ) ].xcol);
        mwm_col_reg( wid, sample, bot,                                   
                     (Pixel)mwm_cust_info.col[ mwm_col_index_get( bot ) ].xcol.pixel );
      }

}

/*******************************************************************
**
**  Description: Return the sample fid from the sample type.
**
**  Formal Parameters
**    Input: sample type.
**  Return:  sample fid.
**  
********************************************************************/

int mwm_col_sample_fid( sample )

int sample;

/* local variables */
{

/********************************/
     
    switch ( sample )
      {
        case k_mwm_active_sample: return( k_mwm_active_sample_fid );
        case k_mwm_inactive_sample: return( k_mwm_inactive_sample_fid );
        case k_mwm_icon_box_sample: return( k_mwm_icon_box_sample_fid );
        case k_mwm_icon_image_sample: return( k_mwm_icon_image_sample_fid );
        case k_mwm_matte_sample: return( k_mwm_matte_sample_fid );
        default: return( k_mwm_active_sample_fid );
      }

}

/*******************************************************************
**
**  Description: Return the sample type from the fid.
**
**  Formal Parameters
**    Input: fid,
**  Return:  sample type.
**  
********************************************************************/
                 
int mwm_col_sample( fid )

int fid;

/* local variables */
{

/********************************/
     
    switch ( fid )
      {
        case k_mwm_fore_fid:
        case k_mwm_back_fid:
        case k_mwm_top_fid:
        case k_mwm_bot_fid: 
          return( k_mwm_inactive_sample );

        case k_mwm_icon_image_fore_fid:
        case k_mwm_icon_image_back_fid:
        case k_mwm_icon_image_top_fid:
        case k_mwm_icon_image_bot_fid:
          return( k_mwm_icon_image_sample );

        case k_mwm_icon_box_back_fid:
          return( k_mwm_icon_box_sample );

        case k_mwm_active_fore_fid:
        case k_mwm_active_back_fid:
        case k_mwm_active_top_fid:
        case k_mwm_active_bot_fid:
          return( k_mwm_active_sample );

        case k_mwm_matte_bot_fid:
        case k_mwm_matte_fore_fid:
        case k_mwm_matte_back_fid:
        case k_mwm_matte_top_fid:
        default:
          return( k_mwm_matte_sample );
      }

}

/*******************************************************************
**
**  Description: A region was exposed. Update it.
**
**  Formal Parameters
**    Input: widget id,               
**           resource,
**           reason.
**  
********************************************************************/

void mwm_col_expo( wid, tag, callback_data )

Widget	wid;
int *tag;
XmAnyCallbackStruct *callback_data;

/* local variables */
{                                 
int fid = *tag;
int index;
Widget temp_wid;

/********************************/
                        
    switch ( fid )
      {
        case k_mwm_active_sample_fid:
          index = k_mwm_active_sample;
          break;
        case k_mwm_inactive_sample_fid:
          index = k_mwm_inactive_sample;
          break;
        case k_mwm_matte_sample_fid:
          index = k_mwm_matte_sample;
          break;
        case k_mwm_icon_image_sample_fid:
          index = k_mwm_icon_image_sample;
          break;
        case k_mwm_icon_box_sample_fid:
          index = k_mwm_icon_box_sample;
          break;
      }

    XCopyArea( DISPLAY, (*mwm_col_pix)[ WID_SCREEN ][ index ],
               XtWindow( (*mwm_fid)[ WID_SCREEN ][ fid ]),
               XDefaultGC( wmGD.dialog_display, WID_SCREEN_NUM ),
               0, 0, mwm_col_sample_width, mwm_col_sample_height, 0, 0 );

}
      
/*******************************************************************/
    
/* Mix module */

/*******************************************************************
**
**  Description: Get the color mix message.
**
**  Formal Parameters
**    Input: widget id.
**  
********************************************************************/

void mwm_col_mix_mess_set( wid )

Widget wid;

{
/* local variables */
int fid;
XmString *message;

/********************************/
             
    /* Get the string */
    if ( mwm_dialog_text_get(        
           mwm_mix_mess[ mwm_cust_info.mix_box[ WID_SCREEN ].index ],
           (XmString *)&mwm_mix_str[ mwm_cust_info.mix_box[ WID_SCREEN ].index ] ))
      {
          mwm_set( wid, DXmNdisplayLabel,
                   mwm_mix_str[ mwm_cust_info.mix_box[ WID_SCREEN ].index ] );
      }

}

/*******************************************************************
**
**  Description: A color button was selected.
**               Bring up the color mix widget.
**
**  Formal Parameters
**    Input: widget id,               
**           resource,
**           reason.
**  
********************************************************************/

void mwm_col_mix_up( wid, tag, reason )

Widget wid;
int *tag;
unsigned int *reason;
                        
{
/* local variables */
int fid = *tag, status;
int did, old_index;
                                                    
/********************************/

    did = k_mwm_cust_col_mix_did;   

    /* Bring up the dialog box. */
                                                      
    /* Get the previous index.
       Was the customize mix box ever managed ? */
    if ( (*mwm_screen_did)[ WID_SCREEN ][ did ].wid == (Widget)NULL )
        /* Nope */
        old_index = k_mwm_none;
    /* Yup, save the index */
    else old_index = mwm_cust_info.mix_box[ WID_SCREEN ].index;

    /* Update the new index */
    mwm_cust_info.mix_box[ WID_SCREEN ].index = mwm_col_index_get( fid );

    /* Bring up and update the dialog box */
    if ( mwm_dialog_get( WID_SCREEN, 
                         &(*mwm_screen_did)[ WID_SCREEN ][ did ].wid, 
                         did, mwm_did[ did ].name, 
                         mwm_dialog_shell[ WID_SCREEN ] ))
      {                                                            
        mwm_cust_info.init[ did ] = True;
        if ( mwm_cust_info.mix_box[ WID_SCREEN ].index != old_index )
          /* Yup, save the fields */
          {
            /* Save the index */    
            mwm_cust_info.mix_box[ WID_SCREEN ].button_fid = fid;   
            mwm_cust_info.mix_box[ WID_SCREEN ].sample = mwm_col_sample( fid );
            mwm_cust_info.mix_box[ WID_SCREEN ].sample_fid = mwm_col_sample_fid( mwm_cust_info.mix_box[ WID_SCREEN ].sample );
            mwm_cust_info.mix_box[ WID_SCREEN ].button_wid = wid;
          }
        status = mwm_dialog_up( (*mwm_screen_did)[ WID_SCREEN ][ did ].wid, 
                                 did, mwm_dialog_shell[ WID_SCREEN ] );
        mwm_col_mix_mess_set( (*mwm_screen_did)[ WID_SCREEN ][ did ].wid );
        /* If it is up, reset it. Note it should be up. */
        if ( mwm_did[ did ].up )
            mwm_dialog_rese( did, mwm_fid_list[ did ] );
      } 
    /* Make sure the cursor is reset in case of an error. */
    mwm_watch_set( false );
        
}

/*******************************************************************
**
**  Description: Initialize the color mix widget.
**
**  Formal Parameters
**    Input: wid.
**  
********************************************************************/

void mwm_col_mix_set( wid )

Widget wid;

{
/* local variables */
int index;

/********************************/

    index = mwm_cust_info.mix_box[ WID_SCREEN ].index;
    mwm_set( wid, DXmNnewRedValue,
             (void *)mwm_cust_info.col[ index ].xcol.red );
    mwm_set( wid, DXmNnewGreenValue,
             (void *)mwm_cust_info.col[ index ].xcol.green );
    mwm_set( wid, DXmNnewBlueValue, 
             (void *)mwm_cust_info.col[ index ].xcol.blue );
    mwm_set( wid, DXmNorigRedValue,
             (void *)mwm_cust_info.col[ index ].xcol.red );
    mwm_set( wid, DXmNorigGreenValue,
             (void *)mwm_cust_info.col[ index ].xcol.green );
    mwm_set( wid, DXmNorigBlueValue, 
             (void *)mwm_cust_info.col[ index ].xcol.blue );

}

/*******************************************************************
**
**  Description: The color mix widget was OKed.
**
**  Formal Parameters
**    Input: widget id,               
**           tag,
**           cbs.
**  
********************************************************************/

int mwm_col_mix_ok( wid, tag, cbs )

Widget wid;
int *tag;
DXmColorMixCallbackStruct *cbs;
                               
{
/* local variables */
Arg arglist[5];
Widget button_wid;                   
int index, sample_fid, button_fid;

/********************************/
                                  
    index = mwm_cust_info.mix_box[ WID_SCREEN ].index;
    button_wid = mwm_cust_info.mix_box[ WID_SCREEN ].button_wid;
    sample_fid = mwm_cust_info.mix_box[ WID_SCREEN ].sample_fid;
    button_fid = mwm_cust_info.mix_box[ WID_SCREEN ].button_fid;
    mwm_cust_info.col[ index ].xcol.red = cbs->newred;
    mwm_cust_info.col[ index ].xcol.green = cbs->newgrn;
    mwm_cust_info.col[ index ].xcol.blue = cbs->newblu;
    /* Was a named color selected ? */                       
    if ( cbs->newname != NULL )
      /* Yes, save the colors */
      {
        if ( mwm_cust_info.col[ index ].name == NULL )
            /* Allocate the color name field */        
            if ( ! mwm_alloc( (void *)&mwm_cust_info.col[ index ].name, 
                k_mwm_max_color_name_size, "Error allocating color name data" ))
                return;
        /* Save the name */
        strcpy( mwm_cust_info.col[ index ].name, cbs->newname );
      }  
    /* No, it's an rbg value */
    else
      {
        if ( mwm_cust_info.col[ index ].name != NULL )
            strcpy( mwm_cust_info.col[ index ].name, "" );
      }
    XAllocColor( wmGD.dialog_display, XDefaultColormap( DISPLAY, WID_SCREEN_NUM ),
                 &mwm_cust_info.col[ index ].xcol);

    /* Save the new color. */
    /* fill push button with current color */
    if ( button_wid != NULL )
      {
        /* Fill in button */   
        mwm_set( button_wid, XmNbackground, (void *)mwm_cust_info.col[ index ].xcol.pixel );
        /* fill sample area with new color */
        mwm_col_reg( (*mwm_fid)[ WID_SCREEN ][ sample_fid ], 
                     mwm_cust_info.mix_box[ WID_SCREEN ].sample,
                     mwm_cust_info.mix_box[ WID_SCREEN ].button_fid, 
                     (Pixel)mwm_cust_info.col[ index ].xcol.pixel );
      }
    
    /* If this was a background color, the check the shadow colors */
    switch ( button_fid )
      {
        case k_mwm_back_fid:
        case k_mwm_icon_image_back_fid:
        case k_mwm_matte_back_fid: 
        case k_mwm_active_back_fid:
          mwm_col_auto_chec( wid, sample_fid );
          break;
        default:
          break;
      }

    return( true );

}

/*******************************************************************/
