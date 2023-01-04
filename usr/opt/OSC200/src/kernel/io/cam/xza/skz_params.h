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
#if !defined(SKZ_PARAMS_INCLUDE)
#define SKZ_PARAMS_INCLUDE


/************************************************************************
 *									*
 * File:	skz_params.h						*
 * Date:	November 6, 1991					*
 * Author:	Ron Hegli						*
 *									*
 * Description:								*
 *	This file contains parameters which control the overall 	*
 *	behavior of the driver and how much memory is consumes		*
 *									*
 ************************************************************************/

/*
** Use threads for interrupt handling, errors, etc?
*/
#define SKZ_TH_POST_BOOT 1
#define SKZ_TH_TIMEOUT 2
#define SKZ_TH_NONE 0

#define SKZ_THREADED SKZ_TH_POST_BOOT

/*
** Style of Autosense to use - CAM's port driver based version using
** the routines in sim_as.c, or the XZA's, in which the adapter will
** automatically write sense data into the sense buffer before
** returning the QB onto the response queue.
*/
#define SKZ_CAM_AUTOSENSE

/*
** skz driver parameters
*/

/*
** Memory Pools
*/
#define SKZ_NUM_ADAPT_QUEUES	5
#define SKZ_CHAN_QUEUES		7
#define SKZ_NUM_QUEUES		( ((int)XZA_CHANNELS * (int)SKZ_CHAN_QUEUES) \
				  + (int)SKZ_NUM_ADAPT_QUEUES )
#define SKZ_ADFQ_ENTRIES	8
#define SKZ_DDFQ_ENTRIES	128
#define	SKZ_QB_POOL_ENTRIES	((int)SKZ_DDFQ_ENTRIES + (int)SKZ_ADFQ_ENTRIES)
#define	SKZ_CAR_POOL_ENTRIES	( (int)SKZ_QB_POOL_ENTRIES + \
				  (int)SKZ_NUM_QUEUES )
/*
** Error pools
*/
#define SKZ_ERR_BUF_POOL_ENTRIES	16
#define SKZ_ERR_BUF_CAR_POOL_ENTRIES	(SKZ_ERR_BUF_POOL_ENTRIES + 2)

/*
** DME sg arrays
*/
#define SKZ_DME_SG_ARRAYS		14

/*
** Timing
*/
#define SKZ_ONE_SECOND_DELAY	1000000		/* in clock ticks */
#define SKZ_ALPHA_TIME_FACTOR	10


/*
** XZA Firmware Revisions
*/
#define XZA_MIN_DISCONN_REV 	0x42
#define XZA_MIN_SDTR_REV	0x3f
#define XZA_MIN_ABORT_REV	0x42
#define XZA_MIN_AS_REV		0x42
#define XZA_MIN_BDR_REV		0x43

#endif /* SKZ_PARAMS_INCLUDE */
