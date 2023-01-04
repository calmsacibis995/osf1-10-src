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

#define UNIT(x) ((x >> 32) & 0xff)

char *environ_table[] = 
{
	"auto_action",
	"boot_dev",
	"bootdef_dev",
	"booted_dev",
#if defined KERNEL || defined SECONDARY
	"boot_file",
	"booted_file",
	"boot_osflags",
	"booted_osflags",
	"boot_reset",
	"dump_dev",
#endif
	0
};

char environ_buffer[MAX_ENVIRON_LENGTH + 1];

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

#ifdef SECONDARY

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
	} while (((status >> 62) == GETC_NOT_READY) || (UNIT(status) != unit));

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
	if (id < 0) {
		return(id);	
	}

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

prom_putc(c)
char c;
{
	static char buf[2] = {'\0','\0'};
	buf[0] = c;
	prom_puts( buf);
}
#endif /* defined SECONDARY */

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
	return((int)0 );
}

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
	register char *string;
{
	register u_long status;
	register u_long unit = 0;
	register u_long string_len;

	/*
	 * Make sure the string length looks reasonable.
	 */
	string_len = (u_long) strlen(string);
	if (string_len > MAX_CONSOLE_STRING) {
		string_len = MAX_CONSOLE_STRING;
	}

        /*
         * Output string a single character at a time.
         */
        while (string_len != 0)
	  {
              while ( !((status = PROM_PUTS(unit, string, 1)) & 0x1))
                ;
              string++;
              string_len--;
            }
}

/*
 * Function: prom_getenv
 * Abstract: Gets an environment variable.
 * Returns:  
 * Notes:    
 */

char *
prom_getenv(varname, buff, buflen)
	register char *varname;
	register char *buff;
	register long buflen;
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
	if (id < 0) {
		return((char *)0);	
	}
	status = PROM_GETENV(id, environ_buffer, MAX_ENVIRON_LENGTH-1);

	/*
	 * Bits 63:62 set specifies error. This means that the variable is not
	 * present in the namespace.
	 * Bit 61 set indicates the name was truncated.
	 */
	if (status >> 62) {
		return((char *)0);	
	}

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

vm_offset_t disp_proc_va = 0;

long
prom_init()
{
	struct rpb *rpb = (struct rpb *)HWRPB_ADDR;
	struct rpb_crb *crb =
		(struct rpb_crb *) ((char *)rpb + rpb->rpb_crb_off);

	disp_proc_va = (vm_offset_t) crb->rpb_va_disp;
	return ( *(long *)(crb->rpb_va_disp + 8)) ;

}
