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
***********************************************************
**                                                        *
**  Copyright (c) Digital Equipment Corporation, 1990  	  *
**  All Rights Reserved.  Unpublished rights reserved	  *
**  under the copyright laws of the United States.	  *
**                                                        *
**  The software contained on this media is proprietary	  *
**  to and embodies the confidential technology of 	  *
**  Digital Equipment Corporation.  Possession, use,	  *
**  duplication or dissemination of the software and	  *
**  media is authorized only pursuant to a valid written  *
**  license from Digital Equipment Corporation.	    	  *
**  							  *
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	  *
**  disclosure by the U.S. Government is subject to	  *
**  restrictions as set forth in Subparagraph (c)(1)(ii)  *
**  of DFARS 252.227-7013, or in FAR 52.227-19, as	  *
**  applicable.	    					  *
**  		                                          *
***********************************************************
**/


/*
**++
**  Subsystem:
**	DXmHelp
**
**  Version: V1.0
**
**  Abstract:
**	This module contains all the routines needed for opening and closing
**	the Help Libraries using the LBR$ routines. It also contains routines
**	to free the memory allocated for the cache structure.
**
**  Keywords:
**	open, close, free
**
**  Environment:
**	User mode, executable image
**
**  Author:
**	Andre Pavanello, CASEE Group, MEMEX Project
**
**  Creation Date: 2-Feb-88
**
**  Modification History:
**
**		Rich					23-Jul-90
**	    Remove DXm/ from private include file includes
**
**		Rich					11-Jul-90
**	    Transmit widget id into LBR_OPEN.
**
**		Leo					02-Aug-89
**	    Q&D port to Motif
**
**  BL11.1	Andre Pavanello				11-Oct-88
**	    use XtMalloc and XtFree only
**
**  IFT2	Andre Pavanello				10-Aug-88
**	    Turn AST back on after open/close failure
**
**  BL8.2	Andre Pavanello				13-May-88
**	    Initialize the nosearch field in the index structure
**
**  X003-0-9	Andre Pavanello				7-Mar-88
**	    Fix up include file name for Ultrix
**
**  X003-0-8	Andre Pavanello				26-Feb-88
**	    Remove library name upcasing (not good for Ultrix)
**	    Remove prototype declaration for pcc
**	    Upcase strings before comparing them
**
**  X003-0-7	Andre Pavanello				25-Feb-88
**	    Add some conditional compilation for port to Ultrix
**
**	    Fix free_include routine for freeing the right pointers
**
**  X003-0-6	Andre Pavanello				23-Feb-88
**	    replace STR$COMPARE_MULTI with the C equivalent routine
**	    (This is for the first port to Ultrix)
**
**  X003-0-5	Andre Pavanello				18-Feb-88
**	    modify init_help to not open the library every time
**
**  X003-0-4	Andre Pavanello				15-Feb-88
**	    Add time check every time the library is opened
**
**  X003-0-3	Andre Pavanello				15-Feb-88
**	    Merge Init_lib and Open_lib
**
**  X003-0-2	Andre Pavanello				10-Feb-88
**	    Check appropriate flag before open operation
**
**  X003-0-1	Andre Pavanello				8-Feb-88
**	    Add new routines for global open support.
**	    
**  X001-0-1	Andre Pavanello				3-Feb-88
**	    return to the caller the status returned by LBR$GET_INDEX
**
**	    Add check for freeing context if not null.
**
**  V2 BL4	Ross Faneuf				24-May-1989
**          Add current I18N semantic of attempting to open file
**          in local directory before attempting default (VMS)
**
**--
*/

/*
**  Include Files
*/
#include <DXm/DXmHelpBP.h>
#include "Help_Lbr_Def.h"

/*
**  Table of Contents
*/
static int fill_in_dsc();
static int DXmHelp_alloc_context_block();
static int DXmHelp_close_lib();
static int DXmHelp_check_lib();
static int DXmHelp_free_context_block();
static int collect_index();
static int DXmHelp_insert_lib();
static int reset_toplevel_entry();
    
/*
**  Macro Definitions
*/
#define LBR_C_READ	1
#define LBR_C_TYP_HLP	3
#define INDEX_NUMBER	1
#define LIB_NOT_FND     0
#define LIB_EXIST       1
    
