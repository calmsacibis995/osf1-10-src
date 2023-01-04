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
 */

static char     *sccsid = "@(#)$RCSfile: devbook.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:52:40 $";
/* devbook.c */
/* This module contains the main routine of the ditroff to bookreader */
/* post processor. Here we accept and check all arguments and options */
/* responding appropriately. This module also contains the parser for */
/* the ditroff output. Individual routines in other modules process   */
/* the parsed data. Naturally, process shutdown occurs here.          */

#include <stdio.h>
#include "dvbk.h"


char    filename[256];
char    usage[] = "devbook: Usage \"devbook -V | [-d n] filename\"\n";
int	book_id;

int	first_ck_id     = -1;
int	first_topic_id  = -1;
int	copyright_ck_id = -1;

/* This is the debug level flag. It is bit significant */
/* bit 0 = book stuff		*/
/* bit 1 = topic stuff		*/
/* bit 2 = chunk stuff		*/
/* bit 3 = sub-chunk stuff	*/
/* bit 4 = word stuff		*/
/* bit 5 = hot-spot stuff	*/
int	debug;

/* data declarations to match tbr.h externs */
char	input_buf[512];
char	cur_word[256];	/* where we accumulate the current word.     */
			/* it gets moved to a data buffer when done. */
int	troff_scale;	/* resolution supplied by troff		     */
int	cur_x_tr, cur_x_br, cur_x_rags,
	cur_y_tr, cur_y_br, cur_y_rags,
	bas_y_tr;
	/* troff x,y is guaranteed to be current. The others are */
	/* updated as and when needed via scaling factors.       */

I_SPOT	spot_free = NULL, spot_active = NULL, spot_head = NULL,
  spot_hold = NULL, foot_hold = NULL, shad_active = NULL;
I_SYMBOL	sym_head = NULL, sym_free = NULL;
I_FONT	font_head = NULL, font_free = NULL, cur_br_font = NULL;
I_TOPIC	topic_head = NULL, free_topic = NULL;
I_CHUNK	chunk_head = NULL, free_chunk = NULL;
I_DIR   directory_head = NULL, free_directory = NULL,
        toc = NULL, dir_exe = NULL, dir_fig = NULL, dir_tab = NULL,
        dir_ind = NULL;
I_TOC_REF	toc_ent_head = NULL, toc_ent_last = NULL, toc_ref_free = NULL;
extern I_DIR	save_toc, save_exe, save_fig, save_tab;

int	dir_exe_used, dir_fig_used, dir_tab_used;
int	cur_font, cur_point, cur_tobook_index;
int	tobook_font_index;
int	book_id;

I_CHUNK	cur_chunk = NULL;
I_FONT  font_hold = NULL;
int	cur_word_index, cur_data_ptr, cur_data_max,
	cur_word_x_br, cur_word_y_br;
int	toc_ent_count = 0;
int	new_line_height,
        cur_topic_id;
char	*cur_data;

I_TOC_REF	ref_head, ref_free;

CHUNK	chunk_array[CHUNKS_NEEDED];
TOPIC	topic_array[TOPICS_NEEDED];

extern char	font_set;

extern char	*optarg;
extern int	optind, opterr;

main ( argc, argv )
char *argv[];
{	int	i, c;

/* expect an argument to define output file name */
/* -d n sets debug level */

	while ( ( c = getopt ( argc, argv, "Vd:" ) ) != EOF )
	{
		switch ( c )
		{
		      case 'V':
			fprintf ( stderr, "%s\n", sccsid );
			exit (0);
		      case 'd':
			debug = atoi ( optarg );
			break;
		      default:
			fprintf ( stderr, "%s", usage );
		}
	}
	
	if ( ( optind + 1 ) != argc )
	{	fprintf ( stderr, "%s", usage );
		exit ( 1 );
	}
	
	strcpy ( filename, argv[optind] );

	for ( i = 0; i < TOPICS_NEEDED; i++ )
	{
		topic_array[i].next = free_topic;
		free_topic = &topic_array[i];
	}
	
	for ( i = 0; i < CHUNKS_NEEDED; i++ )
	{
		chunk_array[i].next = free_chunk;
		free_chunk = &chunk_array[i];
	}
	
	while ( gets ( input_buf ) != NULL )
		parse_input ( strlen ( input_buf ) );

	close_output (0);
}


/* all exit paths come here. This gives us the opportunity to clean up */
/* any incomplete structures and close the book-file before exiting    */
close_output ( status )
{	int	vwi_stat;
	UNDEFSYM	*undef_symbol;
	
	flush_word ();
	output_chunk ();
	flush_chunk ();
	flush_topic ();

	if ( !book_id )
	{
		fprintf ( stderr, "devbook: No output produced\n" );
		exit ( status );
	}
		  
	flush_lmf ();
	process_index ();
	contents_page ();

	if ( copyright_ck_id == -1 )
	  VWI_BOOK_COPYRIGHT ( book_id, first_ck_id, first_topic_id );
	
	flush_directory ();
	flush_spots ( -1, spot_head );
	vwi_stat = VWI_BOOK_CLOSE ( book_id, &undef_symbol );
	if ( vwi_stat != 1 )
	  while ( undef_symbol != NULL )
	  {
		fprintf ( stderr, "devbook: BOOKWRITER can't find symbol %s\n",
			 undef_symbol->name );
		undef_symbol = undef_symbol->next;
	  }
	exit ( status );
} 

