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
/* DEC/CMS REPLACEMENT HISTORY, Element VWI-BOOK.C*/
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1989 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**   BWI -  Bookreader Writer Interface
**
** ABSTRACT:
**
**   This module controls the BOOK level processing 
**
** AUTHORS:
**  JOE KREATSCH - BLISS original
**  MIKE FITZELL - Conversion to C
**
** CREATION DATE: 3-JAN-90
**
**	
**
** MODIFICATION HISTORY:
** 03-Mar-1990
**  Nigel Haslock - Major hacks to cope with non-aligned structures
**
**--
*/


/*
**  INCLUDE FILES  
*/
#ifdef vms
#include <ssdef.h>
#include <descrip.h>
#include <file.h>
#include <libdef.h>
#endif

#include <ctype.h>
#include <time.h>
#include <string.h>

#include "chunk.h"
#include "bxi_def.h"

#define BKB$K_PG_NDX_BUFF_INIT_LEN  sizeof ( PIE ) * 50
#define BKB$K_PG_MAP_BUFF_INIT_LEN  sizeof ( PME ) * 50

#ifdef vms
BWI_ERROR use_rms_open();
BWI_ERROR use_rms_clothes();
globaldef VriContext CONTEXT;
globalref int VWI$GL_PREV_PAGE;
globaldef BKB VXI$GQ_BOOKLIST = {&VXI$GQ_BOOKLIST,&VXI$GQ_BOOKLIST};
globalref int next_rfa;
globalref short next_offset;
#else
extern int VWI$GL_PREV_PAGE;
extern int next_rfa;
extern short next_offset;
VriContext CONTEXT;
#endif

#define ULTRIX_DEFAULT_SHELF_EXT ".decw_bookshelf"
#define ULTRIX_DEFAULT_BOOK_EXT ".decw_book"

UNDEFSYM *VWI_SYMBOL_COMPLAIN();
BWI_ERROR bwi$write_bkh();
BWI_ERROR bwi$write_bkeh();

/***** bwi_book_abort --  Aborts the specified book and deletes the file. */
BWI_ERROR bwi_book_abort(bkb)
BKB *bkb;	    /*  book id from bwi_book_create  */
{
    BWI_ERROR	status;		/* return status from RMS */
    char *delete_file;  /* put command in here that'll delete the file */

#ifdef vms
    struct FAB	*file_fab;	/* FILE ACCESS BLOCK */
#endif

    if(!verify_bkid(bkb))
	return(BwiInvalidBookId);  /*! make sure it is a BKID */

#ifdef vms 
    /* Set the delete flag and close the file */

    file_fab = (struct FAB *) bkb->BKB$L_FAB;
    file_fab->fab$l_fop = FAB$M_DLT;
    if((status = sys$close(file_fab)) != RMS$_SUC) 
        lib$signal(status);
#else  

    delete_file = (char *) malloc(bkb->BKB$B_FILENAME_LEN + 4);
    sprintf(delete_file,"rm %s",bkb->BKB$T_FILENAME);

    /* close the file  we also have to delete it cause it's bogus */
    fclose(CONTEXT.file);

    system(delete_file); 
    free(delete_file);

#endif
    return(BwiOK);
}


/***** bwi_book_copyright - defines where the copyright "page" is *****/
BWI_ERROR bwi_book_copyright (bkb, ckid, pgid)
BKB *bkb;                   /*  book id from bwi_book_create  */
unsigned    ckid;           /*  parent chunk id from VWI_TOPIC_DATA_CHUNK  */
unsigned    pgid;           /*  page id from VWI_TOPIC_CREATE  */
{
    if(!verify_bkid(bkb))
        return(BwiInvalidBookId);	    /*! make sure it is a BKID */

    bkb->BKB$V_BKH.BKH$L_COPYRIGHT_CKID = ckid; 

    bkb->BKB$V_BKH_EXT.BKEH$L_COPYRIGHT_PGID = pgid; /* "FOR POST V3"*/

  return(BwiOK);
}

/***** bwi_book_language - defines language used *****/
unsigned int bwi_book_language (bkb, language)
BKB *bkb;                   /*  book id from bwi_book_create  */
char    *language;
{
    int len;
    char    tlv[6];         /*  packet header */
    unsigned long status;

    if(!verify_bkid(bkb))
        return(BwiInvalidBookId);           /*! make sure it is a BKID */

    if(language != NULL)
    {
        len = strlen(language) + 1;
        put_short ( &tlv[0], TLV$K_LANGUAGE );
        put_int   ( &tlv[2], TLV$K_LENGTH + len );

        if((status = bwi$write_bkeh (bkb, TLV$K_LENGTH, tlv)) != BwiOK)
                return(status);

        if((status = bwi$write_bkeh (bkb, len, language)) != BwiOK)
                return(status);
    }
    return(BwiOK);
}

/***** bwi_book_partnumber - partnumber of this book *****/
unsigned int bwi_book_partnumber (bkb, partnumber)
BKB *bkb;                   /*  book id from bwi_book_create  */
char    *partnumber;
{
    int len;
    char    tlv[6];         /*  packet header */
    unsigned long status;

    if(!verify_bkid(bkb))
        return(BwiInvalidBookId);           /*! make sure it is a BKID */

    if(partnumber != NULL)
    {
        len = strlen(partnumber) + 1;
        put_short ( &tlv[0], TLV$K_PARTNUM );
        put_int   ( &tlv[2], TLV$K_LENGTH + len );

        if((status = bwi$write_bkeh (bkb, TLV$K_LENGTH, tlv)) != BwiOK)
                return(status);

        if((status = bwi$write_bkeh (bkb, len, partnumber)) != BwiOK)
                return(status);
    }
    return(BwiOK);
}

