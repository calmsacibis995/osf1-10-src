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
**/

/*
**  FACILITY:
**
**   BWI -- Book Writer Interface
**
** ABSTRACT:
**
**   This module implements the data storage routines for VWI.
**   This is part of the page management and might be merged with VWI_PAGE
**   when that gets rewritten in C.
**
** AUTHORS:
**
**   Joseph M. Kraetsch
**
** CREATION DATE: 13-JUN-1989
**
** MODIFICATION HISTORY:
*/

/*  
**  INCLUDE FILES  
*/
# include   "bxi_def.h"
# include   "chunk.h"

# define    SYMBOL_LENGTH   32

#ifdef vms
globalref BKB *VXI$GQ_BOOKLIST;
globalref VriContext CONTEXT;
#else
extern VriContext CONTEXT;
#endif


BWI_ERROR    bwi_topic_data_chunk();
BWI_ERROR    bwi_topic_data_subchunk();
BWI_ERROR    bwi_topic_extension_rect();
BWI_ERROR    bwi_topic_extension_poly();
BWI_ERROR    bwi_topic_reference_rect();
BWI_ERROR    bwi_topic_reference_poly();
BWI_ERROR   verify_pgid();
int	    verify_bkid();
int	    verify_ckid();
unsigned    VWI$CKID_TO_PGID();





BWI_ERROR validate(book_id,chunk_id)
BKB *book_id;               /*  book id from VWI_BOOK_CREATE  */
unsigned    chunk_id;       /*  parent chunk id from VWI_TOPIC_DATA_CHUNK  */
{
    BKH     *bkh;           /*  book header  */
    PME     *pgmap;         /*  page map  */
    PME     *pme;           /*  page map entry  */
    unsigned    pgid;       /*  page id  */
    PAGE    *page;          /*  page storage structure  */

    if(!verify_bkid (book_id))
        return(BwiInvalidBookId);

    if(!verify_ckid(book_id, chunk_id))         /* check for valid topic_id */
        return(BwiInvalidChunkId);

    bkh = &book_id->BKB$V_BKH;
    pgmap = book_id->BKB$L_PG_MAP;
    pme = &pgmap[bkh->BKH$L_CK_NDX_PGID];   /*  chunk index  */
    pgid = VWI$CKID_TO_PGID ( pme, chunk_id );

    pme = &pgmap[pgid];

    if(pme->PME$B_FLAGS.PME$B_BITS.PME$V_DIRTY == FALSE)
        return(BwiTopicWritten);

    page = pme->PME$L_PG_BUFF;

    /*  increment the chunk count for this page  */
    page->PAGE$L_NUM_CHUNKS += 1;

    return(BwiOK);
}



/*
**  bwi_topic_data_chunk -- store a data chunk
**
*/
BWI_ERROR bwi_topic_data_chunk (bkb, pgid, title, width, height,
    data_addr, data_len, data_type, ckid)
