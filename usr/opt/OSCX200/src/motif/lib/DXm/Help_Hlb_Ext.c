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

#ifndef VMS
#define module undef
#endif
/*
**++
**  Subsystem:
**	DXmHelp
**
**  Version: V1.0
**
**  Abstract:
**	This module contains all the routines needed to handle the HWHelp
**	extensions (=title, =keyword, =include) to the VMS HLB libraries.
**
**  Keywords:
**	extension
**
**  Environment:
**	User mode, executable image
**
**  Author:
**	Andre Pavanello, CASEE Group, MEMEX Project
**
**  Creation Date: 22-Jan-88
**
**  Modification History:
**
**		Rich					    20-Aug-90
**	    Add in DECIsreal changes.
**
**		Rich					    23-Jul-90
**	    Remove DXm/ from private include file includes
**
**		Rich					    07-Jun-90
**	    Need to include DXmHelpBP.h
**
**		Leo					    02-Aug-89
**	    Q&D port to Motif
**  
**  V2-bl4-2	Andre Pavanello				    14-Jun-89
**	    Modify the parse_include routine to insert new elements at the end
**	    of list directly instaed of running down the list each time
**	    Changes proposed by Ken Moreau, Debug Group
**
**  V2-bl4-1	Andre Pavanello				    13-May-89
**	    Add routines for non-cache support
**	    
**  BL11.1	Andre Pavanello				    11-Oct-88
**	    use XtMalloc and XtFree only
**
**  BL8.2	Andre Pavanello				    13-May-88
**	    no search done if frame is set to no search
**
**  X004-0-2	Andre Pavanello				    28-Apr-88
**	    Change keyword compare for exact match
**
**  X004-0-1	Andre Pavanello				    8-Apr-88
**	    Free the keyword list once the array is loaded
**
**  X003-1-2	Andre Pavanello				    30-Mar-88
**	    Fix bug in ordering keywords
**
**  X003-1-1	Andre Pavanello				    25-Mar-88
**	    remove duplicate keywords in collect_keyword and order them
**	    alphabetically
**	    check for full command extension name
**
**  X003-0-9	Andre Pavanello				    21-Mar-88
**	    Add routines for getting keywords in the library
**
**  X003-0-8	Andre Pavanello				    16-Mar-88
**	    Add routines for searching titles and keywords
**
**  X003-0-7	Andre Pavanello				    7-Mar-88
**	    Fix up include file name for Ultrix
**	    
**  X003-0-6	Andre Pavanello				    25-Feb-88
**	    Specify file extension on include file.
**
**  X003-0-5	Andre Pavanello				    25-Feb-88
**	    Include only the internal header file.
**
**  X003-0-4	Andre Pavanello				    24-Feb-88
**	    Store keyword in upper case.
**
**  X003-0-3	Andre Pavanello				    19-Feb-88
**	    Replaced call to read_module by a call to DXmHelp_set_read_context
**
**  X003-0-2	Andre Pavanello				    15-Feb-88
**	    Modified parse_extension to superseed the title extension if
**	    another one is specified in the same frame
**
**	    Modified parse extension to add to the list of keywords if the
**	    keyword extension is specified more than once
**	    
**  X003-0-1	Andre Pavanello				    15-Feb-88
**	    Added new function to display an error message for include
**	    keys which are not found in the library
**
**--
*/

/*
**  Include Files
*/
#include <DXm/DXmHelpBP.h>
#include "Help_Lbr_Def.h"

/*
**  Macro Definitions
*/
#define SUBSTR_FOUND	 1
#define SUBSTR_NOT_FOUND 0
    
/*
**  Table of Contents
*/
static int parse_include();
static int create_include_entry();
static int create_include_error();
static int find_substring();
static int search_title();
static int search_keyword();
static int build_array();
static int insert_keyword();
static int collect_keyword();
static int build_keyword_array();
	   

char *CopyStrN(str, len)
char *str;
int len;
/*
**++
**  Functional Description:
**	this routine allocates memory for a new string and copies the given
**	string into it
**
**  Keywords:
**	copy
**
**  Arguments:
**	str : address of a string
**	len : length of the string to be copied
**
**  Result:
**	address of the copied string
**
**  Exceptions:
**	None
**--
*/

    {
    char *tmp;

    tmp = (char *) XtMalloc(len);
    DXmBCopy(str, tmp, len);

    return(tmp);
    }
        

char *CopyStr(str)
char *str;
/*
**++
**  Functional Description:
**	this routine aloocates memory and copies the given string into it
**
**  Keywords:
**	copy
**
**  Arguments:
**	str : address of a string
**
**  Result:
**	address of copied string
**
**  Exceptions:
**	None
**--
*/

    {
    char *tmp;
    int len;

    len = strlen(str) + 1;
    
    tmp = (char *) XtMalloc(len);
    DXmBCopy(str, tmp, len);

    return(tmp);
    }
        

