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
 * Modification History: /sys/sas/alpha/prf.c
 *
 * 25-Sep-90 -- Tim Burke
 *	Created this file for Alpha boot support.
 */

#include <sys/types.h>
#include <machine/rpb.h>
#include <machine/entrypt.h>
#include <varargs.h>

#define MAX_CONSOLE_STRING	1024

extern struct rpb_crb *crb;	/* Pointer to console routine block	  */
extern struct rpb_ctb *ctb;	/* Pointer to console terminal block	  */
extern struct rpb *hwrpb_addr;  /* Pointer to HWRPB                       */

unsigned char console_buffer[MAX_CONSOLE_STRING];
int cons_buf_index = 0;

/*
 * Scaled down version of C Library printf.
 * Used to print diagnostic information directly on console tty.
 * Since it is not interrupt driven, all system activities are
 * suspended.  Printf should not be used for chit-chat.
 *
 * One additional format: %b is supported to decode error registers.
 * Usage is:
 *	printf("reg=%b\n", regval, "<base><arg>*");
 * Where <base> is the output base expressed as a control character,
 * e.g. \10 gives octal; \20 gives hex.  Each arg is a sequence of
 * characters, the first of which gives the bit number to be inspected
 * (origin 1), and the next characters (up to a control character, i.e.
 * a character <= 64), give the name of the register.  Thus
 *	printf("reg=%b\n", 3, "\10\2BITTWO\1BITONE\n");
 * would produce output:
 *	reg=2<BITTWO,BITONE>
 */

/*
 * Fix ups remaining to do:
 * I have 2 concerns with this code:
 *	1) It is probably not 8-bit clean.
 *	2) A printf with 2 strings will cause a seg fault.  This is probably
 *	   due to the way the prf routine only goes up to 1 '\0' character.
 *	   consequently a printf of the following type will bomb:
 *	   printf("%s, %s\n","STRING1", "STRING2");
 */

/*VARARGS1*/
printf(fmt, va_alist)
	char *fmt;
	va_dcl
{
	va_list ap;

	va_start(ap);
	cons_buf_index = 0;
	console_buffer[cons_buf_index] = '\0';
	prf(fmt, ap);
	display_string();
}

/*VARARGS1*/
sprintf(cptr, fmt, va_alist)
	char *cptr;
	char *fmt;
	va_dcl
{
	va_list ap;

	va_start(ap);
	prf(fmt, ap);
}

prf(fmt, adx)
	register char *fmt;
	va_list adx;
{
	register long b, c, i;
	char *s;
	int any;
	register long is_long_number;

loop:
	is_long_number = 0;
	while ((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		putchar(c);
	}
again:
	c = *fmt++;
	/* THIS CODE IS VAX DEPENDENT IN HANDLING %l? AND %c */
	switch (c) {

	case 'l':
	case 'L':
		is_long_number = 1;
		goto again;
	case 'X':
		is_long_number = 1;
	case 'x':
		b = 16;
		goto number;
	case 'D':
		is_long_number = 1;
	case 'd':
	case 'u':		/* what a joke */
		b = 10;
		goto number;
	case 'o': case 'O':
		b = 8;
number:
		/*
		 * Cast to appropriate data type size.
		 */
		if (is_long_number) {
			printn(va_arg(adx, u_long), b);
		}
		else {
			printn(va_arg(adx, u_int), b);
		}
		break;
	case 'c':
		b = va_arg(adx, long);
		/*
		 * Print out the longword a byte at a time.  Strip to 7 bits.
	 	 */
		for (i = 56; i >= 0; i -= 8)
			if (c = (b >> i) & 0x7f)
				putchar(c);
		break;
	case 's':
		s = va_arg(adx, char *);
		while (c = *s++)
			putchar(c);
		break;
	}
	goto loop;
}

/*
 * Printn prints a number n in base b.
 * We don't use recursion to avoid deep kernel stacks.
 */
printn(n, b)
	u_long n;
{
	char prbuf[64];
	register char *cp;

	if (b == 10 && (long)n < 0) {
		putchar('-');
		n = (u_long)(-(long)n);
	}
	cp = prbuf;
	do {
		*cp++ = "0123456789abcdef"[n%b];
		n /= b;
	} while (n);
	do
		putchar(*--cp);
	while (cp > prbuf);
}

/*
 * Print a character on console.
 *
 * Since the alpha console has a puts (put STRING) instead of a 
 * putc (put CHARACTER) routine, this code simply inserts the character
 * into an output buffer to be sent to the console when the entire
 * string has been assembled.
 */
putchar(c)
	register c;
{

	/*
	 * Don't overflow the output buffer.
	 */
	if (cons_buf_index >= (MAX_CONSOLE_STRING - 1)) {
		return;
	}
	/*
	 * Map <NL> to <CR><NL>
	 */
	if( c == '\n' ) {
		putchar('\r');
	}
	console_buffer[cons_buf_index] = c;
	cons_buf_index++;
	console_buffer[cons_buf_index] = '\0';
}

/*
 * Now that the string has been assembled call the console puts routine
 * to display the string on the console output display device.
 *
 * Note that the prom_puts routine will not necessarily display the whole
 * string because of flow control or other such reasons.  At this time we
 * aren't checking the return value of puts to go back and try for the 
 * rest of the string.
 */
display_string()
{
	prom_puts(console_buffer);
	cons_buf_index = 0;
}
