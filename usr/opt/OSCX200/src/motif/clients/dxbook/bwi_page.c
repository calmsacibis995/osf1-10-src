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
**++
**  FACILITY:
**
**   BWI -  Bookreader Writer Interface
**
** ABSTRACT:
**
**   This module controls the PAGE/TOPIC level processing 
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
** 06-Mar-1990
**	N.R.Haslock - Major hacks to cope with alignment problems
**
**--
*/


/*
**  INCLUDE FILES  
*/
#include <time.h>
#include <ctype.h>
#include <string.h>
#include "bxi_def.h"
#include "chunk.h"

#define BKB$K_PG_NDX_BUFF_INIT_LEN  sizeof (PIE) * 50
#define BKB$K_PG_MAP_BUFF_INIT_LEN  sizeof (PME) * 50
#define PG_NDX_BUFF_INCR sizeof (PIE) * 50
#define PG_MAP_BUFF_INCR sizeof (PME) * 50
#define PME$L_REF_COUNT PME$L_CHUNK_LIST
#define BUFFER_INCR 2044

#ifdef vms
globaldef int VWI$GL_PREV_PAGE = 0;
globalref BKB *VXI$GQ_BOOKLIST;
globaldef int next_rfa;
globaldef short next_offset;
globalref VriContext CONTEXT;
#else
int VWI$GL_PREV_PAGE = 0;
extern BKB *VXI$GQ_BOOKLIST;
int next_rfa;
short next_offset;
extern VriContext CONTEXT;
#endif

void VWI$CLOSE_PAGE();
BWI_ERROR bwi_topic_previous();
BWI_ERROR bwi_topic_next();
BWI_ERROR VWI$WRITE_PAGE();
BWI_ERROR VWI$OUTPUT_PAGE();
BWI_ERROR use_rms();

/**** bwi_topic_close -- Close a page' ******/
BWI_ERROR bwi_topic_close(bkb, pgid)
BKB *bkb;
unsigned    pgid;
{
    PME *pme;       /* page map entry */
    PAGE *page;     /* ptr to page buffer */
    BWI_ERROR status;

    if(!verify_bkid(bkb))
	return(BwiInvalidBookId);  /*! make sure it is a BKID */

    pme = bkb->BKB$L_PG_MAP;
    pme += pgid;
    
    if(pme->PME$L_PG_BUFF_LEN == 0)
	return(BwiTopicWritten);

    if(pgid >= bkb->BKB$L_NUM_PAGES)
        return(BwiInvalidTopicId);

    page = pme->PME$L_PG_BUFF;
    page->PAGE$L_TYPE = page->PAGE$V_DATA[10];

    if( ((VWI$GL_PREV_PAGE == pgid) || (VWI$GL_PREV_PAGE < 0) ||
	(page->PAGE$L_TYPE != VWI$K_STANDARD_PAGE))
	 && (pme->PME$L_REF_COUNT == 0))
    /******Completely close the page  ****************/
    {   pme->PME$B_FLAGS.PME$B_BITS.PME$V_CLOSE = TRUE;
	    
	put_int   ( &page->PAGE$V_DATA[2], pme->PME$L_PG_LEN );

	if ( page->PAGE$V_DATA[0] == TLV$K_PAGE )
	      /* copy in the page structure to capture updates */
	{
		put_int   ( &page->PAGE$V_DATA[14], page->PAGE$L_NUM_CHUNKS );
		put_int   ( &page->PAGE$V_DATA[18], page->PAGE$L_PREV );
		put_int   ( &page->PAGE$V_DATA[22], page->PAGE$L_NEXT );
	}
	
	/*! write the page out to the bookfile. */

	if((status = VWI$OUTPUT_PAGE(bkb, pgid)) != BwiOK)
	    return(status);

	/*! Close the page */

	(void) VWI$CLOSE_PAGE(bkb, pgid);
    }
    else
    {
	if(!pme->PME$B_FLAGS.PME$B_BITS.PME$V_CLOSE)
	{   pme->PME$B_FLAGS.PME$B_BITS.PME$V_CLOSE = 1;
	    if(page->PAGE$L_TYPE == VWI$K_STANDARD_PAGE)
	    {   if(VWI$GL_PREV_PAGE != 0)	/* not first STANDARD page*/
		{   /*  ! Link with previous page */
			(void) bwi_topic_previous(bkb, pgid, VWI$GL_PREV_PAGE);
			(void) bwi_topic_next(bkb, VWI$GL_PREV_PAGE, pgid);

			/*! Finish closing the previous page */
			if((status = bwi_topic_close(bkb, VWI$GL_PREV_PAGE)) != BwiOK)
			    return(status);
		}
		VWI$GL_PREV_PAGE = pgid;
	    }
        }
    }
    return(BwiOK);
}	/*  END	    ! bwi_topic_close */

