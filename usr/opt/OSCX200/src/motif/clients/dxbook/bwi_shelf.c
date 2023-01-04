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
/* DEC/CMS REPLACEMENT HISTORY, Element VWI-SHELF.C*/
/* *2     9-JUL-1990 15:52:31 FITZELL ""*/
/* *1    24-APR-1990 15:44:22 FITZELL "creating initial elements that shipped with V2 VWI"*/
/* DEC/CMS REPLACEMENT HISTORY, Element VWI-SHELF.C*/
/*  DEC/CMS REPLACEMENT HISTORY, Element VWI-SHELF.C */
/*  *1    11-AUG-1989 15:05:53 DECWBUILD "Initial entry" */
/*  DEC/CMS REPLACEMENT HISTORY, Element VWI-SHELF.C */
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1989 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**/

/*
**  FACILITY:
**
**   VWI -- VOILA Writer Interface
**
** ABSTRACT:
**
**   This module implements the shelf management routines for VWI
**
** AUTHORS:
**
**   Joseph M. Kraetsch
**
** CREATION DATE: 13-JUN-1989
**
** MODIFICATION HISTORY:
*/

/*  
**  INCLUDE FILES  
*/
# include   "bxi_def.h"

#ifdef vcc
#define GLOBAL globaldef
#define EXTERN globalref
#else
#define GLOBAL static
#define EXTERN extern
#endif

extern  VXI$CREATE_BUFFER();

#define MAXSHELVES 30

GLOBAL FILE *LIBRARY[MAXSHELVES];
GLOBAL FILE *ASCII_FILE;
GLOBAL char *asciifilename="library.shelf_list";
/*globalvalue dvc$_cantopen; */

/*
**  bwi_shelf_close	-- close a shelf
**
*/
void bwi_shelf_close (shelf_id)
unsigned    shelf_id;	/*  shelf id (from bwi_shelf_create)    */
{
        fclose(LIBRARY[shelf_id]);
    
};	/*  end of bwi_shelf_close  */



/*
**  bwi_shelf_create  -- create a shelf.
**
**  Returns:  shelf id
*/
unsigned bwi_shelf_create (fname, symbol, title)
char	    *fname;	    /*  name of shelf file to create  */
char	    *symbol;	    /*  symbolic name of this shelf  */
char	    *title;	    /*  shelf title (for display)  */
{
    static int filecount = 0;

    LIBRARY[filecount] = fopen(fname,"w+","dna=.decw$bookshelf");
    /*if(LIBRARY[filecount] == NULL) 
        error_parse(dvc$_cantopen,0,fname,TRUE); */

    if(!filecount)
	ASCII_FILE = fopen(asciifilename,"w+");

    return(filecount++);

};	/*  end of bwi_shelf_create  */


/*
**  bwi_shelf_entry -- Add an entry to a SHELF
*/
int bwi_shelf_entry (shelf_id, entry_type, fname, symbol, title,shelf_level)
unsigned    shelf_id;	    /*  shelf id (from bwi_shelf_create)    */
unsigned    entry_type;	    /*  2 == book; 1 == shelf  */
char	    *fname;	    /*  file name that entry points to  */
char	    *symbol;	    /*  symbolic name of entry  */
char	    *title;	    /*  Title of entry for display  */
int	    shelf_level;	    /*  Indentation level */
{
   int i,status;
   char variable_indent[256];


   status = TRUE;
   for(i=0;i<shelf_level;i++)
	variable_indent[i] = '\t';
   variable_indent[i] = '\0';

   if(fname == NULL)	    /* make sure it's not a null string */
	fname = "";
   if(title == NULL)
	title = "";
   if(symbol == NULL)
	symbol = "";

   if(entry_type == 1)	{    /* 1 = BS_SHELFENTRY , 2 = BS_BOOKENTRY */
        status = fprintf(LIBRARY[shelf_id],"SHELF\\%s\\%s\n",fname,title);
	fprintf(ASCII_FILE,"%s* %s\n",variable_indent,title);
	}
   else {
        status = fprintf(LIBRARY[shelf_id],"BOOK\\%s\\%s\n",fname,title);
	fprintf(ASCII_FILE,"%s+ %s\n",variable_indent,title);
	}

/*   free(variable_indent); */

   if(status)
	status = TRUE;

 return (status);
};	/*  end of bwi_shelf_entry  */

void LOWER_CASE(length,filename)
int length;
char *filename;
{
    int i;
    
    for(i = 0;i < length; i++) 
	filename[i] = tolower(filename[i]);

 }

