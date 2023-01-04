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
/* DEC/CMS REPLACEMENT HISTORY, Element BXI_PRIVATE_DEF.H*/
/* *5    22-JUL-1992 14:51:51 GOSSELIN "streamlf"*/
/* *4     3-MAR-1992 17:13:19 KARDON "UCXed"*/
/* *3    22-NOV-1991 17:00:22 BALLENGER "Conditionalize definition of MIN and MAX"*/
/* *2    13-NOV-1991 14:27:46 GOSSELIN "conditionalized MAX and MIN"*/
/* *1    16-SEP-1991 12:48:54 PARMENTER "common BRI/BWI interfaces"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BXI_PRIVATE_DEF.H*/
/* DEC/CMS REPLACEMENT HISTORY, Element BXI_PRIVATE_DEF.H*/
/* *3    25-JAN-1991 17:03:59 FITZELL "V3_EFT_24_JAN"*/
/* *2    12-DEC-1990 13:48:27 FITZELL "V3 IFT update"*/
/* *1     8-NOV-1990 11:27:19 FITZELL "V3 IFT"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BXI_PRIVATE_DEF.H*/

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
 *      bxi_private_def.h
 *
 * Abstract:
 *
 *      Private BXI (Book Common Interface) definitions for both BRI/BWI
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
 *  	 7-Sep-1990 James A. Ferguson 
 *  	    	    Bump major version to 3.
 *
 *  	15-Aug-1990 James A. Ferguson
 *  	    	    Create new module and separate on-disk from memory 
 *  	    	    definitions.
 *
 */


#ifndef _BXI_PRIVATE_DEF_H
#define _BXI_PRIVATE_DEF_H



/* Defines */

#define TRUE 1
#define FALSE 0

#if !defined(ALPHA)
#ifndef MAX
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#endif 
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif 
#endif

#define BXI_C_PT_UNDEFINED 0
#define BXI_C_MAJOR_VERSION 3
#define BXI_C_MINOR_VERSION 0

#define BXI_C_SHELF_ENTRY_TYPE_MIN 1
#define BXI_C_SHELF_ENTRY_TYPE_BOOK 1
#define BXI_C_SHELF_ENTRY_TYPE_SHELF 2
#define BXI_C_SHELF_ENTRY_TYPE_TITLE 3
#define BXI_C_SHELF_ENTRY_TYPE_COMMENT 4
#define BXI_C_SHELF_ENTRY_TYPE_MAX 4


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
 *          the  queue.  a BXI_QUEUE must be the first field in the
 *          data structure.
 */

#define _FLINK(queue) ((BXI_QUEUE *)(queue))->flink
#define _BLINK(queue) ((BXI_QUEUE *)(queue))->blink

#define BXI_QINIT(qhead) \
    { \
        _FLINK(qhead) = (BMD_GENERIC_PTR)(qhead); \
        _BLINK(qhead) = (BMD_GENERIC_PTR)(qhead); \
    }

#define BXI_QEMPTY(qhead) \
        (_FLINK(qhead) == ((BMD_GENERIC_PTR)(qhead)))

#define BXI_QEND(qhead,entry) \
        (((BMD_GENERIC_PTR)(entry) == ((BMD_GENERIC_PTR)(qhead))) \
         || ((entry) == NULL) \
        )

#define BXI_INSQH(qhead,entry) \
    { \
        _FLINK(entry) = _FLINK(qhead) ; \
        _BLINK(entry) = (BMD_GENERIC_PTR)(qhead) ; \
        _BLINK(_FLINK(qhead)) = (BMD_GENERIC_PTR)(entry); \
        _FLINK(qhead) = (BMD_GENERIC_PTR)(entry) ; \
    }

#define BXI_INSQT(qhead,entry) \
    { \
        _FLINK(entry) = (BMD_GENERIC_PTR)(qhead) ; \
        _BLINK(entry) = _BLINK(qhead) ; \
        _FLINK(_BLINK(qhead)) = (BMD_GENERIC_PTR)(entry) ; \
        _BLINK(qhead) = (BMD_GENERIC_PTR)(entry) ; \
    }

#define BXI_REMQ(entry) { \
    _BLINK(_FLINK(entry)) = _BLINK(entry); \
    _FLINK(_BLINK(entry)) = _FLINK(entry); \
}


/* typedef's */

typedef unsigned char BriEntryType;

/* Define the queue data type */

typedef struct _BXI_QUEUE {
    BMD_GENERIC_PTR flink;
    BMD_GENERIC_PTR blink;
} BXI_QUEUE ;



#endif /* _BXI_PRIVATE_DEF_H */

/* DONT ADD STUFF AFTER THIS #endif */

