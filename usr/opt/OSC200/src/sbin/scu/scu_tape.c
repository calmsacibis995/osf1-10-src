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
static char *rcsid = "@(#)$RCSfile: scu_tape.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/11/23 23:11:19 $";
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
 * File:	scu_tape.c - 'scu' Tape Functions.
 * Author:	Robin T. Miller
 * Date:	November 26, 1991
 *
 * Description:
 *	This file contains the tape support functions.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>

#include "scu.h"
#include "scu_device.h"
#include "scu_pages.h"
#include "scsipages.h"

extern int tape_density_entrys;
extern struct tape_density_entry tape_density_table[];

/************************************************************************
 *									*
 * DoMtCmd()	Setup & Do a Magtape I/O Control Command.		*
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
DoMtCmd (ce, ke)
struct cmd_entry *ce;
register struct key_entry *ke;
{
	struct mtop Mtop;
	register struct mtop *mtop = &Mtop;
	int status;

	mtop->mt_op = (short) ke->k_cmd;

	/*
	 * Setup the 'mt' command count (if any).
	 */
	if (ke->k_argp != NULL) {
	    if ((mtop->mt_count = *(daddr_t *) ke->k_argp) < 0) {
		mtop->mt_count = 1;
	    }
	} else {
	    mtop->mt_count = 0;
	}

	status = DoIoctl (MTIOCTOP, (caddr_t) mtop, ke->k_msgp);

	/*
	 * Set the default for the next 'mt' command.
	 */
	if (ke->k_argp != NULL) {
	    *(u_long *)ke->k_argp = -1;
	}
	return (status);
}

/************************************************************************
 *									*
 * DoMtOp()	Setup & Do a Magtape Operation.				*
 *									*
 * Inputs:	cmd = The magtape command to issue.			*
 *		count = The command count (if any).			*
 *		msg = The error message for failures.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
DoMtOp (cmd, count, msg)
short cmd;
daddr_t count;
caddr_t msg;
{
	struct mtop Mtop;
	register struct mtop *mtop = &Mtop;

	mtop->mt_op = cmd;

	/*
	 * Setup the 'mt' command count (if any).
	 */
	if ((mtop->mt_count = count) < 0) {
	    mtop->mt_count = 1;
	}

	return (DoIoctl (MTIOCTOP, (caddr_t) mtop, msg));
}