/*
**  Global Data Definitions
*/
externaldef(help_hlb_init)
int            first_entry = TRUE;     /* flag for colecting top level index */

externaldef(help_hlb_init)
INDEX_TYPE_PTR index_head_ptr = NULL,  /* pointer to the first element */
	       index_tail_ptr;         /* pointer to the last element */
	       
externaldef(help_hlb_init)
LIBRARY_CONTEXT_BLOCK_PTR cntxt_head_ptr = NULL; /* pointer to the first    */
						 /* library context block   */
						 /* in the list */

/*
**  External Data Declarations
*/
#ifdef VMS
extern int LBR$INI_CONTROL(),
           LBR$OPEN(),
           LBR$CLOSE(),
	   LBR$GET_INDEX(),
	   SYS$SETAST();
#else
extern int LBR_INI_CONTROL(),
           LBR_OPEN(),
           LBR_CLOSE(),
	   LBR_GET_INDEX(),
	   SYS_SETAST();
#endif
	

static int fill_in_dsc(str, str_len, dsc_ptr)
char *str;
int str_len;
DSC_DESCRIPTOR *dsc_ptr;
/*
**++
**  Functional Description:
**	fills in a descriptor with a given C string. The string pointed by
**	the descriptor will be allocated.
**
**  Keywords:
**	none
**
**  Arguments:
**	str     : a pointer to the input string
**	str_len : length of the input string
**	dsc_ptr : a pointer to the descriptor to be filled
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	none
**--
*/
    {
    int i;
    
    if (str_len < 0)
	str_len = strlen(str);
/*
** Fill in descriptor fields
*/
    dsc_ptr->dsc_w_length = str_len;
    dsc_ptr->dsc_b_dtype  = DSC_K_DTYPE_T;
    dsc_ptr->dsc_b_class  = DSC_K_CLASS_S;
    dsc_ptr->dsc_a_pointer = (char *) XtMalloc(str_len + 1);
/*
** copy the string
*/
    DXmBCopy(str, dsc_ptr->dsc_a_pointer, str_len);
    dsc_ptr->dsc_a_pointer[str_len] = '\0';

    return (SUCCESS);
    }

static int DXmHelp_alloc_context_block(lib_spec_dsc_ptr, lib_cntxt_ptr_ptr,
				       help_widget)
DSC_DESCRIPTOR *lib_spec_dsc_ptr;
LIBRARY_CONTEXT_BLOCK_PTR *lib_cntxt_ptr_ptr;
DXmHelpWidget	help_widget;
/*
**++
**  Functional Description:
**	this routine allocates and initializes a library context block.
**	The library name is loaded in the structure.
**
**  Keywords:
**	initialize
**
**  Arguments:
**	lib_spec_dsc_ptr  : library specification descriptor pointer
**	lib_cntxt_ptr_ptr : a pointer to a pointer to a library context block
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	none
**--
*/
    {
    LIBRARY_CONTEXT_BLOCK_PTR lib_cntxt_ptr;

    lib_cntxt_ptr = (LIBRARY_CONTEXT_BLOCK_PTR) XtMalloc(sizeof(LIBRARY_CONTEXT_BLOCK));
    fill_in_dsc(lib_spec_dsc_ptr->dsc_a_pointer,
                lib_spec_dsc_ptr->dsc_w_length,
		&(lib_cntxt_ptr->lib_spec));
    lib_cntxt_ptr->lib_open = FALSE;
    lib_cntxt_ptr->open_cnt = 0;
    lib_cntxt_ptr->next = NULL;
    lib_cntxt_ptr->cache_ptr = NULL;
    lib_cntxt_ptr->caching = 0;
    lib_cntxt_ptr->search_mode = 0;
    lib_cntxt_ptr->help_widget_id = help_widget;
    *lib_cntxt_ptr_ptr = lib_cntxt_ptr;

    return (SUCCESS);
    }

