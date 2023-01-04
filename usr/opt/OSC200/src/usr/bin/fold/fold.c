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
static char rcsid[] = "@(#)$RCSfile: fold.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/10/11 17:07:45 $";
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
 * FUNCTIONS: fold 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.11  com/cmd/files/fold.c, cmdfiles, bos320, 9140320 9/25/91 16:47:23
 */

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include "fold_msg.h" 
#include <wchar.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * NAME:  fold [-bs] [-w Width] [File ...]
 * FUNCTION:  fold (wrap) long lines for limited-width output devices.
 *
 * Note: This code is compliant with POSIX 1003.2 Draft 11.
 *
 */

#define MSGSTR(num,str) catgets(catd,MS_FOLD,num,str)

#define STREQ(s1,s2) (!strcmp(s1,s2))
#define SPECIAL -1

static wctype_t blank_wctype;
#define isblank(c)   iswctype(c, blank_wctype)
#define iswblank(c)  iswctype(c, blank_wctype)

typedef void (*func_ptr)();           /* func_ptr: pointer to a function.     */

int      width = 80;             /* Width of output line (cols or bytes). */
int      col, bytes;             /* Columns, bytes.                       */
wint_t   wc;                     /* Current wide char (could be WEOF).    */
int      ch;                     /* Current char (could be EOF).          */
char     *cb;                    /* Char buffer pointer.                  */
wchar_t  buffer[LINE_MAX+1];     /* Static buffer for char (or wchar_t).  */
wchar_t  *wb;                    /* Wide char buffer pointer.             */
size_t   mbcs;                   /* Multibyte code size (max bytes/char). */
int      mbflag;                 /* Multibyte? Yes or no.                 */
int      longline;               /* Found input line > LINE_MAX chars?    */
nl_catd  catd;                   /* Message catalog descriptor.           */
int      bflag;                  /* -b option?                            */
int      sflag;                  /* -s option?                            */
int      rc;                     /* For holding various return codes.     */
int      err;                    /* Retain knowledge of prior error (for  */
                                 /*   exit under unusual circumstances).  */
int      syntax;                 /* Command-line syntax error?            */

void command_line(int argc, char *argv[]);
int  getwidth(char *s, int *i);  /* String-to-int with error checking.    */
void ps_none();                  /* Process single-byte, no options.      */
void ps_b();                     /* Process single-byte, -b option.       */
void ps_s();                     /* Process single-byte, -s option.       */
void ps_bs();                    /* Process single-byte, -b & -s options. */
void pm_none();                  /* Process multibyte, no options.        */
void pm_b();                     /* Process multibyte, -b option.         */
void pm_s();                     /* Process multibyte, -s option.         */
void pm_bs();                    /* Process multibyte, -b & -s options.   */

/* "proc" is an array of pointers to file-processing functions.           */

func_ptr proc[8] = {ps_none,
                    ps_b,
                    ps_s,
                    ps_bs,
                    pm_none,
                    pm_b,
                    pm_s,
                    pm_bs};
func_ptr process;                /* Holds the pointer to the appropriate  */
                                 /*   function for processing files.      */

/*****************  
 * Main program. * 
 *****************/
 
main(int argc, char *argv[])
{
  int fn;                        /* File name (index into argv).          */
  int indexa;                     /* Index into proc[] array.              */

  /* Set the locale and open the appropriate message catalog.             */

  (void) setlocale(LC_ALL,"");
  catd = catopen(MF_FOLD,NL_CAT_LOCALE);

  /* Get the value of the current maximum byte length of a multibyte     */
  /*   character. If it is greater than 1, the program should follow the */
  /*   multibyte path. If the maximum display width is greater than 1,   */
  /*   the multibyte path will be followed regardless of the value of    */
  /*   MB_CUR_MAX.                                                       */

  mbcs = MB_CUR_MAX;
  blank_wctype = wctype("blank");
  mbflag = (mbcs > 1);

  err = 0;                       /* No errors yet.                       */

  command_line(argc,argv);       /* Parse the command line arguments.    */

  indexa = 0;                     /* The proper file-processing function  */
  if (mbflag)                    /*   is chosen based on three criteria: */
    indexa += 4;                  /*   whether the input is single-byte   */
  if (sflag)                     /*   or multibyte; the use of the -b    */
    indexa += 2;                  /*   option; and the use of the -s      */
  if (bflag)                     /*   option. An offset into the proc[]  */
    indexa += 1;                  /*   array is calculated and put into   */
  process = proc[indexa];         /*   "index."                           */

  if(argv[optind]==NULL)         /* No input files specified?            */
    (void) (*process)();         /* Then process stdin.                  */
  else
    for (fn=optind; fn < argc; fn++)    /* Otherwise, take each file one */
      if(!freopen(argv[fn],"r",stdin))  /*   at a time and associate it  */
      {                                 /*   with stdin.                 */
        fprintf(stderr,MSGSTR(BADFIL,"Cannot open file %s\n"),argv[fn]);
        err = 2;                 /* Error: Don't abort, just report it.  */
      }
      else
        (void) (*process)();     /* Opened OK; process the file.         */

  exit(err);                     /* Did any file fail to open? Exit with */
                                 /*   a non-zero return code.            */
}

