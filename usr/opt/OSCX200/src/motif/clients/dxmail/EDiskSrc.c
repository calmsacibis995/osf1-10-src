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
static char rcs_id[] = "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxmail/EDiskSrc.c,v 1.1.6.10 1994/01/11 22:06:08 Michael_Igoe Exp $";
#endif

/*
 *                     Copyright (c) 1987, 1991 by
 *              Digital Equipment Corporation, Maynard, MA
 *                      All rights reserved.
 *
 *   This software is furnished under a license and may be used and
 *   copied  only  in accordance with the terms of such license and
 *   with the  inclusion  of  the  above  copyright  notice.   This
 *   software  or  any  other copies thereof may not be provided or
 *   otherwise made available to any other person.  No title to and
 *   ownership of the software is hereby transferred.
 *
 *   The information in this software is subject to change  without
 *   notice  and should not be construed as a commitment by Digital
 *   Equipment Corporation.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 * $Log: EDiskSrc.c,v $
 * Revision 1.1.6.10  1994/01/11  22:06:08  Michael_Igoe
 * 	Fix heap corruption in XtCreateEDiskSource.
 * 	[1994/01/11  19:28:46  Michael_Igoe]
 *
 * Revision 1.1.8.2  1994/01/11  19:28:46  Michael_Igoe
 * 	Fix heap corruption in XtCreateEDiskSource.
 *
 * Revision 1.1.6.9  1993/12/21  14:34:26  Michael_Igoe
 * 	Changes to eliminate uneeded shared source on ddifheaders.
 * 	[1993/12/20  13:58:03  Michael_Igoe]
 *
 * 	Add "vw" to "XtEDiskSave" funcs and fix args to "EDiskReplace".
 * 	[1993/12/17  21:00:47  Michael_Igoe]
 *
 * Revision 1.1.10.3  1993/12/20  13:58:03  Michael_Igoe
 * 	Changes to eliminate uneeded shared source on ddifheaders.
 *
 * Revision 1.1.10.2  1993/12/17  21:00:47  Michael_Igoe
 * 	Add "vw" to "XtEDiskSave" funcs and fix args to "EDiskReplace".
 *
 * Revision 1.1.6.8  1993/12/15  23:58:36  Michael_Igoe
 * 	    Use "XmTextGetString" instead of nonstandard "_XmStringSourceGetValue".
 * 	    in "XtEDiskMakeCheckpoint()".
 * 	[1993/11/24  16:54:51  Michael_Igoe]
 *
 * Revision 1.1.9.2  1993/11/24  16:54:51  Michael_Igoe
 * 	    Use "XmTextGetString" instead of nonstandard "_XmStringSourceGetValue".
 * 	    in "XtEDiskMakeCheckpoint()".
 *
 * Revision 1.1.6.7  1993/11/11  19:49:30  Michael_Igoe
 * 	 Change open existing file operation from O_APPEND to O_TRUNC ...
 * 	 in function "XtEDiskSaveAsFile()".
 * 	[1993/11/03  19:00:57  Michael_Igoe]
 *
 * Revision 1.1.8.2  1993/11/03  19:00:57  Michael_Igoe
 * 	 Change open existing file operation from O_APPEND to O_TRUNC ...
 * 	 in function "XtEDiskSaveAsFile()".
 *
 * Revision 1.1.6.6  1993/09/21  17:40:31  Adrienne_Snyder
 * 	Modified XtCreateEDiskSrc() to set the textwidgets pointer to the source
 * 	structure to the reallocated memory.
 * 	[1993/09/13  16:03:31  Adrienne_Snyder]
 *
 * Revision 1.1.8.2  1993/09/13  16:03:31  Adrienne_Snyder
 * 	Modified XtCreateEDiskSrc() to set the textwidgets pointer to the source
 * 	structure to the reallocated memory.
 *
 * Revision 1.1.6.5  1993/09/07  17:41:23  Adrienne_Snyder
 * 	Modified condition for reallocing source structure. So that it would not step on the source and clobber it.
 * 	[1993/09/02  19:36:43  Adrienne_Snyder]
 *
 * Revision 1.1.7.2  1993/09/02  19:36:43  Adrienne_Snyder
 * 	Modified condition for reallocing source structure. So that it would not step on the source and clobber it.
 *
 * Revision 1.1.6.3  1993/08/03  13:36:25  Adrienne_Snyder
 * 	Recover from bmerge code changes
 * 	[1993/08/03  13:24:11  Adrienne_Snyder]
 *
 * Revision 1.1.7.2  1993/08/03  13:24:11  Adrienne_Snyder
 * 	Recover from bmerge code changes
 *
 * Revision 1.1.2.3  1992/11/02  09:38:56  Aju_John
 * 	"extract - append changes"
 *
 * Revision 1.1.3.2  92/11/02  09:35:09  Aju_John
 * 	extract changes to append to file
 * 
 * Revision 1.1.2.2  92/06/26  11:42:09  Dave_Hill
 * 	first submittal to pool for Aju John, replaces former ../mail
 * 	[92/06/26  11:23:32  Dave_Hill]
 * 
 * Revision 1.1.1.2  92/06/26  11:23:32  Dave_Hill
 * 	first submittal to pool for Aju John, replaces former ../mail
 * 
 * Revision 1.2  91/12/30  12:48:20  devbld
 * Initial load of project
 * 
 * Revision 1.15  91/10/09  12:22:59  rmurphy
 * bl4: Only wrap text if wordwrap enabled
 * 
 * Revision 1.14  91/10/08  17:38:26  rmurphy
 * bl4: Save text past select point
 * 
 * Revision 1.13  91/10/07  20:26:59  rmurphy
 * bl4: Disable word wrap of postscript
 * 
 * Revision 1.12  91/10/04  19:34:42  rmurphy
 * bl4: Convert to SVN display
 * 
 * Revision 1.11  91/09/16  17:31:54  rmurphy
 * bl3mup3: Word-wrap according to text widget
 * 
 * Revision 1.10  91/08/21  05:06:05  rmurphy
 * bl3mup1: Change CurrentTime to XtLastTimestampProcessed
 * 
 * Revision 1.9  91/08/17  18:36:57  rmurphy
 * bl3mup1: Remove prop. notice
 * 
 * Revision 1.8  91/08/17  10:59:17  rmurphy
 * bl3mup1: Work around text widget hang
 * 
 * Revision 1.7  91/07/04  07:14:41  rmurphy
 * bl3: Correct source data backpointer
 * 
 * Revision 1.6  91/07/04  06:05:47  rmurphy
 * bl3: Fix crash on text de-selection
 * 
 * Revision 1.5  91/07/03  20:03:38  rmurphy
 * bl3: Correct handling of source records
 * 
 * Revision 1.3  91/06/14  11:32:54  rmurphy
 * bl3: Convert to Motif
 * 
 * Revision 1.2  90/10/30  09:35:47  murphy
 * Convert to Motif V1.1.1
 * 
 * Revision 1.1  90/10/16  17:26:19  samia
 * Initial revision
 * 
 */


