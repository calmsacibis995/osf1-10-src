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
*/
/*
**++
**  Subsystem:
**	DXmHelp
**
**  Version: V1.0
**
**  Abstract:
**	this module contains all the routines needed for reading in a Help
**	module from a Help Library, building Help Frames in memory and
**	retrieving a Help Frame with a given key.
**
**  Keywords:
**	read, retrieve
**
**  Environment:
**	User mode, executable image
**
**  Author:
**	Andre Pavanello, CASEE Group, MEMEX Project
**
**  Creation Date: 22-Dec-87
**
**  Modification History:
**
**		Will                                        02-Aug-91
**	    Set qualifier = 0 in find_include.  qualifier was never
**	    initialized and was causing serious problems on the Sun.
**
**		Rich                                        20-Aug-19
**	    Merge in DECIsreal changes.
**
**		Rich					    23-Jul-90
**	    Remove DXm/ from private include file includes
**
**		Rich					    07-Jun-90
**	    Now need to include DXmHelpBP.h
**
**		Leo					    02-Aug-89
**	    Q&D port to Motif
**
**  V2-bl4-2	Andre Pavanello				    14-Jun-89
**	    Use the _toupper C macro on VMS because it's faster
**	    Changes proposed by Ken Moreau, Debug Group
**
**  V2-bl4-1	Andre Pavanello				    13-May-89
**	    add new routines for non-caching support
**	    
**  B-2-0	Andre Pavanello				    22-Mar-89
**	    Continue parsing if digit followed by a dot
**	    
**  BL11.1	Andre Pavanello				    11-Oct-88
**	    use XtMalloc and XtFree only
**	    
**  BL8.6	Andre Pavanello				    27-Jul-88
**	    Recompute start position in work buffer after realloc's
**
**  X004-0-2	Andre Pavanello				    14-Apr-88
**	    Fix bug in collecting Command Qualifiers
**
**  X004-0-1	Andre Pavanello				    8-Apr-88
**	    Return status when the first character is removed from help text
**
**  X003-1-3	Andre Pavanello				    30-Mar-88
**	    Ignore trailing blanks when parsing keys
**
**  X003-1-2	Andre Pavanello				    29-Mar-88
**	    Fix line length returned in get_record when first character
**	    is skipped
**
**  X003-1-1	Andre Pavanello				    25-Mar-88
**	    print unsupported command extension as part of the help frame
**
**  X003-0-10	Andre Pavanello				    17-Mar-88
**	    Change DXmHelp$$free_frame to DXmHelp$$free_array
**
**  X003-0-9	Andre Pavanello				    15-Mar-88
**	    Remove leading character of Help Text if blank
**
**  X003-0-8    Andre Pavanello				    7-Mar-88
**	    Fix up include file name for Ultrix
**
**  X003-0-7	Andre Pavanello				    26-Feb-88
**	    remove prototype declarations for pcc
**	    Add routine for setting the read context for the module
**
**  X003-0-6	Andre Pavanello				    25-Feb-88
**	    remove RMS include file.
**
**  X003-0-5	Andre Pavanello				    25-Feb-88
**	    modify include files. Include only the internal file
**
**  X003-0-4	Andre Pavanello				    23-Feb-88
**	    Replace STR$ routines with the C equivalent routines
**
**  X003-0-3	Andre Pavanello				    18-Feb-88
**	    Handle error status returned by LBR$FIND: reload the cache
**	    if it fails.
**
**  X003-0-2	Andre Pavanello				    10-Feb-88
**	    Add check for open library before reading the module.
**	    Invoke LBR$LOOKUP_KEY if LBR$FIND fails.
**		
**  X003-0-1	Andre Pavanello				    9-Feb-88
**	    use the new work structure for reading the module.
**	    use the new library context structure.
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
static int store_key();
static int get_record();
static int build_index();
static int process_help_text();
static int set_module_cntxt();
static int read_module();
static int get_text();
static int get_sub_topics();
static int build_frame();

static int load_toplevel_titles();
static int get_frame_nocache();
static int find_frame();
static int scan_module();
static int collect_frame();
static int scan_for_title();
static int get_full_key();
static char *get_title();
static int skip_sublevel();
static int resolve_includes();
static Boolean find_module_to_search();
static int find_include();
static int build_full_key();
static Boolean key_match();
static int format_sub_topic();
static int set_include_not_found();
static INDEX_TYPE *init_entry();
static int collect_qualifiers();
	           
/*
**  Macro Definitions
*/
#define CHUNK	       4096      /* initial size for the work buffer */
#define BUF_CHUNK      1024      /* initial size for the frame buffer */

/*
**  External Data Declarations
*/
#ifdef VMS
extern int LBR$FIND(),
           LBR$GET_RECORD(),
           LBR$SET_LOCATE(),
           LBR$LOOKUP_KEY();
#else
extern int LBR_FIND(),
           LBR_GET_RECORD(),
           LBR_SET_LOCATE(),
           LBR_LOOKUP_KEY();
#endif


Boolean case_less_str_equal(s1, s2)
char *s1;
char *s2;
/*
**++
**  Functional Description:
**	This routine compares two text string. It returns true if the two text  
**	strings are equal. Otherwise it returns false.			    
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    int i;
    char c1, c2;

    i = 0;

    while (TRUE) {
	if (s1[i] == '\0' && s2[i] == '\0')
	    break;
	    
#ifdef VMS  /* Use the _toupper macro because it is faster */

	c1 = _toupper(s1[i]);
	c2 = _toupper(s2[i]);
	
#else	    /* Use the routine because it is portable */

	c1 = toupper(s1[i]);
	c2 = toupper(s2[i]);
	
#endif

	if (c1 != c2)
	    return FALSE;

	i++;
    }

    return TRUE;
    }


int upcase_str(str_ptr, str_len, up_str_ptr_ptr)
char *str_ptr;
int str_len;
char **up_str_ptr_ptr;
/*
**++
**  Functional Description:
**	this routine upcases a string. It allocates space for the
**	new string.
**
**  Keywords:
**	upcase
**
**  Arguments:
**	str_ptr    : a pointer to the string to be upcased
**	str_len    : the length of the string
**	up_str_ptr_ptr : address of a pointer to the upcased routine
**
**  Result:
**	SUCESS
**
**  Exceptions:
**	none
**--
*/
    {
    int i;
    char *tmp_str;

    if (str_len <= 0)
	str_len = strlen(str_ptr);

    tmp_str = (char *) XtMalloc(str_len + 1);
    
    /*
    * In ANSI C, toupper only touches lower-case characters; unfortunately,
    * not everyone complies with ANSI C.
    */
    for (i = 0; i < str_len; i++)

#ifdef VMS  /* Use the _toupper macro because it is faster */

	tmp_str[i] = (islower(str_ptr[i])) ? _toupper(str_ptr[i]) : str_ptr[i];
	
#else	    /* Use the routine because it is portable */

	tmp_str[i] = (islower(str_ptr[i])) ? toupper(str_ptr[i]) : str_ptr[i];
#endif

    tmp_str[i] = '\0';
    
    *up_str_ptr_ptr = tmp_str;
    
    return (SUCCESS);
    }

int free_upcase_str(str_ptr)
char *str_ptr;
/*
**++
**  Functional Description:
**	frees the string allocated by upcase_string
**
**  Keywords:
**	free
**
**  Arguments:
**	str_ptr : a pointer to a string
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	none
**--
*/
    {

    if (str_ptr != NULL)
	XtFree(str_ptr);
	
    return (SUCCESS);
    }

int parse_key(in_str, head_ptr)
char *in_str;
KEY_CHUNK_PTR *head_ptr;
/*
**++
**  Functional Description:
**	parses a help key and decomposes it in levels. Each word is an
**	additional level.
**
**  Keywords:
**	parse
**
**  Arguments:
**	in_str   : the key to be parsed
**	head_ptr : a pointer to the begining of the list of sub-keys
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
	offset = 0,
	parsing = TRUE,
	first_time = TRUE;
    KEY_CHUNK_PTR cur_ptr,
                  prev_ptr;
/*
**  purge leading blanks
*/
    while (in_str[offset] == ' ')
	offset++;

    while (parsing)
	{
/*
**	bump counters while it's not a terminator
*/
	while ((in_str[offset] != ' ') && (in_str[offset] != '\0') &&
	       (in_str[offset] != ',') && (in_str[offset] != '\n'))
	    {
	    str_len++;
	    offset++;
	    }
/*
**	end of word. Allocate data structure and copy the sub-key into it
*/
	cur_ptr = (KEY_CHUNK_PTR) XtMalloc(sizeof(KEY_CHUNK));
	cur_ptr->next = NULL;
	cur_ptr->key.dsc_w_length = str_len;
	cur_ptr->key.dsc_b_dtype = DSC_K_DTYPE_T;
	cur_ptr->key.dsc_b_class = DSC_K_CLASS_S;
	cur_ptr->key.dsc_a_pointer = (char *) XtMalloc(str_len + 1);
	DXmBCopy(&in_str[offset - str_len], cur_ptr->key.dsc_a_pointer, str_len);
	cur_ptr->key.dsc_a_pointer[str_len] = '\0';

/*
**	if first round remember to update the head pointer, otherwise add to the end
*/	
	if (first_time)
	    {
	    *head_ptr = cur_ptr;
	    first_time = FALSE;
	    }
	else
	    prev_ptr->next = cur_ptr;
	    
	prev_ptr = cur_ptr;
	str_len = 0;
/*
**	purge blanks and commas
*/
	if ((in_str[offset] != '\0') && (in_str[offset] != '\n'))
	    {
	    while ((in_str[offset] == ' ') || (in_str[offset] == ','))
		offset++;
	    }
/*
**	check end of string
*/
	if ((in_str[offset] == '\0') || (in_str[offset] == '\n'))
	    parsing = FALSE;
	}
		
    return (SUCCESS);
    }

