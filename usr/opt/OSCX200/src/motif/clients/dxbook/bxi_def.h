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
/* DEC/CMS REPLACEMENT HISTORY, Element VXI-DEF.H*/
/*  3     6-SEP-1990 12:52:27 FITZELL "change major ID to 3"*/
/* *2     9-JUL-1990 15:52:49 FITZELL ""*/
/* *1    24-APR-1990 15:44:27 FITZELL "creating initial elements that shipped with V2 VWI"*/
/* DEC/CMS REPLACEMENT HISTORY, Element VXI-DEF.H*/
/*  DEC/CMS REPLACEMENT HISTORY, Element VXI-DEF.H */
/*  *6    11-AUG-1989 11:03:29 KRAETSCH "remove lib$ from include syntax" */
/*  *5    31-OCT-1988 18:25:11 KRAETSCH "bump eco level due to new font names" */
/*  *4    14-JUL-1988 11:45:20 FERGUSON "Fix bugs with rags and image graphics" */
/*  *3    22-JUN-1988 16:54:42 FERGUSON "Performance fixes" */
/*  *2    12-APR-1988 16:42:13 FERGUSON "FT2 changes" */
/*  *1     6-JAN-1988 16:42:39 FERGUSON "Initial Entry" */
/*  DEC/CMS REPLACEMENT HISTORY, Element VXI-DEF.H */
/*
** MODULE VXI-DEF (IDENT = X-1) =
**
** COPYRIGHT (c) 1988 BY
** DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
** ALL RIGHTS RESERVED.
**
** THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
** ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
** INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
** COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
** OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
** TRANSFERRED.
**
** THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
** AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
** CORPORATION.
**
** DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
** SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
*/

/*
**  FACILITY:
**
**      VRI - VOILA Reader Interface
**	VWI - VOILA Writer Interface
**
**  ABSTRACT:
**
**	This is the primary require file for the VRI and VWI Interfaces
**
**  AUTHORS:
**
**      Joseph M. Kraetsch
**
**
**  CREATION DATE:     28-APR-1987
**
**  MODIFICATION HISTORY:
*/

/*
**  INCLUDE FILES
*/

#ifdef vms
# include   <rms.h>
#endif
# include   <stdio.h>
# include   "bxi_ods.h"			/*  Data Structures		    */

/*
**  MACRO DEFINITIONS
**/
# define    TRUE		    1
# define    FALSE		    0
# define    PGID$K_LENGTH	    4	/*  size of a page id (longword)    */
# define    VOILA$K_MAJOR_VERSION   2	/*  software version number	    */
# define    VOILA$K_MINOR_VERSION   1	/*  ECO level			    */
#ifdef vms
# define    VWI$T_DEF_BOOKNAME	    "DECW$BOOK:.DECW$BOOK"
#else
# define    VWI$T_DEF_BOOKNAME	    "default.decw$book"
#endif

# define    MAX(a,b)		    ((a)<(b)?(b):(a))
# define    MIN(a,b)		    ((a)<(b)?(a):(b))


/* Queue handling macros
 * 
 * The following macros perform some simple queue manipulations:
 *
 * QINIT(qhead)  -- initialize and empty queue
 * QEMPTY(qhead) -- test for an empty queue
 * QEND(qhead,entry) - test if at end of the queue
 * INSQH(qhead,entry) -- insert new entry at head of a queue
 * INSQT(qhead,entry) -- insert new entry at tail of a queue
 * REMQ(entry) -- remove entry from a queue
 *
 * Parameters:
 *
 * qhead -- points to the queue header
 *
 * entry -- points to the queue entry to be inserted or removed from
 *          the  queue.  a VriQueue must be the first field in the
 *          data structure.
 */

#define _FLINK(queue) ((VriQueue *)(queue))->flink
#define _BLINK(queue) ((VriQueue *)(queue))->blink

#define VXI_QINIT(qhead) \
    { \
        _FLINK(qhead) = (VriGenericPtr)(qhead); \
        _BLINK(qhead) = (VriGenericPtr)(qhead); \
    }

#define VXI_QEMPTY(qhead) \
        (_FLINK(qhead) == ((VriGenericPtr)(qhead)))

#define VXI_QEND(qhead,entry) \
        (((VriGenericPtr)(entry) == ((VriGenericPtr)(qhead))) \
         || ((entry) == NULL) \
        )

#define VXI_INSQH(qhead,entry) \
    { \
        _FLINK(entry) = _FLINK(qhead) ; \
        _BLINK(entry) = (VriGenericPtr)(qhead) ; \
        _BLINK(_FLINK(qhead)) = (VriGenericPtr)(entry); \
        _FLINK(qhead) = (VriGenericPtr)(entry) ; \
    }

#define VXI_INSQT(qhead,entry) \
    { \
        _FLINK(entry) = (VriGenericPtr)(qhead) ; \
        _BLINK(entry) = _BLINK(head) ; \
        _FLINK(_BLINK(qhead)) = (VriGenericPtr)(entry) ; \
        _BLINK(qhead) = (VriGenericPtr)(entry) ; \
    }

#define VXI_REMQ(entry) { \
    _BLINK(_FLINK(entry)) = _BLINK(entry); \
    _FLINK(_BLINK(entry)) = _FLINK(entry); \
}




/*
**   __VERIFY_BKID -- check validity of a BKID (BooK ID)
**
**	BKID is the value of the book id.
*/
/*# define    __verify_bkid(bkid)			\
/*    {						\
/*	BKB *lbkid = (BKB *)(bkid);		\
/*	BKB *bkb = VXI$GQ_BOOKLIST[FLINK];	\
/*						\
/*	while (bkb != lbkid)			\
/*	{					\
/*	    if (bkb == VXI$GQ_BOOKLIST)		\
/*		signal (VOILA$_INVBOOKID);	\
/*	    bkb = bkb->BKB$L_FLINK;		\
/*	}					\
/*    }		
    

/*
**   __VERIFY_PGID -- check validity of a page id
**
**	BKID is the value of the book id.
**	PGID is the value of the page id.
*/
/*# define    __VERIFY_PGID(BKID, PGID)  VXI$VERIFY_PGID (BKID, PGID) 
*/

/*
**   __MOVE_RFA -- Copy RFA from source to dest.
*/
# define    __MOVE_RFA(SRC, DST)	    VXI$MOVE_RFA (SRC, DST) 

/*
**   SIGNAL_IF_ERROR -- execute command and check status
*/
# define    SIGNAL_IF_ERROR(command)		\
    {						\
	int	lstatus;			\
						\
	lstatus = (command);			\
	if (!lstatus){perror(lstatus);exit(lstatus)} \
    }

/*
** BUGCHECK - signals the bugcheck error (shouldn't happen)
*/

# define    BUGCHECK	SIGNAL_IF_ERROR(VOILA$_BUGCHECK)

/*
**   LOG_EVENT -- output a message to the log file.
*/
# define    __LOG_EVENT(INSTRING)	    VXI$LOG_EVENT (INSTRING) 
