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
static char	*sccsid = "@(#)$RCSfile: getpasswd.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/08/24 22:33:08 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1988-90, SecureWare, Inc.
 *   All rights reserved
 */


/*
 * Based on:

 */

/*LINTLIBRARY*/

#include <sys/secdefines.h>
#include "libsecurity.h"

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <termio.h>
#include <macros.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>

/*
 * The return value of the signal() system call differs between systems.
 */

#if defined(SYSV_3) || defined(_OSF_SOURCE)
typedef void ((*SIGNAL_T)());
typedef void SIGFUNC_T;
#else
typedef int ((*SIGNAL_T)());
typedef int SIGFUNC_T;
#endif

#ifdef M_XENIX
static int was_eof = 0;		/* ^d flag for pw_was_eof() */
#endif

static int intrupt;
static char ciphertext[AUTH_CIPHERTEXT_SIZE(AUTH_SEGMENTS(AUTH_MAX_PASSWD_LENGTH))];


extern char *strncpy();
extern char *strcat();
extern char *memset();
extern char *crypt();
extern char *crypt16();
extern void setbuf();
extern FILE *fopen();

char *fgetpasswd();

static SIGFUNC_T	catch();

#ifdef	BIGCRYPT_DEBUG
main(argc, argv)
	int argc;
	char *argv[];
{
	char *salt;

	if (argc < 2)  {
		printf(MSGSTR(GETPASSWD_1, "use: bigcrypt cleartext [salt]\n"));
		exit(1);
	}

	if (argc < 3)
		salt = "xx";
	else
		salt = argv[2];

	(void) printf(MSGSTR(GETPASSWD_2, "final passwd = '%s'\n"), bigcrypt(argv[1], salt));
}
#endif


/*
 * Act like crypt(), with an extra argument, handling longer strings.
 * Args are (cleartext, salt, alg_index).  The third arg is to select
 * which crypt-style routine to call.  Values at this time are:
 *	AUTH_CRYPT_BIGCRYPT	SecureWare default
 *	AUTH_CRYPT_CRYPT16	ULTRIX compatibility
 *
 * Return whatever the called function returns.
 */
char *
dispcrypt(cleartext, salt, alg_index)
char const *cleartext;
char const *salt;
int alg_index;
{
	switch(alg_index) {
	  case AUTH_CRYPT_CRYPT16:
		return crypt16(cleartext, salt);
	  case AUTH_CRYPT_BIGCRYPT:
	  default:
		return bigcrypt(cleartext, salt);
	}
}


/*
 * Act like crypt(), but handle much larger strings.  The way bigcrypt() works
 * is to take segments of the cleartext and encrypt them individually, at first
 * using the salt passed in, and then using the first two characters of the
 * previous encrypted as the salt for the next segment.  (That is to avoid
 * duplicated ciphertext chunks when the password characters are repeated, so
 * that the encryption of a segment involves the encryption of all the
 * previous segments.)
 *
 * Each ciphertext segment is concatenated (with the salt at the
 * beginning) to form the entire encrypted string.
 */
char *
bigcrypt(cleartext, salt)
	register char *cleartext;
	register char *salt;
{
	register char *ciphertext_segment;
	register int current_segment;
	register int segments;

	check_auth_parameters();

	/*
	 * Do not overflow the static structure.  Encrypt for as far as
	 * the buffer will hold, or until the string is fully encrypted,
	 * whichever comes first.  For the empty string, ensure at least
	 * one pass through the loop.
	 */
	segments = max(min(AUTH_SEGMENTS(strlen(cleartext)),
			   AUTH_SEGMENTS(AUTH_MAX_PASSWD_LENGTH)), 1);

	(void) strncpy(ciphertext, salt, AUTH_SALT_SIZE);
	ciphertext[AUTH_SALT_SIZE] = '\0';

	for (current_segment = 0; current_segment < segments;
	     current_segment++)  {
		ciphertext_segment = crypt(cleartext, salt);

		/*
		 * Skip over fixed length salt prepended to the ciphertext
		 * by crypt() that we have already stored.
		 */
		(void) strcat(ciphertext, ciphertext_segment + AUTH_SALT_SIZE);
		cleartext += AUTH_CLEARTEXT_SEG_CHARS;

		/*
		 * The new salt is the first two (non-salt) characters
		 * of the ciphertext of the (soon to be) previous segment.
		 */
		salt = ciphertext + AUTH_SALT_SIZE +
		       current_segment * AUTH_CIPHERTEXT_SEG_CHARS;
	}

	/*
	 * Clear from the end of the ciphertext to the end of the buffer.
	 */
	(void) memset(ciphertext + AUTH_SALT_SIZE +
			segments * AUTH_CIPHERTEXT_SEG_CHARS,
		      '\0', 
		      (AUTH_SEGMENTS(AUTH_MAX_PASSWD_LENGTH) - segments) *
			AUTH_CIPHERTEXT_SEG_CHARS);

	return (char *) ciphertext;
}



