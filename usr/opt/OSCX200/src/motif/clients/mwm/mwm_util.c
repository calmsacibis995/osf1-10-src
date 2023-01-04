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
**  Abstract: Window Manager General routines
**
**  Environment: VMS, Unix, Sun                  
**
*******************************************************************/

#include "WmGlobal.h"
#include "WmResNames.h"
#include "WmFunction.h"
#include <Xm/Xm.h>
#include <Xm/XmosP.h>
#include <Xm/Text.h>
#include <X11/ShellP.h>
#include <X11/Shell.h>
#include <Xm/DialogSP.h>
#include <Mrm/MrmPublic.h>                                                  
#include <DXm/DECspecific.h>                                                  
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "WmError.h"
#include "WmFeedback.h"

#include "mwm_cust.h"
#include "mwm_col.h"
#include "mwm_internal.h"
#include "mwm_util_forward.h"
#include "mwm_dialog.h"
#include "mwm_col.h"
#include "mwm_help.h"
#include "mwm_util.h"
                                                   
/* Initialization module */

/*******************************************************************
**                                                                    
**  Description: Initialize the UIL file.
**
**  Formal Parameters
**  
********************************************************************/

int mwm_uil_init()

/* local variables */
{

/********************************/

    if ( ! mwm_hierarchy_init )
      /* Prepare hierarchy for UID file */
      {
        DXmInitialize();
        MrmInitialize();
      }

    /* Open the hierarchy and initialize the customize structure. */
    if ( ! mwm_hierarchy_init )
      {
        if ( MrmOpenHierarchy( 1, mwm_uil_file, NULL, &mwm_hierarchy ) != MrmSUCCESS ) 
          {                                                                   
            fprintf( stderr, 
   "The mwm UID file, %s, could not be opened.\nPlease contact your system manager.\n",
                    *mwm_uil_file );
            return( false );
          }
      }
    
    mwm_hierarchy_init = true;
    return( true );

}

/*******************************************************************
**                                                                    
**  Description: Initialize the customize and help and dialog
**               information.
**
**  Formal Parameters
**    Input: screen index
**           screen num.
**  
********************************************************************/

int mwm_init( screen, screen_num )

int screen;
int screen_num;
/* local variables */
{
Boolean ok;

/********************************/

    ok = false;
    /* Has the UIL and the screen already been initialized ?  Return true
       if the hierarchy is valid. */
    if ( mwm_hierarchy_init )
        if ( mwm_dialog_shell != NULL )                                 
            if ( mwm_dialog_shell[ screen ] != NULL )    
                return( mwm_hierarchy != NULL );

    ShowWaitState( true );

    /* Initialize, register callbacks, etc. */
    if ( mwm_uil_init() )
        /* Initialize callbacks */
        if ( mwm_dialog_init() )
            /* Initialize help */
            if ( mwm_help_init() )
                /* Initialize customize color */
                if ( mwm_col_init() )
                  {
                    mwm_cb_init = true;
                    /* Initialize customize info */
                    if ( mwm_cust_info_init() )
                        /* Initialize the shell */
                        if ( mwm_dialog_shell_init( screen, screen_num ))
                            ok = true;
                  }
 
    /* Any errors ? */
    if ( ! ok )
        /* Yes, reset the cursor. */
        ShowWaitState( false );
    return( ok );

}

/*******************************************************************/

/* Utility module */

/*******************************************************************
**
**  Description: Get the subresources from the correct resource file.
**
**  Formal Parameters
**    Input: string,
**           value,
**    Return: true if the same.
**  
********************************************************************/

void mwm_subres_get( pSD, res_list, num_res, addr )

WmScreenData *pSD;
XtResource res_list[];
int num_res;
caddr_t addr;

