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
/* $XConsortium: build_ind.c,v 5.1 91/02/16 09:45:49 rws Exp $ */

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

#include <ctype.h>
#include <stdio.h>
#include "indexer.h"

struct index_entry	*root;

/*  insert_page_entry places a new page number structure into the
    specified index node.     */

insert_page_entry(node, page)
	struct index_entry	*node;
	struct page_entry	*page;
{


	struct page_entry	*temp;

	page->next_page = NULL;

		/*  If there are no pages in the index node at this time we
		    just insert the new entry and return  */
	if (node->last_page == NULL) {
		node->last_page = page;
		node->page_entry = page;
		return;
	} 
		/*  This is a new page entry to be added to a list of existing
		    page entries.  We have to do some consistency checks  */
		/*  Eliminate duplicate page numbers */
	if (page->page_type == PAGE_NORMAL) {
		for (temp = node->page_entry; temp != NULL; temp = temp->next_page) {
			if (temp->page_type == PAGE_NORMAL ||
					temp->page_type == PAGE_MAJOR)
				if (strcmp(temp->page_number, page->page_number) == 0) {
					free (page);
					return;
				}
		}
	}
		/*  There should be only one major entry per reference  */
	if (page->page_type == PAGE_MAJOR) {
		for (temp = node->page_entry; temp != NULL; temp = temp->next_page) {
			if (temp->page_type == PAGE_MAJOR)
				fprintf(stderr,
		"%s: Page number on line %d -- duplicate major reference\n",
		command_name, line_number);
		}
		/*  Place major entries in front  */
		page->next_page = node->page_entry;
		node->page_entry = page;
		return;
	}
		/*  Start and end of range should follow one after the other  */
	if ((node->last_page->page_type == PAGE_START && page->page_type != PAGE_END) ||
	(node->last_page->page_type != PAGE_START && page->page_type == PAGE_END)) {
		fprintf(stderr, "%s: Page range on line %d must be contiguous\n",
				command_name, line_number);
		node->last_page->page_type = PAGE_NORMAL;
		page->page_type = PAGE_NORMAL;
		node->last_page->next_page = page;
		node->last_page = page;
		return;
	}
		/*  Just an ordinary page entry -- insert it  */
	node->last_page->next_page = page;
	node->last_page = page;
}


/*  insert_index_entry inserts a new index entry at its correct place in
    the tree of index entries and returns the address of the node
    corresponding to the entry inserted.  */

struct index_entry *
insert_index_entry(node, entry)
	struct	index_entry	*node;
	struct	index_entry	*entry;
{

	int	result;
	int	level;

	result = compare_entry(node, entry);

	if (result == 0) {	/*  exact match  */
				/*  Place the page entry for the duplicate */
				/*  into the list of pages for this node */
		insert_page_entry(node, entry->page_entry);
		free(entry);
		return(node);
	}

	if (result > 0)		/*  node greater than new entry -- */
		 		/*  move to lesser nodes */
		if (node->lesser != NULL)
			insert_index_entry(node->lesser, entry);
		else {
			node->lesser = entry;
			return (node->lesser);
		}
	else			/*  node less than new entry -- */
				/*  move to greater nodes */
		if (node->greater != NULL)
			insert_index_entry(node->greater, entry);
		else {
			node->greater = entry;
			return (node->greater);
		}
}


/*  compare_entry compares two index entries.  compare_entry returns
    0 for an exact match, < 0 for entry_1 < entry_2, and > 0 for
    entry_1 > entry_2.  Null entries are considered less than non-null
    entries. */

int
compare_entry(entry_1, entry_2)
	struct	index_entry	*entry_1;
	struct	index_entry	*entry_2;
{
	int	level;
	int	result;

	for (level = 0;  level < LEVELS;  level++) {
		result = compare_strings(entry_1->terms[level],
		entry_2->terms[level]);
		if (result != 0)
			return(result);
	}
	return (0);	/*  exact match  */
}


/*
 *  compare_terms compares the collating terms of two index entries.
 *  compare_terms returns:
 *  0 for an exact match,
 *  < 0 for entry_1 < entry_2,
 *  > 0 for entry_1 > entry_2.
 *
 *  Null entries are considered less than non-null entries.
*/

