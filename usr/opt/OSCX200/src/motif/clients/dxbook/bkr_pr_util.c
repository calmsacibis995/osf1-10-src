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
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1991  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use, 	     **
**  duplication or dissemination of the software and	     **
**  media is authorized only pursuant to a valid written     **
**  license from Digital Equipment Corporation.	    	     **
**  	    	    	    	    	    	    	     **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	     **
**  disclosure by the U.S. Government is subject to 	     **
**  restrictions as set forth in Subparagraph (c)(1)(ii)     **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as  	     **
**  applicable.	    	    	    	    	    	     **
***************************************************************
*/



/*======================== bkr_pr_util.c =============================
File Description: Customer Demand Print: non-edm ps file processing
----------------

Modules: str_equal ioparse getpageordfolio
-------

History: 25-feb-91 f.klum; created
-------
*/
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <Xm/Xm.h>
#include "br_common_defs.h"
#include "br_meta_data.h"
#include "br_typedefs.h"
#include "br_printextract.h"
#include "bkr_pr_util.h"


/*----------------------- rangesort ----------------------------
Purpose: Bubble sort rng structs  - slow but small and simple.
-------  
--------------------------------------------------------------*/
extern void rangesort(first_range)
  range_t    *first_range;
  {
  range_t    *r1;
  range_t    *r2;
  range_t    t;

  if(!first_range)
    return;

  if(!first_range->next)
    return;

  for(r1 = first_range; r1->next; r1 = r1->next)
    {
    for(r2 = r1->next; r2; r2 = r2->next)
      {
      if(r1->ord_start > r2->ord_start)
        {
        t.type = r1->type;
        t.ord_end = r1->ord_end;
        t.ord_start = r1->ord_start;
        t.folio_start = r1->folio_start;
        t.folio_end = r1->folio_end;
        t.topic_start = r1->topic_start;
        t.topic_end = r1->topic_end;
        t.offset_start = r1->offset_start;
        t.offset_end = r1->offset_end;
        t.n_pages = r1->n_pages;
        t.xstring = r1->xstring;

        r1->type = r2->type;
        r1->ord_end = r2->ord_end;
        r1->ord_start = r2->ord_start;
        r1->folio_start = r2->folio_start;
        r1->folio_end = r2->folio_end;
        r1->topic_start = r2->topic_start;
        r1->topic_end = r2->topic_end;
        r1->offset_start = r2->offset_start;
        r1->offset_end = r2->offset_end;
        r1->n_pages = r2->n_pages;
        r1->xstring = r2->xstring;

        r2->type = t.type;
        r2->ord_end = t.ord_end;
        r2->ord_start = t.ord_start;
        r2->folio_start = t.folio_start;
        r2->folio_end = t.folio_end;
        r2->topic_start = t.topic_start;
        r2->topic_end = t.topic_end;
        r2->offset_start = t.offset_start;
        r2->offset_end = t.offset_end;
        r2->n_pages = t.n_pages;
        r2->xstring = t.xstring;
        }
      }
    }
  return;
  }

#if FALSE
/*----------------------- rangesort ----------------------------
Purpose: Bubble sort rng structs  - slow but small and simple.
-------  
--------------------------------------------------------------*/
extern void rangesort(range,n)
  range_t *range[];   /* array of range structs to sort */
  int     n;          /* number of elements in array */
  {
  int i,j,m;
  range_t *temp;

  for(i=0, m=n-1; i < m; ++i)
    {
    for(j=i+1; j < n; ++j)
      {
      if(range[i]->ord_start > range[j]->ord_start)
        {
        temp = range[i];
        range[i] = range[j];
        range[j] = temp;
        }
      }
    }
  return;
  }
#endif

/*----------------------- str_equal ----------------------------------
Purpose: Return TRUE if strings are the same (case sensitive),
-------  FALSE otherwise.
-----------------------------------------------------------------*/
extern int str_equal(s1,s2)
  char      *s1;
  char      *s2;
  {
  while(*s1 && *s2)
    {
    if(*s1++ != *s2++)
      return(FALSE);
    }
  return(!(*s1 || *s2));
  }

/*----------------------- str_equal_nocase ------------------------
Purpose: Return TRUE if strings are the same (case sensitive),
-------  FALSE otherwise.
-----------------------------------------------------------------*/
extern int str_equal_nocase(s1,s2)
  char      *s1;
  char      *s2;
  {
  while(*s1 && *s2)
    {
    if(toupper(*s1) != toupper(*s2))
      return(FALSE);
    s1++;
    s2++;
    }
  return(!(*s1 || *s2));
  }

