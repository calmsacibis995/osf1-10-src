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
static char *rcsid = "@(#)$RCSfile: scu_utils.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/12/15 20:57:15 $";
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
 * File:	scu_utils.c
 * Author:	Robin T. Miller
 * Date:	November 29, 1990
 *
 * Description:
 *	This file contains various utility functions.
 */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include <io/common/iotypes.h>
#include <io/cam/cam.h>
#include <io/cam/dec_cam.h>
#include <io/cam/cam_special.h>
#include <io/cam/scsi_all.h>

#undef SUCCESS
#undef FAILURE

#include "scu.h"
#include "scu_device.h"

/*
 * External Declarations:
 */
extern char *sys_errlist[];
#ifndef OSF
static int sys_nerr = 91;
#endif /* OSF */
extern char *cdbg_GetDeviceName(), *cdbg_SystemStatus();
extern int getuid();
extern void CloseFile (FILE **fp);
extern void ClosePager();
extern int OpenPager (char *pager);

/*
 * Forward References:
 */
void Fprintf(), Printf();
int Fgets (char *str, int size, FILE *stream);
int Fputs (char *str, FILE *stream);
void Lputs (char *str);
char *GetDeviceName (int device_type);
int PutOutput (char *bufptr);

/*
 * Yes/no Table.
 */
char *yesno_table[] = {
	"No",
	"Yes",
	(caddr_t) 0
};

/*
 * Enabled/disabled Table.
 */
char *endis_table[] = {
	"Disabled",
	"Enabled",
	(caddr_t) 0
};

/************************************************************************
 *									*
 * CheckDeviceName() - Check For Proper Device Type.			*
 *									*
 * Description:								*
 *	This function is called from the command parser to perform	*
 * additional checking before calling the command function to actually	*
 * execute the command.							*
 *									*
 * Inputs:	ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
CheckDeviceType (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	register struct scu_device *scu = ScuDevice;
	u_long dtype_mask;
	int status;

	if ( (status = CheckDeviceNexus (ce, ke)) != SUCCESS) {
	    return (status);
	}
	if (ke == (struct key_entry *) 0) {
	    dtype_mask = ce->c_spec;
	} else {
	    dtype_mask = ke->k_spec;
	}
	if (ISCLR (dtype_mask, scu->scu_device_type)) {
	    Fprintf ("%s device (%s) does NOT support this command.",
	        GetDeviceName (scu->scu_device_type), scu->scu_device_name);
	    status = FAILURE;
	}
	return (status);
}

/************************************************************************
 *									*
 * CheckDeviceNexus() - Check For Device Nexus Information.		*
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
CheckDeviceNexus (ce, ke)
struct cmd_entry *ce;
struct key_entry *ke;
{
	register struct scu_device *scu = ScuDevice;
	int status = SUCCESS;

	if (
	( (scu->scu_bus == NEXUS_NOT_SET) && (ISCLR_KOPT (ce, K_BUS)) ) ||
	( (scu->scu_target == NEXUS_NOT_SET) && (ISCLR_KOPT (ce, K_TARGET))) ||
	( (scu->scu_lun == NEXUS_NOT_SET) && (ISCLR_KOPT (ce, K_LUN)) ) ) {
 Fprintf ("All or part of device nexus information (bus/target/lun) is NOT setup!.");
	    status = FAILURE;
	}
	return (status);
}

/************************************************************************
 *									*
 * CheckRootAccess() - Check For Root (Super User) Access.		*
 *									*
 * Inputs:	ke = The keyword table entry.				*
 * 		scu = The SCSI device unit.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
CheckRootAccess (ke, scu)
struct key_entry *ke;
struct scu_device *scu;
{
	int status;

	if ( getuid() ) {
	    errno = EACCES;
	    Fprintf ("'%s' rejected, restricted to super user access.",
					ke->k_msgp, scu->scu_device_entry);
	    status = FAILURE;
	} else {
	    status = SUCCESS;
	}
	return (status);
}

/************************************************************************
 *									*
 * CheckWriteAccess() - Check For Device Write Access.			*
 *									*
 * Inputs:	ke = The keyword table entry.				*
 * 		scu = The SCSI device unit.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
CheckWriteAccess (ke, scu)
struct key_entry *ke;
register struct scu_device *scu;
{
	int status;

	if ( (scu->scu_file_flags & FWRITE) == 0 ) {
	    errno = EACCES;
	    Fprintf ("'%s' requires write access to device '%s'",
				ke->k_msgp, scu->scu_device_entry);
	    status = FAILURE;
	} else {
	    status = SUCCESS;
	}
	return (status);
}

/************************************************************************
 *									*
 * CvtStrtoValue() - Converts ASCII String into Numeric Value.		*
 *									*
 * Inputs:	nstr = String to convert.				*
 *		eptr = Pointer for terminating character pointer.	*
 *		base = The base used for the conversion.		*
 *									*
 * Outputs:	eptr = Points to terminating character or nstr if an	*
 *			invalid if numeric value cannot be formed.	*
 *									*
 * Return Value:							*
 *		Returns converted number or -1 for FAILURE.		*
 *									*
 ************************************************************************/
