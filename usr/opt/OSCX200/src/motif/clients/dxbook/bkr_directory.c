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

/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_DIRECTORY.C*/
/* *12   19-JUN-1992 20:19:14 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *11    8-JUN-1992 19:04:14 BALLENGER "UCX$CONVERT"*/
/* *10    8-JUN-1992 12:44:01 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *9    15-APR-1992 22:01:37 GOSSELIN "fixing for ALPHA"*/
/* *8    20-MAR-1992 11:00:30 GOSSELIN "conditionalized volatile"*/
/* *7    20-MAR-1992 10:55:30 GOSSELIN "removed volatile"*/
/* *6    19-MAR-1992 14:59:50 GOSSELIN "made fix for ALPHA"*/
/* *5    19-MAR-1992 14:57:34 GOSSELIN "made fix for ALPHA"*/
/* *4     3-MAR-1992 16:57:57 KARDON "UCXed"*/
/* *3     1-NOV-1991 13:06:50 BALLENGER "Reintegrate  memex support"*/
/* *2    17-SEP-1991 19:35:07 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:39:04 PARMENTER "Opening directories for Selection window"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_DIRECTORY.C*/
/*  DEC/CMS REPLACEMENT HISTORY, Element BKR_DIRECTORY.C */
/*  *14   15-APR-1992 15:11:10 FISHER "Make a couple more statics volatile" */
/*  *13   30-MAR-1992 16:42:12 GOSSELIN "updated with BookreaderPlus EFT code" */
/*  *12   28-FEB-1992 14:35:06 GOSSELIN "workaround for ALPHA compiler bug" */
/*   10A1 28-FEB-1992 14:29:09 GOSSELIN "WORKAROUND FOR OPTIMIZER BUG" */
/*  *11   15-NOV-1991 10:19:19 GOSSELIN "checked in common sources for VDM 1.1 BL2" */
/*  *10   14-JUN-1991 13:46:17 BALLENGER " Make sure initialize_directory always returns a value" */
/*  *9    14-JAN-1991 10:07:12 FERGUSON "Fix for corrupt directory error checking" */
/*  *8     6-JAN-1991 20:21:20 KRAETSCH "Fix holes in SVN levels" */
/*  *7    20-SEP-1990 19:58:52 FERGUSON "update first_directory_to_open resource" */
/*  *6    21-AUG-1990 17:26:42 FERGUSON "pre-ift (BL6) checkins - MEMEX support" */
/*  *5    20-JUL-1990 19:30:25 FERGUSON "BL5 checkins - new SVN Selection window and DPS support" */
/*  *4    22-JUN-1990 15:46:57 FERGUSON "BL4 checkins" */
/*  *3    13-JUN-1990 16:49:17 FERGUSON "pre-BL4 checkins" */
/*  *2    25-MAY-1990 16:02:59 FERGUSON "Initial Motif checkins for V3" */
/*  *1    27-APR-1990 16:46:28 FERGUSON "Initial entry of V3 sources" */
/*  DEC/CMS REPLACEMENT HISTORY, Element BKR_DIRECTORY.C */
#ifndef VMS
 /*
#else
# module BKR_DIRECTORY "V03-0002"
#endif
#ifndef VMS
  */
#endif

#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxbook/bkr_directory.c,v 1.1.4.2 1993/08/24 15:59:23 Ellen_Johansen Exp $";
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
**	Routines for opening directories for a Selection window.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     24-Jan-1990
**
**  MODIFICATION HISTORY:
**
**      V03-0002    DLB0001     David L Ballenger       13-Jun-1991
**                  Make sure initialize_directory() always returns a valid
**                  value.
**
**	V03-0001    JAF0001	James A. Ferguson   	24-Jan-1990
**  	    	    Extracted V2 routines and created new module.
**
**--
**/


/*
 * INCLUDE FILES
 */
#include "br_common_defs.h"
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "br_resource.h"     /* typedefs and #defines for Xrm customizable
                                resources used in BR */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_directory.h"   /* function prototypes */
#include "bri_dir.h"

/*
 * FORWARD ROUTINES
 */

static    int
initialize_directory PROTOTYPE((BKR_DIR_ENTRY   *parent, 
                                BR_UINT_32 parent_entry_num));

#ifdef TRACE_OUTPUT
static       void  	    print_dir_entry();
#endif /* TRACE_OUTPUT */