BKB        *bkb;	    /*  book id from VWI_BOOK_CREATE  */
unsigned    pgid;	    /*  page id from VWI_TOPIC_CREATE  */
char	    *title;	    /*  title of data chunk  */
unsigned    width;	    /*  width of data chunk  */
unsigned    height;	    /*  height of data chunk  */
char	    *data_addr;	    /*  address of the data to store  */
unsigned    data_len;	    /*  number of bytes to store  */
unsigned    data_type;	    /*  data format identifier  */
unsigned    *ckid;       /*  chunk id  */
{
    BKH	    *bkh;	    /*  book header  */
    PME	    *pgmap;	    /*  page map  */
    PME	    *pme;	    /*  page map entry  */
    PAGE    *page;	    /*  page storage structure  */
    char    tlv[TLV$K_LENGTH];         /*  for building TLV headers  */
    char    sym_buff[SYMBOL_LENGTH];   /*  for processing symbol names  */
    char    ckb[CKB$K_LENGTH];         /*  chunk block  */
    int		ch;
    BWI_ERROR status;

    if(!verify_bkid (bkb))
	return(BwiInvalidBookId);
    
    bkh = &bkb->BKB$V_BKH;

    pgmap = bkb->BKB$L_PG_MAP;	/* set up page map entry */
    pme = &pgmap[pgid];

    /* check for valid topic_id */
    if((status = verify_pgid(bkb, pme, pgid)) != BwiOK) 
        return(status);

    page = pme->PME$L_PG_BUFF;

    /*  Increment the book's chunk count--the new count is the chunk id.  */

    *ckid = bkh->BKH$L_NUM_CHUNKS += 1;

    /*  Write the page id to the chunk index  */
    /*  Note - Need somethin for BIG endian machines  MJF- 10/11/91*/
     if((status = VWI$WRITE_PAGE(bkb,bkh->BKH$L_CK_NDX_PGID,4,&pgid)) != BwiOK)
	return(status);


    /*  Write the title to the titles page  */

    if(title == NULL)
	title = "";

    put_short ( &tlv[0], TLV$K_TITLE );
    put_int   ( &tlv[2], TLV$K_LENGTH + strlen (title) + 1 );
    if((status = VWI$WRITE_PAGE (bkb, bkh->BKH$L_CK_TITLES_PGID,
				 TLV$K_LENGTH, tlv)) != BwiOK)
	return(status);
    if((status = VWI$WRITE_PAGE (bkb, bkh->BKH$L_CK_TITLES_PGID,
				 strlen (title) + 1, title)) != BwiOK)
        return(status);

    /*  Increment the page's chunk count  */

    page->PAGE$L_NUM_CHUNKS += 1;

    /*  form a data packet header and write it to the book  */

    put_short ( &ckb[ 0],   TLV$K_DATA_CHUNK );
    put_int   ( &ckb[ 2],   CKB$K_LENGTH + data_len );
    put_int   ( &ckb[ 6],   data_type );
    put_int   ( &ckb[10],   *ckid );
    put_int   ( &ckb[14],   0 );
    put_int   ( &ckb[18],   0 );
    put_int   ( &ckb[22],   0 );
    put_int   ( &ckb[26],   width );
    put_int   ( &ckb[30],   height );
    put_int   ( &ckb[34],   0 );
    put_int   ( &ckb[38],   data_len );

    if((status = VWI$WRITE_PAGE (bkb, pgid, CKB$K_LENGTH, ckb)) != BwiOK)
        return(status);

    /*  copy the data to the book  */

    if((status = VWI$WRITE_PAGE (bkb, pgid, data_len, data_addr)) != BwiOK)
        return(status);

    return(status);
}			/*  end of bwi_topic_data_chunk  */


/*
**  bwi_topic_data_subchunk
**
*/
BWI_ERROR  bwi_topic_data_subchunk (bkb, ckid, x, y, width, height, 
    data_addr, data_len, data_type)
BKB *bkb;	            /*  book id from VWI_BOOK_CREATE  */
unsigned    ckid;	    /*  parent chunk id from VWI_TOPIC_DATA_CHUNK  */
int	    x;		    /*  x offset within parent chunk  */
int	    y;		    /*  y offset within parent chunk  */
unsigned    width;          /*  width of data chunk  */
unsigned    height;         /*  height of data chunk  */
char        *data_addr;     /*  address of the data to store  */
unsigned    data_len;       /*  number of bytes to store  */
unsigned    data_type;      /*  data format identifier  */
{
    BKH     *bkh;           /*  book header  */
    PME     *pgmap;         /*  page map  */
    PME     *pme;           /*  page map entry  */
    unsigned    pgid;       /*  page id  */
    PAGE    *page;          /*  page storage structure  */
    char     ckb[CKB$K_LENGTH];            /*  chunk block  */
    BWI_ERROR status;

    if(!verify_bkid (bkb))
	return(BwiInvalidBookId);

    if(!verify_ckid(bkb, ckid))         /* check for valid chunk_id */
        return(BwiInvalidChunkId);

    bkh = &bkb->BKB$V_BKH;
    pgmap = bkb->BKB$L_PG_MAP;
    pme = &pgmap[bkh->BKH$L_CK_NDX_PGID];   /*  chunk index  */
    pgid = VWI$CKID_TO_PGID ( pme, ckid );

    pme = &pgmap[pgid];     /* set up pagemap entry for this topic */

    /* check for valid topic_id */
    if((status = verify_pgid(bkb, pme, pgid)) != BwiOK) 
        return(status);

    page = pme->PME$L_PG_BUFF;
    
    /*  increment the chunk count for this page  */
    page->PAGE$L_NUM_CHUNKS += 1;

    /*  form a data packet header and write it to the book  */

    put_short ( &ckb[ 0],   TLV$K_DATA_SUBCHUNK );
    put_int   ( &ckb[ 2],   CKB$K_LENGTH + data_len );
    put_int   ( &ckb[ 6],   data_type );
    put_int   ( &ckb[10],   0 );
    put_int   ( &ckb[14],   ckid );
    put_int   ( &ckb[18],   x );
    put_int   ( &ckb[22],   y );
    put_int   ( &ckb[26],   width );
    put_int   ( &ckb[30],   height );
    put_int   ( &ckb[34],   0 );
    put_int   ( &ckb[38],   data_len );

    if((status = VWI$WRITE_PAGE (bkb, pgid, CKB$K_LENGTH, ckb)) != BwiOK)
	return(status);

    /*  copy the data to the book  */

    if((status = VWI$WRITE_PAGE (bkb, pgid, data_len, data_addr)) != BwiOK)
        return(status);

    return(status);
};			/*  end of bwi_topic_data_subchunk  */


