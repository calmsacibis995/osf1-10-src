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
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988, 1992 BY                      *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**                         ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
** THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**      PS Previewer widget -- DECwindows widget for viewing PostScript 
**
**  AUTHOR:
**
**      
**	Terry Weissman
**	Joel Gringorten
**
**  ABSTRACT:
**
**      This module contains a routine to parse structured comments.
**
**  ENVIRONMENT:
**
**      User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**	NAME		DATE		REASON
**	----		----		------
**      dam             01-Oct-1990     remove #ifdef __vms__'s to fix problem
**					viewing last page on ultrix
**      dam             20-dec-1990     do not trust strlen() to return length
**					of a buffer from fgets(); (does not
**					work in case of new-line characters
**					within ps strings) Instead, get
**					length of buffer from our own
**					fgets: int_fgets()
**      dam             03-apr-1991     cleanup typedefs
**      dam             05-aug-1991     rename headers, remove dollar signs
**      dam             12-aug-1991     remove unncessary statics
**      cjr             12-Nov-1991     Fix extraction of page number from
**					%%Page: comment to correctly get the
**					page number even if the page label
**					is illegal.  Also fix the handling
**					of the different %%Pages: comment.
**      cjr             19-Nov-1991     Ensure that comments other than
**					%%Page: beginning with %%Page are
**					ignored.
**	rmm		06-Apr-1992	Make renovations to DetermineStructure
**					to support incremental reading of the
**					PS file.
**	ts		21-Sep-1993	Handle nested included docs.
**
**
**
**
**--
**/

/*
 * Parse structured comments in the given stream.  Returns NULL if there are
 * no structured comments. For multipage structured documents, this code
 * reads only as far as the next %%Page, adding to its line starts table
 * incrementally. When the last page has been read (the %%Trailer has been
 * encountered) then the number of pages is saved and an "at end of file"
 * flag is set.
 */

#include <cdatrans.h>

#include <stdio.h>
#ifdef __vms__
#pragma nostandard /* turn off /stand=port for "nonclean" X includes */
#include <Intrinsic.h>
#pragma standard
#else
#include <Intrinsic.h>
#endif
#include "psstruct.h"

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef NOPAGE
#define NOPAGE (-1)
#endif

#ifndef myfseek
#define myfseek(file,pos,mode) fseek((file),(pos),(mode))
#endif

/*  int_fgets() works exactly as does fgets() (code copied from
 *  Kernighan and Ritchie) except it returns the length
 *  of the returned buffer
 */

int int_fgets(s, n, iop)
    char *s;
    int   n;
    FILE *iop;
{
    register int  c;
    register char *cs;
    int		  original_n = n;

    cs = s;
    while (--n > 0 && (c = getc(iop)) != EOF)
	if ((*cs++ = c) == '\n')
	    break;
    *cs = '\0';
    
    return (original_n - n);
}

/*
 * Make a copy of the given string in a newly malloced area.  If given NULL,
 * return NULL.
 */

static char *MallocACopy(str)
char *str;
{
    if (str) return( (char *)
		     strcpy(XtMalloc((unsigned) (strlen(str) + 1)), str) );
    else return NULL;
}

/*
 * Free a structured comment record.
 */

void FreeStructure(sinfo)
Structure sinfo;
{
    int i;
    if (sinfo) {
	for (i=0; i < sinfo->numDocInfo; i++)
	    {
	    XtFree((char *)sinfo->docInfo[i]);
	    }
	XtFree((char *) sinfo->docInfo);
	XtFree((char *) sinfo->pagestarts);
	XtFree((char *) sinfo);
    }
}

/*
   This routine reads the PostScript file, remembering page starts, and
   parsing comments. When it is called initially, oldsinfo should be null.
   Upon subsequent calls, pass the preserved sinfo structure pointer from the
   previous call, and it will be used to restore context to continue the
   file scan for the next page.
*/

