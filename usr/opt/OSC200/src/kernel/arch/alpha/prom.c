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
static char *rcsid = "@(#)$RCSfile: prom.c,v $ $Revision: 1.2.4.4 $ (DEC) $Date: 1993/08/18 19:47:24 $";
#endif

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
 * Modification History: /sys/machine/alpha/prom.c
 *
 * 21-Oct-91 -- Andrew Duane
 *	Major fixes to upgrade to the Console rev. 3.2 specification
 *	New calling sequences, arguments, function codes, and register usage.
 *	NOTE: some of these are not yet implemented: see prom.h
 *
 * 13-Nov-90 -- Tim Burke
 *	Created this file for Alpha console interface support.
 */


#include <sys/types.h>
#include <machine/rpb.h>
#include <machine/prom.h>

/*
 * Module description:
 *
 * This file contains routines which provide ULTRIX behavior of the console
 * callback routines.  These jacket routines are typically used by the 
 * standalone boot environment as well as the running system to do things
 * like access environment variables.
 *
 * NOTE: see locore.s for argument register usage;
 *	console calling sequence is not the same as C
 */

/*
 * tim fix -
 * The main piece of work remaining in this module is to complete the
 * interface to the DISPATCH routine.
 *
 * The console prom routines should only be called from the primary processor
 * in a multiprocessor configuration.  Some work may be needed (perhaps in
 * the dispatch routine) to insure this.
 */
char *environ_table[] = 
{
	"auto_action",		/* 1 */
	"boot_dev",		/* 2 */
	"bootdef_dev",		/* 3 */
	"booted_dev",		/* 4 */
#if defined KERNEL || defined SECONDARY
	"boot_file",		/* 5 */
	"booted_file",		/* 6 */
	"boot_osflags",		/* 7 */
	"booted_osflags",	/* 8 */
	"boot_reset",		/* 9 */
	"dump_dev",		/* 10 */
	"enable_audit",		/* 11 */
	"license",		/* 12 */
	"char_set",		/* 13 */
	"language",		/* 14 */
	"tty_dev",		/* 15 */
	"xxx",			/* 16 */
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",			/* 32 */
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",			/* 48 */
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",
	"xxx",			/* 64 */
	"xxx",
	"scsiid",
	"scsifast",
#endif
	0
};

char environ_buffer[MAX_ENVIRON_LENGTH + 1];

extern long hwrpb_addr;

/*
 * Function: match_var_name
 * Abstract: This routine is passed a string representing the name of an
 *	     environment variable.  The string name is compared to the table
 *	     of known environment variable names to find the associated ID.
 *	     The ID is then used in calls to setenv, getenv, and clearenv.
 * Returns:  -1 if no match found, otherwise the ID of the variable.
 */
int
match_var_name(varname)
	register char *varname;
{
	int i;

	for( i=0 ; environ_table[i] ; i++ )
		if( strcmp(varname, environ_table[i]) == 0 ) 
			return (i+1);
	return(-1);
}
#if defined KERNEL || defined SECONDARY

/*
 * Function: prom_getc
 * Abstract: Gets a character from the console terminal.  This is done by
 *	     making a console callback to obtain a character.  This process
 *	     will poll for a character to be input or until an error has
 *	     occurred.
 * Returns:  An unsigned character value.  Note this will need some work to
 *	     enable multibyte characters to be received!
 * Notes:    The unit and interrupt status portions of the returned status are
 *	     not used.  Also there is also status indication that there are 
 *	     more characters to be read.  This is also not presently used.
 *	     The received character is not checked for errors.  If an error
 *	     occurs the returned character will be undefined.
 *
 *	     Presently this assumes that the console is on unit 0.  For
 *	     platforms with a logical console on a different line number this
 *	     field will have to be appropriately filled in.
 */

u_char
prom_getc()
{
	register u_long status;
	register u_long unit = 0;

	do {						/* poll for character */
		status = PROM_GETC(unit);
	} while ((status >> 62) == GETC_NOT_READY);

	/*
	 * The character read in from the console is stored in the low order
	 * bits of the status register.  Strips to 8-bits!
	 */
	return((u_char) status );
}

/*
 * Function: prom_setenv
 * Abstract: Set an environment variable.
 * Returns:  0 on success, -1 on failure.
 * Notes:    
 */

