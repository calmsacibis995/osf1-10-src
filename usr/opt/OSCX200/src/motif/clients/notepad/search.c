/* #module search.c "v1.0"
 *
 *  Copyright (c) Digital Equipment Corporation, 1990
 *  All Rights Reserved.  Unpublished rights reserved
 *  under the copyright laws of the United States.
 *  
 *  The software contained on this media is proprietary
 *  to and embodies the confidential technology of 
 *  Digital Equipment Corporation.  Possession, use,
 *  duplication or dissemination of the software and
 *  media is authorized only pursuant to a valid written
 *  license from Digital Equipment Corporation.
 *
 *  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
 *  disclosure by the U.S. Government is subject to
 *  restrictions as set forth in Subparagraph (c)(1)(ii)
 *  of DFARS 252.227-7013, or in FAR 52.227-19, as
 *  applicable.
 *
 *
 * FACILITY:
 *	Notepad
 *
 * ABSTRACT:
 *	
 *
 * NOTES:
 *	
 *
 * REVISION HISTORY:
 */

#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: [search.c,v 1.3 91/05/23 18:48:17 rmurphy Exp ]$";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */

#include "notepad.h"

#ifndef F_SETSYN
#define F_SETSYN 10
#define F_CLRSYN 11
#endif

#define MAXSEARCHLEN 	50 

void callClosureWithSelection(w, closure, selection, type, value, length,
							format)
  Widget w;
  Opaque closure;
  Atom *selection;
  Atom *type;
  char *value;
  int *length;
  int *format;
/*
 * Function: callClosureWithSelection is called by XtGetSelectionValue
 * 	     after the selection is ready. Note that this may be called
 * 	     before or after XtGetSelectionValue returns. 
 *	
 * Inputs:
 *   w 		- widget requesting the selection
 *   closure	- client passed to XtGetSelectionValue
 *   *selection - atom name of selection
 *   *type 	- type of selection
 *   *value	- its value. Needs to be XtFree'd when done
 *   *length	- its length (in units of the specified format)
 *   *format	- the unit size
 *	
 * Outputs:
 *	
 * Notes:
 */

{
    (*(void(*)())closure)(value, *length); /* Call copyToClipboard */
    XtFree(value);
}

void SetSearchToggle(w, tag, reason)
  Widget   w;
  caddr_t  tag;
  int reason[2];
{
    state[st_CaseSensitive] = 1 ^ state[st_CaseSensitive];
}

char *bufferFromSource(from, to, needsFreeing)
  XmTextPosition from, to;
  int *needsFreeing;
{
  /* static */ char *buf; 
  XmTextBlockRec text;
  int destpos;
  XmTextPosition pos;
    (*Psource->ReadSource)(Psource, from, to, &text);
    if(text.length == to - from){
	*needsFreeing = FALSE;
/* printf("doesn't need freeing\n"); */
	return text.ptr;
    }
    buf = XtMalloc(to - from);
    *needsFreeing = TRUE;
    destpos = 0;
    for(pos = from; pos < to; ){
        pos = (*Psource->ReadSource)(Psource, pos, to, &text);
	memcpy(&buf[destpos], text.ptr, text.length);
	destpos += text.length;
    }
/* printf("NEEDS FREEING\n"); */
    return buf;
}

SearchRight(buf, size, target, count, result)
  char *buf, *target;
  XmTextPosition  *result;
  int size, count;

{
  int n, i;
  char *s1, *s2;
    for( i = 0; i <= size - count; i++){
	n = count;
	s1 = &buf[i];
	s2 = target;
	if(state[st_CaseSensitive])
		while (--n >= 0 && *s1++ == *s2++);
	else
	    while (--n >= 0 && tolower(*s1++) == tolower(*s2++));
	if(n < 0)
	    break;
    }
    if( n < 0){
        *result = i;
	return(1);
    } else {
        return(0);
    }
}

SearchLeft(buf, size, target, count, result)
  char *buf, *target;
  XmTextPosition  *result;
  int size, count;