#include <stdio.h>
#include <sys/types.h>  /* for stat() call */
#include <sys/stat.h>
#include <sys/file.h>

#include "decxmail.h"
#include <Xm/TextP.h>
/*
 * Redefinition of source record - extended to hold filename and callbacks
 */
typedef struct _EDiskSourceRec {
    struct _XmSourceDataRec *data;	/* Source-defined data (opaque type). */
    AddWidgetProc	AddWidget;	/* Normal source fields */
    CountLinesProc	CountLines;
    RemoveWidgetProc	RemoveWidget;
    ReadProc		ReadSource;
    ReplaceProc		Replace;
    ScanProc		Scan;
    GetSelectionProc	GetSelection;
    SetSelectionProc	SetSelection;
    ReplaceProc		origReplace;	/* Original replace */
    char		*filename;	/* Name of file used for this source */
    void 		(*cb_func)();	/* Function to call when a change is made. */
    char		*cb_param;	/* Parameter to pass to above function. */
    short 		changed;	/* If changes have not been written to disk. */
    short 		everchanged;	/* If changes have ever been made. */
    short		checkpointed;	/* Filename to store checkpoints. */
    short 		checkpointchange;/* TRUE if we've changed since checkpointed. */
} EDiskSourceRec;
typedef struct _EDiskSourceRec *EDiskSource;

static Widget NullrText = (Widget) NULL;
static XmTextSource NullrTextSource = (XmTextSource) NULL;

/*
static XmTextStatus EDiskReplace(Source, event, startPos, endPos, block)
EDiskSource source;
*/
static XmTextStatus EDiskReplace(widget, event, startPos, endPos, block,callback_flags)
XmTextWidget widget;
XEvent	*event;
XmTextPosition *startPos, *endPos;
XmTextBlock block;
#if NeedWidePrototypes
    int     callback_flags;
