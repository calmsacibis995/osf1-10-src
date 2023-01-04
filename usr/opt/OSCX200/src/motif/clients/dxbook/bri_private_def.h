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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BRI_PRIVATE_DEF.H*/
/* *12   22-MAR-1993 14:31:42 BALLENGER "Fix crash when directory target is not valid chunk."*/
/* *11   30-OCT-1992 18:32:43 BALLENGER "Change definition of DEFAULT_DIRECTORY for VMS."*/
/* *10   13-AUG-1992 15:11:06 GOSSELIN "updating with necessary A/OSF changes"*/
/* *9    12-AUG-1992 14:43:18 ROSE "Added ISO extensions and error messages"*/
/* *8    24-JUL-1992 12:27:28 KLUM "repress print if NO_PRINT alt-prod-name"*/
/* *7    13-JUL-1992 15:53:58 FITZELL "mswindows conditionals for pC port"*/
/* *6    19-JUN-1992 20:13:24 BALLENGER "Cleanup for Alpha/OSF port"*/
/* *5     9-JUN-1992 09:32:50 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *4    21-APR-1992 16:57:54 ROSE "Done"*/
/* *3    21-APR-1992 15:30:52 ROSE "Added two new codes to handle opening libraries and books with ISO extensions, and moved in*/
/*#defines so more than one module can access them"*/
/* *2     3-MAR-1992 17:11:18 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:48:13 PARMENTER "BRI private definitions"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BRI_PRIVATE_DEF.H*/
/* DEC/CMS REPLACEMENT HISTORY, Element BRI_PRIVATE_DEF.H*/
/* *6     6-APR-1991 19:33:13 BALLENGER "Use correct lmf calls to allow third party licenses."*/
/* *5     3-APR-1991 11:47:19 FITZELL "added blank_dir_entry filed to BRI_BOOK_BLOCK"*/
/* *4    27-MAR-1991 18:24:41 FITZELL "addedd BriErrDirEntryNoText error code"*/
/* *3    25-JAN-1991 16:50:10 FITZELL "V3_EFT_24_JAN"*/
/* *2    12-DEC-1990 11:38:02 FITZELL "V3 ift update snapshot"*/
/* *1     8-NOV-1990 11:26:44 FITZELL "V3 IFT"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BRI_PRIVATE_DEF.H*/
/****************************************************************************
 *                                                                          *
 *  (C) DIGITAL EQUIPMENT CORPORATION 1988, 1989                            *
 *                                                                          *
 *      This is an unpublished work which was created in the indicated      *
 *      year, which contains confidential and secret information,  and      *
 *      which is protected under the copyright laws.  The existence of      *
 *      the copyright notice is not to be construed as an admission or      *
 *      presumption that publication has occurred. Reverse engineering      *
 *      and unauthorized copying is strictly prohibited.   All  rights      *
 *      reserved.                                                           *
 *                                                                          *
 ****************************************************************************
 *
 *
 * Facility:
 *
 *      bri_private_def.h
 *
 * Abstract:
 *
 *      Private BRI (Book-Reader Interface) definitions
 *
 * Author:
 *
 *      David L. Ballenger
 *
 * Date:
 *
 *      Tue May 23 13:52:08 1989
 *
 * Revision History:
 *
 *      04-Apr-1991 David L Ballenger
 *                  Add LMF related error condition definitions.
 *
 *	 1-Apr-1991 Michael J Fitzell
 *		    Add Blank_dir_entry field to BRI_BOOK_BLOCK so
 *		    only one message box is popped per book.
 * 
 *      29-Aug-1990 David L Ballenger
 *                  Rename BRI_PAGE_INDEX_ENTRY typedef to 
 *                  BODS_PAGE_INDEX_ENTRY, and move to bods_private_def.h
 *
 *                  Merge BRI_FILE_SPEC with BRI_CONTEXT and conditionalize
 *                  definition of BRI_CONTEXT to allow use of RMS on VMS
 *                  and stdio on ULTRIX.
 * 
 *  	30-Jul-1990 James A. Ferguson
 *  	    	    Modified BRI_DIRECTORY_BLOCK to include directory
 *  	    	    object id field.  This new field is returned as the 
 *  	    	    directory id instead of the drb which was previously
 *  	    	    returned by several routines.  BRI_DIRECTORY_ENTRY
 *  	    	    now returns an directory entry object id which is 
 *  	    	    built using the directory object id plus the
 *  	    	    entry number.
 *
 *      12-Sep-1989 David L Ballenger
 *
 *                  Add Bookreader V2 changes.
 *
 *      16-Jan-1990 David L Ballenger
 *                  Add latent support for titles and comments in bookshelf
 *                  files.
 *
 *      30-May-1990 David L Ballenger
 *                  Move memory management macros here from bri-mem.c
 */


