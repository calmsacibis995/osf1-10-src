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
/* DEC/CMS REPLACEMENT HISTORY, Element BODS_PRIVATE_DEF.H*/
/* *5    23-JUL-1992 10:32:47 GOSSELIN "added minor verson num flag"*/
/* *4    19-JUN-1992 20:13:12 BALLENGER "Cleanup for Alpha/OSF port"*/
/* *3     3-MAR-1992 17:09:00 KARDON "UCXed"*/
/* *2     2-JAN-1992 10:00:15 FITZELL "Xbook references in OAF level B"*/
/* *1    16-SEP-1991 12:48:07 PARMENTER "Book On Disk Structures"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BODS_PRIVATE_DEF.H*/
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
 *      bods_private_def.h
 *
 * Abstract:
 *
 *      Private BODS (Book On-Disk Structure) definitions
 *
 * Author:
 *
 *  	David L. Ballenger
 *
 * Date:
 *
 *      Tue May 23 13:52:08 1989
 *
 * Revision History:
 *
 *	06-Feb-1991 David L Ballenger
 *		    Fix "illegal zero sized structure member" warnings.
 *
 *      29-Aug-1990 David L Ballenger
 *                  Rename BRI_PAGE_INDEX_ENTRY typedef to 
 *                  BODS_PAGE_INDEX_ENTRY, and move to bods_private_def.h
 *
 *  	30-Jul-1990 James A. Ferguson
 *  	    	    Create new module and separate on-disk from memory 
 *  	    	    definitions.
 *
 */


#ifndef _BODS_PRIVATE_DEF_H
#define _BODS_PRIVATE_DEF_H


/* Page type tags.
 */
#define BODS_C_BOOK_HEADER 1
#define BODS_C_BOOK_HEADER_EXTENSION 2
#define BODS_C_PAGE 3
#define BODS_C_DIRECTORY_PAGE 4
#define BODS_C_PAGE_INDEX 5
#define BODS_C_CHUNK_INDEX 6
#define BODS_C_CHUNK_TITLES 7
#define BODS_C_SYMBOL_TABLE 8
#define BODS_C_FONT_DEFINITIONS 9
#define BODS_C_PAGE_FRAGMENT 10
#define BODS_C_LAST_FRAGMENT 11

/* Control page tags
 */
#define BODS_C_DIRECTORY_HEADER 12
#define BODS_C_SYMBOL 13
#define BODS_C_FONT 14
#define BODS_C_TITLE 15
#define BODS_C_DIR_ENTRY 16

/* Data page tags.
 */
#define BODS_C_DATA 17
#define BODS_C_DATA_CHUNK 18
#define BODS_C_DATA_SUBCHUNK 19
#define BODS_C_REFERENCE_RECT 20
#define BODS_C_REFERENCE_POLY 21
#define BODS_C_EXTENSION_RECT 22
#define BODS_C_EXTENSION_POLY 23

/* More tags that occur in control pages.
 */
#define BODS_C_ALTERNATE_PRODUCT 24
#define BODS_C_PARTNUM 25
#define BODS_C_TOOLSID 26
#define BODS_C_COPYRIGHT_CORP 27
#define BODS_C_COPYRIGHT_DATE 28
#define BODS_C_COPYRIGHT_TEXT 29
#define BODS_C_LANGUAGE 30
#define BODS_C_BOOKNAME 31
#define BODS_C_SYMBOL_EXREF_PAGE 32
#define BODS_C_DIRECTORY_HEADER_PAGE 33
#define BODS_C_STATIC_EXREF_PAGE 34



/* Minimum maximum tag values
 */
#define BODS_C_MIN_TAG 1
#define BODS_C_MIN_PAGE_TAG BODS_C_DATA
#define BODS_C_MAX_PAGE_TAG BODS_C_EXTENSION_POLY
#define BODS_C_MAX_TAG BODS_C_STATIC_EXREF_PAGE

