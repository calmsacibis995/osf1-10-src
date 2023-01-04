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
/* $XConsortium: sub_refs.c,v 5.1 91/02/16 09:45:54 rws Exp $ */

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
#include "xref.h"

struct codeword_entry	*root;


substitute_references(line, char_p)
	char	line[MAXLINE];
	int	*char_p;
{
	int	reference_type;
	char	current_token[MAXLINE];
	char	temp_token[MAXLINE];
	int	token_type;
	struct codeword_entry	*node;

	for (token_type = get_token(line, char_p, current_token, SKIP_SPACES);
		 token_type != ENDOFLINE_TOKEN;
		 token_type = get_token(line, char_p, current_token, DONT_SKIP_SPACES)) {

		if (token_type != DELIMITER_TOKEN) {
			printf("%s", current_token);
			continue;
		}
					 /*  Look for @ sign  */
		if (strcmp(current_token, "@") != 0) {
			printf("%s", current_token);
			continue;
		}
					/*  Cater to doubled up @ signs  */
		token_type = get_token(line, char_p,
					current_token, DONT_SKIP_SPACES);
		if (token_type == ENDOFLINE_TOKEN)
			break;
		if (strcmp(current_token, "@") == 0) {
			printf("%s", current_token);
			continue;
		}
		if (token_type != ALPHA_TOKEN) {
			printf("@%s", current_token);
			continue;
		}
		if (SCComp(current_token, "numberof") == TRUE)
			reference_type = NUMBER;
		else if (SCComp(current_token, "titleof") == TRUE)
			reference_type = TITLE;
		else if (SCComp(current_token, "pagenumber") == TRUE)
			reference_type = TITLE;
		else {
			fprintf(stderr, "%s: Unknown command %s at line %d of file %s\n",
			command_name, current_token, line_number, current_filename);
			printf("@%s", current_token);
			continue;
		}
		token_type = get_token(line, char_p, current_token, DONT_SKIP_SPACES);
		if (token_type != DELIMITER_TOKEN) {
			printf("%s", current_token);
			continue;
		}
		if (strcmp(current_token, "(") != 0) {
			if (reference_type == NUMBER)
				printf("@NumberOf%s", current_token);
			else
				printf("@TitleOf%s", current_token);
			continue;
		}
		token_type = get_token(line, char_p, current_token, DONT_SKIP_SPACES);
		if (token_type != ALPHA_TOKEN) {
			if (reference_type == NUMBER)
				printf("@NumberOf(%s", current_token);
			else
				printf("@TitleOf(%s", current_token);
			continue;
		}
					/*  Locate codeword in database  */
		if ((node = locate_codeword_entry(root, current_token)) == NULL) {
			fprintf(stderr, "%s: Unknown codeword %s at line %d of file %s\n",
			command_name, current_token, line_number, current_filename);
			if (reference_type == NUMBER)
				printf("@NumberOf(%s", current_token);
			else
				printf("@TitleOf(%s", current_token);
			continue;
		}
		strcpy(temp_token, current_token);
		token_type = get_token(line, char_p, current_token, DONT_SKIP_SPACES);
		if (token_type != DELIMITER_TOKEN) {
			if (reference_type == NUMBER)
				printf("@NumberOf(%s%s", temp_token, current_token);
			else
				printf("@TitleOf(%s%s", temp_token, current_token);
			continue;
		}
		if (strcmp(current_token, ")") != 0) {
			if (reference_type == NUMBER)
				printf("@NumberOf(%s%s", temp_token, current_token);
			else
				printf("@TitleOf(%s%s", temp_token, current_token);
			continue;
		}
		print_reference(node, reference_type);
	}
	putchar('\n');
}


int
try_reference_line(line, char_p)
	char	*line;
	int	char_p;
{
	char	current_token[MAXLINE];
	int	token_type;

					/*  Look for .  */
	token_type = get_token(line, char_p, current_token, SKIP_SPACES);
	if (token_type != DELIMITER_TOKEN)
		return (FALSE);
	if (strcmp(current_token, ".") != 0)
		return (FALSE);
					/*  Look for XR  */
	token_type = get_token(line, char_p, current_token, DONT_SKIP_SPACES);
	if (token_type != ALPHA_TOKEN)
		return (FALSE);
	if (strcmp(current_token, "XR") != 0)
		return (FALSE);

	return (TRUE);
}
