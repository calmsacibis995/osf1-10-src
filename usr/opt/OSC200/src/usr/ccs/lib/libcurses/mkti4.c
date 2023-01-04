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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: mkti4.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 21:58:30 $";
#endif
/*
 * HISTORY
 */
/*** "mkti4.c  1.7  com/lib/curses,3.1,9008 11/12/89 20:30:23"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   mkti4
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<stdio.h>


#define		TRUE		1
#define		FALSE		0
#define		FAILURE		(-1)

#define		CAPS		"caps"

char *part1[] =
  {
	"/*\n",
	    " * ti4 [term]\n",
	    " * dummy program to test termlib.\n",
	    " * gets entry, counts it, and prints it.\n",
	    " */\n",
	    "#include <stdio.h>\n",
	    "#include \"curses.h\"\n",
	    "#include \"term.h\"\n",
	    "\n",
	    "#define prb(name)\tif (name) printf(\"name\\n\")\n",
	    "#define prn(name)\tif (name != -1) printf(\"name = %d\\n\", \
name)\n",
	    "#define prs(name)\tif (name) {printf(\"name = '\"); pr(name); \
printf(\"'\\n\");}\n",
	    "\n",
	    "char buf[1024];\n",
	    "char *getenv();\n",
	    "\n",
	    "main(argc, argv) char **argv; {\n",
	    "\tchar *p;\n",
	    "\tint rc;\n",
	    "\n",
	    "\tif (argc < 2)\n",
	    "\t\tp = getenv(\"TERM\");\n",
	    "\telse\n",
	    "\t\tp = argv[1];\n",
	    "\tprintf(\"Terminal type %s\\n\", p);\n",
	    "\tsetupterm(p,1,0);\n",
	    "\n",
	    "\tprintf(\"flags\\n\");\n",
	    "\n",0
  } ;
char *part2[] =
  {
	"\n",
	    "\tprintf(\"\\nnumbers\\n\");\n",
	    "\n",0
  } ;
char *part3[] =
  {
	"\n",
	    "\tprintf(\"\\nstrings\\n\");\n",
	    "\n",0
  } ;
char *part4[] =
  {
	"\n",
	    "\tprintf(\"end of strings\\n\");\n",
	    "\treset_shell_mode();\n",
	    "\texit(0);\n",
	    "}\n",
	    "\n",
	    "pr(p)\n",
	    "register char *p;\n",
	    "{\n",
	    "\tchar *rdchar();\n",
	    "\n",
	    "\tfor (; *p; p++)\n",
	    "\t\tprintf(\"%s\", rdchar(*p));\n",
	    "}\n",
	    "\n",
	    "/*\n",
	    " * rdchar: returns a readable representation of an ASCII char, \
using ^ notation.\n",
	    " */\n",
	    "#include <ctype.h>\n",
	    "char *rdchar(c)\n",
	    "char c;\n",
	    "{\n",
	    "\tstatic char ret[4];\n",
	    "\tregister char *p;\n",
	    "\n",
	    "\t/*\n",
	    "\t * Due to an error in isprint, this prints spaces as ^`, but \
this is OK\n",
	    "\t * because we want something to show up on the screen.\n",
	    "\t */\n",
	    "\tret[0] = ((c&0377) > 0177) ? '\\'' : ' ';\n",
	    "\tc &= 0177;\n",
	    "\tret[1] = isprint(c) ? ' ' : '^';\n",
	    "\tret[2] = isprint(c) ?  c  : c^0100;\n",
	    "\tret[3] = 0;\n",
	    "\tfor (p=ret; *p==' '; p++)\n",
	    "\t\t;\n",
	    "\treturn (p);\n",
	    "}\n",
	    0
  } ;

/*
 * NAME:        mkti
 *
 * EXECUTION ENVIRONMENT:
 *
 *      "mkti4" produces a C file, ti4.c, which contains the
 *      proper string definitions found in "caps", the source of
 *      curses structure element definitions.
 */

main(argc,argv)