int free_key(key_ptr)
KEY_CHUNK_PTR key_ptr;
/*
**++
**  Functional Description:
**	frees the sub-keys data structure built by parse_key
**
**  Keywords:
**	free
**
**  Arguments:
**	key_ptr : a pointer to the head of the list
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {
    
    if (key_ptr != NULL) {
	if (key_ptr->next != NULL)
	    free_key(key_ptr->next);
	if (key_ptr->key.dsc_a_pointer != NULL)
	    XtFree(key_ptr->key.dsc_a_pointer);
	XtFree((char *)key_ptr);
    }
    
    return (SUCCESS);
    }

int locate_key(key_ptr, index_ptr, entry_ptr)
KEY_CHUNK_PTR key_ptr;
INDEX_TYPE_PTR index_ptr, *entry_ptr;
/*
**++
**  Functional Description:
**	this routine finds a sub-key in the current index. If there is another
**	sub-key it recursively calls itself.
**
**  Keywords:
**	find, locate
**
**  Arguments:
**	key_ptr   : a pointer to a parsed key structure
**	index_ptr : a pointer to the head of the index
**	entry_ptr : a pointer to the located entry (if found)
**
**  Result:
**	SUCCESS    : the key is found and in memory
**	NOT_IN_MEM : the key is found, but not yet read in
**	NOT_FND    : no keys matched
**
**  Exceptions:
**	None
**--
*/
    {
    int            status = NOT_FND;
    INDEX_TYPE_PTR cur_ptr;

    cur_ptr = index_ptr;
/*
** traverse the structure
*/
    while ((cur_ptr != NULL) && (status == NOT_FND))
	{
	if (key_ptr->key.dsc_w_length == cur_ptr->keyname.dsc_w_length)
	    {
	    if (case_less_str_equal(cur_ptr->keyname.dsc_a_pointer,
		key_ptr->key.dsc_a_pointer))
		{
/*
** the sub_key matched at this level, is it in memory?
*/
		if (cur_ptr->in_memory)
		    {
/*
** are we done with the sub_keys?
*/
		    if (key_ptr->next == NULL)
			{
			status = SUCCESS;
			*entry_ptr = cur_ptr;
			}
		    else
			status = locate_key(key_ptr->next, cur_ptr->sub_index,
					    entry_ptr);
		    }
		else
		    {
		    *entry_ptr = cur_ptr;
		    status = NOT_IN_MEM;
		    }
		}
	    }
	if (status == NOT_FND)
	    cur_ptr = cur_ptr->next;
	}
    
    return(status);
    }

static int store_key(full_key_ptr, line_ptr, entry_ptr)
DSC_DESCRIPTOR *full_key_ptr;
char **line_ptr;
INDEX_TYPE_PTR entry_ptr;
/*
**++
**  Functional Description:
**	this routine stores the keyname and builds the full key for an entry
**	in the index.
**
**  Keywords:
**	store
**
**  Arguments:
**	full_key_ptr : pointer to the parent's full key
**	line_ptr     : line containing the new key name
**	entry_ptr    : the new entry
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {
    int count = 0,
        i     = 0;
    char *line;

    line = *line_ptr;
/*    
** skip the first character if it is a digit
*/
    if (isdigit(line[0])) i = 1;
/*    
** purge leading blanks
*/
    while (line[i] == ' ') i++;
/*    
** bump counters while a terminator is not seen. We don't want what's in
** parenthesis.
*/
    while ((line[i] != '\0') && (line[i] != '\n') && (line[i] != '(')
           && (line[i] != ' '))
	{
	count++;
	i++;
	}

    if (count > 0)
	{
/*
** store the key name and the descriptor info
*/
	entry_ptr->keyname.dsc_a_pointer = (char *) XtMalloc(count + 1);
	DXmBCopy(&line[i - count], entry_ptr->keyname.dsc_a_pointer, count);
	entry_ptr->keyname.dsc_a_pointer[count] = '\0';
	}
    entry_ptr->keyname.dsc_w_length  = count;
    entry_ptr->keyname.dsc_b_dtype   = DSC_K_DTYPE_T;
    entry_ptr->keyname.dsc_b_class   = DSC_K_CLASS_S;
/*
** build the full key name and store it
*/
    entry_ptr->full_key.dsc_a_pointer = (char *) XtMalloc(full_key_ptr->dsc_w_length +
					       count + 1 + 1);
    strcpy(entry_ptr->full_key.dsc_a_pointer, full_key_ptr->dsc_a_pointer);
    strcat(entry_ptr->full_key.dsc_a_pointer, " ");
    strcat(entry_ptr->full_key.dsc_a_pointer, entry_ptr->keyname.dsc_a_pointer);
    entry_ptr->full_key.dsc_w_length  = full_key_ptr->dsc_w_length + count + 1;
    entry_ptr->full_key.dsc_b_dtype   = DSC_K_DTYPE_T;
    entry_ptr->full_key.dsc_b_class   = DSC_K_CLASS_S;
/*
** initialize the pointer to the title
*/
    entry_ptr->title.dsc_a_pointer = 0;
    
    return (SUCCESS);
    }

static int get_record(contxt_ptr, line_ptr, line_len_ptr)
WORK_BLOCK_TYPE_PTR contxt_ptr;
char **line_ptr;
int *line_len_ptr;
/*
**++
**  Functional Description:
**	this routine gets a record from the library, stores it a the working
**	buffer, adds a line terminator and a string terminator. The string
**	terminator will be overwritten when the next record is stored, if
**	there is one.
**
**  Keywords:
**	store
**
**  Arguments:
**	contxt_ptr : a pointer to the work block
**	line_ptr   : a pointer in the work buffer at the end of the stored 
**	             record
**	line_len_ptr : length of the stored line
**
**  Result:
**	SUCCESS
**	FIRST_SKIPED
**	when no more records are to be read, a character zero will be stored
**
**  Exceptions:
**	None
**--
*/
    {
    int status = SUCCESS,
        i,
	skip_first = FALSE;
    char *start_pos;
    DSC_DESCRIPTOR line_dsc;

    line_dsc.dsc_w_length = 0;
    line_dsc.dsc_b_dtype = DSC_K_DTYPE_T;
    line_dsc.dsc_b_class = DSC_K_CLASS_S;
    line_dsc.dsc_a_pointer = 0;
/*
**  find out the starting position in the buffer
*/
    start_pos = (contxt_ptr->wrk_buf_ptr) + (contxt_ptr->wrk_buf_used);
    
#ifdef VMS
    status = LBR$GET_RECORD(&(contxt_ptr->lbr_index), 0, &line_dsc);
    if (status == RMS$_EOF)
#else
    status = LBR_GET_RECORD(&(contxt_ptr->lbr_index), 0, &line_dsc);
    if (status == RMS__EOF)
#endif
    {
	status = EOM;
	start_pos[0] = '0';
	start_pos[1] = ' ';
    }
    else
	{
/*
**	make sure there is enough space for storing the new record
*/
	while (((contxt_ptr->wrk_buf_siz)-(contxt_ptr->wrk_buf_used))
	       < (line_dsc.dsc_w_length + 100))
	    {
	    contxt_ptr->wrk_buf_ptr = (char *) XtRealloc(contxt_ptr->wrk_buf_ptr,
	                                  contxt_ptr->wrk_buf_siz + CHUNK);
	    contxt_ptr->wrk_buf_siz = contxt_ptr->wrk_buf_siz + CHUNK;
/*
**	    recompute the start position in the work buffer
*/
	    start_pos = (contxt_ptr->wrk_buf_ptr) + (contxt_ptr->wrk_buf_used);
	    }
/*
**	copy the record, check if first character is blank
*/
	if (line_dsc.dsc_w_length > 0)
	    if (line_dsc.dsc_a_pointer[0] == ' ')
		{
		skip_first = TRUE;
		DXmBCopy(&(line_dsc.dsc_a_pointer[1]), start_pos,
			 line_dsc.dsc_w_length - 1);
		status = FIRST_SKIPED;
		}
	    else
		DXmBCopy(line_dsc.dsc_a_pointer, start_pos,
			line_dsc.dsc_w_length);
			
/*
**	Add terminators
*/
	if (skip_first)
	    {
	    start_pos[line_dsc.dsc_w_length - 1] = '\n';
	    *line_len_ptr = line_dsc.dsc_w_length;
	    start_pos[line_dsc.dsc_w_length] = '\0';
	    }
	else
	    {
	    start_pos[line_dsc.dsc_w_length] = '\n';
	    *line_len_ptr = line_dsc.dsc_w_length + 1;
	    start_pos[line_dsc.dsc_w_length + 1] = '\0';
	    }
	}
/*
**  return the start position in the buffer
*/
    *line_ptr = start_pos;

    return (status);
    }

static int build_index(current_level, line, full_key, wrk_blk_ptr, entry_ptr,
                       incl_ptr_ptr)
char *current_level, **line;
DSC_DESCRIPTOR *full_key;
WORK_BLOCK_TYPE_PTR wrk_blk_ptr;
INDEX_TYPE_PTR entry_ptr;
INCLUDE_LIST_PTR *incl_ptr_ptr;
/*
**++
**  Functional Description:
**	this routine fills an index entry in memory. It calls itself
**	recursively for any sibling or child it encounters.
**
**  Keywords:
**	build, fill
**
**  Arguments:
**	level       : the level in the hierarchy ** it's a character **
**	line        : the current line read from the library
**	full_key    : the parent's full key
**	wrk_blk_ptr : a pointer to the work block
**	entry_ptr   : a pointer to the index entry to be filled
**	incl_ptr_ptr : the head of the include list
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {
    int qualifier = 0;
    char new_level;

    new_level = *current_level;

    process_help_text(&new_level, line, wrk_blk_ptr, entry_ptr, incl_ptr_ptr,
	&qualifier);

    if (new_level > '0') {

	if (new_level > *current_level)
	    {
	    /*
	    ** create a new child entry
	    */
	    entry_ptr->sub_index = (INDEX_TYPE_PTR) XtMalloc(sizeof(INDEX_TYPE));
	    store_key(&(entry_ptr->full_key), line, entry_ptr->sub_index);
	    /*
	    ** reset the work buffer
	    */
	    wrk_blk_ptr->wrk_buf_ptr[0] = '\0';
	    wrk_blk_ptr->wrk_buf_used = 0;
	    /*
	    ** initialize the various fields
	    */
	    entry_ptr->sub_index->next = NULL;
	    entry_ptr->sub_index->sub_index = NULL;
	    entry_ptr->sub_index->include_ptr = NULL;
	    entry_ptr->sub_index->keyword_ptr = NULL;
	    entry_ptr->sub_index->qualifier_ptr = NULL;
	    entry_ptr->sub_index->txt_buf_ptr = NULL;
	    entry_ptr->sub_index->txt_buf_siz = 0;
	    entry_ptr->sub_index->nosearch = FALSE;
	    build_index(&new_level, line, &(entry_ptr->full_key), wrk_blk_ptr,
				 entry_ptr->sub_index, incl_ptr_ptr);
	    }
	if (new_level == *current_level)
	    {
	    /*
	    ** create a new sibling
	    */
	    entry_ptr->next = (INDEX_TYPE_PTR) XtMalloc(sizeof(INDEX_TYPE));
	    if (qualifier == 1) (*line)[0] = '/';
	    store_key(full_key, line, entry_ptr->next);
	    wrk_blk_ptr->wrk_buf_ptr[0] = '\0';
	    wrk_blk_ptr->wrk_buf_used = 0;
	    entry_ptr->next->next = NULL;
	    entry_ptr->next->sub_index = NULL;
	    entry_ptr->next->include_ptr = NULL;
	    entry_ptr->next->keyword_ptr = NULL;
	    entry_ptr->next->qualifier_ptr = NULL;
	    entry_ptr->next->txt_buf_siz = 0;
	    entry_ptr->next->txt_buf_ptr = NULL;
	    entry_ptr->next->nosearch = FALSE;
	    build_index(&new_level, line, full_key, wrk_blk_ptr, entry_ptr->next,
			incl_ptr_ptr);
	    }
	*current_level = new_level;
	}
	
    return (SUCCESS);
    }