#else
    Boolean callback_flags;
#endif

{
    XmTextStatus status;
    EDiskSource source= (EDiskSource) widget->text.source;
    XmSourceData data = source->data;
	int i;

/* *\
  *    if (source == NULL){
  *	source = (EDiskSource)widget;
  *	widget = NULL;
  *	data = source->data;
  *    for (i=0 ; i<data->numwidgets ; i++) {
  *	if (data->widgets[i].source == source) {
  *		widget = data->widgets[i];
  *	}
  *	}	
  *    }
  *
  *   if (widget == NULL ) {
  *       DEBUG(("widget is NULL not there "));
  *   }
\* */
    status = (*source->origReplace)(widget, event, startPos, endPos, block,True);
    if (status != EditDone)
	return status;
    source->changed = source->everchanged = source->checkpointchange = TRUE;
    if (source->cb_func) (*source->cb_func)(source->cb_param);
    return status;
}
static void EDiskRemoveWidget(source, widget)
EDiskSource source;
XmTextWidget widget;
{
    XmSourceData data = source->data;
    int i;
    for (i=0 ; i<data->numwidgets ; i++) {
	if (data->widgets[i] == widget) {
            XmTextPosition left, right;
            Boolean had_selection = False;
	    if (data->hasselection) {
                (*source->GetSelection)(source, &left, &right);
                (*source->SetSelection)(source, 1, -999, 
				XtLastTimestampProcessed(theDisplay));
                had_selection = True;
            }

	    data->numwidgets--;
	    data->widgets[i] = data->widgets[data->numwidgets];
	    if (i == 0 && data->numwidgets > 0 && had_selection)
	       (*source->SetSelection)(source, left, right, 
			XtLastTimestampProcessed(theDisplay));
	    return;
	}
    }
}
/*  routine to replace the create one without changeing it.
*/
XmTextSource
XtCreateEDiskSource(fname, editable, scrn, tw)
char	*fname;
Boolean editable;
Scrn   scrn;
Widget tw;
{
    int		fd, rd;
    static char	*ptr = NULL;
    static int	ptrlen = 0;
    struct stat	buf;
    EDiskSource	src;
    XmTextBlockRec text;
    XmTextPosition pos;
    int max_length;
    XmTextWidget tmptw;
    static Arg textargs[] = {
        {XmNeditable, (XtArgVal) True},
        {XmNsensitive, (XtArgVal) True},
        {XmNeditMode, (XtArgVal) XmMULTI_LINE_EDIT},
        {XmNpendingDelete, (XtArgVal) True},
    };
					/* added check since Initialize World
					   calls with NULL ptr for scrn
					*/
/* *    if (scrn != NULL)
 * *    	tw=(Widget)scrn->viewwidget;
 * *     else 
 * *	tw=(Widget) NULL;
 * */

    fd = OpenAndCheck(fname, O_RDONLY, 0);
    if (fd == -1) {
	buf.st_size = 0;
    } else {
	stat (fname, &buf);
    }
    if (buf.st_size <= 0) {
	DEBUG(("Empty source '%s' - setting to the empty string.\n",fname));
	if (ptrlen == 0) {
	    ptr = XtMalloc(1024);
	    ptrlen = 1024;
	    *ptr = '\0';
	}
    } else {
	if (ptrlen == 0) {
	    ptr = XtMalloc((unsigned)buf.st_size + 2);
	    ptrlen = buf.st_size;
	} else {
	    if (buf.st_size > ptrlen) {
		ptr = XtRealloc(ptr,(unsigned)buf.st_size + 2);
		ptrlen = buf.st_size;
	    }
	}
	rd = read(fd, ptr, buf.st_size);
	DEBUG(("@ XtCreateEDiskSource('%s', editable) -> %d (%d)\n",fname,rd,buf.st_size));
	if (rd != buf.st_size)
	    DEBUG(("Failed to read all of the file '%s'!\n",fname));
	ptr[rd] = '\0';
    }
    if (fd != -1) myclose(fd);

    if (buf.st_size == 0) {

        if (NullrText == (Widget) NULL)
        {
            NullrText = XmCreateText(toplevel, "NullrText",textargs,
			XtNumber(textargs));
        }
						/* save the text widget address
						*/
        if ( tw == NULL )
            tw = NullrText;
    	tmptw = (XmTextWidget)NullrText;
        src = (EDiskSource) XmTextGetSource(NullrText);

     } else {

        if ( scrn != NULL && tw == NULL )
            tw = (Widget)scrn->viewwidget;      /* Should NOT get here! */

						/* save the text widget address
						*/


/* *    Code to FIX Multiple READ Screens. .....
 * *    Replaces next two lines below.
 * *
 * *        tmptw = (XmTextWidget) XmCreateText( toplevel, "NullrText",textargs,
 * *                              XtNumber(textargs) );
 * *        src = (EDiskSource) XmTextGetSource( tmptw );
 * *        pos = 0;
 * *        XmTextSetSource( tw, src, pos, pos );
 * *        XmTextSetSource( tmptw, NullrTextSource, pos, pos );
 * *        XtDestroyWidget( (Widget) tmptw );
 * */
        tmptw = (XmTextWidget)tw;
        src = (EDiskSource) XmTextGetSource(tw);
        XmTextSetString(tw,ptr);
        XmTextSetEditable(tw,editable);
     }


    if (src->data->numwidgets <= 1 && (src->Replace != EDiskReplace) ) {

    src = (EDiskSource) XtRealloc((char *)src, sizeof(EDiskSourceRec));
					
						/* reset the source in the 
						   text if realloc used new
						   memory
						*/
    tmptw = (XmTextWidget)tw;
    tmptw->text.source = (XmTextSource) src;

    src->data->source = (XmTextSource) src;
    src->origReplace = src->Replace;

    src->RemoveWidget = EDiskRemoveWidget;
    src->Replace = EDiskReplace;
    src->filename = XtNewString(fname);
    src->changed = src->everchanged = FALSE;
    src->checkpointed = src->checkpointchange = FALSE;
    src->cb_func = NULL;
    src->cb_param = NULL;
    }

    if ( NullrTextSource == (XmTextSource)NULL )
         NullrTextSource = (XmTextSource) src;

    return((XmTextSource) src);
}

