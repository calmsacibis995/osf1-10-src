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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_SEARCH.C*/
/* *16   26-MAR-1993 15:41:10 BALLENGER "Fix compilation problems for VAX ULTRIX."*/
/* *15   25-SEP-1992 10:52:58 KLUM "to link without CBR"*/
/* *14    9-JUN-1992 11:26:28 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *13   19-MAY-1992 13:55:34 FITZELL "get window returned from  open_to_chunk"*/
/* *12   16-APR-1992 17:21:55 FITZELL "memory leaks"*/
/* *11   31-MAR-1992 17:08:11 FITZELL "2nd try at putting in license check"*/
/* *10   31-MAR-1992 16:44:58 FITZELL "added license check"*/
/* *9    23-MAR-1992 13:43:37 PARMENTER "added some more comments"*/
/* *8    19-MAR-1992 11:41:53 PARMENTER "added search and results contexts"*/
/* *7     5-MAR-1992 14:25:35 PARMENTER "adding simple search"*/
/* *6     3-MAR-1992 17:03:22 KARDON "UCXed"*/
/* *5    23-JAN-1992 16:38:52 PARMENTER "fixing search cb's"*/
/* *4     7-JAN-1992 16:50:07 PARMENTER "adding CBR/Search"*/
/* *3     1-NOV-1991 12:58:24 BALLENGER "Reintegrate  memex support"*/
/* *2    18-SEP-1991 19:59:47 BALLENGER "inlcude function prototype headers"*/
/* *1    16-SEP-1991 12:40:19 PARMENTER "Search"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_SEARCH.C*/
#ifndef VMS
 /*
#else
#module BKR_SEARCH "V03-0000"
#endif
#ifndef VMS
  */
#endif

/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1992  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use, 	     **
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
**      Bookreader User Interface ( bkr )
**
**  ABSTRACT:
**
**	Search code:  menu callbacks, topic display, CBR routines
**
**  AUTHORS:
**
**      David Parmenter ( callbacks )
**	David L Ballenger ( displays )
**
**  CREATION DATE:     30-May-1991
**
**  MODIFICATION HISTORY:
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
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_search.h"      /* function prototypes for .c module */
#include "bkr_book.h"        /* Book access routines */
#include "bkr_cursor.h"      /* Curosr routines */
#include "bkr_directory.h"   /* Directory routines */
#include "bkr_error.h"       /* Error routines */
#include "bkr_fetch.h"       /* Resource literal routines */
#include "bkr_shell.h"       /* Shell routines */
#include "bkr_topic_open.h"  /* Topic open routines */
#include "bkr_window.h"      /* Window access routines */
#include <Xm/Text.h>
#include <stdio.h>


/*
 * Global References
 */


#ifdef CBR
extern int			StartupCBR();		/* CBR entry */
extern int 			DoQuery();		/* simple query */
extern int			DoConcepts();		/* concept list */
extern int			DoEdit();		/* edit concept */
#endif

static void			start_cbr();		/* local startup cbr */



/*
 * STATIC VARIABLES 
 */

static MrmRegisterArg     	tag_reglist[] = { { "tag", (caddr_t) 0 } };
static char    			*error_string;	

    /* these two global flags record whether we have initialize a search
     */

static int			simple_search_started = FALSE;

#ifdef CBR
static int			cbr_started = FALSE;
#endif



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_no_op_cb()
**
** 	no op callback.  Not needed in final product, but convenient during
**	development.
**
**  FORMAL PARAMETERS:
**
**	widget - not used
**	tag - not used
**	data - not used
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
bkr_no_op_cb PARAM_NAMES((widget,tag,data))
    Widget			widget PARAM_SEP
    Opaque			*tag PARAM_SEP
    XmAnyCallbackStruct		*data PARAM_END
{

}   /* end of bkr_no_op_cb */




/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_search_everything_cb()
**
** 	generic search menu item callback.  This gets called for all
**	of the search callbacks.  The tag isused to distinguish the menu items
**
**	Note this routine is used by both CBR and SS.
**
**  FORMAL PARAMETERS:
**
**	widget	-- calling widget, used to find the window
**	tag	-- tag is used to denote what kind of search to do
**	data	-- not used
**
**  IMPLICIT INPUTS:
**
**	cbr_started
**	simple_search_started
**
**  IMPLICIT OUTPUTS:
**
**	locator - string used to display desired topics, freed by 
**		  bkr_cbr_get_topic.  passed to CBR.
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	passes control to CBR query widget
**
**--
**/