/**** VWI$CREATE_PAGE_CONTEXT -- Create VWI page structures' ******/
/****   Sets up the data structures for a new page	        ****/

BWI_ERROR VWI$CREATE_PAGE_CONTEXT(bkb, pgid)
BKB *bkb;		/* returned fron VWI_BOOK_CREATE */
unsigned int *pgid;
{
    PME *pme;		/* New page map entry */
    PIE *pie;		/* New page index entry */
    int	pg_ndx_len;	/* Current length of page index */
    int	pg_map_len;	/* Current length of page map   */
    PIE	*pg_ndx_buf;	/* page index buffer (generic packet header) */
    unsigned how_much_bigger;	/* length of new allocation*/
    unsigned new_length;    /* how long the buff will be with this entry */
    BWI_ERROR status;

/*    ! Make sure we have room in the page index buffer			*/

    pg_ndx_len = bkb->BKB$L_NUM_PAGES * sizeof (PIE);
    new_length = pg_ndx_len + sizeof (PIE);
    if(new_length > bkb->BKB$L_PG_NDX_BUFF_LEN)
    {   /* we need a bigger buffer */

	pg_ndx_buf = (PIE *) bkb->BKB$L_PG_NDX;
	how_much_bigger = PG_NDX_BUFF_INCR + bkb->BKB$L_PG_NDX_BUFF_LEN;

	pg_ndx_buf = (PIE *) VwiRealloc(pg_ndx_buf,how_much_bigger,
					bkb->BKB$L_PG_NDX_BUFF_LEN,
					&CONTEXT,&status);
	if(pg_ndx_buf == NULL)
	    return(status);

	bkb->BKB$L_PG_NDX = pg_ndx_buf;

	bkb->BKB$L_PG_NDX_BUFF_LEN = how_much_bigger;
    }
    /*! Make sure we have room in the page map buffer  */

    pg_map_len = bkb->BKB$L_NUM_PAGES * sizeof (PME);
    if((pg_map_len + sizeof (PME)) > bkb->BKB$L_PG_MAP_BUFF_LEN)
    {	/* we need a bigger buffer */
	how_much_bigger = bkb->BKB$L_PG_MAP_BUFF_LEN + PG_MAP_BUFF_INCR;
	if(bkb->BKB$L_PG_MAP_BUFF_LEN == 0) {
	    bkb->BKB$L_PG_MAP = (PME *) VwiMalloc(how_much_bigger,
						  &CONTEXT,
						  &status);

	    }
	else {
	    bkb->BKB$L_PG_MAP = (PME *) VwiRealloc(bkb->BKB$L_PG_MAP,
						 how_much_bigger, 
						 bkb->BKB$L_PG_MAP_BUFF_LEN,
						 &CONTEXT, &status);
	    }

	bkb->BKB$L_PG_MAP_BUFF_LEN = how_much_bigger;

	if(bkb->BKB$L_PG_MAP == NULL)
	    return(status);
    }
    /*! Zero fill the page index entry and page map entry for the new page */

    pie = bkb->BKB$L_PG_NDX;
    pie += bkb->BKB$L_NUM_PAGES;    /*pg_ndx_len; */
    memset(pie,0,sizeof (PIE));

    pme = bkb->BKB$L_PG_MAP;
    pme += bkb->BKB$L_NUM_PAGES;    /*pg_map_len; */
    memset(pme,0,sizeof (PME));

    /*! The old page count is the id for the new page */

    *pgid = bkb->BKB$L_NUM_PAGES;

    /*! Bump the page count    */

    bkb->BKB$L_NUM_PAGES++;

    return(BwiOK);
}  /* END   ! VWI$CREATE_PAGE_CONTEXT */