int DXmHelp_open_lib(lib_cntxt_ptr)
LIBRARY_CONTEXT_BLOCK_PTR lib_cntxt_ptr;
/*
**++
**  Functional Description:
**	initializes and opens a Help Library using LBR$INIT_CONTROL and
**	LBR$OPEN.
**
**  Keywords:
**	open
**
**  Arguments:
**	lib_cntxt_ptr : a pointer to a library context block
**
**  Result:
**	any condition value returned by LBR$INIT_CONTROL, LBR$OPEN 
**
**  Exceptions:
**	none
**--
*/
    {
    DSC_DESCRIPTOR default_spec;
    int lbr_status,
        ast_status,
	access_mode = LBR_C_READ,
	lib_type    = LBR_C_TYP_HLP;

    AST_OFF(ast_status);
/*    
** check if the library isn't already open
*/
    if (!(lib_cntxt_ptr->lib_open))
	{
/*
** Initialize LBR$ structures
*/
#ifdef VMS
        lbr_status = LBR$INI_CONTROL(&(lib_cntxt_ptr->lbr_index),
                                     &access_mode, &lib_type);
#else
        lbr_status = LBR_INI_CONTROL(&(lib_cntxt_ptr->lbr_index),
                                     &access_mode, &lib_type);
#endif

/*				     
**	Check returned status for success
*/
	if ((lbr_status & 1) != 0 )
	    {
/*
**	    Open the library. First attempt an open in the local directory,
**	    then in the default (may vary by platform).
*/
#ifdef VMS
	    default_spec.dsc_b_dtype   = DSC_K_DTYPE_T;
	    default_spec.dsc_b_class  = DSC_K_CLASS_S;
	    default_spec.dsc_a_pointer = LOCAL_SPEC;
	    default_spec.dsc_w_length  = strlen(LOCAL_SPEC);
	    lbr_status = LBR$OPEN(&(lib_cntxt_ptr->lbr_index),
	                          &(lib_cntxt_ptr->lib_spec),
				  0,
				  &default_spec);
	    if ((lbr_status & 1) == 0)
		{
		lbr_status = LBR$INI_CONTROL(&(lib_cntxt_ptr->lbr_index),
					     &access_mode, &lib_type);
		default_spec.dsc_a_pointer = DEFAULT_SPEC;
		default_spec.dsc_w_length  = strlen(DEFAULT_SPEC);
		lbr_status = LBR$OPEN(&(lib_cntxt_ptr->lbr_index),
				      &(lib_cntxt_ptr->lib_spec),
				      0,
				      &default_spec);
		}
#else
	    lbr_status = LBR_OPEN(&(lib_cntxt_ptr->lbr_index),  /* index */
	                          &(lib_cntxt_ptr->lib_spec),	/* file spec */
				  0,				/* create opt*/
				  0,				/* def name str*/
				  0,				/* rel file spec*/
				  0,				/* result fs */
				  0,				/* res nam fs len*/
				  lib_cntxt_ptr->help_widget_id); /* help widget*/
#endif

/*
**	    Check returned status for success
*/
	    if ((lbr_status & 1) != 0)
		{
/*
**		Mark the libary as open
*/
		lib_cntxt_ptr->lib_open = TRUE;
		}
	    }
	}
	
    AST_ON(ast_status);
    
    return (lbr_status);
    }

static int DXmHelp_close_lib(lib_cntxt_ptr)
LIBRARY_CONTEXT_BLOCK_PTR lib_cntxt_ptr;
/*
**++
**  Functional Description:
**	closes a Help Library using the LBR$ routine
**
**  Keywords:
**	close
**
**  Arguments:
**	lib_cntxt_ptr : a pointer to a library context block
**
**  Result:
**	any condition value returned by LBR$CLOSE
**
**  Exceptions:
**	none
**--
*/
    {
    int lbr_status = TRUE;
    int ast_status;
    
    AST_OFF(ast_status);
/*    
** decrement the library open count
*/
    if (lib_cntxt_ptr->open_cnt > 0)
	lib_cntxt_ptr->open_cnt--;
/*
** check if we really have to close the library
*/    
    if ((lib_cntxt_ptr->open_cnt == 0) && (lib_cntxt_ptr->lib_open))
	{
#ifdef VMS
	lbr_status = LBR$CLOSE(&(lib_cntxt_ptr->lbr_index));
#else
	lbr_status = LBR_CLOSE(&(lib_cntxt_ptr->lbr_index));
#endif
	lib_cntxt_ptr->lib_open = FALSE;
	}
	
    AST_ON(ast_status);
    
    return (lbr_status);
    }