/***** bwi_book_toolsid - which authoring tool did this *****/
unsigned int bwi_book_toolsid (bkb, toolsid)
BKB *bkb;                   /*  book id from bwi_book_create  */
char    *toolsid;
{
    int len;
    char    tlv[6];         /*  packet header */
    unsigned long status;

    if(!verify_bkid(bkb))
        return(BwiInvalidBookId);           /*! make sure it is a BKID */

    if(toolsid != NULL)
    {
        len = strlen(toolsid) + 1;
        put_short ( &tlv[0], TLV$K_TOOLSID );
        put_int   ( &tlv[2], TLV$K_LENGTH + len );

        if((status = bwi$write_bkeh (bkb, TLV$K_LENGTH, tlv)) != BwiOK)
                return(status);

        if((status = bwi$write_bkeh (bkb, len, toolsid)) != BwiOK)
                return(status);
    }
    return(BwiOK);
}

/***** bwi_book_copyright_data -  copyright data that is initially displayed **/
unsigned int bwi_book_copyright_data (bkb, date, company, text)
BKB *bkb;                   /*  book id from bwi_book_create  */
char    *date;
char    *company;
char    *text;
{
    int len;
    char    tlv[6];         /*  packet header */
    unsigned long status;

    if(!verify_bkid(bkb))
        return(BwiInvalidBookId);           /*! make sure it is a BKID */

    if(date != NULL)
    {
        len = strlen(date) + 1;
        put_short ( &tlv[0], TLV$K_COPYRIGHT_DATE );
        put_int   ( &tlv[2], TLV$K_LENGTH + len );

        if((status = bwi$write_bkeh (bkb, TLV$K_LENGTH, tlv)) != BwiOK)
                return(status);

        if((status = bwi$write_bkeh (bkb, len, date)) != BwiOK)
                return(status);
    }

    if(company != NULL)
    {
        len = strlen(company) + 1;
        put_short ( &tlv[0], TLV$K_COPYRIGHT_CORP );
        put_int   ( &tlv[2], TLV$K_LENGTH + len );

        if((status = bwi$write_bkeh (bkb, TLV$K_LENGTH, tlv)) != BwiOK)
                return(status);

        if((status = bwi$write_bkeh (bkb, len, company)) != BwiOK)
                return(status);
    }

    if(text != NULL)
    {
        len = strlen(text) + 1;
        put_short ( &tlv[0], TLV$K_COPYRIGHT_TEXT );
        put_int   ( &tlv[2], TLV$K_LENGTH + len );

        if((status = bwi$write_bkeh (bkb, TLV$K_LENGTH, tlv)) != BwiOK)
                return(status);

        if((status = bwi$write_bkeh (bkb, len, text)) != BwiOK)
                return(status);
    }


  return(BwiOK);
}


/***** bwi_book_first_topic - Define first page to open' ******/
BWI_ERROR  bwi_book_first_topic (bkb, pgid)
BKB *bkb;                  /*  book id from bwi_book_create  */
unsigned    pgid;           /*  page id from VWI_TOPIC_CREATE  */
{

    if(!verify_bkid(bkb))
        return(BwiInvalidBookId);	    /*! make sure it is a BKID */

    if( pgid > bkb->BKB$L_NUM_PAGES)	 /* make sure it's a valid topic */
        return(BwiInvalidTopicId);

    bkb->BKB$V_BKH.BKH$L_FIRST_DATA_PGID = pgid;
        
    return(BwiOK);
}


/**** bwi_book_font - Define a font id for use within the book *******/

BWI_ERROR bwi_book_font (bkb, font_id, font_name)
BKB *bkb;                   /*  book id from bwi_book_create  */
unsigned    font_id;	    /* unique font_id	*/
char	    *font_name;	    /* fontname		*/
{
    BKH     *bkh;           /*  book header  */
    char    vfd[8];	    /*  font definition */
    unsigned int namelen;   /* length of font name */
    BWI_ERROR status;

    if(!verify_bkid(bkb))
        return(BwiInvalidBookId);	    /*! make sure it is a BKID */

    if(font_name == NULL)
	return(BwiBadArg); 

    namelen = strlen(font_name) + 1;

    bkh = &bkb->BKB$V_BKH;

    put_short ( &vfd[0], TLV$K_FONT );
    put_int   ( &vfd[2], VFD$K_LENGTH + namelen );
    put_short ( &vfd[6], font_id );

    if((status = VWI$WRITE_PAGE (bkb, bkh->BKH$L_FONT_PGID,
				 VFD$K_LENGTH, vfd)) != BwiOK)
	return(status);
    if((status = VWI$WRITE_PAGE (bkb, bkh->BKH$L_FONT_PGID,
				 namelen, font_name)) != BwiOK)
        return(status);

    bkh->BKH$L_NUM_FONTS++;

    if(font_id > bkh->BKH$L_MAX_FONT_ID)
	bkh->BKH$L_MAX_FONT_ID = font_id;

    return(BwiOK);
}

