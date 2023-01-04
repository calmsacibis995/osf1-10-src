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
**++
**  Subsystem:
**	DXmHelp
**
**  Version: V1.0
**
**  Abstract:
**	this file defines all the data types for handling Help Frames in memory.
**	The file DESCRIP.H should be included before this one.
**
**  Keywords:
**	None
**
**  Environment:
**	Global definition
**
**  Author:
**	Andre Pavanello, CASEE Group, MEMEX Project
**
**  Creation Date: 21-Dec-87
**
**  Modification History:
**
**		Rich					    23-Jul-90
**	    Remove DXm/ from private include file includes
**
**		Rich Reichert				    12-Jul-90
**	    Remove the non-VMS definition of DEFAULT_SPEC
**
**		Rich Reichert				    06-Jun-90
**	    Make slot in LIBRARY_CONTEXT_BLOCK for help widget id.
**
** 		Leo					    02-Aug-89
**	    Q&D port to Motif
**  
**  V2-bl4-1	Andre Pavanello				    14-Jun-89
**	    Add the 'last' field in the include data block
**
**  BL8.2	Andre Pavanello				    13-May-88
**	    Add the no search extension
**
**  X004-0-1	Andre Pavanello				    8-Apr-88
**	    Add new status code
**
**  X003-0-9	Andre Pavanello				    21-Mar-88
**	    Add data structure for get_keyword support
**
**  X003-0-8	Andre Pavanello				    16-Mar-88
**	    Add data structure for search support
**
**  X003-0-7	Andre Pavanello				    10-Mar-88
**	    Fix symbols redifinition problems for DWT builds
**
**  X003-0-6	Andre Pavanello				    7-Mar-88
**	    Fix up include file name for Ultrix build
**
**  X003-0-5	Andre Pavanello				    26-Feb-88
**	    Remove $DECRIPTOR_D macro declaration
**
**  X003-0-4	Andre Pavanello				    25-Feb-88
**	    Define RMS and LBR$ error messages in Ultrix environment
**	    also
**
**  X003-0-3	Andre Pavanello				    25-Feb-88
**	    Add definition for Ultrix environment
**
**  X003-0-2	Andre Pavanello				    18-Feb-88
**	    Remove time fields from index entry structures.
**
**  X003-0-1	Andre Pavanello				    9-Feb-88
**	    Modify and add new data structures for global open support.
**--
*/
#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif


/*
**  Include files
*/
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "DXmPrivate.h"
#include "Help_Descrip.h"
#include <string.h>
#include <ctype.h>

/*
**  Macro Definitions
*/
#ifndef TRUE
#define TRUE        (1==1)
#define FALSE       (1==0)
#endif

#ifndef NULL
#define NULL	    0
#endif

#ifndef SUCCESS
#define SUCCESS     TRUE
#endif

#define NOT_IN_MEM   0
#define NOT_FND     -1
#define KEY_NOT_FND  0
#define NULL_CONTEXT 0
#define FIRST_SKIPED 2
#define EOM	     3
#define ERROR_CHECK(f) if (((status=(f)) & 1) == 0) return(status);

#ifdef VMS		/* VMS only */

#define LBR$_INVRFA   2527322      /* error code returned by LBR$FIND */
#define RMS$_EOF      0x1827A      /* error code returned by LBR$GET_RECORD */
#define LOCAL_SPEC    "[].HLB"
#define DEFAULT_SPEC  "SYS$HELP:.HLB"
#define SET_AST       1
#define CLR_AST       0
#define SS$_WASSET    9
#define AST_OFF(ast_status) (ast_status = SYS$SETAST(CLR_AST))
#define AST_ON(ast_status) if (ast_status == SS$_WASSET) SYS$SETAST(SET_AST)

#else			/* Ultrix and other only */

#define LBR__INVRFA   FALSE
#define RMS__EOF      FALSE
#define LOCAL_SPEC    ""
/* #define DEFAULT_SPEC  "/usr/lib/X11/help/" */
#include <signal.h>
#ifdef UNIX_SYSV
static int *g_sigvals[NSIG];

#define AST_OFF(ast_status) \
       { register int i; \
	 for (i = 12; i <= 18; i++)  \
	    g_sigvals[i] = (int *) sigset(i, SIG_HOLD); \
       }

#define AST_ON(ast_status) \
       { register int i; \
	 for (i = 12; i <= 18; i++)  \
	    sigset(i, g_sigvals[i]); \
       }

#else
#define MASK \
 sigmask(SIGSYS) || sigmask(SIGPIPE) || sigmask(SIGALRM) || sigmask(SIGTERM) ||\
 sigmask(SIGURG) || sigmask(SIGSTOP) || sigmask(SIGTSTP) || sigmask(SIGCHLD) ||\
 sigmask(SIGTTIN) || sigmask(SIGTTOU) || sigmask(SIGIO) || sigmask(SIGXCPU) || \
 sigmask(SIGXFSZ) || sigmask(SIGVTALRM) || sigmask(SIGPROF) || \
 sigmask(SIGUSR1) || sigmask(SIGUSR2)
