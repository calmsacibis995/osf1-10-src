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
/* $XConsortium: indexer.c,v 5.1 91/02/16 09:46:02 rws Exp $ */

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



#include <stdio.h>
#include <ctype.h>
#include "indexer.h"

struct index_entry	*root;

main(argc, argv)
int	argc;
char	*argv[];
{
	int	char_p;
	char	line[MAXLINE];
	int	index_type;
	int	page_type;
	struct index_entry	*entry;
	struct page_entry	*page;


	command_name = argv[0];
	root = NULL;

	line_number = 0;
	while (fgets(line, sizeof(line), stdin) != NULL) {
		line_number++;
		char_p = 0;
				/*  Skip over the start of the line  */
		if (skip_start(line, &char_p) == FALSE)
			continue;

				/*  Obtain type of index entry  */
				/*  ENTRY or DOCUMENT  */
		if ((index_type = get_index_type(line, &char_p)) == 0)
			continue;
		if (index_type == DOCUMENT)
			continue;

				/*  Read the index terms  */
		entry = get_index_terms(line, &char_p);
		entry->index_type = index_type;

		if (index_type == ENTRY) {
				/*  Obtain type of page entry or print entry  */
			if ((page_type = get_page_type(line, &char_p)) == 0) {
				free(entry);
				continue;
			}

			page = get_page_number(line, &char_p);
			page->page_type = page_type;
		
			insert_page_entry(entry, page);
		}

				/*  Insert index entry into the tree  */
		if (root == NULL)
			root = entry;
		else
			entry = insert_index_entry(root, entry);
	}
				/*  Print the index terms  */
	previous_index_entry = NULL;
	print_index(root);

	return(0);
}

/*
 * Read index entries from the index file.  Build a new index entry.
 */
struct	index_entry *
get_index_terms(line, char_p)
	char	*line;
	int	*char_p;
{
	struct index_entry	*entry;
	int	level;
	char	*field;

	entry = new(struct index_entry);
				/*  Read the index terms  */
	for (level = 0; level < LEVELS; level++) {
		field = get_field(line, char_p);
		entry->terms[level] = strdup(field);
	}
				/*  Read the strings to print  */
	for (level = 0; level < LEVELS; level++) {
		field = get_field(line, char_p);
		entry->print_field[level] = strdup(field);
	}
	return(entry);
}


/*
 * Read page number from the index file.  Build a new page number entry.
 */
struct	page_entry *
get_page_number(line, char_p)
	char	*line;
	int 	*char_p;
{
	char	*page_number;
	struct page_entry	*page;


	page_number = get_field(line, char_p);

	page = new(struct page_entry);
	page->page_number = strdup(page_number);
	return(page);
}
