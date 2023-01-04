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
/*****************************************************************************/
/**                                                                         **/
/** Copyright (c) 1989                                                      **/
/** by DIGITAL Equipment Corporation, Maynard, Mass.                        **/
/**                                                                         **/
/** This software is furnished under a license and may be used and  copied  **/
/** only  in  accordance  with  the  terms  of  such  license and with the  **/
/** inclusion of the above copyright notice.  This software or  any  other  **/
/** copies  thereof may not be provided or otherwise made available to any  **/
/** other person.  No title to and ownership of  the  software  is  hereby  **/
/** transferred.                                                            **/
/**                                                                         **/
/** The information in this software is subject to change  without  notice  **/
/** and  should  not  be  construed  as  a commitment by DIGITAL Equipment  **/
/** Corporation.                                                            **/
/**                                                                         **/
/** DIGITAL assumes no responsibility for the use or  reliability  of  its  **/
/** software on equipment which is not supplied by DIGITAL.                 **/
/**                                                                         **/
/*****************************************************************************/
/*******************************************************************/
/* Created 11-SEP-1989 14:30:18 by VAX SDL V3.2-12                 */
/* Source: 11-SEP-1989 14:17:01 WORK1:[VOILA.SRC]VXI-ODS.SDL;11    */
/* Modified: 2-MAR-1990 by N.R.Haslock - setting types of pointers */
/*******************************************************************/

/*** MODULE $ODS ***/
/*                                                                          */
/*	A VOILA book is stored as a sequential file of variable length records. */
/*                                                                          */
/*	The first record in this file is the header which holds global      */
/*	information about the book and a pointer to the page index.         */
/*                                                                          */
/*	The last record in this file is the page index.                     */
/*	(or last records if it won't fit in one record)                     */
/*	The page index contains one entry for each page in the book.        */
/*	This entry is indexed by the page-id (0 = header) and               */
/*	points to the (first) record for that page and tells the total length */
/*	of the page record.                                                 */
/*                                                                          */
/*                                                                         */
/*                                                                          */
/*	TLV -- Tag Length Value format for generically coding information   */
/*                                                                          */
/*	format:                                                             */
/*              +--------+--------+--------+                                */
/*              | Tag    | Length | Value  |                                */
/*              +--------+--------+--------+                                */
/*                                                                          */
/*	(length includes the tag, length, and value fields)                 */
/*	Constant TLV$K_LENGTH equals the total size of the TYPE and LENGTH  */
/*	fields.                                                             */
/*                                                                          */
/*#include "bri_private_def.h" */
/*#include "br_meta_data.h" */
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#ifdef _STDC_
#include <stddef.h>
#endif
#ifdef vms
/*#include <rms.h> */
#ifndef PATH_MAX
#define PATH_MAX NAM$C_MAXRSS
#endif
#include <types.h>
#include <stat.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef vcc
#include <stdlib.h>
#endif
#include "metaformat.h"


#define TLV$K_LENGTH 6                  /* length of header                 */
typedef struct _TLV {
    unsigned short TLV$W_TAG;       /* "type" identifier                */
    unsigned long TLV$L_LEN;        /* length for small packet          */
/*    char TLV$V_VALUE [];                /* start of data                    */
    } TLV;