static int parse_include(incl_ptr_ptr, line, offset, entry_ptr)
INCLUDE_LIST_PTR *incl_ptr_ptr;
char *line;
int offset;
INDEX_TYPE_PTR entry_ptr;
/*
**++
**  Functional Description:
**	this routine parses the include extension. It builds an include
**	list with the key specified.
**
**  Keywords:
**	include
**
**  Arguments:
**	incl_ptr_ptr : head of the include list
**	line         : line to be parsed
**	offset       : start position in the line for starting the parsing
**	entry_ptr    : the index entry in which the include was found
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {
    int str_len = 0,
	parsing = TRUE;
    INCLUDE_LIST_PTR cur_ptr,
                    head_ptr = NULL,
		    prev_ptr = NULL,
                    in_ptr;
/*
** purge space
*/
    while (line[offset] == ' ')
	offset++;

    while (parsing)
	{
/*
** bump counters until we find a terminator
*/
	while ((line[offset] != ',') && (line[offset] != '\0')
	       && (line[offset] != '\n'))
	    {
	    str_len++;
	    offset++;
	    }
/*
** allocate an include data block and store the key and the given entry
*/
	cur_ptr = (INCLUDE_LIST_PTR) XtMalloc(sizeof(INCLUDE_LIST));
	cur_ptr->next = NULL;
	cur_ptr->title = NULL;
	cur_ptr->key = NULL;
	cur_ptr->last = NULL;
	cur_ptr->parent_ptr = entry_ptr;
	cur_ptr->notitle = FALSE;
	upcase_str(&line[offset - str_len], str_len, &cur_ptr->key_ptr);
	
/*	
** check if it is the first time
*/
	if (head_ptr == NULL)
	    head_ptr = cur_ptr;
	else    
	    prev_ptr->next = cur_ptr;
/* prepare for next round */
	prev_ptr = cur_ptr;
	str_len = 0;
/*
** purge blanks
*/
	if ((line[offset] != '\0') && (line[offset] != '\n'))
	    {
	    offset++;
	    while ((line[offset] == ' ') || (line[offset] == ','))
		offset++;
	    }
/*
** check if done
*/
	if ((line[offset] == '\0') || (line[offset] == '\n'))
	    parsing = FALSE;
	}
/*
** add the current list at the end of the module wide list
*/
    if (*incl_ptr_ptr == NULL)
	{
/*
** The list is empty.  Set the head of the list to be the head of the list
** which was just created, and point the "last" entry to that head.  This
** allows the common code below to use the ->last->next field properly.
*/
	*incl_ptr_ptr = head_ptr;
	head_ptr->last = head_ptr;
	}
    else
	{
	in_ptr = *incl_ptr_ptr;
	in_ptr->last->next = head_ptr;
	in_ptr->last = cur_ptr;
	}
	    
    return (SUCCESS);
    }

int parse_extension(entry_ptr, line_ptr, line_len, incl_ptr_ptr)
INDEX_TYPE_PTR entry_ptr;
char **line_ptr;
int line_len;
INCLUDE_LIST_PTR *incl_ptr_ptr;
/*
**++
**  Functional Description:
**	this routine handles the Help Library extension. It will directly
**	store the title if the 'title extension is encountered. Otherwise
**	it will dispatch to the appropriate routines.
**
**  Keywords:
**	dispatch
**
**  Arguments:
**	entry_ptr    : index entry in which the extension is found
**	line_ptr     : the line to be parsed
**	line_len     : the line length
**	incl_ptr_ptr : head of include list
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {
    int i,
        status = SUCCESS,
	NOT_SUPPORT = FALSE;
    char *line, *up_line;
    KEY_CHUNK_PTR keyword_list_ptr, scan_ptr;

    line = *line_ptr;
    upcase_str(line, line_len, &up_line);
    switch (line[1])
	{
	    
	case 't':    /* title extension */
	case 'T':

	    if (strcmp(up_line, "=TITLE") < 0)
		status = NOT_SUPPORT;
	    else
		{
		parse_title(entry_ptr, line, line_len);
		}
	    break;

	case 'k':    /* keyword extension */
	case 'K':
	    if (strcmp(up_line, "=KEYWORD") < 0)
		status = NOT_SUPPORT;
	    else
		{
/*		get to the keyword */
		for(i=1; (line[i]!=' ' && i < line_len); i++);
		if (i+1 < line_len)
		    {
/*		    parse the list of keywords */
		    parse_key(&line[i+1], &keyword_list_ptr);
/*		    check if some are there already */
		    if (entry_ptr->keyword_ptr != NULL)
			{
/*			go to the end and insert */
			scan_ptr = entry_ptr->keyword_ptr;
			while (scan_ptr->next != NULL)
			    scan_ptr = scan_ptr->next;
			scan_ptr->next = keyword_list_ptr;
			}
		    else
			entry_ptr->keyword_ptr = keyword_list_ptr;
		    }
		}
	    break;
	    
	case 'i':    /* include extension */
	case 'I':
	    if (strcmp(up_line, "=INCLUDE") < 0)
		status = NOT_SUPPORT;
	    else
		{
/*		get to the key */
		for(i=1; (line[i]!=' ' && i < line_len); i++);
		if (i+1 < line_len)
		    parse_include(incl_ptr_ptr, line, i+1, entry_ptr);
		}
	    break;

	case 'n':   /* noseach extension */
	case 'N':
	    if (strcmp(up_line, "=NOSEARCH") < 0)
		status = NOT_SUPPORT;
	    else
/*		Mark entry for nosearch */	    
		entry_ptr->nosearch = TRUE;
	    break;
	    
	default:
	    status = NOT_SUPPORT;
	    break;
	}
    free_upcase_str(up_line);

    return (status);
    }
    

