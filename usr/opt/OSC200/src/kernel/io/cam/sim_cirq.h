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
 * @(#)$RCSfile: sim_cirq.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 13:45:26 $
 */
#ifndef _SIM_CIRQ_
#define _SIM_CIRQ_

/* ---------------------------------------------------------------------- */

/* sim_cirq.h		Version 1.03			Nov. 13, 1991 */

/*  This file contains the definitions and data structures needed by the
    circular queue related functions contained in the Digital CAM system.

Modification History

	1.03	11/13/91	janet
	Added VERS

	1.02	09/10/91	janet
	In CIRQ_ADD_BYTE, make sure that there is space available.

	1.01	03/26/91	janet
	Updated after code review.

	1.00	11/09/90	janet
	Created this file.
*/

/* ---------------------------------------------------------------------- */

/*
 * The circular queue structure.
 */
typedef struct {
#define CIR_Q_VERS 1
    u_short size;	/* number of data entries allowed in queue	*/
    u_short curr;	/* index to current data in queue 		*/
    u_short prev;	/* index to previous data in queue 		*/
    u_short curr_cnt;	/* number of data entries used by "curr"	*/
    u_short prev_cnt;	/* number of data entries used by "prev"	*/
    short needed;	/* number of data entries needed to satisfy an	*
			 * internal requirement				*/
} CIR_Q;

/*
 * Initialize the CIR_Q macros.
 */
#define CIRQ_SET_DATA_SZ(Q, sz) (Q).size = sz
#define CIRQ_GET_DATA_SZ(Q) (Q).size
#define CIRQ_INIT_Q(Q);							\
{									\
    (Q).curr = 0;							\
    (Q).prev = 0;							\
    (Q).curr_cnt = 0;							\
    (Q).prev_cnt = 0;							\
    (Q).needed = 0;							\
}

/*
 * Increment and decrement macros for the circular queue.  Will
 * increment and decrement the given index based on the size of the queue.
 */
#define CIRQ_INC(Q, index) (((index) + 1) == ((Q).size) ? 0 : ((index) + 1))
#define CIRQ_DEC(Q, index) (((index) - 1 < 0) ? ((Q).size - 1) : ((index)-1))

/*
 * Simple access to the queue.  Allows access to the "current" data
 * and the "previous" data.  CIRQ_SET_CURR will increment the
 * "current" index, saving it in the "previous" index.  It will then
 * store the supplied data in the new "current" location.
 */
#define CIRQ_GET_CURR(Q, data_array) (data_array)[(Q).curr]
#define CIRQ_GET_PREV(Q, data_array) (data_array)[(Q).prev]
#define CIRQ_SET_CURR(Q, data_array, byte);				\
{									\
    (Q).prev = (Q).curr;						\
    (Q).prev_cnt = (Q).curr_cnt;					\
    (Q).curr = CIRQ_INC(Q, (Q).curr);					\
    (data_array)[(Q).curr] = (byte);					\
    (Q).curr_cnt = 1;							\
}

/*
 * Get "current" and "previous" index.
 */
#define CIRQ_CURR(Q) ((Q).curr)
#define CIRQ_PREV(Q) ((Q).prev)
    
/*
 * Get the size of the "current" and "previous" data areas.
 */
#define CIRQ_CURR_SZ(Q) ((Q).curr_cnt)
#define CIRQ_PREV_SZ(Q) ((Q).prev_cnt)

/*
 * The following defines will be used when working with data
 * groups contained in the queue which are greater than 1.
 * This means that the "current" index differs from the "previous"
 * index by more than one.
 */

/*
 * Set and get the "needed" number of data entries for the "current"
 * data of the queue.
 */
#define CIRQ_SET_NEEDED(Q, value) ((Q).needed) = (value)
#define CIRQ_GET_NEEDED(Q) ((Q).needed)
    
/*
 * Increment an index value by the specified amount.
 */
#define CIRQ_ADJUST_INDEX(Q, index, amount)				\
    (((index) + (amount)) >= (Q).size) ?				\
    ((index) + (amount) - (Q).size) :					\
    ((index) + (amount))

/*
 * Add a data entry to the queue. "needed" will be checked to see
 * if the data should be added to the "current" data area.  If
 * "needed" is zero, the "previous" area will be updated to the
 * "current".
 *
 * Only add bytes to the array if there is space available.
 */
#define CIRQ_ADD_BYTE(Q, data_array, byte);				\
{									\
    if((Q).curr_cnt < (Q).size) {					\
         if((Q).needed == 0) {						\
    	     (Q).prev = (Q).curr;					\
    	     (Q).prev_cnt = (Q).curr_cnt;				\
    	     (Q).curr = CIRQ_INC(Q, (Q).curr);				\
    	     (Q).curr_cnt = 0;						\
         }								\
         else if ((Q).needed > 0) (Q).needed--;				\
         (data_array)[CIRQ_ADJUST_INDEX(Q, (Q).curr, (Q).curr_cnt)] = (byte);	\
         (Q).curr_cnt++;						\
    }									\
}

/*
 * Get a data entry from a queue.  "index" should be either "curr" or
 * "prev" values.  "byte" is the byte offset from the index.
 */
#define CIRQ_GET_BYTE(Q, index, data_array, byte)			\
    data_array[CIRQ_ADJUST_INDEX(Q, index, byte)]

/*
 * Use the queue as a sequence queue.  This means that data added
 * to the queue (using CIRQ_ADD_BYTE) wont change the "current"
 * and "previous" indices.
 */
#define CIRQ_USE_SEQ(Q) (Q).needed = -1

/*
 * Update the "previous" and "current" indices.  This macro should be
 * called when a sequence has been completed.
 */
#define CIRQ_UPDATE_SEQ(Q, count);					\
{									\
    (Q).prev = (Q).curr;						\
    (Q).prev_cnt = (count);						\
    (Q).curr = CIRQ_ADJUST_INDEX(Q, (Q).curr, count);			\
    (Q).curr_cnt = CIRQ_CURR_SZ(Q) - (count);				\
}

#endif /* _SIM_CIRQ_ */