/*
**  bwi_topic_extension_rect
**
*/
BWI_ERROR bwi_topic_extension_rect (bkb, ckid, x, y, width, height)
BKB *bkb;	            /*  book id from VWI_BOOK_CREATE  */
unsigned    ckid;	    /*  parent chunk id from VWI_TOPIC_DATA_CHUNK  */
int         x;              /*  x offset within parent chunk  */
int         y;              /*  y offset within parent chunk  */
unsigned    width;          /*  width of extension  */
unsigned    height;         /*  height of extension  */
{
    BKH     *bkh;           /*  book header  */
    PME     *pgmap;         /*  page map  */
    PME     *pme;           /*  page map entry  */
    unsigned    pgid;       /*  page id  */
    PAGE    *page;          /*  page storage structure  */
    char    ckb[CKB$K_LENGTH];            /*  chunk block  */
    char    sym_buff[SYMBOL_LENGTH];   /*  for processing symbol names  */
    BWI_ERROR status;

    if(!verify_bkid (bkb))
	return(BwiInvalidBookId);

    if(!verify_ckid(bkb, ckid))         /* check for valid chunk_id */
        return(BwiInvalidChunkId);

    bkh = &bkb->BKB$V_BKH;
    pgmap = bkb->BKB$L_PG_MAP;
    pme = &pgmap[bkh->BKH$L_CK_NDX_PGID];   /*  chunk index  */
    pgid = VWI$CKID_TO_PGID ( pme, ckid );
    pme = &pgmap[pgid];

    /* check for valid topic_id */    
    if((status = verify_pgid(bkb, pme, pgid)) != BwiOK)
        return(status);

    page = pme->PME$L_PG_BUFF;

    /*  increment the chunk count for this page  */

    page->PAGE$L_NUM_CHUNKS += 1;

    /*  form a data packet header and write it to the book  */

    put_short ( &ckb[ 0],   TLV$K_EXTENSION_RECT );
    put_int   ( &ckb[ 2],   CKB$K_LENGTH );
    put_int   ( &ckb[ 6],   0 );
    put_int   ( &ckb[10],   0 );
    put_int   ( &ckb[14],   ckid );
    put_int   ( &ckb[18],   x );
    put_int   ( &ckb[22],   y );
    put_int   ( &ckb[26],   width );
    put_int   ( &ckb[30],   height );
    put_int   ( &ckb[34],   0 );
    put_int   ( &ckb[38],   0 );

    return(VWI$WRITE_PAGE (bkb, pgid, CKB$K_LENGTH, ckb));

};			/*  end of bwi_topic_extension_rect  */


