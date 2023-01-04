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
/*****************************************************************************/
/*****************************************************************************/

/*** MODULE $metaformat ***/
/*                                                                         */
/*                                                                          */
/*	FTEXT -- Tag Length Value format for generically coding information */
/*                                                                          */
/*	format:                                                             */
/*              +--------+--------+--------+                                */
/*              | Tag    | Length | Value  |                                */
/*              +--------+--------+--------+                                */
/*                                                                          */
/*	(length includes the tag, length, and value fields)                 */
/*	Constant FTEXT_LENGTH equals the total size of the type and length  */
/*	fields.                                                             */
/*                                                                          */
/*	Length must be less than 256                                        */
/*                                                                          */
#define FTEXT_LENGTH 2                  /* length of header                 */
typedef struct _FTEXT {
    unsigned char tag;                  /* "type" (opcode) identifier       */
    unsigned char len;                  /* length of total packet           */
    char value[1];		/*start of data (generic)          */
    } FTEXT;
/*                                                                          */
/*	Tag values for electrodoc internal data format (FTEXTs)             */
/*	                                                                    */
/*                                                                          */
#define FTEXT_MIN_TAG 1
#define FTEXT_RULE 1                    /* draw a rule                      */
#define FTEXT_TEXT300 2                 /* draw text stored in 300 dpi      */
#define FTEXT_TEXT400 3                 /* draw text stored in 400 dpi      */
#define FTEXT_MAX_TAG 3
/*                                                                          */
/*	RULE -- packet value contents for RULE                              */
/*                                                                          */
#define RULE_LENGTH 8                   /* length of RULE packet            */
typedef struct _RULE {
    short int x;                        /* x coord. of upper left corner    */
    short int y;                        /* y coord. of upper left corner    */
    short int width;                    /* width of rule                    */
    short int height;                   /* height of rule                   */
    } RULE;
/*                                                                          */
/*	TEXT -- Text packet (text length is obtained by subtracting the     */
/*		FTEXT$K_LENGTH from the total packet length.                */
/*                                                                          */
/*		The format of the data field is "text_words", where each    */
/*		text_word starts with a 1 byte "delta" value which          */
/*		is the amount of space to put before the word, followed     */
/*		by a 1 byte count, followed by that many letters,           */
/*                                                                          */
#define TEXT_LENGTH 6                   /* length of TEXT packet header     */
typedef struct _TEXT {
    short int x;                        /* x coord. of first char           */
    short int y;                        /* y coord. of baseline             */
    unsigned short int font_num;        /* number assoc w/font by DEFINE_FONT */
    char data [1];                       /* start of data                    */
    } TEXT;
#define WORD_LENGTH 2                   /* length of WORD packet            */
typedef struct _WORD {
    unsigned char delta;                /* horizontal offset to start of word */
    unsigned char count;                /* number of char's in word         */
    char chars [1];                      /* start of text                    */
    } WORD;
/*                                                                         */
/*                                                                          */
/*	IMAGE -- format for image information                               */
/*                                                                          */
#define IMAGE_LENGTH 8                  /* length of WORD packet            */
typedef struct _IMAGE {
    short int res_x;                    /* horizontal resolution created at */
    short int res_y;                    /* vertical resolution created at   */
    short int pix_width;                /* width of image in pixels         */
    short int pix_height;               /* height of image in pixels        */
    char data [1];                       /* start of image data              */
    } IMAGE;

/*									    */
/*  UNDEF -- format for undefined symbols				    */
/*									    */

typedef struct  _UNDEFSYM {
            struct _UNDEFSYM	*next;
            unsigned		ckid;
            char		name[32];
	    } UNDEFSYM;

/*				    */
/* Launch tags and data structures  */
/*				    */

#define LAUNCH_SCRIPT_NAME  1	    /* script filename and symbolname of chunk*/
#define LAUNCH_SCRIPT	    2	    /* script file data */
#define LAUNCH_DATA_NAME    3	    /* data file name  */
#define LAUNCH_DATA	    4	    /* data or secondary script filename */


/*                                                                          */
/*  Data type values for electrodoc internal data chunks                    */
 /*	                                                                    */