int parse_title(entry_ptr, line, line_len)
INDEX_TYPE_PTR entry_ptr;
char *line;
int line_len;
/*
**++
**  Functional Description:
**	parses a title extension. If a previous title was defined it will be
**	superseeded with the one.
**
**  Keywords:
**	parse
**
**  Arguments:
**	entry_ptr    : index entry in which the extension is found
**	line	     : the line to be parsed
**	line_len     : the line length
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {
    int i;
    int count = 0;
        
    /*
    **	if there is a title already, free it. It will be superseeded
    */
    if (entry_ptr->title.dsc_a_pointer != NULL)
        {
	XtFree(entry_ptr->title.dsc_a_pointer);
        entry_ptr->title.dsc_a_pointer = NULL;
        }
    /*	get to the title */
    for(i=1; (line[i]!=' ' && i < line_len); i++);
    if (i+1 < line_len)
	{
	/*
	**  collect the title and store it
	*/
	for(i=i+1; ((line[i] != '\0') && (line[i] != '\n')) ; i++)
	count++;
	entry_ptr->title.dsc_a_pointer = (char *) XtMalloc(count + 1);
	DXmBCopy(&line[i - count], entry_ptr->title.dsc_a_pointer, count);
	entry_ptr->title.dsc_a_pointer[count] = '\0';
	entry_ptr->title.dsc_w_length = count;
	entry_ptr->title.dsc_b_dtype = DSC_K_DTYPE_T;
	entry_ptr->title.dsc_b_class = DSC_K_CLASS_S;
	}

    return(SUCCESS);
    }
    

static int create_include_error(parent_ptr, incl_ptr)
INDEX_TYPE_PTR parent_ptr;
INCLUDE_LIST_PTR incl_ptr;
/*
**++
**  Functional Description:
**	this routine create an include error message.
**
**  Keywords:
**	None
**
**  Arguments:
**	parent_ptr : entry on which the include entry depends
**	incl_ptr   : a pointer to an entry in an include list
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {
    INCLUDE_TYPE_PTR incl_entry_ptr,
                     scan_ptr;
/*
** allocate space for the include entry
*/
    incl_entry_ptr = (INCLUDE_TYPE_PTR) XtMalloc(sizeof(INCLUDE_TYPE));
    incl_entry_ptr->next = NULL;
/*
** copy the full key of the parent
*/
    incl_entry_ptr->full_key = (char *) XtMalloc(parent_ptr->full_key.dsc_w_length + 1);
    strcpy(incl_entry_ptr->full_key, parent_ptr->full_key.dsc_a_pointer);
/*
** the title is the faulty key + an error message
*/
    incl_entry_ptr->notitle = FALSE;
    incl_entry_ptr->title = (char *) XtMalloc(strlen(incl_ptr->key_ptr)
	+ KeyNotFoundMessageSize + 1);
    strcpy(incl_entry_ptr->title, KeyNotFoundMessage);
    strcat(incl_entry_ptr->title, incl_ptr->key_ptr);
/*
** insert it in the list
*/
    if (parent_ptr->include_ptr == NULL)
	parent_ptr->include_ptr = incl_entry_ptr;
    else
	{
	scan_ptr = parent_ptr->include_ptr;
	while (scan_ptr->next != NULL)
	    scan_ptr = scan_ptr->next;
	scan_ptr->next = incl_entry_ptr;
	}
    
    return(SUCCESS);
    }
static int create_include_entry(parent_ptr, target_ptr)
INDEX_TYPE_PTR parent_ptr, target_ptr;
/*
**++
**  Functional Description:
**	this routine create an include entry in parallel with the sub-topics.
**
**  Keywords:
**	None
**
**  Arguments:
**	parent_ptr : entry on which the include entry depends
**	target_ptr : entry pointed by the include
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {
    INCLUDE_TYPE_PTR incl_entry_ptr,
                     scan_ptr;
/*
** allocate space for the include entry
*/
    incl_entry_ptr = (INCLUDE_TYPE_PTR) XtMalloc(sizeof(INCLUDE_TYPE));
    incl_entry_ptr->next = NULL;
    incl_entry_ptr->notitle = FALSE;
/*
** copy the full key and the title
*/
    incl_entry_ptr->full_key = (char *) XtMalloc(target_ptr->full_key.dsc_w_length + 1);
    strcpy(incl_entry_ptr->full_key, target_ptr->full_key.dsc_a_pointer);

    if (target_ptr->title.dsc_a_pointer == NULL)
	{
	incl_entry_ptr->title = (char *) XtMalloc(target_ptr->full_key.dsc_w_length + 1);
	strcpy(incl_entry_ptr->title, target_ptr->full_key.dsc_a_pointer);
	incl_entry_ptr->notitle = TRUE;
	}
    else
	{
	incl_entry_ptr->title = (char *) XtMalloc(target_ptr->title.dsc_w_length + 1);
	strcpy(incl_entry_ptr->title, target_ptr->title.dsc_a_pointer);
	}