u_long
CvtStrtoValue (nstr, eptr, base)
register char *nstr;
char **eptr;
int base;
{
	register u_long n = 0;

	if ( (n = strtoul (nstr, eptr, base)) == 0) {
	    if (nstr == *eptr) {
		n++;
	    }
	}
#ifdef notdef
	if (nstr == *eptr) {
	    return (n);
	}
#endif notdef
	nstr = *eptr;
	for (;;) {

	    switch (*nstr++) {

		case 'k':
		case 'K':			/* Kilobytes */
			n *= KBYTE_SIZE;
			continue;

		case 'g':
		case 'G':			/* Gigibytes */
			n *= GBYTE_SIZE;
			continue;

		case 'm':
		case 'M':			/* Megabytes */
			n *= MBYTE_SIZE;
			continue;

		case 'w':
		case 'W':			/* Word count. */
			n *= sizeof(int);
			continue;

		case 'b':
		case 'B':			/* Block count. */
			n *= BLOCK_SIZE;
			continue;

		case 'c':
		case 'C':			/* Core clicks. */
		case 'p':
		case 'P':			/* Page size. */
			n *= PageSize;
			continue;

		case '+':
			n += CvtStrtoValue (nstr, eptr, base);
			nstr = *eptr;
			continue;

		case '-':
			n -= CvtStrtoValue (nstr, eptr, base);
			nstr = *eptr;
			continue;

		case '*':
		case 'x':
		case 'X':
			n *= CvtStrtoValue (nstr, eptr, base);
			nstr = *eptr;
			continue;

		case '/':
			n /= CvtStrtoValue (nstr, eptr, base);
			nstr = *eptr;
			continue;

		case '%':
			n %= CvtStrtoValue (nstr, eptr, base);
			nstr = *eptr;
			continue;

		case '~':
			n = ~CvtStrtoValue (nstr, eptr, base);
			nstr = *eptr;
			continue;

		case '|':
			n |= CvtStrtoValue (nstr, eptr, base);
			nstr = *eptr;
			continue;

		case '&':
			n &= CvtStrtoValue (nstr, eptr, base);
			nstr = *eptr;
			continue;

		case '^':
			n ^= CvtStrtoValue (nstr, eptr, base);
			nstr = *eptr;
			continue;

		case '<':
			if (*nstr++ != '<') goto error;
			n <<= CvtStrtoValue (nstr, eptr, base);
			nstr = *eptr;
			continue;
			
		case '>':
			if (*nstr++ != '>') goto error;
			n >>= CvtStrtoValue (nstr, eptr, base);
			nstr = *eptr;
			continue;
			
		case ' ':
		case '\t':
			continue;

		case '\0':
			*eptr = --nstr;
			break;

		default:
error:
			n = 0;
			*eptr = --nstr;
			break;
	    }
	    return (n);
	}
}

/************************************************************************
 *									*
 * GetDeviceName() - Get Device Type String.				*
 *									*
 * Inputs:	device_type = The device type from inquiry command.	*
 *									*
 * Returns:	Pointer to device name string.				*
 *									*
 ************************************************************************/
