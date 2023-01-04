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
static char	*sccsid = "@(#)$RCSfile: cancel.c,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1993/10/08 15:14:00 $";
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
 * cancel - cancel requests made to the line printer spooler
 *
 * This interfaces just to lprm, the "official" program.
 */

#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include "printer_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_PRINTER,n,s)

#include <NLchar.h>
#include <NLctype.h>

#define LPRM "lprm"
#define LPRMPATH "/usr/bin/lprm"

int	newArgc;		/* the new arguments to LPR */
char	**newArgv;

main(argc, argv)
int argc;
char *argv;
{
  char *malloc();
  void getOptions(), fatal();

  (void) setlocale( LC_ALL, "" );
  catd = catopen(MF_PRINTER,NL_CAT_LOCALE);
  newArgv = (char **)calloc(argc+1, sizeof(char *));
  if (newArgv == NULL)
     fatal(MSGSTR(CANCEL_1, "out of memory"));
  newArgv[newArgc++] = LPRM;
  getOptions(argc, argv);

  newArgv[newArgc] = NULL;
  execv(LPRMPATH, newArgv);	/* Never should get beyond that call */
  fprintf(stderr, MSGSTR(CANCEL_2, "Can't exec %s\n"), LPRMPATH);
  return (1);			/* That is a exit(1) */
}

void getOptions(argc, argv)
int argc;
char *argv[];
{

   void fatal();
   void usage();

   if(argc <= 1)
      usage();

   while (--argc)
   {
      int noNumber = 0;
      char *thisArg = *++argv;
      if (thisArg[0] == '-')
         usage();
      while (*thisArg)
         if (!isdigit(*thisArg++))
         {
            noNumber = 1;
            break;
         }
      if (noNumber)		/* assume it was a printer name */
      {
        char *malloc(), *strcat();
        char *arg = malloc(strlen(*argv)+3);
        if (arg == NULL)
           fatal(MSGSTR(CANCEL_1, "out of memory"));
        arg[0] = '-';
        arg[1] = 'P';
        arg[2] = '\0';
        newArgv[newArgc++] = strcat(arg, *argv);
      }
      else
        newArgv[newArgc++] = *argv;
   }
}


void fatal(msg)
char *msg;
{
   fprintf(stderr, MSGSTR(CANCEL_3, "Fatal error: %s\n"));
   exit(1);
}

void usage()
{
   fprintf(stderr, MSGSTR(CANCEL_4, "Usage: cancel request_ID ... | printer ...\n"));
   exit(1);
}
