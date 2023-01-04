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
/* @(#)prom.h	9.2  (ULTRIX)        10/28/91    */

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Modification History: /sys/sas/alpha/prom.h
 *
 * 21-Oct-91 -- Andrew L. Duane
 *	Major fixes to upgrade to the Console rev. 3.2 specification
 *	New calling sequences, arguments, function codes, and register usage.
 *	NOTE: some of these are not yet implemented: see prom.c
 *
 * 13-Nov-90 -- Tim Burke
 *	Created this file for Alpha console support.
 */

#ifndef __PROM_INCLUDE__
#define __PROM_INCLUDE__


/*
 * The console routines are dispatched through the DISPATCH routine by
 * passing function codes and an argument list.  The various routines and
 * their function code assignments are defined in this module.
 */

#define DISPATCH prom_dispatcher	/* tim fix - no doubt will change   */

/*
 * Console routine function codes.
 */
#define CONS_GETC		1	/* Get char from console terminal   */
#define CONS_PUTS		2	/* Put string to console terminal   */
#define CONS_RESET_TERM		3	/* Reset console term to defaults   */
#define CONS_SET_TERM_INTR	4	/* Set console terminal interrupts  */
#define CONS_SET_TERM_CTRL	5	/* Set console terminal controls    */
#define CONS_PROCESS_KEYCODE	6	/* Process/translate keycode	    */
#define CONS_RESERVED_7		7	/* reserved */
#define CONS_RESERVED_8		8	/* reserved */
#define CONS_RESERVED_9		9	/* reserved */
#define CONS_RESERVED_10	10	/* reserved */
#define CONS_RESERVED_11	11	/* reserved */
#define CONS_RESERVED_12	12	/* reserved */
#define CONS_RESERVED_13	13	/* reserved */
#define CONS_RESERVED_14	14	/* reserved */
#define CONS_RESERVED_15	15	/* reserved */
#define CONS_OPEN		16	/* Open IO device for access        */
#define CONS_CLOSE		17	/* Close IO device for access       */
#define CONS_IOCTL		18	/* Device specific IO operations    */
#define CONS_READ		19	/* Read IO device		    */
#define CONS_WRITE		20	/* Write IO device		    */
#define CONS_RESERVED_21	21	/* reserved */
#define CONS_RESERVED_22	22	/* reserved */
#define CONS_RESERVED_23	23	/* reserved */
#define CONS_RESERVED_24	24	/* reserved */
#define CONS_RESERVED_25	25	/* reserved */
#define CONS_RESERVED_26	26	/* reserved */
#define CONS_RESERVED_27	27	/* reserved */
#define CONS_RESERVED_28	28	/* reserved */
#define CONS_RESERVED_29	29	/* reserved */
#define CONS_RESERVED_30	30	/* reserved */
#define CONS_RESERVED_31	31	/* reserved */
#define CONS_SETENV		32	/* Set an environment variable	    */
#define CONS_CLEARENV		33	/* Delete an evnoronment variable   */
#define CONS_GETENV		34	/* Get an environment variable	    */
#define CONS_SAVEENV		35	/* Save environment variables	    */
#define	CONS_PSWITCH		48	/* Switch primary processor (MP)    */
/* #define CONS_FIXUP		16	/* Does virtual address fixup       */

/*
 * The DISPATCH routine is used to call the console routine.  
 *
 * Notes: The dispatch routine is a generic interface to the console
 * routines.  The first argument is always one of the function codes.
 * Following the function code is 0 to 5 parameters.
 *
 * tim fix -
 * I'm not sure if it is necessary to pass null parameters in the case
 * where all 5 parameters are needed.  I suppose this will  depend on how
 * the dispatch routine is implemented and register conventions/usage.
 * For now I'll assume that a full list of 5 parameters must always be
 * provided.
 *
 * ald -
 * The last two arguments are reserved for IOCTL only. Propagate them
 * for all routines, even if unused. Comment above is right: pass them all.
 *
 * Dispatch() calling sequence:
 *	arg0 is function code: e.g. CONS_GETC; goes into r16 (s0)
 *	arg1->5 are arg0->4 of caller; go into r17-r21 (s1-s5)
 *	r26 (k0) is used for the return address back to DISPATCH.
 *	r25 (t9) isn't used, even though the spec says it is.
 */

/* char = getc(unit) */
#define PROM_GETC(P_UNIT) \
	DISPATCH(CONS_GETC, P_UNIT, 0, 0, 0, 0)

/* wcount = puts(unit, string, length) */
#define PROM_PUTS(P_UNIT, P_ADDRESS, P_LENGTH) \
	DISPATCH(CONS_PUTS, P_UNIT, P_ADDRESS, P_LENGTH, 0, 0)