contents_page ()
{	char	entry[20];
	char	command[128];
	int	tmp, tmpdirid;

	if ( save_toc )
	  toc = save_toc;
	if ( save_exe )
	  dir_exe = save_exe;
	if ( save_fig )
	  dir_fig = save_fig;
	if ( save_tab )
	  dir_tab = save_tab;
	
	if ( toc == NULL )
	{
		VWI_DIRECTORY_CREATE ( book_id, "Table of Contents",
						  TOC_FLAG, &tmpdirid );
		tmp = 0;
		entry[0] = FTEXT$K_TEXT;
		entry[1] = 18;
		entry[2] = 2;
		entry[3] = 0;
		entry[4] = 91;
		entry[5] = 0;
		entry[6] = 2;
		entry[7] = 0;
		entry[8] = 0;
		entry[9] = 8;
		strcpy ( &entry[10], "CONTENTS" );

		VWI_DIRECTORY_ENTRY ( book_id, tmpdirid, 0, &tmp, 1, 1428,
				      169, entry, 18, CHUNK_FTEXT, "CONTENTS");

		tmp = first_ck_id;
		entry[1] = 20;
		entry[2] = 69;
		entry[3] = 0;
		entry[4] = 91;
		entry[9] = 10;
		strcpy ( &entry[10], "Title Page" );

		VWI_DIRECTORY_ENTRY ( book_id, tmpdirid, 1, &tmp, 1, 1360,
				      169, entry, 20, CHUNK_FTEXT, "Title Page" );
    
		VWI_DIRECTORY_CLOSE ( book_id, tmpdirid );
		return;
	}

/* toc was set so we have a real table of contents to process.   */
/* close the file, process it through ditroff, parse the output. */
/* Each entry should be bracketed by special characters so that  */
/* we can simply open the output from ditroff and call parse.    */

	fclose ( toc->tmp_file );

	sprintf ( command, "troff -Tb%c %s %s", font_set,
		 "/usr/share/lib/mu/toc.mac", toc->tmp_file_name );

	if ( dir_exe )
		fclose ( dir_exe->tmp_file );
	if ( dir_exe_used )
	{	strcat ( command, " " );
		strcat ( command, dir_exe->tmp_file_name );
	}
	if ( dir_fig )
		fclose ( dir_fig->tmp_file );
	if ( dir_fig_used )
	{	strcat ( command, " " );
		strcat ( command, dir_fig->tmp_file_name );
	}
	if ( dir_tab )
		fclose ( dir_tab->tmp_file );
	if ( dir_tab_used )
	{	strcat ( command, " " );
		strcat ( command, dir_tab->tmp_file_name );
	}
	strcat ( command, " > " );
	strcat ( command, toc->tmp_file_name );
	strcat ( command, ".out" );
	
	system ( command );
	
	sprintf ( command, "%s.out", toc->tmp_file_name );
	toc->tmp_file = fopen ( command, "r" );
	
	initialize_topic ( STANDARD );	/* for initialize chunk */
	initialize_chunk ();	/* for the directory entries */
	
	while ( fgets ( input_buf, 256, toc->tmp_file ) != NULL )
		parse_directory_input ( strlen ( input_buf ) );

	fclose ( toc->tmp_file );

/* close the directories and */
/* remove all of the temporary files */
	VWI_DIRECTORY_CLOSE ( book_id, toc->id );
	sprintf ( command, "rm -f %s*", toc->tmp_file_name );	
	if ( dir_exe )
	{	strcat ( command, " " );
		strcat ( command, dir_exe->tmp_file_name );
		if ( dir_exe_used )
		  VWI_DIRECTORY_CLOSE ( book_id, dir_exe->id );
	}
	if ( dir_fig )
	{	strcat ( command, " " );
		strcat ( command, dir_fig->tmp_file_name );
		if ( dir_fig_used )
		  VWI_DIRECTORY_CLOSE ( book_id, dir_fig->id );
	}
	if ( dir_tab )
	{	strcat ( command, " " );
		strcat ( command, dir_tab->tmp_file_name );
		if ( dir_tab_used )
		  VWI_DIRECTORY_CLOSE ( book_id, dir_tab->id );
	}
	system ( command );

	
}

process_index ()
{	char	command[132];
	
	if ( !dir_ind )
	  return;
	
	fclose ( dir_ind->tmp_file );

/* In here, we want to sort the file, munge in into sections and */
/* format it. We want to keep this separate from toc work in case */
/* there isn't one. */

	sprintf ( command, "sort -u -t\\\" -f +3 +0 -1 %s > %s.out",
		  dir_ind->tmp_file_name, dir_ind->tmp_file_name );
	system ( command );

	sprintf ( command, "conind %s.out > %s.n1",
		  dir_ind->tmp_file_name, dir_ind->tmp_file_name );

	system ( command );
	
	sprintf ( command, "troff -Tb%c /usr/share/lib/mu/ndx.mac %s.n1 > %s.nr",
		  font_set, dir_ind->tmp_file_name, dir_ind->tmp_file_name );

	system ( command );
	
	sprintf ( command, "%s.nr", dir_ind->tmp_file_name );
	dir_ind->tmp_file = fopen ( command, "r" );
	
	initialize_topic ( STANDARD );	/* for initialize chunk */
	initialize_chunk ();	/* for the directory entries */
	
	while ( fgets ( input_buf, 256, dir_ind->tmp_file ) != NULL )
		parse_directory_input ( strlen ( input_buf ) );

	fclose ( dir_ind->tmp_file );

	index_flush ();
	
	sprintf ( command, "rm -f %s*", dir_ind->tmp_file_name );
	system ( command );

	VWI_DIRECTORY_CLOSE ( book_id, dir_ind->id );
}	