char *
GetDeviceName (device_type)
register int device_type;
{
	return cdbg_GetDeviceName (device_type);
}

/************************************************************************
 *									*
 * CvtLowerCase() - Convert String to Lowercase.			*
 *									*
 * Description:								*
 *	This function is used to convert a string to lowercase.		*
 *									*
 * Inputs:	str = Pointer to string to convert.			*
 *									*
 * Outputs:	Returns TRUE/FALSE = Conversion/No Conversion.		*
 *									*
 ************************************************************************/
int
CvtLowerCase (str)
register char *str;
{
	register int c;
	int status = FALSE;

	if (str != NULL) {
	    while ( (c = *str) ) {
		if (isupper (c)) {
			status = TRUE;
			*str = tolower (c);
		}
		str++;
	    }
	}
	return (status);
}

/************************************************************************
 *									*
 * DumpHex() - Dump Data Buffer in Hex Bytes.				*
 *									*
 * Inputs:	buffer = Pointer to buffer to dump.			*
 *		size   = The size of the buffer to dump.		*
 *									*
 * Outputs:	None.							*
 *									*
 ************************************************************************/
void
DumpHex (buffer, size)
char *buffer;
register int size;
{
	register int i;
	register int field_width = 16;
	register u_char *bptr = (u_char *)buffer;

	for (i = 0; i < size; i++) {
	    Printf ("%x ", *bptr++);
	    if ((i % field_width) == 0) {
		if (i) Printf ("\n");
	    }
	}
	if (i && ((i % field_width) != 0) ) {
	    Printf ("\n");
	}
}

/************************************************************************
 *									*
 * GetInput() - Function To Get An Input String.			*
 *									*
 * Description:								*
 *	This function is used to get an input string.  The input may be	*
 * from the terminal or from a file.  The continuation character '\' is	*
 * also handled by this function to allow multiple lines of input.	*
 *									*
 * Inputs:	bufptr = Pointer to input buffer.			*
 *		bufsiz = The input buffer size.				*
 *									*
 * Return Value:							*
 *		Returns pointer to buffer, or NULL on EOF or error.	*
 *									*
 ************************************************************************/
char *
GetInput (bufptr, bufsiz)
char *bufptr;
int bufsiz;
{
	register char *bp;
	register int i = 0;
	FILE *fp = (InputFp != NULL) ? InputFp : stdin;

	(void) bzero (bufptr, bufsiz);

	for (;;) {
	    bp = &bufptr[i];
	    if (Fgets (bp, (bufsiz - i), fp) != SUCCESS) {
		return (NULL);
	    }
	    /*
	     * Skip comment lines.
	     */
	    if ( (i == 0) && (bp[0] == '#') ) {
		continue;
	    }
	    if (LogFp != NULL) {
		Lputs (bp);
	    }
	    if ( (i += strlen (bp)) < bufsiz) {
		if (bufptr[i - 1] == '\n') {
		    bufptr[--i] = 0;
		}
		if ( (i > 0) && (bufptr[i - 1] == '\\') ) {
		    i--;
		    continue;
		}
	    }
	    break;
	}

	return (bufptr);
}

/************************************************************************
 *									*
 * PromptInput() - Function To Prompt and Get an Input String.		*
 *									*
 * Description:								*
 *	This function is used by the portable help function.  This is	*
 * the reason for the non-standard status returns hard coded below.	*
 *									*
 * Inputs:	prompt = Pointer to prompt string.			*
 *		bufptr = Pointer to input buffer.			*
 *		bufsiz = The input buffer size.				*
 *									*
 * Return Value:							*
 *		Returns 1 / -1 = SUCCESS / FAILURE or EOF.		*
 *									*
 ************************************************************************/
int
PromptInput (prompt, bufptr, bufsiz)
char *prompt;
char *bufptr;
int bufsiz;
{
	int status;

	ClosePager();
	(void) PutOutput (prompt);
	if (GetInput (bufptr, bufsiz) != NULL) {
	    status = 1;
	    (void) OpenPager (NULL);
	} else {
	    status = -1;
	}
	return (status);
}

