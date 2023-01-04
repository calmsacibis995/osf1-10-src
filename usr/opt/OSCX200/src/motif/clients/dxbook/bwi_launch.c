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
**                         COPYRIGHT (c) 1991 BY                            *
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
**   This module implements the "launch" data storage routines for the BWI.
**
** AUTHORS:
**
**   Michael Fitzell
**
** CREATION DATE: 4-DEC-1991
**
** MODIFICATION HISTORY:
*/

/*  
**  INCLUDE FILES  
*/
# include   "bxi_def.h"
# include   "chunk.h"

#ifdef vms
globalref BKB *VXI$GQ_BOOKLIST;
globalref VriContext CONTEXT;
#else
extern VriContext CONTEXT;
#endif

BWI_ERROR    bwi_topic_data_launch();
BWI_ERROR    bwi_topic_data_chunk();
BWI_ERROR   verify_pgid();
int	    verify_bkid();
int	    verify_ckid();
unsigned    VWI$CKID_TO_PGID();


/*
/* typedef struct  _BWI_LAUNCH
/*        {
/*            struct _BWI_LAUNCH  *prev;
/*            struct _BWI_LAUNCH  *next;
/*            unsigned            launch_id;
/*	      unsigned		  topic_id;
/*	      unsigned		  data_length;
/*            char                *data;
/*        }   BWI_LAUNCH;
/*      
*/

BWI_ERROR bwi_topic_launch_init(bkb,topic_id, launch_id)
BKB         *bkb;           /*  book id from bwi_book_create  */
unsigned    topic_id;	    /*  topic id from bwi_topic_data */
unsigned    *launch_id;      /*  parent id from  bwi_launch_init  */
{
    BWI_LAUNCH	*launch;
    BWI_ERROR   status;

    /* check bookid */

    if(!verify_bkid (bkb))
        return(BwiInvalidBookId);

    /* initialize launch data structs */

    launch = (BWI_LAUNCH *)VwiMalloc((sizeof(BWI_LAUNCH)),&CONTEXT,&status);

    if(launch == NULL)
	return(status);

    /* new launch objects are put on the top of the launch list */
    /* so prev ptr is always initialized to NULL		*/
    launch->prev = NULL;

    if(bkb->BKB$L_NUM_LAUNCHES == 0) {
	launch->next = NULL;
	bkb->BKB$L_LAUNCH_LIST = launch;
	}
    else {
	launch->next = bkb->BKB$L_LAUNCH_LIST;
	bkb->BKB$L_LAUNCH_LIST->prev = launch;
	bkb->BKB$L_LAUNCH_LIST = launch;
	}	

    launch->data = NULL;
    launch->data_length = 0;
    launch->topic_id = topic_id;
    /* increment launch id */

    *launch_id = launch->launch_id = ++bkb->BKB$L_NUM_LAUNCHES;

    return(BwiOK);
}


/*
**  bwi_topic_data_launch -- store  launch data 
**
*/
BWI_ERROR bwi_topic_launch_data (bkb, launch_id, title, data_addr, data_len, 
				 data_type)
BKB	    *bkb;           /*  book id from bwi_book_create  */
unsigned    launch_id;      /*  parent id from  bwi_launch_init  */
char        *title;         /*  title of data chunk  */
char        *data_addr;     /*  address of the data to store  */
unsigned    data_len;       /*  number of bytes to store  */
unsigned    data_type;      /*  data format identifier  */