/*
** insert it in the list
*/
    if (parent_ptr->include_ptr == NULL)
	parent_ptr->include_ptr = incl_entry_ptr;
    else
	{
	scan_ptr = parent_ptr->include_ptr;
	while (scan_ptr->next != NULL)
	    scan_ptr = scan_ptr->next;
	scan_ptr->next = incl_entry_ptr;
	}
    
    return(SUCCESS);
    }

int free_include_list(incl_ptr)
INCLUDE_LIST_PTR incl_ptr;
/*
**++
**  Functional Description:
**	frees an include list
**
**  Keywords:
**	free
**
**  Arguments:
**	incl_ptr : a pointer to the include list
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {
    INCLUDE_LIST_PTR to_free_ptr;

    while (incl_ptr != NULL)
	{
	
	if (incl_ptr->key_ptr != NULL)
	    XtFree(incl_ptr->key_ptr);

	/*
	**  Check if the title is not pointing to the key
	*/
	
	if (!(incl_ptr->notitle))
	    if (incl_ptr->title != NULL)
		XtFree(incl_ptr->title);
		
	if (incl_ptr->key != NULL)
	    free_key(incl_ptr->key);
	    
	to_free_ptr = incl_ptr;
	incl_ptr = incl_ptr->next;
	
	XtFree((char *)to_free_ptr);
	}
	
    return (SUCCESS);
    }

int fix_up_include(incl_ptr, cntxt_ptr)
INCLUDE_LIST_PTR incl_ptr;
LIBRARY_CONTEXT_BLOCK_PTR cntxt_ptr;
/*
**++
**  Functional Description:
**	this routine will scan an include list and create an include
**	entry if the key referenced exists
**
**  Keywords:
**	None
**
**  Arguments:
**	incl_ptr  : pointer to an include list
**	cntxt_ptr : the library context block
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {
    int status,
        locate_status;
    KEY_CHUNK_PTR key_list_ptr;
    INDEX_TYPE_PTR target_ptr;

    while (incl_ptr != NULL)
	{
/*
** parse and locate the key referenced by the include
*/
	parse_key(incl_ptr->key_ptr, &key_list_ptr);
	locate_status = locate_key(key_list_ptr, cntxt_ptr->cache_ptr,
	                           &target_ptr);
/*
** read the module if it's not read in yet
*/
	if (locate_status == NOT_IN_MEM)
	    {
	    ERROR_CHECK(DXmHelp_set_read_context(cntxt_ptr, target_ptr,
	                                         key_list_ptr));
	    locate_status = locate_key(key_list_ptr, target_ptr, &target_ptr);
	    }
	free_key(key_list_ptr);

	if (locate_status == SUCCESS)
	    create_include_entry(incl_ptr->parent_ptr, target_ptr);
	else
	    create_include_error(incl_ptr->parent_ptr, incl_ptr);

	incl_ptr = incl_ptr->next;
	}

    free_include_list(incl_ptr);
    
    return (SUCCESS);
    }

int read_in_library(lib_cntxt_ptr)
LIBRARY_CONTEXT_BLOCK_PTR lib_cntxt_ptr;
/*
**++
**  Functional Description:
**	loads a library in cache
**
**  Keywords:
**	read
**
**  Arguments:
**	lib_cntxt_ptr : a pointer to a library context block
**   
**  Non-local references:
**	none
**   
**  Non-local changes:
**	none
**   
**  Side effects:
**	the whole cache may be reloaded
**
**  Result:
**	SUCCESS
**	any status code returned by the LBR routines
**
**  Exceptions:
**	none
**--
*/
    {
    int status = SUCCESS;
    INDEX_TYPE_PTR top_entry_ptr, entry_ptr;
    KEY_CHUNK_PTR key_list_ptr;
/*
**  assume cache is up to date
*/
    lib_cntxt_ptr->new_cache = FALSE;
    top_entry_ptr = lib_cntxt_ptr->cache_ptr;
/*
**  check if every top level index is in memory
*/
    while (top_entry_ptr != NULL)
	{
	if (!top_entry_ptr->in_memory)
	    {
/*
**          make sure library is open
*/
	    if (!lib_cntxt_ptr->lib_open)
		{
		ERROR_CHECK(DXmHelp_open_lib(lib_cntxt_ptr));
		lib_cntxt_ptr->open_cnt++;
		}
/*
**	    read module in
*/
	    parse_key(top_entry_ptr->keyname.dsc_a_pointer, &key_list_ptr);
    	    ERROR_CHECK(DXmHelp_set_read_context(lib_cntxt_ptr, top_entry_ptr,
	                                     key_list_ptr));
	    free_key(key_list_ptr);
	    }
	top_entry_ptr = top_entry_ptr->next;
	}
/*
**  if cache has been invalided while reading, make sure everything is there
*/
    if (lib_cntxt_ptr->new_cache)
	ERROR_CHECK(read_in_library(lib_cntxt_ptr));
	
    return (status);
    }

static int find_substring(candidate_str, pattern_str)
char *candidate_str,
     *pattern_str;
