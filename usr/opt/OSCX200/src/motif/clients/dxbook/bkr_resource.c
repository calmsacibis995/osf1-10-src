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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_RESOURCE.C*/
/* *9    30-OCT-1992 18:50:08 BALLENGER "Add CC specific resources, and resources for DECW*BOOK*"*/
/* *8     5-AUG-1992 21:35:12 BALLENGER "Remove the !@%^&$ BOOKREADER_CC conditionals"*/
/* *7     3-AUG-1992 18:43:09 BALLENGER "Add support for character cell specific resources."*/
/* *6    28-JUL-1992 13:39:15 KARDON "Update to one image"*/
/* *5    28-JUL-1992 13:28:04 BALLENGER "Character cell work."*/
/* *4    20-JUL-1992 13:49:23 BALLENGER "Character cell support"*/
/* *3    19-JUN-1992 20:19:47 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *2     3-MAR-1992 17:02:56 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:40:11 PARMENTER "Fetch and Customize resource names"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_RESOURCE.C*/
#ifndef VMS
 /*
#else
# module BKR_RESOURCE "V03-0001"
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
**  ABSTRACT:
**
**	Bookreader customizable resource names and fetching routines.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     16-Apr-1990
**
**  MODIFICATION HISTORY:
**  V03-0001	JAF0001	James A. Ferguson   	22-Jan-1990
**	    	Add check to "bkr_resource_fetch_*" routines so routines
**  	    	can only be called once.  This is part of the fix for QAR 684.
**
**--
**/


/*
 * INCLUDE FILES
 */


#include "br_common_defs.h"  /* common BR #defines */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
/* #include "br_resource.h"     * typedefs and #defines for Xrm customizable
                                resources used in BR */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_resource.h"    /* function prototypes for .c module */
#include  <X11/StringDefs.h>
#include  <ctype.h>


/*
 * LOCAL ROUTINES
 */
static void  	    cvt_string_to_dir_name();
static void  	    cvt_string_to_shelf_open();



/*
 * FORWARD DEFINITIONS
 */

/*
 *  Resources for the Bookreader Library window
 */

static XtResource library_resource_list[] = 
{
    {	bkrNconfirm_close,
    	bkrCConfirm_Close,
    	XmRBoolean,
    	sizeof( Boolean ),
    	XtOffsetOf( BKR_LIBRARY_RESOURCES, confirm_close ), 
    	XmRString,
    	XtEtrue },

    {	bkrNshelf_to_open_on_startup,
    	bkrCShelf_To_Open_On_Startup,
    	bkrRShelf_To_Open_On_Startup,
    	sizeof( unsigned char ),
    	XtOffsetOf( BKR_LIBRARY_RESOURCES, shelf_to_open ), 
    	XmRImmediate, 
    	(caddr_t) FIRST_SHELF },

    {	bkrNmm_width,
    	bkrCMM_Width,
    	XmRDimension,
    	sizeof( Dimension ),
    	XtOffsetOf( BKR_LIBRARY_RESOURCES, mm_width ),
    	XmRImmediate,
    	(caddr_t) 98 },    	/* in millimeters */

    {	bkrNmm_height,
    	bkrCMM_Height,
    	XmRDimension,
    	sizeof( Dimension ),
    	XtOffsetOf( BKR_LIBRARY_RESOURCES, mm_height ),
    	XmRImmediate,
    	(caddr_t) 100 },     	/* in millimeters */

    {	bkrNsearchList,
        bkrCSearchList,
    	XmRString,
    	sizeof( String ),
    	XtOffsetOf( BKR_LIBRARY_RESOURCES, searchList ),
    	XmRString,
    	(caddr_t) NULL },

    {	bkrNdefaultLibraryName,
        bkrCDefaultLibraryName,
    	XmRString,
    	sizeof( String ),
    	XtOffsetOf( BKR_LIBRARY_RESOURCES, defaultLibraryName ),
    	XmRString,
    	(caddr_t) NULL },

    {	XmNinitialState,
    	XmCInitialState,
    	XmRInt,
    	sizeof( int ),
    	XtOffsetOf( BKR_LIBRARY_RESOURCES, initial_state ),
    	XmRImmediate,
    	(caddr_t) NormalState }
};