/* local variables */
{
int i, type;
caddr_t def;

/********************************/

    /* Is this b&w or gray-scale  ? */
    if ( pSD->reset_resources )
      /* Yes, override the resources */
      {
        ACTIVE_PSD = pSD;
        for ( i = 0; i < num_res; i++ )
          {
            /* Set the type */
            /* Is it a pixel value ? */    
            if ( !strcmp( res_list[ i ].resource_type, XtRPixel ))
                type = k_mwm_res_pixel;  
            /* Is it a string ? */
            else if ( !strcmp( res_list[ i ].resource_type, XtRString ))
                type = k_mwm_res_alloc_string;
            /* Is it an integer ? */                           
            else if ( !strcmp( res_list[ i ].resource_type, XtRInt ))
                continue;
            /* Is it a boolean ? */
            else if ( !strcmp( res_list[ i ].resource_type, XtRBoolean ))
                type = k_mwm_subres_boolean;                                 
            else continue;

            /* Is a default specified ? */
            if ( !strcmp( res_list[ i ].default_type, XtRCallProc ))
                /* Nope */
                def = (caddr_t)NULL;
            else def = (caddr_t)(addr+res_list[ i ].resource_offset);

            /* Is the value in the user database ? */
            if ( !mwm_res_get( pSD, pSD->user_database, res_list[ i ].resource_name,
                               type, 
                               (addr+res_list[ i ].resource_offset), def ))
                /* No, check the system database */
                mwm_res_get( pSD, pSD->sys_database, res_list[ i ].resource_name,
                             type, 
                             (addr+res_list[ i ].resource_offset), def );
          }
      }

}

/*******************************************************************
**
**  Description: Remove training and leading whitespace.
**
**  Formal Parameters
**    I/O:   string.
**
********************************************************************/

void mwm_str_ws( string )

char *string;

/* local variables */
{
char *buffer;
int len, save_len;

/********************************/

    /* Remove leading whitespace */
    if ( !strstr( string, k_mwm_space_str ) &&
         !strstr( string, k_mwm_tab_str ))
        return;

    buffer = string;
    if ( isspace( *buffer ))
      {
        while( isspace( *buffer ) )
            buffer++;
        strcpy( string, buffer );
      }

    /* Remove trailing whitespace. */
    save_len = len = strlen( string );
    buffer = string + len - 1;
    while ( len-- )
      {
        if (!isspace(*buffer))
            break;
        *buffer-- = '\0';
      }
    
}

/*******************************************************************
**
**  Description: Compare two strings.  Use a case insensitive compare.
**
**  Formal Parameters
**    Input: string,
**           value,
**    Return: true if the same.
**  
********************************************************************/

Boolean mwm_str_eq( string, value )

char *string;
char *value;

/* local variables */
{
int i;
char temp[ 128 ];

/********************************/
                      
    /* Are these valid strings ? */
    if (( strlen( value ) == 0 ) ||
        ( strlen( string ) == 0 )) 
        return( false );

    /* Convert the value to lowercase. */
    strcpy( temp, value );
    for ( i = 0; i < strlen( temp ); i++ )
      {
       temp[ i ] = tolower( temp[ i ] );
      }

    /* Convert the string to lowercase */
    for ( i = 0; i < strlen( string ); i++ )
      {
       string[ i ] = tolower( string[ i ] );
      }
 
    /* Compare the string to the value */
    return( strcmp( string, temp ) == 0 );

}

/*******************************************************************
**
**  Description: Change all leading blanks to 0s.
**
**  Formal Parameters
**    Input: string,
**  
********************************************************************/

int mwm_str_0( string )

char *string;

/* local variables */
{
char *temp;

/********************************/
                      
    /* Find a blank */
    temp = (char *)strstr( string, " " );
    /* Was a blank found ? */
    while ( temp != NULL )
      /* Yes, substitute "0" and find the next one. */
      {
        temp[ 0 ] = '0';      
        temp ++;
        temp = (char *)strstr( temp, " " );
      }

}