static int process_help_text(level_ptr, line, wrk_blk_ptr, entry_ptr,
    incl_ptr_ptr, qualifier)
char *level_ptr;
char **line;
WORK_BLOCK_TYPE_PTR wrk_blk_ptr;
INDEX_TYPE_PTR entry_ptr;
INCLUDE_LIST_PTR *incl_ptr_ptr;
int *qualifier;
/*
**++
**  Functional Description:
**	this parses an help topic and collects the text and the extension
**
**  Keywords:
**	text, build, fill
**
**  Arguments:
**	level       : the level in the hierarchy ** it's a character **
**	line        : the current line read from the library
**	wrk_blk_ptr : a pointer to the work block
**	entry_ptr   : a pointer to the index entry to be filled
**	incl_ptr_ptr : the head of the include list
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {
    int get_rec_stat;
    int line_len;
    int parse_stat;

    get_rec_stat = get_record(wrk_blk_ptr, line, &line_len);
    
    /*
    **  while we don't have another level, parse the records.
    **  A digit followed by a dot is NOT a new level!
    */
    
    while ( !( ((isdigit((*line)[0]) && (*line)[1] == ' ') ||
	       ((*line)[0] == '/')) &&
	       (get_rec_stat != FIRST_SKIPED))) {
	       
	if (((*line)[0] != '!') || (get_rec_stat == FIRST_SKIPED))
	    {
	    if (((*line)[0] == '=') && (get_rec_stat != FIRST_SKIPED))
		{
		parse_stat = parse_extension(entry_ptr, line, line_len,
					     incl_ptr_ptr);
		if (!parse_stat)
		    wrk_blk_ptr->wrk_buf_used =
			      wrk_blk_ptr->wrk_buf_used + line_len;
		}
	    else
	    /*
	    **  acknowledge the fact that the read in record is text for
	    **  the topic
	    */
		wrk_blk_ptr->wrk_buf_used =
			      wrk_blk_ptr->wrk_buf_used + line_len;
	    }
	get_rec_stat = get_record(wrk_blk_ptr, line, &line_len);
    }

    /*
    **  Check if we reached a qualifier
    */
	 
    if (((*line)[0] == '/') && (get_rec_stat != FIRST_SKIPED)) {
	*qualifier = 1;
    }
    else {
    
	/*
	**  If it is not a qualifier report back the new level reached
	*/
    
	*qualifier = 0;
	*level_ptr = (*line[0]);
    }
    
    /*
    **  store the topic's text and mark the entry as in memory
    */
    if (wrk_blk_ptr->wrk_buf_used > 0)
	{
	entry_ptr->txt_buf_ptr = (char *) XtMalloc(wrk_blk_ptr->wrk_buf_used + 1);
	entry_ptr->txt_buf_siz = wrk_blk_ptr->wrk_buf_used;
	DXmBCopy(wrk_blk_ptr->wrk_buf_ptr, entry_ptr->txt_buf_ptr,
	    wrk_blk_ptr->wrk_buf_used);
	entry_ptr->txt_buf_ptr[wrk_blk_ptr->wrk_buf_used] = '\0';
	}
    else
	entry_ptr->txt_buf_ptr = NULL;
	
    entry_ptr->in_memory = 1;

    return(SUCCESS);
    }


static int set_module_cntxt(wrk_blk_ptr, line_ptr_ptr)
WORK_BLOCK_TYPE_PTR wrk_blk_ptr;
char **line_ptr_ptr;
/*
**++
**  Functional Description:
**	this routine sets the read context for the module
**
**  Keywords:
**	read
**
**  Arguments:
**	wrk_blk_ptr  : address of a work block
**	line_ptr_ptr : a pointer to a pointer to a buffer
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	none
**--
*/
    {
    int line_len;
    
    do {
	get_record(wrk_blk_ptr, line_ptr_ptr, &line_len);
    } while (((*line_ptr_ptr)[0] != '1') && ((*line_ptr_ptr)[0] != '0'));

    return (SUCCESS);
    }

static int read_module(lib_context_ptr, entry_ptr)
LIBRARY_CONTEXT_BLOCK_PTR lib_context_ptr;
INDEX_TYPE_PTR entry_ptr;
/*
**++
**  Functional Description:
**	this routine reads in a whole help module and builds the cache
**	structure in memory.
**
**  Keywords:
**	read, build
**
**  Arguments:
**	lib_context_ptr : the context block
**	entry_ptr       : the top level index entry
**
**  Result:
**	SUCCESS
**	any status returned by the LBR$ routines
**
**  Exceptions:
**	None
**--
*/
    {
    int		     status,
                     ast_status,
		     line_len,
		     find_status;
    char	     *line_ptr,
		     start_level = '1';   /* start at the top */
    INCLUDE_LIST_PTR include_ptr = NULL;
    WORK_BLOCK_TYPE_PTR wrk_blk_ptr;
/*
** allocate the work structure and work buffer
*/
    wrk_blk_ptr = (WORK_BLOCK_TYPE_PTR) XtMalloc(sizeof(WORK_BLOCK_TYPE));
    wrk_blk_ptr->wrk_buf_ptr  = (char *) XtMalloc(CHUNK);
    wrk_blk_ptr->wrk_buf_siz  = CHUNK;
    wrk_blk_ptr->wrk_buf_used = 0;
    wrk_blk_ptr->lbr_index = lib_context_ptr->lbr_index;
/*	
** read the module in
*/
    set_module_cntxt(wrk_blk_ptr, &line_ptr);
    build_index(&start_level, &line_ptr, &(entry_ptr->full_key), wrk_blk_ptr,
                entry_ptr, &include_ptr);
/*
** resolve the include extension, if any
*/
    if (include_ptr != NULL)
	fix_up_include(include_ptr, lib_context_ptr);
/*
** free the work block and buffer
*/
    XtFree(wrk_blk_ptr->wrk_buf_ptr);
    XtFree((char *)wrk_blk_ptr);
	
    return (SUCCESS);
    }

int DXmHelp_set_read_context(cntxt_ptr, entry_ptr, key_list_ptr)
LIBRARY_CONTEXT_BLOCK_PTR cntxt_ptr;
INDEX_TYPE_PTR entry_ptr;
KEY_CHUNK_PTR key_list_ptr;
/*
**++
**  Functional Description:
**	This routines sets the read context in the Help Library. It uses
**	LBR$SET_LOCATE routine to set the RMS mode to locate and LBR$FIND
**	routine to get the specified module. If LBR$FIND fails, which means that
**	the library as been updated, the whole cache is freed and the needed
**	module is read back in.
**
**  Keywords:
**	none
**
**  Arguments:
**	cntxt_ptr    : a pointer to a library context block
**	entry_ptr    : a pointer to a entry in cache
**	key_list_ptr : a pointer to the requested topic identifier
**
**  Result:
**	any status code returned by LBR$SET_LOCATE, LBR$FIND
**
**  Exceptions:
**	none
**--
*/
    {
    int status,
        ast_status,
	locate_status;
/*    
** set the read context for the given module
*/
#ifdef VMS
    ERROR_CHECK(LBR$SET_LOCATE(&(cntxt_ptr->lbr_index)));
    status = LBR$FIND(&(cntxt_ptr->lbr_index), &(entry_ptr->rfa));

    if (status == LBR$_INVRFA)
#else
    ERROR_CHECK(LBR_SET_LOCATE(&(cntxt_ptr->lbr_index)));
    status = LBR_FIND(&(cntxt_ptr->lbr_index), &(entry_ptr->rfa));

    if (status == LBR__INVRFA)
#endif
	{
	AST_OFF(ast_status);
	free_index(cntxt_ptr->cache_ptr);
	DXmHelp_collect_lib_index(cntxt_ptr);
	cntxt_ptr->new_cache = TRUE;
	AST_ON(ast_status);
	locate_status = locate_key(key_list_ptr, cntxt_ptr->cache_ptr,
                                   &entry_ptr);
	if (locate_status == NOT_IN_MEM)
	    {
#ifdef VMS
	    ERROR_CHECK(LBR$FIND(&(cntxt_ptr->lbr_index),
	                         &(entry_ptr->rfa)));
#else
	    ERROR_CHECK(LBR_FIND(&(cntxt_ptr->lbr_index),
	                         &(entry_ptr->rfa)));
#endif

	    if ((cntxt_ptr->caching == 1) || (cntxt_ptr->search_mode == 1))
		ERROR_CHECK(read_module(cntxt_ptr, entry_ptr));
	    }
	else
	    return (KEY_NOT_FND);
	}
    else
	if ((cntxt_ptr->caching == 1) || (cntxt_ptr->search_mode == 1))
	    ERROR_CHECK(read_module(cntxt_ptr, entry_ptr));
	
    return (status);
    }