/***** bwi_topic_create -- Create a page' ******/
BWI_ERROR  bwi_topic_create(bkb, page_type, pgid)
BKB *bkb;	        /* returned from VWI_BOOK_CREATE */
unsigned page_type;	/* type of topic STANDARD or REFERENCE */
unsigned int *pgid;	/* ID of new page - from VWI$CREATE_PAGE_CONTEXT */
{
    PME  *pme;			    /* New page map entry */
    char page[PAGE$K_LENGTH];	    /* page header entry */
    BWI_ERROR status = 0;
    int  page_id;

    /* set everything to zero */
    memset(page,0,PAGE$K_LENGTH);

    if(!verify_bkid(bkb))
	return(BwiInvalidBookId);  /*! make sure it is a BKID */

    /*! Set up the VWI data structures for a new page */
    if((status = VWI$CREATE_PAGE_CONTEXT (bkb,&page_id)) != BwiOK)
	return(status);

    *pgid = page_id;

    /*! Form a page header  */
    /*! the page length gets filled in later (before we write the page out) */

    put_short ( &page[0], TLV$K_PAGE );
    put_int   ( &page[6], page_id );
    put_int   ( &page[10], page_type );
    put_int   ( &page[14], 0 );
    put_int   ( &page[18], 0 );
    put_int   ( &page[22], 0 );

    if((status = VWI$WRITE_PAGE(bkb,page_id,PAGE$K_LENGTH,page)) != BwiOK)
	return(status);

/* let VWI$WRITE_PAGE allocate the PAGE structure before we fill it in */
    pme = bkb->BKB$L_PG_MAP;
    pme += page_id;

    pme->PME$L_PG_BUFF->PAGE$W_TAG = TLV$K_PAGE;
    pme->PME$L_PG_BUFF->PAGE$L_LEN = 0;
    pme->PME$L_PG_BUFF->PAGE$L_ID = page_id;
    pme->PME$L_PG_BUFF->PAGE$L_TYPE = page_type;
    pme->PME$L_PG_BUFF->PAGE$L_NUM_CHUNKS = 0;
    pme->PME$L_PG_BUFF->PAGE$L_PREV = 0;
    pme->PME$L_PG_BUFF->PAGE$L_NEXT = 0;

    /*! mark the page */

    pme->PME$B_FLAGS.PME$B_BITS.PME$V_DIRTY = TRUE;

    return(status);

}   /* END    ! bwi_topic_create */


/**** VWI$CLOSE_PAGE --  Closes the specified page.  Updates the page map **/
void VWI$CLOSE_PAGE (bkb, pgid)
BKB *bkb;	    /*  book id from VWI_BOOK_CREATE  */
unsigned pgid;	    /*  page id from VWI_TOPIC_CREATE  */
{
    PME     *pme;           /*  page map entry*/

    pme = bkb->BKB$L_PG_MAP;
    pme += pgid;

    /*! Deallocate the page buffer */
    VriFree(pme->PME$L_PG_BUFF,&CONTEXT);

    /*! Show that the buffer is gone */

    pme->PME$L_PG_BUFF = 0;
    pme->PME$L_PG_BUFF_LEN = 0;

}   /* END VWI$CLOSE_PAGE */


/**** bwi_topic_next -- Bind next page ptr to page id' *****/
BWI_ERROR bwi_topic_next (bkb, pgid, next_pgid)
BKB *bkb;	    /* from bwi_book_create */
unsigned    pgid;
unsigned    next_pgid;
{
    PME *pme;	    /* page map entry */

    if(!verify_bkid(bkb))
	return(BwiInvalidBookId);	/* make sure it is a BOOK_ID */


    pme = bkb->BKB$L_PG_MAP;
    pme += pgid;
  
    pme->PME$L_PG_BUFF->PAGE$L_NEXT = next_pgid;

    pme->PME$B_FLAGS.PME$B_BITS.PME$V_DIRTY = TRUE;

    return(BwiOK);
}   /* END    ! bwi_topic_next */


/**** bwi_topic_previous -- Bind previous page ptr to page id' ***/
BWI_ERROR bwi_topic_previous(bkb, pgid, prev_pgid)
BKB *bkb;
unsigned    pgid;
unsigned    prev_pgid;
{
    PME *pme;       /* page map entry */

    if(!verify_bkid(bkb))
        return(BwiInvalidBookId);       /* make sure it is a BOOK_ID */

    pme = bkb->BKB$L_PG_MAP;
    pme += pgid;

    pme->PME$L_PG_BUFF->PAGE$L_PREV = prev_pgid;

    pme->PME$B_FLAGS.PME$B_BITS.PME$V_DIRTY = TRUE;

    return(BwiOK);
}   /* END    ! VWI_TOPIC_PREVIOUS */