/***** bwi_book_license - Define license requirements' *****/
BWI_ERROR bwi_book_license(bkb,producer,product_name,product_date, product_version) 
BKB *bkb;		        /*  book id from bwi_book_CREATE  */
char	    *producer;		/*  producer name */
char	    *product_name;	/*  product name */
unsigned    *product_date;	/*  product date */
unsigned    product_version;	/*  version number */
{
    BKH     *bkh;           /*  book header  */
    int	    prod_len;	    /*  length of producer name */
    int	    name_len;	    /*  length of product name */

    if(!verify_bkid(bkb))
        return(BwiInvalidBookId);	    /*! make sure it is a BKID */

    if((product_name == NULL) || (producer == NULL))
	return(BwiBadArg);

    /* check length of strings. only 24 bytes allocated in header*/
    prod_len = strlen(producer) + 1;
    name_len = strlen(product_name) + 1;

    if (( prod_len > 24 ) || ( name_len > 24 ))
	return(BwiStrTooLong);

    bkh = &bkb->BKB$V_BKH;
    (void) strncpy( bkh->BKH$T_LMF_PRODUCER, producer, prod_len);
    (void) strncpy( bkh->BKH$T_LMF_PRODUCT_NAME, product_name, name_len);
    (void) strncpy( bkh->BKH$Q_LMF_PRODUCT_DATE, product_date, 8);
    bkh->BKH$L_LMF_PRODUCT_VERSION = product_version;

    return(BwiOK);
}

/***** bwi_book_license_alt - Define alternate product licenses' *****/
BWI_ERROR bwi_book_license_alt (bkb, product_name)
BKB *bkb;                       /*  book id from bwi_book_create  */
char	    *product_name;      /*  product name */
{
    BKH     *bkh;           /*  book header  */
    char    tlv[6];	    /*  packet header */
    int	    name_len;	    /*  length of alternate product name */
    BWI_ERROR status;

    if(!verify_bkid(bkb))
        return(BwiInvalidBookId);	    /*! make sure it is a BKID */

    if(product_name == NULL)
	return(BwiBadArg);
   
    /** check length of string ***/
    name_len = strlen(product_name) + 1;
    if ( name_len > 24 )
	return(BwiStrTooLong);

    /*! Write a TLV packet header */

    put_short ( &tlv[0], TLV$K_ALT_PROD );
    put_int   ( &tlv[2], TLV$K_LENGTH + name_len );

    if((status = bwi$write_bkh (bkb, TLV$K_LENGTH, tlv)) != BwiOK)
	return(status);

    /*! Write the alt. product name */

    if((status = bwi$write_bkh (bkb, name_len, product_name)) != BwiOK)
        return(status);
    
    bkh = &bkb->BKB$V_BKH;
    bkh->BKH$B_LMF_ALTPROD_COUNT++;
    
    return(BwiOK);
}

/****** bwi_book_symbol - Define the books symbol' ****/
BWI_ERROR bwi_book_symbol (bkb, book_sym) 
BKB *bkb;                       /*  book id from bwi_book_create  */
char	    *book_sym;		/*  books symbol name */
{
    BKH     *bkh;           /*  book header  */
    int	    len;	    /*  length of the symbol name */

    if(!verify_bkid(bkb))
        return(BwiInvalidBookId);	    /*! make sure it is a BKID */

    if(book_sym == NULL)
	return(BwiBadArg);

    len = strlen(book_sym) + 1;
    if ( len > 32 )
	return(BwiStrTooLong);

    bkh = &bkb->BKB$V_BKH;
    (void) strncpy(bkh->BKH$T_SYMBOL, book_sym, len);

    return(BwiOK);
}

/*****  bwi_book_title - Define the books title' *****/
BWI_ERROR bwi_book_title (bkb, book_name) 
BKB *bkb;                       /*  book id from bwi_book_create  */
char       *book_name;          /*  books  name */
{
    BKH     *bkh;           /*  book header  */
    char    tlv[6];	    /*  packet header */
    int     len;            /*  length of the book name */
    BWI_ERROR status;

    if(!verify_bkid(bkb))
        return(BwiInvalidBookId);	    /*! make sure it is a BKID */
 
    if(book_name == NULL)
	return(BwiBadArg);

    bkh = &bkb->BKB$V_BKH;
 
    len = strlen(book_name) + 1;
    bkh->BKH$B_NAME_LEN = len;

    if ( len > 128 ) {
        put_short ( &tlv[0], TLV$K_BOOKNAME );
        put_int   ( &tlv[2], TLV$K_LENGTH + len );

        if((status = bwi$write_bkeh (bkb, TLV$K_LENGTH, tlv)) != BwiOK)
            return(status);

        /*! Write the books title */

        if((status = bwi$write_bkeh (bkb, len, book_name)) != BwiOK)
           return(status);
    }
    else {
        (void) strncpy(bkh->BKH$T_NAME, book_name, len);
    }


    return(BwiOK);
}


/**** bwi_book_create  ******/
/**   Create a new book.  Set default page size and coordinates. */


