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
static char rcsid[] = "@(#)$RCSfile: paste.c,v $ $Revision: 4.2.5.5 $ (DEC) $Date: 1993/10/11 17:42:07 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.12  com/cmd/files/paste.c, cmdfiles, bos320, 9130320 7/2/91 13:28:27"
 */

#include <sys/limits.h>
#include <unistd.h>
#include <wchar.h>
#include "paste_msg.h"
#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#define MSGSTR(id,ds) catgets(catd, MS_PASTE, id, ds)
static nl_catd catd;

#define MAXOPNF 12      /* maximal no. of open files (not with -s option) */
#define RUB  '\177'
#define SPECIAL -1
wchar_t del[LINE_MAX] =  {L'\t'};

int mbcm;
  
void 	diag(char *, int);
int 	move(char *, wchar_t *);

/*
 * NAME: paste file1 file2 ...
 *       paste -dList file1 file2 ...
 *       paste -s [-dList] file1 ...
 *                                                                    
 * FUNCTION: Merges the lines of several files or subsequent lines in 
 *           one file.
 *                                                                    
 * NOTES:  Reads standard input if - is given as a file.
 *         Default is to treat each file as a column and join them
 *         horizontally.
 *         -d List   List of delimiters to separate fields
 *         -s        Merges subsequent lines from the first file
 *                   horizontally
 *
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.
 *
 * RETURN VALUE DESCRIPTION: What this code returns (NONE, if it has no 
 *                           return value)
 */  

/* Note: This code is compliant with POSIX 1003.2 Draft 11. */

main(int argc, char **argv)
{
        int i, k, eofcount, nfiles, glue;
        int delcount = 1;
        int onefile  = 0;
        wint_t wc ;
        wchar_t l;
        wchar_t *p;
        FILE *inptr[MAXOPNF];
        int arg;
  
        (void) setlocale(LC_ALL,"");
        catd = catopen(MF_PASTE, NL_CAT_LOCALE);
        mbcm = MB_CUR_MAX;

        while ((arg = getopt(argc,argv,"sd:")) != SPECIAL)
        {
          switch (arg)
          {
            case 's' : onefile++;
                         break;
            case 'd' : delcount = move(optarg, &del[0]);
			if (delcount == 0) {	/* POSIX.2 D11 - 0 is allowed */
				delcount = 1;
				del[0] = RUB;	/* empty string */
			}
                       break;
             default : diag(MSGSTR(M_USAGE,
				"Usage: paste [-s] [-d List ] File1 ...\n"), 1);
                       break;
          }
        }

        argv += optind;
        argc -= optind;

	if (argc <= 0)
		diag(MSGSTR(M_USAGE,
				"Usage: paste [-s] [-d List ] File1 ...\n"), 1);

 
        if ( ! onefile)
        {       /* not -s option: parallel line merging */
          for (i = 0; i<argc &&  i<MAXOPNF; i++)
          {
            if (argv[i][0] == '-')
              inptr[i] = stdin;
            else
              inptr[i] = fopen(argv[i], "r");
            if (inptr[i] == NULL)
            {
              diag(argv[i], 0);
              diag(MSGSTR(M_NOPEN, " : cannot open\n"), 1);
            }
          }
          if (argc > MAXOPNF)
            diag(MSGSTR(M_2MANY, "too many files\n"),1);
         
                nfiles = i;
  
                 do
                 {
			int x = 0;
			wchar_t savedels[MAXOPNF] = {'\0'};

                 	eofcount = 0;
                        k = 0;
                        for (i = 0; i < nfiles; i++)
                        {
                          while((wc = getwc(inptr[i])), wc!=L'\n' && wc!=WEOF)
			  {
			     if (x >0)	/* print delimeters */
			     {
			       int z = 0;
			       do {
			    	   putwchar(savedels[z]);
				   savedels[z] = '\0';
			       } while (++z < x);
			       x = 0;
			     }
                             putwchar(wc);
			  }
                          if(wc == WEOF)
                            eofcount++;
			  /*
			   * no separator if
			   *  - del[k] = RUB
			   *  - last file in list
			   */
                          if ((l = del[k]) != RUB && i != (nfiles-1))
                            savedels[x++] = l;
                          k = (k + 1) % delcount;
                        }

			if (eofcount < nfiles) {
				putwchar(L'\n');
			}
                } 
                while (eofcount < nfiles);
        }
        else
        {        /* -s option: serial file pasting (old 127 paste command) */
          for (i = 0; i < argc && i < MAXOPNF; i++)
          {
            if (argv[i][0] == '-' && argv[i][1]==NULL)
              inptr[0] = stdin;
            else
              inptr[0] = fopen(argv[i], "r");
            if (inptr[0] == NULL)
            {
              diag(argv[i], 0);
              diag(MSGSTR(M_NOPEN, " : cannot open\n"), 1);
            }

            glue =  k = 0; /* POSIX says delimiters reset after each file */
            while((wc = getwc(inptr[0])) != WEOF)
            {
		if (glue) {	/* last char was newline, and not eof */
			putwchar(l);	/* put delimiter */
			glue = 0;
		}
		if (wc != '\n')
			putwchar(wc);
		else
		{
			l = del[k];
			if (l != RUB)
				glue = 1;
			k = (k + 1) % delcount;
		}
            }
	    putwchar(L'\n');	/* final newline */
          }
        }
        exit(0);        /* all ok */
}

/*
 * NAME: diag
 *                                                                    
 * FUNCTION: Display error message.
 */  
void
diag(char *s, int r)
{
  static first_time = 1;

  if (first_time)
  {
    fputs("paste: ", stderr);
    first_time = 0;
  }
  fputs(s, stderr);
  if(r != 0)
    exit(r);
}
  
/*
 * NAME: move
 *                                                                    
 * FUNCTION: move one string to another checking for special characters
 *           along the way.
 *
 * RETURN: return the length of the copied string.
 */  
int
move(char *from, wchar_t *to)
{
wchar_t wc;
int     i, len;

        i = 0;
        do
        {
          len = mbtowc(&wc,from,mbcm);
          from += len;
          i++;
          if (wc != '\\')
            *to++ = wc;
          else
          {
            len = mbtowc(&wc,from,mbcm);
            from += len;
            switch (wc)
            {
              case '0' : *to++ = RUB;
                         break;
              case 't' : *to++ = '\t';
                         break;
              case 'n' : *to++ = '\n';
                         break;
              default  : *to++ = wc;
                         break;
            }
          }
        } while (wc) ;
return(--i);
}