#define BODS_C_MAX_RECORD_SIZE 32254
#define BODS_C_FRAGMENT_SIZE   32254
#define BODS_C_FIRST_FRAGMENT  32244
#define BODS_C_FRAGMENT_TAIL   10

/*  system directory ids  */

#define	BODS_C_CHUNK_DIR_NUM 0
#define BODS_C_TOPIC_DIR_NUM 1
#define BODS_C_SYMBOL_EXREF_DIR_NUM 2
#define BODS_C_STATIC_EXREF_DIR_NUM 3
/*  The following literal is valid through format V3.0  */
#define BODS_C_FIRST_BOOK_DIR_NUM 4

#define BODS_CHUNK_DIR_ID   (BODS_C_CHUNK_DIR_NUM << 24)
#define BODS_TOPIC_DIR_ID   (BODS_C_TOPIC_DIR_NUM << 24)
#define BODS_SYMBOL_EXREF_DIR_ID   (BODS_C_SYMBOL_EXREF_DIR_NUM << 24)
#define BODS_STATIC_EXREF_DIR_ID   (BODS_C_STATIC_EXREF_DIR_NUM << 24)


/* typedef's */

typedef struct _BODS_TAG {
    BR_UINT_16 rec_type;
    BR_UINT_32 rec_length;
} BODS_TAG ;

#define BODS_C_TAG_PACKED_SIZE \
        (sizeof(BR_UINT_16) + sizeof(BR_UINT_32))

typedef struct _RmsRecordFileAddress {
    BR_UINT_32 block_number;
    BR_UINT_16 byte_offset;
} RmsRecordFileAddress ;


typedef struct _BODS_PAGE_INDEX_ENTRY {
    RmsRecordFileAddress rfa;
    BR_UINT_32 record_length ;
} BODS_PAGE_INDEX_ENTRY, *BODS_PAGE_INDEX_ENTRY_PTR ;



/* Book Header
 */

typedef struct _BODS_BOOK_HEADER {
    BMD_VERSION version;
    char LMF_producer[24];
    char LMF_product_name[24];
    BR_UINT_32 LMF_product_date[2];
    BR_UINT_32 LMF_product_version;
    BR_UINT_32 n_pages ;
    BR_UINT_32 n_chunks ;
    BR_UINT_32 n_directories ;
    BR_UINT_32 n_fonts ;
    BR_UINT_32 max_font_id ;
    RmsRecordFileAddress page_index_rfa;
    BR_UINT_16 page_index_length;
    BMD_OBJECT_ID vbh_extension;
    BMD_OBJECT_ID chunk_index_page ;
    BMD_OBJECT_ID chunk_titles_page ;
    BMD_OBJECT_ID symbol_table_page ;
    BMD_OBJECT_ID font_definitions_page ;
    BMD_OBJECT_ID first_data_page ;
    BMD_OBJECT_ID copyright_chunk;
    BR_UINT_8 name_length ;
    char book_name[128];

    /*  Fields added for V2.  */
    BR_UINT_8 book_build_date[8];
    char symbolic_name[32] ;
    BR_UINT_8 LMF_alternate_product_count;

    /*  Fields added for V3.0  */
    /*  longword alignment is maintained  */
    BR_UINT_16 filler_offset;    /* offset of variable part of header
				     *  = num bytes of fixed part of hdr  */
    BMD_OBJECT_ID copyright_topic;
    BMD_VERSION oaf_version;
    BMD_OBJECT_ID dir_headr_page;   /* directory page index */
    BMD_OBJECT_ID symbol_exref_index_page ;
    BMD_OBJECT_ID static_exref_index_page ;
    BR_UINT_32 n_symbols;    /*  num entries in symbol table  */
    BR_UINT_32 n_symbol_exrefs; /*  num entries in sym ext ref. table  */
    BR_UINT_32 n_sys_directories ;
    BMD_VERSION bwi_version;	    /* BWI version number     */
    BR_UINT_8 vbh_filler [684];
    BR_UINT_32 checksum;	    /* sum of all bytes except checksum */

} BODS_BOOK_HEADER ;


/*  Symbol table entry */