BWI_ERROR  bwi_book_create (filename,bookid) 
char	*filename;	/* dvi file name */
unsigned int *bookid;
{
    BKB     *bkb;           /*  book block  */
    BKH     *bkh;           /*  book header  */
    BKH_EXT *bkh_ext;           /*  book header  */
    PME     *pme;	    /*  page map  */
    TLV	    *pg_ndx;	    /*  page map index  */
    unsigned int pgid;	    /*  for book header page */
    int filename_len,len;
    char    pg_head[6];	    /* for control page headers */
    char    *filespec_esa;  /* expanded filename   */
    char    *dna = ULTRIX_DEFAULT_BOOK_EXT;
    char    *root_filename;
    char    *temp_filename;
    char    *first_check;
    char    *ext_ptr;
    BWI_ERROR status;
    VriQueue *direc_que;

    PIE *pie, *page_index;
    
    VWI$GL_PREV_PAGE = *bookid = 0;

    if(filename == NULL) 
	return(BwiBadArg);

    /*! Set up a pool to allocate structures for this book from  */
    CONTEXT.book = (BKB *) VwiMalloc(sizeof(BKB),&CONTEXT,&status);

    if(CONTEXT.book == NULL)
	return(status);

    bkb = (BKB *) CONTEXT.book;

    memset(bkb, 0, sizeof (BKB) );

    bkb->BKB$L_NUM_PAGES = 0;

    /*! Create the page index   */
    /*! The page index TLV is built at the time the page is written */
    bkb->BKB$L_PG_NDX = (PIE *) VwiMalloc(BKB$K_PG_NDX_BUFF_INIT_LEN,
					  &CONTEXT,
					  &status);
    if(bkb->BKB$L_PG_NDX == NULL)
	return(status);

    bkb->BKB$L_PG_NDX_BUFF_LEN = BKB$K_PG_NDX_BUFF_INIT_LEN;

    /*! create the page map  */

    bkb->BKB$L_PG_MAP_BUFF_LEN = BKB$K_PG_MAP_BUFF_INIT_LEN;

    bkb->BKB$L_PG_MAP = (PME *)VwiMalloc(BKB$K_PG_MAP_BUFF_INIT_LEN,
					    &CONTEXT, &status);

    if(bkb->BKB$L_PG_MAP == NULL)
        return(status);

#ifdef vms
    if((status = use_rms_open(bkb,filename)) != BwiOK) 
	return(status);
#else

    /*! Get the root file name  */
    
    bkb->BKB$B_FILENAME_LEN = strlen(filename) + 1;
    memcpy(bkb->BKB$T_FILENAME ,filename, bkb->BKB$B_FILENAME_LEN);

    if ((first_check = strrchr(bkb->BKB$T_FILENAME,'.')) == NULL) {
	strcat(bkb->BKB$T_FILENAME,dna);
	bkb->BKB$B_FILENAME_LEN += strlen(dna);
	}
    else {
	if(strcmp(first_check,dna) != NULL) {
	    strcat(bkb->BKB$T_FILENAME,dna);
	    bkb->BKB$B_FILENAME_LEN += strlen(dna);
            }
	}
  
    CONTEXT.file = fopen(bkb->BKB$T_FILENAME,"w+" );

    if(CONTEXT.file == NULL) 
        return(BwiFailFileOpen);


#endif

    /*! Initialize the book header  */
    bkh = &bkb->BKB$V_BKH;	    /*! BKH is a sub-block of BKB */
    memset(bkh, 0, sizeof (BKH) ); 

    bkh->BKH$W_TAG = TLV$K_BKH;
    bkh->BKH$L_LEN = BKH_C_FIXED_LENGTH;

    /*! Use filename as default title and symbol */
#ifdef vms
    (void) bwi_book_title (bkb, bkb->BKB$T_FILENAME);
    (void) bwi_book_symbol (bkb, bkb->BKB$T_FILENAME);
    sys$gettim(bkh->BKH$Q_BUILD_DATE);	/* Timestamp the book */
#else
    /* unix doesn't have anything like rms so we have to improvise*/
    temp_filename = (char *) malloc(bkb->BKB$B_FILENAME_LEN);
    if(temp_filename == NULL)
	return(BwiFailMalloc);

    (void) strcpy(temp_filename,bkb->BKB$T_FILENAME);
    root_filename = strrchr(temp_filename,'/');
    if(root_filename != NULL)
	root_filename++;
    else
	root_filename = temp_filename;

    ext_ptr = strrchr(root_filename,'.'); 
    if(ext_ptr != NULL) {
        len = ext_ptr - root_filename;
	root_filename[len] = 0;
	}

    (void) bwi_book_title (bkb, root_filename);
    (void) bwi_book_symbol (bkb, root_filename);
    free(temp_filename);

#endif

    bkh->BKH$B_LMF_ALTPROD_COUNT = 0;

/*    ! Write out the header as the first record (page) in the book
      ! This reserves the space for the header which is written again
      ! when the book is closed
*/
    pme = (PME *) bkb->BKB$L_PG_MAP;	    /*! first element (map[0])  */
    if((status = VWI$CREATE_PAGE_CONTEXT (bkb,&pgid)) != BwiOK)
	return(status);
    
    if(pgid != 0) 
	return(BwiFailHeadCreate);

    pme->PME$B_FLAGS.PME$B_BITS.PME$V_DIRTY = TRUE;
    pme->PME$L_PG_BUFF = (PAGE *) VwiMalloc (BKH_C_LENGTH + PAGE$K_LENGTH ,
					     &CONTEXT,
					     &status);
    if(pme->PME$L_PG_BUFF == NULL)
	return(status);

    pme->PME$L_PG_LEN = pme->PME$L_PG_BUFF_LEN = BKH_C_LENGTH;
    if((status = VWI$OUTPUT_PAGE (bkb, pgid)) != BwiOK)
	return(status);


    /* initialize the "GREEN EXTENSION HEADER" */

    bkh_ext = &bkb->BKB$V_BKH_EXT;	    /*! BKH is a sub-block of BKB */
    memset(bkh_ext, 0, sizeof (BKH_EXT) ); 

    bkh_ext->BKEH$W_TAG = TLV$K_BKH_EXT;
    bkh_ext->BKEH$L_LEN = BKH_EXT_C_FIXED_LENGTH;
    bkh_ext->BKEH$B_FILLER_OFFSET =
			    BKH_EXT_C_FIXED_LENGTH + BKH_EXT_C_LENGTH_CHANGE;
    bkh_ext->BKEH$L_NUM_SYS_DIRS = 1;        /* this is a system directory */
    bkh->BKH$L_NUM_DIRS = 1;
    
    /* really what we are doing is using the next page after the header	    */
    /* as the extension to OAF LEVEL B. Instead of hard coding we will use  */
    /* the BKH$L_BKH_EXT_PGID field in the book header	and do the same	    */
    /* thing we do with the header(write out a place holder)		    */
	    
    if((status = VWI$CREATE_PAGE_CONTEXT (bkb,&pgid)) != BwiOK)
            return(status);
	    
    bkh->BKH$L_BKH_EXT_PGID = pgid;

    pme += pgid;
    pme->PME$B_FLAGS.PME$B_BITS.PME$V_DIRTY = TRUE;
    pme->PME$L_PG_BUFF = (PAGE *) VwiMalloc (BKH_C_LENGTH + PAGE$K_LENGTH,
					     &CONTEXT,
					     &status);

    if(pme->PME$L_PG_BUFF == NULL)
            return(status);

    pme->PME$L_PG_LEN = pme->PME$L_PG_BUFF_LEN = BKH_C_LENGTH;

    if((status = VWI$OUTPUT_PAGE (bkb, pgid)) != BwiOK)
            return(status);					     

    /* now create the internal system pages that only post V3 Bookreader    */
    /* will know what to do with					    */

    /** ! create the external references buffer */
    if((status = VWI$CREATE_PAGE_CONTEXT (bkb,&pgid)) != BwiOK)
        return(status);
    bkh_ext->BKEH$L_SYM_XREF_INDEX_PG = pgid;
    put_short ( &pg_head[0], TLV$K_EXT_REF_PG );
    if((status = VWI$WRITE_PAGE (bkb, bkh_ext->BKEH$L_SYM_XREF_INDEX_PG,
                                 TLV$K_LENGTH, pg_head)) != BwiOK)
        return(status);

    bkh_ext->BKEH$L_NUM_SYS_DIRS++;  /* ext ref page is a system directory */
    bkh->BKH$L_NUM_DIRS++;      /* got to incr the #total dirs */

    
    /*! create the chunk index */

    if((status = VWI$CREATE_PAGE_CONTEXT (bkb,&pgid)) != BwiOK)
	return(status);

    bkh->BKH$L_CK_NDX_PGID = pgid;

    put_short ( &pg_head[0], TLV$K_CK_NDX );
    put_int   ( &pg_head[2], 0 );
    if((status = VWI$WRITE_PAGE (bkb, bkh->BKH$L_CK_NDX_PGID, 
				 TLV$K_LENGTH, pg_head)) != BwiOK)
	return(status);

    /*! write a 0 page id for chunk 0 (unused) */
    /* need a workaround for v2 bookreader and xbook refs */ 
    put_int   ( &pg_head[2], 6); 
    if((status = VWI$WRITE_PAGE (bkb, bkh->BKH$L_CK_NDX_PGID, 
				 4, &pg_head[2])) !=  BwiOK)
	return(status);


    /** ! create the chunk titles buffer */

    if((status = VWI$CREATE_PAGE_CONTEXT (bkb,&pgid)) != BwiOK)
	return(status);
    bkh->BKH$L_CK_TITLES_PGID = pgid;
    put_short ( &pg_head[0], TLV$K_CK_TITLES );
    if((status = VWI$WRITE_PAGE (bkb, bkh->BKH$L_CK_TITLES_PGID,
				 TLV$K_LENGTH, pg_head)) != BwiOK)
	return(status);

    /** create the symbol table  */

    if((status = VWI$CREATE_PAGE_CONTEXT (bkb,&pgid)) != BwiOK)
	return(status);
    bkh->BKH$L_SYMBOL_PGID = pgid;
    put_short ( &pg_head[0], TLV$K_SYMBOL );
    if((status = VWI$WRITE_PAGE (bkb, bkh->BKH$L_SYMBOL_PGID,
				 TLV$K_LENGTH, pg_head)) != BwiOK)
	return(status);

    /**! create the font list page */

    if((status = VWI$CREATE_PAGE_CONTEXT (bkb,&pgid)) != BwiOK)
	return(status);
    bkh->BKH$L_FONT_PGID = pgid;
    put_short ( &pg_head[0], TLV$K_FONT_PG );
    if((status = VWI$WRITE_PAGE (bkb, bkh->BKH$L_FONT_PGID,
				 TLV$K_LENGTH, pg_head)) != BwiOK)
        return(status);


    /* Initialize directory queue */
    bkb->BKB$L_DRB_LIST = (VriQueue *) VwiMalloc(sizeof(VriQueue),
					  &CONTEXT,
					  &status);
    if(bkb->BKB$L_DRB_LIST == NULL)
	return(status);


    direc_que = (VriQueue *) bkb->BKB$L_DRB_LIST;
    VXI_QINIT(direc_que);

                                                                 
    /*! Return the address of the BKB as the Book-Id */
    *bookid = (unsigned int ) bkb;
    
    return(status);
}


