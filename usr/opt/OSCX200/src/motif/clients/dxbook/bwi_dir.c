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
/* DEC/CMS REPLACEMENT HISTORY, Element VWI-DIR.C*/
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
**   This module implements the directory management routines for VWI
**
** AUTHORS:
**
**   Joseph M. Kraetsch
**
** CREATION DATE: 15-JUN-1987
**
** MODIFICATION HISTORY:
** 06-Mar-1990
**	N.R.Haslock - hacks to alleviate alignment problems
** 24-JAn-1991 
**	M.J. FITZELL - V2.1 BWI
*/

/*  
**  INCLUDE FILES  
*/
# include   "bxi_def.h"

#ifdef vms
globalref int  VWI$GL_PREV_PAGE;
globalref  VriContext CONTEXT;
#else
extern    int  VWI$GL_PREV_PAGE;
extern     VriContext CONTEXT;
#endif

extern	VWI$CREATE_PAGE_CONTEXT();
extern	bwi_topic_close ();
extern	VWI$WRITE_PAGE ();
extern	bwi$write_bkh ();


int verify_drid(bkb,drid)
BKB  *bkb;        /*  Book id (from VWI$CREATE_BOOK)              */
DRB *drid;
{
    DRB *drb;
    VriQueue *que_head = (VriQueue *) bkb->BKB$L_DRB_LIST;
    drb = (DRB *) que_head->flink;
    while (TRUE)
    {

        if (drb == (DRB *)drid)
            return(TRUE);      /*  found it -- exit search loop  */

        if ( VXI_QEND(que_head,drb) ) 
            return(FALSE);

        drb = (DRB *)drb->DRB$QUEUE.flink;  /*  try next drb */
    }
}

/*
**  bwi_directory_close	-- close a directory
**
*/
BWI_ERROR bwi_directory_close (bkb, drb)
BKB  *bkb;	/*  Book id (from VWI$CREATE_BOOK)		*/
DRB  *drb;	/*  Directory id (from bwi_directory_create)    */
{
    char    *vdh;
    int	    save_prev_page;

    if(!verify_bkid (bkb))
        return(BwiInvalidBookId);

    if(! verify_drid (bkb,drb))
	return(BwiInvalidDirId);
 
    vdh = (char *)drb->DRB$L_VDH;

    /*  close the dir page  */
    save_prev_page = VWI$GL_PREV_PAGE;
    VWI$GL_PREV_PAGE = drb->DRB$L_PGID;
    bwi_topic_close (drb->DRB$L_BKID, drb->DRB$L_PGID, 0, 0);
    VWI$GL_PREV_PAGE = save_prev_page;

    put_int ( &vdh[11], drb->DRB$L_NUM_ENTRIES );
    /*  VDH is already in book header--put there on VWI$CREATE 
     *  so don't need to write it again, the updated entry count will
     *  get written when the book is closed  
     */

    VXI_REMQ(drb);
    return(BwiOK);

};	/*  end of bwi_directory_close  */


/*
**  bwi_directory_create  -- create a directory.
**
**	The book must be open.
**
**  Returns:  Directory id
*/
BWI_ERROR bwi_directory_create (bkb, dir_name, dir_flags, drid)
BKB        *bkb;	    /*  Book id (from VWI$CREATE_BOOK)  */
char       *dir_name;	    /*  Directory name  (= title?)      */
unsigned    dir_flags;	    /*  Directory flags		        */
unsigned int *drid;	    /*  address of dir block*/
{
    BKH	    *vbh;	/*  VOILA book header			*/
    DRB	    *drb;	/*  Directory block			*/
    unsigned char    vdh[VDH$K_LENGTH];	/*  VOILA directory header		*/
    PME	    *pme;	/*  page map entry for book header	*/
    char    packet[6];	/*  TLV packet for dir page tags	*/
    int	    size;
    unsigned int pgid;  /* ID of new page - from VWI$CREATE_PAGE_CONTEXT */
    unsigned int status= 0;
    VriQueue *que_head;
    unsigned long dir_counter;

    if(!verify_bkid (bkb))
        return(BwiInvalidBookId);

    /* check len of string only an unsigned char in length field */
    if((size = (strlen(dir_name) + 1)) > 255)
	return(BwiStrTooLong);

    /* Make sure there is a valid string*/
    if(size == 1)
	return(BwiDirNoTitle);

    bkb->BKB$V_BKH.BKH$L_NUM_DIRS++;

    if(dir_flags == TOC_FLAGS)
        bkb->BKB$V_BKH_EXT.BKEH$L_NUM_SYS_DIRS++;  /* this is a system directory */

    dir_counter = bkb->BKB$V_BKH.BKH$L_NUM_DIRS << 24;
    dir_counter = dir_counter & 0xFF000000;   /*make sure entry fields are Null
  
    /*  create and init. the directory block (DRB)  */

    drb = (DRB *) VwiMalloc( sizeof (DRB), &CONTEXT, &status);
    if(drb == NULL)
	return(status);
    if((status = VWI$CREATE_PAGE_CONTEXT (bkb,&pgid)) != BwiOK)
	return(status);
    
    /* Put this directory in the book's directory list  */
    que_head = (VriQueue *) bkb->BKB$L_DRB_LIST;
    VXI_INSQH(que_head,drb);


    drb->DRB$L_PGID = pgid;
    drb->DRB$L_NUM_ENTRIES = 0;
    drb->DRB$L_BKID = (unsigned) bkb;

    /*  build a header packet and write it to the dir. page  */

    put_short ( &packet[0], TLV$K_DIR_PG );
    put_int   ( &packet[2], TLV$K_LENGTH );

    if((status = VWI$WRITE_PAGE (bkb, drb->DRB$L_PGID,
				 TLV$K_LENGTH, packet)) != BwiOK)
	return(status);


    /* Initialize the directory header  */

    put_short ( &vdh[0], TLV$K_VDH );
    put_int   ( &vdh[2], VDH$K_LENGTH + strlen (dir_name) + 1 );
    put_int   ( &vdh[6], dir_counter);	    /* object id  */
    vdh[10] = dir_flags;
    put_int   ( &vdh[11], 0 );
    put_int   ( &vdh[15], drb->DRB$L_PGID );
    vdh[19] = (BWI_BYTE) size;

    /*  write the VDH to the BKH 
     *  first save the address in the book header where the directory header 
     *  vdh will end up--this will allow us to update the entry count later.
     */
    drb->DRB$L_VDH = (unsigned)(&bkb->BKB$V_BKH.BKH$V_FILLER[0])
                     + bkb->BKB$V_BKH.BKH$L_LEN - BKH_C_FIXED_LENGTH;

    if((status = bwi$write_bkh (bkb, VDH$K_LENGTH, vdh)) != BwiOK)
	return(status);
    if((status = bwi$write_bkh (bkb, strlen (dir_name) + 1, dir_name)) != BwiOK)
	return(status);

    /* The address of the DRB is the Directory-Id  */
    *drid = (unsigned int) drb;

    /* return status value */
    return (status);
};	/*  end of bwi_directory_create  */