static XtResource cc_library_resource_list[] = 
{
    {	bkrNconfirm_close,
    	bkrCConfirm_Close,
    	XmRBoolean,
    	sizeof( Boolean ),
    	XtOffsetOf( BKR_LIBRARY_RESOURCES, confirm_close ), 
    	XmRString,
    	XtEtrue },

    {	bkrNshelf_to_open_on_startup,
    	bkrCShelf_To_Open_On_Startup,
    	bkrRShelf_To_Open_On_Startup,
    	sizeof( unsigned char ),
    	XtOffsetOf( BKR_LIBRARY_RESOURCES, shelf_to_open ), 
    	XmRImmediate, 
    	(caddr_t) FIRST_SHELF },

    {	bkrNsearchList,
        bkrCSearchList,
    	XmRString,
    	sizeof( String ),
    	XtOffsetOf( BKR_LIBRARY_RESOURCES, searchList ),
    	XmRString,
    	(caddr_t) NULL },

    {	bkrNdefaultLibraryName,
        bkrCDefaultLibraryName,
    	XmRString,
    	sizeof( String ),
    	XtOffsetOf( BKR_LIBRARY_RESOURCES, defaultLibraryName ),
    	XmRString,
    	(caddr_t) NULL },

    {	bkrNcinitialState,
        bkrCCInitialState,
    	XmRInt,
    	sizeof( int ),
    	XtOffsetOf( BKR_LIBRARY_RESOURCES, initial_state ),
    	XmRImmediate,
    	(caddr_t) NormalState }
};


/*
 *  Resources for the Bookreader Selection window
 */

static XtResource selection_resource_list[] = 
{
    {	bkrNfirst_directory_to_open,
    	bkrCFirst_Directory_To_Open,
    	bkrRFirst_Directory_To_Open,
    	sizeof( unsigned char ),
    	XtOffsetOf( BKR_RESOURCES, first_directory_to_open ),
    	XmRImmediate, 
    	(caddr_t) TOC_DIR },

    {	bkrNmm_width,
    	bkrCMM_Width,
    	XmRDimension,
    	sizeof( Dimension ),
    	XtOffsetOf( BKR_RESOURCES, mm_width ),
    	XmRImmediate,
    	(caddr_t) 101 },    	/* in millimeters */

    {	bkrNmm_height,
    	bkrCMM_Height,
    	XmRDimension,
    	sizeof( Dimension ),
    	XtOffsetOf( BKR_RESOURCES, mm_height ),
    	XmRImmediate,
    	(caddr_t) 145 },     	/* in millimeters */

    {	bkrNx_offset,
    	bkrCX_Offset,
    	XmRPosition,
    	sizeof( Position ),
    	XtOffsetOf( BKR_RESOURCES, x_offset ),
    	XmRImmediate,
    	(caddr_t) 0 },	    	/* in pixels */

    {	bkrNy_offset,
    	bkrCY_Offset,
    	XmRPosition,
    	sizeof( Position ),
    	XtOffsetOf( BKR_RESOURCES, y_offset ),
    	XmRImmediate,
    	(caddr_t) 0 }	    	/* in pixels */
};


static XtResource cc_selection_resource_list[] = 
{
    {	bkrNfirst_directory_to_open,
    	bkrCFirst_Directory_To_Open,
    	bkrRFirst_Directory_To_Open,
    	sizeof( unsigned char ),
    	XtOffsetOf( BKR_RESOURCES, first_directory_to_open ),
    	XmRImmediate, 
    	(caddr_t) TOC_DIR },

    {	bkrNcx_offset,
    	bkrCCX_Offset,
    	XmRPosition,
    	sizeof( Position ),
    	XtOffsetOf( BKR_RESOURCES, x_offset ),
    	XmRImmediate,
    	(caddr_t) 0 },	    	/* in pixels */

    {	bkrNcy_offset,
    	bkrCCY_Offset,
    	XmRPosition,
    	sizeof( Position ),
    	XtOffsetOf( BKR_RESOURCES, y_offset ),
    	XmRImmediate,
    	(caddr_t) 0 }	    	/* in pixels */
};


/*
 *  Resources for the Bookreader Topic windows (both FORMAL and STANDARD)
 */