/*
 * Create a new source from the given string.  However, it acts like an
 * ordinary disk source, except it can't be saved...
 */
XmTextSource
XtCreateEDiskSourceFromString(str, editable, scrn, tw)
char	*str;
Boolean	editable;
Scrn scrn;
Widget tw;
{
EDiskSource	src;
XmTextWidget tmptw;

    src = (EDiskSource) XmTextGetSource(tw);
    XmTextSetString(tw,str);
    tmptw = (XmTextWidget)tw;
    src = (EDiskSource) XtRealloc((char *)src, sizeof(EDiskSourceRec));
   					/* reset source struct in widget */
    tmptw->text.source = (XmTextSource) src;

    src->data->source = (XmTextSource) src;
    if ( src->RemoveWidget !=EDiskRemoveWidget && src->Replace != EDiskReplace)
        src->origReplace = src->Replace;

    src->RemoveWidget = EDiskRemoveWidget;
/*
 *   if (src->origReplace == NULL)
 * ajs - fix for trashing source struct..
 */
    src->Replace = EDiskReplace;
    src->filename = NULL;
    src->cb_func = NULL;
    src->cb_param = NULL;
    src->changed = src->everchanged = FALSE;
    src->checkpointed = src->checkpointchange = FALSE;
    return((XmTextSource) src);

} 

XtDestroyEDiskSource(source)
EDiskSource	source;
{
    char str[500];
    DEBUG(("  Destroying ptr=%x, value=%x\n",source->data->ptr,source->data->value));

    if (source != NULL) {
	if (source->checkpointed) {
	    (void) sprintf(str, "%s.CKP", source->filename);
	    (void) unlink(str);
	    source->checkpointed = FALSE;
	}
	XtFree((char *)source->data->ptr);
	source->data->ptr = NULL;
	XtFree((char *)source->data->value);
	source->data->value = NULL;
	XtFree((char *)source->data->widgets);
	source->data->widgets = NULL;
	XtFree((char *)source->data);
	source->data = NULL;
	if (source->filename)
	    XtFree((char *)source->filename);
	source->filename = NULL;
	XtFree((char *)source);
	source = NULL;
    }
}

char *
XtEDiskGetValue(source)
EDiskSource	source;
{
    if (source == NULL) return NULL;
    return(_XmStringSourceGetValue((XmTextSource) source));
}

Boolean
XtEDiskGetEditable(source)
EDiskSource	source;
{
    if (source == NULL) return False;
    return (_XmStringSourceGetEditable((XmTextSource) source));
}

