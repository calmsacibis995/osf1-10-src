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
static char *rcsid = "@(#)$RCSfile: scu_set.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/05/04 16:42:07 $";
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
 * File:	scu_set.c
 * Author:	Robin T. Miller
 * Date:	December 3, 1990
 *
 * Description:
 *	This file contains functions for setting parameters.
 */

#include "scu.h"

/*
 * External References:
 */
extern void CloseFile (FILE **fp);
extern int OpenOutputFile (FILE **fp, char *file);

/************************************************************************
 *									*
 * SetOutputFile() - Set the Output File to Write Reports to.		*
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
SetOutputFile (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	char *fnp = (char *) *(long *) ke->k_argp;
	int status = SUCCESS;

	if ( (fnp == NULL) || (strlen(fnp) == 0) ) {
	    CloseFile (&OutputFp);
	} else {
	    status = OpenOutputFile (&OutputFp, fnp);
	}
	return (status);
}

/************************************************************************
 *									*
 * SetOutputLog() - Set the Output Log File to Write to.		*
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
SetOutputLog (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	char *fnp = (char *) *(long *) ke->k_argp;
	int status = SUCCESS;

	if ( (fnp == NULL) || (strlen(fnp) == 0) ) {
	    CloseFile (&LogFp);
	} else {
	    status = OpenOutputFile (&LogFp, fnp);
	}
	return (status);
}

/************************************************************************
 *									*
 * SetRadixDec() - Set Output Display Radix to Decimal.			*
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
SetRadixDec (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	OutputRadix = DEC_RADIX;	/* Until 'set radix' parse...	*/
	return (SUCCESS);
}

/************************************************************************
 *									*
 * SetRadixHex() - Set Output Display Radix to Hexidecimal.		*
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
SetRadixHex (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	OutputRadix = HEX_RADIX;	/* Until 'set radix' parse...	*/
	return (SUCCESS);
}

/************************************************************************
 *									*
 * SetPosition() - Set The Position to Logical Block Address.		*
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
SetPosition (ce, ke)
struct cmd_entry *ce;
register struct key_entry *ke;
{
	return (DoIoctl (ke->k_cmd, ke->k_argp, ke->k_msgp));
}