/***** bwi_book_close -- Close a book' ****/
BWI_ERROR bwi_book_close (bkb,undef_symbol)
BKB *bkb;	            /*  book id from bwi_book_create  */
UNDEFSYM **undef_symbol;    /* pointer to linked list of undef'ed symbols */
{
    BKH     *bkh;           /*  book header  */
    BKH_EXT *bkh_ext;	    /* extension header */
    PME	    *pg_map;	    /*  page _map */
    PME	    *pme;	    /*  page map entry*/
    int	    pgid;	    /*  page id  */
    PIE     *pie;	    /*  page index entry */
    PIE	    *pye;	    /*  page index entry */
    int     i;		    /*  increment counter */
    char    *bkh_bytes;	    /*  pointer to beginning of BKH  */
    unsigned checksum;	    /*  check sum value  */
    char    line_buff[512]; /* bookshelf entry buffer */
    BWI_ERROR status;    /* status returned from RMS calls */
    FILE *shelf_file;
    char    *pie_ptr;		/* reformating pointer for page index page */
    char    *root_filename;
    char    *ext_ptr;
    int	    len;
    

    if(!verify_bkid(bkb))
	return(BwiInvalidBookId);		/*! make sure it is a BKID */

    bkh_ext = &bkb->BKB$V_BKH_EXT;

    /* Fill in version numbers of OnlineAuthorFormat */
    bkh_ext->BKEH$_BK_VERSION.major_num = VOILA$K_MAJOR_VERSION;
    bkh_ext->BKEH$_BK_VERSION.minor_num = VOILA$K_MAJOR_VERSION;


    pg_map = bkb->BKB$L_PG_MAP;

    /*! create a next page packet and write to prev_page (if there is one) */

    if(VWI$GL_PREV_PAGE > 0)
	bwi_topic_close (bkb, VWI$GL_PREV_PAGE);

    VWI$GL_PREV_PAGE = -1;


    /*! Close any open data pages */
/* hard code 2 as extension page but should be defined as something -MJF */
	pg_map++;
    for(pgid= 2; pgid < bkb->BKB$L_NUM_PAGES; pgid++) {
	pg_map++;
	if(pg_map->PME$B_FLAGS.PME$B_BITS.PME$V_DIRTY) {
	    if((status = bwi_topic_close (bkb, pgid)) != BwiOK)
		return(status);
	    }	
	}


    /*! Complain if any symbols undefined */
    if(bkb->BKB$L_SYMBOLS != NULL) {
	*undef_symbol = (UNDEFSYM *)bwi_symbol_complain(bkb);
	bwi_book_abort(bkb);
	return(BwiUndefSymbol);
	}

    /*	Write the page index			      */
    /*	first create a page context (page map entry)  */
    /*	then write this "page" out		      */
    if((status = VWI$CREATE_PAGE_CONTEXT(bkb,&pgid)) != BwiOK)
	return(status);

    pme = bkb->BKB$L_PG_MAP; 
    pme += pgid;
    pie = bkb->BKB$L_PG_NDX;
    pie += pgid;
    
/* this is normally done in VWI$OUTPUT_PAGE but we need to fix the structure */
/* before we copy it into the buffer */

    put_int ( &pie->PIE$V_PG_RFA[0], next_rfa );
    put_short ( &pie->PIE$V_PG_RFA[4], next_offset );

    if(next_offset > 255)
	pie->PIE$V_PG_RFA[5] = (next_offset >> 8);

    pme->PME$L_PG_LEN = TLV$K_LENGTH +  bkb->BKB$L_NUM_PAGES * PIE$K_LENGTH;

    pme->PME$L_PG_BUFF = (PAGE *) malloc ( PAGE$K_LENGTH + TLV$K_LENGTH +
			        PIE$K_LENGTH * bkb->BKB$L_NUM_PAGES );
    
    if(pme->PME$L_PG_BUFF == NULL)
	return(BwiFailMalloc);

    pie_ptr = (char *) &pme->PME$L_PG_BUFF->PAGE$V_DATA[0];
    
    put_short ( pie_ptr, TLV$K_PG_NDX );
    pie_ptr += 2;
    put_int   ( pie_ptr, pme->PME$L_PG_LEN );
    pie_ptr += 4;
    
    i = 0;
    pye = bkb->BKB$L_PG_NDX;
    while ( i++ < bkb->BKB$L_NUM_PAGES )
    {
	    memcpy  ( pie_ptr, pye->PIE$V_PG_RFA, 6 );
	    pie_ptr += 6;
/* since the previous version didn't get this right, this may be unnecessary */
/* For the last structure, the length is not present so use the real length. */
	    if ( i < bkb->BKB$L_NUM_PAGES )
	      put_int ( pie_ptr, pye->PIE$L_PG_LEN );
	    else
	      put_int ( pie_ptr, pme->PME$L_PG_LEN );
	    pie_ptr += 4;
	    pye++;
    }
    
    if((status = VWI$OUTPUT_PAGE (bkb, pgid)) != BwiOK)
	return(status);
    free ( pme->PME$L_PG_BUFF );
    
    /*! put page index rfa and length in the book header */

    bkh = &bkb->BKB$V_BKH;
    pie = bkb->BKB$L_PG_NDX;
    pie +=  pgid;
    
    memcpy(bkh->BKH$V_PG_NDX_RFA, pie->PIE$V_PG_RFA, 6);

    bkh->BKH$W_PG_NDX_LEN = pie->PIE$L_PG_LEN;

    /* Fill in version numbers of Bookreader */

    bkh->BKH$_BK_VERSION.major_num = VOILA$K_MAJOR_VERSION;
    bkh->BKH$_BK_VERSION.minor_num = VOILA$K_MINOR_VERSION;

    
    /* Fill in number of pages */
    bkh->BKH$L_NUM_PAGES = bkb->BKB$L_NUM_PAGES;

/* Checksum is calculated and inserted before the page is output */


#ifdef vms
    if((status = use_rms_clothes(bkb)) != BwiOK)
	return(status);
#else
    /*! Write out the header     */

    if((status = VWI$OUTPUT_PAGE (bkb, 0)) != BwiOK)
        return(status);

    fclose(CONTEXT.file);

    /*! Create the BOOKSHELF file and  entry */

    strcat(bkb->BKB$T_FILENAME,"shelf");
    shelf_file = fopen(bkb->BKB$T_FILENAME,"w" );

    root_filename = strrchr(bkb->BKB$T_FILENAME,'/');
    if(root_filename != NULL)
	root_filename++;
    else
	root_filename = bkb->BKB$T_FILENAME;

    ext_ptr = strrchr(root_filename,'.'); 
    if(ext_ptr != 0) {
        len = ext_ptr - root_filename;
	root_filename[len] = 0;
	}

    /* make the one line entry up */
    sprintf(line_buff,"BOOK\\%s\\%s\n",root_filename,bkh->BKH$T_NAME);

    /* write it out */
    fprintf(shelf_file,"%s",line_buff);
    fclose(shelf_file);
#endif

    return(status);
/*  ! bwi_book_close */
}


