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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BR_GLOBALS.H*/
/* *9    20-SEP-1992 18:21:13 BALLENGER "Fix hotspot selection for character cell."*/
/* *8    16-JUN-1992 14:49:52 ROSE "Added globals to manage message boxes during Search"*/
/* *7    19-MAR-1992 12:02:17 FITZELL "added structure for keeping track of whats highlighted"*/
/* *6    18-MAR-1992 15:37:18 KARDON "Add literal for max file size"*/
/* *5     8-MAR-1992 19:13:56 BALLENGER " Add topic data and text line support"*/
/* *4     4-MAR-1992 13:44:12 KARDON "Add some BookreaderPlus globals"*/
/* *3     3-MAR-1992 17:12:03 KARDON "UCXed"*/
/* *2     1-NOV-1991 12:52:37 BALLENGER "Reintegrate memex support."*/
/* *1    16-SEP-1991 12:48:27 PARMENTER "Global Variables Declarations"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BR_GLOBALS.H*/
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
**	All global variable definitions for the Bookreader.
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     17-Apr-1990
**
**  MODIFICATION HISTORY:
**
**--
**/
#ifndef BR_GLOBALS_H
#define BR_GLOBALS_H

/*
 * INCLUDE FILES
 */

#include    <DXm/DXmSvn.h>
#include    <DXm/DECspecific.h>
#include    <Mrm/MrmPublic.h>
#include    "br_resource.h"


/*
 * GLOBAL VARIABLES
 */
#ifdef BKR_GLOBAL_DATA
#define INITIAL_VALUE(x) = x
#else
#define INITIAL_VALUE(x)
#define BKR_GLOBAL_DATA externalref
#endif 

/* Error message buffer - compile time allocation */

#define	    ERROR_BUFFER_SIZE	512
#define	    MAX_FILESPEC_LEN	255

BKR_GLOBAL_DATA Display 	*bkr_display;
BKR_GLOBAL_DATA MrmHierarchy   	bkr_hierarchy_id; 
BKR_GLOBAL_DATA int 	    	bkr_monitor_resolution;
BKR_GLOBAL_DATA Widget  	bkr_toplevel_widget INITIAL_VALUE(NULL); /* Unmanaged widget */
BKR_GLOBAL_DATA XtAppContext    bkr_app_context     INITIAL_VALUE(NULL);

BKR_GLOBAL_DATA BKR_WINDOW         *bkr_library INITIAL_VALUE(NULL);
BKR_GLOBAL_DATA BKR_BOOK_CTX       *bkr_open_books INITIAL_VALUE(NULL);
BKR_GLOBAL_DATA BKR_CLIENT         *bkr_client_context INITIAL_VALUE(NULL);
BKR_GLOBAL_DATA BKR_SHELL          *bkr_all_shells INITIAL_VALUE(NULL);
BKR_GLOBAL_DATA BKR_SHELL          *bkr_default_shell INITIAL_VALUE(NULL);

BKR_GLOBAL_DATA int    	    	bkr_display_width INITIAL_VALUE(0);
BKR_GLOBAL_DATA int    	    	bkr_display_height INITIAL_VALUE(0);

BKR_GLOBAL_DATA Pixel	    	bkr_window_foreground;
BKR_GLOBAL_DATA Pixel	    	bkr_window_background;

BKR_GLOBAL_DATA Boolean         bkr_char_cell INITIAL_VALUE(FALSE);
BKR_GLOBAL_DATA GC	    	bkr_extension_gc INITIAL_VALUE(NULL);
BKR_GLOBAL_DATA GC   	    	bkr_outline_off_gc INITIAL_VALUE(NULL);
BKR_GLOBAL_DATA GC   	    	bkr_text_gc INITIAL_VALUE(NULL);
BKR_GLOBAL_DATA GC   	    	bkr_reverse_text_gc INITIAL_VALUE(NULL);
BKR_GLOBAL_DATA GC   	    	bkr_xor_gc INITIAL_VALUE(NULL);
BKR_GLOBAL_DATA XFontStruct     *bkr_default_font INITIAL_VALUE(NULL);
BKR_GLOBAL_DATA int             bkr_default_space_width INITIAL_VALUE(0);
BKR_GLOBAL_DATA int             bkr_default_line_height INITIAL_VALUE(0);

BKR_GLOBAL_DATA BKR_LIBRARY_RESOURCES  bkr_library_resources;
BKR_GLOBAL_DATA BKR_RESOURCES  	       bkr_resources;
BKR_GLOBAL_DATA BKR_TOPIC_RESOURCES    bkr_topic_resources;


BKR_GLOBAL_DATA int    	    	bkrplus_g_allow_charcell	INITIAL_VALUE(0);
BKR_GLOBAL_DATA int    	    	bkrplus_g_allow_copy		INITIAL_VALUE(0);
BKR_GLOBAL_DATA int    	    	bkrplus_g_allow_cbr		INITIAL_VALUE(0);
BKR_GLOBAL_DATA int    	    	bkrplus_g_allow_search		INITIAL_VALUE(0);
BKR_GLOBAL_DATA int    	    	bkrplus_g_allow_print		INITIAL_VALUE(0);
BKR_GLOBAL_DATA int    	    	bkrplus_g_charcell_display	INITIAL_VALUE(0);

BKR_GLOBAL_DATA Boolean    	bkrplus_g_search_in_progress	INITIAL_VALUE(FALSE);
BKR_GLOBAL_DATA Boolean    	bkrplus_g_message_during_search INITIAL_VALUE(FALSE);

/* error msg. array */
BKR_GLOBAL_DATA char    	    	errmsg[ERROR_BUFFER_SIZE];

/* highlighted/selected text */
BKR_GLOBAL_DATA BKR_GLOBAL_SELECTION bkr_copy_ctx;
#endif
