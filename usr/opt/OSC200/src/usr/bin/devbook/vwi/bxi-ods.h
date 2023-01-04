Return-Path: decwet::haslock
Date: Mon, 13 Aug 90 17:50:44 PDT
From: decwet::haslock
To: slough::haslock

/**/
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
#include "vri-private-def.h"
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
#define TLV$K_VBH 1                     /* voila book header page           */
#define TLV$K_VBH_EXT 2                 /* voila book header extension page */
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
#define TLV$K_MAX_TAG 24
#define TLV$K_MIN_PG_TAG 17
#define TLV$K_MAX_PG_TAG 23

typedef	struct	_UNDEF
	{
	    struct _UNDEF   *next;
	    unsigned	    pgid;
	    unsigned	    offset;
	    unsigned        parent_ckid;
	    char	    name[64];
	}	UNDEF;

typedef unsigned char BYTE;

typedef struct  _SYMBOL_ENTRY   {
            BYTE             len;
            unsigned long    id;
            char             name[32];
        }       SYMBOL_ENTRY;
 

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
#define VBH$K_V1_0_FIXED_LENGTH 255     /* length of VBH                    */
#define VBH$K_FIXED_LENGTH 296          /* length of VBH                    */
#define VBH$K_LENGTH 1022               /* length of VBH                    */
typedef struct _VBH {
    unsigned short VBH$W_TAG;       /* tag for Tag-Length-Value (TLV) format */
    unsigned long VBH$L_LEN;        /* length of VBH for TLV format     */
    unsigned short VBH$W_MAJOR_VERSION; /* ODS major version number     */
    unsigned short VBH$W_MINOR_VERSION; /* ODS eco level                */
    char VBH$T_LMF_PRODUCER [24];       /* LMF Producer name                */
    char VBH$T_LMF_PRODUCT_NAME [24];   /* LMF Product name                 */
    unsigned int VBH$Q_LMF_PRODUCT_DATE [2]; /* LMF Product release date    */
    unsigned long VBH$L_LMF_PRODUCT_VERSION; /* LMF Product Version     */
    unsigned long VBH$L_NUM_PAGES;  /* number of pages in book          */
    unsigned long VBH$L_NUM_CHUNKS; /* number of (addressable) chunks   */
    unsigned long VBH$L_NUM_DIRS;   /* number of directories in book    */
    unsigned long VBH$L_NUM_FONTS;  /* number of fonts in book          */
    unsigned long VBH$L_MAX_FONT_ID; /* highest font id in book         */
    char VBH$V_PG_NDX_RFA [6];          /* RFA of page index                */
    unsigned short VBH$W_PG_NDX_LEN; /* total bytes in page index record(s) */
    unsigned long VBH$L_VBH_EXT_PGID; /* page id of the book header extension */
    unsigned long VBH$L_CK_NDX_PGID; /* page id of chunk index          */
    unsigned long VBH$L_CK_TITLES_PGID; /* page id of chunk titles      */
    unsigned long VBH$L_SYMBOL_PGID; /* page id of symbol table         */
    unsigned long VBH$L_FONT_PGID;  /* page id of font definitions      */
    unsigned long VBH$L_FIRST_DATA_PGID; /* id of page to open with book */
    unsigned long VBH$L_COPYRIGHT_CKID; /* id of copyright chunk        */
    unsigned char VBH$B_NAME_LEN;       /* length of bookname               */
    char VBH$T_NAME [128];              /* formal bookname (title)          */
    unsigned int VBH$Q_BUILD_DATE [2];  /* Creation date of book            */
    char VBH$T_SYMBOL [32];             /* symbolic name of book            */
    unsigned char VBH$B_LMF_ALTPROD_COUNT; /* number of alt. product names  */
    char VBH$V_FILLER [722];
    unsigned long VBH$L_CHECKSUM;   /* sum of all bytes except checksum */
    } VBH;