/*                                                                          */
#define MIN_CHUNK_TYPE 1
#define CHUNK_ASCII 1                   /* ASCII text                       */
#define CHUNK_FTEXT 2                   /* DOCUMENT Formatted Text          */
#define CHUNK_RAGS 3                    /* RAGS Graphics Editor format      */
#define CHUNK_IMAGE75 4                 /* Bitmap Image --  75 dpi resolution */
#define CHUNK_IMAGE 5                   /* Bitmap Image                     */
#define NEVER_USED 6                    /* CHUNK_IMAGE100 -- 100dpi Bitmap Image */
#define CHUNK_DDIF 7                    /* DDIF format                      */
#define CHUNK_POSTSCRIPT 8              /* Postscript format                */
#define CHUNK_RAGS_NO_FILL 9        /* RAGS data but don't fill the     *
                                         * background with white before     *
                                         * displaying.                      */
#define CHUNK_SGX        10
#define CHUNK_PIXMAP     11
#define CHUNK_SIXEL	 12			/* Sixel format			    */
#define CHUNK_TIFF	 13			/* Tiff format			    */
#define CHUNK_LAUNCH	 14			/* Generic Launch		    */
#define CHUNK_AUDIO 	 15		/* SOUND			    */

#define MAX_CHUNK_TYPE    15
/*                                                                          */
/*  Data chunk types                                                        */
/*                                                                          */
#define MIN_DATA_CHUNK 18
#define MAIN_CHUNK 18                   /* main level data chunk            */
#define SUB_CHUNK 19                    /* data sub chunk                   */
#define REFERENCE_CHUNK 20              /* cross reference                  */
#define LAUNCH_CHUNK 21
#define MAX_DATA_CHUNK 21
/*                                                                          */
/*  Topic types                                                             */
/*                                                                          */
#define MIN_TOPIC_TYPE 1
#define STANDARD 1                      /* mainline topic                   */
#define REFERENCE 2                     /* formal reference topic           */
#define MAX_TOPIC_TYPE 2
/*                                                                          */
/*  Directory flags                                                         */
/*                                                                          */
#define CONTENTS_MASK 1                 /* True for Table of Contents       */
#define INDEX_MASK 2                    /* True for main Index              */
#define DEFAULT_MASK 4                  /* True for default directory       */
#define MULTI_VALUED_MASK 8             /* True if multiple hits allowed    */
/*  bit mask for standard table of contents                                 */
#define TOC_FLAGS 5
/*  bitmask for standard index                                              */
#define INDEX_FLAGS 10

/* return value type */
typedef unsigned long int BWI_OBJECT_ID;

/* error code type */
typedef unsigned long int BWI_ERROR;

/* BWI Error Codes */
#define BwiOK		        1	/* Successfull completion */
#define BwiUndefSymbol	        2	/* A symbol was referenced but never defined */
#define BwiInvalidBookId        3	/* Book Identifier is Invalid */
#define BwiInvalidTopicId       4	/* Topic Identifier is Invalid   */
#define BwiInvalidUpdate        5	/* An error was encountered when updating the file header */
#define BwiFailFileOut	        6	/*  OBSOLETE */
#define BwiFailFileRead	        7	/* Error encountered when reading the file */
#define BwiTopicWritten	        8	/* Once a Topic is closed it can't be updated */
#define BwiInvalidChunkId       9	/* Chunk Identifier is Invalid */
#define BwiFailHeadCreate      10	/* Error when creating the file header */
#define BwiFailHeadLength      11	/* Indicates the header has overflowed it's fixed length */
#define BwiBadArg	       12	/* Expected parameter is missing or NULL */
#define BwiErrNoMemoryNum      13	/* Memory allocation failure (internal)*/
#define BwiErrInvalidMemNum    14	/* Memory de-allocation failure */
#define BwiStrTooLong	       15	/* String has exceeded defined limits */
#define BwiFailMalloc	       16	/* Memory allocation failure (generic)*/
#define BwiFailFileOpen	       17	/* Error encountered when opening the file */
#define BwiFailFileWrite       18	/* Error encountered when writing to the file */
#define BwiDirLevelSkip	       19	/* Directory levels MUST be in single acsending increments */
#define BwiInvalidDirId	       20	/* Directory Identifier is Invalid */
#define BwiInvalidLaunchId     21	/* Launch id is no good */
#define BwiDirEntNoTitle       22	/* No title for a Directory Entry */
#define BwiDirNoTitle	       23	/* no Title for a Directory */
