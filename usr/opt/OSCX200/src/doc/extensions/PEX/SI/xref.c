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
/* $XConsortium: xref.c,v 5.1 91/02/16 09:45:52 rws Exp $ */

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
struct counters {
	int	levels[5];	/*  Chapter Level Counter  */
	int	table_number;	/*  Table Number  */
	int	figure_number;	/*  Figure Number  */
}	counters = {{0, 0, 0, 0, 0}, 0, 0};

int	outline_file_read = FALSE;	/*  TRUE if outline file(s) supplied  */
int	appendix_seen = FALSE;	/*  TRUE when appendix counting  */
int	start_chapter = 1;	/*  Starting Chapter Number  */
int	start_appendix = 1;	/*  Starting Appendix Number  */
int	plain_preprocessor = FALSE;	/* plain = don't source files */

main(argc, argv)
int	argc;
char	*argv[];
{
	int	file_number;
	int	ap_ix;


	command_name = argv[0];
	argc--;
	argv++;
	root = NULL;

	if (argc < 1) {
		fprintf(stderr, "Usage: %s [-sbegin] [-o outline] filename ...\n", command_name);
		exit(1);
	}
	while (argc > 0 && argv[0][0] == '-') {
					/*  look for starting chapter or appendix  */
		if (argv[0][1] == 's') {
			if (isdigit(argv[0][2]))
				start_chapter = atoi(&argv[0][2]);
			else if (isalpha(argv[0][2])) {
				start_appendix = 0;
				for (ap_ix = 2; isalpha(argv[0][ap_ix]); ap_ix++) {
					if (isupper(argv[0][ap_ix]))
						start_appendix = 26*start_appendix
						+ argv[0][ap_ix] - 'A' + 1;
					else
						start_appendix = 26*start_appendix
						+ toupper(argv[0][ap_ix]) - 'A' + 1;
					/*  outline file  */
				}
			}
		} else if (argv[0][1] == 'o') {
			current_filename = argv[1];
			process_current_file(current_filename, GATHER_REFERENCES);
			outline_file_read = TRUE;
			argc--;
			argv++;
		} else if (argv[0][1] == 'p') {
			plain_preprocessor = TRUE;
		} else {
			fprintf(stderr, "%s: unknown option -%c\n",
				command_name, argv[0][1]);
			exit(1);
		}
		argc--;
		argv++;
	}

	if (argc < 1) {
		fprintf(stderr, "Usage: %s [-sbegin] [-o outline] filename ...\n", command_name);
		exit(1);
	}

	if (!outline_file_read) {
		for (file_number = 0;  file_number < argc;  file_number++) {
			current_filename = argv[file_number];
			process_current_file(current_filename, GATHER_REFERENCES);
		}
	}
	for (file_number = 0;  file_number < argc;  file_number++) {
		current_filename = argv[file_number];
		process_current_file(current_filename, SUBSTITUTE_REFERENCES);
	}
#ifdef DEBUG
	dump_codewords(root);
#endif

}

process_current_file(filename, phase)
	char	*filename;
	int	phase;
{
	FILE	*current_file;
	char	line[MAXLINE];
	char	new_filename[BUFSIZ];

	if ((current_file = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "%s: cannot access %s\n", command_name, filename);
		exit(1);
	}
	line_number = 0;
	while (fgets(line, sizeof(line), current_file) != NULL) {
		line_number++;
		if (source_another_file(line, new_filename)) {
			process_current_file(new_filename, phase);
		} else {
		if (phase == GATHER_REFERENCES)
			build_tree(line);
		else
			fill_in_references(line);
		}
	}
	fclose(current_file);
}

source_another_file(line, filename)
	char	*line;
	char	*filename;
{

	int	char_p;
	char	*file_pos;

	char_p = 0;
	if (plain_preprocessor == TRUE)
		return(FALSE);
	if (line[char_p] == '\0' || line[char_p] == '\n')
		return(FALSE);
	if (line[char_p++] != '.')
		return(FALSE);
	while (line[char_p] == ' ' || line[char_p] == '\t')
		char_p++;
	if (line[char_p] == '\0' || line[char_p] == '\n')
		return(FALSE);
	if (line[char_p] != 's')
		return(FALSE);
	char_p++;
	if (line[char_p] == '\0' || line[char_p] == '\n')
		return(FALSE);
	if (line[char_p] != 'o')
		return(FALSE);
	char_p++;
	while (line[char_p] == ' ' || line[char_p] == '\t')
		char_p++;
	if (line[char_p] == '\0' || line[char_p] == '\n')
		return(FALSE);
	file_pos = filename;
	while (line[char_p] != '\n' && line[char_p] != ' ' && line[char_p] != '\t')
		*file_pos++ = line[char_p++];
	*file_pos = '\0';
	if (file_pos == filename)
		return(FALSE);
	else
		return(TRUE);
}

