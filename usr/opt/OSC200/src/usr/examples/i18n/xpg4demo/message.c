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
static char *rcsid = "@(#)$RCSfile: message.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/25 22:03:07 $";
#endif

#include <langinfo.h>
#include <varargs.h>
#include <errno.h>
#define MESSAGE_C
#include "xpg4demo.h"
#include "xpg4demo_msg.h"

void	Ask();
static	char *msgWarning;
static	char *msgError;
static	char *msgPanic;
int	NErrors = 0;

void
Fatal(va_alist)
va_dcl
{
	va_list args;
	char *fmt;

	if (FullScreenMode)
		GotoMessageLine();
	va_start(args);
	fmt = va_arg(args, char *);
	(void) vfprintf(stderr, fmt, args);
	va_end(args);
	(void) fprintf(stderr, GetErrorMsg(E_MES_FATAL2,". Exiting...\n"));
	exit(3);
}

void
NoMemory(const char *for_what)
{
	char buf[256];

	(void) strcpy(buf, for_what);
	Fatal(GetErrorMsg(E_MES_NOMEMORY, "No memory for %s"), buf);
}

int
Confirm(const char *msg)
{
       char buf[256];
       char ans[80];
       int response = TRUE;

       (void) strcpy(buf, msg);
       (void) strcat(buf, GetMsg(I_MES_YN," (yes/no) [yes]: "));
       for (;;) {
	       Ask((const char *) buf);
	       (void) gets(ans);
	       if (ans[0] == '\0' || strcmp(ans, nl_langinfo(YESSTR)) == 0)
		       break;
	       if (strcmp(ans, nl_langinfo(NOSTR)) == 0) {
		       response = FALSE;
		       break;
	       }
       }
       ErasePromptLine();
       return (response);
}

void
Ask(va_alist)
va_dcl
{
	va_list args;
	char *fmt;
	FILE *fp = stderr;

	if (FullScreenMode) {
		GotoPromptLine();
		fp = stdout;
	}
	va_start(args);
	fmt = va_arg(args, char *);
	(void) vfprintf(fp, fmt, args);
	va_end(args);
	fflush(fp);
}

void
Error(va_alist)
va_dcl
{
	va_list args;
	char *fmt;
	FILE *fp = stderr;

	if (FullScreenMode) {
		GotoMessageLine();
		fp = stdout;
	} else
		NErrors++;
	va_start(args);
	fmt = va_arg(args, char *);
	(void) vfprintf(fp, fmt, args);
	va_end(args);
	fputc((int) '.', fp);
	if (!FullScreenMode)
		fputc((int) '\n', fp);
	fflush(fp);
}

void
Warning(va_alist)
va_dcl
{
	va_list args;
	char *fmt;
	FILE *fp = stderr;

	if (FullScreenMode) {
		GotoMessageLine();
		fp = stdout;
	} else
		NErrors++;
	va_start(args);
	fmt = va_arg(args, char *);
	(void) vfprintf(fp, fmt, args);
	va_end(args);
	fputc((int) '.', fp);
	if (!FullScreenMode)
		fputc((int) '\n', fp);
	fflush(fp);
}

/*
 * Issue the "Aborting..." message and dump core.
 */
void
Panic(va_alist)
va_dcl
{
	va_list args;
	char *fmt;

	if (FullScreenMode)
		GotoMessageLine();
	va_start(args);
	fmt = va_arg(args, char *);
	(void) vfprintf(stderr, fmt, args);
	va_end(args);
	(void) fprintf(stderr, GetErrorMsg(E_MES_ABORT,". Aborting...\n"));
	abort();
	/*NOTREACHED*/
}