static int get_text(entry_ptr, txt_ptr)
INDEX_TYPE_PTR entry_ptr;
char **txt_ptr;
/*
**++
**  Functional Description:
**	this routine returns the text associate with a topic. If the topic
**	is followed by '/' entries, it concatenates all the text and stores
**	it for this topic.
**
**  Keywords:
**	collect
**
**  Arguments:
**	entry_ptr : the entry for which we collect the text
**	txt_ptr   : a pointer to the text
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {
    int buf_siz = BUF_CHUNK,
        used_buf = 0,
	same_qualifiers = 1;
    char *buf;
    INDEX_TYPE_PTR cur_ptr;
/*
** did we already collected the text
*/
    if (entry_ptr->qualifier_ptr != NULL)
	*txt_ptr = entry_ptr->qualifier_ptr;
    else if (entry_ptr->next->keyname.dsc_a_pointer[0] == '/')
/* the next entry must be a '/' level entry  */
	    {
/* allocate a buffer  */
	    buf = (char *) XtMalloc(BUF_CHUNK);
	    buf[0] = '\0';
/*
** copy the first text into it, if any
*/
	    if (entry_ptr->txt_buf_ptr != NULL)
		{
		while (buf_siz < entry_ptr->txt_buf_siz)
		    {
		    buf = (char *) XtRealloc(buf, buf_siz + BUF_CHUNK);
		    buf_siz = buf_siz + BUF_CHUNK;
		    }
		strcat(buf, entry_ptr->txt_buf_ptr);
		used_buf = used_buf + entry_ptr->txt_buf_siz;
		}
	    cur_ptr = entry_ptr->next;
	    while ((cur_ptr != NULL) && (same_qualifiers == 1))
		{
/* always the same qualifiers? */
		if (cur_ptr->keyname.dsc_a_pointer[0] == '/')
		    {
/* copy the keyname and an extra \n */
		    while ((buf_siz - used_buf) <
			(cur_ptr->keyname.dsc_w_length + 1))
			{
			buf = (char *) XtRealloc(buf, buf_siz + BUF_CHUNK);
			buf_siz = buf_siz + BUF_CHUNK;
			}
		    strcat(buf, cur_ptr->keyname.dsc_a_pointer);
		    strcat(buf, "\n");
		    used_buf = used_buf + cur_ptr->keyname.dsc_w_length + 1;
/* check if there is some text  */
		    if (cur_ptr->txt_buf_ptr != NULL)
			{
/* check for space  */
			while ((buf_siz - used_buf) < cur_ptr->txt_buf_siz)
			    {
			    buf = (char *) XtRealloc(buf, buf_siz + BUF_CHUNK);
			    buf_siz = buf_siz + BUF_CHUNK;
			    }
			strcat(buf, cur_ptr->txt_buf_ptr);
			used_buf = used_buf + cur_ptr->txt_buf_siz;
			}
		    cur_ptr = cur_ptr->next;
		    }
		else
		    same_qualifiers = 0;
		}
/*
** return collected text and save it for next time
*/
	    entry_ptr->qualifier_ptr = buf;
	    *txt_ptr = entry_ptr->qualifier_ptr;
	    }
	else
	    *txt_ptr = entry_ptr->txt_buf_ptr;

    return (SUCCESS);
    }


static int get_sub_topics(entry_ptr, sub_topic_arr_ptr, title_arr_ptr,
                         arr_siz_ptr)
INDEX_TYPE_PTR entry_ptr;
char ***sub_topic_arr_ptr, ***title_arr_ptr;
int *arr_siz_ptr;
/*
**++
**  Functional Description:
**	this routine returns an array of pointers to the subtopic titles and
**	keys, as well as a count. It collects the sub topic by scanning the
**	sub-index level entry list. It also looks if any include entries
**	have to be returned as well.
**
**  Keywords:
**	None
**
**  Arguments:
**	entry_ptr         : entry for which sub-topics are collected
**	sub_topic_arr_ptr : the sub-topic array
**	title_arr_ptr     : the matching title array
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {
    int topic_cnt = 0,
        incl_cnt = 0,
	i;
    INDEX_TYPE_PTR topic_ptr;
    INCLUDE_TYPE_PTR incl_ptr;
#ifdef GENERAL_TEXT
    long count,stat;
#endif /* GENERAL_TEXT */
/*
** count the number of sub-topics
*/
    topic_ptr = entry_ptr->sub_index;
    while (topic_ptr != NULL)
	{
	topic_cnt++;
	topic_ptr = topic_ptr->next;
	}
/*
** count the number of include entries
*/
    incl_ptr = entry_ptr->include_ptr;
    while (incl_ptr != NULL)
	{
	incl_cnt++;
	incl_ptr = incl_ptr->next;
	}
/*
** allocate the space for the arrays
*/
    *sub_topic_arr_ptr = (char **) XtMalloc(sizeof(char *) * (topic_cnt + incl_cnt));
    *title_arr_ptr     = (char **) XtMalloc(sizeof(char *) * (topic_cnt + incl_cnt));
    *arr_siz_ptr = topic_cnt + incl_cnt;
/*
** fill in the sub-topic array with the real sub-topics
*/
    topic_ptr = entry_ptr->sub_index;
	for (i = 0; i < topic_cnt; i++)
	    {
	    (*sub_topic_arr_ptr)[i] =
#ifdef GENERAL_TEXT
		(char *)DXmCvtFCtoCS(topic_ptr->full_key.dsc_a_pointer,
				     &count,&stat);
#else
		(char *)XmStringLtoRCreate(topic_ptr->full_key.dsc_a_pointer, "ISO8859-1");
#endif /* GENERAL_TEXT	*/	
	    if (topic_ptr->title.dsc_a_pointer == NULL)
		(*title_arr_ptr)[i] =
#ifdef GENERAL_TEXT
		    (char *)DXmCvtFCtoCS(topic_ptr->keyname.dsc_a_pointer,
				         &count,&stat);
#else
		    (char *)XmStringLtoRCreate(topic_ptr->keyname.dsc_a_pointer, "ISO8859-1");
#endif /* GENERAL_TEXT	*/	
	    else
	        (*title_arr_ptr)[i] =
#ifdef GENERAL_TEXT
		    (char *)DXmCvtFCtoCS(topic_ptr->title.dsc_a_pointer,
				     	 &count,&stat);
#else
		    (char *)XmStringLtoRCreate(topic_ptr->title.dsc_a_pointer, "ISO8859-1");
#endif /* GENERAL_TEXT	*/	
	    
	    topic_ptr = topic_ptr->next;
	    }
/*
** fill in the sub-topic array with the include entries
*/
    incl_ptr = entry_ptr->include_ptr;
	for (i; i < (topic_cnt + incl_cnt); i++)
	    {
#ifdef GENERAL_TEXT
	    (*sub_topic_arr_ptr)[i] = (char *)DXmCvtFCtoCS(incl_ptr->full_key,
				     			   &count,&stat);
	    (*title_arr_ptr)[i] = (char *)DXmCvtFCtoCS(incl_ptr->title,
				     		       &count,&stat);
#else
	    (*sub_topic_arr_ptr)[i] = (char *)XmStringLtoRCreate(incl_ptr->full_key, "ISO8859-1");
	    (*title_arr_ptr)[i] = (char *)XmStringLtoRCreate(incl_ptr->title, "ISO8859-1");
#endif /* GENERAL_TEXT*/		
	    incl_ptr = incl_ptr->next;
	    }

    return (SUCCESS);
    }

static int build_frame(lib_context_ptr, entry_ptr, title_ptr, txt_ptr,
    sub_topic_arr_ptr, title_arr_ptr, arr_siz_ptr)
LIBRARY_CONTEXT_BLOCK_PTR lib_context_ptr;
INDEX_TYPE_PTR entry_ptr;
char **title_ptr, **txt_ptr;
char ***sub_topic_arr_ptr, ***title_arr_ptr;
int *arr_siz_ptr;
/*
**++
**  Functional Description:
**	this routine builds a help frame. It collects the title, the text
**	and the sub-topic array with the matching title array.
**
**  Keywords:
**	build
**
**  Arguments:
**	entry_ptr     : the entry for which the frame is built
**	title_ptr     : the title of the frame
**	txt_ptr       : the text of the frame
**	sub_topic_arr_ptr : the array of pointers to the sub-topics
**	title_arr_ptr : the array of pointers to the titles
**	arr_siz_ptr   : the number of sub-topics
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    {

    INDEX_TYPE_PTR sub_entry_ptr;
    INDEX_TYPE_PTR empty_fr;

    /*
    **  If no keyname is specified build an empty frame with the list of
    **	level-1 topics as additional topics
    */
    
    if (entry_ptr == NULL)
	{
	empty_fr = (INDEX_TYPE_PTR) XtMalloc(sizeof(INDEX_TYPE));
	empty_fr->sub_index = lib_context_ptr->cache_ptr;
	empty_fr->include_ptr = NULL;
	get_sub_topics(empty_fr, sub_topic_arr_ptr, title_arr_ptr, arr_siz_ptr);
	*title_ptr = NULL;
	*txt_ptr = NULL;
	}
    else
	{
	/*
	** get the title or the full key if any
	*/
	if (entry_ptr->title.dsc_a_pointer == NULL)
	    *title_ptr = entry_ptr->full_key.dsc_a_pointer;
	else
	    *title_ptr = entry_ptr->title.dsc_a_pointer;
	/*	
	** collect text if entries are qualifiers or special case of negatate
	** qualifiers e.g., /foo /nofoo
	*/
	if (entry_ptr->keyname.dsc_a_pointer[0] == '/')
	    {
	    if ((entry_ptr->txt_buf_ptr == NULL) && (entry_ptr->next != NULL))
		{
		*txt_ptr = entry_ptr->next->txt_buf_ptr;
		}
	    else
		{
		*txt_ptr = entry_ptr->txt_buf_ptr;
		}
	    }
	else
	    {
	    if (entry_ptr->next != NULL)
		{
		if (entry_ptr->next->keyname.dsc_a_pointer[0] == '/')
		    {
		    get_text(entry_ptr, txt_ptr);
		    }
		else
		    {
		    *txt_ptr = entry_ptr->txt_buf_ptr;
		    }
		}
	    else
		{
		*txt_ptr = entry_ptr->txt_buf_ptr;
		}
	    }
	/*    
	** collect the sub-topics if any
	*/
	if ((entry_ptr->sub_index == NULL) && (entry_ptr->include_ptr == NULL))
	    {
	    *sub_topic_arr_ptr = 0;
	    *title_arr_ptr = 0;
	    *arr_siz_ptr = 0;
	    }
	else
	    {
	    get_sub_topics(entry_ptr, sub_topic_arr_ptr, title_arr_ptr, arr_siz_ptr);
	    }
	}

    return (SUCCESS);
    }

