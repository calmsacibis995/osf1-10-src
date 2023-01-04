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
static char	*sccsid = "@(#)$RCSfile: grpck.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/10/08 16:21:10 $";
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
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 * grpck: group file checker
 */

#include <stdio.h>
#include <pwd.h>
#include <limits.h>
#include <sys/types.h>

#ifdef NLS
#include <locale.h>
#include <NLchar.h>
#include <NLctype.h>
#else
#include <ctype.h>
#endif NLS

#ifdef MSG
#include "grpck_msg.h" 
nl_catd  catd;   /* Cat descriptor for scmc conversion */
#define MSGSTR(num,str) NLcatgets(catd,MS_grpck,num,str)  /*MSG*/
#else
#define MSGSTR(num,str) str
#endif

#define ERROR1 "Too many/few fields"
#define ERROR2a "No group name"
#define ERROR2b "Bad character(s) in group name"
#define ERROR3  "Invalid GID"
#define ERROR4a "Null login name"
#define ERROR4b "Logname not found in password file"

#define GROUP_FILE	"/etc/group"
int eflag, badchar, baddigit,badlognam,colons,len,i;
char tmpbuf[PATH_MAX];
char buf[PATH_MAX]; 

struct passwd *getpwnam();
char *strchr();
char *nptr;
char *cptr;
FILE *fptr;
int delim[PATH_MAX];
gid_t gid;
int error();

/*
 * NAME: grpck [file]
 *                                                                    
 * FUNCTION: scan the name file or /etc/group and writes to standard out
 *     any inconsistencies.
 */  
main (argc,argv)
int argc;
char *argv[];
{

#ifdef NLS
  (void ) setlocale (LC_ALL,"");
#endif

#ifdef  MSG
	catd = NLcatopen(MF_GRPCK, NL_CAT_LOCALE);
#endif

  if ( argc == 1)
    argv[1] = GROUP_FILE;
  else if ( argc != 2 )
       {
	 fprintf (stderr, MSGSTR(M_MSG_0, "\nusage: %s filename\n\n") ,*argv);
	 exit(1);
       }


  if ( ( fptr = fopen (argv[1],"r" ) ) == NULL )
  { 
	fprintf (stderr, MSGSTR(M_MSG_1, "\ncannot open file %s\n\n") ,argv[1]);
	exit(1);
  }

  while(fgets(buf,PATH_MAX,fptr) != NULL )
  {
	if ( buf[0] == '\n' )    /* blank lines are ignored */
          continue;

	for (i=0; buf[i]!=NULL; i++)
	{
	  tmpbuf[i]=buf[i];          /* tmpbuf is a work area */
	  if (tmpbuf[i] == '\n')     /* newline changed to comma */  
	    tmpbuf[i] = ',';
	}

	for (i; i < PATH_MAX; ++i)     /* blanks out rest of tmpbuf */ 
	{
	  tmpbuf[i] = NULL;
	}
	colons=0;
	eflag=0;
	badchar=0;
	baddigit=0;
	badlognam=0;
	gid=0l;

    /*	Check number of fields	*/

	for (i=0 ; buf[i] != NULL ; i++)
	{
	  if (buf[i]==':')
          {
            delim[colons]=i;
            ++colons;
          }
	}
	if (colons != 3 )
	{
	  error(MSGSTR(M_MSG_2,ERROR1));
	  continue;
	}

    /*	check to see that group name is at least 1 character	*/
    /*		and that all characters are printable.		*/
 
	if ( buf[0] == ':' )
	  error(MSGSTR(M_MSG_3,ERROR2a));
	else
	{
		NLchar nlbuf[PATH_MAX], *nlp, nlcolon;
		int k;

		k = NCdecstr( buf, nlbuf, PATH_MAX);
		k = NCdecode(":", &nlcolon);
		 for ( nlp=nlbuf; *nlp != nlcolon ; nlp++ )
	  	{
		if ( !(NCisprint(*nlp)) ) { 
			badchar++;
		}
	  	}
	  if ( badchar > 0 )
	    error(MSGSTR(M_MSG_4,ERROR2b));
	}

    /*	check that GID is numeric and <= 65535	*/

	len = ( delim[2] - delim[1] ) - 1;

	if ( len > 5 || len == 0 )
	  error(MSGSTR(M_MSG_5,ERROR3));
	else
	{
	  for ( i=(delim[1]+1); i < delim[2]; i++ )
	  {
	    if ( ! (isdigit(buf[i])))
	      baddigit++;
	    else if ( baddigit == 0 )
		gid=gid * 10 + (buf[i]) - '0';    /* converts ascii */
                                                  /* GID to decimal */
	  }
	  if ( baddigit > 0 )
	    error(MSGSTR(M_MSG_5,ERROR3));
	  else if ( gid > UID_MAX || gid < 0l )
	      error(MSGSTR(M_MSG_5,ERROR3));
	}

     /*  check that logname appears in the passwd file  */

	nptr = &tmpbuf[delim[2]];
	nptr++;
	while ( ( cptr = strchr(nptr,',') ) != NULL )
	{
	  *cptr=NULL;
	  if ( *nptr == NULL )
	  {
	    error(MSGSTR(M_MSG_6,ERROR4a));
	    nptr++;
	    continue;
	  }
	  if (  getpwnam(nptr) == NULL )
	  {
	    badlognam=1;
	    error(MSGSTR(M_MSG_7,ERROR4b));
	  }
	  nptr = ++cptr;
	  setpwent();
	}
	
  }
}

/*
 * NAME: error
 * FUNCTION:  Error printing routine
 */
error(msg)
char *msg;
{
	if ( eflag==0 )
	{
	  fprintf(stdout,"\n\n%s",buf);
	  eflag=1;
	}

	if ( badchar != 0 )
	{
	  fprintf (stdout,"\t%d %s\n",badchar,msg);
	  badchar=0;
	  return;
	}
	else if ( baddigit != 0 )
	     {
		fprintf (stdout,"\t%s\n",msg);
		baddigit=0;
		return;
	     }
	     else if ( badlognam != 0 )
		  {
		     fprintf (stdout,"\t%s - %s\n",nptr,msg);
		     badlognam=0;
		     return;
		  }
		  else
		  {
		    fprintf (stdout,"\t%s\n",msg);
		    return;
		  }
}