/**** VWI$WRITE_PAGE -- Write information to page buffer  ****/
BWI_ERROR VWI$WRITE_PAGE(bkb, pgid, data_len, data_addr)
BKB *bkb;           /* book id */
unsigned pgid;	    /* page id */
int data_len;	    /* length of data in bytes */
char *data_addr;    /* address of data */
{

#define BUFF$K_MAX_MOVE  65535

#define BUFF$K_WARP 16384   /* some rms magic number */
#define BUFF$K_PAD 1024    /* another rms magic # */    

    PIE *pg_ndx;	    /* page index */
    PME *pg_map;	    /* page map   */
    PIE *pie;		    /* page index entry */
    PME *pme;		    /* page map entry  */
    char *target;	    /* address in buffer to copy to */
    PAGE *page;		    /* the page structure */
    int	moved;		    /* num chars copied to buffer */
    int this_move;	    /* num chars copied this iteration */
    int	incr;
    BWI_ERROR status;




    pg_ndx = bkb->BKB$L_PG_NDX;
    pg_map = bkb->BKB$L_PG_MAP;

    pie = pg_ndx + pgid;
    pme = pg_map + pgid;

         /* check for valid topic_id */
    if((status = verify_pgid(bkb, pme, pgid)) != BwiOK)
        return(status);

    /* Illegal to append data after the page has been output */

    if(pie->PIE$L_PG_LEN != 0)
	return(BwiTopicWritten);

    /* Make sure the buffer is big enough. */

    if((pme->PME$L_PG_LEN + data_len + sizeof(PAGE) ) > pme->PME$L_PG_BUFF_LEN)
    {	if((pme->PME$L_PG_BUFF_LEN < data_len) || (data_len > BUFF$K_WARP)) 
	    incr = pme->PME$L_PG_BUFF_LEN + data_len + BUFF$K_PAD;
	else if(pme->PME$L_PG_BUFF_LEN < BUFF$K_WARP)
	    incr = pme->PME$L_PG_BUFF_LEN * 2;
	else
	    incr =  pme->PME$L_PG_BUFF_LEN + BUFF$K_WARP;

	if(pme->PME$L_PG_BUFF_LEN == 0) 
	     pme->PME$L_PG_BUFF = (PAGE *) VwiMalloc(incr,&CONTEXT,&status);
	else
	     pme->PME$L_PG_BUFF = (PAGE *) VwiRealloc(pme->PME$L_PG_BUFF,
						   incr,pme->PME$L_PG_BUFF_LEN,
						   &CONTEXT,&status);

	if(pme->PME$L_PG_BUFF == NULL)
	    return(status);

	pme->PME$L_PG_BUFF_LEN = incr;

	}

    page  = pme->PME$L_PG_BUFF;
    target = (char *) &page->PAGE$V_DATA[0];
    target += pme->PME$L_PG_LEN;


    moved = 0;
    while(moved < data_len) {
	this_move = data_len - moved;
	if(this_move > BUFF$K_MAX_MOVE)
	    this_move = BUFF$K_MAX_MOVE;
	memcpy(target, data_addr ,this_move);
	target += this_move;
	data_addr += this_move;
	moved += this_move;
	}

    pme->PME$L_PG_LEN += data_len;

    pme->PME$B_FLAGS.PME$B_BITS.PME$V_DIRTY = TRUE;
  
    return(BwiOK);
}   /* END    ! VWI$WRITE_PAGE */