/* this routine does nothing now. it use to call the _Xmstringsourceeditable
   no one calls it
*/
XtEDiskChangeEditable(source, value)
EDiskSource	source;
Boolean		value;
{
    if (source == NULL) return;
}

XtEDiskSetChanged(source, value)
EDiskSource	source;
Boolean		value;
{

    if (source == NULL) return;
    if (value) {
	source->changed = True;
    }
    else
	source->changed = False;
}

/* ARGSUSED */
Boolean
XtEDiskChanged(source)
EDiskSource	source;
{

    return(source->changed);
}

/* ARGSUSED */
XtEDiskSetCallbackWhenChanged(source, func, param)
EDiskSource	source;
void		(*func)();
char		*param;
{
    DEBUG(("@ XtEDiskSetCallbackWhenChanged(source,func,param)\n"));
    source->cb_func = func;
    source->cb_param = param;
}

/* Save any unsaved changes. */

Boolean XtEDiskSaveFile(source, vw)
  EDiskSource    source;
  XmTextWidget   vw;
{
    char str[1024];
    char *ptr;
    int	 fd;
    XmTextBlockRec	block;
    XmTextPosition pos = 0;
    XmTextPosition endPos;
    LineTableExtra extra = NULL;
/* *\ XmTextWidget tw = source->data->widgets[0];
\* */
    XmTextWidget tw = vw;
    Boolean psFile = False;		/* True if postscript */

    if (source->filename == NULL)
	Punt("Tried to save string edisk source!");
    if (!source->changed)
	return True;

    source->changed = source->checkpointchange = FALSE;

    fd = myopen(source->filename,O_WRONLY|O_TRUNC|O_CREAT,0666);
    if (fd < 0)
	return False;
    if (_XmTextShouldWordWrap(tw)) {
/*
 * Word-wrap the text as it appears in the text window.
 * This requires scanning the text from begin to end looking
 * for line ends; 'FindLineEnd' in the text widget output
 * routine is used to do the scanning.
 */
	while (pos < source->data->length) {
	    endPos = _XmTextFindLineEnd(tw, pos, &extra);
	    (*source->ReadSource)(source, pos, endPos, &block);
	    WriteAndCheck(fd, block.ptr, block.length);
	    ptr = block.ptr + block.length - 1;
	    /* If it doesn't end in a newline, add one */

	    if (block.length > 1 && (strncmp(ptr, "%!", 2) == 0))
		psFile = True;

	    if (!psFile && *ptr != '\n' && (block.length == endPos - pos)) {
		WriteAndCheck(fd, "\n", 1);
	    }
	    if (extra) XtFree((char *)extra);
	    extra = NULL;
	    pos = pos + block.length; /* Skip to next line */
	}
    } else {
	ptr = XmTextGetString(tw);
	WriteAndCheck(fd, ptr, strlen(ptr));
	XtFree(ptr);
    }
    if (source->checkpointed) {
	(void) sprintf(str, "%s.CKP", source->filename);
	(void) unlink(str);
	source->checkpointed = FALSE;
    }
    myclose(fd);
    return True;
}

/*
 * Save into a new file.  Future normal saves will still go to the original
 * file.  Returns TRUE on success, FALSE if it couldn't open output file.
 */

