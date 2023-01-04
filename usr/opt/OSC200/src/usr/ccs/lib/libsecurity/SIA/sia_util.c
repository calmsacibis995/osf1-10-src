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
static char *rcsid = "@(#)$RCSfile: sia_util.c,v $ $Revision: 1.1.7.3 $ (DEC) $Date: 1993/04/06 12:11:02 $";
#endif
/*****************************************************************************
*
*	Internal C2 mechanism support functions.
*
******************************************************************************/
#include <varargs.h>
#include <stdio.h>
#include <pwd.h>
#include <sia.h>
#include <siad.h>
#include "sia_mech.h"

#define	_PATH_NOLOGIN	"/etc/nologin"

int (*_c2_collect)() = sia_collect_trm;
int _c2_collinput = 1;

struct passwd nouser = { "", "nope", -1, -1, -1, "", "", "", "" };

/*
 * Function for creating and filling in the C2 mechanism specific
 * section of an entity structure.
 */
int c2_make_mech(entity, pkgind, prpwd, prterm)
SIAENTITY *entity;
int pkgind;
PR_PASSWD *prpwd;
PR_TERM *prterm;
{
	C2_MECH *mechp;

	mechp = (C2_MECH *) malloc(sizeof (C2_MECH));
	if(!mechp) {
		SIALOG("ERROR", "unable to malloc c2_mech struct");
		return SIADFAIL;
	}
	memset(mechp, 0, sizeof (C2_MECH));
	mechp->prpwd = prpwd;
	mechp->prterm = prterm;
	mechp->auth_type = entity->authtype;
	EN_MECH(entity, pkgind) = mechp;
	return SIADSUCCESS;
}

/*
 * Function for displaying a formatted error message using the
 * application supplied collect function.
 */
int show_error(va_alist)
va_dcl
{
	va_list ap;
	unsigned char *fmt, buff[1025];
	prompt_t prompt;

	va_start(ap);
	fmt = va_arg(ap, unsigned char *);
/*
 * Format the message.
 */
	vsprintf((char *)buff, (char *)fmt, ap);
	prompt.prompt = buff;
	prompt.control_flags = 0;
	va_end(ap);
	prompt.min_result_length = 0;
	prompt.max_result_length = 0;
/*
 * Try and display the message and return the status of the attempt.
 */
	if(_c2_collect)
		return (*_c2_collect)(0, SIAWARNING, "", 1, &prompt);
	else
		return SIADFAIL;
}

/*
 * Function for displaying a formatted informational message using
 * the application supplied collect function.
 */
int show_mesg(va_alist)
va_dcl
{
	va_list ap;
	unsigned char *fmt, buff[1025];
	prompt_t prompt;

	va_start(ap);
	fmt = va_arg(ap, unsigned char *);
/*
 * Format the message.
 */
	vsprintf((char *)buff, (char *)fmt, ap);
	prompt.prompt = buff;
	prompt.control_flags = 0;
	va_end(ap);
	prompt.min_result_length = 0;
	prompt.max_result_length = 0;
/*
 * Try and display the message and return the status of the attempt.
 */
	if(_c2_collect)
		return (*_c2_collect)(0, SIAINFO, "", 1, &prompt);
	else
		return SIADSUCCESS;
}

/*
 * Function for displaying a system error message.
 */
int show_perror(string)
char *string;
{
	extern int errno;

	return show_error("%s: %s\n", string, strerror(errno));
}

/*
 * Function for checking if logins are disabled.
 */
int checknologin()
{
	int fid, i;

	if((fid=open(_PATH_NOLOGIN, O_RDONLY)) >= 0) {
		if(_c2_collect) {
			prompt_t prompt;
			unsigned char tbuf[8192];

			while((i=read(fid, tbuf, (sizeof tbuf)-1)) > 0) {
				tbuf[i] = '\0';
				prompt.prompt = tbuf;
				prompt.min_result_length = 0;
				prompt.max_result_length = 0;
				prompt.control_flags = 0;
				(*_c2_collect)(0, SIAINFO, "", 1, &prompt);
			}
		}
		close(fid);
		return 1;
	}
	return 0;
}

#ifdef	DEBUG
/*
 * Debug function for displaying the contents of an entity.
 */
void print_entity(entity)
SIAENTITY *entity;
{
	int i;

	if(!entity) {
		puts("Entity = NIL");
		return;
	}
	printf("colinput = %d\n", entity->colinput);
	printf("authcount = %d\n", entity->authcount);
	printf("authtype = %d\n", entity->authtype);
	printf("name = %s\n", entity->name?entity->name:"NIL");
	printf("acctname = %s\n", entity->acctname?entity->acctname:"NIL");
	for(i=0; i < entity->argc; i++)
		printf("argv[%1d] = %s\n", i, entity->argv[i]?entity->argv[i]:"NIL");
	printf("argc = %d\n", entity->argc);
	printf("hostname = %s\n", entity->hostname?entity->hostname:"NIL");
	printf("tty = %s\n", entity->tty?entity->tty:"NIL");
	printf("error = %d\n", entity->error);
	if(entity->pwd)
		putpwent(entity->pwd, stdout);
	else
		puts("pwd = NIL");
}
#endif /* DEBUG */