/*                                                                          */
/*	Tag values for VOILA TLV format                                     */
/*	                                                                    */
/*                                                                          */
#define TLV$K_MIN_TAG 1
/*  book level tags (page types)                                            */
#define TLV$K_BKH 1                     /* voila book header page           */
#define TLV$K_BKH_EXT 2                 /* voila book header extension page */
#define TLV$K_PAGE 3                    /* data page                        */
#define TLV$K_DIR_PG 4                  /* directory page                   */
#define TLV$K_PG_NDX 5                  /* page index page                  */
#define TLV$K_CK_NDX 6                  /* chunk index page                 */
#define TLV$K_CK_TITLES 7               /* chunk titles page                */
#define TLV$K_SYMBOLS 8                 /* symbol table page                */
#define TLV$K_FONT_PG 9                 /* font definitions page            */
#define TLV$K_FRAG 10                   /* fragment of page, dir_pg, etc.   */
#define TLV$K_LAST_FRAG 11              /* last fragment of page, dir_pg, etc. */
/*  tags that occur in control pages                                        */
#define TLV$K_VDH 12                    /* voila directory header           */
#define TLV$K_SYMBOL 13                 /* symbol table entry               */
#define TLV$K_FONT 14                   /* font definition                  */
#define TLV$K_TITLE 15                  /* chunk title                      */
#define TLV$K_DIR_ENTRY 16              /* directory entry                  */
/* tags that occur in data pages                                            */
#define TLV$K_DATA 17                   /* display data                     */
#define TLV$K_DATA_CHUNK 18             /* display data chunk               */
#define TLV$K_DATA_SUBCHUNK 19          /* display data sub-chunk           */
#define TLV$K_REFERENCE_RECT 20         /* cross reference rectangle        */
#define TLV$K_REFERENCE_POLY 21         /* cross reference polygon          */
#define TLV$K_EXTENSION_RECT 22         /* extension rectangle              */
#define TLV$K_EXTENSION_POLY 23         /* extension polygon                */
/*  tags that occur in control pages                                        */
#define TLV$K_ALT_PROD 24               /* LMF alt. product name;           */
#define TLV$K_MAX_TAG TLV$K_LAUNCH
#define TLV$K_MIN_PG_TAG 17
#define TLV$K_MAX_PG_TAG 23

/* tags that define variable length data in the bookheader - V3 changes    */
#define TLV$K_PARTNUM 25
#define TLV$K_TOOLSID 26
#define TLV$K_COPYRIGHT_CORP 27
#define TLV$K_COPYRIGHT_DATE 28
#define TLV$K_COPYRIGHT_TEXT 29
#define TLV$K_LANGUAGE 30
#define TLV$K_BOOKNAME 31
#define TLV$K_EXT_REF_PG 32
#define TLV$K_DIR_HDR_PG 33
#define TLV$K_LAUNCH 34


#define BriBuildDirectoryEntryId(drb_ptr,entry_number)      \
    ( (BMD_OBJECT_ID)                                       \
        ( ((drb_ptr)->directory_object_id & 0xFF000000)     \
            | ((entry_number) & 0x00FFFFFF)                 \
        )                                                   \
    )

    /* Store the directory object id in the high order byte */
#define BriStoreDirObjectId(drb_ptr,obj_id) \
            (drb_ptr)->directory_object_id = ((obj_id) << 24)

#define BriGetDrbIndexFromObjectId(drid_value)  \
    (unsigned long int)  ( ((drid_value) & 0xFF000000) >> 24 )
   
typedef unsigned long int BMD_OBJECT_ID;
#define BWI_OBJECT_ID BMD_OBJECT_ID

typedef unsigned char BWI_BYTE;
typedef unsigned long BKR_U32_INT;
typedef unsigned short BKR_U16_INT;

typedef struct {
    unsigned short int major_num;
    unsigned short int minor_num;
} BMD_VERSION ;

typedef char *VriGenericPtr;

typedef struct _VriMemoryPool {
    int  n_slots;
    VriGenericPtr *buffer_slots;
} VriMemoryPool ;


typedef struct _VriQueue {
    VriGenericPtr flink;
    VriGenericPtr blink;
} VriQueue ;


typedef	struct	_UNDEF
	{
	    struct _UNDEF   *next;
	    unsigned	    pgid;
	    unsigned	    offset;
	    unsigned        parent_ckid;
	    char	    name[32];
	}	UNDEF;

typedef	struct	_BWI_TEMPSYM
	{
	    struct _BWI_TEMPSYM   *next;
	    BWI_OBJECT_ID     parent_ckid;
	    char	    name[32];
	}	BWI_TEMPSYM;

typedef struct  _BWI_LAUNCH
	{
	    struct _BWI_LAUNCH  *prev;
	    struct _BWI_LAUNCH  *next;
	    unsigned		topic_id;
	    unsigned		launch_id;
	    long		data_length;
	    char		*data;
	}   BWI_LAUNCH;


typedef struct  _BWI_SYMBOL_ENTRY   {
            unsigned long    id;
            BWI_BYTE         len;
            char             name[32];
        }       BWI_SYMBOL_ENTRY;
 