static XtResource topic_resource_list[] = 
{
    { 	bkrNmax_default_topic_width,
    	bkrCMax_Default_Topic_Width,
    	XmRDimension,
    	sizeof( Dimension ),
    	XtOffsetOf( BKR_TOPIC_RESOURCES, max_default_topic_width ),
    	XmRImmediate, 
    	(caddr_t) 765 },   	/* in pixels */

    { 	bkrNmax_default_topic_height,
    	bkrCMax_Default_Topic_Height,
    	XmRDimension,
    	sizeof( Dimension ),
    	XtOffsetOf( BKR_TOPIC_RESOURCES, max_default_topic_height ), 
    	XmRImmediate, 
    	(caddr_t) 645 },    	/* in pixels */

    {	bkrNshow_hot_spots,
    	bkrCShow_Hot_Spots,
    	XmRBoolean,
    	sizeof( Boolean ),
    	XtOffsetOf( BKR_TOPIC_RESOURCES, show_hot_spots ), 
    	XmRString,
    	XtEfalse },

    {	bkrNshow_extensions,
    	bkrCShow_Extensions,
    	XmRBoolean,
    	sizeof( Boolean ),
    	XtOffsetOf( BKR_TOPIC_RESOURCES, show_extensions ), 
    	XmRString,
    	XtEtrue },

    {	bkrNmm_width,
    	bkrCMM_Width,
    	XmRDimension,
    	sizeof( Dimension ),
    	XtOffsetOf( BKR_TOPIC_RESOURCES, mm_width ),
    	XmRImmediate,
    	(caddr_t) 159 },    	/* in millimeters */

    {	bkrNmm_height,
    	bkrCMM_Height,
    	XmRDimension,
    	sizeof( Dimension ),
    	XtOffsetOf( BKR_TOPIC_RESOURCES, mm_height ),
    	XmRImmediate,
    	(caddr_t) 178 },     	/* in millimeters */

    {	bkrNx_offset,
    	bkrCX_Offset,
    	XmRPosition,
    	sizeof( Position ),
    	XtOffsetOf( BKR_TOPIC_RESOURCES, x_offset ),
    	XmRImmediate,
    	(caddr_t) 0 },	    	/* in pixels */

    {	bkrNy_offset,
    	bkrCY_Offset,
    	XmRPosition,
    	sizeof( Position ),
    	XtOffsetOf( BKR_TOPIC_RESOURCES, y_offset ),
    	XmRImmediate,
    	(caddr_t) 0 }	    	/* in pixels */
};

/* Character cell topic resource list
 */
