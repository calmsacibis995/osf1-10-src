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
static char *rcsid = "@(#)$RCSfile: scu_show.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/04/14 16:17:38 $";
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
 * File:	scu_show.c
 * Author:	Robin T. Miller
 * Date:	August 8, 1991
 *
 * Description:
 *	This file contains functions for generic show commands.
 */

#include <sys/types.h>
#include "scu.h"

/************************************************************************
 *									*
 * ShowExpression() - Show Results of User Specified Expression.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *									*
 * Return Value:							*
 *		Alwyas returns SUCCESS.					*
 *									*
 ************************************************************************/
int
ShowExpression (ce)
struct cmd_entry *ce;
{
	register u_long value = (u_long) *(u_long *) ce->c_argp;
	float blocks, kbytes, mbytes, gbytes;
	char blocks_buf[16], kbyte_buf[16], mbyte_buf[16], gbyte_buf[16];

	blocks = ( (float) value / (float) BLOCK_SIZE);
	kbytes = ( (float) value / (float) KBYTE_SIZE);
	mbytes = ( (float) value / (float) MBYTE_SIZE);
	gbytes = ( (float) value / (float) GBYTE_SIZE);
	
	(void) sprintf (blocks_buf, "%.3f", blocks);
	(void) sprintf (kbyte_buf, "%.3f", kbytes);
	(void) sprintf (mbyte_buf, "%.3f", mbytes);
	(void) sprintf (gbyte_buf, "%.3f", gbytes);

	if (DisplayVerbose) {
	    Puts ("Expression Values:\n");
	    Printf ("            Decimal: %ld\n", value);
	    Printf ("        Hexadecimal: %#lx\n", value);
	    Printf ("    512 byte Blocks: %s\n", blocks_buf);
	    Printf ("          Kilobytes: %s\n", kbyte_buf);
	    Printf ("          Megabytes: %s\n", mbyte_buf);
	    Printf ("          Gigabytes: %s\n", gbyte_buf);
	} else {
	    Printf ("Dec: %ld Hex: %#lx Blks: %s Kb: %s Mb: %s Gb: %s\n",
		value, value, blocks_buf, kbyte_buf, mbyte_buf, gbyte_buf);
	}
	return (SUCCESS);
}
