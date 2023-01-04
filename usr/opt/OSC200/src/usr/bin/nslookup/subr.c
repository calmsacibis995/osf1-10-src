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
static char	*sccsid = "@(#)$RCSfile: subr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:44:26 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

/*
#ifndef lint

#endif /* not lint */

/*
 *******************************************************************************
 *
 *  subr.c --
 *
 *	Miscellaneous subroutines for the name server 
 *	lookup program.
 *  
 *	Copyright (c) 1985 
 *  	Andrew Cherenson
 *	U.C. Berkeley
 *  	CS298-26  Fall 1985
 *
 *******************************************************************************
 */

#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>
#include <signal.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "res.h"



/*
 *******************************************************************************
 *
 *  IntrHandler --
 *
 *	This routine is called whenever a control-C is typed. 
 *	It performs three main functions:
 *	 - close an open socket connection.
 *	 - close an open output file (used by LookupHost, et al.)
 *	 - jump back to the main read-eval loop.
 *		
 *
 *      If a user types a ^C in the middle of a routine that uses a socket,
 *      the routine would not be able to close the socket. To prevent an
 *      overflow of the process's open file table, the socket and output
 *      file descriptors are closed by the interrupt handler.
 *
 *  Side effects:
 *      Open file descriptors are closed.
 *      If filePtr is valid, it is closed.
 *	Flow of control returns to the main() routine.
 *
 *******************************************************************************
 */

void
IntrHandler()
{
    extern jmp_buf env;
    extern FILE *yyin;          /* scanner input file */
#ifdef notyet
    extern void yyrestart();    /* routine to restart scanner after interrupt */
#endif

    SendRequest_close();
    ListHost_close();
    if (filePtr != NULL && filePtr != stdout) {
        fclose(filePtr);
        filePtr = NULL;
    }

    printf( MSGSTR( NEWLINE, "\n"));
#ifdef notyet
    yyrestart(yyin);
#endif
    longjmp(env, 1);
}


/*
 *******************************************************************************
 *
 *  Malloc --
 *  Calloc --
 *
 *      Calls the malloc library routine with SIGINT blocked to prevent
 *      corruption of malloc's data structures. We need to do this because
 *      a control-C doesn't kill the program -- it causes a return to the
 *      main command loop.
 *
 *      NOTE: This method doesn't prevent the pointer returned by malloc
 *      from getting lost, so it is possible to get "core leaks".
 *
 *
 *  Results:
 *	(address)	- address of new buffer.
 *
 *******************************************************************************
 */

char *
Malloc(size)
    int size;
{
    char        *ptr;
    int saveMask;

    saveMask = sigblock(sigmask(SIGINT));
    ptr = malloc((unsigned) size);
    (void) sigsetmask(saveMask);

    if (ptr == NULL) {
        fflush(stdout);
        fprintf(stderr, "malloc failed\n");
        fflush(stderr);
        abort();
        /*NOTREACHED*/
    } else {
        return(ptr);
    }
}

char *
Calloc(num, size)
    register int num, size;
{
    char *ptr = Malloc(num*size);
    bzero(ptr, num*size);
    return(ptr);
}


/*
 *******************************************************************************
 *
 *  PrintHostInfo --
 *
 *	Prints out the HostInfo structure for a host.
 *
 *******************************************************************************
 */

void
PrintHostInfo(file, title, hp)
	FILE 	*file;
	char 	*title;
	register HostInfo *hp;
{
	register char 		**cp;
	register ServerInfo 	**sp;
	char 			comma;
	int  			i;

	fprintf(file, MSGSTR( FORMAT27, "%-7s  %s\n"), title, hp->name);

	if (hp->addrList != NULL) {
	    if (hp->addrList[1] != NULL) {
		fprintf(file, MSGSTR( ADDRS, "Addresses:"));
	    } else {
		fprintf(file, MSGSTR( ADDRS1, "Address:"));
	    }
	    comma = ' ';
	    i = 0;
	    for (cp = hp->addrList; cp && *cp; cp++) {
		i++;
		if (i > 4) {
		    fprintf(file, MSGSTR( FORMAT28, "\n\t"));
		    comma = ' ';
		    i = 0;
		}
		fprintf(file, MSGSTR( FORMAT29, "%c %s"), comma, inet_ntoa(*(struct in_addr *)*cp));
		comma = ',';
	    }
	}

	if (hp->aliases != NULL) {
	    fprintf(file, MSGSTR( ALIASES, "\nAliases:"));
	    comma = ' ';
	    i = 10;
	    for (cp = hp->aliases; cp && *cp && **cp; cp++) {
		i += strlen(*cp) + 2;
		if (i > 75) {
		    fprintf(file, MSGSTR( FORMAT28, "\n\t"));
		    comma = ' ';
		    i = 10;
		}
		fprintf(file,  MSGSTR( FORMAT29, "%c %s"), comma, *cp);
		comma = ',';
	    }
	}

	if (hp->servers != NULL) {
	    fprintf(file, MSGSTR( SERVED_BY, "Served by:\n"));
	    for (sp = hp->servers; *sp != NULL ; sp++) {

		fprintf(file, MSGSTR( FORMAT30, "- %s\n\t"),  (*sp)->name);

		comma = ' ';
		i = 0;
		for (cp = (*sp)->addrList; cp && *cp && **cp; cp++) {
		    i++;
		    if (i > 4) {
			fprintf(file, MSGSTR( FORMAT28, "\n\t"));
			comma = ' ';
			i = 0;
		    }
		    fprintf(file, 
			MSGSTR( FORMAT29, "%c %s"), comma, inet_ntoa(*(struct in_addr *)*cp));
		    comma = ',';
		}
		fprintf(file, MSGSTR( FORMAT28, "\n\t"));

		comma = ' ';
		i = 10;
		for (cp = (*sp)->domains; cp && *cp && **cp; cp++) {
		    i += strlen(*cp) + 2;
		    if (i > 75) {
			fprintf(file, MSGSTR( FORMAT28, "\n\t"));
			comma = ' ';
			i = 10;
		    }
		    fprintf(file, MSGSTR( FORMAT29, "%c %s"), comma, *cp);
		    comma = ',';
		}
		fprintf(file, MSGSTR( NEWLINE, "\n"));
	    }
	}

	fprintf(file, MSGSTR( FORMAT31, "\n\n"));
}