/*
**  bwi_topic_extension_poly
**
*/
BWI_ERROR bwi_topic_extension_poly (bkb, ckid, num_pts, pt_vec)
BKB *bkb;	            /*  book id from VWI_BOOK_CREATE  */
unsigned    ckid;	    /*  parent chunk id from VWI_TOPIC_DATA_CHUNK  */
unsigned    num_pts;	    /*  number of points in polygon  */
POINT	    pt_vec[];	    /*  vector on num_points polygon points  */
{
    BKH     *bkh;           /*  book header  */
    PME     *pgmap;         /*  page map  */
    PME     *pme;           /*  page map entry  */
    unsigned    pgid;       /*  page id  */
    PAGE    *page;          /*  page storage structure  */
    char    ckb[CKB$K_LENGTH];         /*  chunk block  */
    char    sym_buff[SYMBOL_LENGTH];   /*  for processing symbol names  */
    BWI_ERROR status;

    if(!verify_bkid (bkb))
	return(BwiInvalidBookId);

    if(!verify_ckid(bkb, ckid))         /* check for valid chunk_id */
        return(BwiInvalidChunkId);

    bkh = &bkb->BKB$V_BKH;
    pgmap = bkb->BKB$L_PG_MAP;
    pme = &pgmap[bkh->BKH$L_CK_NDX_PGID];   /*  chunk index  */
    pgid = VWI$CKID_TO_PGID ( pme, ckid );
    pme = &pgmap[pgid];

    /* check for valid topic_id */
    if((status = verify_pgid(bkb, pme, pgid)) != BwiOK)
        return(status);
    
    page = pme->PME$L_PG_BUFF;

    /*  increment the chunk count for this page  */

    page->PAGE$L_NUM_CHUNKS += 1;

    /*  form a data packet header and write it to the book  */

    put_short ( &ckb[ 0],   TLV$K_EXTENSION_POLY );
    put_int   ( &ckb[ 2],   CKB$K_LENGTH + 4 + num_pts * sizeof (POINT) );
    put_int   ( &ckb[ 6],   0 );
    put_int   ( &ckb[10],   0 );
    put_int   ( &ckb[14],   ckid );
    put_int   ( &ckb[18],   0 );
    put_int   ( &ckb[22],   0 );
    put_int   ( &ckb[26],   0 );
    put_int   ( &ckb[30],   0 );
    put_int   ( &ckb[34],   0 );
    put_int   ( &ckb[38],   4 + num_pts * sizeof (POINT) );

    if((status = VWI$WRITE_PAGE (bkb, pgid, CKB$K_LENGTH, ckb)) != BwiOK)
	return(status);

    /*  next write the number of points then the vector of points  */

    if((status = VWI$WRITE_PAGE (bkb, pgid, 4, &num_pts)) != BwiOK)
        return(status);

    return( VWI$WRITE_PAGE (bkb, pgid, num_pts * sizeof (POINT), pt_vec));

};			/*  end of bwi_topic_extension_poly  */


/*
**  bwi_topic_reference_rect
**
*/
BWI_ERROR bwi_topic_reference_rect (bkb, ckid, x, y, width, height, 
				    obj_symbol, book_symbol)
BKB *bkb;	            /*  book id from VWI_BOOK_CREATE  */
unsigned    ckid;	    /*  parent chunk id from VWI_TOPIC_DATA_CHUNK  */
int         x;              /*  x offset within parent chunk  */
int         y;              /*  y offset within parent chunk  */
unsigned    width;          /*  width of hotspot  */
unsigned    height;         /*  height of hotspot  */
char        *obj_symbol;    /*  symbolic name of target  */
char        *book_symbol;   /*  symbolic name of book if a X-book-ref */
{
    BKH     *bkh;           /*  book header  */
    PME     *pgmap;         /*  page map  */
    PME     *pme;           /*  page map entry  */
    unsigned    pgid;       /*  page id  */
    PAGE    *page;          /*  page storage structure  */
    char    ckb[CKB$K_LENGTH];         /*  chunk block  */
    char    sym_buff[SYMBOL_LENGTH];   /*  for processing symbol names  */
    unsigned	target_id;  /*  page id of reference target  */
    unsigned	xref_target_id = 0;  /*  page id of reference target  */
    BWI_ERROR status;
    int	    i;


    if(!verify_bkid (bkb))
	return(BwiInvalidBookId);

    if(!verify_ckid(bkb, ckid))         /* check for valid topic_id */
        return(BwiInvalidChunkId);

    if(obj_symbol == NULL)
        {               /* got to have at least 1 symbol */
        if(book_symbol == NULL)
                return(BwiBadArg);
        }

    bkh = &bkb->BKB$V_BKH;
    pgmap = bkb->BKB$L_PG_MAP;
    pme = &pgmap[bkh->BKH$L_CK_NDX_PGID];   /*  chunk index  */
    pgid = VWI$CKID_TO_PGID ( pme, ckid );
    pme = &pgmap[pgid];

    /* check for valid topic_id */
    if((status = verify_pgid(bkb, pme, pgid)) != BwiOK)
        return(status);
    
    page = pme->PME$L_PG_BUFF;

    /*  increment the chunk count for this page  */

    page->PAGE$L_NUM_CHUNKS += 1;

    /*  Convert symbol to standard format and look it up  */
    
    /*  Make sure symbols are lower case   */

    if(obj_symbol == NULL)
        obj_symbol = "";
    else
    {
        for(i=0;i < (strlen(obj_symbol)); i++)
            sym_buff[i] = tolower(obj_symbol[i]);

        sym_buff[i] = '\0';
    }
    /* no book symbol implies a local refernece */

    if(book_symbol != NULL)
    {
        for(i=0;i < (strlen(book_symbol)); i++)
                book_symbol[i] = tolower(book_symbol[i]);

        book_symbol[i] = '\0';

        if((status = bwi_symbol_xref (bkb, book_symbol,
                                           obj_symbol, &target_id)) != BwiOK)
            return(status);
    }
    else
	target_id = BWI_SYMBOL_LOOKUP (bkb, sym_buff);

    if (!target_id)	/*  symbol not defined  */
    {
	int	offset;

	offset = 34 + pme->PME$L_PG_LEN;
/*		((int)&(ckb.CKB$L_TARGET) - (int)&ckb) == 34 */
	BWI_SYMBOL_UNDEF (bkb, pgid, offset, ckid, sym_buff);
    }

/* for backward compatibility we have to hide the external reference in an */
/* unused element of the data structure and then zero out the normal target_id*/
    if(book_symbol != NULL) {
	xref_target_id = target_id;
	target_id = 0;
	}

    /*  form a data packet header and write it to the book  */

    put_short ( &ckb[ 0],   TLV$K_REFERENCE_RECT );
    put_int   ( &ckb[ 2],   CKB$K_LENGTH );
    put_int   ( &ckb[ 6],   xref_target_id );
    put_int   ( &ckb[10],   0 );
    put_int   ( &ckb[14],   ckid );
    put_int   ( &ckb[18],   x );
    put_int   ( &ckb[22],   y );
    put_int   ( &ckb[26],   width );
    put_int   ( &ckb[30],   height );
    put_int   ( &ckb[34],   target_id );
    put_int   ( &ckb[38],   0 );

    return(VWI$WRITE_PAGE (bkb, pgid, CKB$K_LENGTH, ckb));

};			/*  end of bwi_topic_reference_rect  */