/*
**++
**  Functional Description:
**	finds substring in source string
**
**  Keywords:
**	find
**
**  Arguments:
**	candidate_str	: address of the source string
**	pattern_str	: address of the pattern string
**
**  Result:
**	1 : the substring was found in the source string
**	0 : the substring is not in the source string
**
**  Exceptions:
**	none
**--
*/
    {
    int pattern_len,
	candidate_len;
/*
**  The NULL string matches everything
*/
    if (pattern_str == NULL)
	return(SUBSTR_FOUND);
    else
	{
	pattern_len = strlen(pattern_str);
	for(candidate_len = strlen(candidate_str);
	     (candidate_len >= pattern_len);
	       candidate_len--)
	    {
	    candidate_str = strchr(candidate_str, pattern_str[0]);
	    if (candidate_str == NULL)
		return(SUBSTR_NOT_FOUND);
	    else
		if (strncmp(candidate_str, pattern_str, pattern_len) == 0)
		    return(SUBSTR_FOUND);
		else
		    candidate_str++;
	    }
	}
	
    return (SUBSTR_NOT_FOUND);
    }

static int search_title(entry_ptr, head_match_list_ptr, title_pattern_ptr)
INDEX_TYPE_PTR entry_ptr;
ENTRY_LIST_TYPE_PTR *head_match_list_ptr;
char *title_pattern_ptr;
/*
**++
**  Functional Description:
**	searches for a frame title in the library
**
**  Keywords:
**	search
**
**  Arguments:
**	entry_ptr	: a pointer to an entry in the cache
**	head_match_list_ptr : a pointer to a list of matched entries
**	title_pattern_ptr : a pointer to the pattern title to search for
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	none
**--
*/
    {
    char		*candidate_title;
    ENTRY_LIST_TYPE_PTR tmp_ptr;
/*
**  search all child first
*/
    if (entry_ptr->sub_index != NULL)
	search_title(entry_ptr->sub_index, head_match_list_ptr,
	             title_pattern_ptr);
/*
**  search all sibling first
*/
    if (entry_ptr->next != NULL)
	search_title(entry_ptr->next, head_match_list_ptr, title_pattern_ptr);
/*
**  Don't do anything if frame is set no-search
*/
    if (entry_ptr->nosearch) return (SUCCESS);
/*
**  upcase title pattern for comparison
*/
    if (entry_ptr->title.dsc_a_pointer == NULL)
	upcase_str(entry_ptr->keyname.dsc_a_pointer,
	           entry_ptr->keyname.dsc_w_length,  &candidate_title);
    else
	upcase_str(entry_ptr->title.dsc_a_pointer,
	           entry_ptr->title.dsc_w_length,  &candidate_title);
/*
**  look for match
*/
    if (find_substring(candidate_title, title_pattern_ptr) == 1)
	{
/*
**	insert in the list entry which has matching title 
*/
	tmp_ptr = (ENTRY_LIST_TYPE_PTR) XtMalloc(sizeof(ENTRY_LIST_TYPE));
	tmp_ptr->matched_entry = entry_ptr;
	if (*head_match_list_ptr == NULL)
	    {
	    tmp_ptr->next = NULL;
	    tmp_ptr->cnt = 1;
	    }
	else
	    {
	    tmp_ptr->next = *head_match_list_ptr;
	    tmp_ptr->cnt = (*head_match_list_ptr)->cnt + 1;
	    }
	*head_match_list_ptr = tmp_ptr;
	}

    free_upcase_str(candidate_title);
    
    return (SUCCESS);
    }

static int search_keyword(entry_ptr, head_match_list_ptr, kwd_pattern_ptr)
INDEX_TYPE_PTR entry_ptr;
ENTRY_LIST_TYPE_PTR *head_match_list_ptr;
char *kwd_pattern_ptr;
/*
**++
**  Functional Description:
**	searches a given keyword in the frame library
**
**  Keywords:
**	search
**
**  Arguments:
**	entry_ptr	: a pointer to an entry in the cache
**	head_match_list_ptr : a pointer to a list of matched entries
**	kwd_pattern_ptr : a pointer to the pattern keyword to search for
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	none
**--
*/
    {
    int			found_match = FALSE;
    KEY_CHUNK_PTR	cur_keyword;
    ENTRY_LIST_TYPE_PTR tmp_ptr;
/*
**  search all child first
*/
    if (entry_ptr->sub_index != NULL)
	search_keyword(entry_ptr->sub_index, head_match_list_ptr,
	             kwd_pattern_ptr);
/*
**  search all sibling first
*/
    if (entry_ptr->next != NULL)
	search_keyword(entry_ptr->next, head_match_list_ptr, kwd_pattern_ptr);
/*
**  Don't do anything if frame is set no-search
*/
    if (entry_ptr->nosearch) return (SUCCESS);
/*
**  process the list of keywords
*/
    cur_keyword = entry_ptr->keyword_ptr;
    while ((cur_keyword != NULL) && (!found_match))
	{
/*
**	look for match
*/
	if (case_less_str_equal(cur_keyword->key.dsc_a_pointer,
	    kwd_pattern_ptr))
	    {
	    found_match = TRUE;
/*
**	    insert entry which has matching keyword in the list
*/
	    tmp_ptr = (ENTRY_LIST_TYPE_PTR) XtMalloc(sizeof(ENTRY_LIST_TYPE));
	    tmp_ptr->matched_entry = entry_ptr;
	    if (*head_match_list_ptr == NULL)
		{
		tmp_ptr->next = NULL;
		tmp_ptr->cnt = 1;
		}
	    else
		{
		tmp_ptr->next = *head_match_list_ptr;
		tmp_ptr->cnt = (*head_match_list_ptr)->cnt + 1;
		}
	    *head_match_list_ptr = tmp_ptr;
	    }
	cur_keyword = cur_keyword->next;
	}
    
    return (SUCCESS);
    }

