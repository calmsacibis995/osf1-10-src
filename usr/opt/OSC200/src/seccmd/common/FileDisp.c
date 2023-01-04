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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: FileDisp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:59:36 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#ifdef SEC_BASE

/*
	filename:
		FileDisp.c - load file for display
	
	copyright:
		Copyright 1990 SecureWare Inc.
		ALL RIGHTS RESERVED

*/
		
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include "IFdefs.h"
#include "Utils.h"

/* Definitions */

/* External routines */
extern int
	eaccess ();

extern FILE	
	*popen();


static char 
	**msg,
	*msg_text;

static char
	*filbuf;	/* contents of entire file */
		
static long
	file_size;


int 
FileDisplayOpen(fname, title, err_class, err_index, err_chrptr, err_tag)
	char        *fname;
	char        *title;
	char        **err_class;
	int         err_index;
	char        *err_chrptr;
	int         err_tag;
{
	int         i,
			j,
			k,
			kk,
			nrows;
	char        lstring[255];
	struct stat sbuf;
	FILE        *fp;
	IFString    *list_strings;

	
	/* Load message text if not already loaded */
	if (! msg)
		LoadMessage("msg_filedisplay", &msg, &msg_text);

	/* set up for & open file */

	if (eaccess(fname, 4) < 0) {
		goto file_error;
	}

	if (stat(fname, &sbuf)) {
		goto file_error;
	}
	file_size = sbuf.st_size;
	
	fp = fopen(fname, "r");
	if (! fp) {
		goto file_error;
	}

	/* allocate memory for contents & load file */

	filbuf = Malloc(file_size + 1);
	if (! filbuf)
		MemoryError();	/* Dies */
	
	if (fread(filbuf, file_size, 1, fp) != 1) {
		fclose(fp);
		Free(filbuf);
		goto file_error;
	}
	fclose(fp);
	filbuf[file_size] = '\0';

	/* scan file contents to determine number rows (count line feeds) */
	nrows = 0;
	for (j = 0; j < file_size; j++) 
		if (filbuf[j] == '\n') 
			nrows++;    
	
	list_strings = (IFString *) Calloc(sizeof(IFString), nrows);
	if (! list_strings)
		MemoryError();	/* dies */

	/* break file at line feeds and convert each line to IFString */
	/* converting tabs to spaces */

	for (i = j = k = 0; j < file_size; j++) {
		if (filbuf[j] == '\n') {		/* newline found */
			for (; k < 77; k++)	/* coerce line lenth to 78 */
				lstring[k] = ' ';        
			lstring[k] = '\0';	/* replace newline with null */
			list_strings[i] = IFStringCreate(lstring, charset); 
			if (! list_strings[i])
				MemoryError();	/* Dies */
			k = 0;			/* new empty target string */
			i++;			/* increment string index */
		} 
		else if (filbuf[j] == '\t') {	/* tab character found */
			kk = ((k/8)+1)*8;	/* find next tab stop */
			for (; k < kk; k++)	/* pad with spaces */
				lstring[k] = ' ';
		}
		else {
			lstring[k] = filbuf[j];	/* copy other characters */
			k++;
		}    
	}       
	IFHelpPopup(nrows, list_strings);
	
	Free(filbuf);
	if (list_strings) {
		for (i = 0; i < nrows; i++)
			IFStringFree(list_strings[i]);
		Free(list_strings);
		list_strings = NULL;
	}
	return TRUE;

file_error:
	ErrorMessageOpen(err_tag, err_class, err_index, err_chrptr);  
	return FALSE;
}   

#endif /* SEC_BASE */