#ifndef _BRI_PRIVATE_DEF_H
#define _BRI_PRIVATE_DEF_H

/* Includes */

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#ifdef _STDC_
#include <stddef.h>
#endif 

#ifdef vms
#include <rms.h>
#ifndef PATH_MAX
#define PATH_MAX NAM$C_MAXRSS
#endif
#include <types.h>
#include <stat.h>
#else 
#include <sys/types.h>
#include <sys/stat.h>
#endif


#ifndef MSWINDOWS
#include <X11/Intrinsic.h>

#include "br_common_defs.h"
#include "br_meta_data.h"
#include "br_prototype.h"
#include "bods_private_def.h"
#include "bxi_private_def.h"
#else
#define PATH_MAX 255
#include <heap.h>
#include "bmtcdefs.h"
#include "bmtmetad.h"
#include "bmtdpdef.h"
#include "bmtxpdef.h"
#endif

/* Defines */
#define BROWSER_PRODUCT_NAME "BOOKBROWSER"
#ifdef VMS
#define DEFAULT_DIRECTORY		"DECW$BOOK_SEARCHLIST:"
#define CURRENT_DIRECTORY		"[]"
#define DECW_DEFAULT_SHELF_EXT		"*.DECW$BOOKSHELF"
#define DECW_DEFAULT_BOOK_EXT		"*.DECW$BOOK"
#define ISO9960_SHELF_EXT		"*.BKS"
#define ISO9960_BOOK_EXT		"*.BKB"
#define NFS_SHELF_EXT			"*.DECW_BOOKSHELF"
#define NFS_BOOK_EXT			"*.DECW_BOOK"
#else
#ifdef MSWINDOWS
#define DEFAULT_DIRECTORY               "C:\BOOKS"
#else
#define DEFAULT_DIRECTORY		"/usr/lib/dxbook"
#define ULTRIX_SEARCH_LIST_NAME		"DECW_BOOK"
#endif
#define CURRENT_DIRECTORY		"."
#define DECW_DEFAULT_SHELF_EXT		".decw_bookshelf"
#define DECW_DEFAULT_BOOK_EXT		".decw_book"
#define ISO9960_SHELF_EXT		".bks"
#define ISO9960_BOOK_EXT		".bkb"
#define NFS_SHELF_EXT			".decw$bookshelf"
#define NFS_BOOK_EXT			".decw$book"
#endif

/* Definitions for file parsing */
#define PERIOD		'.'
#define RIGHT_BRACKET	']'
#define RIGHT_SQUIGLLY	'}'
#define NULL_CHAR	'\0'



/* Define BRI error codes and messages
 */
/* Codes passed via BriLongJmp
 */
