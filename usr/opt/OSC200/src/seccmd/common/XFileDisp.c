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
static char	*sccsid = "@(#)$RCSfile: XFileDisp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:02:01 $";
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
		XFileDisp.c
	
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		Copyright (c) 1989-1990 MetaMedia Inc.
		ALL RIGHTS RESERVED

	function:
		utility to display a file
  
	entry points:
		FileDisplayStart()
		FileDisplayOpen(fname,title,err_class,err_index,err_chrptr,err_tag)
		FileDisplayClose()
		FileDisplayStop()
		PrintFile(cp, file_size) 
*/
		
/* Common C include files */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>

/* Role program include file */
#include <XMain.h>

/* Definitions */

/* External routines */
extern int
		eaccess ();

extern FILE	
		*popen();

extern char
		*Malloc();

/* Local routines */
static void 
		MakeWidgets(),
		FileDisplayCallback(),
		FileDisplayClose(),
		PrintFile();

/* Local variables */
static int
		open_flag;
		
static Widget
		form_widget,
		scrolled_window_widget,
		list_widget,
		title_widget;

static char 
		**msg,
		*msg_text;

static char
		*cp;
		
static long
		file_size;

void 
FileDisplayStart()
{
	open_flag = FALSE;
}

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
	XmString    xmstring,
				*list_xmstrings;
	Cardinal    n;
	Arg         args[20];

	/* Synchronize to prevent X from blowing up
	*  Eventually would be nice if we could strip this out -- very
	*  slow, but would be hard to test -- would need days of use in
	*  user environment in which we knew that everthing else was
	*  tight!     !!!! REMOVE WHEN KNOW SAFE -- ALSO REMOVE FROM ...CLOSE()
	*/
	XSynchronize(XtDisplay(main_shell),TRUE);
	
	WorkingOpen(main_shell);
	XtSetSensitive(mother_form, False);
	
	/* Load message text if not already loaded */
	if (! msg)
		LoadMessage("msg_filedisplay", &msg, &msg_text);

	/**********************************************************************/
	/* Load file first to check for problems before messing w/ windows    */
	/**********************************************************************/

	/* Test file for access rights */
	if (eaccess(fname, 4) < 0) {
file_error:
		WorkingClose();
		XtSetSensitive(mother_form, True);
		ErrorMessageOpen(err_tag, err_class, err_index, err_chrptr);  
		return FALSE;
	}

	/* Calculate size of file */
	if (stat(fname, &sbuf))
		goto file_error;
	file_size = sbuf.st_size;
	
	/* Open file */
	fp = fopen(fname, "r");
	if (! fp)
		goto file_error;

	/* Malloc memory for file contents */
	cp = Malloc(file_size + 1);
	if (! cp)
		MemoryError();
		/* Dies */
	
	/* Read in file */
	if (fread(cp, file_size, 1, fp) != 1) {
		fclose(fp);
		Free(cp);
		goto file_error;
	}
	fclose(fp);
	cp[file_size] = '\0';

	/* scan file contents to determine number rows (count line feeds) */
	nrows = 0;
	for (j = 0; j < file_size; j++) 
		if (cp[j] == '\n') 
			nrows++;    
	
	/* allocate pointers to XmStrings */
	list_xmstrings = (XmString *) Malloc(sizeof(XmString) * nrows);
	if (! list_xmstrings)
	MemoryError();

	/* break file at line feeds and convert each line to XmString */
	/* also convert tabs to spaces - XmStringCreate has a problem with them */
	for (i = j = k = 0; j < file_size; j++) {
		if (cp[j] == '\n') {      /* line feed found */
			for (; k < 77; k++)   /* coerce line lenth to 78 */
				lstring[k] = ' ';        
			lstring[k] = '\0';    /* replace line feed with null */
			list_xmstrings[i] = XmStringCreate(lstring, charset); 
			if (! list_xmstrings[i])
				MemoryError();
			k = 0;                /* new empty target string */
			i++;                  /* increment xmstring index */
		} 
		else if (cp[j] == '\t') { /* tab character found */
			kk = ((k/8)+1)*8;     /* note integer division: discard fraction */
			for (; k < kk; k++)   /* kk = next col which is multiple of 8 */
				lstring[k] = ' '; /* pad with spaces to column kk */
		}
		else {
			lstring[k] = cp[j];   /* copy other characters */
			k++;
		}    
	}       
  
	if (! open_flag) {
		MakeWidgets();
		open_flag = TRUE;
	}    

	n = 0;
	XtSetArg(args[n], XmNitems,             list_xmstrings); n++;
	XtSetArg(args[n], XmNitemCount,                  nrows); n++;
	XtSetValues(list_widget, args, n);
		
	/* Set form title */
	xmstring = XmStringCreate(title, charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString,             xmstring); n++;
	XtSetValues(title_widget, args, n);
	XmStringFree(xmstring);

	XtManageChild(form_widget);
	
	Free(cp);
	if (list_xmstrings) {
		for (i = 0; i < nrows; i++)
			XmStringFree(list_xmstrings[i]);
		Free(list_xmstrings);
		list_xmstrings = NULL;
	}
	WorkingClose();
}   

static void
FileDisplayClose() 
{
	XtSetSensitive(mother_form, True);
	XtUnmanageChild(form_widget);
	if (save_memory) {
		if (open_flag) {
			XtDestroyWidget(form_widget);
			open_flag = FALSE;
		}
	}

	/* Turn off synchronize */
	XSynchronize(XtDisplay(main_shell), FALSE);
}

void 
FileDisplayStop()
{
	if (open_flag) {
		XtDestroyWidget(form_widget);
		open_flag = FALSE;
	}
}

static void 
FileDisplayCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmListCallbackStruct    *info;
{
	if (! strcmp(ptr, "Print")) {
		/* user pressed "Print" button */
		WorkingOpen(main_shell);       /* cursor to clock */
		PrintFile(cp, file_size);              /* Do print routine */
		WorkingClose();
	}
	FileDisplayClose();
}

static void 
MakeWidgets() {
}

static void        
PrintFile(buf, file_size)
	char   *buf;
	long   file_size;
{
	FILE   *pfp;
	
	pfp = popen("lp -s", "w");
	if (pfp != (FILE *) 0) {
		fwrite(buf, file_size, 1, pfp);
		pclose(pfp);
	}
	else
		ErrorMessageOpen(3500, msg, 2, NULL);
}
#endif /* SEC_BASE */