static int build_array(match_list_ptr, topic_arr_ptr_ptr, title_arr_ptr_ptr,
		       arr_siz_ptr, in_cache)
ENTRY_LIST_TYPE_PTR match_list_ptr;
char ***topic_arr_ptr_ptr, ***title_arr_ptr_ptr;
int *arr_siz_ptr;
Boolean in_cache;
    
/*
**++
**  Functional Description:
**	builds a topic and a title array from a matched list
**
**  Keywords:
**	build
**
**  Arguments:
**	match_list_ptr	    : a pointer to a list of matched entries
**	topic_arr_ptr_ptr   : a pointer to a pointer to an array of topic pointers
**	title_arr_ptr_ptr   : a pointer to a pointer to an array of title pointers
**	arr_siz_ptr	    : a pointer to the array size
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	none
**--
*/
    {
    int array_size,
        i;
    ENTRY_LIST_TYPE_PTR match_ptr;
#ifdef GENERAL_TEXT
    long count,stat;
#endif /* GENERAL_TEXT */
    if (match_list_ptr != NULL)
	{
/*
**	get size of the array to allocate
*/
	*arr_siz_ptr = match_list_ptr->cnt;
/*    
**	Allocate arrays
*/
	*topic_arr_ptr_ptr = (char **) XtMalloc(sizeof(char *) * (*arr_siz_ptr));
	*title_arr_ptr_ptr = (char **) XtMalloc(sizeof(char *) * (*arr_siz_ptr));
/*
**	scan the list of matched entries and load arrays
*/
	match_ptr = match_list_ptr;
	for (i = 0; i < (*arr_siz_ptr); i++)
	    {
	    (*topic_arr_ptr_ptr)[i] =
#ifdef GENERAL_TEXT
		(char *)DXmCvtFCtoCS
		  (match_ptr->matched_entry->full_key.dsc_a_pointer,
		    &count,&stat);
#else
		(char *)XmStringLtoRCreate
		  (match_ptr->matched_entry->full_key.dsc_a_pointer,
		   "ISO8859-1");
#endif /* GENERAL_TEXT */
		    
	    if (match_ptr->matched_entry->title.dsc_a_pointer == NULL) {
		(*title_arr_ptr_ptr)[i] =
#ifdef GENERAL_TEXT
		    (char *)DXmCvtFCtoCS
		     (match_ptr->matched_entry->keyname.dsc_a_pointer,
		         &count,&stat);
#else
		    (char *)XmStringLtoRCreate
		     (match_ptr->matched_entry->keyname.dsc_a_pointer,
		        "ISO8859-1");
#endif /* GENERAL_TEXT */
	    }
	    else {
		(*title_arr_ptr_ptr)[i] =
#ifdef GENERAL_TEXT
		    (char *)DXmCvtFCtoCS
		     (match_ptr->matched_entry->title.dsc_a_pointer,
		    	 &count,&stat);
#else
		    (char *)XmStringLtoRCreate
		     (match_ptr->matched_entry->title.dsc_a_pointer,
		        "ISO8859-1");
#endif /* GENERAL_TEXT */
	    }
    
	    match_ptr = match_ptr->next;
	    }
	}
    else
	*arr_siz_ptr = 0;
	
    return (SUCCESS);
    }


int DXmHelp__search_title(lib_cntxt_ptr, title_ptr, topic_arr_ptr_ptr,
			  title_arr_ptr_ptr, arr_size_ptr)
LIBRARY_CONTEXT_BLOCK_PTR lib_cntxt_ptr;
char *title_ptr, ***topic_arr_ptr_ptr, ***title_arr_ptr_ptr;
int *arr_size_ptr;
/*
**++
**  Functional Description:
**	searches for a title in the Help Library
**
**  Keywords:
**	search
**
**  Arguments:
**	lib_cntxt_ptr	    : a pointer to a library context block
**	title_ptr	    : a pointer to a title string
**	topic_arr_ptr_ptr   : a pointer to a pointer to topic pointers
**	title_arr_ptr_ptr   : a pointer to a pointer to title pointers
**	arr_size_ptr	    : a pointer to the array size
**
**  Result:
**	SUCCESS
**	any status code retuned by the LBR routines
**
**  Exceptions:
**	none
**--
*/
    {
    int status = SUCCESS;
    ENTRY_LIST_TYPE_PTR head_match_list_ptr = NULL;
    char *up_title;
    Boolean in_cache = FALSE;

    lib_cntxt_ptr->search_mode = 1;
/*
**  make sure all the library is in cache
*/
    ERROR_CHECK(read_in_library(lib_cntxt_ptr));
/*
**  title pattern must be upcased for comparison ! temporary?
*/
    upcase_str(title_ptr, strlen(title_ptr), &up_title);
/*
**  look for matches
*/
    search_title(lib_cntxt_ptr->cache_ptr, &head_match_list_ptr, up_title);
/*
**  build arrays to be returned
*/
    if (lib_cntxt_ptr->caching == 1)
	in_cache = TRUE;
	
    build_array(head_match_list_ptr, topic_arr_ptr_ptr, title_arr_ptr_ptr,
	        arr_size_ptr, in_cache);

    free_upcase_str(up_title);

    lib_cntxt_ptr->search_mode = 0;
    
    /*
    **  If the cache was turned on while searching leave it on otherwise turn it
    **	off
    */
    
    if (lib_cntxt_ptr->caching == 0)
	DXmHelp__set_cache_mode(lib_cntxt_ptr, FALSE);
    
    return (status);
    }
    