/*                                                                         */
/*                                                                          */
/*                                                                          */
#define PAGE$K_LENGTH 26                /* length of PAGE                   */
typedef struct _PAGE {
    unsigned short PAGE$W_TAG;      /* tag for Tag-Length-Value (TLV) format */
    unsigned long PAGE$L_LEN;       /* length of page (TLV)             */
    unsigned long PAGE$L_ID;        /* ID of this page                  */
    unsigned long PAGE$L_TYPE;      /* page type                        */
    unsigned long PAGE$L_NUM_CHUNKS; /* number of (addressable) chunks  */
    unsigned long PAGE$L_PREV;      /* ID of previous page              */
    unsigned long PAGE$L_NEXT;      /* ID of next page                  */
    unsigned char PAGE$V_DATA [26];    /* start of data - really much bigger */
    } PAGE;
/*                                                                          */
/*	Page types                                                          */
/*	                                                                    */
#define VWI$K_MIN_PAGE_TYPE 1
#define VWI$K_STANDARD_PAGE 1
#define VWI$K_REFERENCE_PAGE 2
#define VWI$K_MAX_PAGE_TYPE 2
/*                                                                         */
/*                                                                          */
/*	PIE -- Page Index Entry                                             */
#define PIE$K_LENGTH 10                 /* length of PIE                    */
typedef struct _PIE {
    char PIE$V_PG_RFA [6];              /* RFA of this page in book file    */
    unsigned long PIE$L_PG_LEN;     /* byte length of this page in file */
    } PIE;
/*                                                                         */
/*                                                                          */
/*	PME -- Page Map Entry                                               */
/*                                                                          */
#define PME$M_DIRTY 1
#define PME$M_CLOSE 2
#define PME$K_LENGTH 17                 /* length of PME                    */
typedef struct _PME {
    union  {
        unsigned char PME$B_ALL_FLAGS;
        struct  {                       /* control bits                     */
            unsigned PME$V_DIRTY : 1;   /* contents not saved on disk       */
            unsigned PME$V_CLOSE : 1;   /* page can be closed               */
            unsigned PME$V_fill_0 : 6;
            } PME$B_BITS;
        } PME$B_FLAGS;
    PAGE *PME$L_PG_BUFF;                /* page buffer address              */
    unsigned long PME$L_PG_BUFF_LEN; /* length of buffer                */
    unsigned long PME$L_PG_LEN;     /* length of this page (in buffer)  */
    unsigned long PME$L_CHUNK_LIST; /* address of list of chunks        */
    } PME;
/*                                                                         */
/*                                                                          */
/*	CKB -- Chunk Block                                                  */
/*                                                                          */
#define CKB$K_LENGTH 42                 /* length of CKB                    */
typedef struct _CKB {
    unsigned short CKB$W_TAG;       /* tag for Tag-Length-Value (TLV) format */
    unsigned long CKB$L_LEN;        /* length of VDH for TLV format     */
    unsigned long CKB$L_DATA_TYPE;  /* data type identifier             */
    unsigned long CKB$L_ID;         /* chunk identifier                 */
    unsigned long CKB$L_PARENT;     /* chunk identifier of parent       */
    unsigned long CKB$L_X;          /* x offset within parent           */
    unsigned long CKB$L_Y;          /* Y offset within parent           */
    unsigned long CKB$L_WIDTH;      /* width of display                 */
    unsigned long CKB$L_HEIGHT;     /* height of display                */
    unsigned long CKB$L_TARGET;     /* target of cross reference        */
    unsigned long CKB$L_DATA_LEN;   /* length of display data           */
/*    char CKB$V_DATA [];                 /* display data                     */
    } CKB;