void
bkr_search_everything_cb PARAM_NAMES((widget,tag,data))
    Widget			widget PARAM_SEP
    Opaque			*tag PARAM_SEP
    XmAnyCallbackStruct		*data PARAM_END
{
    int			do_cbr = FALSE;
    BKR_WINDOW 		*window;
    char		*locator;	/* ptr to locator string */

    /* Check global set by LMF check */
    if(! bkrplus_g_allow_search) {
        bkr_display_need_license_box ();
        return;
    }
    /* 
     * get window, if null, assume library
     */

    if ( ( window = bkr_window_find(widget)) == NULL )
    	window = bkr_library;			

#ifdef CBR

    /* 
     * allocate CBR locator. get huge amt for now (FIX ME!!)
     * start up CBR if need be
     * do CBR Query 
     */

    locator = (char *) BKR_CALLOC( 1000, sizeof( char ) );	
						
    if (! cbr_started )				
	start_cbr( locator );

    if (do_cbr)
	DoQuery();
#endif

    /*
     *	start up search stuff if neccesary
     *  do simple search query.
     */

/*** we use our own results widget now ***
    if (! simple_search_started )
	DXmCbrInitializeForMrm();  ***/

    if (! do_cbr )
	bkr_setup_simple_search( window, tag );

}   /* end of bkr_search_everything_cb() */







#ifdef SEARCH

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_cbr_results_cb()
**
** 	this routine is called from CBR results box, when user wants
**	to view a particular result.
**
**	Causes bookreader to open the desired result in a new window.
**
**
**  FORMAL PARAMETERS:
**
**      locator - character pointer.  String consists of two parts  separated
**         	  by a '.'.  First part is the filename of the book, the 
**         	  second part is the ascii representation of the first chunk 
**         	  from the desired topic.  [[ this is a hack.  should use a 
**         	  real struct]]
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
bkr_cbr_results_cb PARAM_NAMES((locator))
    char		*locator PARAM_END	/* filename and chunk number */
{
    int			status;		/* status */
    char 		*c;		/* ch ptr, used in parsing locator */
    char		filename[255];	/* receives true filename for book */
    BMD_OBJECT_ID	chunk_id;	/* desired chunk */


  printf("\n bkr_cbr_results_cb\n");

    if ( locator )
    	{
    	c = strchr( locator, '.' ); 		/* look for '.' */
    	*c = '\0';				/* replaced '.' with '\0' */
    	chunk_id = 				/* get chunk_id */
           	( BMD_OBJECT_ID ) atoi( c + 1 );	
	c = locator;				/* start at beginning */

	sprintf( filename, "CBRDATA:%s.decw$book", c );

	/* we've got what we want, try to display topic */

	(void) bkr_book_open_to_chunk ( filename, chunk_id, FALSE );

	BKR_CFREE( locator );
    	}
    else
	{
	    error_string = ( char * ) bkr_fetch_literal( "CBR_ERROR", ASCIZ );
    	    if ( error_string != NULL )
    	    {
	    	sprintf( errmsg, error_string );
	    	bkr_error_modal( errmsg, NULL );
	    	XtFree( error_string );
    	    }
    	return;
	}

}   /* end of bkr_cbr_results_cb */



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_cbr_get_topic()
**
** 	this routine is called from CBR results box, when user wants
**	to view a particular topic.
**
**	Causes bookreader to open the desired topic in a new window.
**
**
**  FORMAL PARAMETERS:
**
**      locator - character pointer.  String consists of two parts  separated
**         	  by a '.'.  First part is the filename of the book, the 
**         	  second part is the ascii representation of the first chunk 
**         	  from the desired topic.  [[ this is a hack.  should use a 
**         	  real struct]]
**
**  IMPLICIT INPUTS:
**
**	errmsg[] - for error reporting
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
**	if succesful, causes topic window to display appropriate topic
**
**--
**/