/*
 * Act much like getpass(3S), except that here the size of the
 * password field is adjustable to handle variable length maximum
 * password sizes.  Also, if the prompt is the NULL string, don't
 * get a new password, but merely clear the cleartext buffer.
 */
char *
getpasswd(prompt, max_size)
	char *prompt;
	int max_size;
{
	return (fgetpasswd (prompt, max_size, (FILE *) 0, (FILE *) 0));
}

char *
fgetpasswd (prompt, max_size, file_in, file_out)
	char *prompt;
	int max_size;
	FILE *file_in;
	FILE *file_out;
{

#ifdef _OSF_SOURCE
	struct termios ttyb;
#else
	struct termio ttyb;
#endif
	unsigned short flags;
	register char *p;
	register int c;
	register int buflen;
	FILE	*fi;
	static char cleartext[AUTH_MAX_PASSWD_LENGTH+1];
	SIGNAL_T	sig;

	check_auth_parameters();

#ifdef M_XENIX
	was_eof = 0;
#endif
	if (prompt == (char *) 0)
		memset(cleartext, '\0', sizeof(cleartext));
	else  {
		if (file_in == (FILE *) 0) {
			if((fi = fopen("/dev/tty", "r+")) == (FILE *) 0)
				return((char*) 0);
		} else
			fi = file_in;

		if (file_out == (FILE *) 0)
			file_out = stderr;
		setbuf(fi, (char*) 0);
		sig = signal(SIGINT, catch);
		intrupt = 0;
#ifdef _OSF_SOURCE
		(void) ioctl(fileno(fi), TIOCGETA, &ttyb);
#else
		(void) ioctl(fileno(fi), TCGETA, &ttyb);
#endif
		flags = ttyb.c_lflag;
		ttyb.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
#ifdef _OSF_SOURCE
		(void) ioctl(fileno(fi), TIOCSETAF, &ttyb);
#else
		(void) ioctl(fileno(fi), TCSETAF, &ttyb);
#endif
		(void) fputs(prompt, file_out);
		buflen = min(sizeof(cleartext)-1, max_size);
		for (p=cleartext;
		     !intrupt && ((c = getc(fi)) != '\n') && (c != EOF); ) {
			if (p < cleartext + buflen)  {
				*p = c;
				p++;
			}
		}
		*p = '\0';
		(void) putc('\n', file_out);
#ifdef M_XENIX
		if(c==EOF)
		   was_eof = 1;
#endif
		ttyb.c_lflag = flags;
#ifdef _OSF_SOURCE
		(void) ioctl(fileno(fi), TIOCSETA, &ttyb);
#else
		(void) ioctl(fileno(fi), TCSETA, &ttyb);
#endif
		(void) signal(SIGINT, sig);
		if(fi != stdin)
			(void) fclose(fi);
		if(intrupt)
			(void) kill(getpid(), SIGINT);
	}

	return cleartext;
}
#ifdef M_XENIX
/*
 * This procedure, called by sulogin_proper_passwd() after getpasswd(),
 * returns the value of was_eof. This will tell sulogin that
 * ^d was entered as a password terminator, so let's go multi-user.
 * The sulogin feature is V.3.2 only.
 */
int
pw_was_eof()
{
	return(was_eof);
}
#endif

/*
 * Note when an interrupt is received through the keyboard.
 */

static SIGFUNC_T
catch()
{
	++intrupt;
}

/* #endif */ /*} SEC_BASE */