/**** VWI$OUTPUT_PAGE -- Write a page out to the book file' */
BWI_ERROR VWI$OUTPUT_PAGE(bkb, pgid)
BKB *bkb;
unsigned    pgid;
{
#define LAST_FRAG 1
#define NEXT_TO_LASTFRAG 2

    PIE	    *pie;		/* page index entry */
    PME	    *pme;		/* page map entry */
    int		offset;
    unsigned    data_len;
    int current_offset, x;

    char        *frag;                  /* pointer to frag record */
    TLV         frag_tag;               /* pointer to frag record */
    TLV         *firstfrag_tag;          /* pointer to first frag record */
    int         lastfrag_len;           /* pointer to last frag record */
    PIE		frag_tail;		/* rfa that links frags */
    char        *frag_buffptr;          /* TLV header of frag record */
    int		frag_len;		/* length of frag record  */
    int		num_frags;		/* number of records needed */
    unsigned short  full_frag;		/* length of a full fragment */
    unsigned char   temp[10];
    BWI_ERROR    status;		   
#ifdef vms
    struct RAB *file_rab;       /* RAB block */
    file_rab = (struct RAB *) bkb->BKB$L_RAB;
#endif

    pie = bkb->BKB$L_PG_NDX + pgid;
    pme = bkb->BKB$L_PG_MAP + pgid;

    if(pgid == 0) {	/* first write needs RFA */
	next_rfa = 1;
	next_offset = 0;
	}

    put_int ( &pie->PIE$V_PG_RFA[0], next_rfa);
    put_short ( &pie->PIE$V_PG_RFA[4], next_offset );
  
    data_len = pme->PME$L_PG_LEN;
    
    /*! See if this is an update, or the first write for this page */

    if (pie->PIE$L_PG_LEN != 0) {	/*! this is an update */
	/*! length must remain the same */
	if ( pie->PIE$L_PG_LEN != data_len )
	    return(BwiInvalidUpdate);
	/*! can't rewrite fragmented pages (for now anyway) */
	if ( data_len > FRAG$K_LENGTH )
	    return(BwiInvalidUpdate);

#ifdef vms
        /*! Set up the RAB for RFA access and find the target page */

	if(pgid == 1) {
	    next_rfa = 3;
	    next_offset = 0;
	    }  
	put_int ( &pie->PIE$V_PG_RFA[0], next_rfa);
	put_short ( &pie->PIE$V_PG_RFA[4], next_offset );
        
	file_rab->rab$b_rac = RAB$C_RFA;
        memmove(file_rab->rab$w_rfa, pie->PIE$V_PG_RFA, 6);

        if((status = sys$find(file_rab)) != RMS$_SUC) {
            lib$signal(status);
            return(status);
            }


        /*! Point the RAB to the page buffer */

	if(pgid == 0)
	    file_rab->rab$l_rbf = &bkb->BKB$V_BKH;
	else
	    file_rab->rab$l_rbf = &bkb->BKB$V_BKH_EXT;

        file_rab->rab$w_rsz = pme->PME$L_PG_LEN;

        /*! Update the page's record */
        if((status = sys$update(file_rab)) != RMS$_SUC) {
            lib$signal(status);
	    return(status);
            }
#else
/* Implicitly, this is for page 0 only, forced by the fseek, */
	
	if(fseek(CONTEXT.file, 0, 0 ) == EOF)
	    return(BwiFailFileRead);
	
	if(fwrite ( &data_len, 1, 2, CONTEXT.file ) != 2)
	    return(BwiFailFileWrite);
	
	if(status = output_vbh ( &bkb->BKB$V_BKH ) != BwiOK)
	    return(status);
	
        if((current_offset = ftell(CONTEXT.file)) == EOF)
	    return(BwiFailFileRead);
#endif	
	}
    else {		  /*  ! First write for this page */
#ifdef vms
	if((status = use_rms(bkb,data_len,pme,pgid)) != BwiOK)
	    return(status);
#else
	/*!  See if the page will fit in a single RMS record */

	    if ( data_len <= FRAG$K_LENGTH ) {

		if((current_offset = ftell(CONTEXT.file)) == EOF)
		    return(BwiFailFileRead);

		if(fwrite ( &data_len, 1, 2, CONTEXT.file ) != 2)
		    return(BwiFailFileWrite);

		x = data_len & 1;
		if(fwrite ( pme->PME$L_PG_BUFF->PAGE$V_DATA,
			    1,
			    data_len + x, 
			    CONTEXT.file ) != data_len + x)
		    return(BwiFailFileWrite);

		offset = data_len + 2 + x + current_offset;
		next_rfa = (offset >> 9) + 1;
		next_offset = offset & 511;
		}
	    else {	    /*! Fragment the page */
		full_frag = FRAG$K_LENGTH;

		/* how many fragments */
		num_frags = (data_len - TLV$K_LENGTH +
			     FRAG$K_DATA_LENGTH - 1) / FRAG$K_DATA_LENGTH;
	    
		frag_buffptr = (char *) pme->PME$L_PG_BUFF->PAGE$V_DATA;
		lastfrag_len = data_len - ((num_frags - 1) * FRAG$K_DATA_LENGTH);

		frag_len = FRAG$K_DATA_LENGTH;

		while(num_frags) {
		    current_offset = ftell(CONTEXT.file);

		    if(num_frags == LAST_FRAG)
		    {
			    frag_len = lastfrag_len - TLV$K_LENGTH;
			    put_short(&temp[0], TLV$K_LAST_FRAG);
			    put_int(&temp[2], lastfrag_len);
		    }
		    else
		    {
			    put_short(&temp[0], TLV$K_FRAG);
			    put_int(&temp[2], FRAG$K_LENGTH);
		    }
		    
		    /* write 2 bytes with length that rms does for you */
		    if(num_frags != LAST_FRAG)
			fwrite(&full_frag,1,2,CONTEXT.file);
		    else
			fwrite(&lastfrag_len,1,2,CONTEXT.file);

		    /* if it's an odd # of bytes even it up */
		    x = frag_len & 1;	
	
		    /* Overwrite  len of first frag */
		    if(frag_buffptr == (char *) pme->PME$L_PG_BUFF->PAGE$V_DATA) {
			put_int(&frag_buffptr[2],FRAG$K_LENGTH);
			fwrite(frag_buffptr,1,TLV$K_LENGTH,CONTEXT.file);
			frag_buffptr += TLV$K_LENGTH;
			} 
		    else {
			fwrite(temp,1,TLV$K_LENGTH,CONTEXT.file);
			}

		    /* write data out */
		    fwrite(frag_buffptr,1,frag_len+x,CONTEXT.file);
		
		    /* figure out next rfa and tack it on the end */
		    if(num_frags == LAST_FRAG) 
			offset = lastfrag_len + 2 + x + current_offset;
		    else
			offset = FRAG$K_LENGTH + 2 + x + current_offset;

		    next_rfa = (offset >> 9) + 1;
		    next_offset = offset & 511;
		    put_int ( &temp[0], next_rfa);
		    put_short ( &temp[4], next_offset );

		    if(num_frags == NEXT_TO_LASTFRAG) 
			put_int(&temp[6] ,lastfrag_len);
		    else
			put_int(&temp[6] ,FRAG$K_LENGTH);
		
		    if(num_frags != LAST_FRAG) 
			fwrite(temp,1,PIE$K_LENGTH,CONTEXT.file);

		    frag_buffptr += frag_len;
		    num_frags--;
		    }
		}

#endif
	}
    pie->PIE$L_PG_LEN = data_len;


    pme->PME$B_FLAGS.PME$B_BITS.PME$V_DIRTY = FALSE;
    
    return(BwiOK);
}