Structure DetermineStructure(stream, error, oldsinfo)
    FILE *stream;		/* The PostScript file in question */
    int *error;			/* RETURN value */
    Structure oldsinfo;		/* Previous structure context block ptr */
				/* (RMM 4/1/92) */
{
    char *adobestr = "%!PS-Adobe-";
    char *trailer = "%%Trailer";
    char *totalpages = "%%Pages";
    char *pagemark = "%%Page";
    char *prologend = "%%EndProlog";
    char *commentend = "%%EndComments";
    char *document = "%%BeginDocument";
    char *enddoc = "%%EndDocument";
#define NUM_KEYWORDS 4
    int docLevel = 0; /*This keeps track of BeginDocument and EndDocument level*/

    /* These are what you search for in the document: */

    static char *psstruct_docInfoKeywords[NUM_KEYWORDS] =
	{
	"%%Creator:",
	"%%CreationDate:",
	"%%Title:",
	"%%For:"
	};
    static int ldocInfoKeywords[NUM_KEYWORDS];
    static int ladobestr = -1, ltrailer, ltotalpages, lpagemark, lprologend;
    static int lcommentend, ldocument, lenddoc;

    int lastPageSeen;
    Boolean ignorePage = False;
    int result, ordinal, i;
    Boolean seenPage = False;
    Boolean watchingComments = True;
    char label[100], buf[BUF_LEN+1];
    Structure sinfo = NULL;		/* Local pointer to structure */
    long curFPos = 0;			/* File position on entry */

    long lastFtell, pageBytesRead = 0;	/* Last position, #bytes for page */
    int bytesRead=0,
	pagesReadThisTime = 0;		/* Counts pages read during call */
    Boolean hereBefore = FALSE,		/* Boolean set if re-entry */
	earlyExit = FALSE;		/* Did we see another %%Page? */

    /* Initialize string length values on the first entry only */
    if (ladobestr < 0) {
	ladobestr = strlen(adobestr);
        ltrailer = strlen(trailer);
        ltotalpages = strlen(totalpages);
        lpagemark = strlen(pagemark);
        lprologend = strlen(prologend);
        lcommentend = strlen(commentend);
	ldocument = strlen(document);
	lenddoc = strlen(enddoc);
	for (i = 0; i < NUM_KEYWORDS; i++)
	    ldocInfoKeywords[i] = strlen(psstruct_docInfoKeywords[i]);
    }

    if (error) *error = 0;

    /*
      If we have been here before (oldsinfo not null), restore saved context
    */
    if ( oldsinfo )
      {
	sinfo = oldsinfo;			/* Use same value */
	lastFtell = oldsinfo->savedLastFtell;	/* Restore file pointer */
	bytesRead = oldsinfo->savedBytesRead;	/* and last record read len */
	pageBytesRead = oldsinfo->savedPageBytesRead; /* Last page read len */
	strcpy( buf, oldsinfo->buf);		/* Last buf read */
	hereBefore = TRUE;			/* Deja vu */
	lastPageSeen = oldsinfo->savedLastPageSeen;
	curFPos = ftell(stream);		/* Save current position */
	myfseek(stream,lastFtell,0);		/* Seek to old position */
      }
    else
      {
	lastPageSeen = 0;
	lastFtell = ftell(stream);

        bytesRead = int_fgets(buf, BUF_LEN, stream);
        pageBytesRead += bytesRead;
      }

    /*
	Check for the %!PS-Adobe etc. comment, or re-enter if we're resuming
    */
    if (strncmp(buf, adobestr, ladobestr) == 0 || hereBefore )
      {
	if ( !hereBefore )	/* Have we been here before? */
	  {			/* No, initialize the structed info context */
	    sinfo = XtNew(StructuringDescRec);
	    sinfo->sizepagestarts = -1;
	    sinfo->pagestarts = NULL;
	    sinfo->numPages = NOPAGE;
	    sinfo->numDocInfo = NUM_KEYWORDS;
	    sinfo->docInfo = (char**)XtCalloc(NUM_KEYWORDS, sizeof(char *));
	    sinfo->atEnd = FALSE;	/* End of file flag */
	    sinfo->sizepagestarts += 10;

	/* On VMS, we want a descriptor record for the preamble as well.  At
         * this point, we have found %!Adobe- and know this should be structured.
         * Therefore, allocate the first 10 pages and init page 0 for the
         * preamble.  Note that for Ultrix, page 0 is still there; it is just
         * not used.  This is how it was when I got it.  (BF) 
	 * (update for to behave the same for ultrix -dam)
	 */

	    sinfo->pagestarts = (pageDescr)
		XtRealloc((char *) sinfo->pagestarts,
		    (unsigned) ((sinfo->sizepagestarts + 2) * sizeof(pdesc)));
	    for (i=0; i<=sinfo->sizepagestarts; i++)
		sinfo->pagestarts[i].start = -1;
	    sinfo->pagestarts[0].start = lastFtell;
	  }  /* End of { if (!hereBefore) } */

        while (!feof(stream)) {

	/* Most of the ifdef VMS stuff here is to remove the assumption that
	   the value of ftell before and after a read will differ exactly
	   by the number of bytes read.  This is not true for record oriented
	   files.  Thus we have to keep both start and end addresses as well
	   as the number of bytes read. (ultrix too -dam) */

	    lastFtell = ftell(stream);		/* Remember position */

	    bytesRead = int_fgets(buf, BUF_LEN, stream); /* Get a line */
	    pageBytesRead += bytesRead;		/* Increment total for page */

	    if (buf[0] != '%')			/* Not a PS comment? */
	      {
		watchingComments = False;
		continue;
	      }
	    if (watchingComments)
	      {
	        /*Parse comments containing document information*/

	        if (strncmp(commentend,buf,lcommentend) == 0)
		  watchingComments = False;
	        for (i=0; i < NUM_KEYWORDS; i++)
		  {
		    if (strncmp(psstruct_docInfoKeywords[i],
				buf,ldocInfoKeywords[i])==0)
		      {
			sinfo->docInfo[i] =
			    MallocACopy(&buf[ldocInfoKeywords[i]]);
			if (bytesRead < BUF_LEN)
			  {
			    (sinfo->docInfo[i])
				[bytesRead-ldocInfoKeywords[i]-1] = 0;
			  } /* No line terminator on this string, please*/

		      } /* End of if (strncmp...) */

		    } /* End of for */

	      } /*End of WatchingComments*/

	    if (docLevel)
	      {
		/* 
		 * All we're interested in is the end of the included doc.
		 * Scan for end, and begin - so we swallow nested included
		 * docs.
		 */
		if (strncmp(enddoc,buf,lenddoc) == 0)
		    docLevel--;
	        else if (strncmp(document,buf,ldocument) == 0)
		    docLevel++;
	      }
	    else if (ignorePage == TRUE )
	      {
	        if (strncmp(trailer, buf, ltrailer) == 0)
		    ignorePage = False;
		
	      }
	    else if (strncmp(document,buf,ldocument) == 0)
	      {
		docLevel++;
	      }

            else if (strncmp(totalpages, buf, ltotalpages) == 0)
	      {
                result = sscanf(buf, "%%%%Pages: %d[ ]*\n", &ordinal);
		if (result && (!ordinal)) ignorePage = TRUE;
	      }

            else if ((strncmp(pagemark, buf, lpagemark) == 0) &&
		     ((buf[lpagemark] == ':') ||
		      (buf[lpagemark] == ' ')))
	      {
		/*
		    *** Here's where we start processing a new page ***
		*/
		if ( pagesReadThisTime > 0 )	/* Did we get one already? */
		  {
		    /* Save end position, calculate length of last page read */
		    sinfo->pagestarts[lastPageSeen].end = lastFtell;
		    sinfo->pagestarts[lastPageSeen].length =
			pageBytesRead - bytesRead;

		    /* Save context for the next call */
		    sinfo->savedLastFtell = lastFtell;
		    sinfo->savedBytesRead = bytesRead;
		    sinfo->savedPageBytesRead = pageBytesRead;
		    strcpy( sinfo->buf, buf );
		    sinfo->savedLastPageSeen = lastPageSeen;
		    earlyExit = TRUE;		/* We're not done with file */
		    break;			/* Leave the while loop */
		  }

		result = extract_page_number (buf, lpagemark, &ordinal);
                if (result > 0)
		  {
                    seenPage = TRUE;
		    pagesReadThisTime++;
		    if (ordinal > sinfo->sizepagestarts)
		      {
			/*
			   New page number is larger than last page start;
			   fill the intervening page starts (up to 10) with -1
			*/
		        int old;
			old = sinfo->sizepagestarts;
			sinfo->sizepagestarts += 
			    MAX(10, ordinal - sinfo->sizepagestarts);
			sinfo->pagestarts = (pageDescr)
			    XtRealloc((char *) sinfo->pagestarts,
				      (unsigned) ((sinfo->sizepagestarts + 2) *
						  sizeof(pdesc)));
		        for (i=old+1; i<=sinfo->sizepagestarts; i++)
	                    sinfo->pagestarts[i].start = -1;
		      } /* End { if(ordinal > sinfo...) } */

		    sinfo->pagestarts[ordinal].start = 
		    sinfo->pagestarts[lastPageSeen].end = 
			lastFtell;

		    sinfo->pagestarts[lastPageSeen].length =
			pageBytesRead - bytesRead;

		    pageBytesRead = bytesRead;

		    lastPageSeen = ordinal;
		    /*if (sinfo->numPages < ordinal)
		        sinfo->numPages = ordinal; */
		  }  /* End { if ( result > 0 ) } */
		else
		  {
		    /*
			Invalid page number -- error
		    */
		    if (error)
			*error = 1;
		    FreeStructure(sinfo);
		    sinfo = NULL;
		    rewind(stream);
		    return NULL;
		  }
	      }  /* End { if ( line == pagemark ) } */
	    else if ((strncmp(prologend, buf, lprologend) == 0)
		 && seenPage)
	      {
	        ignorePage = TRUE;
	      }
            else if (strncmp(trailer, buf, ltrailer) == 0)
	      {
	        if (! seenPage)
		  {
		    rewind(stream);
		    FreeStructure(sinfo);
		    sinfo = NULL;
		    return NULL;
		  }
		/* Mark next page start/current page end */
		if ( sinfo->numPages == NOPAGE )
		    sinfo->numPages = lastPageSeen;
			/* Set last page if we haven't seen it yet */
		sinfo->pagestarts[sinfo->numPages + 1].start = 
		    sinfo->pagestarts[lastPageSeen].end =
		    lastFtell;

		sinfo->pagestarts[lastPageSeen].length = 
			pageBytesRead - bytesRead;

		pageBytesRead = bytesRead;

	        break;
	    }  /* End of else if (trailer...) */

	}  /* End of { while (!feof(stream)) } */

        if (feof(stream))	/* trailer missing or comment wrong */
	  {
	    if (error) *error = 1;
	    FreeStructure(sinfo);
	    sinfo = NULL;
	    rewind(stream);
	  }
    }  /* End of { if ( line == adobestr ) } */

    /* Was a previous position established (i.e. during reentry)? */
    if ( curFPos )
      myfseek(stream,curFPos,0);		/* Restore prior position */
    else
      rewind(stream);				/* No prior, go back to start */

    if (sinfo)
      sinfo->curlastpage = lastPageSeen;	/* Remember last page we've */
						/* seen so far... */
    if ( !earlyExit )				/* If we're at the end... */
      {
	if (sinfo)				/* and if we saw structure... */
	  {
	    sinfo->atEnd = TRUE;		/* Set the flag */
	    sinfo->numPages = lastPageSeen;	/* Set the OFFICIAL count */
	    if (sinfo->numPages <= 0)		/* Check for a bad count */
	      {
	        FreeStructure(sinfo);
	        sinfo = NULL;
	      }
	  }
      }
    return sinfo;
}