Boolean
XtEDiskSaveAsFile(source, name, force, vw)
EDiskSource     source;
char            *name;
Boolean         force;
XmTextWidget    vw;
{
    int         fd;
    struct stat buffer;
    XmTextBlockRec      block;
    XmTextPosition pos = 0;
    XmTextPosition endPos;
    LineTableExtra extra = NULL;
/* *\ XmTextWidget tw = source->data->widgets[0];
\* */
    XmTextWidget tw = vw;
    char *ptr;
    Boolean psFile = False;		/* True if postscript */

/* If the source has not changed, then we don't need to save it! */

    if (!force && !source->changed) {
	DEBUG(("Don't need to save file!\n"));
	return False;
    }

    if (stat(name, &buffer) < 0){
      fd = myopen(name,O_WRONLY|O_TRUNC|O_CREAT,0666);
      DEBUG(("Extract: Message appended to %s\n", name));
    }
    else{
      fd = myopen(name,O_RDWR|O_TRUNC, 0666);
      DEBUG(("Extract: Message placed in  %s\n", name));
    }
    if (fd < 0)
	return False;
    if (_XmTextShouldWordWrap(tw)) {
/*
 * Word-wrap the text as it appears in the text window.
 * This requires scanning the text from begin to end looking
 * for line ends; 'FindLineEnd' in the text widget output
 * routine is used to do the scanning.
 */
	while (pos < source->data->length) {
	    endPos = _XmTextFindLineEnd(tw, pos, &extra);
	    (*source->ReadSource)(source, pos, endPos, &block);
	    WriteAndCheck(fd, block.ptr, block.length);
	    ptr = block.ptr + block.length - 1;
	    /* If it doesn't end in a newline, add one */

	    if (block.length > 1 && (strncmp(ptr, "%!", 2) == 0))
		psFile = True;

	    if (!psFile && *ptr != '\n' && (block.length == endPos - pos)) {
		WriteAndCheck(fd, "\n", 1);
	    }
	    if (extra) XtFree((char *)extra);
	    extra = NULL;
	    pos = pos + block.length; /* Skip to next line */
	}
    } else {
	ptr = XmTextGetString(tw);
	WriteAndCheck(fd, ptr, strlen(ptr));
	XtFree(ptr);
    }
    myclose(fd);
    return True;
}

XtEDiskMarkSaved(source)
EDiskSource source;
{
    source->changed = True;
}

/* ARGSUSED */
XtEDiskMakeCheckpoint(source)
EDiskSource	source;
{
    char name[1024];
    char *text;
    int fid;
    XmTextWidget msgw = source->data->widgets[0];

    if (!source->checkpointchange) return; /* No changes to save. */
    (void) sprintf(name, "%s.CKP", source->filename);
    fid = creat(name, 0600);
    if (fid < 0) return;
    text = XmTextGetString(msgw);
    (void) write(fid, text, strlen(text));
    XtFree(text);
    (void)close(fid);
    source->checkpointed = TRUE;
    source->checkpointchange = FALSE;
}

XtEDiskSetSource(widget, source, startPos, curPos)
XmTextWidget	widget;
EDiskSource	source;
XmTextPosition	startPos;
XmTextPosition	curPos;
{
    XmTextWidget tw = (XmTextWidget) widget;  /* added for motif 1.2 */
    XmTextWidget tw2;
    XmTextPosition x, y;
    XmSourceData data = source->data;

    DEBUG(("@ XtEDiskSetSource(w %lx,source %lx,%ld,%ld)\n", widget,source,
			startPos,curPos));
/* if source exists then call Xm library routine to set the source structure
 *   to the text widget, and return.
 *        tw->text.cursor_position = curPos;
 */

    if (source != (EDiskSource)NULL ) { 
	XmTextSetSource(tw,source,startPos,curPos);
        return;
    }


/* if source is null then clear out the widget infor to set to null to
 *   clear it out
 */
    widget->text.source = (XmTextSource) source;
    widget->text.cursor_position = startPos;
    widget->text.new_top = curPos;
    widget->text.total_lines = 1;		/* changed for motif 1.2 */
    widget->text.top_line = 0;			/* added for motif 1.2 */
    widget->text.needs_refigure_lines = True;
    widget->text.last_position = source->data->length;
    widget->text.bottom_position = source->data->length;
    widget->text.top_character = 0;
    widget->text.on_or_off = off;
    (void) XmTextPosToXY(widget, (XmTextPosition) 0, &x, &y);

    if (widget->text.source != (XmTextSource)NULL &&
	widget->text.source != NullSource)
	(*widget->text.source->RemoveWidget)(widget->text.source, widget);

    if (source != (EDiskSource) NullSource)
	(*widget->text.source->AddWidget)(widget->text.source, widget);
    data->source = (XmTextSource) source;
    XmTextSetCursorPosition(widget, startPos);
/*
 * Text widget does not set line count if numwidgets not = 1
 * so we do our own counting..
 */
    if (widget->text.source->data->numwidgets > 1) {
	XmTextWidget tw = widget->text.source->data->widgets[0];
	widget->text.total_lines = tw->text.total_lines;
    }
    if (source != (EDiskSource) NullSource) {
	_XmTextInvalidate(widget, 0, 0, NODELTA);
	XmTextSetHighlight(widget, 0, MAXINT, 0);
        if (widget->text.disable_depth == 0)
/*
	    Redisplay(widget);
*/
/* Cause a Redisplay() to be called. */
	    XmTextScroll(widget, 0);
    }
}