/*
 *******************************************************************************
 *
 *  OpenFile --
 *
 *	Parses a command string for a file name and opens
 *	the file.
 *
 *  Results:
 *	file pointer	- the open was successful.
 *	NULL		- there was an error opening the file or
 *			  the input string was invalid.
 *
 *******************************************************************************
 */

FILE *
OpenFile(string, file)
    char *string;
    char *file;
{
	char 	*redirect;
	FILE 	*tmpPtr;

	/*
	 *  Open an output file if we see '>' or >>'.
	 *  Check for overwrite (">") or concatenation (">>").
	 */

	redirect = strchr(string, '>');
	if (redirect == NULL) {
	    return(NULL);
	}
	if (redirect[1] == '>') {
	    sscanf(redirect, MSGSTR( ARROWS, ">> %s"), file);
	    tmpPtr = fopen(file, "a+");
	} else {
	    sscanf(redirect, MSGSTR( ARROWS_FILE, "> %s"), file);
	    tmpPtr = fopen(file, "w");
	}

	if (tmpPtr != NULL) {
	    redirect[0] = '\0';
	}

	return(tmpPtr);
}

/*
 *******************************************************************************
 *
 *  DecodeError --
 *
 *	Converts an error code into a character string.
 *
 *******************************************************************************
 */

char *
DecodeError(result)
    int result;
{
	switch(result) {
	    case NOERROR: 	return( MSGSTR( ERR_SUCCESS, "Success")); 
				break;
	    case FORMERR:	return( MSGSTR( ERR_FORMAT, "Format error")); 
				break;
	    case SERVFAIL:	return( MSGSTR( ERR_SERVER, "Server failed")); 
				break;
	    case NXDOMAIN:	return( MSGSTR( ERR_NODOMAIN, 
					"Non-existent domain"));
				break;
	    case NOTIMP:	return( MSGSTR( ERR_NOT_IMP, "Not implemented"));
				break;
	    case REFUSED:	return( MSGSTR( ERR_QUERY, "Query refused")); 
				break;
	    case NOCHANGE:	return( MSGSTR( ERR_NOCHANGE, "No change")); 
				break;
	    case NO_INFO: 	return( MSGSTR( ERR_NOINFO, "No information"));
				break;
	    case ERROR: 	return( MSGSTR( ERR_UNSPEC, "Unspecified error")); 				break;
	    case TIME_OUT: 	return( MSGSTR( ERR_TIMEOUT, "Timed out")); 	
				break;
	    case NONAUTH: 	return( MSGSTR( ERR_NOAUTH, 
					"Non-authoritative answer")); 
				break;
	    default: 		break;
	}
	return(MSGSTR( BAD_VALUE, "BAD ERROR VALUE")); 
}

int
StringToClass(class, dflt)
    char *class;
    int dflt;
{
	if (strcasecmp(class, MSGSTR( IN, "IN")) == 0)
		return(C_IN);
	if (strcasecmp(class, MSGSTR( CHAOS, "CHAOS")) == 0)
		return(C_CHAOS);
	if (strcasecmp(class, MSGSTR( ANY, "ANY")) == 0)
		return(C_ANY);
	fprintf(stderr, MSGSTR( UNKN_QUERY, "unknown query class: %s\n"), class);
	return(dflt);
}
/*
 *******************************************************************************
 *
 *  StringToType --
 *
 *	Converts a string form of a query type name to its 
 *	corresponding integer value.
 *
 *******************************************************************************
 */