/*
 * FORWARD DEFINITIONS
 */

static BMD_BOOK_ID	    current_book_id = NULL;



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_directory_open
**
** 	Opens a directory given the book id and directory id, and initializes
**  	the directory entry list.
**
**  FORMAL PARAMETERS:
**
**	book_id	    	- id of the book containing the directory.
**	dir_id	    	- id of the directory to open.
**	toplevel_entry	- pointer to the DIRECTORY structure to initialize.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	The fields "children" and "num_entries" are modified in the 
**  	structure pointer to by toplevel_entry.
**
**  COMPLETION CODES:
**
**	Returns:    1 - if directory was opened successfully.
**  	    	    0 - if the open failed.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
Boolean
bkr_directory_open PARAM_NAMES((book_id,toplevel_entry))
    BMD_BOOK_ID	    book_id PARAM_SEP
    BKR_DIR_ENTRY   *toplevel_entry PARAM_END

{
    BMD_OBJECT_ID  ldir_id;
    BR_UINT_32	    dir_flags;	    	/* directory flags */
    BR_UINT_32 	    num_entries;
    BKR_DIR_ENTRY   *dir_list;

    if ( toplevel_entry->entry_type != DIRECTORY )
	return( FALSE );

    /* Open the directory, and get the flags and number of entries */

    ldir_id = bri_directory_open(
	    	book_id,
	    	toplevel_entry->object_id,
	    	&dir_flags,
	    	&num_entries );
    if ( ldir_id == 0 )
	return( FALSE );

    /* Allocate a buffer for the directory entries */

    current_book_id = book_id;
    dir_list = (BKR_DIR_ENTRY *) BKR_CALLOC( num_entries, sizeof( BKR_DIR_ENTRY ) );
    memset( dir_list, 0, ( num_entries * sizeof( BKR_DIR_ENTRY ) ) );
    toplevel_entry->children = dir_list;
    toplevel_entry->u.directory.num_entries = num_entries;

    (void) initialize_directory( toplevel_entry, 0 );
    current_book_id = NULL;

#ifdef TRACE_OUTPUT
 {
    BKR_DIR_ENTRY   *entry;
    int	    i;
    print_dir_entry( toplevel_entry );
    for ( i = 1, entry = toplevel_entry->children; 
    	    entry != NULL; entry = entry->sibling )
    {
    	print_dir_entry( entry );
    }
 };
#endif /* TRACE_OUTPUT */

    return ( TRUE );

};  /* end of bkr_directory_open */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	initialize_directory
**
** 	Initializes each entry with in a directory given the directory list.
**  	This is a recursive routine which initializes each directory entry
**  	by call BRI and at the same time counts the number of children for
**  	the parent.
**
**  FORMAL PARAMETERS:
**
**	parent		 - pointer to the parent entry.
**  	parent_entry_num - position of the parent entry within the list.
**
**  IMPLICIT INPUTS:
**
**  	dir_list    	- pointer to children to intialize.
**  	dir_id	    	- id of directory to get entry data from.
**  	num_entries 	- number of entries in directory.
**	current_book_id - id of book which contains directory being initialized.
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns:    position number of the next ancestor in the directory list.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static int 
initialize_directory PARAM_NAMES(( parent, parent_entry_num ))
    BKR_DIR_ENTRY   *parent PARAM_SEP
    BR_UINT_32	    parent_entry_num PARAM_END
{
#ifdef VMS
    static BKR_DIR_ENTRY    * volatile dir_list = NULL;
    static BMD_OBJECT_ID    volatile dir_id = 0;
    static BR_UINT_32  	    volatile num_entries = 0;
#else
    static BKR_DIR_ENTRY    * dir_list = NULL;
    static BMD_OBJECT_ID   dir_id = 0;
    static BR_UINT_32      num_entries = 0;
#endif
    BKR_DIR_ENTRY   	    *entry;
    BR_UINT_32	    	    entry_num;
    BKR_DIR_ENTRY   	    *prev_sibling;
    BKR_DIR_ENTRY   	    *prev_entry;

    /* Save the dir id and number of entries for this directory */

    if ( parent->entry_type == DIRECTORY )
    {
    	dir_list = parent->children;
    	dir_id = parent->object_id;
    	num_entries = parent->u.directory.num_entries;
    }

    /* Set the sibling pointers and count the children at each level */

    entry_num = parent_entry_num + 1;
    prev_sibling = NULL;
    do
    {
    	entry = (BKR_DIR_ENTRY *) &dir_list[entry_num - 1];
	entry->object_id = bri_directory_entry(
    	    	    	current_book_id,
	    	    	dir_id,
	    	    	entry_num,
	    	    	NULL,
	    	    	&entry->u.entry.target,
	    	    	&entry->level,
    	    	    	NULL,	    	    /* width        */
    	    	    	NULL,   	    /* height       */
    	    	    	NULL,   	    /* data address */
    	    	    	NULL,   	    /* data length  */
    	    	    	NULL,   	    /* data type    */
	    	    	&entry->title );

    	entry->entry_type   = DIRECTORY_ENTRY;

	/*  Check for bad entry level--must be > 0 (only the directory
	 *  itself can be 0); must be < parent + 2 in all cases and must
	 *  be < parent_level + 1 if no previous siblings at this level
    	 *  NOTE: we force the first entry after the DIRECTORY parent to 1.
	 */

	if ( entry->level == 0 )
	    entry->level = 1;
	else if ( entry->level > parent->level + 2 )
	    entry->level = parent->level + 2;
	else if ( ( parent->children == NULL )
    	    	  && ( entry->level > ( parent->level + 1 ) ) )
	    entry->level = parent->level + 1;
    	else if ( ( parent->entry_type == DIRECTORY ) && ( entry_num == 1 ) )
	    entry->level = 1;	/* 1st entry after DIRECTORY parent */

    	if ( entry->level == parent->level + 1 )    /* Immediate child ? */
    	{
    	    if ( ( parent->num_children == 0 )	    /* First child in list */
    	    	 && ( parent->children == NULL ) )
    	    	parent->children = entry;
    	    parent->num_children++;
    	    if ( prev_sibling != NULL ) 	/* Set the sibling pointer */
    	    	prev_sibling->sibling = entry;
    	    prev_sibling = entry;
    	    entry_num++;
    	}
    	else if ( entry->level == parent->level + 2 )	/* New level */
    	{
    	    /*  Pass the previous entry and its entry number as the new parent.
    	     *
    	     *  NOTE: The previous entry is (entry_num - 2) because entry_num 
    	     *  was already incremented and because dir_list is a zero
    	     *  based array. 
    	     */

    	    prev_entry = (BKR_DIR_ENTRY *) &dir_list[entry_num - 2];
    	    entry_num = initialize_directory( prev_entry, 
    	    	    BRI_DIRECTORY_ENTRY_NUMBER( prev_entry->object_id ) );
    	}
    	else if ( entry->level <= parent->level )   /* Ancestor or sibling */
    	{
    	    return ( entry_num );
    	}
    	else
    	{
#ifdef TRACE_OUTPUT
    	    printf( "initialize_directory: error -- entry #%d \"%s\" \n", entry_num,
    	    	    	(entry->title) ? entry->title : "No title" );
    	    printf( "     @ level %d is an invalid level \n", entry->level );
    	    printf( "  parent entry #%d \"%s\" @ level %d \n", parent_entry_num,
    	    	(parent->title) ? parent->title : "No title", parent->level );
    	    printf( "Entry will not be counted\n" );
#endif
    	    entry_num++;
    	}

    } while ( entry_num <= num_entries );

    /* Reinitialize for next time */

    dir_list = NULL;
    dir_id = 0;
    num_entries = 0;
    
    return ( entry_num );
    
};  /* end of initialize_directory */


#ifdef TRACE_OUTPUT
static void
print_dir_entry( entry )
    BKR_DIR_ENTRY *entry;
{
    int	    i;
#ifdef LONG_OUTPUT
    printf( "--------\n" );
    printf( "   sibling       = %s \n", 
    	    	( entry->sibling ) ? entry->sibling->title : "None" );
    printf( "   num_children  = %d \n", entry->num_children );
    printf( "   level         = %d \n", entry->level );
    printf( "   title         = %s \n", entry->title );
#else
    for ( i = 0; i < entry->level * 3; i++ )
    	printf( " " );
    printf( " %s  (%d) \n", entry->title, entry->num_children );
#endif
};  /* end of print_dir_entry */
#endif /* TRACE_OUTPUT */