/*****************************************************************************
 * extract_page_number (page_comment, keyword_length, page_number)
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Extracts the page number from a PS structured %%Page: comment
 *	that is formatted as follows:
 *
 *		%%Page: <label> <ordinal>
 *
 *	where <label> is text that must be enclosed in parens if it
 *	contains whitespace or other special characters, and <ordinal>
 *	is an unsigned integer containing the page number.  This function
 *	will extract the page number even if the label is illegal (e.g.,
 *	contains whitespace without being enclosed in parens.
 *
 *  FORMAL PARAMETERS:
 *
 *	page_comment		R	String containing the %%Page:
 *					structured comment.
 *	keyword_length		R	Length of keyword, used to determine
 *					where to stop the reverse search.
 *	page_number		W	Ptr to integer into which the
 *					extracted page number is placed.
 *
 *  IMPLICIT INPUTS:
 *
 *      None.
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None.
 *
 *  FUNCTION VALUE:
 *
 *      Returns 1 if a numeric page number is found and successfully
 *	extracted, otherwise returns 0.
 *
 *  SIDE EFFECTS:
 *
 *      None.
 *
 ******************************************************************************/


int
extract_page_number (page_comment, keyword_length, page_number)

char *page_comment;
int keyword_length;
int *page_number;

{

int page_number_found;		/* Return value; 1 if number found, else 0. */
char *ch;			/* Sliding pointer into page_comment. */
char *stop;			/* Pointer where reverse search should stop. */
int power_of_ten;		/* Multiplier for current digit in page num. */

/*
* Initialize; page_number is used as an accumulator, so init to zero;
* init page_number_found to "not found" (this is an int rather than a
* boolean so as to sort-of mimic strncmp); init ch to the end of the
* page_comment to set up the reverse search; set the stop point (where
* the reverse search should stop) to just after the %%Page keyword;
* init the power-of-ten multiplier for the current digit to one.
*/
*page_number = 0;
page_number_found = 0;
ch = page_comment + strlen(page_comment);
stop = page_comment + keyword_length;
power_of_ten = 1;

/*
* Run ch backwards until we hit something other than whitespace.
* If this rightmost character is numeric, we've found the page
* number.  This isn't inside the loop that follows, because it
* might get optimized out of the loop and really mess things up.
*/
while ((--ch > stop) &&
       ((*ch == ' ') || (*ch == '\t') || (*ch == '\n')))
    ;
if((*ch >= '0') && (*ch <= '9'))
    page_number_found = 1;

/*
* Run ch backwards, building the page_number, for as long we're
* seeing numerics; note that if the rightmost token is not numeric
* at all, we just fall through this loop and return "not found".
*/
while ((ch > stop) && (*ch >= '0') && (*ch <= '9'))
    {
    *page_number += (*ch - '0') * power_of_ten;
    power_of_ten *= 10;
    ch--;    
    }

/*
* Return 1 if the rightmost token was numeric (the extracted
* page number), otherwise return 0.
*/
return (page_number_found);

}