int DXmHelp__get_frame(lib_context_ptr, query_ptr, title_ptr, txt_ptr,
                     sub_topic_arr_ptr, title_arr_ptr, arr_siz_ptr)
LIBRARY_CONTEXT_BLOCK_PTR lib_context_ptr;
char *query_ptr, **title_ptr, **txt_ptr, **sub_topic_arr_ptr, **title_arr_ptr;
int *arr_siz_ptr;
/*
**++
**  Functional Description:
**	this routine will locate the frame for the given key. If the module
**	in which the frame should be is not in memory, it is read the
**	appropriate module in. Otherwise it builds the frame.
**
**  Keywords:
**	get
**
**  Arguments:
**	lib_context_ptr : the library context block
**	query_ptr       : the key to look for
**	title_ptr       : title to return for the frame
**	txt_ptr         : text for the frame
**	sub_topic_arr_ptr : array of pointers to the sub-topics
**	title_arr_ptr   : array of pointer to the matching titles
**	arr_siz_ptr     : number of sub-topics
**
**  Result:
**	SUCCESS : the frame was found
**	NOT_FND : couldn't find the key
**	any error status returned by the LBR$ routines
**	
**  Exceptions:
**	None
**--
*/
    {
    int locate_status,
        status = SUCCESS;
    KEY_CHUNK_PTR key_list_ptr;
    INDEX_TYPE_PTR entry_ptr;

    /*
    **  Build an empty frame with additional topics if the keyname is NULL
    */

    if (query_ptr == NULL)
	{
	if (lib_context_ptr->caching == 1) {
	    ERROR_CHECK(read_in_library(lib_context_ptr));
	}
	else {
	    ERROR_CHECK(load_toplevel_titles(lib_context_ptr));
	}
	build_frame(lib_context_ptr, NULL, title_ptr, txt_ptr,
	    sub_topic_arr_ptr, title_arr_ptr, arr_siz_ptr);
	}
    else
	{
	/*
	** decompose the key and locate it in cache
	*/
	parse_key(query_ptr, &key_list_ptr);
	locate_status = locate_key(key_list_ptr, lib_context_ptr->cache_ptr,
				   &entry_ptr);
	/*    
	** read the module in and locate the key again
	*/
	if (locate_status == NOT_IN_MEM)
	    {
	    /*
	    ** library open ?
	    */
	    if (!lib_context_ptr->lib_open)
		{
		DXmHelp_open_lib(lib_context_ptr);
		lib_context_ptr->open_cnt++;
		}
	    ERROR_CHECK(DXmHelp_set_read_context(lib_context_ptr, entry_ptr,
						 key_list_ptr));
	    if ((lib_context_ptr->caching == 1) ||
		(lib_context_ptr->search_mode == 1))
		locate_status = locate_key(key_list_ptr, entry_ptr, &entry_ptr);
	    else
		locate_status = get_frame_nocache(lib_context_ptr, query_ptr,
		    &entry_ptr, key_list_ptr, title_ptr, txt_ptr,
		    sub_topic_arr_ptr, title_arr_ptr, arr_siz_ptr);
	    }
	free_key(key_list_ptr);
	/*    
	** get the frame components if the key exists
	*/
	if (locate_status == SUCCESS) {
	    if ((lib_context_ptr->caching == 1) ||
		(lib_context_ptr->search_mode == 1)){
		build_frame(lib_context_ptr, entry_ptr, title_ptr, txt_ptr,
		    sub_topic_arr_ptr, title_arr_ptr, arr_siz_ptr);
	    }
	}
	else
	    status = KEY_NOT_FND;
	}
		
    return (status);
    }


static int load_toplevel_titles(context)
LIBRARY_CONTEXT_BLOCK *context;
/*
**++
**  Functional Description:
**	collects the titles for the top level entries
**
**  Keywords:
**	scan, title, top level
**
**  Arguments:
**	context : address of a library context block
**
**  Result:
**	SUCCESS
**	any status code returned by the LBR$ routines
**
**  Exceptions:
**	None
**--
*/

    {
    WORK_BLOCK_TYPE *wrk_blk;
    INDEX_TYPE *top_entry;
    INDEX_TYPE *entry;
    char level = '1';
    int status = SUCCESS;
    int locate_status;
    KEY_CHUNK *key;
    char *line;
    char *title = NULL;
    
    /*
    ** allocate the work structure and work buffer
    */

    wrk_blk = (WORK_BLOCK_TYPE_PTR) XtMalloc(sizeof(WORK_BLOCK_TYPE));
    wrk_blk->wrk_buf_ptr  = (char *) XtMalloc(CHUNK);
    wrk_blk->wrk_buf_siz  = CHUNK;
    wrk_blk->wrk_buf_used = 0;
    wrk_blk->lbr_index = context->lbr_index;

    top_entry = context->cache_ptr;

    /*
    **  Run down the list of top level entries
    */
    
    while (top_entry != NULL) {

	/*
	**  Check if the title has already been collected
	*/
	    
	if (top_entry->title.dsc_a_pointer == NULL) {
	    parse_key(top_entry->keyname.dsc_a_pointer, &key);
	    locate_status = locate_key(key, context->cache_ptr, &entry);
	    if (locate_status == NOT_IN_MEM) {
		if (!context->lib_open) {
		    DXmHelp_open_lib(context);
		    context->open_cnt++;
		}
		ERROR_CHECK(DXmHelp_set_read_context(context, entry, key));
		
		/*	
		**	Get to the beginning of the module 
		*/

		set_module_cntxt(wrk_blk, &line);

		/*
		**  Look for the title
		*/

		scan_for_title(wrk_blk, &line, &title, &level);

		/*
		**  Load the title in the top level entry
		*/
		
		if (title != NULL) {
		    entry->title.dsc_a_pointer = title;
		    entry->title.dsc_w_length = strlen(title);
		    title = NULL;
		} else {
		    entry->title.dsc_a_pointer =
			CopyStr(entry->keyname.dsc_a_pointer);
		    entry->title.dsc_w_length = entry->keyname.dsc_w_length;
		}
		entry->title.dsc_b_dtype  = DSC_K_DTYPE_T;
		entry->title.dsc_b_class  = DSC_K_CLASS_S;
		
		wrk_blk->wrk_buf_ptr[0] = '\0';
		wrk_blk->wrk_buf_used = 0;
	    }
	    free_key(key);
	}
	
	top_entry = top_entry->next;
    }

    /*
    **  Free the work buffer
    */

    XtFree(wrk_blk->wrk_buf_ptr);
    XtFree((char *)wrk_blk);
	
    return (status);
    }
    

int DXmHelp__free_array(topic_arr, title_arr, caching, size)
char **topic_arr;
char **title_arr;
Boolean caching;
int size;
	    
/*
**++
**  Functional Description:
**	frees the sub-topic array and the title array
**
**  Keywords:
**	free
**
**  Arguments:
**	sub_topic_arr_ptr : sub-topic array to be freed
**	title_arr_ptr     : title array to be freed
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
        
    if (!caching) {
	
	/*
	**  Free the contents of the array as well
	*/

	for (i = 0; i < size; i++) {
	    if (topic_arr != NULL)
		if (topic_arr[i] != NULL)
		    XtFree(topic_arr[i]);
	    if (title_arr != NULL)
		if (title_arr[i] != NULL)
		    XtFree(title_arr[i]);
	}
    }

    XtFree((char *)topic_arr);
    XtFree((char *)title_arr);
	
    return (SUCCESS);
    }


static int get_frame_nocache(context, query_ptr, frame_ptr, key_list, title,
    text, sub_topic_arr, title_arr, count)
LIBRARY_CONTEXT_BLOCK_PTR context;
char *query_ptr;
INDEX_TYPE **frame_ptr;
KEY_CHUNK *key_list;
char **title;
char **text;
char ***sub_topic_arr;
char ***title_arr;
int *count;
/*
**++
**  Functional Description:
**	this routine fetches a topic from the library, resolve the include
**	extension if any and build an hel frame,
**	
**  Keywords:
**	topic, frame
**
**  Arguments:
**	query_ptr   : address of topic full key character string
**	frame_ptr   : the index structure to be returned
**	key_list    : the key structure to scan the module
**	title	    : the title of the frame
**	text	    : the text of the frame
**	sub_topic_arr	: array of additional topics full keys
**	title_arr   : array of additional topics titles
**	count	    : array size
**
**  Result:
**	SUCCESS
**	KEY_NOT_FND
**
**  Exceptions:
**	None
**--
*/
    
    {
    int status;
    INCLUDE_LIST *include_list = NULL;
    INCLUDE_TYPE *topic_list = NULL;
    WORK_BLOCK_TYPE *wrk_blk;

    /*
    ** allocate the work structure and work buffer
    */

    wrk_blk = (WORK_BLOCK_TYPE_PTR) XtMalloc(sizeof(WORK_BLOCK_TYPE));
    wrk_blk->wrk_buf_ptr  = (char *) XtMalloc(CHUNK);
    wrk_blk->wrk_buf_siz  = CHUNK;
    wrk_blk->wrk_buf_used = 0;
    wrk_blk->lbr_index = context->lbr_index;

    /*
    **  Locate the topic in the help library
    */
        
    status = find_frame(wrk_blk, frame_ptr, key_list, &include_list,
	&topic_list, query_ptr);

    if (status == SUCCESS) {

	if (include_list != NULL) 
	    resolve_includes(context, wrk_blk, include_list);

	/*
	**  Return the frame title if any
	*/
	
	if ((*frame_ptr)->title.dsc_a_pointer != NULL)
	    *title = (*frame_ptr)->title.dsc_a_pointer;
	else
	    *title = query_ptr;

	/*
	**  Return the help text
	*/
		    
	*text = (*frame_ptr)->txt_buf_ptr;

	/*
	**  Check if there are additional topics and format the output arrays
	*/
	
	if ((topic_list == NULL) && (include_list == NULL)) {
	    *sub_topic_arr = NULL;
	    *title_arr = NULL;
	    *count = 0;
	}
	else {
	    format_sub_topic(topic_list, include_list, sub_topic_arr, title_arr,
		count);

	/*
	**  Free the topic and include lists
	*/

	free_include_list(include_list);
	free_include(topic_list);
	
	}
    }
    else
	status = KEY_NOT_FND;

    /*
    **  Free the work buffer
    */

    XtFree(wrk_blk->wrk_buf_ptr);
    XtFree((char *)wrk_blk);
	
    return(status);
    }
    