/*******************************************************************
**
**  Description: Search for a substring.  Use a case insensitive compare.
**               Return the address if it is found.
**
**  Formal Parameters
**    Input: string,
**           value,
**    Return: address if found.
**  
********************************************************************/

char *mwm_str_get( string, value )
                                 
char *string;
char *value;

/* local variables */
{
int i;
char temp[ 128 ];
char *low;

/********************************/

    /* Are these valid strings ? */
    if (( strlen( value ) == 0 ) ||
        ( strlen( string ) == 0 )) 
        return( string );
                   
    /* Convert the value to lowercase. */
    strcpy( temp, value );
    low = temp;
    for ( i = 0; i < strlen( temp ); i++ )
      {
       *low = tolower( *low );
       low++;
      }     

    /* Convert the string to lowercase */
    low = string;
    for ( i = 0; i < strlen( string ); i++ )
      {
       *low = tolower( *low );
       low++;
      }

    return( (char *)strstr( string, temp ));

}

/*******************************************************************
**
**  Description: Set a resource in the resource database.
**
**  Formal Parameters
**    Input: database,
**           resource,
**           resource type,
**           value.
**  
********************************************************************/

void mwm_res_set( database, resource, type, value )

XrmDatabase *database;
char *resource;
int type;                         
void *value;

/* local variables */
{
char spec[ 256 ];
char string[ 128 ];
char color[ 8 ];
Boolean named_col;
                               
/********************************/

    if ( database == NULL )
        return;
    strcpy( spec, k_mwm_prefix_str );
    strcat( spec, resource );

    /* Is it a boolean ? */
    if ( type == k_mwm_res_boolean )
      {
        if ( *(Boolean *)value )                          
            XrmPutStringResource( database, spec, k_mwm_true_str );
        else XrmPutStringResource( database, spec, k_mwm_false_str );
      }                 
    /* Is it a string ? */
    else if ( type == k_mwm_res_string )
        XrmPutStringResource( database, spec, (char *)value );
    /* Is it a col ? */
    else if ( type == k_mwm_res_col )
      {
        named_col = false;
        if ( ((mwm_col_type *)value)->name != NULL )
            if ( strlen( ((mwm_col_type *)value)->name ) > 0 )
                named_col = true;

        /* Is this a named color ? */
        if ( named_col )
            /* Yes, write out the named color */
            XrmPutStringResource( database, spec, ((mwm_col_type *)value)->name );
        /* No, convert the value to an rgb string */
        else
          {
            strcpy( string, "#" );
            if ( ((mwm_col_type *)value)->xcol.red == 0 )
                strcpy( color, k_mwm_hex_0_str );
            else sprintf( color, "%4X", ((mwm_col_type *)value)->xcol.red );    
            strcat( string, color );

            if ( ((mwm_col_type *)value)->xcol.green == 0 )
                strcpy( color, k_mwm_hex_0_str );
            else sprintf( color, "%4X", ((mwm_col_type *)value)->xcol.green );    
            strcat( string, color );              

            if (( (mwm_col_type *)value)->xcol.blue == 0 )
                strcpy( color, k_mwm_hex_0_str );
            else sprintf( color, "%4X", ((mwm_col_type *)value)->xcol.blue );    
            strcat( string, color );
            /* Convert leading blanks to 0's */
            mwm_str_0( string );
            XrmPutStringResource( database, spec, string );
          }
      }
    /* Is it an integer ? */
    else if ( type == k_mwm_res_int )
      {                        
        sprintf( string, "%d", *((int *)value) );
        XrmPutStringResource( database, spec, string );
      }
}

/*******************************************************************
**
**  Description: Get a resource in the resource database.
**
**  Formal Parameters
**    Input: screen data
**           database,
**           resource,
**           resource type,
**           value,
**           default_value.
**  
********************************************************************/

int mwm_res_get( pSD, database, resource, type, value, default_value )

WmScreenData *pSD;
XrmDatabase database;
char *resource;
int type;
void *value;
void *default_value;