/************************************************************************
 *									*
 * PutOutput() - Function To Put Output to Current Output Stream.	*
 *									*
 * Description:								*
 *	This function is the common point where all output is funneled	*
 * through.  This way, we can easily route the output to an output file	*
 * a pipe, and/or the users' terminal.					*
 *									*
 * Inputs:	bufptr = Pointer to input buffer.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
PutOutput (bufptr)
char *bufptr;
{
	register FILE *fp;
	int status;

	if ( (fp = PipeFp) != NULL) {
	    status = Fputs (bufptr, fp);	/* Using pipe for paging. */
	    (void) fflush (fp);
	} else {
	    status = Fputs (bufptr, stdout);
	    if (DebugFlag) {
		(void) fflush (stdout);		/* Synchronize the output. */
	    }
	}

	if ( (fp = LogFp) != NULL) {		/* Saving output in a log. */
    	    Lputs (bufptr);
	}

	return (status);
}

/************************************************************************
 *									*
 * Perror() - Common Function to Print Error Messages.			*
 *									*
 * Implicit Inputs:							*
 *	msg = Pointer to format control string.				*
 *	arg1-15 - Arguments to print.					*
 *									*
 * Return Value:							*
 *	Void.								*
 * 									*
 ************************************************************************/
/*VARARGS*/
void
Perror (msg, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15)
char *msg;
long a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15;
{
	register FILE *fp = stderr;
	register char *bp;
	int err = errno;

	if ( (PerrorFlag == FALSE) && ((errno == EIO) || (errno == EINTR)) ) {
	    return;
	}
	(void) fflush (stdout);
	if ( (bp = OutputBufPtr) == NULL) {
	    (void) fprintf (fp, "%s: ", OurName);
	    (void) fprintf (fp, msg, a1, a2, a3, a4, a5, a6, a7, a8, a9,
						a10, a11, a12, a13, a14, a15);
	    if (err < sys_nerr) {
		(void) fprintf (fp, ", %s (%d) - %s\n",
			cdbg_SystemStatus(err), err, sys_errlist[err]);
	    } else {
		(void) fprintf (fp, ", errno = %d\n", err);
	    }
	    (void) fflush (fp);
	} else {
	    bp += Sprintf (bp, "%s: ", OurName);
	    bp += Sprintf (bp, msg, a1, a2, a3, a4, a5, a6, a7, a8, a9,
						a10, a11, a12, a13, a14, a15);
	    if (err < sys_nerr) {
		bp += Sprintf (bp, ", %s (%d) - %s\n",
			cdbg_SystemStatus(err), err, sys_errlist[err]);
	    } else {
		bp += Sprintf (bp, ", errno = %d\n", err);
	    }
	    bp = OutputBufPtr;
	    (void) Fputs (bp, fp);
	    (void) fflush (fp);
	    Lputs (bp);
	}
	errno = err;
	ExitStatus = errno;
}

/************************************************************************
 *									*
 * Fgets()	Common Function to Get Input from Input Stream.		*
 *									*
 * Inputs:	str = The string buffer pointer.			*
 *		size = The number of bytes to read.			*
 *		stream = The file stream to access.			*
 *									*
 * Return Value:							*
 *		Returns 0 / 1 / -1 = SUCCESS / END_OF_FILE / FAILURE	*
 * 									*
 ************************************************************************/
int
Fgets (str, size, stream)
char *str;
int size;
FILE *stream;
{
	int status = SUCCESS;

	if (fgets ((char *) str, size, stream) == NULL) {
	    if (feof (stream) != 0) {
		status = END_OF_FILE;
	    } else {
		status = FAILURE;
	    }
	    clearerr (stream);
	}
	return (status);
}

/************************************************************************
 *									*
 * Fputs()	Common function to Write String to an Output Stream.	*
 *									*
 * Inputs:	str = The string buffer pointer.			*
 *		stream = The file stream to access.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 * 									*
 ************************************************************************/