static int DXmHelp_check_lib(lib_spec_dsc_ptr, lib_cntxt_ptr_ptr)
DSC_DESCRIPTOR *lib_spec_dsc_ptr;
LIBRARY_CONTEXT_BLOCK_PTR *lib_cntxt_ptr_ptr;
/*
**++
**  Functional Description:
**	this routine checks in the list of library context blocks, if
**	the given library has already been read in.
**
**  Keywords:
**	lookup
**
**  Arguments:
**	lib_spec_dsc_ptr  : a library spec descriptor pointer
**	lib_cntxt_ptr_ptr : a pointer to a pointer to a library context block
**   
**  Non-local references:
**	cntxt_head_ptr : the begining of the list of library context blocks
**   
**  Non-local changes:
**	none
**   
**  Side effects:
**	none
**
**  Result:
**	LIB_NOT_FND : the given library is not in the list
**	LIB_EXIST   : the given library is in the list
**
**  Exceptions:
**	none
**--
*/
    {
    int  status = LIB_NOT_FND,
         ast_status;
    LIBRARY_CONTEXT_BLOCK_PTR scan_ptr;
/*
** turn on protection against interrupts
*/
    AST_OFF(ast_status);
/*    
** start at the begining of the list
*/
    scan_ptr = cntxt_head_ptr;
    while ((scan_ptr != NULL) && (status == LIB_NOT_FND))
	{
/*
** check library file name
*/
	
	if (case_less_str_equal(lib_spec_dsc_ptr->dsc_a_pointer,
	    scan_ptr->lib_spec.dsc_a_pointer))
	    {
/*
** return a pointer to its context block
*/
    	    *lib_cntxt_ptr_ptr = scan_ptr;
	    status = LIB_EXIST;
	    }
	else
	    scan_ptr = scan_ptr->next;
	}
	
    AST_ON(ast_status);
    
    return (status);
    }

static int DXmHelp_free_context_block(lib_cntxt_ptr)
LIBRARY_CONTEXT_BLOCK_PTR lib_cntxt_ptr;
/*
**++
**  Functional Description:
**	this routine frees a library context block and the library
**	in cache.
**
**  Keywords:
**	free
**
**  Arguments:
**	lib_cntxt_ptr : a pointer to a library context block
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	none
**--
*/
    {
    if (lib_cntxt_ptr != NULL)
	{
/*
** free the library file spec
*/
       	if (lib_cntxt_ptr->lib_spec.dsc_a_pointer != NULL)
	    XtFree(lib_cntxt_ptr->lib_spec.dsc_a_pointer);
/*
** free the library cache structure
*/
    	if (lib_cntxt_ptr->cache_ptr != NULL)
	    free_index(lib_cntxt_ptr->cache_ptr);
/*
** free the context block
*/
    	XtFree((char *)lib_cntxt_ptr);
	}
    
    return (SUCCESS);
    }

static int collect_index(key_dsc_ptr, rfa_ptr)
DSC_DESCRIPTOR *key_dsc_ptr;
RFA_TYPE *rfa_ptr;
/*
**++
**  Functional Description:
**	this routines collects all the level 1 indeces of the Help Library.
**	It is called from LBR$GET_INDEX.
**
**  Keywords:
**	collect
**
**  Arguments:
**	key_dsc_ptr : a pointer to the key name descriptor
**	rfa_ptr     : a pointer to the key's RFA
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {
/*
** First time round, remember to update the head of the list
*/
    if (first_entry)
	{
	index_head_ptr = (INDEX_TYPE_PTR) XtMalloc(sizeof(INDEX_TYPE));
	index_tail_ptr = index_head_ptr;
	first_entry = FALSE;
	}
    else
/*
** otherwise, just append to it
*/
	{
	index_tail_ptr->next = (INDEX_TYPE_PTR) XtMalloc(sizeof(INDEX_TYPE));
	index_tail_ptr = index_tail_ptr->next;
	}
/*
** store the RFA
*/    
    index_tail_ptr->rfa.rfa_high = rfa_ptr->rfa_high;
    index_tail_ptr->rfa.rfa_low = rfa_ptr->rfa_low;
/*
** store the key name (the C string and the descriptor)
*/
    index_tail_ptr->keyname.dsc_a_pointer = (char *) XtMalloc(key_dsc_ptr->dsc_w_length + 1);
    index_tail_ptr->keyname.dsc_w_length = key_dsc_ptr->dsc_w_length;
    index_tail_ptr->keyname.dsc_b_dtype = DSC_K_DTYPE_T;
    index_tail_ptr->keyname.dsc_b_class = DSC_K_CLASS_S;
    DXmBCopy(key_dsc_ptr->dsc_a_pointer,
	    index_tail_ptr->keyname.dsc_a_pointer,
	    key_dsc_ptr->dsc_w_length);
    index_tail_ptr->keyname.dsc_a_pointer[key_dsc_ptr->dsc_w_length] = '\0';