{
  int n, i;
  char *s1, *s2;
    for( i = size-count; i >= 0; i--){
	n = count;
	s1 = &buf[i];
	s2 = target;
	if(state[st_CaseSensitive])
	    while (--n >= 0 && *s1++ == *s2++);
	else
	    while (--n >= 0 && tolower(*s1++) == tolower(*s2++));
	if(n < 0)
	    break;
    }
    if(n < 0){
        *result = i;
	return(1);
    }
    else {
	return(0);
    }
}

void BottomProc()
{
  int pos;
    pos =  (*Psource->Scan)(Psource, 0, XmSELECT_ALL, XmsdRight, 1, FALSE);
    XmTextSetInsertionPosition( textwindow, pos);
    XmTextSetCursorPosition( textwindow, pos);
}

void TopProc()
{
  int pos;
    pos =  (*Psource->Scan)(Psource, 0, XmSELECT_ALL, XmsdLeft, 1, FALSE);
    XmTextSetInsertionPosition  (textwindow, pos);
    XmTextSetCursorPosition  (textwindow, pos);
}

searchRightForThingWithinRange(target, count, from, to)
  char *target;
  int count;
  XmTextPosition from, to;
{
    int res, needsFreeing;
    XmTextPosition result;
    Time	timestamp;
    char *buf = bufferFromSource(from, to, &needsFreeing);

    if (SearchRight(buf, to - from, target, count, &result))
    {
	result += from;
        XmTextSetInsertionPosition(textwindow, result+count);
        XmTextSetCursorPosition(textwindow, result+count);
	timestamp = XtLastTimestampProcessed(XtDisplay(textwindow));
	(*Psource->SetSelection) (Psource, result, result+count, timestamp);
	res = TRUE;
    }
    else
    {
	res = FALSE;
    }
    if(needsFreeing) XtFree(buf);
    return res;
}

searchLeftForThingWithinRange(target, count, from, to)
  char *target;
  int count;
  XmTextPosition from, to;
{
    char *buf;
    int res, needsFreeing;
    XmTextPosition result;
    Time	timestamp;
    buf = bufferFromSource(to, from, &needsFreeing);

    if (SearchLeft(buf, from - to, target, count, &result))
    {
        result += to;
	XmTextSetInsertionPosition(textwindow, result);
	XmTextSetCursorPosition(textwindow, result);
	timestamp = XtLastTimestampProcessed(XtDisplay(textwindow));
	(*Psource->SetSelection) (Psource, result, result+count, timestamp);
	res = TRUE;
    }
    else
    {
        res = FALSE;
    }
    if(needsFreeing)XtFree(buf);
    return res;
}

/* look for target starting at insert point */
searchRightForThing(target, count)
  char *target;
  int count;
{
  XmTextPosition from, to, result;
  char *buf;
    int res;
    from = XmTextGetInsertionPosition(textwindow);
    to = (*Psource->Scan)(Psource, 0, XmSELECT_ALL, XmsdRight, 1, FALSE);
    return (searchRightForThingWithinRange(target,count,from,to));	
}

searchLeftForThing(target, count)
  char *target;
  int count;
{
  XmTextPosition from, to, result;
  char *buf;
    int res;
    from = XmTextGetInsertionPosition(textwindow);
    to = (*Psource->Scan)(Psource, 0, XmSELECT_ALL, XmsdLeft, 1, FALSE);
    return (searchLeftForThingWithinRange(target,count,from,to));

}

void DoSearchNextAndFinish(w, tag, reason)
  Widget w;
  int tag;
  int reason;
{
  char* target = XmTextGetString(seaDialog.stringWidget);
    if(!searchRightForThing(target, strlen(target)))
	Feep();
    XSetInputFocus(XtDisplay(textwindow), XtWindow(textwindow),
             RevertToParent, XtLastTimestampProcessed(XtDisplay(textwindow)));
    XtUnmanageChild(seaDialog.popupWidget);
}

void DoSearchPrevAndFinish(w, tag, reason)
  Widget w;
  int tag;
  int reason;
{
  char* target = XmTextGetString(seaDialog.stringWidget);
    if(!searchLeftForThing(target, strlen(target)))
	Feep();
    XSetInputFocus(XtDisplay(textwindow), XtWindow(textwindow),
             RevertToParent, XtLastTimestampProcessed(XtDisplay(textwindow)));
    XtUnmanageChild(seaDialog.popupWidget);
}