output_vbh ( bkh )
BKH *bkh;
{
	unsigned char	*ck_ptr;
	int	checksum = 0;
	
	ck_ptr = (unsigned char *) bkh;
	
	fwrite ( &bkh->BKH$W_TAG,		1, 2, CONTEXT.file );
	fwrite ( &bkh->BKH$L_LEN,		1, 4, CONTEXT.file );
	fwrite ( &bkh->BKH$_BK_VERSION.major_num,1, 2, CONTEXT.file );
	fwrite ( &bkh->BKH$_BK_VERSION.minor_num,1, 2, CONTEXT.file );
	fwrite ( &bkh->BKH$T_LMF_PRODUCER[0],	1, 48, CONTEXT.file );
	fwrite ( &bkh->BKH$Q_LMF_PRODUCT_DATE[0], 1, 8, CONTEXT.file );
	fwrite ( &bkh->BKH$L_LMF_PRODUCT_VERSION, 1, 4, CONTEXT.file );
	fwrite ( &bkh->BKH$L_NUM_PAGES,		1, 4, CONTEXT.file );
	fwrite ( &bkh->BKH$L_NUM_CHUNKS,	1, 4, CONTEXT.file );
	fwrite ( &bkh->BKH$L_NUM_DIRS,		1, 4, CONTEXT.file );
        fwrite ( &bkh->BKH$L_NUM_FONTS,		1, 4, CONTEXT.file );
	fwrite ( &bkh->BKH$L_MAX_FONT_ID,	1, 4, CONTEXT.file );
	fwrite ( &bkh->BKH$V_PG_NDX_RFA[0],	1, 6, CONTEXT.file );
	fwrite ( &bkh->BKH$W_PG_NDX_LEN,	1, 2, CONTEXT.file );
	fwrite ( &bkh->BKH$L_BKH_EXT_PGID,	1, 4, CONTEXT.file );
	fwrite ( &bkh->BKH$L_CK_NDX_PGID,	1, 4, CONTEXT.file );
	fwrite ( &bkh->BKH$L_CK_TITLES_PGID,	1, 4, CONTEXT.file );
	fwrite ( &bkh->BKH$L_SYMBOL_PGID,	1, 4, CONTEXT.file );
	fwrite ( &bkh->BKH$L_FONT_PGID,		1, 4, CONTEXT.file );
	fwrite ( &bkh->BKH$L_FIRST_DATA_PGID,	1, 4, CONTEXT.file );
	fwrite ( &bkh->BKH$L_COPYRIGHT_CKID,	1, 4, CONTEXT.file );
	fwrite ( &bkh->BKH$B_NAME_LEN,		1, 1, CONTEXT.file );
	fwrite ( &bkh->BKH$T_NAME[0],		1, 128, CONTEXT.file );
	fwrite ( &bkh->BKH$Q_BUILD_DATE[0],	1, 8, CONTEXT.file );
	fwrite ( &bkh->BKH$T_SYMBOL[0],		1, 32, CONTEXT.file );
	fwrite ( &bkh->BKH$B_LMF_ALTPROD_COUNT, 1, 1, CONTEXT.file );
        fwrite ( &bkh->BKH$V_FILLER[0],		1, 722, CONTEXT.file );

/* calculate checksum */
	while ( ck_ptr != (unsigned char *)&bkh->BKH$L_CHECKSUM )
	{	checksum += *ck_ptr & 255;
		ck_ptr++;
	}
	
	fwrite ( &checksum, 1, 4, CONTEXT.file );
}