static int find_frame(wrk_blk, frame_ptr, key_list, include_list_ptr,
	topic_list_ptr, query)
WORK_BLOCK_TYPE *wrk_blk;
INDEX_TYPE **frame_ptr;
KEY_CHUNK *key_list;
INCLUDE_LIST **include_list_ptr;
INCLUDE_TYPE **topic_list_ptr;
char *query;
/*
**++
**  Functional Description:
**	this routine finfs a topic in a module and returns an help frame.
**	
**  Keywords:
**	find, topic, frame
**
**  Arguments:
**	wrk_blk		: address of a work block structure
**	key_list	: address of a key list structure
**	frame_ptr	: address of pointer to an index type structure
**	include_list_ptr    : address of a pointer to an include list structure
**	topic_list_ptr  : address of a pointer to a topic list structure
**	query	    : address of a character pointer containing the topic key
**
**  Result:
**	SUCCESS
**	KEY_NOT_FOUND
**
**  Exceptions:
**	None
**--
*/

    {
    Boolean is_qualifier = FALSE;
    int status = SUCCESS;
    char *line;
    char level = '1';
    
    /*	
    **	Get to the beginning of the module 
    */

    set_module_cntxt(wrk_blk, &line);

    /*
    **  Scan the module to locate the key if it isn't the top level key
    */
    
    if (key_list->next != NULL)
	status = scan_module(&level, key_list, wrk_blk, &is_qualifier);

    /*
    **  If the key was found, collect topic information
    */
    
    if (status == SUCCESS) {

	/*
	**  Allocate and initialize the frame structure
	*/
	
	(*frame_ptr) = init_entry(query);
	
	/*
	**  Collect the frame data
	*/

	collect_frame(level, wrk_blk, *frame_ptr, include_list_ptr,
	    topic_list_ptr, is_qualifier);
    }
    else {
	*frame_ptr = NULL;
	status = KEY_NOT_FND;
    }

    return(status);
    }


static int scan_module(level_ptr, key_list, wrk_blk, is_qualifier)
char *level_ptr;
KEY_CHUNK *key_list;
WORK_BLOCK_TYPE *wrk_blk;
Boolean *is_qualifier;
/*
**++
**  Functional Description:
**	this routine scans a module until the given key is found.
**	
**  Keywords:
**	scan, topic, frame
**
**  Arguments:
**	level_ptr	: address of a character
**	key_list	: address of a key structure containing the key to match
**	wrk_blk		: address of a work block structure
**	is_qualifier	: boolean
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/

    {
    Boolean done = FALSE;
    Boolean qualifier = FALSE;
    char current_level = '1';
    char *line;
    char match_level = '2';
    int status = KEY_NOT_FND;
    int i;
    int line_len;
    int get_rec_stat;
    int match_lev;
    KEY_CHUNK *key_at_level;
    KEY_CHUNK *tmp_key;

    get_rec_stat = get_record(wrk_blk, &line, &line_len);

    while ((get_rec_stat != EOM) && (!done)) {

	/*
	**  Check if we have reached another level
	*/
	
	if (((isdigit(line[0]) && (line[1] == ' ')) ||
	    (line[0] == '/')) &&
	    (get_rec_stat != FIRST_SKIPED)) {

	    /*
	    **  slashes are equivalent to the current level
	    */
	
	    if (line[0] == '/') {
		qualifier = TRUE;
	    }

	    /*
	    **  Check if the new level is the level we are trying to match
	    */

	    if (!qualifier)	    
		current_level = line[0];
		
	    if (current_level == match_level) {

		/*
		**  Yes, so compare the keys for that level
		*/

		if (qualifier)		
		    parse_key(&line[0], &tmp_key);
		else
		    parse_key(&line[2], &tmp_key);
		key_at_level = key_list;
		match_lev = match_level - '0';
		for (i = 1; i < match_lev; i++)
		    key_at_level = key_at_level->next;

		/*
		**  If keys match, increment the match level but not if qualifiers
		*/
		
		if (case_less_str_equal(key_at_level->key.dsc_a_pointer,
			tmp_key->key.dsc_a_pointer)) {

		    if (!qualifier)
			match_level++;

		    /*
		    **  Check if we are done
		    */
		    
		    if (key_at_level->next == NULL) {
			done = TRUE;
			status = SUCCESS;
			if (key_at_level->key.dsc_a_pointer[0] == '/')
			    *is_qualifier = TRUE;
		    }
		free_key(tmp_key);
		}
	    }
	    else

		/*
		**  The key was not found, stop parsing the module
		*/
		
		if (current_level < match_level)
		    done = TRUE;
	}
	qualifier = FALSE;
	if (!done)
	    get_rec_stat = get_record(wrk_blk, &line, &line_len);
    }

    *level_ptr = current_level;
    
    return(status);
    }
            

static int collect_frame(key_level, wrk_blk, entry, include_list_ptr,
	topic_list_ptr, is_qualifier)
char key_level;
WORK_BLOCK_TYPE *wrk_blk;
INDEX_TYPE *entry;
INCLUDE_LIST **include_list_ptr;
INCLUDE_TYPE **topic_list_ptr;
Boolean is_qualifier;
/*
**++
**  Functional Description:
**	this routine collects all the information needed to build a frame.
**	
**  Keywords:
**	scan, topic, frame
**
**  Arguments:
**	key_level	: character
**	wrk_blk		: address of a work block structure
**	entry		: address of an index type structure
**	include_list_ptr    : address of an include list structure
**	topic_list_ptr  : address of a topic list structure
**	is_qualifier	: boolean
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
	
    {
    KEY_CHUNK *keyname;
    INCLUDE_TYPE *current_topic;
    char *line;
    char parse_level;
    char next_level;
    int qualifier;
    
    parse_level = key_level;
    
    /*
    **  Collect the help text and the extension
    */
        
    process_help_text(&parse_level, &line, wrk_blk, entry, include_list_ptr,
	&qualifier);

    /*
    **  This is for the /FOO/NOFOO special case
    */

    if ((entry->txt_buf_ptr == NULL) && (line[0] == '/')) 
	process_help_text(&parse_level, &line, wrk_blk, entry,
	include_list_ptr, &qualifier);

    /*
    **  This is for 'Command Qualifiers' special case
    */

    if ((line[0] == '/') && !(is_qualifier)) {
	collect_qualifiers(&parse_level, &line, wrk_blk, entry,
	    include_list_ptr, &qualifier);
    }
    
    /*
    **  Continue parsing module for the additional sub-topics
    */
    
    while ((parse_level == (key_level + 1)) && (line[0] != '0')) {

	/*
	**  Collect the full key of an additional topic
	*/

	if (line[0] == '/')
	    parse_key(line, &keyname);
	else
	    parse_key(&line[2], &keyname);
	    
	get_full_key(keyname, entry, topic_list_ptr, &current_topic);

	/*
	**  Reset the work buffer to clear previous line
	*/
	
	wrk_blk->wrk_buf_ptr[0] = '\0';
	wrk_blk->wrk_buf_used = 0;

	/*
	**  Collect the title for the additional topic
	*/
	
	scan_for_title(wrk_blk, &line, &current_topic->title, &next_level);

	/*
	**  If the next level reached is a '/', we are still at the same level
	*/
		
	if (next_level != '/')
	    parse_level = next_level;
	    
	/*
	**  If no title found, default it to the keyname
	*/
		    
	if (current_topic->title == NULL)
	    current_topic->title = CopyStrN(keyname->key.dsc_a_pointer,
		keyname->key.dsc_w_length + 1);

	/*
	**  Skip any lower level topic
	*/
	
	if (parse_level > (key_level + 1))
	    skip_sublevel(wrk_blk, &line, &parse_level);

	free_key(keyname);
    }

    return(SUCCESS);
    }


static int collect_qualifiers(parse_level_ptr, line_ptr, wrk_blk, entry,
    include_list_ptr, qualifier)
char *parse_level_ptr;
char **line_ptr;
WORK_BLOCK_TYPE *wrk_blk;
INDEX_TYPE *entry;
INCLUDE_LIST **include_list_ptr;
int *qualifier;
/*
**++
**  Functional Description:
**	this routine scans a module to collect the text of several topics.
**	This routine is used in the special case of the 'command qualifiers'
**	topic in the VMS help library,
**
**  Keywords:
**	scan, topic, command qualifier
**
**  Arguments:
**	parse_level_ptr	: address of character
**	line_ptr	: address of a pointer to a character string
**	wrk_blk		: address of a work block structure
**	entry		: address of an index type structure
**	include_list_ptr    : address of an include list structure
**	qualifier	: address of an integer
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/

    {
    INDEX_TYPE *tmp_entry;

    tmp_entry = init_entry(NULL);

    while ((*qualifier) == 1) {
	process_help_text(parse_level_ptr, line_ptr, wrk_blk, tmp_entry,
	    include_list_ptr, qualifier);
	entry->txt_buf_ptr = (char *) XtRealloc(entry->txt_buf_ptr,
	    entry->txt_buf_siz + tmp_entry->txt_buf_siz + 1);
	entry->txt_buf_ptr = strcat(entry->txt_buf_ptr, tmp_entry->txt_buf_ptr);
	entry->txt_buf_siz += tmp_entry->txt_buf_siz;
	wrk_blk->wrk_buf_ptr[0] = '\0';
	wrk_blk->wrk_buf_used = 0;
    }

    return(SUCCESS);
    }


static INDEX_TYPE *init_entry(key)
char *key;
/*
**++
**  Functional Description:
**	this routine allocates a index type structure and initialize it.
**
**  Keywords:
**	entry, index type, initialize, allocate
**
**  Arguments:
**	key : address of a character string
**
**  Result:
**	address of an index entry structure
**
**  Exceptions:
**	None
**--
*/


    {
    INDEX_TYPE *entry;
    int key_len;

    entry = (INDEX_TYPE *) XtMalloc(sizeof(INDEX_TYPE));

    if (entry != NULL) {
    
	entry->title.dsc_a_pointer = NULL; /* Needs to be init to NULL */
	
	entry->next = NULL;
	entry->sub_index = NULL;
	entry->qualifier_ptr = NULL;
	entry->keyword_ptr =  NULL;
	entry->include_ptr = NULL;
	entry->txt_buf_ptr = NULL;

	if (key != NULL) {	
	    key_len = strlen(key);
	    entry->full_key.dsc_a_pointer = (char *) XtMalloc(key_len + 1);
	    strcpy(entry->full_key.dsc_a_pointer, key);
	    entry->full_key.dsc_w_length = key_len;
	}
    }

    return(entry);
    }
    

