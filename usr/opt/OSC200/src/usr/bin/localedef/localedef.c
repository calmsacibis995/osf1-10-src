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
static char     *sccsid = "@(#)$RCSfile: localedef.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/10/29 19:08:52 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1.1
 */
/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.8  com/cmd/nls/localedef.c, cmdnls, bos320, 9130320 7/17/91 17:39:41
 */

#include <sys/localedef.h>
#include <sys/limits.h>
#include <sys/method.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>	
#include <stdlib.h>	
#include <string.h>
#include <unistd.h>
#include <nl_types.h>
#include <sys/wait.h>
#include <errno.h>
#include "symtab.h"
#include "err.h"
#include "pathnames.h"
#include "locdef.h"

char	*tpath;
char	*ccopts;
char	*ldopts;

extern char *lib_array[];	/* Array of libraries required for extensible methods */

char *yyfilenm;			/* global file name pointer */

int warn = FALSE;		/* Suppress warnings flag */

const char *options =	{
#ifdef DEBUG
			"scdvwm:f:i:C:L:P:"
#else /* DEBUG */
			"cvwm:f:i:C:L:P:"
#endif /* DEBUG */
			};

extern void initlex(void);
extern void initparse(void);

void initparse(void) {
    initlex();
    initgram();
}

void
main(int argc, char *argv[])
{
  extern int yylineno, yycharno;
  extern int err_flag;
#ifdef DEBUG
  extern symtab_t cm_symtab;
#endif /* DEBUG */
  extern int yydebug;
  extern int optind;
  extern char *optarg;

  int      c;
  int      verbose;
  int	   force;
  int      sym_dump;
  int      fd;
  FILE     *fp;
  char     *cmdstr;
  char     *locname;
  char     *methsrc;
  char     *cmapsrc;
  char     *locsrc;
  char     *tmpfilenm;
  char	   *s;
  char	   *base;

  size_t	cmdlen;
  int		i;

  yydebug = verbose = sym_dump = force = FALSE;
  ldopts = ccopts = tpath = cmapsrc = locsrc = methsrc  = NULL;

  while ((c=getopt(argc, argv, (char *)options)) != -1) 
    switch (c) {
#ifdef DEBUG
    case 's':
      sym_dump = TRUE;
      break;
    case 'd':			/* parser debug */
      yydebug = TRUE;
      break;
#endif /* DEBUG */

    case 'w':			/* display duplicate definition warnings */
      warn = TRUE;
      break;

    case 'c':			/* generate a locale even if warnings */
      force = TRUE;
      break;

    case 'v':			/* verbose table dump */
      verbose = TRUE;
      break;

    case 'm':			/* specify method file name */
      methsrc = optarg;
      break;

    case 'f':			/* specify charmap file name */
      cmapsrc = optarg;
      break;

    case 'i':			/* specify locale source file */
      locsrc = optarg;
      break;
	
    case 'P':			/* tool path */
      tpath = optarg;
      break;

    case 'C':			/* special compiler options */
      ccopts = optarg;
      break;

    case 'L':			/* special linker options */
      ldopts = optarg;
      break;

    default:			/* Bad option or invalid flags */
      usage(4);			/* Never returns! */
    }

  if (optind < argc) {
      /*
       * Create the locale name
       */
      locname = MALLOC(char, strlen(argv[optind])+1);
      strcpy(locname, argv[optind]);
  } else {
      usage(4);			/* NEVER returns */
  }

  /* if there is a method source file, process it */

  if (methsrc != NULL) {
      int fd0;			     	/* file descriptor to save stdin */
      fd0 = dup(0);                    	/* dup current stdin */
      close(0);				/* close stdin, i.e. fd 0 */
      fd = open(methsrc, O_RDONLY);
      if (fd < 0)
	  error(ERR_OPEN_READ, methsrc);
      
      initparse();
      yyfilenm = methsrc;	     /* set filename begin parsed for */	
				     /* error reporting.  */
      yyparse();		     /* parse the methods file */

      close(fd);

      /* restore stdin */
      close(0);			     /* close methods file */
      dup(fd0);			     /* restore saved descriptor to 0 */
  }

  /*
   * seed symbol table with default values for mb_cur_max, mb_cur_min, 
   * and codeset 
   */
  if (cmapsrc != NULL)
    init_symbol_tbl(FALSE);	/* don't seed the symbol table */
  else
    init_symbol_tbl(TRUE);	/* seed the symbol table with POSIX PCS */

  /* process charmap if present */
  if (cmapsrc != NULL) {
    int fd0;			     /* file descriptor to save stdin */

    fd0 = dup(0);                    /* dup current stdin */
    close(0);			     /* close stdin, i.e. fd 0 */
    fd = open(cmapsrc, O_RDONLY);    /* new open gets fd 0 */
    if (fd != 0)
      error(ERR_OPEN_READ, cmapsrc);

    initparse();
    yyfilenm = cmapsrc;		     /* set filename begin parsed for */
				     /* error reporting.  */
    yyparse();			     /* parse charmap file */

    /* restore stdin */
    close(0);			     /* close charmap file */
    dup(fd0);			     /* restore saved descriptor to 0 */
  } else
      define_all_wchars();	     /* Act like all code points legal */


  /* process locale source file.  if locsrc specified use it,
  ** otherwise process input from standard input 
  */
  {
      int fd0 = 0;
    
      if (locsrc != NULL) {
	  fd0 = dup(0);
	  close(0);
	  fd = open(locsrc, O_RDONLY);
	  if (fd != 0)
	    error(ERR_OPEN_READ, locsrc);

	  yyfilenm = locsrc;			/* set file name being parsed for */
						/* error reporting. */
      } else {
	  yyfilenm = "stdin";
	  fd = 0;
      }

      initparse();
      yyparse();

      if( fd0 != 0 ) {
	  close(0);
	  dup(fd0);
      }
  }

#ifdef DEBUG
  if (sym_dump) {
    /* dump symbol table statistics */
    int      i, j;
    symbol_t *p;

    for (i=0; i < HASH_TBL_SIZE; i++) {
      j=0;
      for (p= &(cm_symtab.symbols[i]); p->next != NULL; p=p->next)
	j = j + 1;
      printf("bucket #%d - %d\n", i, j);
    }
  }
#endif /* DEBUG */

  if (!force && err_flag)	/* Errors or Warnings without -c present */
    exit(4);

  /* Open temporary file for locale source.  */
  s = tempnam("./", "locale");
  tmpfilenm = MALLOC(char, strlen(s)+3);	/* Space for ".[co]\0" */
  strcpy(tmpfilenm, s);
  strcat(tmpfilenm,".c");
  fp = fopen(tmpfilenm, "w");
  if (fp == NULL) {
    error(ERR_WRT_PERM, tmpfilenm);
  }

  /* generate the C code which implements the locale */
  gen(fp);
  fclose(fp);

  /* check and initialize if necessary linker/compiler opts and tool paths. */
  if (ldopts==NULL)
      ldopts = "";
  if (tpath == NULL)
      tpath  = "";
  if (ccopts==NULL)
      ccopts = "";

  /* compile the C file created */

  cmdlen = sizeof(CCPATH) + strlen(tpath) + sizeof(CCFLAGS) +
    	   strlen(tmpfilenm) + strlen(ccopts) + 10;

  cmdstr = malloc(cmdlen+1);		/* Space for trailing NUL */

  s = cmdstr + sprintf(cmdstr, "%s" CCPATH  CCFLAGS " %s %s", tpath, ccopts, tmpfilenm);

  if (verbose) printf("%s\n",cmdstr);
  c = system(cmdstr);
  free(cmdstr);

  /* delete the C file after compiling */
  if (!verbose) 
    unlink(tmpfilenm);
  else {
    /* rename to localename.c */
    char *s;

    s = MALLOC(char, strlen(locname)+3);
    strcpy(s, locname);
    strcat(s, ".c");
    rename(tmpfilenm, s);
    free(s);
  }

  if (WIFEXITED(c))
    switch (WEXITSTATUS(c)) {
      case 0:	break;			/* Successful compilation */

      case -1:	perror("localedef");	/* system() problems? */
      		exit(4);

      case 127:	error(ERR_NOSHELL);	/* cannot exec /usr/bin/sh */
      default:	error(ERR_BAD_CHDR);	/* take a guess.. */
    }
  else
    error(ERR_INTERNAL, tmpfilenm, 0);
	    
  /*
   * Since we might have TMPDIR pointing to somewhere else, the .o file
   * might not be where the .c file was.
   */
  tmpfilenm[strlen(tmpfilenm)-1] = 'o';
  base = basename(tmpfilenm);
  s = MALLOC(char, strlen(base)+3);	/* Space for './\0' (3 chars) */
  strcpy(s, "./");
  strcat(s, base);
  free(tmpfilenm);
  tmpfilenm = s;

  /* 
    re-link the created object, specifying the entry point as
    instantiate 
  */
  /* if there are other "libraries to compile in with the locale for user
     defined methods, the list will be in lib_array */

  s = strrchr(locname, '/');	/* Get LOCALE name unadorned by path */
  if (s)
    s++;			/* There was a path component */
  else
    s = locname;		/* Use whole name */

  cmdlen = sizeof(LDPATH) + strlen(tpath) + strlen(tmpfilenm)
    		+ sizeof(LDFLAGS_FMT) + strlen(ldopts) + strlen(s) +
		  strlen(locname) + sizeof(LDLIBRARY) + 20;

  for (i=0; i<= LAST_METHOD; i++) {
      if ( !lib_array[i] ) break;	/* No more libraries */
      cmdlen += strlen(lib_array[i]) + 1;
  }

  cmdstr = malloc(cmdlen+1);

  s = cmdstr + sprintf(cmdstr, "%s" LDPATH " " LDFLAGS_FMT " %s",
	  tpath, s, ldopts, locname, tmpfilenm);

  for (i=0; i<=LAST_METHOD; i++) {
      if ( !lib_array[i] ) break;
      s = s + sprintf(s, " %s", lib_array[i]);
  }

  s = s + sprintf(s, " %s", LDLIBRARY);

  if (verbose) printf("%s\n",cmdstr);
  c = system(cmdstr);
  free(cmdstr);

  if(!verbose)
    /* unlink the original object */
    unlink(tmpfilenm);
  else {
      char *s;
      s = MALLOC(char, strlen(locname)+3);
      strcpy(s,locname);
      strcat(s, ".o");
      rename(tmpfilenm, s);
      free(s);
  }

  if (WIFEXITED(c))
    switch (WEXITSTATUS(c)) {
      case 0:	break;			/* Successful compilation */

      case -1:	perror("localedef");	/* system() problems? */
      		exit(4);

      case 127:	error(ERR_NOSHELL);	/* cannot exec /usr/bin/sh */

      default:
#ifdef DEBUG
		printf("Unexpected return (%d)\n",WEXITSTATUS(c));
#endif /* DEBUG */
      		error(ERR_WRT_PERM, locname);	/* take a guess.. */
    }
  else
    error(ERR_INTERNAL, tmpfilenm, 0);

  exit( err_flag != 0 );	/* 1=>created with warnings */
				/* 0=>no problems */
}