/*
** same for the full key
*/
    index_tail_ptr->full_key.dsc_a_pointer = (char *) XtMalloc(key_dsc_ptr->dsc_w_length + 1);
    index_tail_ptr->full_key.dsc_w_length = key_dsc_ptr->dsc_w_length;
    index_tail_ptr->full_key.dsc_b_dtype = DSC_K_DTYPE_T;
    index_tail_ptr->full_key.dsc_b_class = DSC_K_CLASS_S;
    DXmBCopy(key_dsc_ptr->dsc_a_pointer,
	     index_tail_ptr->full_key.dsc_a_pointer,
	     key_dsc_ptr->dsc_w_length);
    index_tail_ptr->full_key.dsc_a_pointer[key_dsc_ptr->dsc_w_length] = '\0';
/*
** initialize the other fields
*/
    index_tail_ptr->next = NULL;
    index_tail_ptr->sub_index = NULL;
    index_tail_ptr->title.dsc_a_pointer = NULL;
    index_tail_ptr->txt_buf_ptr = NULL;
    index_tail_ptr->qualifier_ptr = NULL;
    index_tail_ptr->include_ptr = NULL;
    index_tail_ptr->keyword_ptr = NULL;
    index_tail_ptr->in_memory = FALSE;
    index_tail_ptr->nosearch = FALSE;
        
    return (SUCCESS);
    }

int DXmHelp_collect_lib_index(lib_cntxt_ptr)
LIBRARY_CONTEXT_BLOCK_PTR lib_cntxt_ptr;
/*
**++
**  Functional Description:
**	this routine collects the top level index of a Help Library.
**	Interrupts must be turned off because the LBR$ routine is not
**	re-entrant.
**
**  Keywords:
**	collect
**
**  Arguments:
**	lib_cntxt_ptr : a pointer to a library context block
**   
**  Non-local references:
**	first_entry : when TRUE tells the action routine passed to
**	              LBR$GET_INDEX to start a new list
**   
**  Non-local changes:
**	first_entry : is reset when the top level index is collected
**   
**  Side effects:
**	the top level index is stored in a cache in memory
**
**  Result:
**	any status code returned by LBR$GET_INDEX
**
**  Exceptions:
**	none
**--
*/
    {
    int status,
        ast_status,
	index_number = INDEX_NUMBER;
/*
** Protection against interrupts
*/
        AST_OFF(ast_status);
/*    
** collect the top level index
*/
#ifdef VMS
    status = LBR$GET_INDEX(&(lib_cntxt_ptr->lbr_index), &index_number,
                           collect_index);
#else
    status = LBR_GET_INDEX(&(lib_cntxt_ptr->lbr_index), &index_number,
                           collect_index);
#endif
/*
** store the start point of the index
*/
    lib_cntxt_ptr->cache_ptr = index_head_ptr;
    first_entry = TRUE;
/*
** re-enable the interrupts
*/
    AST_ON(ast_status);
    
    return (status);
    }

static int DXmHelp_insert_lib(library_spec_dsc_ptr, new_cntxt_ptr_ptr)
DSC_DESCRIPTOR *library_spec_dsc_ptr;
LIBRARY_CONTEXT_BLOCK_PTR *new_cntxt_ptr_ptr;
/*
**++
**  Functional Description:
**	this routine inserts a library context block in the library context list
**	if needed.
**  Keywords:
**	none
**
**  Arguments:
**      library_spec_dsc_ptr  : the library file spec
**	new_cntxt_ptr_ptr     : a pointer to a pointer to a library context
**	                        block
**   
**  Side effects:
**	the global context list might be modified
**
**  Result:
**      SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {
    int	ast_status,
	check_lib_stat;
    LIBRARY_CONTEXT_BLOCK_PTR lib_cntxt_ptr,
			      new_cntxt_ptr;

    AST_OFF(ast_status);
    
    new_cntxt_ptr = *new_cntxt_ptr_ptr;
/*
** check if the library is in the list of contexts
*/
    check_lib_stat = DXmHelp_check_lib(library_spec_dsc_ptr, &lib_cntxt_ptr);

    if (check_lib_stat == LIB_EXIST)
	{
/*
** throw away the recently allocated context block 
*/
	DXmHelp_free_context_block(new_cntxt_ptr);
	*new_cntxt_ptr_ptr = lib_cntxt_ptr;
	}
    else
	{
/*
** register new user for this library
*/
	new_cntxt_ptr->open_cnt++;
/*
** Insert the new context block in the list
*/
	new_cntxt_ptr->next = cntxt_head_ptr;
	cntxt_head_ptr = new_cntxt_ptr;
	}

    AST_ON(ast_status);

    return (SUCCESS);
    }