/*
**  bwi_topic_reference_poly
**
*/
BWI_ERROR bwi_topic_reference_poly (bkb, ckid, num_pts, pt_vec, 
				    obj_symbol, book_symbol)
BKB *bkb;	            /*  book id from VWI_BOOK_CREATE  */
unsigned    ckid;	    /*  parent chunk id from VWI_TOPIC_DATA_CHUNK  */
unsigned    num_pts;	    /*  number of points in polygon  */
POINT	    pt_vec[];	    /*  vector on num_points polygon points  */
char	    *obj_symbol;	    /*  symbolic name of target  */
char        *book_symbol;   /* external reference symbol name */
{
    BKH     *bkh;           /*  book header  */
    PME     *pgmap;         /*  page map  */
    PME     *pme;           /*  page map entry  */
    unsigned    pgid;       /*  page id  */
    PAGE    *page;          /*  page storage structure  */
    char    ckb[CKB$K_LENGTH];         /*  chunk block  */
    char    sym_buff[SYMBOL_LENGTH];   /*  for processing symbol names  */
    unsigned	target_id;  /*  page id of reference target  */
    unsigned	xref_target_id = 0;  /*  page id of reference target  */
    BWI_ERROR status;
    int	    i;

    if(!verify_bkid (bkb))
	return(BwiInvalidBookId);

    if(!verify_ckid(bkb, ckid))         /* check for valid topic_id */
        return(BwiInvalidChunkId);

    bkh = &bkb->BKB$V_BKH;
    pgmap = bkb->BKB$L_PG_MAP;
    pme = &pgmap[bkh->BKH$L_CK_NDX_PGID];   /*  chunk index  */
    pgid = VWI$CKID_TO_PGID ( pme, ckid );
    pme = &pgmap[pgid];

    /* check for valid topic_id */
    if((status = verify_pgid(bkb, pme, pgid)) != BwiOK)
        return(status);
    
    page = pme->PME$L_PG_BUFF;

    /*  increment the chunk count for this page  */

    page->PAGE$L_NUM_CHUNKS += 1;

    /*  Convert symbol to standard format and look it up  */
/* NEED SOME ERROR CHECKING *MJF* */
    if(obj_symbol == NULL)
        obj_symbol = "";
    else
        {
        for(i=0;i < (strlen(obj_symbol)); i++)
            sym_buff[i] = tolower(obj_symbol[i]);

        sym_buff[i] = '\0';

        }

    if(book_symbol != NULL)
        {
        for(i=0;i < (strlen(book_symbol)); i++)
            book_symbol[i] = tolower(book_symbol[i]);

        book_symbol[i] = '\0';

        if((status = bwi_symbol_xref (bkb, book_symbol,
                                           obj_symbol, &target_id)) != BwiOK)
            return(status);
        }
    else    
	target_id = BWI_SYMBOL_LOOKUP (bkb, sym_buff);

    if (!target_id)	/*  symbol not defined  */
    {
	int	offset;

	offset = 34 + pme->PME$L_PG_LEN;
/*		((int)&(ckb.CKB$L_TARGET) - (int)&ckb) == 34 */
	BWI_SYMBOL_UNDEF (bkb, pgid, offset, ckid, sym_buff);
    }

/* for backward compatibility we have to hide the external reference in an */
/* unused element of the data structure and then zero out the normal target_id*/
    if(book_symbol != NULL) {
	xref_target_id = target_id;
	target_id = 0;
	}

    /*  form a data packet header and write it to the book  */

    put_short ( &ckb[ 0],   TLV$K_REFERENCE_POLY );
    put_int   ( &ckb[ 2],   CKB$K_LENGTH + 4 + num_pts * sizeof (POINT) );
    put_int   ( &ckb[ 6],   xref_target_id);
    put_int   ( &ckb[10],   0 );
    put_int   ( &ckb[14],   ckid );
    put_int   ( &ckb[18],   0 );
    put_int   ( &ckb[22],   0 );
    put_int   ( &ckb[26],   0 );
    put_int   ( &ckb[30],   0 );
    put_int   ( &ckb[34],   target_id );
    put_int   ( &ckb[38],   4 + num_pts * sizeof (POINT) );

    if((status = VWI$WRITE_PAGE (bkb, pgid, CKB$K_LENGTH, ckb)) != BwiOK)
	return(status);

    /*  next write the number of points then the vector of points  */

    if((status = VWI$WRITE_PAGE (bkb, pgid, 4, &num_pts)) != BwiOK)
	return(status);
    
    return(VWI$WRITE_PAGE (bkb, pgid, num_pts * sizeof (POINT), pt_vec));

};			/*  end of bwi_topic_reference_poly  */