#define BriErrBadPageIndexNum   1
#define BriErrBadVersionNum     2
#define BriErrRecUnpackNum      3
#define BriErrBookPosNum        4
#define BriErrPageReadNum       5
#define BriErrDirReadNum        6
#define BriErrInvalidDridNum    7
#define BriErrFontReadNum       8
#define BriErrChkStrReadNum     9
#define BriErrBadPageIdNum     10
#define BriErrFileSearchNum    11
#define BriErrNoEnvNum         12
#define BriErrNoMemoryNum      13
#define BriErrInvalidMemNum    14
#define BriErrShelfReadNum     15
#define BriErrBadShelfEntryNum 16
#define BriErrFieldReadNum     17
#define BriErrBadPgTypeNum     18
#define BriErrBadShelfIdNum    19
#define BriErrBadBookIdNum     20
#define BriErrLmfNoLicenseNum   21
#define BriErrUnknownFileNum    22
#define BriErrFileOpenNum       23
#define BriErrLmfNotStartedNum  24
#define BriErrLmfInvalidDateNum 25
#define BriErrLmfTerminatedNum  26
#define BriErrLmfInvalidVersNum 27
#define BriErrLmfInvalidHwIdNum 28
#define BriErrLmfBadParamNum    29
#define BriErrLmfExEnqlmNum     30
#define BriErrLmfInvalidLicNum  31
#define BriErrFileOpenAnyNum	32

/* Codes passed directly to BriError
 */
#define BriErrShelfOpenNum    1
#define BriErrBookOpenNum     2
#define BriErrDirOpenNum      3
#define BriErrPageOpenNum     4
#define BriErrBadChkIdNum     5
#define BriErrChkSymNum       6
#define BriErrPageCloseNum    7
#define BriErrBookCloseNum    8
#define BriErrNoLmfNum        9
#define BriErrLibraryOpenNum 11
#define BriErrDirEntryNoText 12
#define BriErrLibraryOpenAnyNum 13
#define BriErrBadDirTargetNum   14



#define BriCalloc(type,cnt,env) (type *)BriMalloc(sizeof(type)*(cnt),env)

/* BriAlign
 *
 * Macro to align data appropriately.  On the mips machines this calls
 * a routine to allocate a properly aligned buffer of the appropriate
 * size, and copy the data at the give address to it.  The address of
 * the new buffer is returned.  On the VAX where the alignment is not
 * as critical, no new buffer is allocated and the given address is
 * simply returned.
 */
#if defined(mips)  || defined(ALPHA) || defined(MSWINDOWS)
#define BriAlign(type,addr,cnt,env) (type *)BriDataAlign(addr,sizeof(type)*(cnt),env)
#else 
#define BriAlign(type,addr,cnt,env) (type *)(addr)
#endif


/* memory management macros
*/

#ifdef vms
# define BXI_MALLOC(size) lib$vm_malloc((size))

# define BXI_FREE(ptr)\
	lib$vm_free((ptr));\
	(ptr) = NULL

# define BXI_CALLOC(num_elem, elem_size) lib$vm_calloc((num_elem), (elem_size))

# define BXI_CFREE(ptr)\
	lib$vm_free((ptr));\
	(ptr) = NULL

# define BXI_REALLOC(ptr, size) lib$vm_realloc((ptr), (size))
#endif

#ifdef MSWINDOWS
# define BXI_MALLOC(size) HeapMalloc((size))

# define BXI_FREE(ptr) HeapFree((ptr)); (ptr) = NULL

# define BXI_CALLOC(num_elem, elem_size) HeapCalloc((num_elem), (elem_size))

# define BXI_CFREE(ptr) HeapFree((ptr)); (ptr) = NULL

# define BXI_REALLOC(ptr, size) HeapRealloc((ptr), (size))
#endif

#if !defined(VMS) && !defined(MSWINDOWS)
# define BXI_MALLOC(size) malloc((size))

# define BXI_FREE(ptr) free((ptr)); (ptr) = NULL

# define BXI_CALLOC(num_elem, elem_size) calloc((num_elem), (elem_size))

# define BXI_CFREE(ptr) free((ptr)); (ptr) = NULL

# define BXI_REALLOC(ptr, size) realloc((ptr), (size))

#endif 


/*
 *  Convenience macros
 */
#define BriBookPtr(bkid) (((BRI_CONTEXT *)(bkid))->data.book)
#define BriShelfPtr(shelf_id) (((BRI_CONTEXT *)(shelf_id))->data.shelf)

