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
static char *rcsid = "@(#)$RCSfile: scu_help.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 09:29:12 $";
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
 * File:	scu_help.c
 * Author:	Robin T. Miller
 * Date:	September 24, 1991
 *
 * Description:
 *	This file contains the functions to obtain and display help text
 * for the user.
 *
 * Modification History:
 *
 */
#include <stdio.h>
#include "scu.h"

/*
 * External Declarations:
 */
extern char *getenv(char *name);
extern int portable_help (char *inp_libr, char *inp_usrtop, char *inp_initop,
			  int inp_paged, int (*inp_ttyget)(),
			  int (*inp_ttyput)(), int (*inp_ttyerr)());
extern int OpenPager (char *pager);
extern int PromptInput (char *prompt, char *bufptr, int bufsiz);
extern int PutOutput (char *bufptr);

static char *ScuHelp;
static char *ScuTopic = "scu";

/************************************************************************
 *									*
 * Help()	Display Help Text for User.				*
 *									*
 * Description:								*
 *	This function is a front-end to the portable help function
 * Inputs:	ce = The command table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
Help()
{
	if (ScuHelp == NULL) {
	   if ( (ScuHelp = getenv ("SCU_HELP")) == NULL) {
#ifdef OSF
		ScuHelp = "/sbin/scu.hlp";
#else /* OSF */
		ScuHelp = "/bin/scu.help";
#endif /* OSF */

	   }
	}
	(void) OpenPager (NULL);
	return (portable_help (	ScuHelp,	/* Name of help file.	*/
				HelpTopic,	/* User help topic.	*/
				ScuTopic,	/* Initial help topic.	*/
			        FALSE,		/* Page output flag.	*/
				PromptInput,	/* Get input function.	*/
				PutOutput,	/* Put output function.	*/
				(int (*)()) 0));/* Put error function.	*/
}