typedef struct _BODS_SYMBOL_ENTRY {
    BMD_OBJECT_ID   id;
    BR_UINT_8   len;	    /*  total length of entry  */
    char	    name[1];
    }	    BODS_SYMBOL_ENTRY;


/* Directory Header
 */

typedef struct _BODS_DIRECTORY_HEADER {
    BMD_OBJECT_ID directory_id ;
    union {
        BR_UINT_8 all_flags;
        struct {
            unsigned contents : 1;
            unsigned index : 1;
            unsigned default_directory : 1 ;
            unsigned multi_valued : 1 ;
            unsigned minor_version : 1 ;
            unsigned fill : 3 ;
        } bits ;
    } flags ;
    BR_UINT_32 n_entries ;
    BMD_OBJECT_ID dir_entries_page ;
    BR_UINT_8 name_len;
    char directory_name[512];
} BODS_DIRECTORY_HEADER ;


/* Directory Entry 
 */

typedef struct _BODS_DIRECTORY_ENTRY {
    BODS_TAG tag;
    BR_UINT_8 header_level;
    BR_UINT_8 data_type;
    BR_UINT_32 width;
    BR_UINT_32 height;
    BR_UINT_8 title_length ;
    char *title;
    BR_UINT_8 n_targets;
    BR_UINT_32 *target_list;
    BR_UINT_16 data_length;
    BR_UINT_8 *data;
} BODS_DIRECTORY_ENTRY;


/* Static External Reference Directory Header
 */

typedef struct _BODS_STATIC_EXREF_DIR_HEADER {
    BR_UINT_32 num_books;		    /*  num entries in book list  */
    BR_UINT_32 book_list_offset;	    /*  from start of page  */
    BR_UINT_32 num_targets;	    /*  num entries in target list  */
    BR_UINT_32 target_list_offset;    /*  from start of page  */
} BODS_STATIC_EXREF_DIR_HEADER;


/* Static External Reference Directory Book list entry.
 * The book list has one entry for each book referenced
 */

typedef struct _BODS_STATIC_EXREF_BOOK {
    BR_UINT_16 len;		    /*  total length of entry  */
    BR_UINT_8 timestamp[8];	    /*  timestamp of referenced book  */
    char book_name[32];		    /*  symbolic name of ref. book  */
    char book_title[1];		    /*  display title of ref. book  */
} BODS_STATIC_EXREF_BOOK;


/* Static External Reference Directory Target list entry
 * The target list has one entry for each static object referenced
 */

typedef struct _BODS_STATIC_EXREF_TARGET {
    BR_UINT_16 book_num;	    /*  index into book list  */
    BMD_OBJECT_ID  object_id;	    /*  object id within target book  */
} BODS_STATIC_EXREF_TARGET;



/* Chunk Header 
*/

typedef struct _BODS_CHUNK_HEADER {
    BODS_TAG tag;
    BR_UINT_32 data_type;
    BMD_OBJECT_ID chunk_id;
    BMD_OBJECT_ID parent_chunk;
    BR_UINT_32 x;
    BR_UINT_32 y;
    BR_UINT_32 width;
    BR_UINT_32 height;
    BR_UINT_32 target;
    BR_UINT_32 data_length;
    BR_UINT_8  *data;
} BODS_CHUNK_HEADER;


/*  Page data
 */

typedef struct _BODS_PAGE_HEADER {
    BMD_OBJECT_ID page_id ;
    BR_UINT_32 page_type;
    BR_UINT_32 n_chunks ;
    BMD_OBJECT_ID prev_page_id ;
    BMD_OBJECT_ID next_page_id ;
    BODS_CHUNK_HEADER *chunks;
} BODS_PAGE_HEADER ;


/* Font defintion 
 */

typedef struct _BODS_FONT_DEFINITION {
    BODS_TAG tag;
    BR_UINT_16 font_id;
} BODS_FONT_DEFINITION ;


#endif /* _BODS_PRIVATE_DEF_H */

/* DONT ADD STUFF AFTER THIS #endif */