#define BriBuildDirectoryEntryId(drb_ptr,entry_number) 	    \
    ( (BMD_OBJECT_ID)	    	    	    	    	    \
    	( ((drb_ptr)->directory_object_id & 0xFF000000)     \
    	    | ((entry_number) & 0x00FFFFFF) 	    	    \
    	)   	    	    	    	    	    	    \
    )

    /* Store the directory object id in the high order byte */
#define BriStoreDirObjectId(drb_ptr,obj_id) \
    	    (drb_ptr)->directory_object_id = ((obj_id) << 24)

#define BriValidateDrbIndex(env_ptr,index_value)    	    	    	\
 {	    	    	    	    	    	    	    	    	\
    if (((index_value) < 0) 	    	    	    	    	    	\
    	 || ((index_value) >= (env_ptr)->data.book->vbh.n_directories)) \
    { 	    	    	    	    	    	    	    	    	\
    	BriLongJmp((env_ptr),BriErrInvalidDridNum);   	    	    	\
    }	    	    	    	    	    	    	    	    	\
 }



#ifdef vms
typedef struct _VMS_ITEM_LIST {
    BR_UINT_16  length; 	/*  buffer length  */
    BR_UINT_16  itemcode;	/*  symbolic item code  */
    char        *pointer;	/*  buffer address  */
    BR_UINT_16  *ret_len;	/*  length returned by $TRNLNM  */
} VMS_ITEM_LIST ;

#endif 

typedef struct _BRI_BUFFER_DESC {
    BMD_GENERIC_PTR address;
    BR_UINT_32 length;
} BRI_BUFFER_DESC ;

typedef struct _BRI_STRING_LIST {
    char *string;
    struct _BRI_STRING_LIST *next;
} BRI_STRING_LIST ;

#ifndef vms
typedef enum { absolute, relative, complete } BRI_FILE_SEARCH_STATE ;
#endif





/* Page Map Entry
 */
typedef struct _BRI_PAGE_MAP_ENTRY {
    union {
        BR_UINT_8 all_flags;
        struct {
            unsigned dirty_page ;
            unsigned ok_to_close ;
            unsigned fill ;
        } bits ;
    } flags ;
    BR_INT_32 n_users;
    BODS_TAG tag;
    BODS_PAGE_HEADER *data_page;
    BR_UINT_32 index;
    BR_UINT_32 record_start;
    BR_UINT_8 buffer[1];
} BRI_PAGE_MAP_ENTRY ; 


/* Directory Block 
 */

typedef struct _BRI_DIRECTORY_BLOCK {
    BXI_QUEUE queue;
    BMD_OBJECT_ID directory_object_id;
    BMD_BOOK_ID *book;
    BODS_DIRECTORY_HEADER vdh;
    BODS_DIRECTORY_ENTRY *entry_list;
} BRI_DIRECTORY_BLOCK ;


/* Run time Symbol table  */

#define	BRI_SYMBOL_HASH_BITS 10
#define	BRI_SYMBOL_HASH_SIZE (1 << BRI_SYMBOL_HASH_BITS)
#define	BRI_SYMBOL_HASH_MASK (BRI_SYMBOL_HASH_SIZE - 1)
#define BriHash(object_id)  ((BR_UINT_32) (object_id) & BRI_SYMBOL_HASH_MASK)

typedef struct _BRI_SYMBOL_ENTRY {
    struct _BRI_SYMBOL_ENTRY	*next;
    BMD_OBJECT_ID		id;
    char			*name;
    }       BRI_SYMBOL_ENTRY;

/*  symbolic external reference table  */
typedef struct _BRI_SYMBOL_EXREF_ENTRY {
    char    *book_name;	    /*  symbolic name of book  */
    char    *object_name;   /*  symbolic name of object in book  */
    }       BRI_SYMBOL_EXREF_ENTRY;


/* Book Block 
 */