void DoSearchNext(w, tag, reason)
  Widget w;
  int tag;
  int reason;
{
  char* target = XmTextGetString(seaDialog.stringWidget);
    if(!searchRightForThing(target, strlen(target)))
	Feep();
}

void DoSearchPrevious(w, tag, reason)
  Widget w;
  int tag;
  int reason;
{
  char* target = XmTextGetString(seaDialog.stringWidget);
    if(!searchLeftForThing(target, strlen(target)))
	Feep();
}

SelSearchRightForThing(target, count)
  char *target;
  int count;
{
    if(!searchRightForThing(target, count))
	Feep();
}

SelSearchLeftForThing(target, count)
  char *target;
  int count;
{
    if(!searchLeftForThing(target, count))
	Feep();
}

void DoSearchNextForSelection(w, tag, reason)
  Widget w;
  int tag;
  int reason;
{
    XtGetSelectionValue
    (
	toplevel,
	XA_PRIMARY,
	XA_STRING, 
	(XtSelectionCallbackProc)callClosureWithSelection,
	SelSearchRightForThing,
	XtLastTimestampProcessed(XtDisplay(toplevel))
    );
}

void DoSearchPreviousForSelection(w,tag,reason)   
  Widget w;
  int tag;
  int reason;	   
{
    XtGetSelectionValue
    (
	toplevel,
	XA_PRIMARY,
	XA_STRING, 
	(XtSelectionCallbackProc)callClosureWithSelection,
	SelSearchLeftForThing,
	XtLastTimestampProcessed(XtDisplay(toplevel))
    );
}

void GotoPositionProc()
{
    /* NULL ROUTINE */
}

ReplaceWithinRange(from, to)
XmTextPosition from, to;
{
    int  tcount, rcount;
    XmTextBlockRec block;
    XmTextPosition left, right, newFrom = from, newTo = to;
    XmTextStatus status;

    char *target = XmTextGetString(replaceDialog.stringWidget);
    char *desire = XmTextGetString(replaceDialog.stringWidget_1);
    Time	timestamp;

    tcount = strlen(target); rcount = strlen(desire);
    if(!tcount)
    {
	Feep(); 
	return FALSE;
    }
#ifndef VMS
    fcntl(fileno(journalFile), F_CLRSYN, 0);
#endif /* VMS */
    _XmTextDisableRedisplay(textwindow, TRUE);
    while(searchRightForThingWithinRange(target, tcount, newFrom, newTo))
    {
        block.length = rcount;
	block.ptr = desire;
        block.format = FMT8BIT;
	timestamp = XtLastTimestampProcessed(XtDisplay(textwindow));
	(*Psource->GetSelection)(Psource, &left, &right);
        (*Psource->SetSelection)(Psource, right, right, timestamp);
	status = (*Psource->Replace)
	    (textwindow, NULL, &left, &right, &block, True);
        if (status == EditDone)
	{
            XmTextSetInsertionPosition (textwindow,(right-tcount)+rcount);
	    newFrom = (right-tcount)+rcount;
	    newTo = (newTo - tcount) + rcount;
	}
	else
	{
	    Feep();
	    break;
	}
    }
#ifndef VMS
    fcntl(fileno(journalFile), F_SETSYN, 0);
#endif /* VMS */
    _XmTextEnableRedisplay(textwindow);
}

void ReplaceOnceProc()
{
    int left, right, tcount, rcount;
    XmTextBlockRec block;
    XmTextStatus status;
    char *target = XmTextGetString(replaceDialog.stringWidget);
    char *desire = XmTextGetString(replaceDialog.stringWidget_1);

    tcount = strlen(target); rcount = strlen(desire);
    if(tcount)
    {
        if (!(*Psource->GetSelection)(Psource, &left, &right))
	{
	    Feep();
	    searchRightForThing(target, tcount);
        }
	else
	{
            block.length = rcount;
	    block.ptr = desire;
            block.format = FMT8BIT;
	    status = (*Psource->Replace)
		(textwindow, NULL, &left, &right, &block, True);
            if (status == EditDone)
	    {
		Time	timestamp;
                XmTextSetInsertionPosition(textwindow,(right-tcount)+rcount);
                XmTextSetCursorPosition(textwindow,(right-tcount)+rcount);
		timestamp = XtLastTimestampProcessed(XtDisplay(textwindow));
	        if(!searchRightForThing(target, tcount))
		    (*Psource->SetSelection)
		    (
			Psource,
			(right-tcount)+rcount, (right-tcount)+rcount,
			timestamp
		    );
            }
	    else
	    {
                Feep();
            }
	}
    }
    else
    {
        Feep();
    }
/*    hideSimpleDialog(replaceDialog.dialogWidget, &replaceDialog, 0); */
}   