/* status = reset_term(unit) */
#define PROM_RESET_TERM(P_UNIT)	\
	DISPATCH(CONS_RESET_TERM, P_UNIT, 0, 0, 0, 0)

/* status = set_term_intr(unit, mask) */
#define PROM_SET_TERM_INTR(P_UNIT, P_MASK) \
	DISPATCH(CONS_SET_TERM_INTR, P_UNIT, P_MASK, 0, 0, 0)

/* status = set_term_ctrl(unit, crb) */
#define PROM_SET_TERM_CTRL(P_CRB) \
	DISPATCH(CONS_SET_TERM_CTRL, P_UNIT, P_CRB, 0, 0, 0)

/* char = process_keycode(unit, keycode, again) */
#define PROM_PROCESS_KEYCODE(P_UNIT, P_KEYCODE, P_AGAIN) \
	DISPATCH(CONS_PROCESS_KEYCODE, P_UNIT, P_KEYCODE, P_AGAIN, 0, 0)

/* channel = open(devstr, length) */
#define PROM_OPEN(P_DEVSTR, P_LENGTH) \
	DISPATCH(CONS_OPEN, P_DEVSTR, P_LENGTH, 0, 0, 0)

/* status = close(channel) */
#define PROM_CLOSE(P_CHANNEL) \
	DISPATCH(CONS_CLOSE, P_CHANNEL, 0, 0, 0, 0)

/* count = ioctl(channel, param2, param3, param4, param5) */
#define PROM_IOCTL(P_CHANNEL, P_PARAM2, P_PARAM3, P_PARAM4, P_PARAM5) \
	DISPATCH(CONS_IOCTL, P_CHANNEL, P_PARAM2, P_PARAM3, P_PARAM4, P_PARAM5)

/* rcount = read(channel, count, address, block) */
#define PROM_READ(P_CHANNEL, P_COUNT, P_ADDRESS, P_BLOCK) \
	DISPATCH(CONS_READ, P_CHANNEL, P_COUNT, P_ADDRESS, P_BLOCK, 0)

/* wcount = write(channel, count, address, block) */
#define PROM_WRITE(P_CHANNEL, P_COUNT, P_ADDRESS, P_BLOCK) \
	DISPATCH(CONS_WRITE, P_CHANNEL, P_COUNT, P_ADDRESS, P_BLOCK, 0)

/* status = setenv(id, value, length) */
#define PROM_SETENV(P_ID, P_VALUE, P_LENGTH) \
	DISPATCH(CONS_SETENV, P_ID, P_VALUE, P_LENGTH, 0, 0)

/* status = clearenv(id, value, length) */
#define PROM_CLEARENV(P_ID, P_BUFFER, P_BUFLEN) \
	DISPATCH(CONS_CLEARENV, P_ID, P_BUFFER, P_BUFLEN, 0, 0)

/* status = getenv(id, buffer, length) */
#define PROM_GETENV(P_ID, P_BUFFER, P_BUFLEN) \
	DISPATCH(CONS_GETENV, P_ID, P_BUFFER, P_BUFLEN, 0, 0)

/* status = save_env() */
#define PROM_SAVEENV() \
	DISPATCH(CONS_SAVEENV, 0, 0, 0, 0, 0)

/* status = prom_pswtch(action) */
#define PROM_PSWTCH(P_ACTION, P_CPUID) \
	DISPATCH(CONS_PSWTCH, P_ACTION, P_CPUID, 0, 0, 0)

/* This is BROKEN! FIXUP uses a separate entry point from DISPATCH !!! */
#define PROM_FIXUP() \
 	DISPATCH(CONS_FIXUP, 0, 0, 0, 0, 0)

/*
 * The following constants are used to examine the status of prom calls.
 */
#define CONS_WRITE_ERROR	0x8000000000000000
#define CONS_WRITE_EOT		0x4000000000000000
#define CONS_WRITE_ILI		0x2000000000000000
#define CONS_WRITE_EOM		0x1000000000000000
#define CONS_READ_ERROR		0x8000000000000000
#define CONS_READ_EOF		0x4000000000000000
#define CONS_READ_ILI		0x2000000000000000
#define CONS_READ_EOM		0x1000000000000000
#define GETC_RCV_OK	0	/* Success, character received 		*/
#define GETC_NOT_READY	2	/* Not ready for reception		*/
#define GETC_RCV_ERR	3	/* Character received with error	*/

/*
 * Assume that there is a maximum sized output string to protect from the
 * case where cprintf is called with an improperly terminated string.
 */
#define MAX_CONSOLE_STRING	1023	/* Maximum size of output string */

/*
 * This is an internal buffer used to pass back null terminated environment
 * variable values with getenv.
 */
#define MAX_ENVIRON_LENGTH	255

#endif /* __PROM_INCLUDE__ */