int
Fputs (str, stream)
char *str;
FILE *stream;
{
	int status = SUCCESS;

	(void) fputs ((char *) str, stream);
	if (ferror (stream) != 0) {
	    clearerr (stream);
	    status = FAILURE;
	}
	return (status);
}

/************************************************************************
 *									*
 * Fprintf()/Printf() - Common Functions to Print Messages.		*
 *									*
 * Implicit Inputs:							*
 *	msg = Pointer to format control string.				*
 *	arg1-15 - Arguments to print.					*
 *									*
 * Return Value:							*
 *	Void.								*
 * 									*
 ************************************************************************/
/*VARARGS*/
void
Printf (msg, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15)
char *msg;
long a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15;
{
	if (OutputBufPtr == NULL) {
	    (void) printf (msg, a1, a2, a3, a4, a5, a6, a7,
					a8, a9, a10, a11, a12, a13, a14, a15);
	} else {
	    (void) sprintf (OutputBufPtr, msg, a1, a2, a3, a4, a5, a6, a7,
					a8, a9, a10, a11, a12, a13, a14, a15);
	    (void) PutOutput (OutputBufPtr);
	}
}

void
Puts (str)
char *str;
{
	Printf ("%s\n", str);
}

/*VARARGS*/
void
Fprintf (msg, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15)
char *msg;
long a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15;
{
	register char *bp;
	register FILE *fp = stderr;

	(void) fflush (stdout);
	if ( (bp = OutputBufPtr) == NULL) {
	    (void) fprintf (fp, "%s: ", OurName);
	    (void) fprintf (fp, msg, a1, a2, a3, a4, a5, a6, a7, a8, a9,
						a10, a11, a12, a13, a14, a15);
	    (void) fprintf (fp, "\n");
	    (void) fflush (fp);
	} else {
	    bp += Sprintf (bp, "%s: ", OurName);
	    bp += Sprintf (bp, msg, a1, a2, a3, a4, a5, a6, a7, a8, a9,
						a10, a11, a12, a13, a14, a15);
	    bp += Sprintf (bp, "\n");
	    bp = OutputBufPtr;
	    (void) Fputs (bp, fp);
	    (void) fflush (fp);
	    Lputs (bp);
	}
#ifdef notdef
	if (ExitStatus == SUCCESS) {
	    ExitStatus = FAILURE;
	}
#endif
}

/*VARARGS*/
void
FprintC (msg, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15)
char *msg;
long a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15;
{
	register char *bp;
	register FILE *fp = stderr;

	if ( (bp = OutputBufPtr) == NULL) {
	    (void) fprintf (fp, "     ");
	    (void) fprintf (fp, msg, a1, a2, a3, a4, a5, a6, a7, a8, a9,
						a10, a11, a12, a13, a14, a15);
	    (void) fprintf (fp, "\n");
	    (void) fflush (fp);
	} else {
	    bp += Sprintf (bp, "     ");
	    bp += Sprintf (bp, msg, a1, a2, a3, a4, a5, a6, a7, a8, a9,
						a10, a11, a12, a13, a14, a15);
	    bp += Sprintf (bp, "\n");
	    bp = OutputBufPtr;
	    (void) Fputs (bp, fp);
	    (void) fflush (fp);
	    Lputs (bp);
	}
}

/*
 * Write the output to the log file (if any).
 */
/*VARARGS*/
void
Lprintf (msg, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15)
char *msg;
long a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15;
{
	register char *bp;
	register FILE *fp = LogFp;

	if ( (fp == NULL) || ((bp = OutputBufPtr) == NULL) ) {
	    return;
	}
	(void) sprintf (bp, msg, a1, a2, a3, a4, a5, a6, a7, a8, a9,
						a10, a11, a12, a13, a14, a15);
	Lputs (bp);
}

void
Lputs (str)
char *str;
{
	register FILE *fp;

	if ( (fp = LogFp) != NULL) {
	    if (Fputs (str, fp) == SUCCESS) {
		(void) fflush (fp);
	    } else {
		CloseFile (&LogFp);
	    }
	}
}

