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
static char	*sccsid = "@(#)$RCSfile: lp.c,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1993/10/08 15:14:17 $";
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
 * lp - send requests to the line printer spooler
 *
 * This interfaces just to lpr, the "official" spooling program.
 * Things to consider: 
 *    lpr copies files by default, lp links them by default
 *    lpr looks at the PRINTER environment variable, lp looks at LPDEST
 *    lpr requires options AND THEN files, lp allows them to be mixed
 */


#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include "printer_msg.h"

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_PRINTER,n,s)

#define LPR "lpr"
#define LPRPATH "/usr/bin/lpr"

int	newArgc;		/* the new arguments to LPR */
char	**newArgv;

void getOptions(int, char **);
void usage(void);
void fatal(char *);
char *Sdup(char *, char *);

main(int argc, char **argv)
{
int i;
   (void) setlocale( LC_ALL, "" );
   catd = catopen(MF_PRINTER,NL_CAT_LOCALE);

   newArgv = (char **)calloc(argc+4, sizeof(char *));	/* the '4' is -j, -s, -P, NULL */

   if (newArgv == NULL)
      fatal(MSGSTR(LP_1, "Out of memory"));

   newArgv[newArgc++] = LPR;	/* establish argv[0] for lpr */

   getOptions(argc, argv);	/* sort out the options for lpr */

   argc -= optind;		/* advance past options */
   argv += optind;

   while (argc--)		/* append filenames to command line */
      newArgv[newArgc++] = *argv++;

   newArgv[newArgc] = NULL;
   execv(LPRPATH, newArgv);
}

void getOptions(int argc, char **argv)
{
   int copyFlag = 0;
   int destFlag = 0;
   int jobFlag = 1;
   register int ch;
   register char *num;

   while ((ch = getopt(argc, argv, "cd:mn:o:st:w")) != EOF)
   {
      switch (ch)
      {
         case 'c': copyFlag = 1;	/* make a copy, nothing to do */
                   break;

         case 'd': newArgv[newArgc++] = Sdup("-P", optarg);	/* destination printer specified */
                   destFlag = 1;
                   break;

         case 'n': num = optarg;				/* make a # of copies */
                   while(*num)
                   {
                      if (isdigit(*num))
                         ++num;
                      else
                      {
                         fprintf(stderr, MSGSTR(LP_4,
                                "The -n option requires a numeric argument.\n"));
                         usage();
                      } 
                   }

                   newArgv[newArgc++] = Sdup("-#", optarg);
                   break;

         case 'o': break;		/* not supported */

         case 's': jobFlag = 0;		/* suppress job-number on stdout */
		   break;

         case 't': newArgv[newArgc++] = Sdup("-J", optarg);	/* title for 1st page */
                   break;

         case 'm':			/* send mail */
         case 'w': newArgv[newArgc++] = "-m";
                   break;

         default : usage();		/* invalid option */
         }
   }   

   if (!copyFlag)			/* don't copy, so link */
      newArgv[newArgc++] = "-s";

   if (jobFlag)				/* make lpr print the job-number */
       newArgv[newArgc++] = "-j";

   if (!destFlag)			/* so look for LPDEST in env */
   {
      char *dest;

      if ((dest = getenv("LPDEST")) != NULL && strlen(dest))
         newArgv[newArgc++] = Sdup("-P", dest);
   }
}

void usage(void)
{
   fprintf(stderr, MSGSTR(LP_2,
	  "Usage: lp [-cmsw] [-d printer] [-n copies] [-t title] [file...]\n"));
   exit(1);
}

void fatal(char *msg)
{
   fprintf(stderr, MSGSTR(LP_3, "Fatal error: %s\n"));
   exit(1);
}

char *Sdup(char *s1, char *s2)
{
   char *newstr;

   newstr = (char *)malloc(strlen(s1) + strlen(s2) + 1);
   if (newstr == NULL)
      fatal(MSGSTR(LP_1, "Out of memory"));
   strcpy(newstr, s1);
   strcat(newstr, s2);
   return(newstr);
}