/** bwi$write_bkh -- Write information to the book header buffer (BKH)'**/

BWI_ERROR bwi$write_bkh (bkb, data_len, data_addr)
BKB *bkb;                   /*  book id from bwi_book_CREATE  */
unsigned    data_len;	    /*  length of data in bytes */
char	    *data_addr;	    /*  address of data  */
{
    BKH     *bkh;           /*  book header  */
    PME     *pgmap;         /*  page map  */
    char    *bufferinhead;  /*  ptr to header buffer */
    int	    index;	    /* index into header buffer */

    bkh = &bkb->BKB$V_BKH;
    pgmap = bkb->BKB$L_PG_MAP;

    /* Make sure the buffer is big enough. */

    if((bkh->BKH$L_LEN + data_len) >  BKH_C_LENGTH) {
	return(BwiFailHeadLength);
	}

    /* Copy the data to the page buffer and update the page length */
    if(bkh->BKH$L_LEN == BKH_C_FIXED_LENGTH )
	bufferinhead = &bkh->BKH$V_FILLER[0];
    else {
	index = bkh->BKH$L_LEN - BKH_C_FIXED_LENGTH;
	bufferinhead = &bkh->BKH$V_FILLER[index];
	}
    
    memcpy(bufferinhead , data_addr, data_len);
    bkh->BKH$L_LEN +=  data_len;

    pgmap->PME$B_FLAGS.PME$B_BITS.PME$V_DIRTY = TRUE;

    return(BwiOK);
}
/** bwi$write_bkeh -- Write information to the book extension header buffer **/