/*  NOTE:                                                                   */
/*                                                                          */
/*	if we case on the tag (MAIN_CHUNK, SUB_CHUNK, X_REF),               */
/*	    we could use a single id for ID/PARENT                          */
/*	    we could overlap TARGET/DATA_LEN                                */
/*                                                                          */
/*                                                                         */
/*                                                                          */
/*	VBH -- VOILA Book Header                                            */
/*                                                                          */
/*	BKH --  Book Header                                            */
/*                                                                          */
#define BKH_C_FIXED_LENGTH 296	    /* 330 length of fixed data in BKH      */
#define BKH_C_VAR_LENGTH   722	    /* 688 length of variable data in BKH   */
#define BKH_C_LENGTH_CHANGE 0
#define BKH_C_LENGTH 1022               /* length of BKH                    */
typedef struct _BKH {
    unsigned short BKH$W_TAG;       /* tag for Tag-Length-Value (TLV) format */
    unsigned long BKH$L_LEN;        /* length of BKH for TLV format     */
    BMD_VERSION BKH$_BK_VERSION;    /* Bookreader version */
    char BKH$T_LMF_PRODUCER [24];           /* LMF Producer name            */
    char BKH$T_LMF_PRODUCT_NAME [24];       /* LMF Product name             */
    unsigned int BKH$Q_LMF_PRODUCT_DATE [2]; /* LMF Product release date    */
    unsigned long BKH$L_LMF_PRODUCT_VERSION; /* LMF Product Version     */
    unsigned long BKH$L_NUM_PAGES;	/* number of pages in book          */
    unsigned long BKH$L_NUM_CHUNKS;	/* number of (addressable) chunks   */
    unsigned long BKH$L_NUM_DIRS;	/* number of directories in book    */
    unsigned long BKH$L_NUM_FONTS;	/* number of fonts in book          */
    unsigned long BKH$L_MAX_FONT_ID;	/* highest font id in book         */
    char BKH$V_PG_NDX_RFA [6];          /* RFA of page index                */
    unsigned short BKH$W_PG_NDX_LEN; /* total bytes in page index record(s) */
    BMD_OBJECT_ID BKH$L_BKH_EXT_PGID; /* page id of the book header extension */
    BMD_OBJECT_ID BKH$L_CK_NDX_PGID;	/* page id of chunk index          */
    BMD_OBJECT_ID BKH$L_CK_TITLES_PGID; /* page id of chunk titles      */
    BMD_OBJECT_ID BKH$L_SYMBOL_PGID;	/* page id of symbol table         */
    BMD_OBJECT_ID BKH$L_FONT_PGID;	/* page id of font definitions      */
    BMD_OBJECT_ID BKH$L_FIRST_DATA_PGID; /* id of page to open with book */
    BMD_OBJECT_ID BKH$L_COPYRIGHT_CKID;  /* id of copyright chunk        */
    unsigned char BKH$B_NAME_LEN;       /* length of bookname               */
    char BKH$T_NAME [128];              /* formal bookname (title)          */
/* End of v1 fixed data		*/
    unsigned int BKH$Q_BUILD_DATE [2];  /* Creation date of book            */
    char BKH$T_SYMBOL [32];             /* symbolic name of book            */
    unsigned char BKH$B_LMF_ALTPROD_COUNT; /* number of alt. product names  */
/* End of V2 fixed data   */
    char BKH$V_FILLER [BKH_C_VAR_LENGTH];
    unsigned long BKH$L_CHECKSUM;   /* sum of all bytes except checksum */
    } BKH;

#define BKH_EXT_C_FIXED_LENGTH 42
#define BKH_EXT_C_VAR_LENGTH 466
#define BKH_EXT_C_LENGTH_CHANGE 0
typedef struct _BKH_EXT {
    BKR_U16_INT   BKEH$W_TAG;       /* tag for Tag-Length-Value (TLV) format */
    BKR_U32_INT   BKEH$L_LEN;             /* length of BKH for TLV format     */
    BKR_U32_INT   BKEH$B_FILLER_OFFSET;   /* offset of variable part of header*/
    BMD_OBJECT_ID BKEH$L_COPYRIGHT_PGID;	/* copyright page id  */
    BMD_VERSION   BKEH$_BK_VERSION;		/* OAF  version */
    BMD_OBJECT_ID BKEH$L_SYM_XREF_INDEX_PG;	/* Symbol ext ref  index */
    BMD_OBJECT_ID BKEH$L_STAT_XREF_INDEX_PG;	/* Static ext ref  index */
    BKR_U32_INT   BKEH$L_NUM_SYMBOLS;		/* # symbols*/
    BKR_U32_INT   BKEH$L_NUM_XREFS;		/* # ext refs*/
    BKR_U32_INT   BKEH$L_NUM_SYS_DIRS;		/* # system directories */
    BMD_VERSION   BKEH$_BWI_VERSION;		/* BWI version number     */
    char	  BKEH$V_FILLER [BKH_EXT_C_VAR_LENGTH];
    BKR_U32_INT   BKEH$L_CHECKSUM;   /* sum of all bytes except checksum */
    } BKH_EXT;


    

