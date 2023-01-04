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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: altconn.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/09/07 14:49:22 $";
#endif
/* 
 * COMPONENT_NAME: UUCP altconn.c
 * 
 * FUNCTIONS: altconn 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* "altconn.c	1.4  com/cmd/uucp,3.1,9013 11/30/89 09:15:09"; */
/*  "altconn.c	5.1 -  - "; */
/*	altconn.c	1.8 			*/
#include "uucp.h"

extern char *strrchr();

static struct call {		/* cu calling info-also in cu.c */
				/* make changes in both places!*/
	char *speed;		/* transmission baud rate */
	char *line;		/* device name for outgoing line */
	char *telno;		/* ptr to tel-no digit string */
	char *class;		/* class of call */
} call;

/*

 * altconn - place a telephone call to system 
 * from cu when telephone number or direct line used
 *
 * return codes:
 *	FAIL - connection failed
 *	>0  - file no.  -  connect ok
 * When a failure occurs, Uerror is set.
 */

altconn(call)
struct call *call;
{
	int nf, fn = FAIL, i;
	char *alt[7];
	extern char *Myline;

	alt[F_NAME] = "dummy";	/* to replace the Systems file fields  */
	alt[F_TIME] = "Any";	/* needed for getto(); [F_TYPE] and    */
	alt[F_TYPE] = "";	/* [F_PHONE] assignment below          */
	alt[F_CLASS] = call->speed;
	alt[F_PHONE] = "";
	alt[F_LOGIN] = "";
	alt[6] = NULL;

	CDEBUG(4,MSGSTR(MSG_ALTC_CD1,"altconn called\r\n"),"");

	/* cu -l dev ... */
	if(call->line != NULL) {
		Myline = BASENAME((call->line),'/');
	}

	/* cu  ... telno */
	if(call->telno != NULL) {
		alt[F_PHONE] = call->telno;
		alt[F_TYPE] = "ACU";
	}
	/* cu direct line */
	else {
		alt[F_TYPE] = "Direct";
	}

#ifdef forfutureuse
	if (call->class != NULL)
		alt[F_TYPE] = call->class;
#endif


	fn = getto(alt);
	CDEBUG(4, MSGSTR(MSG_ALTC_CD2,"getto ret %d\n"), fn);

	return(fn);

}