int
StringToType(type, dflt)
    char *type;
    int dflt;
{
	if (strcasecmp(type, MSGSTR( A, "A")) == 0)
		return(T_A);
	if (strcasecmp(type, MSGSTR( NS, "NS")) == 0)
		return(T_NS);			/* authoritative server */
	if (strcasecmp(type, MSGSTR( MX, "MX")) == 0)
		return(T_MX);			/* mail exchanger */
	if (strcasecmp(type, MSGSTR( CNAME, "CNAME")) == 0)
		return(T_CNAME);		/* canonical name */
	if (strcasecmp(type, MSGSTR( SOA, "SOA")) == 0)
		return(T_SOA);			/* start of authority zone */
	if (strcasecmp(type, MSGSTR( MB, "MB")) == 0)
		return(T_MB);			/* mailbox domain name */
	if (strcasecmp(type, MSGSTR( MG, "MG")) == 0)
		return(T_MG);			/* mail group member */
	if (strcasecmp(type, MSGSTR( MR, "MR")) == 0)
		return(T_MR);			/* mail rename name */
	if (strcasecmp(type, MSGSTR( WKS, "WKS")) == 0)
		return(T_WKS);			/* well known service */
	if (strcasecmp(type, MSGSTR( PTR, "PTR")) == 0)
		return(T_PTR);			/* domain name pointer */
	if (strcasecmp(type, MSGSTR( HINFO, "HINFO")) == 0)
		return(T_HINFO);		/* host information */
	if (strcasecmp(type, MSGSTR( MHINFO, "MINFO")) == 0)
		return(T_MINFO);		/* mailbox information */
	if (strcasecmp(type, MSGSTR( AXFER, "AXFR")) == 0)
		return(T_AXFR);			/* zone transfer */
	if (strcasecmp(type, "MAILA") == 0)
                return(T_MAILA);                /* mail agent */
	if (strcasecmp(type, MSGSTR( MAILB, "MAILB")) == 0)
		return(T_MAILB);		/* mail box */
	if (strcasecmp(type, MSGSTR( ANY, "ANY")) == 0)
		return(T_ANY);			/* matches any type */
	if (strcasecmp(type, MSGSTR( UINFO, "UINFO")) == 0)
		return(T_UINFO);		/* user info */
	if (strcasecmp(type, MSGSTR( UID, "UID")) == 0)
		return(T_UID);			/* user id */
	if (strcasecmp(type, MSGSTR( GID, "GID")) == 0)
		return(T_GID);			/* group id */
	if (strcasecmp(type, "TXT") == 0)
                return(T_TXT);                  /* text */
	fprintf(stderr, MSGSTR( UNKN_TYPE, "unknown query type: %s\n"), type);
	return(dflt);
}

/*
 *******************************************************************************
 *
 *  DecodeType --
 *
 *	Converts a query type to a descriptive name.
 *	(A more verbose form of p_type.)
 *
 *
 *******************************************************************************
 */

static  char nbuf[20];

char *
DecodeType(type)
	int type;
{
	switch (type) {
	case T_A:
		return(MSGSTR( ADDRESS, "address"));
	case T_NS:
		return(MSGSTR( NAMESER, "name server"));
	case T_CNAME:		
		return(MSGSTR( CANN_NAME, "cannonical name"));
	case T_SOA:		
		return(MSGSTR( START_AUTH, "start of authority zone"));
	case T_MB:		
		return(MSGSTR( MAIL_DOMAIN, "mailbox domain name"));
	case T_MG:		
		return(MSGSTR( MAIL_GRP, "mail group member"));
	case T_MR:		
		return(MSGSTR( MAIL_REN, "mail rename name"));
	case T_NULL:		
		return(MSGSTR( NULL_RESOURCE, "null resource record"));
	case T_WKS:		
		return(MSGSTR( KNOWN_SERVICE, "well known service"));
	case T_PTR:		
		return(MSGSTR( DOMAIN_PTR, "domain name pointer"));
	case T_HINFO:		
		return(MSGSTR( HOST_TYPE, "host"));
	case T_MINFO:		
		return(MSGSTR( MAIL_MINFO, "mailbox (MINFO)"));
	case T_MX:		
		return(MSGSTR( MAILEXCH, "mail exchanger"));
	case T_TXT:
                return("text");
	case T_AXFR:		
		return(MSGSTR( ZONE_TRANS, "zone transfer"));
	case T_MAILB:		
		return(MSGSTR( MAIL_BOX, "mail box"));
	case T_ANY:		
		return(MSGSTR( ANY_TYPE, "any type"));
	case T_UINFO:
		return(MSGSTR( USER_INFO, "user info"));
	case T_UID:
		return(MSGSTR( USER_ID, "user id"));
	case T_GID:
		return(MSGSTR( GROUP_ID, "group id"));
	default:
		(void) sprintf(nbuf, MSGSTR( FORMAT32, "%d"), type);
		return (nbuf);
	}
}