/*
**   VERIFY_BKID -- check validity of a BKID (BooK ID)
**
**      BKID is the value of the book id.
*/
int verify_bkid (bkid)
BKB	*bkid;
{
    if(bkid == CONTEXT.book)
	return(TRUE);
    else
	return(FALSE);
}

/*
**   VERIFY_PGID -- check validity of a page id
**
**      BKID is the value of the book id.
**      PGID is the value of the page id.
*/
BWI_ERROR verify_pgid (bkid, pme, pgid)
BKB	    *bkid;
PME	    *pme;
unsigned    pgid;
{

    if(pgid >= bkid->BKB$L_NUM_PAGES)
	return(BwiInvalidTopicId);
   
    if( pme->PME$B_FLAGS.PME$B_BITS.PME$V_CLOSE == TRUE)
	return(BwiTopicWritten);
    else 
        return(BwiOK);
}  

/*
**   VERIFY_CKID -- check validity of a chunk id
**
**      BKID is the value of the book id.
**      CKID is the value of the chunk id.
*/
int verify_ckid (bkb, ckid)
BKB	    *bkb;
unsigned    ckid;
{
    BKH *bkh;

    bkh = &bkb->BKB$V_BKH;
    if(ckid > bkh->BKH$L_NUM_CHUNKS)
	return(FALSE);
    else
         return(TRUE);
}

/* routine to fetch a pgid from the chunk index page data */
unsigned
VWI$CKID_TO_PGID ( pme, ckid )
PME *pme;
unsigned ckid;
{
	unsigned char	*ptr;
	unsigned	 pgid;
	
	ptr = &pme->PME$L_PG_BUFF->PAGE$V_DATA[6];
	ptr += ckid * 4;
	pgid = (unsigned)*ptr++;
	pgid += ( (unsigned)*ptr++ <<  8 );
	pgid += ( (unsigned)*ptr++ << 16 );
	pgid += ( (unsigned)*ptr   << 24 );
	
	return pgid;
}