static int scan_for_title(wrk_blk, line, title, next_level)
WORK_BLOCK_TYPE *wrk_blk;
char **line;
char **title;
char *next_level;
/*
**++
**  Functional Description:
**	this routine scans a topic in a module. It specifically looks for a
**	title extension.
**
**  Keywords:
**	scan, title
**
**  Arguments:
**	wrk_blk	    : address of a work block structure
**	line	    : address of a pointer to a character string
**	title	    : address if a pointer to a character string
**	next_level  : address of a character
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
		
    {
    int line_len;
    int get_rec_stat;
    
    get_rec_stat = get_record(wrk_blk, line, &line_len);
    
    /*
    **  Scan module until the next level or the end of the module
    */
    
    while ( !( ((isdigit((*line)[0]) && (*line)[1] == ' ') ||
	       ((*line)[0] == '/')) &&
	       (get_rec_stat != FIRST_SKIPED))) {
		    
	/*
	**  If a TITLE extension is encountered process the title
	*/
	
	if (case_less_str_n_equal((*line), "=TITLE", 6)) {
	    if (*title != NULL)
		XtFree(*title);
	    *title = get_title(*line, line_len);
	}
	get_rec_stat = get_record(wrk_blk, line, &line_len);
    }

    *next_level = (*line)[0];
	
    return(SUCCESS);
    }

Boolean case_less_str_n_equal(s1, s2, size)
char *s1;
char *s2;
int size;
/*
**++
**  Functional Description:
**	This routine compares two text string. It returns true if the two text  
**	strings are equal. Otherwise it returns false.			    
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    int i;
    char c1, c2;

    i = 0;

    while (size > 0) {
	if (s1[i] == '\0' && s2[i] == '\0')
	    break;

	if (s1[i] == '\0' || s2[i] == '\0')
	    return FALSE;
	    
	c1 = toupper(s1[i]);
	c2 = toupper(s2[i]);

	if (c1 != c2)
	    return FALSE;

	i++;
	size--;
    }

    return TRUE;
    }
	     

static int get_full_key(tmp_key, entry, topic_list_ptr, current_topic)
KEY_CHUNK *tmp_key;
INDEX_TYPE *entry;
INCLUDE_TYPE **topic_list_ptr;
INCLUDE_TYPE **current_topic;
/*
**++
**  Functional Description:
**	this routine builds a full key definition for the given topic
**
**  Keywords:
**	include
**
**  Arguments:
**	tmp_key	    : address of a key structure
**	entry	    : address of an entry type structure
**	topic_list_ptr	: address of a pointer to an topic list structure
**	current_ptr : address of an pointer to an include type structure
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    
    {
    INCLUDE_TYPE *topic_list;
    
    /*
    **  Insert at beginning if first element in list
    */
    
    if (*topic_list_ptr == NULL) {
	*topic_list_ptr = *current_topic = topic_list =
	    (INCLUDE_TYPE *) XtMalloc(sizeof(INCLUDE_TYPE));
    }
    else {

	/*
	**  If not the first element, insert at end
	*/
	
	topic_list = *topic_list_ptr;
	while (topic_list->next != NULL) {
	    topic_list = topic_list->next;
	}
	topic_list->next = *current_topic =
	    (INCLUDE_TYPE *) XtMalloc(sizeof(INCLUDE_TYPE));
	topic_list = topic_list->next;
    }

    /*
    **  Initialize the fields in the topic data structure
    */
    
    topic_list->title = NULL;
    topic_list->next = NULL;
    topic_list->notitle = FALSE;
    topic_list->full_key = (char *) XtMalloc(entry->full_key.dsc_w_length +
	tmp_key->key.dsc_w_length + 2);
    strcpy(topic_list->full_key, entry->full_key.dsc_a_pointer);
    strcat(topic_list->full_key, " ");
    strcat(topic_list->full_key, tmp_key->key.dsc_a_pointer);

    return(SUCCESS);
    }
    

static char *get_title(line, line_len)
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
    char *title = NULL;
        
    /*
    **  Parse the title
    */
    
    for(i=1; (line[i]!=' ' && i < line_len); i++);
    if (i+1 < line_len)
	{
	for(i = i + 1; ((line[i] != '\0') && (line[i] != '\n')) ; i++)
	count++;
	title = (char *) XtMalloc(count + 1);
	DXmBCopy(&line[i - count], title, count);
	title[count] = '\0';
	}

    return(title);
    }
    

static int skip_sublevel(wrk_blk, line_ptr, level)
WORK_BLOCK_TYPE *wrk_blk;
char **line_ptr;
char *level;
/*
**++
**  Functional Description:
**	this routine skips all the topics in a module 'below' the given level.
**
**  Keywords:
**	skip, scan
**
**  Arguments:
**	wrk_blk	    : address of a work-block structure
**	line_ptr    : address of a pointer to an character string
**	level	    : address of a character
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/

    {
    int get_rec_stat;
    int line_len;
    char skip_level;
    Boolean SKIP = TRUE;
    
    /*
    **  We want to skip all the levels 'below' the current parse level 
    */
    
    skip_level = *level;
    
    /*
    **  Scan the module until we get back to the additional topics level or
    **	higher
    */

    get_rec_stat = ~EOM;    
    while ((SKIP) && (get_rec_stat != EOM)) {
    
	get_rec_stat = get_record(wrk_blk, line_ptr, &line_len);

	/*
	**  Check if we reached another level. We are not interested in '/'
	**  because those are equivalent to the current level
	*/

	if ((isdigit((*line_ptr)[0]) && ((*line_ptr)[1] == ' ')) &&
	    (get_rec_stat != FIRST_SKIPED)) {

	    /*
	    **  Stop skipping levels if the new level is 'lower' in the
	    **	hierarchy 
	    */
	    
	    if ((*line_ptr)[0] < skip_level) {

		/*
		**  Report back the new 'higher' level reached to determine if
		**  there are more additional topics to collect
		*/
		
		*level = (*line_ptr)[0];
		SKIP = FALSE;
	    }
	    else
		get_rec_stat = get_record(wrk_blk, line_ptr, &line_len);
	}
    }

    return(SUCCESS);
    }


static int resolve_includes(context, wrk_blk, include_list)
LIBRARY_CONTEXT_BLOCK *context;
WORK_BLOCK_TYPE *wrk_blk;
INCLUDE_LIST *include_list;
/*
**++
**  Functional Description:
**	this routine determine include extension title by locating and then
**	scanning the module in which the include are located.
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
**	Any status code returned by LBR$SET_LOCATE or LBR_FIND
**
**  Exceptions:
**	None
**--
*/
    
    {
    int status;
    char *line;
    INDEX_TYPE *top_entry;
    KEY_CHUNK *module_key;

    while (find_module_to_search(context, &top_entry, include_list)) {
    
#ifdef VMS
	ERROR_CHECK(LBR$SET_LOCATE((&wrk_blk->lbr_index)));
	ERROR_CHECK(LBR$FIND(&(wrk_blk->lbr_index), &(top_entry->rfa)));
#else
	ERROR_CHECK(LBR_SET_LOCATE((&wrk_blk->lbr_index)));
	ERROR_CHECK(LBR_FIND(&(wrk_blk->lbr_index), &(top_entry->rfa)));
#endif
    
	set_module_cntxt(wrk_blk, &line);

	parse_key(top_entry->full_key.dsc_a_pointer, &module_key);
	find_include(wrk_blk, line, include_list, module_key);
	free_key(module_key);
    }
    
    return(status);
    }


static Boolean find_module_to_search(context, top_entry, include)
LIBRARY_CONTEXT_BLOCK *context;
INDEX_TYPE **top_entry;
INCLUDE_LIST *include;
/*
**++
**  Functional Description:
**	this routine parses a list of include extension and returns the module
**	which contains the include extension. 
**
**  Keywords:
**	include, scan, module
**
**  Arguments:
**	context	    : address of a library context block
**	top_entry   : address of an address to an entry block
**	include	    : address of an include list structure
**
**  Result:
**	TRUE if a module needs to be scanned to resolve include extension
**	FALSE if all the incude extensions are resolved
**
**  Exceptions:
**	None
**--
*/

    {
    Boolean found = FALSE;
    int status;

    /*
    **  Traverse the include list and look for an include to resolve
    */
    
    while ((include != NULL) && (!found)) {
	if (include->key == NULL)
	    parse_key(include->key_ptr, &include->key);
	if (include->title == NULL) {
	    found = TRUE;

	    /*
	    **  Select the module in which the include topic is to be found
	    */
	    
	    status = locate_key(include->key, context->cache_ptr, top_entry);
	    if ((status != SUCCESS) && (status != NOT_IN_MEM)) {
		include->title = include->key_ptr;
		found = FALSE;
	    }
	}
	else
	    include = include->next;
    }
    
    return(found);
    }
    