/*VARARGS*/
int
Sprintf (bufptr, msg, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15)
char *bufptr, *msg;
long a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15;
{
	(void) sprintf (bufptr, msg, a1, a2, a3, a4, a5, a6, a7, a8, a9,
						a10, a11, a12, a13, a14, a15);
	return (strlen (bufptr));
}

/************************************************************************
 *									*
 * Popen()	Common Function to Open a Pipe.				*
 *									*
 * Inputs:	pstr = The program to open pipe to.			*
 *		mode = The mode to open the pipe.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE			*
 * 									*
 ************************************************************************/
FILE *
Popen (pstr, mode)
char *pstr;
char *mode;
{
	FILE *fp;

	if ((fp = popen (pstr, mode)) == (FILE *) 0) {
	    if (DebugFlag) {
		Fprintf ("Failed to open pipe using '%s', mode '%s'.",
							pstr, mode);
	    }
	}
	return (fp);
}

void
Pclose (fp)
FILE **fp;
{
	if (*fp != (FILE *) 0) {
	    (void) pclose (*fp);
	    *fp = (FILE *) 0;
	}
}

/************************************************************************
 *									*
 * PrintHeader() - Function to Displays Header Message.			*
 *									*
 * Inputs:	header = The header strings to display.			*
 *									*
 * Return Value: void							*
 *									*
 ************************************************************************/
void
PrintHeader (header)
char *header;
{
	Printf ("%s:\n\n", header);
}

/************************************************************************
 *									*
 * PrintNumeric() - Function to Print a Numeric Field.			*
 *									*
 * Inputs:	field_str = The field name string.			*
 *		numeric_value = The numeric value.			*
 *		nl_flag = Flag to control printing newline.		*
 *									*
 * Return Value: void							*
 *									*
 ************************************************************************/
void
PrintNumeric (field_str, numeric_value, nl_flag)
char *field_str;
int numeric_value, nl_flag;
{
	char *printf_str;

	if (OutputRadix == HEX_RADIX) {
		printf_str = HEX_FIELD;
	} else {
		printf_str = NUMERIC_FIELD;
	}
	Printf (printf_str, field_str, numeric_value);
	if (nl_flag) Printf ("\n");
}


/************************************************************************
 *									*
 * PrintDecimal() - Function to Print a Numeric Field in Decimal.	*
 *									*
 * Inputs:	field_str = The field name string.			*
 *		numeric_value = The numeric value.			*
 *		nl_flag = Flag to control printing newline.		*
 *									*
 * Return Value: void							*
 *									*
 ************************************************************************/
void
PrintDecimal (field_str, numeric_value, nl_flag)
char *field_str;
int numeric_value, nl_flag;
{
	char *printf_str = NUMERIC_FIELD;

	Printf (printf_str, field_str, numeric_value);
	if (nl_flag) Printf ("\n");
}

/************************************************************************
 *									*
 * PrintHex() - Function to Print a Numeric Field in Hexadecimal.	*
 *									*
 * Inputs:	field_str = The field name string.			*
 *		numeric_value = The numeric value.			*
 *		nl_flag = Flag to control printing newline.		*
 *									*
 * Return Value: void							*
 *									*
 ************************************************************************/
void
PrintHex (field_str, numeric_value, nl_flag)
char *field_str;
int numeric_value, nl_flag;
{
	char *printf_str = HEX_FIELD;

	Printf (printf_str, field_str, numeric_value);
	if (nl_flag) Printf ("\n");
}

/************************************************************************
 *									*
 * PrintAscii() - Function to Print an ASCII Field.			*
 *									*
 * Inputs:	field_str = The field name string.			*
 *		ascii_str = The ASCII string to print.			*
 *		nl_flag = Flag to control printing newline.		*
 *									*
 * Return Value: void							*
 *									*
 ************************************************************************/
void
PrintAscii (field_str, ascii_str, nl_flag)
char *field_str, *ascii_str;
int nl_flag;
{
	int length = strlen(field_str);
	char *printf_str = ((length) ? ASCII_FIELD : EMPTY_FIELD);

	Printf (printf_str, field_str, ascii_str);
	if (nl_flag) Printf ("\n");
}