void
bkr_cbr_get_topic PARAM_NAMES((locator))
    char		*locator PARAM_END	/* filename and chunk number */
{
    int			status;		/* status */
    char 		*c;		/* ch ptr, used in parsing locator */
    char		filename[255];	/* receives true filename for book */
    BMD_OBJECT_ID	chunk_id;	/* desired chunk */

    if ( locator )
    	{
    	c = strchr( locator, '.' ); 		/* look for '.' */
    	*c = '\0';				/* replaced '.' with '\0' */
    	chunk_id = 				/* get chunk_id */
           	( BMD_OBJECT_ID ) atoi( c + 1 );	
	c = locator;				/* start at beginning */

	sprintf( filename, "CBRDATA:%s.decw$book", c );

	/* we've got what we want, try to display topic */

	(void) bkr_book_open_to_chunk ( filename, chunk_id, FALSE );

	BKR_CFREE( locator );
    	}
    else
	{
	    error_string = ( char * ) bkr_fetch_literal( "CBR_ERROR", ASCIZ );
    	    if ( error_string != NULL )
    	    {
	    	sprintf( errmsg, error_string );
	    	bkr_error_modal( errmsg, NULL );
	    	XtFree( error_string );
    	    }
    	return;
	}

}   /* end of bkr_cbr_get_topic */




/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_initialize_search_context()
**
**	initializes bkr_search_context for a bookreader window
**
**  FORMAL PARAMETERS:
**
**	window - pointer to the BKR_WINDOW in which the search is being
**               conducted.
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
bkr_initialize_search_context PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{

    /*
     * set the counters to 0
     */

    window->search.query_type 	= 0;
    window->search.n_results 	= 0;
    window->search.n_filenames 	= 0;

    /*
     * start with a predefined amount of memory
     */

    window->search.n_results_allocated 		= BKR_SEARCH_CONTEXT_HASH;
    window->search.n_filenames_allocated	= BKR_SEARCH_CONTEXT_HASH;

    /*
     * allocate an array of RESULTS
     * allocate an array of filenames
     */

    if (window->search.results == NULL) 
        window->search.results = (BKR_SEARCH_RESULTS *)
            BKR_CALLOC ( BKR_SEARCH_CONTEXT_HASH, sizeof( BKR_SEARCH_RESULTS ));

    if (window->search.filenames == NULL) 
        window->search.filenames = (char **)
	    BKR_CALLOC ( BKR_SEARCH_CONTEXT_HASH, sizeof( char * ));

}   /* end of bkr_initialize_search_context */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_delete_search_context()
**
**	deletes the search context for a Bookreader window
**
**  FORMAL PARAMETERS:
**
**	window - pointer to the Bookeader in which the search is being
**               conducted.
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
**	deletes bkr_search_context
**
**--
**/
void
bkr_delete_search_context PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    int 	i;

    /*
     * start off by cleaning up the context
     */

    bkr_reset_search_context( window );

    /*
     * then free up the results and filenames arrays.
     */

    if (window->search.results) 
        BKR_CFREE( window->search.results );
    window->search.results = NULL;

    if (window->search.filenames) 
	BKR_CFREE( window->search.filenames );
    window->search.filenames = NULL;

    /*
     * reset the allocation counters
     */

    window->search.n_results_allocated 		= 0;
    window->search.n_filenames_allocated	= 0;

}   /* end of bkr_delete_search_context */



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_reset_search_context()
**
**	resets the search context for a Bookreader window.  This routine should
**	be called whenever a new search is begun.
**
**  FORMAL PARAMETERS:
**
**	window - pointer to the Bookeader in which the search is being
**               conducted.
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
bkr_reset_search_context PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    int 	i;

    /*
     * first clean out the name field from RESULTS
     */

    if (window->search.results) 
	{
	for( i = 0; i < window->search.n_results; i++)
            if( window->search.results[i].name )
		BKR_CFREE( window->search.results[i].name );
        }

    /*
     * clean out the filenames
     */

    if (window->search.filenames) 
	{
	for( i = 0; i < window->search.n_filenames; i++)
            if( window->search.filenames[i])
		BKR_CFREE( window->search.filenames[i] );
    	}

    /*
     * then reset the counters
     */

    window->search.n_results 	= 0;
    window->search.n_filenames 	= 0;

}   /* end of bkr_reset_search_context */