int
prom_setenv(varname, value)
	register char *varname;
	register char *value;
{
	register u_long status;
	register int id;

	/*
	 * Find the environment variable ID that corresponds to the given
	 * environment variable string name.  Return an error if this is not
	 * a recognized environment variable.
	 */
	id = match_var_name(varname);
	if (id < 0) {
		return((int) -1);
	}
	status = PROM_SETENV(id, value, strlen(value));

	/*
	 * If either of the top 2 bits are set an error has occured.  This
	 * could mean that the namespace is full, the value is not quadword
	 * alligned, or the value is readonly.
	 */
	if (status >> 62) {
		return((int) -1);
	}
	return((int) 0 );
}

/*
 * Function: prom_clearenv
 * Abstract: Delete an environment variable from the namespace.
 * Returns:  0 on success, -1 on failure.
 * Notes:    
 */

int
prom_clearenv(varname)
	register char *varname;
{
	register u_long status;
	register int id;

	/*
	 * Find the environment variable ID that corresponds to the given
	 * environment variable string name.  Return an null string if no 
	 * match is found.
	 */
	environ_buffer[0] = '\0';
	id = match_var_name(varname);
#if defined KERNEL || defined SECONDARY
	if (id < 0) {
		return(id);	
	}
#endif

	status = PROM_CLEARENV(id, environ_buffer, MAX_ENVIRON_LENGTH-1);
	/* The environ_buffer is discarded; could return like getenv() */

	/*
	 * If either of the top 2 bits are set an error has occured.  This
	 * could mean that the variable is not present in the namespace or is
	 * readonly.
	 */
	if (status >> 62) {
		return(-1);
	}
	return(0);
}

/*
 * Function: prom_close
 * Abstract: Close an IO device for access.
 * Returns:  0 on success, -1 on failure.
 */

int
prom_close(channel)
	register int channel;
{
	register u_long status;

	status = PROM_CLOSE(channel);

	/*	
	 * High order bit set indicates an error.
	 */
	if (status >> 63) {
		return((int) -1);
	}
	return((int) 0 );
}

/*
 * Function: prom_write
 * Abstract: Write to an IO device.
 * Returns:  The number of bytes actually written.
 * Notes:    
 */

int
prom_write(channel, buffer, count, block)
	register int channel;
	register char *buffer;
	register int count;
	register int block;
{
	register u_long status;

	status = PROM_WRITE(channel, count, buffer, block);

	if (status & CONS_WRITE_ERROR) {
		return((int) -1);
	}
	/*
	 * The number of bytes written is stored in the low 32 bits of the 
	 * status register.  Cast to int to extract that piece.
	 */
	return((int) status );
}
#endif

/*
 * Function: prom_puts
 * Abstract: Put a string to the console terminal.
 * Returns:  Presently there are no explicit return values.
 * Notes:    This routine makes a call to the console prom to output the
 *	     specified string.  No status checking is done, or error status
 *	     returned because none of the callers look at it or would be in
 *	     much of a position to do anything about errors.  For example on
 *	     the VAX, if the console gets flooded then you just lose the output.
 *
 *	     Presently this assumes that the console is on unit 0.  For
 *	     platforms with a logical console on a different line number this
 *	     field will have to be appropriately filled in.
 */

prom_puts(string)
	register u_char *string;
{
	register int limit;
	char cr = '\r'; /* must reside on kernel stack for 32-bit addressing */
	extern vm_offset_t pmap_remap_prom_addr();

	/* Make sure address if OK */
	string = (u_char *)pmap_remap_prom_addr(string);

        /*
         * Output string a single character at a time.
         */
        for (limit = MAX_CONSOLE_STRING; *string && limit-- > 0; string++) {
		if (*string == '\n') {
			while (!(PROM_PUTS((u_long)0, &cr, 1) & 1))
				;
		}
		while (!(PROM_PUTS((u_long)0, string, 1) & 1))
			;
	}
}

/*
 * Function: prom_getenv
 * Abstract: Gets an environment variable.
 * Returns:  
 * Notes:    
 */