put_short ( ptr, num )
char *ptr;
short num;
{
	  *ptr++ = num & 255;
	  *ptr = ( num >> 8 ) & 255;
}


put_int ( ptr, num )
char *ptr;
int num;
{
	  *ptr++ = num & 255;
	  *ptr++ = ( num >> 8 ) & 255;
	  *ptr++ = ( num >> 16 ) & 255;
	  *ptr = ( num >> 24 ) & 255;
}
#ifdef vms
BWI_ERROR use_rms(bkb,data_len,pme,pgid)
BKB *bkb;
int data_len;
PME *pme;
unsigned pgid;
{
#define	FRAG$K_MAX_COUNT = 64;		/*! max # fragments/page */
    struct RAB *file_rab;	/* RAB block */
    int	num_frags;	    /* number of records needed */
    int	count;
    int	save_rfa_len;  
    char	*save_rfa;		/* RFA's of fragments*/
    char	*frag;			/* pointer to frag record */
    TLV		*temp_tlvfrag;		/* pointer to frag record */
    char	*last_frag;		/* pointer to last frag record */
    char	*frag_head;		/* TLV header of frag record */
    PIE		*frag_tail;		/* PIE tail of frag record */
    int		frag_len;		/* length of frag record  */
    char	save_head[TLV$K_LENGTH];/* for saving overlaid data */
    int		offset;
    BWI_ERROR    status;		/* status returned from rms call */

    PIE	    *pie;		/* page index entry */
    pie = bkb->BKB$L_PG_NDX + pgid;


    file_rab = (struct RAB *) bkb->BKB$L_RAB;

	/*!  See if the page will fit in a single RMS record */

	if(data_len <= FRAG$K_LENGTH ) {
	    /* ! Point the RAB to the page buffer */

	    file_rab->rab$l_rbf = pme->PME$L_PG_BUFF->PAGE$V_DATA;
	    file_rab->rab$w_rsz = data_len;

	    /*! Write the record to the end of file */

	    file_rab->rab$b_rac = RAB$C_SEQ;  /*! Sequential access */
	    if((status = sys$put(file_rab)) != RMS$_SUC) {
		lib$signal(status);
		return(status);
		}
	    }
	else {	    /*! Fragment the page */
	    
	    /* how many fragments */
	    num_frags = (data_len - TLV$K_LENGTH 
		+ FRAG$K_DATA_LENGTH - 1) / FRAG$K_DATA_LENGTH;
	    
	    save_rfa_len = 6 * num_frags; /* how much memory do we need */ 
	    save_rfa = (char *) VwiMalloc(save_rfa_len,&CONTEXT,&status);
	    if(save_rfa == NULL)
		return(status);

	   /*  Fragments must be written in reverse order (final...initial) so*/
	   /*  the next Frag RFA's can be filled in, but we want them in    */
	   /*  (initial...final order in the file, so we first write a dummy*/
	   /*  record for each frag sequentially, saving the RFA's, then    */
	   /*  write out the real data using the RFA's.			    */

	    file_rab->rab$b_rac = RAB$C_SEQ;  /* Sequential access */
	    file_rab->rab$l_rbf = pme->PME$L_PG_BUFF->PAGE$V_DATA;
	    file_rab->rab$w_rsz = FRAG$K_LENGTH;

	    count = 0;
	    while(count < num_frags - 1) {
		if((status = sys$put(file_rab)) != RMS$_SUC) {
		    lib$signal(status);
		    return(status);
		    }
		memmove(save_rfa, file_rab->rab$w_rfa, 6);
		save_rfa += 6;
		count++;
		}	    
	    /*! set up pointer and length of last fragment */

	    frag = pme->PME$L_PG_BUFF->PAGE$V_DATA;
	    offset = FRAG$K_DATA_LENGTH * (num_frags - 1);
	    frag += offset;
	    last_frag = frag;
	    frag_len = data_len - FRAG$K_DATA_LENGTH 
		* (num_frags - 1);

	    file_rab->rab$l_rbf = last_frag;
	    file_rab->rab$w_rsz = frag_len;
	    if((status = sys$put(file_rab)) != RMS$_SUC){
                    lib$signal(status);
		    return(status);
		    }
            
	    memmove(save_rfa, file_rab->rab$w_rfa, 6);	    
	    
	    /*! Set up the RAB for RFA access  */

	    file_rab->rab$b_rac = RAB$C_RFA;

	    while(frag >= pme->PME$L_PG_BUFF->PAGE$V_DATA) {
		/*! save the page data to be overlaid by the frag header */
		memcpy(save_head, frag, TLV$K_LENGTH);

		/*! set up the frag header */
		temp_tlvfrag = (TLV *) frag;
		if(frag == last_frag)
		    temp_tlvfrag->TLV$W_TAG = TLV$K_LAST_FRAG;
		else if(frag != pme->PME$L_PG_BUFF->PAGE$V_DATA)
		    temp_tlvfrag->TLV$W_TAG = TLV$K_FRAG;

		temp_tlvfrag->TLV$L_LEN = frag_len;

		/*! Point the RAB to the page fragment */

		file_rab->rab$l_rbf = frag;
		file_rab->rab$w_rsz = frag_len;

		/* set up the RFA, find the target page, and update the record*/

		memmove(file_rab->rab$w_rfa, save_rfa, 6);
		save_rfa -= 6;

		/*$$MOVE_RFA (.SAVE_RFA + 6 * .COUNT, file_rab->rab$w_rfa]); */
		if((status = sys$find(file_rab)) != RMS$_SUC){
                    lib$signal(status);
		    return(status);
		    }
		if((status = sys$update(file_rab)) != RMS$_SUC) {
		    lib$signal(status);
                    return(status);
                    }

		/*! restore the page data overlaid by the frag header */
		memmove(frag, save_head, TLV$K_LENGTH);

		/* set up the frag tail for the next frag to point to this one*/
		frag_head = frag;
		frag_head += TLV$K_LENGTH;

		frag_tail = (PIE *) frag_head;	/* + TLV$K_LENGTH; */

		memmove(frag_tail->PIE$V_PG_RFA, file_rab->rab$w_rfa, 6);
		/*$$MOVE_RFA (file_rab->rab$w_rfa], FRAG_TAIL[PIE$V_PG_RFA]); */
		frag_tail->PIE$L_PG_LEN = frag_len;

		/*! set up the frag head and length for the next frag */
		frag = (char *) frag;
		frag -= FRAG$K_DATA_LENGTH; /*FRAG = .FRAG - FRAG$K_DATA_LENGTH;  */
		frag_len = FRAG$K_LENGTH;
		count--;
		}
	    }

	/*  ! Save the RFA and length in the page index */
	memmove(pie->PIE$V_PG_RFA,file_rab->rab$w_rfa, 6);

    return(BwiOK);
 }

#endif