/* command_line -- Handle all the command line options and arguments.    */

void command_line(int argc, char *argv[])
{
  int arg;                       /* Return value from getopt().          */

  /* Handle all the command-line arguments using getopt(). The error     */
  /*   messages from getopt() are not disabled although the program      */
  /*   issues its own messages.                                          */

  syntax = 0;                    /* No syntax errors found yet.          */
  while ((arg = getopt(argc,argv,"bsw:")) != SPECIAL)
  {
    switch(arg)
    { 
      case 'b': bflag = 1;       /* Set the -b flag.                     */    
                break;           
      case 's': sflag = 1;       /* Set the -s flag.                     */
                break;
      case 'w': if(getwidth(optarg,&width))  /* Width; valid number?     */
                {
                  fprintf(stderr,MSGSTR(ILLNUM,"Invalid number.\n"));
                  syntax = 1;    /* Bad command-line syntax.             */
                }
                break;
      case '?':
      default :
                syntax = 1;      /* Bad command-line syntax.             */
                break;
    }
  }
 
  /* The hyphen (-) is not allowed as a way of indicating standard input. */

  if(argv[optind] && STREQ(argv[optind],"-"))
  {
    fprintf(stderr,MSGSTR(NOHYPH,"Cannot use '-' here."));
    syntax = 1;                  /* Bad command-line syntax.             */
  }

  if(syntax)
  {
    fprintf(stderr,MSGSTR(USAGE,"Usage: fold [-bs] [-w Width] [File...]\n"));
    exit(1);
  }
}
 
/* ps_none() -- Process single-byte data, no command-line options.       */
 
void ps_none()
{
  ch=fgetc(stdin);

  for(col=1;;)                   /* Loop until eof forces a break.       */
  {
    if(ch==EOF)
      break; 

    putc(ch,stdout);             /* Output the character.                */
    switch (ch)                  /* No eof yet; adjust column-count      */
    {                            /*   based on the last character.       */
      case '\n': col = 1;        /* Newline: Reset the column count.     */
                 break;
      case '\t': col = ((col + 8) & ~7) + 1;  /* Tab: Adjust to next tab */
                 break;          /*   stop; multiple of 8 plus 1.        */
      case '\b': if(col > 1)     /* Backspace: Decrement column-count.   */
                   --col;
                 break;
      case '\r': col=1;          /* Carriage return: col. to beginning   */
                 break;
      default  : ++col;          /* Other: Increment column count by 1.  */
    }
    ch=fgetc(stdin);
    if ((col > width) && (ch != '\r') && (ch != '\b') && (ch != '\n'))
    {
      putc('\n',stdout);         /* Then print a newline and start over. */
      col = 1;
    }
  }
}

/* ps_b() -- Process single-byte data with the -b option.                */
 
void ps_b()
{
  for(bytes=0;;)                 /* Counting bytes, not columns.         */
  {
    if((ch=fgetc(stdin))==EOF)   /* Exit loop upon reaching eof.         */
      break;
    putc(ch,stdout);             /* Output the character.                */
    if(++bytes>=width)           /* Width about to be exceeded?          */
    {
      putc('\n',stdout);         /* Then print a newline and start over. */
      bytes = 0;
    }
  }
}
 
/* ps_s() -- Process single-byte data with the -s option.                */
 
