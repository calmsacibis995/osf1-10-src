%{
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
 * @(#)$RCSfile: screentabl.l,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/08/14 12:25:42 $
 */
%}

%{
/*
 * screentabl.l
 *	scanner for screentab database
 *
 * 19 December 1988	Jeffrey Mogul/DECWRL
 *	Created.
 *	Copyright 1989, 1990 Digital Equipment Corporation
 */

#include <sys/types.h>
#include <netinet/in.h>
#include "screentab.h"
#include "y.tab.h"

char *malloc();
long makestring();

extern	long	yylval;
char linebuf[255];

/*
 * Reserved word table
 */
struct rw {
	char *name;
	int token;
} reserved_words[] = {
	{"accept",	ACCEPT},
	{"and",		AND},
	{"any",		ANY},
	{"between",	BETWEEN},
	{"default",	DEFAULT},
	{"for",		FOR},
	{"from",	FROM},
	{"host",	HOST},
	{"host-not",	HOSTNOT},
	{"icmp",	ICMP},
	{"infotype",	INFOTYPE},
	{"is",		IS},
	{"log",		LOG},
	{"multicast",	MULTICAST},
	{"net",		NET},
	{"net-not",	NETNOT},
	{"netmask",	NETMASK},
	{"notify",	NOTIFY},
	{"port",	PORT},
	{"port-not",	PORTNOT},
	{"proto",	PROTO},
	{"proto-not",	PROTONOT},
	{"reject",	REJECTIT},
	{"reserved",	RESERVED},
	{"subnet",	SUBNET},
	{"subnet-not",	SUBNETNOT},
	{"tcp",		TCP},
	{"to",		TO},
	{"type",	TYPE},
	{"type-not",	TYPENOT},
	{"udp",		UDP},
	{"xserver",	XSERVER},
	{0, 0}
};

%}

Word	[a-zA-Z_][0-9a-zA-Z_.-]*	/* maybe should include "$"? */
String	\"([^\"]|(\\\"))*\"	/* quotes are escaped within strings */

%%	/* beginning of rules section */

^.*$		{ 
		 strcpy(linebuf, yytext);
		 REJECT;
		} /* for error messages */


"/*".*"*/"	;	/* comment */
"#".*$		;	/* comment, too */
\n		;
[ \t]+		;	/* white space (not in a string) */

{Word}		{
		    int i;
		    
		    if ((i = rw_lookup(yytext)) < 0) {
		    	yylval = makestring(yytext);
			return(NAME);
		    }
		    return(reserved_words[i].token);
		}

{String}	{yytext[strlen(yytext)-1] = 0;		/* strip leading " */
		 yylval = makestring(&(yytext[1]));	/* strip trailing " */
		 return(LITERAL);
		}

0x[0-9a-fA-F]*	{	/* hex */
			yylval = makestring(yytext);
			return(HEXNUMBER);
		}

[0-9][0-9]*		{	/* decimal */
			yylval = makestring(yytext);
			return(DECNUMBER);
		}




"."		return(DOT);

";"		return(SEMICOLON);


%%	/* user subroutines section */

long
makestring(s)
char *s;
{
	char *snew = malloc(strlen(s)+1);
	
	strcpy(snew, s);
	
	return((long)snew);
}

/*
 * look up a reserved word; return the table index or -1 if not found.
 */
rw_lookup(word)
register char *word;
{
	register int i = 0;
	register struct rw *rp;
	
	rp = &(reserved_words[i]);
	
	while (rp->name) {
	    if (strcmp(word, rp->name) == 0)
	    	return(i);
	    rp++;
	    i++;
	}
	return(-1);
}
