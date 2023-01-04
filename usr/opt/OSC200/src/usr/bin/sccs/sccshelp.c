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
static char	*sccsid = "@(#)$RCSfile: sccshelp.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/11 19:00:39 $";
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
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: ask, clean_up, findprt, lochelp, main
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * sccshelp.c 1.9 com/cmd/sccs/cmd,3.1,9021 1/4/90 18:10:55";
 */

# include	"defines.h"

# include 	"sccshelp_msg.h"
#define MSGSTR(Num, Str) catgets(catd, MS_SCCSHELP, Num, Str)

#ifdef TRACE
#define tr(x,y,w,z) fprintf(stderr,"%s %s %s %s\n",x,y,w,z);
#else	
#define tr(x,y,w,z) /* x y w z */
#endif

/*
     Program to locate helpful info in an ascii file.
     The program accepts a variable number of arguments.
*/

/*
     If MSG is defined then the help message file is assumed to be
     a message catalog.  The alphabetic portion of the argument is
     used as the message catalog name and a catopen is done on the 
     file to open it.  NLSPATH is assumed to be set up to find the
     specified message catalog.  If the catopen fails, meaning that
     the file could not be found then the normal help procssing is 
     done (see below).  If the catopen is successful then the numeric
     portion of the argument is used as a message set identifier (after
     changing 0 to 49, to account for set 0 not supported), and
     all messages in the message set are displayed.  Note that the
     messages in the message set must be contiguous starting at
     the value MSGSTART because the printing of messages from the message
     set terminates on the first failure of catgets.  If the argument
     has no numeric portion then all messages in message set 49 are
     displayed.
*/

/*
     The file to be searched is determined from the argument. If the
     argument does not contain numerics, the search will be attempted 
     on '/usr/share/lib/sccshelp/cmds', with the search key being the 
     whole argument.
     If the argument begins with non-numerics but contains numerics 
     (e.g, zz32) the file /usr/share/lib/sccshelp/helploc will be checked 
     for a file corresponding to the non numeric prefix. That file will 
     then be seached for the message. If /usr/share/lib/sccshelp/helploc
     does not exist or the prefix is not found there the search will
     be attempted on '/usr/share/lib/sccshelp/<non-numeric prefix>',
     (e.g,/usr/share/lib/sccshelp/zz), with the search key being 
     <remainder of arg>,(e.g., 32).
     If the argument is all numeric, or if the file as
     determined above does not exist, the search will be attempted on
     '/usr/lib/help/default' with the search key being the entire argument.
     In no case will more than one search per argument be performed.

     File is formatted as follows:

		* comment
		* comment
		-str1
		text
		-str2
		text
		* comment
		text
		-str3
		text

	The "str?" that matches the key is found and
	the following text lines are printed.
	Comments are ignored.

	If the argument is omitted, the program requests it.
*/
#define HELPLOC "/usr/share/lib/sccshelp/helploc"
struct stat Statbuf;
char Error[128];

char	dftfile[]   =   "/usr/share/lib/sccshelp/default";
char	helpdir[]   =   "/usr/share/lib/sccshelp/";
char	hfile[64];
char	*repl();
FILE	*iop, *fdfopen();
char	line [512];


#define MSGSTART 1
#define SETSTART 49
#define MSGDEFAULT "default"
static char *longnames[10][2] ={ { "prs_kywds", "prskwd" },
				{ "sccsdiff", "sccsdf" },
				{ "ad", "admin" },
				{ "bd", "bdiff" },
				{ "cb", "comb" },
				{ "de", "delta" },
				{ "he", "" },
				{ "ge", "get" },
				{ "rc", "rmdel" },
				{ "sccshelp", "help" }
			      };
#define NNAMES 10

nl_catd catd;


main(argc,argv)
int argc;
char *argv[];
{
	register int i;
	extern int Fcnt;
	char *ask();

	catd = catopen(MF_SCCS, NL_CAT_LOCALE);

	/*
	Tell 'fatal' to issue messages and return to its caller.
	*/
	Fflags = FTLMSG | FTLJMP;

	if (argc == 1)
		findprt(ask());		/* ask user for argument */
	else
		for (i = 1; i < argc; i++)
			findprt(argv[i]);

	exit(Fcnt ? 1 : 0);
}


