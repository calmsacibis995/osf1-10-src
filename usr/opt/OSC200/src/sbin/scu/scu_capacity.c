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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: scu_capacity.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/25 10:22:50 $";
#endif
/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * File:	scu_capacity.c
 * Author:	Robin T. Miller
 * Date:	August 8, 1991
 *
 * Description:
 *	This file contains functions for the Read Capacity command.
 */

#include <sys/types.h>
#include <io/cam/rzdisk.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/scsi_direct.h>
#undef SUCCESS
#undef FATAL_ERROR
#include "scu.h"

/*
 * External References:
 */
extern void PrintHeader (char *header);

/*
 * Strings for Capacity Data Fields:
 */
static char *caph_str =			"Disk Capacity Information";
static char *capacity_str =		"Maximum Capacity";
static char *block_len_str =		"Block Length";

/************************************************************************
 *									*
 * GetCapacity() - Get The Device Capacity Information.			*
 *									*
 * Inputs:	capacity = Pointer to capacity data buffer.		*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
GetCapacity (capacity)
register DIR_READ_CAP_DATA *capacity;
{
	register struct key_entry *ke;

	/*
	 * Find & execute the Read Capacity command.
	 */
	if ((ke = FindKeyEntry (NULL, C_SHOW, K_CAPACITY)) == NULL) {
	    return (FAILURE);
	}

	return (DoIoctl (ke->k_cmd, (caddr_t)capacity, ke->k_msgp));
}

/************************************************************************
 *									*
 * ShowCapacity() - Show The Disk Capacity.				*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
int
ShowCapacity (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	struct dir_read_cap_data capacity_data;
	register struct dir_read_cap_data *capacity = &capacity_data;
	u_long logical_blocks, block_length;
	int status;

	if ( (status = GetCapacity (capacity)) != SUCCESS) {
		return (status);
	}

	PrintHeader (caph_str);

	/*
	 * We add one to the logical block number returned since the
	 * value returned with read capacity is the last addressable
	 * logical block number.
	 */
	logical_blocks = (u_long)( ((u_long)capacity->lbn3 << 24) +
				   ((u_long)capacity->lbn2 << 16) +
				   ((u_long)capacity->lbn1 << 8) +
				   (capacity->lbn0) + 1 );

	PrintDecimal (capacity_str, logical_blocks, PNL);

	block_length =  (u_long)((u_long)capacity->block_len3 << 24) +
				((u_long)capacity->block_len2 << 16) +
				((u_long)capacity->block_len1 << 8) +
				(capacity->block_len0);

	PrintDecimal (block_len_str, block_length, PNL);

	return (status);
}