void ReplaceAllProc()
{
  XmTextPosition left, right;
    left  = (*Psource->Scan)(Psource, 0, XmSELECT_ALL, XmsdLeft, 1, FALSE);
    right = (*Psource->Scan)(Psource, 0, XmSELECT_ALL, XmsdRight, 1, FALSE);
    ReplaceWithinRange(left, right);
    hideSimpleDialog(replaceDialog.dialogWidget, &replaceDialog, 0);
}

void ReplaceSelectedProc()
{
    XmTextPosition left, right;
    Time	timestamp;
    timestamp = XtLastTimestampProcessed(XtDisplay(textwindow));
    if((*Psource->GetSelection)(Psource,&left,&right))
    {
        (*Psource->SetSelection) (Psource, left, left, timestamp);
	ReplaceWithinRange(left, right);
    }
    else
    {
	Feep();
    }
    hideSimpleDialog(replaceDialog.dialogWidget, &replaceDialog, 0);
}

static Jump(line)
  int line;
{
  XmTextPosition pos;
    if(line <= 1)
        pos = 0;
    else
        pos =  (*Psource->Scan)(Psource, 0, XmSELECT_LINE, XmsdRight, line-1, 1);
    XmTextSetInsertionPosition( textwindow, pos);
    XmTextSetCursorPosition( textwindow, pos);
}

void DoJump(selection, count)
    char *selection;
    int count; /* XXX fix to use correct count !!! */
{
    int  line;
	
    if(selection && count && sscanf(selection, "%d", &line) > 0){
        Jump(line);
    } else {
        XeditPrintf("\nPlease 'Select' a line number and try again");
	Feep();
    }
}

void GotoLineProc(w, tag, reason)
  Widget w;
  int tag;
  int reason;
{
    XtGetSelectionValue
    (
	toplevel,
	XA_PRIMARY,
	XA_STRING,
	(XtSelectionCallbackProc)callClosureWithSelection,
	DoJump,
	XtLastTimestampProcessed(XtDisplay(textwindow))
    );
}

void DoGotoLine(w, tag, reason)
  Widget w;
  int tag;
  int reason;
{
  int count;
  char *target;
    target = XmTextGetString(LineDialog.stringWidget);
    count = strlen(target);
    DoJump(target, count);
    XSetInputFocus(XtDisplay(textwindow), XtWindow(textwindow),
             RevertToParent, XtLastTimestampProcessed(XtDisplay(textwindow)));
    XtUnmanageChild(LineDialog.popupWidget);
}

void DoShowLine(w, tag, reason)
  Widget w;
  int tag;
  int reason;
{
}

void SearchStringModified(w, tag, reason)
  Widget w;
  int tag;
  int reason;
{
  int count;
  char *target;
    if(!textwindow)
	return;
    target = XmTextGetString(w);
    count = strlen(target);
	if(count){
	  if(incr_direction == 0){
	    if(!searchRightForThingWithinRange(target, count, searchIncrIndex,
    	      (*Psource->Scan)(Psource, 0, XmSELECT_ALL, XmsdRight, 1, FALSE))){
	        Feep();
	      }
	    } else {
	    if(!searchLeftForThingWithinRange(target, count, searchIncrIndex,
              (*Psource->Scan)(Psource, 0, XmSELECT_ALL, XmsdLeft, 1, FALSE))){
                Feep();
              }
	    }
	} else {
	    searchIncrIndex = XmTextGetInsertionPosition(textwindow);
	}
}