int DXmHelp__search_keyword(lib_cntxt_ptr, keyword_ptr, topic_arr_ptr_ptr,
			  title_arr_ptr_ptr, arr_size_ptr)
LIBRARY_CONTEXT_BLOCK_PTR lib_cntxt_ptr;
char *keyword_ptr, ***topic_arr_ptr_ptr, ***title_arr_ptr_ptr;
int *arr_size_ptr;
/*
**++
**  Functional Description:
**	searches for a keyword in the Help Library
**
**  Keywords:
**	search
**
**  Arguments:
**	lib_cntxt_ptr	    : a pointer to a library context block
**	keyword_ptr	    : a pointer to a keyword string
**	topic_arr_ptr_ptr   : a pointer to a pointer to topic pointers
**	title_arr_ptr_ptr   : a pointer to a pointer to title pointers
**	arr_size_ptr	    : a pointer to the array size
**
**  Result:
**	SUCCESS
**	any status code retuned by the LBR routines
**
**  Exceptions:
**	none
**--
*/
    {
    int status = SUCCESS;
    ENTRY_LIST_TYPE_PTR head_match_list_ptr = NULL;
    char *up_keyword;
    Boolean in_cache = FALSE;

    lib_cntxt_ptr->search_mode = 1;
/*
**  make sure all the library is in cache
*/
    ERROR_CHECK(read_in_library(lib_cntxt_ptr));
/*
**  keyword pattern must be upcased for comparison ! temporary?
*/
    upcase_str(keyword_ptr, strlen(keyword_ptr), &up_keyword);
/*
**  look for matches
*/
    search_keyword(lib_cntxt_ptr->cache_ptr, &head_match_list_ptr, up_keyword);
/*
**  build arrays to be returned
*/
    if (lib_cntxt_ptr->caching == 1)
	in_cache = TRUE;
	
    build_array(head_match_list_ptr, topic_arr_ptr_ptr, title_arr_ptr_ptr,
	        arr_size_ptr, in_cache);

    free_upcase_str(up_keyword);
    
    lib_cntxt_ptr->search_mode = 0;
    
    /*
    **  If the cache was turned on while searching leave it on otherwise turn it
    **	off
    */
    
    if (lib_cntxt_ptr->caching == 0)
	DXmHelp__set_cache_mode(lib_cntxt_ptr, FALSE);
    
    return (status);
    }

static int insert_keyword(head_list, list_ptr, cur_ptr, keyword_ptr)
KEYWORD_LIST_TYPE **head_list,
		  *list_ptr,
		  *cur_ptr;
KEY_CHUNK	  *keyword_ptr;
/*
**++
**  Functional Description:
**	stores a keyword in a list
**
**  Keywords:
**	store
**
**  Arguments:
**	head_list : address of the pointer head of the list
**	list_ptr  : pointer to an item in the list
**	cur_ptr	  : pointer to an item in the list
**	keyword_ptr : pointer to the keyword 
**	
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	none
**--
*/
    {
    KEYWORD_LIST_TYPE *tmp_ptr, *sav_next;
/*
**  Allocate a new element
*/
    tmp_ptr = (KEYWORD_LIST_TYPE *) XtMalloc(sizeof(KEYWORD_LIST_TYPE));
    tmp_ptr->keyw_ptr = keyword_ptr;
/*
**  First element in the list
*/
    if (*head_list == NULL)
	{
	tmp_ptr->next = NULL;
	*head_list = tmp_ptr;
	}
    else
/*
**	Case for only one element in the list
*/
	if ((*head_list == list_ptr) && (*head_list == cur_ptr))
	    {
	    tmp_ptr->next = (*head_list);
	    (*head_list) = tmp_ptr;
	    }
	else
/*
**	    All the other cases
*/
	    {
	    sav_next = list_ptr->next;
	    list_ptr->next = tmp_ptr;
	    tmp_ptr->next = sav_next;
	    }

    return (SUCCESS);
    }