/*                                                                          */
/*	BKB -- Book Block                                                   */
/*                                                                          */
#define BKB$K_LENGTH 2644
typedef struct _BKB {
    VriQueue	BKB$QUEUE;
    unsigned char BKB$B_FILENAME_LEN;    /* file name length               */
    char BKB$T_FILENAME [511];           /* file name                      */
    unsigned long BKB$L_POOL_ID;         /* buffer pool for this book      */
    unsigned long BKB$L_NUM_PAGES;       /* number of pages in book        */
    PIE *BKB$L_PG_NDX;                   /* address of the page index [0]  */
    unsigned long BKB$L_PG_NDX_BUFF_LEN; /* length of page index buffer    */
    PME *BKB$L_PG_MAP;                   /* address of the page map [0]    */
    unsigned long BKB$L_PG_MAP_BUFF_LEN; /* length of page map buffer      */
    CKB *BKB$L_CK_NDX;                   /* address of the chunk index [0] */
    unsigned long BKB$L_CK_NDX_BUFF_LEN; /* length of page index buffer    */
    unsigned long BKB$L_CK_TITLES;  /* address of chunk titles buffer      */
    unsigned long BKB$L_CK_TITLES_BUFF_LEN; /* length of chunk title buf   */
    unsigned long BKB$L_DIR_NDX;    /* address of the dir. index[0]     */
    unsigned long BKB$L_DIR_NDX_BUFF_LEN; /* length of dir. index buf   */
    UNDEF *BKB$L_SYMBOLS;           /* address of symbol table buffer   */
    BWI_TEMPSYM *BKB$L_TEMP_SYMBOLS;    /* address of temp symbol table buffer   */
    unsigned long BKB$L_SYMBOLS_BUFF_LEN; /* length of symbol table buf */
    unsigned long BKB$L_FONT_LIST;  /* address of list of fonts         */
    unsigned long BKB$L_FAB;        /* address of FAB for this book     */
    unsigned long BKB$L_RAB;        /* address of RAB for this book     */
    VriQueue *BKB$L_DRB_LIST;	    /* address of directory list        */
    BWI_BYTE BKB$B_DIR_LEVEL;
    unsigned long BKB$L_DRB_BLINK;  /* addr of end of dir list          */
    unsigned long BKB$L_NUM_LAUNCHES;
    BWI_LAUNCH *BKB$L_LAUNCH_LIST;  /* address of linked list of launch obj's */
    BKH BKB$V_BKH;			/* BKH for this book            */
    BKH_EXT BKB$V_BKH_EXT;	    /*INFOWORKS AKA green extensions	*/
    } BKB;

typedef struct _VriContext {
    VriMemoryPool   pool;
    BKB		    *book;
    FILE	    *file;
    char	    reason[256];
    jmp_buf	    jump_buffer;
} VriContext;



/*                                                                         */
/*                                                                          */
/*	SFB -- Shelf Block                                                  */
/*                                                                          */
#define SFB$K_LENGTH 33                 /* length of SFB                    */
typedef struct _SFB {
    unsigned long SFB$L_NEXT_SFB;   /* address of next shelf in list    */
    unsigned long *SFB$A_VSE_LIST;  /* ptr to list of shelf entries (VSEs) */
    unsigned char SFB$B_FILENAME_LEN;   /* file name length                 */
    char *SFB$A_FILENAME;               /* file name                        */
    unsigned long SFB$L_POOL_ID;    /* buffer pool for this shelf       */
    unsigned long SFB$L_NUM_ENTRIES; /* number of entries               */
    unsigned long SFB$L_SYMBOLS;    /* address of symbol table buffer   */
    unsigned long SFB$L_SYMBOLS_BUFF_LEN; /* length of symbol table buffer */
    unsigned long SFB$L_FONT_LIST;  /* address of list of fonts         */
    } SFB;
/*                                                                         */
/*                                                                          */
/*	VSE -- Voila Shelf Entry                                            */
/*                                                                          */
#define VSE$K_LENGTH 45                 /* length of VSE                    */
typedef struct _VSE {
    unsigned char VSE$B_TYPE;           /* book or shelf                    */
    char *VSE$A_HOME_FILE;              /* file name of parent file         */
    char *VSE$A_TARGET_FILE;            /* file name of target file         */
    char *VSE$A_TITLE;                  /* title of entry                   */
    char VSE$T_SYMBOL [32];             /* symbolic name                    */
    } VSE;