int argc ;
char **argv ;

  {
	register char **section ;
	register char *ptr ;
	char line[256],word[120] ;
	FILE *fp ;

	if (--argc <= 0) ptr = CAPS ;
	else ptr = *++argv ;

	if ((fp = fopen(ptr,"r")) == (FILE*)NULL)
	  {
	    perror(ptr) ;
	    exit(1) ;
	  }

/*	Print out the first section of the program.			*/

	for (section= &part1[0]; *section ;) fputs(*section++,stdout) ;

/*	Look for "--- begin bool" in "caps" file.			*/

	while (ptr = fgets(&line[0],(int)(sizeof(line)),fp))
	  {
	    if (strncmp(ptr,"--- begin bool",sizeof("--- begin bool")-1)
		== 0) break ;
	  }
	if (ptr == (char*)NULL)
	  {
	    fprintf(stderr,"Unable to find \"--- begin bool\"\n") ;
	    exit(1) ;
	  }

	while (ptr = fgets(&line[0],(int)(sizeof(line)),fp))
	  {
	    if (*ptr == '#') continue ;
	    if (strncmp(ptr,"--- end bool",sizeof("--- end bool")-1)
		== 0) break ;

	    if (sscanf(ptr,"%s",&word[0]) == 1)
	      {
/*	Remove the comma after the word.				*/

		word[strlen(&word[0])-1] = '\0' ;
		fprintf(stdout,"\t    prb(%s) ;\n",&word[0]) ;
	      }
	  }
	if (ptr == (char*)NULL)
	  {
	    fprintf(stderr,"Premature termination during bools\n") ;
	    exit(1) ;
	  }

/*	Print out the second section of the program.			*/

	for (section= &part2[0]; *section ;) fputs(*section++,stdout) ;

/*	Look for "--- begin num" in "caps" file.			*/

	while (ptr = fgets(&line[0],(int)(sizeof(line)),fp))
	  {
	    if (*ptr == '#') continue ;
	    if (strncmp(ptr,"--- begin num",sizeof("--- begin num")-1)
		== 0) break ;
	  }
	if (ptr == (char*)NULL)
	  {
	    fprintf(stderr,"Unable to find \"--- begin num\"\n") ;
	    exit(1) ;
	  }

	while (ptr = fgets(&line[0],(int)(sizeof(line)),fp))
	  {
	    if (*ptr == '#') continue ;
	    if (strncmp(ptr,"--- end num",sizeof("--- end num")-1)
		== 0) break ;

	    if (sscanf(ptr,"%s",&word[0]) == 1)
	      {
/*	Remove the comma after the word.				*/

		word[strlen(&word[0])-1] = '\0' ;
		fprintf(stdout,"\t    prn(%s) ;\n",&word[0]) ;
	      }
	  }
	if (ptr == (char*)NULL)
	  {
	    fprintf(stderr,"Premature termination during nums\n") ;
	    exit(1) ;
	  }

/*	Print out the third section of the program.			*/

	for (section= &part3[0]; *section ;) fputs(*section++,stdout) ;

/*	Look for "--- begin str" in "caps" file.			*/

	while (ptr = fgets(&line[0],(int)(sizeof(line)),fp))
	  {
	    if (*ptr == '#') continue ;
	    if (strncmp(ptr,"--- begin str",sizeof("--- begin str")-1)
		== 0) break ;
	  }
	if (ptr == (char*)NULL)
	  {
	    fprintf(stderr,"Unable to find \"--- begin str\"\n") ;
	    exit(1) ;
	  }

	while (ptr = fgets(&line[0],(int)(sizeof(line)),fp))
	  {
	    if (*ptr == '#') continue ;
	    if (strncmp(ptr,"--- end str",sizeof("--- end str")-1)
		== 0) break ;

	    if (sscanf(ptr,"%s",&word[0]) == 1)
	      {
/*	Remove the comma after the word.				*/

		word[strlen(&word[0])-1] = '\0' ;
		fprintf(stdout,"\t    prs(%s) ;\n",&word[0]) ;
	      }
	  }
	if (ptr == (char*)NULL)
	  {
	    fprintf(stderr,"Premature termination during strs\n") ;
	    exit(1) ;
	  }

/*	Print out final section of program.				*/

	for (section= &part4[0]; *section ;) fputs(*section++,stdout) ;
	exit(0) ;
  }
