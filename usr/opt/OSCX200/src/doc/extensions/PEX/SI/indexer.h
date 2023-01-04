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
/* $XConsortium: indexer.h,v 5.1 91/02/16 09:45:53 rws Exp $ */

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


#define LEVELS		2
#define PRIMARY		0
#define SECONDARY	1
#define	FALSE	0
#define	TRUE	1

/*
 *  Types of Index Entries.  There are three types today:
 *  ENTRY     means this is a straightforward index entry
 *  DOCUMENT  means this is a special entry describing the name of the
 *            document -- this is used when building the master index.
 */

#define	ENTRY		1
#define	DOCUMENT	3
/*
 * Structure for an index entry.  An index entry has the following components:
 * index_type  indicates the type of entry that this is, and is one of the
 *             three types listed above.
 * terms       are the primary and secondary collating terms for this entry.
 * greater     points to index entries that collate greater than this entry.
 * lesser      points to index entries that collate less than this entry.
 * equal       points to index entries that collate equal to this
 *             entry, but whose printing terms are different.
 * print_field is what is printed in place of the collating terms.  If the
 *             print_field entries are null, the collating terms are printed.
 * page_entry  is a list of page numbers for this entry.  last_page keeps
 *             track of the last page entry in the list.
 */
struct	index_entry	{
	int	index_type;		/*  Type of entry  */
	char	*terms[LEVELS];		/* Levels of terms */
	struct	index_entry	*greater;  /* Entries greater than this one */
	struct	index_entry	*lesser;  /* Entries less than this one */
	struct	index_entry	*equal;  /* Chain to equal index terms */
	char	*print_field[LEVELS];	/* What to print instead of the term */
	struct	page_entry	*page_entry;	/* List of page numbers */
	struct	page_entry	*last_page;	/* Last page entry */
	char	*document;	/* Name of document */
};


/*
 *  Types of Page Entries.  There are five types today:
 *  PAGE_NORMAL  means this is a normal page entry -- just print it with no
 *               special treatment.
 *  PAGE_MAJOR   means this is a principal page entry -- the page number is
 *               printed in bold faced text and first in the list of page numbers.
 *  PAGE_START   means this is the start of a page range.
 *  PAGE_END     means this is the end of a page range.
 *  PAGE_PRINT   means that the string following the word PRINT is printed
 *               instead of the page number on which this index entry appears.
 */
#define	PAGE_NORMAL	1
#define	PAGE_MAJOR	2
#define	PAGE_START	3
#define	PAGE_END	4
#define	PAGE_PRINT	5
struct	page_entry	{
	int	page_type;		/* What type of entry this is */
	char	*page_number;		/* A page number */
	struct	page_entry	*next_page;	/* Linked list */
};


int	line_number;		/*  Line number in input file  */
char	*command_name;		/*  Name of command  */
struct index_entry	*previous_index_entry;

#define MAXLINE	512
#define VERTICAL_CHANGE	1
#define DELIM	'\t'
#define SPACE	' '
#define TAB	'\t'

#define	strdup(str)	strcpy(malloc(strlen(str) + 1), str)
#define new(type)	(type *) calloc(sizeof(type), 1)
#define string_exists(arg)	(arg != NULL && strcmp(arg, "") != 0)

char	*malloc();
char	*calloc();
char	*strcpy();
char	get_char();
char	*get_field();
char	*skipspace();
struct index_entry	*get_index_terms();
struct page_entry	*get_page_number();
struct index_entry	*insert_index_entry();