build_tree(line)
	char	*line;
{
	int	char_p;
	int	entry_type;
	struct codeword_entry	*node;

	char_p = 0;
				/*  Obtain type of codeword entry  */
				/*  HEADING or TABLE or FIGURE  */
	if ((entry_type = get_codeword_type(line, &char_p)) == 0)
		return;

				/*  Read stuff from the line  */
	if ((node = build_codeword_entry(line, &char_p, entry_type, &counters)) == NULL)
		return;

	node->entry_type = entry_type;

				/*  Insert codeword entry into the tree  */
	if (root == NULL)
		root = node;
	else
		node = insert_codeword_entry(root, node);
}

fill_in_references(line)
	char	*line;
{
	int	char_p;
	char	out_line[MAXLINE];

	char_p = 0;
					/*  Decide if this line is a cross reference  */
	if (try_reference_line(line, &char_p) == FALSE) {
		strcpy(out_line, line);
		fputs(out_line, stdout);
		return;
	}
	substitute_references(line, &char_p);
}

/*
 * Read codeword entry from the codeword file.  Build a new codeword entry.
 */
struct	codeword_entry *
build_codeword_entry(line, char_p, entry_type, counters)
	char	*line;
	int	*char_p;
	int	entry_type;
	struct	counters *counters;
{
	struct codeword_entry	*node;
	char	current_token[MAXLINE];
	char	*field;

	int	clear_level,  i;


	node = new(struct codeword_entry);
	node->entry_type = entry_type;
				/*  If this is a heading we must  */
				/*  set up the various counters  */
	if (entry_type == HEADING) {
		get_token(line, char_p, current_token, SKIP_SPACES);
					/*  Chapter Level  */
		if (strcmp(current_token, "C") == 0 || strcmp(current_token, "1") == 0) {
			node->appendix = FALSE;
			if (strcmp(current_token, "1") == 0)
				document_type = MINOR_SECTIONED;
			else {
				document_type = MAJOR_SECTIONED;
				counters->table_number = 0;
				counters->figure_number = 0;
			}
			if (start_chapter == 0)
				counters->levels[0]++;
			else {
				counters->levels[0] = start_chapter;
				start_chapter = 0;
			}
			clear_level = 1;
		}
						/*  Appendix Level  */
		else if (strcmp(current_token, "A") == 0 || strcmp(current_token, "PA") == 0) {
			node->appendix = TRUE;
			if (strcmp(current_token, "PA") == 0)
				document_type = MINOR_SECTIONED;
			else {
				document_type = MAJOR_SECTIONED;
				counters->table_number = 0;
				counters->figure_number = 0;
			}
			if (appendix_seen)
				counters->levels[0]++;
			else {
				appendix_seen = TRUE;
				if (start_appendix == 0)
					counters->levels[0] = 1;
				else {
					counters->levels[0] = start_appendix;
					start_appendix = 0;
				}
			}
			clear_level = 1;
		}
						/*  Section Level  */
		else if (strcmp(current_token, "2") == 0) {
			counters->levels[1]++;
			clear_level = 2;
		}
						/*  SubSection Level  */
		else if (strcmp(current_token, "3") == 0) {
			counters->levels[2]++;
			clear_level = 3;
		}
						/*  Paragraph Level  */
		else if (strcmp(current_token, "4") == 0) {
			counters->levels[3]++;
			clear_level = 4;
		}
						/*  SubParagraph Level  */
		else if (strcmp(current_token, "5") == 0) {
			counters->levels[4]++;
			clear_level = 5;
		}
		for (i = clear_level;  i < 5;  i++)
			counters->levels[i] = 0;
	}
	if (entry_type == TABLE) {
		counters->table_number++;
	}
	if (entry_type == FIGURE) {
		counters->figure_number++;
	}
	if (entry_type == CROSSREF) {	/*  ignore for now  */
		free(node);
		return(NULL);
	}
				/*  Read the title  */
	get_token(line, char_p, current_token, SKIP_SPACES);
		node->title = strdup(current_token);
				/*  Read the codeword  */
	get_token(line, char_p, current_token, SKIP_SPACES);
	if (strcmp(current_token, "") == 0) {
		free(node);
		return(NULL);
	} else {
		node->codeword = strdup(current_token);
		node->appendix = appendix_seen;
		node->h1_counter = counters->levels[0];
		node->h2_counter = counters->levels[1];
		node->h3_counter = counters->levels[2];
		node->h4_counter = counters->levels[3];
		node->h5_counter = counters->levels[4];
		node->table_number = counters->table_number;
		node->figure_number = counters->figure_number;
		return(node);
	}
}