void ps_s()
{
  int  nc;                       /* Number of characters read so far.    */
  int  sc;                       /* Index of last space character.       */
  int  sl;                       /* String length of input line (bytes). */
  int  pos;                      /* Position at which to break line.     */
  char save;                     /* Temporary variable.                  */

  cb = (char *) buffer;          /* Set cb to the start of the buffer.   */
  *cb = '\0';              	 /* Ensure that we have a null string.   */

  for(;;)                        /* Loop until line is broken.           */
  {
    if(!*cb)                     /* Empty buffer? Then try to get a line */
    {
      cb = (char *) buffer;      /* Set cb to the start of the buffer.   */
      if(!fgets(cb,LINE_MAX,stdin)) /*   of input from stdin.            */
          break;                 /* EOF: Break and return to main().     */
      else
      {
        sl = strlen(cb);          /* Find the length of the string and   */
        longline = cb[sl-1]!='\n'; /*   check whether the last byte      */
        if(!longline)             /*   is a newline; if so, change it to */
          cb[sl-1] = '\0';   	  /*   a terminating NULL character.     */
      }
    }

    sl = strlen(cb);

    /* Loop until the column count reaches the specified width or the    */
    /*   end of the input line is reached.                               */
    for(col=1, nc=sc=0; col<=width && nc<sl-1; nc++)
    {
      switch(cb[nc])             /* Adjust column based on last char.    */
      {
        case '\b': if(col>1)     /* Backspace: Decrement counter by 1,   */ 
                     --col;      /*   but don't let it become negative.  */ 
                   break;
        case '\t': col = ((col + 8) & ~7) + 1; /* Tab: Adjust counter.   */
                   break;
        case '\r': col = 1;      /* Carriage return: column to beginning */
                   break;
        default  : ++col;        /* Other: Just increment the counter.   */
      }
      if(isblank(cb[nc]))
      {
        sc = nc;
      }
    }

    if(isblank(cb[sc]))          /* Loop terminated because we found a   */
      pos = sc + 1; 		/*   space? Then break at "sc+1". */ 
    else
      pos = nc;                  /* Otherwise, break at position "nc".   */

    if(col<=width)                 /* Width not yet reached?               */
    {
      puts(cb);                  /* Then print the entire line.          */
      *cb = '\0';          	 /* Empty the buffer.                    */
    }
    else                         /* Line is longer than "width".         */
    {                            /* Save the character at the break      */
      save = cb[pos];            /*   position, stash a terminating NULL */
      cb[pos] = '\0';            /*   there, and print out the string;   */
      puts(cb);                  /*   after that, move the entire tail   */
      cb[pos] = save;            /*   of the buffer to the head. We are  */
      cb += pos;                 /*   then ready for the next iteration. */
    }
  }
}
  
/* ps_bs() -- Process single-byte data with the -b and -s options. */
 
void ps_bs()
{
  int  off;                      /* Offset into cb (last space char).    */
  int  sl;                       /* String length.                       */
  char save;                     /* Temporary variable.                  */

  cb = (char *) buffer;          /* Set cb to the start of the buffer.   */
  *cb = '\0';              	 /* Ensure that we have a null string.   */
  for(;;)                        /* Loop until line is broken.           */
  {                                                                        
    if(!*cb)                     /* Empty buffer? Then try to get a line */
    {
      cb = (char *) buffer;      /* Set cb to the start of the buffer.   */
      if(!fgets(cb,LINE_MAX,stdin)) /*   of input from stdin.            */
          break;                 /* Then break and return to main().     */    
      else                                                                     
      {                                                                        
        sl = strlen(cb);          /* Find the length of the string and   */    
        longline = cb[sl-1]!='\n'; /*   check whether the last byte      */
        if(!longline)             /*   is a newline; if so, change it to */
          cb[sl-1] = '\0';  	  /*   a terminating NULL.               */
      }
    }

    sl = strlen(cb);             /* Find length of line.                 */
    if(sl<=width)                /* Entire line will fit?                */
    {
      puts(cb);                  /* Output entire line.                  */
      *cb = '\0';          	 /* Ensure buffer is empty.              */
    }
    else
    {
      for(off=width; off>0; off--) /* Find the last space char in range. */
        if(isblank(cb[off]))
          break;

      if (off)
        ++off;                   /* Increment past the space character.  */
      else
        off = width;             /* off is zero. So output default width */

      save = cb[off];            /* Replace char with a NULL and output  */
      cb[off] = '\0';      	 /*   the resulting string.              */
      puts(cb);                  /* Output the string, replace the over- */
      cb[off] = save;            /*   written char, and move the tail of */
      cb += off;                 /*   the buffer to its head.            */
    }
  }
}
   