/*
**  bwi_directory_entry -- Add an entry to a directory
*/
BWI_ERROR bwi_directory_entry (bkb, drb, ref_count, ref_list, level,  
    width, height, data, data_len, data_type, title, dir_entry_id)
BKB	   *bkb;	    /*  Book id (from VWI$CREATE_BOOK)		    */
DRB        *drb;	    /*  Directory id (from bwi_directory_create)    */
unsigned    ref_count;	    /*  count of referenced data chunk id's	    */
unsigned   *ref_list[];     /*  list of referenced data chunk id's	    */
unsigned    level;	    /*  Header level (top level = 1)		    */
unsigned    width;	    /*  total width of directory entry		    */
unsigned    height;	    /*  total height of directory entry		    */
unsigned    char *data;	    /*  address of buffer containing display data   */
unsigned    data_len;	    /*  length of display data			    */
unsigned    data_type;	    /*  type id of display data			    */
char	   *title;	    /*  Ascii string identifying entry		    */
BWI_OBJECT_ID *dir_entry_id; /*  Entries identification			    */
{
    unsigned char    dre[DRE$K_LENGTH];	    /*  dir. entry block  */
    unsigned int status = 0;
    int str_length;
    
    
    if(!verify_bkid (bkb))
        return(BwiInvalidBookId);
    
    /* bookreader can only handle single increments */
    if(( bkb->BKB$B_DIR_LEVEL + 1 ) < level)
        return(BwiDirLevelSkip);

    bkb->BKB$B_DIR_LEVEL = level;
 
    if(! verify_drid(bkb,drb))
	return(BwiInvalidDirId);

    /* check len of string only an unsigned char in length field */
    if((str_length = (strlen(title) + 1)) > 255)
	return(BwiStrTooLong);
    
    if(str_length == 1)
	return(BwiDirEntNoTitle);

    put_short ( &dre[0], TLV$K_DIR_ENTRY );
    put_int   ( &dre[2], DRE$K_LENGTH + data_len +
	        ref_count * sizeof (unsigned) + strlen (title) + 1 );
    dre[6] = level;
    dre[7] = data_type;  

    put_int   ( &dre[ 8], width );
    put_int   ( &dre[12], height );
    dre[16] = (BWI_BYTE) str_length;  
    dre[17] = ref_count;

    put_short ( &dre[18], data_len );

/*MAYBE WE SHOULD TRY TO PACK ALL THIS INTO ONE CALL? */

    if((status = VWI$WRITE_PAGE (drb->DRB$L_BKID, drb->DRB$L_PGID,
				 DRE$K_LENGTH, dre)) != BwiOK)
	return(status);
    if((status = VWI$WRITE_PAGE (drb->DRB$L_BKID, drb->DRB$L_PGID,
				 data_len, data)) != BwiOK)
        return(status);
    if((status = VWI$WRITE_PAGE (drb->DRB$L_BKID, drb->DRB$L_PGID,
				 strlen (title) + 1, title)) != BwiOK)
        return(status);
    if((status = VWI$WRITE_PAGE (drb->DRB$L_BKID, drb->DRB$L_PGID, 
		    ref_count * sizeof (unsigned), ref_list)) != BwiOK)
        return(status);

    drb->DRB$L_NUM_ENTRIES++;
    *dir_entry_id = drb->DRB$L_NUM_ENTRIES;

    return(status);
};	/*  end of VWI_DIRECTORY_ENTRY  */

