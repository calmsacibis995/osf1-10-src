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
/* $XConsortium: dump_cw.c,v 5.1 91/02/16 09:45:38 rws Exp $ */

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
#include "xref.h"



/*  dump_codeword_entry dumps a codeword entry in
    the tree of codeword entries and returns the address of the node
    corresponding to the entry dumpd.  */

dump_codewords(node)
	struct codeword_entry	*node;
{


	if (node == NULL)
		return;
	dump_codewords(node->lesser);
	print_node(node);
	dump_codewords(node->greater);
}

print_node(node)
	struct codeword_entry	*node;
{
	char	out_string[MAXLINE];
	char	rest_string[MAXLINE];


	printf("codeword = %s\n", node->codeword);
	printf("%s\n", node->title);
			/*  Handle first level conversion specially */
	if (node->appendix)
		number_to_letters(node->h1_counter, out_string);
	else
		sprintf(out_string, "%d", node->h1_counter);

	strcpy(rest_string, "");
	if (node->entry_type == HEADING) {
		if (node->h5_counter != 0)
			sprintf(rest_string, ".%d.%d.%d.%d", node->h2_counter,
			node->h3_counter, node->h4_counter, node->h5_counter);
		else if (node->h4_counter != 0)
			sprintf(rest_string, ".%d.%d.%d", node->h2_counter,
			node->h3_counter, node->h4_counter);
		else if (node->h3_counter != 0)
			sprintf(rest_string, ".%d.%d", node->h2_counter,
			node->h3_counter);
		else if (node->h2_counter != 0)
			sprintf(rest_string, ".%d", node->h2_counter);
		strcat(out_string, rest_string);
		
	}
	else if (node->entry_type == TABLE) {
		if (document_type == MAJOR_SECTIONED) {
			sprintf(rest_string, "-%d", node->table_number);
			strcat(out_string, rest_string);
		}
		else
			sprintf(out_string, "%d", node->table_number);
	}
	else if (node->entry_type == FIGURE) {
		if (document_type == MAJOR_SECTIONED) {
			sprintf(rest_string, "-%d", node->figure_number);
			strcat(out_string, rest_string);
		}
		else
			sprintf(out_string, "%d", node->figure_number);
	}
	puts(out_string);
}