/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_book_open_to_chunk()
**
** 	Opens a book to a chunk.  NOTE this routine written by Dave Ballenger
**
**  FORMAL PARAMETERS:
**
**  	filename    	 - string name of book to open.
**  	chunk_id         - id of chunk to display
**      view_in_default  - if true display in default topic window of book
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
**	Returns:    window ptr so we can highlight the search hit
**
**  SIDE EFFECTS:
**
**	opens a topic window with the desired chunk
**
**--
**/
BKR_WINDOW *
bkr_book_open_to_chunk PARAM_NAMES((filename,chunk_id,view_in_default))
    char    	  		*filename PARAM_SEP
    BMD_OBJECT_ID 		chunk_id PARAM_SEP
    Boolean       		view_in_default PARAM_END
{
    BKR_BOOK_CTX *book;
    BKR_SHELL	 *shell;
    BKR_WINDOW   *window;
    BKR_WINDOW   *window_rtn;


    window = NULL;

    /* First see if the book is already open
     */
    book = bkr_book_get(filename,NULL,0);
    if (book == NULL) 
	{
        return NULL;
    	}

    shell = bkr_shell_get(book,NULL,(view_in_default == FALSE));
    if (shell == NULL) 
	{
        bkr_book_free(book);
        return NULL;
    	}

    
    window_rtn = bkr_topic_open_to_position(shell,
                               ((view_in_default) ? shell->default_topic : window),
                               0,
                               0,0,
                               chunk_id,
                               view_in_default);

    
    return ( window_rtn );

};  /* end of bkr_book_open_to_chunk */





#ifdef CBR

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	start_cbr()
**
** 	Initializes the CBR engine and widgets
**
**  FORMAL PARAMETERS:
**
**	locator -- character string to help CBR callback the bookreader
**
**  IMPLICIT INPUTS:
**
**	bkr_toplevel_widget		/* top level widget
**	bkr_app_context 		/* application context */
**	bkr_display 			/* display */
**	bkr_cbr_results_cb()		/* CBR return point */
**
**  IMPLICIT OUTPUTS:
**
**	cbr_started gets set to TRUE
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	cbr_started gets set to TRUE
**
**--
**/
static void
start_cbr PARAM_NAMES((locator))
    char	*locator PARAM_END
{
    StartupCBR( 
	bkr_toplevel_widget, 
	bkr_app_context, 
	bkr_display, 
        "CBRDATA", 		/* hardwired location of data */
	bkr_cbr_results_cb, 	/* CBR return point */
	locator, 		/* filled in by CBR */
	NULL );			/* context ( not used ) */

    cbr_started = TRUE;

}   /* end of start_cbr */

#endif CBR



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_search_concept_list_cb()
**
** 	'concept list' callback
**
**  FORMAL PARAMETERS:
**      
**	widget
**	tag
**	data
**
**  IMPLICIT INPUTS:
**
**	cbr_started
**
**  IMPLICIT OUTPUTS:
**
**	locator - string used to display desired topics, freed by 
**		  bkr_cbr_get_topic.  passed to CBR.
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	passes control to CBR concept list widget
**
**--
**/

void
bkr_search_concept_list_cb PARAM_NAMES((widget,tag,data))
    Widget			widget PARAM_SEP
    Opaque			*tag PARAM_SEP
    XmAnyCallbackStruct		*data PARAM_END
{
    char			*locator;	/* ptr to locator string */

#ifdef CBR
    /* 
     * allocate CBR locator. get huge amt for now (FIX ME!!)
     */

    locator = (char *) BKR_CALLOC( 1000, sizeof( char ) );	
						
    if (! cbr_started )				/* start up CBR if need be */ 
	start_cbr( locator );

     /* startup concept list */
     
    DoConcepts();
#endif
}   /* end of bkr_search_concept_list_cb */




/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_search_edit_concept_cb()
**
** 	'concept list' callback
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**	cbr_started
**
**  IMPLICIT OUTPUTS:
**
**	locator - string used to display desired topics, freed by 
**		  bkr_cbr_get_topic.  passed to CBR.
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	passes control to CBR concept editor widget
**
**--
**/

void
bkr_search_edit_concept_cb PARAM_NAMES((widget,tag,data))
    Widget			widget PARAM_SEP
    Opaque			*tag PARAM_SEP
    XmAnyCallbackStruct		*data PARAM_END
{
    char			*locator;	/* ptr to locator string */


#ifdef CBR
    /* 
     * allocate CBR locator. get huge amt for now (FIX ME!!)
     */

    locator = (char *) BKR_CALLOC( 1000, sizeof( char ) );	
						
    if (! cbr_started )				/* start up CBR if need be */ 
	start_cbr( locator );

    /* startup concept list */
     
    DoEdit();
#endif
}   /* end of bkr_search_edit_concept_cb */

#endif /* SEARCH */