BWI_ERROR bwi$write_bkeh (bkb, data_len, data_addr)
BKB *bkb;                   /*  book id from bwi_book_CREATE  */
unsigned    data_len;	    /*  length of data in bytes */
char	    *data_addr;	    /*  address of data  */
{

    BKH_EXT *bkh_ext;           /*  book header  */
    BKH     *bkh;           /*  book header  */
    PME     *pgmap;         /*  page map  */
    char    *bufferinhead;  /*  ptr to header buffer */
    int	    index;	    /* index into header buffer */

    bkh_ext = &bkb->BKB$V_BKH_EXT;
    pgmap = bkb->BKB$L_PG_MAP;

    /* Make sure the buffer is big enough. */

    if((bkh_ext->BKEH$L_LEN + data_len) >  BKH_C_LENGTH) {
	return(BwiFailHeadLength);
	}

    /* Copy the data to the page buffer and update the page length */
    if(bkh_ext->BKEH$L_LEN == BKH_EXT_C_FIXED_LENGTH )
	bufferinhead = &bkh_ext->BKEH$V_FILLER[0];
    else {
	index = bkh_ext->BKEH$L_LEN - BKH_EXT_C_FIXED_LENGTH;
	bufferinhead = &bkh_ext->BKEH$V_FILLER[index];
	}
    
    memcpy(bufferinhead , data_addr, data_len);
    bkh_ext->BKEH$L_LEN +=  data_len;

    pgmap->PME$B_FLAGS.PME$B_BITS.PME$V_DIRTY = TRUE;

    return(BwiOK);
}

#ifdef vms
BWI_ERROR use_rms_open(bkb,filespec)
BKB *bkb;	    /* bookid from book create */
char *filespec;     /*books file name */
{

    struct NAM *file_nam;   /*  Name block           */
    struct FAB *file_fab;   /* FILE ACCESS BLOCK     */
    struct RAB *file_rab;   /* RECORD ACCESS BLOCK   */
    int filename_len;       /* length of filename    */
    BWI_ERROR status;
    char    *filespec_esa;  /* expanded filename     */
    struct dsc$descriptor_s
        input_filename_d = {0,DSC$K_DTYPE_T,DSC$K_CLASS_S,0};

    filename_len = strlen(filespec);
    input_filename_d.dsc$w_length = filename_len;
    input_filename_d.dsc$a_pointer = filespec;
    /*! Initialize the FAB, RAB and NAM   */
    bkb->BKB$L_FAB = VwiMalloc(FAB$C_BLN,&CONTEXT,&status);
    if(bkb->BKB$L_FAB == NULL)
	return(status);
    
    bkb->BKB$L_RAB = VwiMalloc(RAB$C_BLN,&CONTEXT,&status);
    if(bkb->BKB$L_RAB == NULL)
        return(status);

    file_nam = VwiMalloc(NAM$C_BLN,&CONTEXT,&status);
    if(file_nam == NULL)
	return(status);

    filespec_esa  = VwiMalloc(NAM$C_MAXRSS,&CONTEXT,&status);
    if(filespec_esa == NULL)
	return(status);

    *file_nam = cc$rms_nam;
    file_nam->nam$l_name = input_filename_d.dsc$a_pointer;
    file_nam->nam$b_name = input_filename_d.dsc$w_length;
    file_nam->nam$l_esa = filespec_esa;
    file_nam->nam$b_ess = NAM$C_MAXRSS;

    file_fab = (struct FAB *) bkb->BKB$L_FAB;
    *file_fab = cc$rms_fab;
    file_fab->fab$l_nam = file_nam;
    file_fab->fab$l_dna = ".decw$book";
    file_fab->fab$b_dns = 11;
    file_fab->fab$l_fna = input_filename_d.dsc$a_pointer;
    file_fab->fab$b_fns = input_filename_d.dsc$w_length;
    file_fab->fab$l_fop = FAB$M_CBT | FAB$M_TEF;
    file_fab->fab$b_fac = FAB$M_GET | FAB$M_PUT |FAB$M_UPD;
    file_fab->fab$b_org = FAB$C_SEQ;
    file_fab->fab$b_rfm = FAB$C_VAR;


    file_rab = (struct RAB *) bkb->BKB$L_RAB;
    *file_rab = cc$rms_rab;
    file_rab->rab$l_fab = file_fab;
    file_rab->rab$b_rac = RAB$C_SEQ;
    file_rab->rab$l_rop = RAB$V_EOF;



    /*! Create the book file and connect the RAB  */

    if((status = sys$create(file_fab)) != RMS$_SUC) {
        lib$signal(status);
        return(status);
        }
    if((status = sys$connect(file_rab)) != RMS$_SUC) {
        lib$signal(status);
        return(status);
        }

    /*! Get the root file name  */

    bkb->BKB$B_FILENAME_LEN = file_nam->nam$b_name + 1;
    memcpy(bkb->BKB$T_FILENAME ,file_nam->nam$l_name, file_nam->nam$b_name);

    return(BwiOK);
}

