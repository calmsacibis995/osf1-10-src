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
/* $XConsortium: xref.h,v 5.1 91/02/16 09:45:55 rws Exp $ */

/***********************************************************
Copyright (c) 1990, 1991 by Sun Microsystems, Inc. and the X Consortium.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Sun Microsystems,
the X Consortium, and MIT not be used in advertising or publicity 
pertaining to distribution of the software without specific, written 
prior permission.  

SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT 
SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/


#define	FALSE	0
#define	TRUE	1

/*
 * Structure for a codeword entry
 */
struct	codeword_entry	{
	int	entry_type;		/*  Type of entry  */
	char	*codeword;		/*  Actual Codeword  */
	char	*title;			/*  Text of Title  */
	int	h1_counter;		/*  Chapter Level Counter  */
	int	h2_counter;		/*  Section Level Counter  */
	int	h3_counter;		/*  SubSection Level Counter  */
	int	h4_counter;		/*  Paragraph Level Counter  */
	int	h5_counter;		/*  SubParagraph Level Counter  */
	int	table_number;		/*  Table Number  */
	int	figure_number;		/*  Figure Number  */
	int	appendix;		/*  TRUE if this is an appendix  */
	int	page_number;		/*  Page Number (not yet available)  */
	struct	codeword_entry	*lesser;/*  pointer to lesser number */
	struct	codeword_entry	*greater;/*  pointer to greater number */
};

				/*  Codeword Types  */
#define	HEADING		1
#define	TABLE		2
#define	FIGURE		3
#define	CROSSREF	4
				/*  Phase of Processing  */
#define	GATHER_REFERENCES	1
#define	SUBSTITUTE_REFERENCES	2
				/*  Document Types  */
#define	MINOR_SECTIONED	1
#define	MAJOR_SECTIONED	2
				/*  Reference Types  */
#define	NUMBER	1
#define	TITLE	2
				/*  Instructions to the token reader  */
#define	SKIP_SPACES	1
#define	DONT_SKIP_SPACES	2
				/*  Types of tokens  */
#define	SPACES_TOKEN	1
#define	DELIMITER_TOKEN	2
#define	STRING_TOKEN	3
#define	ALPHA_TOKEN	4
#define	NUMBER_TOKEN	5
#define	ENDOFLINE_TOKEN	6

FILE	*current_file;		/*  Current input file  */
char	*current_filename;	/*  Name of current input file  */
int	line_number;		/*  Line number in current file  */
int	document_type;		/*  Major Sectioned or Minor Sectioned  */
char	*command_name;		/*  Name of command  */
struct codeword_entry	*previous_codeword_entry;

#define MAXLINE	512

#define SPACE	' '
#define TAB	'\t'

#define	strdup(str)	strcpy(malloc(strlen(str) + 1), str)
#define new(type)	(type *) calloc(sizeof(type), 1)
#define exists(arg)	(strcmp(arg, "") != 0)

char	*malloc();
char	*calloc();
char	*strcpy();
char	get_char();
char	*get_field();
char	*skipspace();
struct codeword_entry	*build_codeword_entry();
struct codeword_entry	*insert_codeword_entry();
struct codeword_entry	*locate_codeword_entry();