int DXmHelp__init_help(library_context_ptr_ptr, library_spec_dsc_ptr, caching,
                       help_widget_id)
LIBRARY_CONTEXT_BLOCK_PTR *library_context_ptr_ptr;
DSC_DESCRIPTOR *library_spec_dsc_ptr;
Boolean caching;
DXmHelpWidget	help_widget_id;
/*
**++
**  Functional Description:
**	This routine first checks if the specified library is already open.
**	If it isn't, it will open the library and add a control block in
**	the globaly maintained list of contexts. If the library is already
**	there, a counter will be incremented.
**
**  Keywords:
**	open, initialize
**
**  Arguments:
**	library_index_ptr_ptr : a pointer to a pointer to the library context block
**      library_spec_dsc_ptr  : the library file spec
**   
**  Side effects:
**	none
**
**  Result:
**      SUCCESS,
**	any status code returned by LBR$ routines
**
**  Exceptions:
**	None
**--
*/
    {
    int			      status = SUCCESS,
			      check_lib_stat;
    LIBRARY_CONTEXT_BLOCK_PTR lib_cntxt_ptr,
			      new_cntxt_ptr = NULL;
    /*
    ** Check if the library is in the cache structure
    */
    
    check_lib_stat = DXmHelp_check_lib(library_spec_dsc_ptr, &lib_cntxt_ptr);
    if (check_lib_stat == LIB_NOT_FND)
	{
	
	/*	
	** Allocate a new context block, open the library and read in the
	** top level index
	*/

	DXmHelp_alloc_context_block(library_spec_dsc_ptr, &new_cntxt_ptr,
				    help_widget_id);
	ERROR_CHECK(DXmHelp_open_lib(new_cntxt_ptr));
	ERROR_CHECK(DXmHelp_collect_lib_index(new_cntxt_ptr));
	DXmHelp_insert_lib(library_spec_dsc_ptr, &new_cntxt_ptr);
	if (caching)
	    new_cntxt_ptr->caching = TRUE;
	    
	/*	    
	** return the valid context
	*/
	
	*library_context_ptr_ptr = new_cntxt_ptr;
	}
    else
	{
	
	/*
	** otherwise register new user of the library
	*/
	
	lib_cntxt_ptr->open_cnt++;

	/*	
	**  Set the caching mode in the right state according to the request
	*/

	DXmHelp__set_cache_mode(lib_cntxt_ptr, caching);

	/*	
	** return the context pointer 
	*/
	
	*library_context_ptr_ptr = lib_cntxt_ptr;
	}

    return (status);
    }

int free_include(incl_ptr)
INCLUDE_TYPE_PTR incl_ptr;
/*
**++
**  Functional Description:
**	frees an include type structure
**
**  Keywords:
**	free
**
**  Arguments:
**	incl_ptr : pointer to the head of the include list structure
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {
    INCLUDE_TYPE_PTR to_free_ptr;

    while (incl_ptr != NULL)
	{

	/*
	**  Check if the title is a different string than the full_key
	*/
	
	if (!(incl_ptr->notitle))
	    XtFree(incl_ptr->title);
	    
	XtFree(incl_ptr->full_key);
	
	to_free_ptr = incl_ptr;
	incl_ptr = incl_ptr->next;
	
	XtFree((char *)to_free_ptr);
	}
	
    return (SUCCESS);
    }