/* pm_none() -- Process multibyte data with no command-line options. */
 
void pm_none()
{
  wc=fgetwc(stdin);
  for(col=1;;)                   /* Loop until eof.                      */
  {                                                                        
    if(wc==WEOF)                 /* Break if eof reached.                */   
      break;                                                               

    switch (wc)                                                            
    {
      case L'\n': col = 1;       /* Reset column count.                  */ 
                  break;
      case L'\t': col = ((col + 8) & ~7) + 1; /* Go to next tab stop.    */
                  break;
      case L'\b': if(col > 1)    /* Decrement columns, but don't let it  */
                    --col;       /*   become negative.                   */
                  break;
      case L'\r': col=1;         /* Carriage return: col. to beginning   */
                  break;
      default   : col += wcwidth((wchar_t)wc); /* Add the display width  */
                  /* Will displaying this character exceed width? */
                  /* (col-1) is the last column this character would occupy. */

                  if ( (col-1) > width )  
                  {
                     putwc(L'\n',stdout); /* Output a newline and reset count.*/
                     col = 1 + wcwidth((wchar_t)wc);  /* Put this char. on next line */
                  }
    }

    putwc(wc,stdout);            /* Output the current character.        */
    wc=fgetwc(stdin);
    if ((col > width) && (wc != L'\r') && (wc != L'\b') && (wc != L'\n'))
    {
      putwc(L'\n',stdout);       /* Output a newline and reset count.    */
      col = 1;
    }

  }
}

/* pm_b() -- Process multibyte data with the -b option. */
 
void pm_b()
{
  char  mb[10];
  short bc;

  for(bytes=0;;)			/* Counting bytes, not columns.	*/
  {
    if((wc=fgetwc(stdin))==WEOF)	/* Exit loop upon reaching eof.	*/
      break;
    bc = wctomb(mb,(wchar_t)wc);	/* compute width		*/
    putwc(wc,stdout);			/* Output the character.	*/
    bytes += bc;
    if (bytes >= width)			/* Does it exceed width?	*/
    {					/*      print a newline		*/
      putwc(L'\n',stdout);
      bytes = 0;
    }
  }
}

/* pm_s() -- Process multibyte with the -s option. */
 