{
#define BUFF$K_MAX_MOVE  65535

    BWI_LAUNCH	    *launch_ptr;
    BKR_U32_INT	    loop_cntrl;
    char	    tlv[6];
    char	    *current_ptr;
    int moved;			    /* num chars copied to buffer */
    int this_move;		    /* num chars copied this iteration */
    int size_needed;
    BWI_ERROR status;
    int name_len;
    /* check bookid */

    if(!verify_bkid (bkb))
        return(BwiInvalidBookId);

    /* set up ptr to right launch sequence*/

    launch_ptr = bkb->BKB$L_LAUNCH_LIST;

    while(launch_ptr->next != NULL) {
	if(launch_id == launch_ptr->launch_id)
	    break;

	launch_ptr = launch_ptr->next;    
	if(loop_cntrl++ > bkb->BKB$L_NUM_LAUNCHES)
	    return(BwiInvalidLaunchId);
	}

    if((data_type == LAUNCH_SCRIPT) || (data_type == LAUNCH_DATA)) {
	name_len = strlen(title) + 1;
	status = bwi_topic_launch_data(bkb,
                                       launch_id,
                                       NULL,
                                       title,
                                       name_len,
                                       LAUNCH_SCRIPT_NAME);
	}

    /* store data_type as the tag and the data_len as the length*/
    put_short ( &tlv[0], data_type );
    put_int   ( &tlv[2], data_len );

    size_needed = TLV$K_LENGTH + data_len;
    
    if(launch_ptr->data == NULL)
	launch_ptr->data = (char *) VwiMalloc(size_needed,&CONTEXT,&status);
    else {
	size_needed += launch_ptr->data_length;
	launch_ptr->data = (char *) VwiRealloc(launch_ptr->data,
					       size_needed,
					       launch_ptr->data_length,
					       &CONTEXT,&status);
        }

    if(launch_ptr->data == NULL)
	return(status);

    current_ptr = launch_ptr->data;
    current_ptr += launch_ptr->data_length;

    /* put the tag and length in the buffer */

    memcpy(current_ptr, tlv, TLV$K_LENGTH);
    current_ptr += TLV$K_LENGTH;
        
    /* cop data from data_addr to launch_ptr */
    moved = 0;
    while(moved < data_len) {
        this_move = data_len - moved;
        if(this_move > BUFF$K_MAX_MOVE)
            this_move = BUFF$K_MAX_MOVE;
        memcpy(current_ptr, data_addr ,this_move);
        current_ptr += this_move;
        data_addr += this_move;
        moved += this_move;
        }

    launch_ptr->data_length = size_needed;
    
    return(BwiOK);
}

BWI_ERROR bwi_topic_launch_store(bkb, pgid, launch_id, title, symbol_name,ckid)
BKB         *bkb;           /*  book id from bwi_book_create  */
unsigned    pgid;           /*  page id from bwi_topic_create  */
unsigned    launch_id;      /*  parent id from  bwi_launch_init  */
char	    *title;
char	    *symbol_name;
unsigned    *ckid;
{
    BWI_LAUNCH	    *launch_ptr;
    BKR_U32_INT	    loop_cntrl;
    BWI_ERROR	    status;
    unsigned	    chunk_id;    
    PME		    *pme;               /* page map entry  */
    /* check bookid and launch id */

    if(!verify_bkid (bkb))
        return(BwiInvalidBookId);

    /* check for valid topic_id */
    pme = bkb->BKB$L_PG_MAP;
    pme +=  pgid;
    if((status = verify_pgid(bkb, pme, pgid)) != BwiOK)
        return(status);


    /* set up ptr to right launch sequence*/

    launch_ptr = bkb->BKB$L_LAUNCH_LIST;

    while(launch_ptr->next != NULL) {
	if(launch_id == launch_ptr->launch_id)
	    break;

	launch_ptr = launch_ptr->next;    
	if(loop_cntrl++ > bkb->BKB$L_NUM_LAUNCHES)
	    return(BwiInvalidLaunchId);
	}

    /* store the whole launch sequence in a "launch Chunk" */
    status =  bwi_topic_data_chunk (bkb, pgid, title, 0, 0,
				    launch_ptr->data, launch_ptr->data_length,
				    CHUNK_LAUNCH, &chunk_id);

    /* we need to free up memory and fix up the links */
    (void) free(launch_ptr->data);
    

    if(status != BwiOK)
	return(status);

    *ckid = chunk_id;

    /* need a symbol name if it's going to be addressable */
    status = bwi_symbol_define(bkb,chunk_id,symbol_name,TRUE);

    return(status);
    
}
