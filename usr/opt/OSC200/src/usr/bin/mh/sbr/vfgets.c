/* vfgets.c - virtual fgets */

#include "../h/mh.h"
#include <stdio.h>


#define	QUOTE	'\\'


int	vfgets (in, bp)
     register FILE *in;
     register char  **bp;
{
  register int    toggle, sl;
  register char  *cp,
  *dp,
  *ep,
  *fp;
  static int  len = 0;
  static char *pp = NULL;
  
  if (pp == NULL)
    if ((pp = malloc ((unsigned) (len = BUFSIZ))) == NULL)
      adios (NULLCP, "unable to allocate string storage");
  
  for (ep = (cp = pp) + len - 1;;) {
    if (fgets (cp, ep - cp + 1, in) == NULL) {
      if (cp != pp) {
	*bp = pp;
	return OK;
      }
      return (ferror (in) && !feof (in) ? NOTOK : DONE);
    }
    
    /* Begin new code by Bob Heiney, Digital Equipment Corporation */

    /* The vfgets that ships with MH 6.7 (and the MH in ULTRIX 4.3) will
       consume a QUOTE (i.e. not provide the quoting effect) if the quote is
       the very last character in the 'pp' buffer (i.e. when fgets is
       called, there's space to read in the line up to the QUOTE but not
       including the following new line.

       This code fixes the problem, but removes the ability to quote the
       quote before the '\n'.  E.g. '\\\n' will be returned as '\' not
       '\\n'. */

    /* ok.  we've got a string, let's check it out */

    sl = strlen(cp);

    /* is last char \n? */
    if (cp[sl-1]=='\n')
    {
      /* is second-to-last char (if any) a QUOTE? */
      if (sl>1 && cp[sl-2]==QUOTE)
	/* yes, set next char to write to be the QUOTE */
	cp += sl-2;
      else
      {
	/* no quote, so we're at the end of the virtual line;
	   return the line */
	*bp=pp;
	return OK;
      }
    }
    else /* last char not \n */
    {
      /* is last char a quote? */
      if (cp[sl-1]==QUOTE)
      {
	/* yes, peek at the next char and figure out what to do */
	int c;
	
	c = getc(in);
	/* if the next char isn't \n put it back, otherwise consume it */
	if (c!='\n')
	  if (c!=EOF)
	    ungetc(c, in);
	
	cp += sl-1;  /* make the next char to write be the QUOTE, so
			we'll overwrite it */
      }
      else
      {
	cp += sl;
      }
    }

    /* End new code by Bob Heiney, Digital Equipment Corporation */

    if (cp >= ep) {
      register int curlen = cp - pp;
      
      if ((dp = realloc (pp, (unsigned) (len += BUFSIZ))) == NULL)
	adios (NULLCP, "unable to allocate string storage");
      else
	cp = dp + curlen, ep = (pp = dp) + len - 1;
    }
  }
}