#define AST_OFF(ast_status) (ast_status = sigsetmask(MASK))
#define AST_ON(ast_status) (sigsetmask(ast_status))
#endif

#endif
/*
**  Type Definitions
*/
typedef struct dsc_descriptor DSC_DESCRIPTOR;
	    
typedef struct _rfa_type   /* simulates a quadword */
	    {
	    int rfa_high,
	        rfa_low;
	    } RFA_TYPE;
	    
typedef struct _key_chunk 
	    {
	    DSC_DESCRIPTOR    key;
	    struct _key_chunk *next;
	    } KEY_CHUNK, *KEY_CHUNK_PTR;

typedef struct _include_type
	    {
	    char		 *title,
				 *full_key;
	    Boolean		 notitle;
	    struct _include_type *next;
	    } INCLUDE_TYPE, *INCLUDE_TYPE_PTR;

typedef struct _index_type         /* information for each key in the library*/
	    {
	    RFA_TYPE               rfa;
	    int                    in_memory,
	                           txt_buf_siz;
	    Boolean		   nosearch;
	    DSC_DESCRIPTOR         keyname,
	                           title,
	                           full_key;
            char                   *txt_buf_ptr,
	                           *qualifier_ptr;
	    KEY_CHUNK_PTR          keyword_ptr;
	    INCLUDE_TYPE_PTR       include_ptr;
	    struct _index_type     *next,
	                           *sub_index;
	    } INDEX_TYPE, *INDEX_TYPE_PTR;

typedef struct _include_list
	    {
	    char		 *key_ptr;
	    char		 *title;
	    Boolean		 notitle;
	    INDEX_TYPE_PTR	 parent_ptr;
	    KEY_CHUNK		 *key;
	    struct _include_list *next;
	    struct _include_list *last;
	    } INCLUDE_LIST, *INCLUDE_LIST_PTR;

typedef struct _library_context_block
            {
	    DSC_DESCRIPTOR		    lib_spec;
	    int				    lib_count,
	                                    lib_open,	/* lib open flag    */
					    open_cnt,	/* open cnt for the */
							/* given lib	    */
					    new_cache,
	           			    caching,	/* caching mode	    */
					    search_mode;
	    void *			    lbr_index; /* opaque ptr */
	    INDEX_TYPE			    *cache_ptr;
	    DXmHelpWidget		    help_widget_id;
	    struct _library_context_block   *next;
	    } LIBRARY_CONTEXT_BLOCK, *LIBRARY_CONTEXT_BLOCK_PTR;

typedef struct _work_block_type
            {
	    void *         lbr_index;  /* opaque ptr */
	    int            wrk_buf_siz,
	                   wrk_buf_used;
	    char           *wrk_buf_ptr;
	    } WORK_BLOCK_TYPE, *WORK_BLOCK_TYPE_PTR;

typedef struct _entry_list_type
	    {
	    int			    cnt;
	    INDEX_TYPE		    *matched_entry;
	    struct _entry_list_type *next;
	    } ENTRY_LIST_TYPE, *ENTRY_LIST_TYPE_PTR;

typedef struct _keyword_list_type
	    {
	    KEY_CHUNK_PTR	      keyw_ptr;
	    struct _keyword_list_type *next;
	    } KEYWORD_LIST_TYPE, *KEYWORD_LIST_TYPE_PTR;

/*
**  Global data definition
*/
static char KeyNotFoundMessage[] = "** Key not found **  ";
static int KeyNotFoundMessageSize = sizeof(KeyNotFoundMessage);

/*
**  Global routine declarations
*/

/*  Help_hlb_init module */

int DXmHelp_open_lib();
int DXmHelp_collect_lib_index();
int DXmHelp__init_help();
int free_index();
int free_include();
int DXmHelp__close_help();
int DXmHelp__set_cache_mode();

/*  help_hlb_get module */

Boolean	   case_less_str_equal();
Boolean	   case_less_str_n_equal();
int upcase_str();
int free_upcase_str();
int parse_key();
int free_key();
int locate_key();
int DXmHelp_set_read_context();
int DXmHelp__get_frame();
int DXmHelp__free_array();
int DXmHelp__free_frame();

/* Help_hlb_ext module */

int parse_extension();
int parse_title();
int fix_up_include();
int free_include_list();
int DXmHelp__search_title();
int DXmHelp__search_keyword();
int DXmHelp__get_keyword();
int read_in_library();
char *CopyStrN();
char *CopyStr();

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