/* local variables */
{
char spec[ 128 ];
char string[ 128 ]; 
char color[ 15 ];
XrmValue xrm_value;
char *rep;                     
int found;
Status status;
XColor temp_color, temp2_color;
mwm_col_type *value_col;
char *temp_string;
Boolean named_col;
Colormap colormap;
Display *display;
             
/********************************/

    if ( database == NULL )
        return( false );
    strcpy( spec, k_mwm_full_prefix_str );
    strcat( spec, resource );
    XrmGetResource( database, spec, NULL, &rep, &xrm_value );

    string[ xrm_value.size ] = '\0';
    memcpy( string, xrm_value.addr, xrm_value.size );
    found = strlen( string ) > 0;
    if ( found )          
        /* Remove leading and trailing ws */
        mwm_str_ws( string );
        
    switch ( type )
      {
        /* It is a boolean. */
        case k_mwm_res_boolean:
        case k_mwm_subres_boolean:
          /* Was a resource found ? */
          if ( !found )
            {
              if ( type == k_mwm_res_boolean )
                  *(Boolean *)value = (Boolean)default_value;
            }
          else if ( mwm_str_eq( string, k_mwm_true_str ))
              *(Boolean *)value = true;
          else if ( mwm_str_eq( string, k_mwm_false_str ))
              *(Boolean *)value = false;
          else if ( mwm_str_eq( string, k_mwm_yes_str ))
              *(Boolean *)value = true;         
          else if ( mwm_str_eq( string, k_mwm_no_str ))
              *(Boolean *)value = false;
          else if ( mwm_str_eq( string, k_mwm_1_str ))
              *(Boolean *)value = true;         
          else if ( mwm_str_eq( string, k_mwm_0_str ))
              *(Boolean *)value = false;
        break;

        /* A string */
        case k_mwm_res_string:
          if ( !found )
            {
              if ( default_value != NULL )
                  strcpy( (char *)value, (char *)default_value );
            }
          else strcpy( (char *)value, string );
          break;
        
        /* A string to be allocated */
        case k_mwm_res_alloc_string:
          if ( found )
            {
              /* Allocate the string field over any existing value. */
#ifdef DEC_MOTIF_BUG_FIX
              *(long *)value = 0;
#else
              *(int *)value = 0;
#endif
              if ( ! mwm_alloc( (char **)value,
                                strlen( string ) + 1,
                                "Error allocating resource string" ))
                  return( false );
              strcpy( *((char **)value), string );
            }             
          break;
        
        /* A color name or pixel.  Pixel type is for getting subresources
           and the XColor value only is saved.
           Color type is for customization and the color name and XColor
           values are saved. */
        case k_mwm_res_pixel:
        case k_mwm_res_col:                         
          if ( !found )
            {  
              /* Is it color ? */       
              if ( type == k_mwm_res_col )
                {
                  if ( default_value != NULL )
                      *(int *)value = 0;
                }
              else if ( default_value != NULL )
                  *(int *)value = *(Pixel *)default_value;
            }
          else 
            {
              /* Get the colormap. */
              colormap = DefaultColormap( DISPLAY, pSD->screen );
              /* Is it a hex value ? */
              if (( string[ 0 ] == '#' ) && ( strlen( string ) >= 13 ))
                {                                                  
                  temp_string = string;
                  /* Go past "#" */
                  temp_string++;                                    
                  color[ 4 ] = '\0';

                  /* Copy the address for readability */
                  /* Is it color ? */
                  value_col = (mwm_col_type *)value;

                  /* Convert the red value */
                  memcpy( color, temp_string, 4 );
                  value_col->xcol.red = strtol( color, (char **)0, 16 );

                  /* Convert the green value */
                  temp_string = temp_string + 4;          
                  memcpy( color, temp_string, 4 );
                  value_col->xcol.green = strtol( color, (char **)0, 16 );

                  /* Convert the blue value */
                  temp_string = temp_string + 4;
                  memcpy( color, temp_string, 4 );
                  value_col->xcol.blue = strtol( color, (char **)0, 16 );

                  /* Get the pixel value for these colors */
                  memcpy( &temp_color, (XColor *)value, sizeof( XColor ) );
                  /* Free the named color since the name does
                     not match the new color pixel value */
                  if ( value_col->name )  
                    {
                      XtFree( value_col->name );
                      value_col->name = NULL;
                    }
                }
              /* It must be a named color */
              else
                {
                  /* Copy the address for readability */
                  value_col = (mwm_col_type *)value;
                  /* Is it color ? */
                  if ( type == k_mwm_res_col )
                    {
                      /* Is there a destination ? */
                      if ( value_col->name == NULL ) 
                        {
                          /* No, allocate the color name field */        
                          if ( ! mwm_alloc( &value_col->name,
                                            k_mwm_max_color_name_size,          
                                            "Error allocating color name data" ))
                            return( false );
                        }                                         
                      strcpy( value_col->name, string );
                    }                               
                  if ( ! XLookupColor( DISPLAY, colormap,
                                       string, &temp_color, &temp2_color ))
                      /* Could not lookup color */
                      return( false );
                  /* Copy the entire color */
                  if ( type == k_mwm_res_col )
                      memcpy( &value_col->xcol, &temp_color, sizeof( XColor ));
                }
              if ( ! XAllocColor( DISPLAY, colormap, &temp_color ))
                  /* Could not allocate color */
                  return( false );

              /* Update the pixel value */
              value_col->xcol.pixel = temp_color.pixel;
            }
          break;                    
                                                       
        /* An integer */
        case k_mwm_res_int:
          if ( !found )
              *(int *)value = (int)default_value;
          else *(int *)value = strtol( string, (char **)0, 10 );
          break;
         
        default: 
          break;
      }
    return( found );

}