void pm_s()
{
  int     nc;                    /* Number of characters read so far.    */
  int     sc;                    /* Index of last space character.       */
  int     sl;                    /* String length of input line (bytes). */
  int     pos;                   /* Position at which to break line.     */
  wchar_t save;                  /* Temporary variable.                  */
                                                                           
  wb = (wchar_t *) buffer;       /* Set cb to the start of the buffer.   */
  *wb = '\0';          		 /* Ensure that we have a null string.   */
                                 
  for(;;)                        /* Loop until line is broken.           */
  {                                                                        
    if(!*wb)                     /* Empty buffer? Then try to get a line */
    {
      wb = (wchar_t *) buffer;   /* Set cb to the start of the buffer.   */
      if(!fgetws(wb,LINE_MAX,stdin)) /* Get a line of input.            */
          break;                 /* Then break and return to main().     */
      else                                                                 
      {                                                                    
        sl = wcslen(wb);          /* Find the length of the string and   */
        longline = wb[sl-1]!=L'\n'; /*   check whether the last byte      */
        if(!longline)              /*   is a newline; if so, change it to */
          wb[sl-1] = '\0'; 	   /*   a terminating NULL.               */
      }
    }

    /* Loop until the column count reaches the specified width or the    */
    /*   end of the input line is reached.                               */

    sl = wcslen(wb);

    for(col=1, nc=sc=0; col<=width && nc<=sl-1; nc++)
    {
      switch(wb[nc])               /* Adjust column based on last char.    */
      {                        
        case L'\n': col = 1;       /* Newline: Start counter over.         */
                    break;                                                   
        case L'\b': if(col>1)      /* Backspace: Decrement counter by 1,   */
                      --col;       /*   but don't let it become negative.  */
                    break;                                                   
        case L'\t': col = ((col + 8) & ~7) + 1; /* Tab: Adjust counter.    */
                    break;
        case L'\r': col = 1;       /* Carriage return: col. to beginning   */
                   break;
        default   : col += wcwidth(wb[nc]); /* Other: Just increment the   */
                                            /*   counter.                  */ 
      }
      if(iswblank(wb[nc]))
      {
        sc = nc;
      }                                                                       
    }                                                                         
                                                                              
    if(iswblank(wb[sc]))           /* Loop terminated because we found a   */ 
      pos = sc + 1;		  /*   space? Then break at "sc+1".   */ 
    else                                                                      
      pos = nc - ((col-1)>width ? 1 : 0); /* Otherwise, break at position "nc".   */ 
				/* col-1 is the last display column. */
				/* If last character exceeds width, break at */
				/* position nc-1 */
                                                                              
    if(col<=width)                 /* Width not yet reached?               */ 
    {                                                                         
      fputws(wb, stdout);          /* Then print the entire line.          */
      fputwc(L'\n', stdout);
      *wb = '\0';         	   /* Empty the buffer.                    */ 
    }                                                                         
    else                           /* Line is longer than "width".         */ 
    {                              /* Save the character at the break      */ 
      save = wb[pos];              /*   position, stash a terminating NULL */ 
      wb[pos] = '\0';     	   /*   there, and print out the string;   */ 
      fputws(wb, stdout);          /*   after that, move the entire tail   */ 
      fputwc(L'\n', stdout);
      wb[pos] = save;              /*   of the buffer to the head. We are  */ 
      wb += pos;                   /*   then ready for the next iteration. */ 
    }
  }
}
  
/* pm_bs() -- Process multibyte with the -b and -s options. */
 
void pm_bs()
{
  int     sl;                    /* Offset into cb (last space char).    */
  int     off;                   /* String length.                       */
  wchar_t save;                  /* Temporary variable.                  */
 
  wb = (wchar_t *) buffer;       /* Set cb to the start of the buffer.   */
  *wb = '\0';           	 /* Ensure that we have a null string.   */
  for(;;)                        /* Loop until line is broken.           */
  {                                                                        
    if(!*wb)                     /* Empty buffer? Then try to get a line */
    {
      wb = (wchar_t *) buffer;   /* Set cb to the start of the buffer.   */
      if(!fgetws(wb,LINE_MAX,stdin)) /*   of input from stdin.          */
          break;                 /* EOF: Break and return to main().     */    
      else                                                                     
      {
        sl = wcslen(wb);          /* Find the length of the string and   */    
        longline = wb[sl-1]!=L'\n'; /*   check whether the last byte     */
        if(!longline)             /*   is a newline; if so, change it to */
          wb[sl-1] = '\0';   	  /*   a terminating NULL.               */
      }
    }
                                                                           
    sl = wcslen(wb);             /* Find length of line.                 */

    if(sl<=width)                /* Entire line will fit?                */
    {
      fputws(wb, stdout);        /* Output entire line.                  */
      fputwc(L'\n', stdout);
      *wb = '\0';       	 /* Ensure buffer is empty.              */
    }
    else
    {
      for(off=width; off>0; off--) /* Find the last space char in range. */
        if(isblank(wb[off]))
          break;                                                           

      if (off)
        ++off;                   /* Increment past the space character.  */
      else
        off = width;             /* off is zero. So output default width */

      save = wb[off];            /* Replace char with a NULL and output  */
      wb[off] = '\0';   	 /*   the resulting string.              */
      fputws(wb, stdout);        /* Output the string, replace the over- */
      fputwc(L'\n', stdout);
      wb[off] = save;            /*   written char, and move the tail of */
      wb += off;                 /*   the buffer to its head.            */
    }                                                                      
  }
}

/* getwidth() -- Convert a string to an integer with error checking.  */

int getwidth(char *s, int *i)
{
  if(*s>='0' && *s<='9')
  {
    *i = atoi(s);
    if(*i>0 && *i<=LINE_MAX)
      return 0;
  }
  return 1;
}