/*---------------------------- ioparse -------------------------------
Purpose: Read and optionally write bytes until "\n%%" is found.
-------  When found, read into comment buffer until \n.

Description: Read and write until "\n%%" or eof is found.
-----------  Read bytes into comment until '\n' or eof.
             Inital whitespace after "%%" is filtered.
             The comment string is then nulled at the
             first space or ':' after the comment keyword, unless
             the comment keyword is '+' ("\n%%+ is a continuation
             of previous structured comment). Argbuf is returned to
             point into the comment string to the first non-space char
             after the comment keyword and also past ':' or '+' if
             present. The end result is that comment points to the
             first nul terminated string (the comment keyword)
             with ':' and whitespace stripped out and argbuf
             points to the second string (the first argument) with leading
             whitespace removed. If the line is "%%+", comment points
             to the entire line starting at '+', and argbuf points
             to the first argument with leading whitespace removed.
-----------------------------------------------------------------------*/
int ioparse(in_fp, out_fp, filebytes, comment, argbuf, output)
  FILE   *in_fp;   /* input file ptr */
  FILE   *out_fp;  /* output file ptr */
  char   *filebytes; /* buffer of bytes in comment string */
  char   *comment; /* comment buffer to fill after seeing "\n%%" or "\n%D */
  char   **argbuf; /*  ptr ptr into comment to first argument */
  int    output;   /* if true, write */
  {
  int    i_ch;     /* byte read into int to check for eof */
  char   ch;       /* byte read into char */
  char   first_ch; /* saves a ch to test for "\n%%" */     
  int    i;        /* counter */
  int    eof;
  char   *cp;      /* ptr into comment buffer */
  char   *f_cp;    /* ptr into filebytes buffer */
  externalref unsigned long ex_offset;


  /* read and (maybe) write until %% found */
  *filebytes = '\0';
  eof = FALSE;
  for(first_ch = '\n'; !eof; first_ch = ch)
    {
    READ;
    if(eof)
      break;
    WRITE;
    if(ch == '%')
      {
      READ;
      if(eof)
        break;
      WRITE;
      if(((ch == '%') || (ch == 'D')) && (first_ch == '\n'))
        {   /* "\n%%" or "\n%D" has been read and written */

        ex_offset = (unsigned long)ftell(in_fp);

        /* read until buffer is full or newline ends comment */
        for(i=1, cp=comment, f_cp = filebytes;
             ((ch != '\n') && (i < COMENTSIZ)); ++i)
          {
          READ;  /* filter leading whitespace */
          if(eof)
            break;
          if((cp > comment) || !isspace(ch))
            *cp++ = ch;
          *f_cp++ = ch;
          }
        *(f_cp-1) = '\n';  /* make sure we end ok */
        *f_cp = '\0';

        if(cp == comment)  /* empty structured comment */
          continue;        /* resume reading and writing */

        for(--cp; isspace(*cp); --cp); /* nul trailing spaces */
        *++cp = '\0';

        if(i >= COMENTSIZ)
          puts("\n  Warning: comment buffer size exceeded");

        /* set argbuf to point to first arg after comment keyword
           if there is an arg, else 0 */
        /* stop at nul or first space or ':'*/
        for(cp=comment;(*cp && !isspace(*cp) && (*cp!=':') && (*cp!='+'));++cp);
        ch = *cp;
        if(ch != '+')
          *cp = '\0';   /* nul terminate comment keyword if not continue */
        /* filter around ':' */
        if((ch == ':') || (ch == '+'))
          for(++cp; isspace(*cp); ++cp); 
        else if(isspace(ch))  /* *cp is a space */
          {
          ++cp;
          while(isspace(*cp))  /* filter whitespace before ':' */
            ++cp;
          if(*cp == ':')
            for(++cp; isspace(*cp); ++cp); /* filter whitespace after ':' */
          }
        *argbuf = (*cp)? cp: (char *)0;
        return(0);
        }
      }
    }
  return(eof);
  }

/*----------------------- getpageordfolio ------------------------
Purpose: Return ordinal page number and ptr to folio string from
-------  %%Page comment argbuf
------------------------------------------------------------------*/
extern void getpageordfolio(argbuf,folio,ordinal)
  char     *argbuf;    /* ptr to page arg */
  char     **folio;    /* returns ptr to nul terminated folio string */
  int      *ordinal;   /* returns ordinal page number */
  {
  char     *ord;       /* ptr to ordinal string */
  char     *cp;        /* general char ptr */

  /* init as though one arg, ordinal number */
  ord = argbuf;
  *folio = (char *)0;
  /* go past first arg */
  for(cp=argbuf; (*cp && !isspace(*cp)); ++cp);
  if(*cp)  /* space after first arg, maybe a second arg */
    {
    *cp++ = '\0';    /* nul terminate first arg */
    while(isspace(*cp))  /* filter intermediate space */
      ++cp;
    if(*cp)  /* two args to Page comment */
      {
      ord = cp;  /* ordinal is second arg */
      while(*cp && !isspace(*cp))
        ++cp;
      *cp = '\0';  /* nul terminate second arg */
      *folio = argbuf;  /* folio is first arg */
      }
    }
  *ordinal = atoi(ord);  /* convert ord string to integer */
  return;
  }