/*******************************************************************
**
**  Description: Search for a substring.  Use a case insensitive compare.
**
**  Formal Parameters
**    Input: string,
**           value,
**    Return: true if found.
**  
********************************************************************/

Boolean mwm_str_find( string, value )

char *string;
char *value;

/* local variables */
{
char *result;
char *temp;
char *temp_not;

/********************************/
               
    result = mwm_str_get( string, value );

    /* Was anything found ? */
    if ( result == NULL )
      {
        /* Nope, return true if they all have "-" and
           hence this value is implicitly selected. */
        /* Are there any "-" ? */
        if ( mwm_str_get( string, k_mwm_not_str ) == NULL )
            /* Nope, done */
            return( false );
        /* Yes, are all the other ones "-"s.  Check to make sure
           they all have -'s */
        else
          {
            temp = string;         
            /* Any more characters in the string ? */
            while (( temp != NULL ) && ( *temp != 0 ))
              /* Yes, search for ws. */
              {
                /* Is there "-" before the text ? */                  
                if ( strncmp( temp, k_mwm_not_str, 1 ))
                   /* No, set to false */
                   return( false );
                /* Go to the next string */
                else 
                  {
                    /* Find a space or the end of the string */
                    while ( ( temp != NULL ) && ( *temp != 0 ) && !isspace( *temp ))
                        temp++;                          
                    /* Find a character or the end of the string */
                    while ( ( temp != NULL ) && ( *temp != 0 ) && isspace( *temp )) 
                        temp++;
                    /* No more characters. */
                    if (( temp == NULL ) || ( *temp == 0 ))
                        /* All options have "-", value is implicitly true */
                        return( true );
                    /* More text was found, continue search */
                  }
              }
            return( false );
          }
      }    

    /* The value was found, is it nested ?
       Are there any trailing characters ? */
    if ( ( result + strlen( value )) < ( string + strlen( string )))
      /* Yes, is it a space ? */
      {      
        temp = result + strlen( value );
        if ( !isspace( *temp ))
            /* Not a space, try again. */
            return( mwm_str_find( temp, value ));
      }

    /* Check for -value and check for nested strings.
       Did the result start at the beginning of the string ? */
    if ( result == string )
        /* Yes, the result was found. */
        return( true );
    /* No, check for previous nots. */
    else
      {
        result--;
        /* Was a "not" found before the result ? */
        if ( !strncmp( result, k_mwm_not_str, 1 ))
            /* Yes, value set to false */
            return( false );
        /* No, Is there a space before the value ? */
        else if ( !isspace( *result ))
          /* No, check for future cases. */
          {
            result = result + 2;
            return( mwm_str_find( result, value ));
          }
        /* There is a space, are there any nots before this point ? */
        else 
          {
            temp_not = (char *)strstr( string, k_mwm_not_str );
            /* Any nots ? */
            if ( temp_not == NULL )
                /* No, done */
                return( true );
            /* Any nots before the value ? */
            else return( temp_not > result );
          }
      }

}