int
compare_terms(entry_1, entry_2)
	struct	index_entry	*entry_1;
	struct	index_entry	*entry_2;
{
	int	level;
	int	result;

	for (level = 0;  level < LEVELS;  level++) {
		result = compare_strings(entry_1->terms[level],
		entry_2->terms[level]);
		if (result != 0)
			return(result);
	}
	return (0);	/*  exact match  */
}


/*  compare_prints compares the printing terms of two index entries.
 *  compare_prints returns:
 *  0 for an exact match,
 *  1 for different
*/

/* int
compare_prints(entry_1, entry_2)
	struct	print_entry	*entry_1;
	struct	print_entry	*entry_2;
{
	int	level;
	int	result;

	for (level = 0;  level < LEVELS;  level++) {
		if (entry_1->print_field[level] == NULL &&
				entry_2->print_field[level] == NULL)
			continue;
		if (entry_1->print_field[level] == NULL &&
				entry_2->print_field[level] != NULL)
			return(1);
		if (entry_1->print_field[level] != NULL &&
				entry_2->print_field[level] == NULL)
			return(1);
		if (strcmp(entry_1->print_field[level],
				entry_2->print_field[level]) 1= 0)
			return(1);
	}
	return (0);
}	 */





/*  compare_strings compares the two strings passed as arguments, with the
    following extra constraints:
    .  lower-case letters and upper-case letters are considered the same.
    .  Punctuation characters come to the front of the collating sequence.
    .  A shorter string is considered extended with spaces.

    compare_strings returns:  0 if string_1 is equal to string_2,
			     -1 if string_1 is less than string_2,
    			     +1 if string_1 is greater than string_2  */

int
compare_strings(string_1, string_2)
	char	*string_1;
	char	*string_2;
{
	int	length_1;	/*  length of first string  */
	int	length_2;	/*  length of second string  */
	int	pos_1;	 	/*  character position in string_1 */
	int	pos_2;		/*  character position in string_2 */
	char	char_1;	 	/*  character from string_1 */
	char	char_2;		/*  character from string_2 */

				/*  first check for null strings  */
	if (string_1 == NULL && string_2 == NULL)
		return(0);
	if (string_1 == NULL && string_2 != NULL)
		return(-1);
	if (string_1 != NULL && string_2 == NULL)
		return(1);
				/*  Exact match just returns straight away  */
	if (strcmp(string_1, string_2) == 0)
		return(0);

	length_1 = strlen(string_1);
	length_2 = strlen(string_2);
				/*  check for zero-length strings  */
	if (length_1 == 0 && length_2 == 0)
		return(0);
	if (length_1 == 0 && length_2 != 0)
		return(-1);
	if (length_1 != 0 && length_2 == 0)
		return(1);

	pos_1 = pos_2 = 0;
	while (TRUE) {
		if (pos_1 >= length_1)
			char_1 = ' ';
		else
			char_1 = string_1[pos_1];
		char_1 = isupper(char_1) ? tolower(char_1) : char_1;

		if (pos_2 >= length_2)
			char_2 = ' ';
		else
			char_2 = string_2[pos_2];
		char_2 = isupper(char_2) ? tolower(char_2) : char_2;

			/*  First test alphanumerics  */
		if (isalnum(char_1) && isalnum(char_2)) {
			if (char_1 > char_2)
				return(1);
			if (char_1 < char_2)
				return(-1);
		}

			/*  Alphanumerics collate higher than punctuation  */
		if (isalnum(char_1) && ispunct(char_2))
			return(1);
		if (ispunct(char_1) && isalnum(char_2))
			return(-1);

			/*  Punctuation collates among itself  */
		if (ispunct(char_1) && ispunct(char_2)) {
			if (char_1 > char_2)
				return(1);
			if (char_1 < char_2)
				return(-1);
		}

			/*  Spaces collate lower than anything else  */
		if (char_1 > char_2)
			return(1);
		if (char_1 < char_2)
			return(-1);

		if (pos_1 >= length_1 && pos_2 >= length_2)
			break;
		pos_1++;
		pos_2++;
	}
	if (char_1 > char_2)
		return(1);
	if (char_1 < char_2)
		return(-1);
	return(0);	/*  Should not happen  */
}