#define VSE$K_MIN_VSE_TYPE 1
#define VSE$K_BOOK 1
#define VSE$K_SHELF 2
#define VSE$K_MAX_VSE_TYPE 2
/*                                                                         */
/*                                                                          */
/*	VDH -- VOILA Directory Header                                       */
/*                                                                          */
#define VDH$M_CONTENTS 1
#define VDH$M_INDEX 2
#define VDH$M_DEFAULT 4
#define VDH$M_MULTI_VALUED 8
#define VDH$K_LENGTH 20                 /* length of VDH                    */
typedef struct _VDH {
    unsigned short VDH$W_TAG;       /* tag for Tag-Length-Value (TLV) format */
    unsigned long VDH$L_LEN;        /* length of VDH for TLV format     */
    unsigned long VDH$L_ID;         /* id of this directory             */
    union  {
        unsigned char VDH$B_ALL_FLAGS;
        struct  {                       /* control bits                     */
            unsigned VDH$V_CONTENTS : 1; /* True for Table of Contents      */
            unsigned VDH$V_INDEX : 1;   /* True for main Index              */
            unsigned VDH$V_DEFAULT : 1; /* True for default directory       */
            unsigned VDH$V_MULTI_VALUED : 1; /* True if multiple hits allowed */
            unsigned VDH$V_fill_1 : 4;
            } VDH$B_BITS;
        } VDH$B_FLAGS;
    unsigned long int VDH$L_NUM_ENTRIES; /* number of entries in the directory */
    unsigned long VDH$L_PGID;       /* page id of dir. entries page     */
    unsigned char VDH$B_NAME_LEN;       /* length of directory name         */
/*    char VDH$V_NAME [];                 /* directory name                   */
    } VDH;
/*                                                                         */
/*                                                                          */
/*	DRB -- Directory Block                                              */
/*    unsigned long DRB$L_FLINK;      /* address of next directory in list */
/*    unsigned long DRB$L_BLINK;      /* address of previous directory in list */
/*                                                                          */
#define DRB$K_LENGTH 28                 /* length of DRB                    */
typedef struct _DRB {
    VriQueue DRB$QUEUE;  
    unsigned long DRB$L_BKID;       /* id of book this directory belongs to */
    unsigned long DRB$L_PGID;       /* page id of dir. entries page     */
    unsigned long DRB$L_NUM_ENTRIES; /* number of entries in directory  */
    unsigned long DRB$L_VDH;        /* address of directory header      */
    unsigned long DRB$L_DRE_LIST;   /* vector of ptrs to entries (DREs) */
    } DRB;
/*                                                                         */
/*                                                                          */
/*	DRE -- Directory Entry Block                                        */
/*                                                                          */
#define DRE$K_LENGTH 20                 /* length of DRE                    */
typedef struct _DRE {
    unsigned short DRE$W_TAG;       /* tag for Tag-Length-Value (TLV) format */
    unsigned long DRE$L_LEN;        /* length of VDH for TLV format     */
    unsigned char DRE$B_LEVEL;          /* Header level                     */
    unsigned char DRE$B_DATA_TYPE;      /* data type identifier             */
    unsigned long DRE$L_WIDTH;      /* width of display                 */
    unsigned long DRE$L_HEIGHT;     /* height of display                */
    unsigned char DRE$B_TITLE_LEN;      /* length of entry title            */
    unsigned char DRE$B_NUM_TARGETS;    /* number of hits                   */
    unsigned short DRE$W_DATA_LEN;  /* length of display data           */
/*    char DRE$V_DATA [];                 /* display data                     */
/* target list follows the data                                             */
    } DRE;
/*	this structure is very similar to DRE                               */
/*                                                                         */
/*                                                                          */
/*	VFD -- VOILA Font Definition                                        */
/*                                                                          */
#define VFD$K_LENGTH 8                  /* length of VFD                    */
typedef struct _VFD {
    unsigned short VFD$W_TAG;       /* tag for Tag-Length-Value (TLV) format */
    unsigned long VFD$L_LEN;        /* length of VFD for TLV format     */
    unsigned short VFD$W_ID;        /* id of this font                  */
/*    char VFD$V_NAME [];                 /* font name                        */
    } VFD;
/*                                                                         */
/*                                                                          */
/*	FRAG -- constants, etc. for page fragmentation                      */
/*                                                                          */
#define FRAG$K_LENGTH 32254             /* length of a fragment record      */
#define FRAG$K_DATA_LENGTH 32238        /* length of the data part of a fragment */