/*******************************************************************
**
**  Description: Set the cursor to wait or not to wait.
**  
**  Formal Parameters
**    Input: flag.
**
*******************************************************************/

int mwm_watch_set( flag )

int flag;

{
/* local variables */

/********************************/

    /* Change the cursor */
    ShowWaitState( flag );   

}

/*******************************************************************
**
**  Description: Get the value in a text field.
**               Free the storage.
**               
**  Formal Parameters
**    Input:  wid,
**            dialog id,
**            field id,
**            value,
**    Output: string.
**    Return:True if no error.
**
********************************************************************/

int mwm_num_get( wid, did, fid, num, string )

Widget wid;
int did;
int fid;
int *num;
char *string;
{
/* local variables */
int error;
char *result;

/********************************/

    /* Get the string */
    result = (char *)XmTextGetString( (*mwm_fid)[ WID_SCREEN ][ fid ] );
    /* Convert it */
    error = strspn( result, "0123456789" ) != strlen( result );
    if ( !error )                             
      {
        *num = atoi( result );              
        /* Save the result */
        strcpy( string, result );
      }
    /* There was an error.  Bring up the dialog box. */
    else mwm_dialog_error_up( wid, did, fid, result );

    /* Free it */
    XtFree( result );
    return ( !error );

}
 
/*******************************************************************
**
**  Description: Set one resource value.
**
**  Formal Parameters
**    Input: widget id,               
**           resource,
**           reason.
**  
********************************************************************/

void mwm_set( wid, resource, value )       

Widget wid;                      
char *resource;
void *value;

{
/* local variables */
Arg arg_list[ 1 ];

/********************************/

    if ( wid != NULL )
      {
        XtSetArg( arg_list[ 0 ], resource, (XtArgVal)value );
        XtSetValues( wid, arg_list, 1 );
      }

}

/*******************************************************************
**
**  Description: Get one resource value.
**
**  Formal Parameters
**    Input: widget id,               
**           resource,
**           address of value.
**  
********************************************************************/

void mwm_get( wid, resource, value )       

Widget wid;                      
char *resource;
void *value;

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
**  Description: Allocate a structure. Report an error if it fails.
**
**  Formal Parameters
**    Input: ptr to structure,
**           size,
**           error message.
**
********************************************************************/

int mwm_alloc( block, size, message )
                  
void *block;
int size;
char *message;
{
char **the_block = (char **)block;

/********************************/

    /* Is there even something there? */
    if ( the_block == (char **)NULL )
      {
        ShowWaitState( false );
        Warning( message );
        return( false );
      }
    /* Is it already allocated ? */
    else if ( (char *)*the_block == NULL )
      /* No, do it. */
      {
        if (!( (*the_block) = (char *)XtCalloc( size, sizeof(char) )))
          {
            ShowWaitState( false );
            Warning( message );
            return( false );
          }
      }
    return( true );         

}

/*******************************************************************/