static int find_include(wrk_blk, line, include_list, module_key)
WORK_BLOCK_TYPE *wrk_blk;
char *line;
INCLUDE_LIST *include_list;
KEY_CHUNK *module_key;
/*
**++
**  Functional Description:
**	this routine parses a module to resolve the include extension list.
**
**  Keywords:
**	include
**
**  Arguments:
**	wrk_blk	    : address of a work block structure
**	line	    : address of a character string
**	include_list	: address of a include list structure
**	module_key  : address of a key structure
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
    
    {
    int line_len;
    int qualifier;
    int get_rec_stat = 1;
    int include_count = 0;
    char level;
    char next_level;
    INCLUDE_LIST *include;
    KEY_CHUNK *current_key = NULL;
    Boolean parsing_done = FALSE;
    Boolean module_done = TRUE;
    Boolean include_match = FALSE;
    Boolean first_pass = TRUE;

    /*
    **  Scan until the end of the module is reached
    */
    
    while ((get_rec_stat != EOM) && (!parsing_done)) {

	/*
	**  Check if a new level is reached
	*/
	
	if (((isdigit(line[0]) && (line[1] != '.')) ||
	    (line[0] == '/')) &&
	    (get_rec_stat != FIRST_SKIPED)) {

	    if ((line[0] == '/'))
		qualifier = 1;
	    else
	    {
		qualifier = 0;
		level = line[0];
	    }
	    
	    
	    /*
	    **  Build the full key for the new level we are at
	    */
	    
	    if (qualifier == 1)
		build_full_key(&current_key, line, level);
	    else
		build_full_key(&current_key, &line[2], level);

	    /*
	    **  Traverse the list of include keys to find a match
	    */
	    
	    include = include_list;
	    while ((include != NULL) && (!include_match)) {

		if (include->title == NULL) {

		    /*
		    **  If an empty title is found, more include to resolve
		    */
		    
		    if (include->key == NULL)
			parse_key(include->key_ptr, &include->key);

		    /*
		    **  Check if the include key belongs to the current module
		    */
			
		    if (case_less_str_equal(include->key->key.dsc_a_pointer,
			module_key->key.dsc_a_pointer)) {

			if (first_pass)
			    include_count++;
			    
			/*
			**  If the include belongs in this module, we are not
			**  done parsing this module
			*/
								    
			module_done = FALSE;

			if (key_match(include->key, current_key)) {

			    include_match = TRUE;
			    include_count--;
			    /*
			    **  If the include key matches the current key in
			    **	the module, look for its title
			    */

			    scan_for_title(wrk_blk, &line, &include->title,
				&next_level);
			    
			    /*
			    **  Default the title to the key if not found
			    */
			    
			    if (include->title == NULL) {
				include->title = include->key_ptr;
				include->notitle = TRUE;
			    }
			} 
		    }
		}
		include = include->next;
	    }
	    first_pass = FALSE;
	    
	    /*
	    **  Can we stop parsing the module?
	    */
			    
	    if (module_done) 
		parsing_done = TRUE;
	    else {

		module_done = TRUE;
		
		/*
		**  If we scanned for a title, we have the new level or the end
		**  of module
		*/
		 
		if (!include_match)
		    get_rec_stat = get_record(wrk_blk, &line, &line_len);
		else {
		    if (line[0] == '0') 
			parsing_done = TRUE;
		    include_match = FALSE;
		}
	    }
	}   /* end of if new level */
	else
	    get_rec_stat = get_record(wrk_blk, &line, &line_len);
	    
    }	/* end of while parsing */

    if (include_count > 0)
	set_include_not_found(include_list, module_key);

    free_key(current_key);
    
    return(SUCCESS);
    }

static int set_include_not_found(include_list, module_key)
INCLUDE_LIST *include_list;
KEY_CHUNK *module_key;
/*
**++
**  Functional Description:
**	this routine checks if an include extension was not found and builds an
**	error message into the title field.
**
**  Keywords:
**	include, error
**
**  Arguments:
**	include_list	: address of list of include extension structure
**	module_key	: address of a key structure
**
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
	    
    {
    int str_len;
    INCLUDE_LIST *include;

    include = include_list;

    while (include != NULL) {

	/*
	**  If the belongs to the module and the title isn't resolved, the
	**  include wasn't found
	*/

	if (case_less_str_equal(include->key->key.dsc_a_pointer,
	    module_key->key.dsc_a_pointer) && (include->title == NULL)) {

	    /*
	    **  Build the error message in the title field
	    */
	    
	    str_len = strlen(include->key_ptr);
	    include->title = (char *) XtMalloc(str_len +
		KeyNotFoundMessageSize + 1);
	    DXmBCopy(KeyNotFoundMessage, include->title,
		KeyNotFoundMessageSize);
	    strcat(include->title, include->key_ptr);
	}
	
	include = include->next;
    }    

    return(SUCCESS);
    }	    
	    

static int build_full_key(current_key, line, level_c)
KEY_CHUNK **current_key;
char *line;
char level_c;
/*
**++
**  Functional Description:
**	this routine build a key structure for the current full key
**	specification given the new key and level reached.
**
**  Keywords:
**	full key, key
**
**  Arguments:
**	current	: address of a key structure
**	line	: address of string
**	level_c	: character
**	
**  Result:
**	SUCCESS
**
**  Exceptions:
**	None
**--
*/
			    
    {
    int level;
    KEY_CHUNK *key;
    KEY_CHUNK *new_key;
    KEY_CHUNK *scan_key;
    KEY_CHUNK *prev_key;

    parse_key(line, &new_key);
    level = level_c - '0';

    /*
    **  If the current key is null, we are at the top level
    */
    
    if ((*current_key) == NULL)
	*current_key = new_key;
	
    else {
    
	scan_key = *current_key;

	/*
	**  Traverse the key to match the level
	*/
	
	prev_key = NULL;	    
	while ((level > 1) && (scan_key != NULL)) {
	    prev_key = scan_key;
	    scan_key = scan_key->next;
	    level--;
	}
	
	/*
	**  Append the new key to the end if lower level reached.  Free any
	**  lower key
	*/
	
	if (scan_key != NULL) 
	    free_key(scan_key);
	    
	prev_key->next = new_key;
    }

    return (SUCCESS);
    }
			    

static Boolean key_match(key1, key2)
KEY_CHUNK *key1;
KEY_CHUNK *key2;
/*
**++
**  Functional Description:
**	this routine checks if 2 key structures are equal.
**
**  Keywords:
**	compare, equal, key
**
**  Arguments:
**	key1	: address of a key structure
**	key2	: address of a key structure
**
**  Result:
**	TRUE if the keys match
**	FALSE if the keys don't match
**
**  Exceptions:
**	None
**--
*/

    {

    /*
    **  Run down the 2 keys
    */
    
    while ((key1 != NULL) && (key2 != NULL)) {

	/*
	**  Stop comparing the key lists if not equal
	*/
	
	if (!case_less_str_equal(key1->key.dsc_a_pointer,
	    key2->key.dsc_a_pointer)) {
	    return(FALSE);
	}
	
	key1 = key1->next;
	key2 = key2->next; 
    }

    if ((key1 == NULL) && (key2 == NULL))
	return(TRUE);
    else
	return(FALSE);
    }
    

static int format_sub_topic(topic_list, include_list, topic_arr, title_arr,
    count)
INCLUDE_TYPE *topic_list;
INCLUDE_LIST *include_list;
char ***topic_arr;
char ***title_arr;
int *count;
/*
**++
**  Functional Description:
**	this routine allocates the topic and title arrays and fills them in with
**	compound strings.
**
**  Keywords:
**	allocate, additional topics
**
**  Arguments:
**	topic_list	: address of a topic list structure
**	include_list	: address of a include list structure
**	topic_array	: address of an address to an array of string address
**	title_array	: address of an address to an array of string address
**	count		: size of the arrays
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
    int topic_cnt = 0;
    int incl_cnt = 0;
    int str_len;
    INCLUDE_TYPE *topic;
    INCLUDE_LIST *incl;
#ifdef GENERAL_TEXT
    long countn, stat;
#endif /* GENERAL_TEXT */

    /*
    **  Count the number of sub-topics
    */

    topic = topic_list;
    while (topic != NULL) {
	topic_cnt++;
	topic = topic->next;
    }

    /*
    **  Count the number of includes
    */

    incl = include_list;
    while (incl != NULL) {
	incl_cnt++;
	incl = incl->next;
    }

    /*
    ** allocate the space for the arrays
    */
    
    *topic_arr = (char **) XtMalloc(sizeof(char *) * (topic_cnt + incl_cnt));
    *title_arr = (char **) XtMalloc(sizeof(char *) * (topic_cnt + incl_cnt));
    *count = topic_cnt + incl_cnt;

    /*
    ** fill in the sub-topic array with the real sub-topics
    */

    topic = topic_list;
    for (i = 0; i < topic_cnt; i++) {
    
#ifdef GENERAL_TEXT
	(*topic_arr)[i] = (char *)DXmCvtFCtoCS(topic->full_key, &countn, &stat);
	(*title_arr)[i] = (char *)DXmCvtFCtoCS(topic->title, &countn, &stat);
#else
	(*topic_arr)[i] = (char *)XmStringLtoRCreate(topic->full_key, "ISO8859-1");
	(*title_arr)[i] = (char *)XmStringLtoRCreate(topic->title, "ISO8859-1");
#endif /* GENERAL_TEXT */
	
	topic = topic->next;
    }

    /*
    ** fill in the sub-topic array with the include entries
    */

    incl = include_list;
    for (i; i < (topic_cnt + incl_cnt); i++) {
    
#ifdef GENERAL_TEXT
	(*topic_arr)[i] = (char *)DXmCvtFCtoCS(incl->key_ptr, &countn, &stat);
	(*title_arr)[i] = (char *)DXmCvtFCtoCS(incl->title, &countn, &stat);
#else
	(*topic_arr)[i] = (char *)XmStringLtoRCreate(incl->key_ptr, "ISO8859-1");
	(*title_arr)[i] = (char *)XmStringLtoRCreate(incl->title, "ISO8859-1");
#endif /* GENERAL_TEXT */
			    
	incl = incl->next;
    }    

    return(SUCCESS);
    }


int DXmHelp__free_frame(title, text, topic_array, title_array, size,
    caching)
char *title;
char *text;
char **topic_array;
char **title_array;
int size;
Boolean caching;
/*
**++
**  Functional Description:
**	this routine frees a help frame return by DXmHelp__get_frame
**
**  Keywords:
**	free, frame
**
**  Arguments:
**	title	: address of the title string to be freed
**	text	: address of the text string to be freed
**	topic_array : address of a string address array to be freed
**	title_array : address of a string address array to be freed
**	size	: size of the arrays
**	caching : boolean specifying the caching mode for the Help Widget
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

    if (title != NULL)
	XtFree(title);
	
    if (text != NULL)
	XtFree(text);

    DXmHelp__free_array(topic_array, title_array, caching, size);

    return(SUCCESS);
    }	