BWI_ERROR use_rms_clothes(bkb)
BKB *bkb;
{
    BKH     *bkh;           /*  book header  */
    int     i;              /*  increment counter */
    unsigned char    *bkh_bytes;     /*  pointer to beginning of BKH  */
    unsigned checksum;      /*  check sum value  */
    struct NAM *file_nam;   /*  Name block  */
    struct FAB *file_fab;   /* FILE ACCESS BLOCK */
    struct RAB *file_rab;   /* RECORD ACCESS BLOCK */
    int     line_length;    /* length of the bookshelf entry */
    char    line_buff[512]; /* bookshelf entry buffer */
    unsigned status;        /* status returned from RMS calls */

    bkh = (BKH *) &bkb->BKB$V_BKH;
    

    /*! Fill in checksum */

    bkh_bytes = bkh;
    i = checksum = 0;

    while(i < BKH_C_LENGTH - 4) {
        checksum += bkh_bytes[i];
        i++;
        }
    bkh->BKH$L_CHECKSUM = checksum;
    

    /*! Write out the header     */

    if((status = VWI$OUTPUT_PAGE (bkb, 0)) != BwiOK)
	return(status);

    if((status = VWI$OUTPUT_PAGE (bkb,bkb->BKB$V_BKH.BKH$L_BKH_EXT_PGID )) 
								       != BwiOK)
        return(status);

    /*! Close the file   */
    file_fab = (struct FAB *) bkb->BKB$L_FAB;
    file_rab = (struct RAB *) bkb->BKB$L_RAB;
    if((status = sys$disconnect(file_rab)) != RMS$_SUC) {
        lib$signal(status);
        return(status);
        }
    if((status = sys$close(file_fab)) != RMS$_SUC) {
        lib$signal(status);
        return(status);
        }

    /*! Remove the book from the booklist */

    /*VXI_REMQ(bkb);*/
    /*! Convert thefile name to lowercase for ULTRIX compatibility */

    LOWER_CASE (bkb->BKB$B_FILENAME_LEN - 1, bkb->BKB$T_FILENAME);
    line_length = sprintf(line_buff,"BOOK\\%s\\%s\n",
			  bkb->BKB$T_FILENAME,
			  bkh->BKH$T_NAME); 

    file_fab = (struct FAB *) bkb->BKB$L_FAB;
    file_rab = (struct RAB *) bkb->BKB$L_RAB;
    file_nam = (struct NAM *) file_fab->fab$l_nam;

   /* ! The book is closed, so just reuse it's FAB and RAB */

    *file_fab = cc$rms_fab;
    file_fab->fab$l_fna = ".DECW$BOOKSHELF";
    file_fab->fab$b_fns = 15;
    file_fab->fab$l_dna = file_nam->nam$l_esa;
    file_fab->fab$b_dns = file_nam->nam$b_esl;
    file_fab->fab$l_fop = FAB$M_CBT | FAB$M_TEF;
    file_fab->fab$b_fac = FAB$M_GET | FAB$M_PUT |FAB$M_UPD;
    file_fab->fab$b_org = FAB$C_SEQ;
    file_fab->fab$b_rfm = FAB$C_VAR;
    file_fab->fab$b_rat = FAB$M_CR;

    *file_rab = cc$rms_rab;
    file_rab->rab$l_fab = file_fab;
    file_rab->rab$b_rac = RAB$C_SEQ;
    file_rab->rab$l_rop = RAB$V_EOF;

   /* ! Create the bookshelf file and connect the RAB */


    if((status = sys$create(file_fab)) != RMS$_SUC) {
        lib$signal(status);
        return(status);
        }
    if((status = sys$connect(file_rab)) != RMS$_SUC) {
        lib$signal(status);
        return(status);
        }

    /*! Point the RAB to the book entry */

    file_rab->rab$l_rbf = &line_buff;
    file_rab->rab$w_rsz = line_length;
    file_rab->rab$b_rac = RAB$C_SEQ;  /*! Sequential access */

    /*! Write the record to the end of file */
    if((status = sys$put(file_rab)) != RMS$_SUC) {
        lib$signal(status);
        return(status);
        }

    /*! Close the shelf file */
    if((status = sys$close(file_fab)) != RMS$_SUC) {
        lib$signal(status);
        return(status);
        }

    return(BwiOK);
}

#endif