typedef struct _BRI_BOOK_BLOCK {
    BXI_QUEUE book_queue;
    BR_UINT_32 n_pages ;
    BODS_PAGE_INDEX_ENTRY *page_index;
    BRI_PAGE_MAP_ENTRY **page_map;
    BMD_OBJECT_ID *chunk_index;
    char ** chunk_titles;
    BRI_SYMBOL_ENTRY ** symbol_table;
    BRI_SYMBOL_EXREF_ENTRY * symbol_exref_table;
    BODS_STATIC_EXREF_BOOK ** static_exref_booklist;
    BODS_STATIC_EXREF_TARGET * static_exref_table;
    BR_UINT_32 num_static_exref_books;
    BR_UINT_32 num_static_exrefs;
    BR_UINT_32 blank_dir_entry;
    BRI_BUFFER_DESC directory_index;
    char **font_list;
    BRI_DIRECTORY_BLOCK **drb_list;
    BRI_STRING_LIST *alternate_product_names;
    char *partnum;
    char *toolsid;
    char *copyright_corp;
    char *copyright_date;
    char *copyright_text;
    char *language;
    char *bookname;
    BODS_BOOK_HEADER vbh;
} BRI_BOOK_BLOCK ;


/*  Shelf Entry Item
 */

typedef struct _BRI_SHELF_ENTRY_ITEM {
    BMD_ENTRY_TYPE entry_type;
    char *target_file;
    char *title;
    char *home_directory ;
    char symbolic_name[32];
} BRI_SHELF_ENTRY_ITEM ;


/*  Shelf Block
 */

typedef struct _BRI_SHELF_BLOCK {
    struct  _BRI_SHELF_BLOCK *next_shelf;
    BRI_SHELF_ENTRY_ITEM *shelf_entries;
    BR_UINT_32 n_entries;
    BRI_BUFFER_DESC symbol_table;
    char **font_list;
} BRI_SHELF_BLOCK;


/* Define pool descriptor data structure
 */

typedef struct _BRI_MEMORY_POOL {
    BR_INT_32  n_slots;
    BMD_GENERIC_PTR *buffer_slots;
} BRI_MEMORY_POOL ;

typedef struct _BRI_CONTEXT {
    BRI_SHELF_ENTRY_ITEM entry;
    union {
        BRI_BOOK_BLOCK *book;
        BRI_SHELF_BLOCK *shelf;
    } data ;
    BRI_MEMORY_POOL pool;
    BR_UINT_32 file_open;
    BR_UINT_32 no_print;
#ifdef vms
    struct FAB fab;
    struct NAM nam;
    struct RAB rab;
    char default_file_spec[PATH_MAX+1] ;
    char parsed_file_spec[PATH_MAX+1];
#else
    FILE *file;
    BRI_FILE_SEARCH_STATE search_state;
    char *target_file;
    char *file_extension;
    char *search_list;
#endif
    char found_file_spec[PATH_MAX+1] ;
    char reason[256];
    jmp_buf jump_buffer;
} BRI_CONTEXT;   


/* File handling routines.
 */
extern void BriFileOpen();
extern void BriFileParse();
extern BR_INT_32 BriFileSearch();

/* Error Handling routines
 */
extern void BriError();
extern void BriLongJmp();

/* Book handling routines
 */
extern void BriBookGetHeader();
extern void BriBookLmfCheck();
extern void BriBookGetPageIndexAndMap();
extern void BriBookGetChunkIndex();
extern void BriBookGetSymbolTable();
extern void BriBookGetChunkTitles();
extern void BriBookGetFontDefinitions();

/* Public memory management routines */
extern void *BriRealloc();
extern void *BriMalloc();
extern void BriStringAlloc();
extern void BriContextDelete();
extern void BriContextInherit();
extern void BriContextNew();
extern void BriFree();
extern void *BriDataAlign();

extern char *FtextToAscii PROTOTYPE((BRI_CONTEXT *context,
                                    BR_UINT_8 *data_addr,/*  address of ftext chunk  */
                                    BR_UINT_32 data_len	/*  length of data  */
                                    ));

#endif /* _BRI_PRIVATE_DEF_H */

/* DONT ADD STUFF AFTER THIS #endif */