int free_index(entry_ptr)
INDEX_TYPE_PTR entry_ptr;
/*
**++
**  Functional Description:
**	frees the whole structure in cache
**
**  Keywords:
**	free
**
**  Arguments:
**	entry_ptr : a pointer to the head of the list
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {

    if (entry_ptr != NULL) {

	/*
	** if there is a child, free it before
	*/
	
	if (entry_ptr->sub_index != NULL)
	    free_index(entry_ptr->sub_index);
	/*
	** if there is a sibling, free it before
	*/
	
	if (entry_ptr->next != NULL)
	    free_index(entry_ptr->next);

	if (entry_ptr->keyname.dsc_a_pointer != NULL)
	    XtFree(entry_ptr->keyname.dsc_a_pointer);

	if (entry_ptr->full_key.dsc_a_pointer != NULL)
	    XtFree(entry_ptr->full_key.dsc_a_pointer);
	
	if (entry_ptr->title.dsc_a_pointer != NULL)
	    XtFree(entry_ptr->title.dsc_a_pointer);
	    
	if (entry_ptr->keyword_ptr != NULL)
	    free_key(entry_ptr->keyword_ptr);
	    
	if (entry_ptr->include_ptr != NULL)
	    free_include(entry_ptr->include_ptr);

	if (entry_ptr->txt_buf_ptr != NULL)
	    XtFree(entry_ptr->txt_buf_ptr);
	    
	if (entry_ptr->qualifier_ptr != NULL)
	    XtFree(entry_ptr->qualifier_ptr);
	    
	XtFree((char *)entry_ptr);

    }
	
    return (SUCCESS);
    }

int DXmHelp__close_help(context_ptr)
LIBRARY_CONTEXT_BLOCK_PTR context_ptr;
/*
**++
**  Functional Description:
**	closes the library, frees the cache structure and the context block
**
**  Keywords:
**	close, free
**
**  Arguments:
**	context_ptr : a pointer to the context block structure
**
**  Result:
**	returns any status from the LBR$CLOSE routine
**
**  Exceptions:
**	None
**--
*/
    {
    int status;

    if (context_ptr != NULL)
	ERROR_CHECK(DXmHelp_close_lib(context_ptr));
    
    return (status);
    }


int DXmHelp__set_cache_mode(context, enable)
LIBRARY_CONTEXT_BLOCK_PTR context;
Boolean enable;
/*
**++
**  Functional Description:
**	Turns the cahing mode on or off. If turned off, the cache is freed
**
**  Keywords:
**	cache, free
**
**  Arguments:
**	enable  : boolean for the caching mode
**	context : a pointer to the context block structure
**
**  Result:
**	SUCCESS
**	NULL_CONTEXT
**	
**  Exceptions:
**	None
**--
*/
    {
    int status = SUCCESS;
    INDEX_TYPE_PTR top_level_index;

    if (context != NULL) {
    
	if (enable) {
	
	    /*
	    **  Turn caching on
	    */
	    
	    context->caching = 1;
	}
	else {

	    /*
	    **  Turn the cache off 
	    */

	    context->caching = 0;

	    /*
	    **  Free the cache if any but save the top level index
	    */

	    if (context->cache_ptr != NULL) {
		top_level_index = context->cache_ptr;
		while (top_level_index != NULL) {
		    reset_toplevel_entry(top_level_index);
		    top_level_index = top_level_index->next;
		}
	    }
	}
    }
    else {
	status = NULL_CONTEXT;
    }

    return(status);
    }

static int reset_toplevel_entry(entry)
INDEX_TYPE *entry;
    {

    /*
    **  Free all cached fields. Keep the keyname, title and full key for
    **	no-cache retrieval
    */
    
    if (entry->sub_index != NULL) {
	free_index(entry->sub_index);
	entry->sub_index = NULL;
    }
    
    if (entry->txt_buf_ptr != NULL) {
	XtFree(entry->txt_buf_ptr);
	entry->txt_buf_ptr = NULL;
	entry->txt_buf_siz = 0;
    }
    
    if (entry->qualifier_ptr != NULL) {
	XtFree(entry->qualifier_ptr);
	entry->qualifier_ptr = NULL;
    }
    
    if (entry->keyword_ptr != NULL) {
	free_key(entry->keyword_ptr);
	entry->keyword_ptr = NULL;
    }
    
    if (entry->include_ptr != NULL) {
	free_include(entry->include_ptr);
	entry->include_ptr = NULL;
    }
    
    entry->in_memory = 0;

    return(SUCCESS);
    }