/************************************************************************
 *									*
 * DoXXXXX()	Setup & Do Specific Magtape Operations.			*
 *									*
 * Description:								*
 *	These functions provide a simplistic interface for issuing	*
 * magtape commands from within the program.  They all take 'count'	*
 * as an argument, except for those which do not take a count.		*
 *									*
 * Inputs:	count = The command count (if any).			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
DoWriteFileMark (count)
daddr_t count;
{
	short cmd = MTWEOF;
	return (DoMtOp (cmd, count, "write file mark"));
}

int
DoForwardSpaceFile (count)
daddr_t count;
{
	short cmd = MTFSF;
	return (DoMtOp (cmd, count, "forward space file"));
}

int
DoBackwardSpaceFile (count)
daddr_t count;
{
	short cmd = MTBSF;
	return (DoMtOp (cmd, count, "backward space file"));
}

int
DoForwardSpaceRecord (count)
daddr_t count;
{
	short cmd = MTFSR;
	return (DoMtOp (cmd, count, "forward space record"));
}

int
DoBackwardSpaceRecord (count)
daddr_t count;
{
	short cmd = MTBSR;
	return (DoMtOp (cmd, count, "backward space record"));
}

int
DoRewindTape()
{
	short cmd = MTREW;
	return (DoMtOp (cmd, (daddr_t) 0, "rewind tape"));
}

int
DoTapeOffline()
{
	short cmd = MTOFFL;
	return (DoMtOp (cmd, (daddr_t) 0, "tape offline"));
}

int
DoRetensionTape()
{
	short cmd = MTRETEN;
	return (DoMtOp (cmd, (daddr_t) 0, "retension tape"));
}

int
DoSpaceEndOfData()
{
	short cmd = MTSEOD;
	return (DoMtOp (cmd, (daddr_t) 0, "space to end of data"));
}

int
DoEraseTape()
{
	short cmd = MTERASE;
	return (DoMtOp (cmd, (daddr_t) 0, "erase tape"));
}

int
DoTapeOnline()
{
	short cmd = MTONLINE;
	return (DoMtOp (cmd, (daddr_t) 0, "tape online"));
}

int
DoLoadTape()
{
	short cmd = MTLOAD;
	return (DoMtOp (cmd, (daddr_t) 0, "load tape"));
}

int
DoUnloadTape()
{
	short cmd = MTUNLOAD;
	return (DoMtOp (cmd, (daddr_t) 0, "unload tape"));
}

/************************************************************************
 *									*
 * SetTapeParameters() - Set The Specified Tape Parameters.		*
 *									*
 * Description:								*
 *	This function allows the tape density to be setup.  Previously,	*
 * density had to be set via selecting the appropriate /dev/rmtNX entry	*
 * which caused undesirable tape commands during the device driver open	*
 * sequence when attempting to test via this utility.			*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
SetTapeParameters (struct cmd_entry *ce, struct key_entry *ke)
{
	struct scu_device *scu = ScuDevice;
	struct mode_page_header mode_parameters;
	register struct mode_page_header *mh = &mode_parameters;
	register struct mode_parameter_header *mph;
	register struct mode_block_descriptor *mbd;
	u_char pcf = PCF_CURRENT, page_code = MODE_PARAMETERS_PAGE;
	int status;

	status = SensePage (mh, pcf, page_code, sizeof(*mh));
	if (status == FAILURE) return (status);

	/*
	 * Setup the specified tape parameters.
	 */
	mph = &mh->parameter_header;
	mbd = &mh->block_descriptor;
	if ( ISSET_KOPT (ce, K_BLOCKING_MODE) ) {
	    if ( ISSET_KOPT (ce, K_FIXED_LENGTH) && (BlockingSize == 0) ) {
		BlockingSize = DEC_BLOCK_SIZE;
	    }
	    scu->scu_block_size = BlockingSize;
	    mbd->mbd_block_length_2 = LTOB(BlockingSize,2);
	    mbd->mbd_block_length_1 = LTOB(BlockingSize,1);
	    mbd->mbd_block_length_0 = LTOB(BlockingSize,0);
	}
	if ( ISSET_KOPT (ce, K_BUFFERED_MODE) ) {
	    mph->mph_buffer_mode = BufferedMode;
	}
	/*
	 * Caveat:  Since the density code does not actually go into
	 * affect until the tape is written, the next mode sense does
	 * *NOT* reflect the new density code, thus we must setup the
	 * density code whenever doing another Mode Select (e.g. when
	 * setting either the blocking size and/or buffered mode).
	 * Failure to do this, results in the density reverting to the
	 * default or current density setting.
	 */
	if ( (ISSET_KOPT (ce, K_TAPE_DENSITY)) || (DensityCode >= 0) ) {
	    mbd->mbd_density_code = (u_char) DensityCode;
	}
	return (SelectPage (mh, sizeof(*mh), MS_DONT_SAVE_PARMS));
}

/************************************************************************
 *									*
 * MapTapeDensity() - Map Selected Tape Density to Density Code.	*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
MapTapeDensity (struct cmd_entry *ce, struct key_entry *ke)
{
	int i;

	for (i = 0; i < tape_density_entrys; i++) {
	    if ((int)tape_density_table[i].td_density == (int)DensityCode) {
		DensityCode = i;
		return (SUCCESS);
	    }
	}
	Fprintf ("Invalid tape density code detected, code = 0x%x", DensityCode);
	return (FAILURE);
}