static XtResource cc_topic_resource_list[] = 
{
    {	bkrNcshow_hot_spots,
    	bkrCCShow_Hot_Spots,
    	XmRBoolean,
    	sizeof( Boolean ),
    	XtOffsetOf( BKR_TOPIC_RESOURCES, show_hot_spots ), 
    	XmRString,
    	XtEtrue },

    {	bkrNcx_offset,
    	bkrCCX_Offset,
    	XmRPosition,
    	sizeof( Position ),
    	XtOffsetOf( BKR_TOPIC_RESOURCES, x_offset ),
    	XmRImmediate,
    	(caddr_t) 0 },	    	/* in pixels */

    {	bkrNcy_offset,
    	bkrCCY_Offset,
    	XmRPosition,
    	sizeof( Position ),
    	XtOffsetOf( BKR_TOPIC_RESOURCES, y_offset ),
    	XmRImmediate,
    	(caddr_t) 0 }	    	/* in pixels */
};


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_resource_fetch_library
**
** 	Initializes the resources supported by the Library window.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_resource_fetch_library(VOID_PARAM)
{
    Widget  	    dummy_library_shell = NULL;
    static Boolean library_resources_fetched = FALSE;

    if ( library_resources_fetched )
    	return;

    XtAppAddConverter( XtDisplayToApplicationContext( bkr_display ),
    	    	       XmRString, bkrRShelf_To_Open_On_Startup,
    	    	       cvt_string_to_shelf_open, NULL, 0 );

    /* Create a dummy application shell so we can fetch the resources */

    dummy_library_shell = XtAppCreateShell(
    	    	    	    BKR_LIBRARY_WINDOW_NAME,	    /* application name  */
    	    	    	    BKR_APPLICATION_CLASS, 	    /* application class */
    	    	    	    applicationShellWidgetClass,    /* widget class */
    	    	    	    bkr_display,
    	    	    	    NULL, 0 );

    /* Retrieve the application resources */

    if (bkrplus_g_charcell_display) 
    {
        XtGetApplicationResources( dummy_library_shell, 
                                  &bkr_library_resources,
                                  cc_library_resource_list, 
                                  XtNumber(cc_library_resource_list), 
                                  NULL, 0 );

    }
    else 
    {
        Dimension	    width;
        Dimension	    height;

        XtGetApplicationResources(dummy_library_shell, 
                                  &bkr_library_resources,
                                  library_resource_list, 
                                  XtNumber( library_resource_list ), 
                                  NULL, 0 );
        /* 
         *  Convert the width and height values from millimeters to pixels and 
         *  check the range as not to exceed the width and height of the current display.
         */
        width = CONVERT_MM_TO_PIXELS( bkr_library_resources.mm_width );
        height = CONVERT_MM_TO_PIXELS( bkr_library_resources.mm_height);
        bkr_library_resources.width = MIN( width, bkr_display_width );
        bkr_library_resources.height = MIN( height, bkr_display_height );
    }

    XtDestroyWidget( dummy_library_shell );

    library_resources_fetched = TRUE;

};  /* end of bkr_resource_fetch_library */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_resource_fetch_selection
**
** 	Initializes the resources supported by the Selection window.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_resource_fetch_selection(VOID_PARAM)
{
    Widget  	    dummy_shell = NULL;

    static Boolean selection_resources_fetched = FALSE;

    if ( selection_resources_fetched )
    	return;

    XtAppAddConverter( XtDisplayToApplicationContext( bkr_display ),
    	    	       XmRString, bkrRFirst_Directory_To_Open,
    	    	       cvt_string_to_dir_name, NULL, 0 );

    /* Create a dummy application shell so we can fetch the resources */

    dummy_shell = XtAppCreateShell(
    	    	    	    BKR_SELECTION_WINDOW_NAME,	    /* application name  */
    	    	    	    BKR_APPLICATION_CLASS, 	    /* application class */
    	    	    	    applicationShellWidgetClass,    /* widget class */
    	    	    	    bkr_display,
    	    	    	    NULL, 0 );

    /* Retrieve the application resources */

    if (bkrplus_g_charcell_display) 
    {
        XtGetApplicationResources(dummy_shell,
                                  &bkr_resources,
                                  cc_selection_resource_list, 
                                  XtNumber(cc_selection_resource_list), 
                                  NULL, 0 );
    }
    else 
    {
        Dimension	    width;
        Dimension	    height;

        XtGetApplicationResources(dummy_shell,
                                  &bkr_resources,
                                  selection_resource_list, 
                                  XtNumber(selection_resource_list), 
                                  NULL, 0 );
        /* 
         *  Convert the width and height values from millimeters to pixels and 
         *  check the range as not to exceed the width and height of the current display.
         */
        width = CONVERT_MM_TO_PIXELS( bkr_resources.mm_width );
        height = CONVERT_MM_TO_PIXELS( bkr_resources.mm_height );

        bkr_resources.width = MIN( width, bkr_display_width );
        bkr_resources.height = MIN( height, bkr_display_height );
    }

    XtDestroyWidget( dummy_shell );

    selection_resources_fetched = TRUE;

};  /* end of bkr_resource_fetch_selection */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_resource_fetch_topic
**
** 	Initializes the resources supported by the Topic window.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_resource_fetch_topic(VOID_PARAM)
{
    Widget  	    dummy_topic_shell = NULL;
    static Boolean topic_resources_fetched = FALSE;

    if ( topic_resources_fetched )
    	return;

    /* Create a dummy application shell so we can fetch the resources */

    dummy_topic_shell = XtAppCreateShell(
    	    	    	    BKR_TOPIC_WINDOW_NAME,	    /* application name  */
    	    	    	    BKR_APPLICATION_CLASS, 	    /* application class */
    	    	    	    applicationShellWidgetClass,    /* widget class */
    	    	    	    bkr_display,
    	    	    	    NULL, 0 );

    /* Retrieve the application resources */

    if (bkrplus_g_charcell_display) 
    {
        XtGetApplicationResources(dummy_topic_shell,
                                  &bkr_topic_resources,
                                  cc_topic_resource_list,
                                  XtNumber(cc_topic_resource_list), 
                                  NULL, 0 );

        bkr_topic_resources.show_extensions = FALSE;       
    }
    else 
    {
        Dimension	    width;
        Dimension	    height;

        XtGetApplicationResources(dummy_topic_shell,
                                  &bkr_topic_resources,
                                  topic_resource_list,
                                  XtNumber(topic_resource_list), 
                                  NULL, 0 );

        /*  Convert the width and height values from millimeters to pixels and 
         *  check the range as not to exceed the width and height of the current display.
         */
        width = CONVERT_MM_TO_PIXELS( bkr_topic_resources.mm_width );
        height = CONVERT_MM_TO_PIXELS( bkr_topic_resources.mm_height );

        bkr_topic_resources.width = MIN( width, bkr_display_width );
        bkr_topic_resources.height = MIN( height, bkr_display_height );

        /* Check the maximum default size for FORMAL topic shell's 
         */
        if ( bkr_topic_resources.max_default_topic_width > bkr_display_width ) {
            bkr_topic_resources.max_default_topic_width =  bkr_display_width;
        }
        if ( bkr_topic_resources.max_default_topic_height > bkr_display_height ) {
            bkr_topic_resources.max_default_topic_height = bkr_display_height;
        }
    }


    XtDestroyWidget( dummy_topic_shell );

    topic_resources_fetched = TRUE;

};  /* end of bkr_resource_fetch_topic */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	cvt_string_to_dir_name
**
** 	Resource converter which converts a string representation of a
**  	directory name to a directory value.
**
**  FORMAL PARAMETERS:
**
**  	args	 - pointer to the Xrm value 	    (unused)
**  	num_args - pointer to number of Xrm values  (unused)
**  	from_val - pointer to the value to convert.
**  	to_val   - address to return the converted value.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static void
cvt_string_to_dir_name( args, num_args, from_val, to_val )
    XrmValuePtr     args;
    Cardinal 	    *num_args;
    XrmValue 	    *from_val;
    XrmValue 	    *to_val;
{
    char    	    	 *src_str = (char *) (from_val->addr);
    char     	    	 *input_str;
    int	    	    	 cnt;
    int	    	    	 str_len;
    static unsigned char i;

    to_val->size = sizeof ( unsigned char );
    to_val->addr = (caddr_t) &i;

    /* Lower case the input string for comparisons */

    str_len = strlen( from_val->addr );
    input_str = (char *) BKR_MALLOC( str_len + 1 );
    for ( cnt = 0; cnt < str_len; cnt++ )
    {
    	if ( isupper( src_str[cnt] ) )
    	    input_str[cnt] = _tolower( src_str[cnt] );
    	else
    	    input_str[cnt] = src_str[cnt];
    }
    input_str[str_len] = NULLCHAR;

    /* Try to match the input against the valid values for the resource */

    if ( strcmp( input_str, "toc" ) == 0 ) 
    	i = TOC_DIR;
    else if ( strcmp( input_str, "index" ) == 0 )
    	i = INDEX_DIR;
    else if ( strcmp( input_str, "default" ) == 0 )
    	i = DEFAULT_DIR;
    else if ( strcmp( input_str, "none" ) == 0 )
    	i = NO_DIRECTORY;
    else
    {
    	to_val->size = 0;
    	to_val->addr = NULL;
    	XtStringConversionWarning( (char *) from_val->addr, 
    	    	    	    	   bkrRFirst_Directory_To_Open );
    }

    BKR_FREE( input_str );

};  /* end of cvt_string_to_dir_name */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	cvt_string_to_shelf_open
**
** 	Resource converter which converts a string representation of a
**  	shelf open type to a shelf open value.
**
**  FORMAL PARAMETERS:
**
**  	args	 - pointer to the Xrm value 	    (unused)
**  	num_args - pointer to number of Xrm values  (unused)
**  	from_val - pointer to the value to convert.
**  	to_val   - address to return the converted value.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static void
cvt_string_to_shelf_open( args, num_args, from_val, to_val )
    XrmValuePtr     args;
    Cardinal 	    *num_args;
    XrmValue 	    *from_val;
    XrmValue 	    *to_val;
{
    char    	    	 *src_str = (char *) (from_val->addr);
    char     	    	 *input_str;
    int	    	    	 cnt;
    int	    	    	 str_len;
    static unsigned char i;

    to_val->size = sizeof ( unsigned char );
    to_val->addr = (caddr_t) &i;

    /* Lower case the input string for comparisons */

    str_len = strlen( from_val->addr );
    input_str = (char *) BKR_MALLOC( str_len + 1 );
    for ( cnt = 0; cnt < str_len; cnt++ )
    {
    	if ( isupper( src_str[cnt] ) )
    	    input_str[cnt] = _tolower( src_str[cnt] );
    	else
    	    input_str[cnt] = src_str[cnt];
    }
    input_str[str_len] = NULLCHAR;

    /* Try to match the input against the valid values for the resource */

    if ( strcmp( input_str, "none" ) == 0 ) 
    	i = NO_SHELF;
    else if ( strcmp( input_str, "first" ) == 0 )
    	i = FIRST_SHELF;
    else if ( strcmp( input_str, "all" ) == 0 )
    	i = ALL_SHELVES;
    else
    {
    	to_val->size = 0;
    	to_val->addr = NULL;
    	XtStringConversionWarning( (char *) from_val->addr, 
    	    	    	    	   bkrRShelf_To_Open_On_Startup );
    }

    BKR_FREE( input_str );

};  /* end of cvt_string_to_shelf_open */
