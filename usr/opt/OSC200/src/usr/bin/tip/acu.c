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
static char rcsid[] = "@(#)$RCSfile: acu.c,v $ $Revision: 4.2.6.4 $ (DEC) $Date: 1993/10/19 13:35:23 $";
#endif
/*
acu.c	1.6  com/cmd/tip,3.1,9013 12/21/89 16:51:33";
 */
/* 
 * COMPONENT_NAME: UUCP acu.c
 * 
 * FUNCTIONS: MSGSTR, acuabort, acutype, connect, disconnect 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* Static char sccsid[] = "acu.c	5.3 (Berkeley) 4/3/86"; */

#include "tip.h"

#ifndef BUFSIZE
#define BUFSIZE          1024
#endif
static acu_t *acu = NOACU;
static int conflag;
static int acuabort();
static acu_t *acutype();
static jmp_buf jmpbuf;

/*
 * Establish connection for tip
 *
 * If DU is true, we should dial an ACU whose type is AT.
 * The phone numbers are in PN, and the call unit is in CU.
 *
 * If the PN is an '@', then we consult the PHONES file for
 *   the phone numbers.  This file is /etc/phones, unless overriden
 *   by an exported shell variable.
 *
 * The data base files must be in the format:
 *	host-name[ \t]*phone-number
 *   with the possibility of multiple phone numbers
 *   for a single host acting as a rotary (in the order
 *   found in the file).
 */
#ifdef GENACU
extern int gen_dialer(), gen_disconnect(), gen_abort();
acu_t gen = { "generic", gen_dialer, gen_disconnect, gen_abort };
#endif
char *
connect()
{
	register char *cp = PN;
	char *phnum, string[256];
	FILE *fd;
	int tried = 0;
#ifdef GENACU
        extern int generrno;
#endif

	if (!DU) {		/* regular connect message */
		if (CM != NOSTR)
			pwrite(FD, CM, size(CM));
		logent(value(HOST), "", DV, MSGSTR(CALLCOMPLETE, "call completed"));
		return (NOSTR);
	}
	/*
	 * @ =>'s use data base in PHONES environment variable
	 *        otherwise, use /etc/phones
	 */
	signal(SIGINT, (void(*)(int)) acuabort);
	signal(SIGQUIT, (void(*)(int)) acuabort);
	if (setjmp(jmpbuf)) {
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		printf(MSGSTR(CALLABORT, "\ncall aborted\n")); /*MSG*/
		logent(value(HOST), "", "", MSGSTR(CALLABORT2, "call aborted")); /*MSG*/
		if (acu != NOACU) {
			boolean(value(VERBOSE)) = FALSE;
			if (conflag)
				disconnect(NOSTR);
			else
				(*acu->acu_abort)();
		}
		return( MSGSTR(INTERRUPT, "interrupt"));
	}
        if ( AT == (char *) 0) {
           return (MSGSTR(NOACUTYPE,"The ACU type 'at' in /etc/remote is NOT setup."));
        }
	if ((acu = acutype(AT)) == NOACU)
		return (MSGSTR(UNKNOWNACU, "unknown ACU type")); /*MSG*/
	if (*cp != '@') {
		while (*cp) {
			for (phnum = cp; *cp && *cp != ','; cp++)
				;
			if (*cp)
				*cp++ = '\0';
			
			if (conflag = (*acu->acu_dialer)(phnum, CU)) {
				logent(value(HOST), phnum, acu->acu_name,
					MSGSTR(CALLCOMPLETE, "call completed")); /*MSG*/
				return (NOSTR);
			} else{
				logent(value(HOST), phnum, acu->acu_name,
					MSGSTR(CALLFAIL,"call failed")); /*MSG*/
#ifdef GENACU
#define NOSYNC 1
#define BADDIAL 2
#define NOCAR 3
                                if(generrno != 0) {
                                        if(generrno == NOSYNC) {
                                                printf("can't synchronize\n");
                    logent(value(HOST), phnum, "generic", "can't synch up");
                                        } else if(generrno == BADDIAL) {
                                                printf("error dialing\n");
                                        } else if(generrno == NOCAR) {
                                  printf("No carrier detected...\n");
                    logent(value(HOST), phnum, "generic", "dialer timeout");
                                        }
                                }
#endif
			} /* Added from GENACU - this } to integrate DEC OSF*/

			tried++;
		}
	} else {
		if ((fd = fopen(PH, "r")) == (FILE *)TIP_NOFILE) {
			printf("%s: ", PH);
			return (MSGSTR(NOPNFILE, "can't open phone number file")); /*MSG*/
		}
		while (fgets(string, sizeof(string), fd) != NOSTR) {
			for (cp = string; !any(*cp, " \t\n"); cp++)
				;
			if (*cp == '\n') {
				fclose(fd);
				return (MSGSTR(UNRECHOST, "unrecognizable host name")); /*MSG*/
			}
			*cp++ = '\0';
			if (strcmp(string, value(HOST)))
				continue;
			while (any(*cp, " \t"))
				cp++;
			if (*cp == '\n') {
				fclose(fd);
				return (MSGSTR(NOPN, "missing phone number")); /*MSG*/
			}
			for (phnum = cp; *cp && *cp != ',' && *cp != '\n'; cp++)
				;
			if (*cp)
				*cp++ = '\0';
			
			if (conflag = (*acu->acu_dialer)(phnum, CU)) {  
				fclose(fd);
				logent(value(HOST), phnum, acu->acu_name,
					MSGSTR(CALLCOMPLETE, "call completed")); /*MSG*/
				return (NOSTR);
			} else
				logent(value(HOST), phnum, acu->acu_name,
					MSGSTR(CALLFAIL, "call failed")); /*MSG*/
			tried++;
		}
		fclose(fd);
	}
	if (!tried)
		logent(value(HOST), "", acu->acu_name, MSGSTR(NOPN, "missing phone number")); /*MSG*/
	else
		(*acu->acu_abort)();
	return (tried ? MSGSTR(CALLFAIL, "call failed") : MSGSTR(NOPN, "missing phone number")); /*MSG*/ /*MSG*/
}

disconnect(reason)
	char *reason;
{
	if (!conflag) {
		logent(value(HOST), "", DV, MSGSTR(CALLTERM, "call terminated")); /*MSG*/ 
		return;
	}
	if (reason == NOSTR) {
		logent(value(HOST), "", acu->acu_name, MSGSTR(CALLTERM, "call terminated")); /*MSG*/
		if (boolean(value(VERBOSE)))
			printf(MSGSTR(DISCONNECTING, "\r\ndisconnecting...")); /*MSG*/
	} else 
		logent(value(HOST), "", acu->acu_name, reason);
	(*acu->acu_disconnect)();
}

static int
acuabort(s)
{
	signal(s, SIG_IGN);
	longjmp(jmpbuf, 1);
}

static acu_t *
acutype(s)
	register char *s;
{
	register acu_t *p;
	extern acu_t acutable[];

#ifdef GENACU
        extern acu_t *gen_setup();
        char gbuf[BUFSIZE];

        if (agetent(gbuf, s) > 0) {
                (void) gen_setup(gbuf, FD);
                return (&gen);
        }
        else
#endif
	for (p = acutable; p->acu_name != '\0'; p++)
		if (!strcmp(s, p->acu_name))
			return (p);
	return (NOACU);
}
