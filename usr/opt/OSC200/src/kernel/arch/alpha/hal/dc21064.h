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
 * @(#)$RCSfile: dc21064.h,v $ $Revision: 1.1.4.2 $Date: $
 */

#ifndef __DC21064_H__
#define __DC21064_H__

/*
 * BIU Status Register bits.
 */
#define BIU_HERR			1L
#define BIU_SERR			(1L << 1L)
#define BIU_TPERR			(1L << 2L)
#define BIU_TCPERR      		(1L << 3L)
#define BIU_CMD_SHIFT			4L
#define BIU_CMD				(7L << BIU_CMD_SHIFT)
#define BIU_SEO				(1L << 7L)
#define BIU_FILL_ECC			(1L << 8L)
#define BIU_STAT_RAZ1			(1L << 9L)
#define BIU_FILL_DPERR			(1L << 10L)
#define BIU_FILL_IRD			(1L << 11L)
#define BIU_FILL_QW			(3L << 12L)
#define BIU_FILL_SEO			(1L << 14L)
#define BIU_STAT_RAZ2			0xFFFFFFFF8000L

/*
 * BIU Command Register values.
 */
#define BIU_CMD_IDLE		(0x00L << BIU_CMD_SHIFT)
#define BIU_CMD_BARRIER		(0x01L << BIU_CMD_SHIFT)
#define BIU_CMD_FETCH		(0x02L << BIU_CMD_SHIFT)
#define BIU_CMD_FETCHM		(0x03L << BIU_CMD_SHIFT)
#define BIU_CMD_READ_BLOCK	(0x04L << BIU_CMD_SHIFT)
#define BIU_CMD_WRITE_BLOCK	(0x05L << BIU_CMD_SHIFT)
#define BIU_CMD_LDxL		(0x06L << BIU_CMD_SHIFT)
#define BIU_CMD_STxC		(0x07L << BIU_CMD_SHIFT)

/*
 * Aliases for some BIU constants.
 *  These symbols conform to names for BIU_STAT with the
 *  description in the EV4 spec rev. 2.0
 */
#define BC_TPERR			BIU_TPERR
#define BC_TCPERR			BIU_TCPERR
#define FILL_ECC 			BIU_FILL_ECC
#define FILL_DPERR			BIU_FILL_DPERR
#define FILL_IRD			BIU_FILL_IRD
#define FILL_QW				BIU_FILL_QW
#define FILL_SEO			BIU_FILL_SEO

/*
 * Fill Syndrome bits.
 */
#define FILL_L_WIDTH			7L
#define FILL_L_SHIFT			0L
#define FILL_L				0x07FL

#define FILL_H_WIDTH			FILL_L_WIDTH
#define FILL_H_SHIFT			7L
#define FILL_H				(0x7FL << FILL_H_SHIFT)

#define FILL_SYNDROME_NONE		0L
#define FILL_SYNDROME_POISONED		0x1FL

#endif /* __DC21064_H__ */