typedef struct _BKH {
    unsigned short VBH$W_TAG;       /* tag for Tag-Length-Value (TLV) format */
    unsigned long VBH$L_LEN;        /* length of VBH for TLV format     */
    unsigned short VBH$W_MAJOR_VERSION; /* ODS major version number     */
    unsigned short VBH$W_MINOR_VERSION; /* ODS eco level                */
    char VBH$T_LMF_PRODUCER [24];       /* LMF Producer name                */
    char VBH$T_LMF_PRODUCT_NAME [24];   /* LMF Product name                 */
    unsigned int VBH$Q_LMF_PRODUCT_DATE [2]; /* LMF Product release date    */
    unsigned long VBH$L_LMF_PRODUCT_VERSION; /* LMF Product Version     */
    unsigned long VBH$L_NUM_PAGES;  /* number of pages in book          */
    unsigned long VBH$L_NUM_CHUNKS; /* number of (addressable) chunks   */
    unsigned long VBH$L_NUM_DIRS;   /* number of directories in book    */
    unsigned long VBH$L_NUM_FONTS;  /* number of fonts in book          */
    unsigned long VBH$L_MAX_FONT_ID; /* highest font id in book         */
    char VBH$V_PG_NDX_RFA [6];          /* RFA of page index                */
    unsigned short VBH$W_PG_NDX_LEN; /* total bytes in page index record(s) */
    unsigned long VBH$L_VBH_EXT_PGID; /* page id of the book header extension */
    unsigned long VBH$L_CK_NDX_PGID; /* page id of chunk index          */
    unsigned long VBH$L_CK_TITLES_PGID; /* page id of chunk titles      */
    unsigned long VBH$L_SYMBOL_PGID; /* page id of symbol table         */
    unsigned long VBH$L_FONT_PGID;  /* page id of font definitions      */
    unsigned long VBH$L_FIRST_DATA_PGID; /* id of page to open with book */
    unsigned long VBH$L_COPYRIGHT_CKID; /* id of copyright chunk        */
    unsigned char VBH$B_NAME_LEN;       /* length of bookname               */
    char VBH$T_NAME [128];              /* formal bookname (title)          */
    unsigned int VBH$Q_BUILD_DATE [2];  /* Creation date of book            */
    char VBH$T_SYMBOL [32];             /* symbolic name of book            */
    unsigned char VBH$B_LMF_ALTPROD_COUNT; /* number of alt. product names  */
    unsigned long VBH$L_DIRECTORY_PGID;	/* page id of directory headers */
    unsigned long VBH$L_TOPIC;
    unsigned long VBH$L_TOOLSID;	/* authoring tool id		*/
    unsigned long VBH$L_LANGUAGE;	/* language formatted in	*/    
    char VBH$V_FILLER [722];
    unsigned long VBH$L_CHECKSUM;   /* sum of all bytes except checksum */
    } BKH;








/*    unsigned long int BKB$L_FLINK;      /* address of next book in list     */
/*    unsigned long int BKB$L_BLINK;      /* address of previous book in list */
/*                                                                          */
/*	BKB -- Book Block                                                   */
/*                                                                          */
/*#define BKB$K_LENGTH 1618               /* length of BKB (including VBH)    */
#define BKB$K_LENGTH 1612               /* length of BKB (including VBH)    */
typedef struct _BKB {
    VriQueue	BKB$QUEUE;
    unsigned char BKB$B_FILENAME_LEN;   /* file name length                 */
    char BKB$T_FILENAME [511];          /* file name                        */
    unsigned long BKB$L_POOL_ID;    /* buffer pool for this book        */
    unsigned long BKB$L_NUM_PAGES;  /* number of pages in book          */
    PIE *BKB$L_PG_NDX;                  /* address of the page index [0]    */
    unsigned long BKB$L_PG_NDX_BUFF_LEN; /* length of page index buffer */
    PME *BKB$L_PG_MAP;                  /* address of the page map [0]      */
    unsigned long BKB$L_PG_MAP_BUFF_LEN; /* length of page map buffer   */
    CKB *BKB$L_CK_NDX;                  /* address of the chunk index [0]   */
    unsigned long BKB$L_CK_NDX_BUFF_LEN; /* length of page index buffer */
    unsigned long BKB$L_CK_TITLES;  /* address of chunk titles buffer   */
    unsigned long BKB$L_CK_TITLES_BUFF_LEN; /* length of chunk title buf */
    unsigned long BKB$L_DIR_NDX;    /* address of the dir. index[0]     */
    unsigned long BKB$L_DIR_NDX_BUFF_LEN; /* length of dir. index buf   */
    UNDEF *BKB$L_SYMBOLS;               /* address of symbol table buffer   */
    unsigned long BKB$L_SYMBOLS_BUFF_LEN; /* length of symbol table buf */
    unsigned long BKB$L_FONT_LIST;  /* address of list of fonts         */
    unsigned long BKB$L_FAB;        /* address of FAB for this book     */
    unsigned long BKB$L_RAB;        /* address of RAB for this book     */
    VriQueue *BKB$L_DRB_LIST;   /* address of directory list        */
    unsigned long BKB$L_DRB_BLINK;  /* addr of end of dir list          */
    VBH BKB$V_VBH;                      /* VBH for this book                */
    } BKB;

typedef struct _VriContext {
    VriShelfEntry entry;
    union {
        BKB *book;
        VriShelfBlock *shelf;
    } data ;
    VriMemoryPool pool;
    FILE *file;
    char reason[256];
    jmp_buf jump_buffer;
} VriContext;

#define VriBookPtr(bkid) (((VriContext *)(bkid))->data.book)

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