static int collect_keyword(entry_ptr, keyw_list, keyw_cnt)
INDEX_TYPE *entry_ptr;
KEYWORD_LIST_TYPE **keyw_list;
int *keyw_cnt;
/*
**++
**  Functional Description:
**	collects keywords for a given help frame
**
**  Keywords:
**	collect
**
**  Arguments:
**	entry_ptr   : address of an help entry in cache
**	keyw_list   : address of the head of a keyword list
**	keyw_cnt    : address of the number of keywords
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	none
**--
*/
    {
    int  i,
         cmp_stat,
         duplicate;
    char *up_keyw_list, *up_keyw_in;
    KEYWORD_LIST_TYPE *cur_list, *prev_list;
    KEY_CHUNK_PTR cur_keyw_ptr;
/*
**  Visit child first
*/
    if (entry_ptr->sub_index != NULL)
	collect_keyword(entry_ptr->sub_index, keyw_list, keyw_cnt);
/*
**  Visit sibling first
*/
    if (entry_ptr->next != NULL)
	collect_keyword(entry_ptr->next, keyw_list, keyw_cnt);
/*
**  Don't do anything if frame is set no-search
*/
    if (entry_ptr->nosearch) return (SUCCESS);

    cur_keyw_ptr = entry_ptr->keyword_ptr;
/*
**  Visit all the keywords for this entry
*/
    while (cur_keyw_ptr != NULL)
	{
	cur_list = prev_list = *keyw_list;
	duplicate = FALSE;
/*
**	Run down the list of collected keywords
*/
	while (( cur_list != NULL) && (!duplicate))
	    {
	    upcase_str(cur_list->keyw_ptr->key.dsc_a_pointer,
	               cur_list->keyw_ptr->key.dsc_w_length,
		       &up_keyw_list);
	    upcase_str(cur_keyw_ptr->key.dsc_a_pointer,
	               cur_keyw_ptr->key.dsc_w_length,
		       &up_keyw_in);
/*
**	    Compare
*/
	    cmp_stat = strcmp(up_keyw_list, up_keyw_in);
/*
**	    If keyword to insert is bigger, get the next one
*/
	    if (cmp_stat < 0)
		{
		prev_list = cur_list;
		cur_list = cur_list->next;
		}
	    else
/*
**		If result is smaller, insert new keyword
*/
		if (cmp_stat > 0)
		    {
		    insert_keyword(keyw_list, prev_list, cur_list, cur_keyw_ptr);
		    duplicate = TRUE;
		    (*keyw_cnt)++;
		    }
		else
		    duplicate = TRUE;
	    free_upcase_str(up_keyw_list);
	    free_upcase_str(up_keyw_in);
	    }
/*
**	Insert the new keyword at the end of the list
*/
	if ((cur_list == NULL) && (!duplicate))
	    {
	    insert_keyword(keyw_list, prev_list, cur_list, cur_keyw_ptr);
	    (*keyw_cnt)++;
	    }
	cur_keyw_ptr = cur_keyw_ptr->next;
	}
	
    return (SUCCESS);
    }

static int build_keyword_array(keyw_list, keyw_cnt, keyw_array, in_cache)
KEYWORD_LIST_TYPE *keyw_list;
int keyw_cnt;
char ***keyw_array;
Boolean in_cache;
    
/*
**++
**  Functional Description:
**	builds the array of keywords to be returned
**
**  Keywords:
**	build
**
**  Arguments:
**	keyw_list   : address of the list of keywords
**	keyw_cnt    : the count of keywords
**	keyw_array  : address of a pointer to the keyword array
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
    KEYWORD_LIST_TYPE *list_ptr,
                      *to_free_ptr;
#ifdef GENERAL_TEXT
    long count,stat;
#endif /* GENERAL_TEXT */
/*
**  Allocate the array of keyword pointers
*/
    *keyw_array = (char **) XtMalloc(sizeof(char *) * keyw_cnt);
    list_ptr = keyw_list;
/*
**  Scan the list of collected keywords and load the array
*/
    for(i = 0; i < keyw_cnt; i++) {
	(*keyw_array)[i] =
#ifdef GENERAL_TEXT
	    (char *)DXmCvtFCtoCS(list_ptr->keyw_ptr->key.dsc_a_pointer,
		    &count,&stat);
#else
	    (char *)XmStringLtoRCreate(list_ptr->keyw_ptr->key.dsc_a_pointer,
		   "ISO8859-1");
#endif /* GENERAL_TEXT */

	to_free_ptr = list_ptr;
	list_ptr = list_ptr->next;
/*
**	Free the keyword list element
*/
	XtFree((char *)to_free_ptr);
    }
	
    return (SUCCESS);
    }

int DXmHelp__get_keyword(lib_cntxt, keyw_array, keyw_cnt)
LIBRARY_CONTEXT_BLOCK *lib_cntxt;
char ***keyw_array;
int *keyw_cnt;
/*
**++
**  Functional Description:
**	collects all the keywords in the library
**
**  Keywords:
**	collect
**
**  Arguments:
**	lib_cntxt   : address of a library context block
**	keyw_array  : address of a pointer to the array of keywords
**	keyw_cnt    : address of the number of keywords in the array
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	none
**--
*/
    {
    int status;
    KEYWORD_LIST_TYPE *keyw_list = NULL;
    Boolean in_cache = FALSE;

    lib_cntxt->search_mode = 1;
    
    ERROR_CHECK(read_in_library(lib_cntxt));
    *keyw_cnt = 0;
    collect_keyword(lib_cntxt->cache_ptr, &keyw_list, keyw_cnt);

    if (lib_cntxt->caching == 1)
	in_cache = TRUE;
	
    build_keyword_array(keyw_list, *keyw_cnt, keyw_array, in_cache);
    
    lib_cntxt->search_mode = 0;
    
    /*
    **  If the cache was turned on while searching leave it on otherwise turn it
    **	off
    */
    
    if (lib_cntxt->caching == 0)
	DXmHelp__set_cache_mode(lib_cntxt, FALSE);
    
    return (SUCCESS);
    }