findprt(p)
char *p;
{
	register char *q;
	char key[150];
	char *strcpy();

	int msgset;
	nl_catd msgd;
	int i;
	char *msg;

	if (setjmp(Fjmp))		/* set up to return here from */
		return;			/* 'fatal' and return to 'main' */
	if (size(p) > 50)
		fatal(MSGSTR(ARGTOOLNG, "\nCommand parameter cannot exceed 50 characters. (he2)\n"));

	strcpy(hfile, "h.");
	q = p;

	while (*q && !numeric(*q))
		q++;

	if (*q == '\0') {			/* no number; only name */
		for ( i = 0; i < NNAMES; i++) {
			if (!strcmp(p,longnames[i][0])) {
				strcat(hfile,longnames[i][1]);
				break;
			}
		}
		if (i == NNAMES)
			strcat(hfile,p);
		msgset = SETSTART;
	}
	else
		if (q == p) {
			sscanf(q,"%d",&msgset);	/* get the set # */
			if (!msgset) msgset = 49;
			strcat(hfile, MSGDEFAULT);
		}
		else {
			strcat(hfile,p);
			*(hfile + (q-p) + 2) = '\0';
			sscanf(q,"%d",&msgset);
			if (!msgset) msgset = 49;
		}
	strcat(hfile,".cat");
	if ((int)(msgd = catopen(hfile,NL_CAT_LOCALE)) != -1) {
		for (i=MSGSTART; ;i++) {
			msg = catgets(msgd,msgset,i,"");
			if (i == MSGSTART && strcmp(msg, "") == 0)
				goto go_on;	/* try normal mechanism */
			else if (i == MSGSTART)
				printf("\n%s:\n",p);
			else if (strcmp(msg, "") == 0)
				break;
			printf("%s\n",msg);
		}
		catclose(msgd);
		return;
	}
go_on:

	q = p;

	while (*q && !numeric(*q))
		q++;

	if (*q == '\0') {		/* all alphabetics */
		strcpy(key,p);
		sprintf(hfile,"%s%s",helpdir,"cmds");
		if (!exists(hfile))
			strcpy(hfile,dftfile);
	}
	else
		if (q == p) {		/* first char numeric */
			strcpy(key,p);
			strcpy(hfile,dftfile);
		}
	else {				/* first char alpha, then numeric */
		strcpy(key,p);		/* key used as temporary */
		*(key + (q - p)) = '\0';
		if(!lochelp(key,hfile))
			sprintf(hfile,"%s%s",helpdir,key);
		else
			cat(hfile,hfile,"/",key,0);
		tr(hfile,helpdir,key,NULL);
		strcpy(key,q);
		if (!exists(hfile)) {
			strcpy(key,p);
			strcpy(hfile,dftfile);
		}
	}

	iop = (FILE *) xfopen(hfile,0);
	Fflags |= FTLCLN;    /* now add cleanup to fatal's instructions */

	/*
	Now read file, looking for key.
	*/
	while ((q = fgets(line,512,iop)) != NULL) {
		repl(line,'\n','\0');		/* replace newline char */
		if (line[0] == '-' && equal(&line[1],key))
			break;
	}

	if (q == NULL) {	/* endfile? */
		printf("\n");
		sprintf(Error,MSGSTR(NOTFOUND, "\n%s is not a valid parameter.\n\
\tSpecify a valid command or error code. (he1)\n"),p);
		fatal(Error);
	}

	printf("\n%s:\n",p);

	while (fgets(line,512,iop) != NULL && line[0] == '-')
		;
	do {
		if (line[0] != '*')
			printf("%s",line);
	} while (fgets(line,512,iop) != NULL && line[0] != '-');

	fclose(iop);
}


char *
ask()
{
	static char resp[51];

	iop = stdin;

	printf(MSGSTR(MSGNOCMDNM, "Provide message number or command name.\n"));
	fgets(resp,51,iop);
	return(repl(resp,'\n','\0'));
}

/* lochelp finds the file which cojntains the help messages 
if none found returns 0
*/
lochelp(ky,fi)
	char *ky,*fi; /*ky is key  fi is found file name */
{
	FILE *fp;
	char locfile[513];
	char *hold;
	extern char *strtok();

	if(!(fp = fopen(HELPLOC,"r")))
	{
		/*no lochelp file*/
		return(0); 
	}
	while(fgets(locfile,512,fp)!=NULL)
	{
		hold=strtok(locfile,"\t ");
		if(!(strcmp(ky,hold)))
		{
			hold=strtok((char *)0,"\n");
			strcpy(fi,hold); /* copy file name to fi */
			return(1); /* entry found */
		}
	}
	return(0); /* no entry found */
}


clean_up()
{
	fclose(iop);
}