char *
prom_getenv(varname)
	register char *varname;
{
	register u_long status;
	register int id;
	register int var_length;
	register int i;
	register char *cp;

	/*
	 * Find the environment variable ID that corresponds to the given
	 * environment variable string name.  Return an null string if no 
	 * match is found.
	 */
	environ_buffer[0] = '\0';
	id = match_var_name(varname);
#if defined KERNEL || defined SECONDARY
	if (id < 0) {
		return((char *)0);	
	}
#endif
	status = PROM_GETENV(id, environ_buffer, MAX_ENVIRON_LENGTH-1);

	/*
	 * Bits 63:62 set specifies error. This means that the variable is not
	 * present in the namespace.
	 * Bit 61 set indicates the name was truncated.
	 */
#if defined KERNEL || defined SECONDARY
	if (status >> 62) {
		return((char *)0);	
	}
#endif

	/*
	 * There is only 1 such buffer.  Consequently if 2 getenv calls are done
	 * the following would fall appart:
	 * boot = (char *)prom_getenv("booted_dev");
	 * ttyline = (char *)prom_getenv("tty_dev");
	 * At the conslusion of this boot would be pointing to the same value
 	 * as ttyline.  However this same restriction applies in the value
	 * returned by the alpha console.
	 *
	 * The low half of the return status contains the length, use this
	 * to null terminate the string.
	 */
	environ_buffer[status & 0xffffffff] = '\0';

	return((char *)environ_buffer);
}

/*
 * Function: prom_reset_term
 * Abstract: Resets console terminal to default parameters.
 * Notes:    Not implemented, no known usage at this time.
 */

/*
 * Function: prom_set_term_intr
 * Abstract: Set console terminal interrupts.
 * Notes:    Not implemented, no known usage at this time.
 *	     This may be needed to setup initial parameters, for example the
 *	     bootpath may with to init terminal not to interrupt if the 
 *	     defaults are not acceptable.
 */

/*
 * Function: prom_open
 * Abstract: Open an IO device for access.
 * Returns:  Channel number.  This will be -1 on error.
 * Notes:    This routine does not have a parameter for access modes (ie read,
 *	     or write access); make sure the calls to this pass the propper
 *	     number of parameters.
 */

int
prom_open(devstr)
	register char *devstr;
{
	register u_long status;

	status = PROM_OPEN(devstr, strlen(devstr));

	/*	
	 * If either of the top 2 bits are set the open has failed.
	 */
	if ((status >> 62) & 3) {
		return((int) -1);
	}
	/*
	 * The channel number is stored in the low 32 bits of the status
	 * register.  Cast to int to extract that piece.
	 */
	return((int) status );
}
/*
 * Function: prom_ioctl
 * Abstract: Perform device-specific IO operations.
 * Notes:    Not implemented, no known usage at this time.
 */

/*
 * Function: prom_read
 * Abstract: Read from an IO device.
 * Returns:  The number of bytes actually read, 0 on EOF, -1 on error.
 * Notes:    
 */

int
prom_read(channel, count, buffer, block)
	register int channel;
	register int count;
	register char *buffer;
	register int block;
{
	register u_long status;

	status = PROM_READ(channel, count, buffer, block);

	if (status & CONS_READ_ERROR) {
		if (status & CONS_READ_EOF) {
			return((int) 0);
		}
		/*
		 * Possible errors include running off the end of the tape or
		 * illegal record size specified.
		 */
		return((int) -1);
	}
	/*
	 * The number of bytes read is stored in the low 32 bits of the status
	 * register.  Cast to int to extract that piece.
	 */
	return((int) status );
}

/*
 * Function: prom_init
 * Abstract: Initialize the dispatch routine vector
 * Returns:
 * Notes:    
 */

vm_offset_t prom_dispatch_pv = 0;
vm_offset_t prom_stack_va = 0;
vm_offset_t prom_stack_save_sp = 0;

prom_init()
{
	struct rpb *rpb = (struct rpb *)hwrpb_addr;
	struct rpb_crb *crb =
		(struct rpb_crb *) ((char *)rpb + rpb->rpb_crb_off);

	prom_dispatch_pv = crb->rpb_va_disp;
}

/*
 * Function: prom_fixup
 * Abstract: Invoke the prom fixup callback
 * Returns:
 * Notes:    
 */

prom_fixup(stack_va)
vm_offset_t stack_va;
{
	struct rpb *rpb = (struct rpb *)hwrpb_addr;
	struct rpb_crb *crb =
		(struct rpb_crb *) ((char *)rpb + rpb->rpb_crb_off);

	prom_fixup_dispatch(stack_va, OSF_HWRPB_ADDR, crb->rpb_va_fixup);
	prom_stack_va = stack_va;
}
