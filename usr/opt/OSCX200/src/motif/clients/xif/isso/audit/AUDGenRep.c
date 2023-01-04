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
static char *rcsid = "@(#)$RCSfile: AUDGenRep.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/12/20 21:31:39 $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/limits.h>
#include <sys/dir.h>

#include <Mrm/MrmAppl.h>
#include <Xm/Text.h>
#include <Xm/TextP.h>
#include <Xm/ToggleB.h>
#include <X11/X.h>

#include "Vuit.h"
#include "XIsso.h"
#include "Utilities.h"
#include "Resources.h"
#include "AUD.h"

/* ZZZ We should really use literals for these */
#define MAIN_WIDGET	"AUDGenRepFRMD"
#define CHANGE_WIDGET	"AUDGenRepChangeLogDirFRMD"
#define REPORT_WIDGET	"AUDGenRepActualReportFRM"
#define STATUS_WIDGET	"AUDGenRepStatusWindowFRM"
#define ASK_WIDGET	"AUDGenRepAskNewDirFRMD"
#define FULLREC_WIDGET	"AUDGenRepFullRecFRMD"

/* The time between mouse clicks which will cause a selection
 *  in the text widget
 */
#define MOUSE_CLICK_INTERVAL	500
#define MB1			  1

/* conditions for report display */
#define TOFILE		0
#define TOAUID		1
#define TOSCREEN	2       

/* external variables */
extern XtTimerCallbackProc listen_to_audit_tool();
extern XtAppContext app_context;

/* global variables */

/* master widget for data structure look-up */
static Widget mainWidget = (Widget) NULL;

/* all sub-windows must be created to access widget info */
Boolean	createdReportWindow = False;
Boolean	createdStatusWindow = False;
Boolean	createdChangeLogDirWindow = False;
Boolean	createdAskNewDirWindow = False;
Boolean	createdFullRecWindow = False;

Boolean	isManagedReportWindow = False;
Boolean	isManagedStatusWindow = False;
Boolean	isManagedChangeLogDirWindow = False;
Boolean	isManagedAskNewDirWindow = False;
Boolean	isManagedFullRecWindow = False;

Boolean	reportAtBeginning = False;
Boolean	reportAtEnd       = False;

/* Keeps current location of audit logs */
char	*currentAuditLogDir = (char *) NULL;
char	*currentAuditLog = (char *) NULL;

/* communication structure between io code and ui code */
WorkDataPtr	ioData;

/* globals for data management */
int systemPageSize;
static AUDPagePtr poolHead = (AUDPagePtr) NULL;
int	   pageMin = 0, pageMax = 0;
int	   currentPage;
AUDPagePtr OnScreenPage = (AUDPagePtr) NULL;


/* forward declaration */
void 	   AUDGenRepCancelCallback();
void       AUDGenRepActualReportMapCallback();
void       AUDGenRepActualReportCancelCallback();
AUDPagePtr AUDCreateDataPage();
void       AUDIncrementNumberofSections();
void       AUDDecrementNumberofSections();


/*==================<Data buffer management Routines>================*/

void
AUDInitPageScheme()
{
    currentPage = -1;
    poolHead    = (AUDPagePtr) NULL;
    pageMin     = 0;
    pageMax     = 0;

}

void
AUDTermPageScheme()
{
    AUDPagePtr	curPage;
    /* walk entire linked list and free up all memory */
    
    while(poolHead)
    {
	curPage = poolHead->next;

	if(poolHead->data != (char *) NULL)
	    XtFree(poolHead->data);

	if(poolHead->map != (AUDRecordMapPtr) NULL)
	    XtFree((char *)poolHead->map);

	XtFree((char *)poolHead);
	
	poolHead = curPage;
    }
}



AUDPagePtr
AUDGetPage(pageNum)
int	pageNum;
{
    AUDPagePtr	pagePtr = (AUDPagePtr) NULL;

    if(pageNum < pageMin || pageNum > pageMax)
	return ((AUDPagePtr) NULL);
    else
    {
	/* walk list until we find it */
	pagePtr = poolHead;
	while(pagePtr &&
	      pagePtr->number != pageNum)
	{
	    pagePtr = pagePtr->next;
	}
    }
    
    return(pagePtr);
}



AUDPagePtr
AUDGetNextPage()
{
    AUDPagePtr	page;

    /* if we are at the beginning or the current one is full, 
     * then create a new one.  Otherwise, pass back the current
     * one.
     */

    if(((page = AUDGetPage(currentPage)) == (AUDPagePtr) NULL) ||
       (page->full))
	    return(AUDCreateDataPage());
    else
	    return(page);
}


AUDPagePtr
AUDCreateDataPage()
{
AUDPagePtr	page = (AUDPagePtr) NULL;

    /* if we have slots left in memory, then just allocate a 
     * new page.  Otherwise, we must page out our oldest page
     * and reuse that.
     */

    if(++currentPage < AUDresources.maxMemoryPages)
    {
	/* allocate! */
	page = (AUDPagePtr) XtMalloc(sizeof(AUDPage));

	if(page)
	{
	    page->number     = currentPage;

	    /* set active display page */
	    if(currentPage == 0)
	    {
		OnScreenPage = page;
		page->isOnScreen = True;
	    }
	    else
		page->isOnScreen = False;

	    page->full       = False;
	    page->data = (char *) XtMalloc(AUDIT_PAGE_SIZE);

	    if(page->data == (char *) NULL)
		    MemoryError();

	    page->data[0] = '\0';

	    page->map  = (AUDRecordMapPtr) NULL;

	    /* link into list */
	    page->next = poolHead;
	    page->prev = (AUDPagePtr) NULL;
	    if(poolHead)
		    poolHead->prev = page;
	    poolHead = page;
	}
	else
		MemoryError();
    }
    else
    {
	/* need to make some room by removing oldest page and re-using it.
	 * If the oldest page is actively being displayed, then we must
	 * wait for the user to move out of it.
	 */

	/* find oldest page */
	if(((page = AUDGetPage(pageMin)) != (AUDPagePtr) NULL))
	{
	    if(page->isOnScreen)
	    {
		currentPage--;
		return((AUDPagePtr) NULL);
	    }
	    else
	    {
		/* good, let's re-use it! */
		page->number     = currentPage;
		page->isOnScreen = False;
		page->full       = False;
		if(page->map)
		    XtFree((char *)page->map);
		page->map        = (AUDRecordMapPtr) NULL;
		page->data[0]    = '\0';

		/* already linked into list */

		AUDDecrementNumberofSections();
	    }
	}
	else
	{
	    /* Shouldn't happen */
	}
    }

    if(page)
    {
	/* change slider value */
	AUDIncrementNumberofSections();
    }

    return(page);
}



int
AUDCopyBriefDataToPage(page, buffer, length, start)
AUDPagePtr	page;
char		*buffer;
int		length;
char		**start;
{
    Boolean	twiddled, done, firstPass;
    int		read, taken, blen, flen;
    char	save, *dataPtr, *dataMax, *briefPtr, *fullPtr;

    read      = 0;
    firstPass = True;

    if(start != (char **) NULL)
	*start = (char *) NULL;

    /* if we haven't already allocated our mapping structure, do so */
    if(page->map == (AUDRecordMapPtr) NULL)
    {
	page->map = (AUDRecordMapPtr) XtMalloc(sizeof(AUDRecordMap));
	if(page->map == (AUDRecordMapPtr) NULL)
	    MemoryError();

	bzero((char *)page->map, sizeof(AUDRecordMap));
	page->map->numRecs  = 0;
	page->map->briefPos = 0;
	page->map->fullPos  = AUDIT_FULL_POOL;
    }

    /*
     * While the incoming data has a brief/full pair, we check to see
     * if our current page has enough room to hold it.
     * If so, we append the brief record to the accumulating
     * brief string and we copy the full record to the full area
     * and record information on these to do look ups when the
     * user double clicks.
     *
     * If we can't fit a record into our current page, we are finished.
     */
    dataPtr = buffer;
    dataMax = &buffer[length - 1];

    /* we must protect against going off the end of the buffer by 
     * artificially putting a NULL there.
     */
    twiddled = True;
    if(*dataMax == '\0')
	twiddled = False;

    if(twiddled)
    {
	save = *dataMax;
	*dataMax = '\0';
    }

    done = False;
    while(!done &&
	  dataPtr < dataMax)
    {
	briefPtr = dataPtr;
	taken    = 0;

	if((blen = strlen(briefPtr)) <= 
	   (AUDIT_FULL_POOL - page->map->briefPos - 1))
	{	
	    /* we have the room in the brief side! */
	    fullPtr = briefPtr + blen + 1;
	    if(fullPtr >= dataMax)
		done = True;
	    else if((flen = strlen(fullPtr)+1) <=
	       (AUDIT_PAGE_SIZE - page->map->fullPos))
	    {	 
		int n = page->map->numRecs;

		/* we have enough room in the full side!! */
		dataPtr = fullPtr + flen;
		if((dataPtr >= dataMax)
		   && twiddled)
		    done = True;
		else
		{
		    /* transfer the data and continue */
		    strcat(page->data, briefPtr);

		    /* if the caller wants the starting place, then
		     * give it to him
		     */
		    if(firstPass &&
		       start != (char **)NULL)
		    {
			*start = page->data + page->map->briefPos;
			firstPass = False;
		    }

		    taken = blen + 1;
		    page->map->briefCursorPos[n] = page->map->briefPos; 
		    page->map->briefPos += blen;		    

		    /* Full Record */
		    bcopy(fullPtr,
			  &(page->data[page->map->fullPos]),
			  flen);

		    taken += flen;
		    page->map->fullMemoryPos[n] = 
			    &page->data[page->map->fullPos];
			                          
		    page->map->fullPos += flen;
 
		    page->map->numRecs++;

		    /* get read for next round */
		    read    += taken;
		}
	    }
	    else
	    {
		page->full = done = True;
	    }
	}
	else
	{
	    page->full = done = True;
	}
    }

    if(twiddled)
    {
	*dataMax = save;
    }

    return(read);
}



int
AUDCopyDataToPage(page, buffer, length)
AUDPagePtr	page;
char		*buffer;
int		length;
{
    char	*look;
    char	save;
    int		read, limit;

    /* bring up to the last <CR> that will
     * fit in this page
     */
    read  = 0;
    limit = AUDIT_PAGE_SIZE - (strlen(page->data)+1);

    if(limit > 0)
    {
	look = buffer + min(length-1, limit);
	while(look > buffer &&
	      *look != '\n')
	    look--;
    
	/* substitute a NULL for this character, temporarily
	 * and grab the data.
	 */
	if(look > buffer)
	{
	    save = *(look+1);
	    *(look+1) = '\0';
	    strcat(page->data, buffer);
	    read += strlen(buffer);
	    *(look+1) = save;
	}
	/* the else here means we have a full buffer with no <CR>.
	 * at the moment we are not handling that!!
	 */
    }

    /* detect page being full */
    page->full = (length > limit);

    return(read);
}







/* 
 * AUDHandleIncomingReportData()
 *
 * This routine is called with the data read from audit_tool
 */

int
AUDHandleIncomingReportData(data, length)
char	*data;
int	length;
{
    AUDGenRepPtr	pGenRepData;
    AUDPagePtr		origPage, page;
    char		*dataPtr, **startPtr;
    int			read, toCurrent;


    read      = 0;
    toCurrent = 0;
    dataPtr   = data;

    if(length > 0 && data != (char *) NULL)
    {
	/* obtain options data */
	GetUserData(mainWidget, pGenRepData);

	if (pGenRepData)
	{
	    /* First order of business is to take in as much data
	     * as we can up to a "boundary".  In the case of Full
	     * Records, this will be at a <CR>.  In the case of
	     * Brief Records, this will be a brief\0full\0 pair.
	     *
	     * If, along the way, we cross a "page" boundary, that
	     * is OK as long as we can get a next page.
	     *
	     * We will not take more than an integral amount of data.
	     * Sometimes we take none if our page pool is full and the
	     * user is looking at the oldest page.
	     *
	     * The second order of business is to update the Text Widget
	     * iff the original page in use is on the screen.
	     */
	    if(origPage = AUDGetNextPage())
	    {
		if(pGenRepData->data.brief)
		{
		    char	*title;
		    int	count, newLineCount;

		    count = 0;
		    if(reportAtBeginning)
		    {
			/* We need to grab the first 2 lines (title) and
			 * put in a separate text field.
			 */
			newLineCount = 2;
			while(newLineCount)
			{
			    if(dataPtr[count] == '\n')
				newLineCount--;
			    count++;
			}

			title = (char *) malloc(count);

			if(title)
			{
			    strncpy(title, dataPtr, count-1);
			    title[count-1] = '\0';
				    
			    XmTextSetString(
				   pGenRepData->ui.actualReportTitleTXT_W,
				   title);
			    free(title);
			}
			reportAtBeginning = False;

			/* skip past for actual report */
			read = count;
		    }

		    /* Brief Record Mode */
		    read += AUDCopyBriefDataToPage(origPage,
						   &dataPtr[read],
						   length-read,
						   &startPtr);
		    toCurrent = read;

		    /* data did not fit in the current page */
		    if(read < length)
		    {
			/* put the rest in a new page */
			if(page = AUDGetNextPage())
			{
			    read += AUDCopyBriefDataToPage(page, 
							   &dataPtr[read],
							   length-read,
							   (char **)NULL);
			}
		    }
		}
		else
		{
		    /* Full Record Mode */
		    if(reportAtBeginning)
		    {
			XmTextSetString(pGenRepData->ui.actualReportTitleTXT_W,
					"Full Record Audit Report");
			reportAtBeginning = False;
		    }

		    /* bring up to the last <CR> that will
		     * fit in this page
		     */
		    read += AUDCopyDataToPage(origPage,
					      dataPtr,
					      length);
		    
		    toCurrent = read;

		    /* data did not fit in the current page */
		    if(read < length)
		    {
			/* put the rest in a new page */
			if(page = AUDGetNextPage())
			{
			    read += AUDCopyDataToPage(page, 
						      &dataPtr[read],
						      length-read);
			}
		    }
		}

		/* Must update the display if we have updated the
		 * page that's currently in the Text Widget.
		 */
		if(origPage->isOnScreen)
		{
		    XmTextPosition	pos;
		    char		save;

		    if(pGenRepData->data.brief && startPtr)
		    {
			if(toCurrent)
			{
			    /* need to append the latest addition
			     * to our brief string
			     */
			    pos = XmTextGetLastPosition
				    (pGenRepData->ui.actualReportTXT_W);

			    XmTextReplace(pGenRepData->ui.actualReportTXT_W,
					  pos,
					  pos,
					  startPtr);
			}
		    }
		    else if(!pGenRepData->data.brief)
		    {
			if(toCurrent)
			{
			    /* must append up to what was inserted into
			     * the active page
			     */
			    save = dataPtr[min(toCurrent,length-1)];
			    dataPtr[min(toCurrent,length-1)] = '\0';

			    pos = XmTextGetLastPosition
			    (pGenRepData->ui.actualReportTXT_W);

			    XmTextReplace(pGenRepData->ui.actualReportTXT_W,
					  pos,
					  pos,
					  dataPtr);

			    dataPtr[min(toCurrent,length-1)] = save;

			    if(toCurrent == length)
			    {
				char buf[2];
				buf[0] = dataPtr[length-1];
				buf[1] = '\0';

				pos = XmTextGetLastPosition
					(pGenRepData->ui.actualReportTXT_W);

				XmTextReplace
					(pGenRepData->ui.actualReportTXT_W,
					 pos,
					 pos,
					 buf);
			    }
			}
		    }
		}
	    }
	}
    }

    return(read);
}

void
AUDHandleIncomingErrorData(data)
char	*data;
{
    AUDGenRepPtr	pGenRepData;

    /* get the widget ID */
    GetUserData(mainWidget, pGenRepData);

    if ((data != (char *) NULL) && pGenRepData)
    {
	XmTextPosition	pos;

	pos = XmTextGetLastPosition(pGenRepData->ui.statusWindowTXT_W);

	XmTextReplace(pGenRepData->ui.statusWindowTXT_W,
		      pos,
		      pos,
		      data);
    }
}


/*==================<UI-Level Support Routines/Callbacks>================*/

void 
CenterDialog(parent, child)
Widget	parent,child;
{
    if(parent && child)
    {
	Cardinal	n;
	Arg		args[4];
	Position	parentX, parentY;
	Position	childX, childY;
	Dimension	parentWidth, parentHeight;
	Dimension	childWidth, childHeight;

	/* we want to center this box over the Generate Reports screen */
	n = 0;
	XtSetArg(args[n], XmNx     , &parentX     ); n++;
	XtSetArg(args[n], XmNy     , &parentY     ); n++;
	XtSetArg(args[n], XmNwidth , &parentWidth ); n++;
	XtSetArg(args[n], XmNheight, &parentHeight); n++;
	XtGetValues(parent, args, n); 

	n = 0;
	XtSetArg(args[n], XmNwidth , &childWidth ); n++;
	XtSetArg(args[n], XmNheight, &childHeight); n++;
	XtGetValues(child, args, n); 

	childX = parentX + (parentWidth/2 - childWidth/2);
	childY = parentY + (parentHeight/2 - childHeight/2);
	    
	n = 0;
	XtSetArg(args[n], XmNx, childX); n++;
	XtSetArg(args[n], XmNy, childY); n++;
	XtSetValues(child, args, n); 	    
    }
}

void
DeIconify(w)
Widget	w;
{
    Boolean iconified;
    Arg args[10];

    if(w)
    {
	XtSetArg(args[0], XmNiconic, &iconified);
	XtGetValues(w, args, 1);

	if (iconified)
	{
	    XtSetArg(args[0], XmNiconic, False);
	    XtSetValues(w, args, 1);
	}
    }
}



void
Iconify(w)
Widget	w;
{
    Arg args[10];

    if(w)
    {
	XtSetArg(args[0], XmNiconic, True);
	XtSetValues(w, args, 1);
    }
}



void
AUDIncrementNumberofSections()
{
    Widget 		ancestor;
    AUDGenRepPtr	pGenRepData;


    if(currentPage >= 1)
    {
	/* all screens are hooked to the same data structure */
	if(ancestor = mainWidget)
	{
	    GetUserData(ancestor, pGenRepData);

	    if( pGenRepData )
	    {
		Arg args[10];
		int max;

		/* only move ahead once we have 3 */
		if(currentPage >= 2)
		{
		    XtSetArg(args[0], XmNmaximum, &max);
		    XtGetValues(pGenRepData->ui.actualReportSCALE_W, args, 1);

		    max++;
		    XtSetArg(args[0], XmNmaximum, max);
		    XtSetValues(pGenRepData->ui.actualReportSCALE_W, args, 1);
		    pageMax = max - 1;
		}
		else
		{
		    pageMax = currentPage;
		}
		XtSetSensitive(pGenRepData->ui.actualReportSCALE_W, True);
	    }
	}
    }
    else
    {
	pageMax = currentPage;
    }
}



void
AUDDecrementNumberofSections()
{
    Widget 		ancestor;
    AUDGenRepPtr	pGenRepData;


    /* all screens are hooked to the same data structure */
    if(ancestor = mainWidget)
    {
	GetUserData(ancestor, pGenRepData);

	if( pGenRepData )
	{
	    Arg args[10];
	    int min;

	    XtSetArg(args[0], XmNminimum, &min);
	    XtGetValues(pGenRepData->ui.actualReportSCALE_W, args, 1);

	    min++;
	    XtSetArg(args[0], XmNminimum, min);
	    XtSetValues(pGenRepData->ui.actualReportSCALE_W, args, 1);

	    pageMin = min - 1;
	}
    }
}



static void
AUDGenRepClearFields(pData)
AUDGenRepUIPtr	pData;
{
    if( pData )
    {
	/* Toggle Button Settings */
	if( pData->displayNowTB_W )
		XmToggleButtonSetState(pData->displayNowTB_W,
				       True,
				       False);

	if( pData->displayCurrentTB_W )
		XmToggleButtonSetState(pData->displayCurrentTB_W,
				       False,
				       False);

	if( pData->saveToFileTB_W )
		XmToggleButtonSetState(pData->saveToFileTB_W,
				       False,
				       False);

	if( pData->reportFilesTB_W )
		XmToggleButtonSetState(pData->reportFilesTB_W,
				       False,
				       False);

	if( pData->fullRecTB_W )
		XmToggleButtonSetState(pData->fullRecTB_W,
				       False,
				       False);
	if( pData->briefRecTB_W )
		XmToggleButtonSetState(pData->briefRecTB_W,
				       True,
				       False);

	if( pData->followChangeTB_W )
		XmToggleButtonSetState(pData->followChangeTB_W,
				       True,
				       False);

	if( pData->translateIDsTB_W )
		XmToggleButtonSetState(pData->translateIDsTB_W,
				       True,
				       False);

	if( pData->displayStatsTB_W )
		XmToggleButtonSetState(pData->displayStatsTB_W,
				       False,
				       False);

	if( pData->printRulesetTB_W )
		XmToggleButtonSetState(pData->printRulesetTB_W,
				       False,
				       False);

	if( pData->fileNameTF_W )
	    XmTextSetString(pData->fileNameTF_W, "");

	if( pData->auditLogDateTF_W )
	    XmTextSetString(pData->auditLogDateTF_W, "");

	if( pData->auditLogSizeTF_W )
	    XmTextSetString(pData->auditLogSizeTF_W, "");
    }
}



void
AUDHandleErrorCondition()
{
    Widget		sw;
    Widget 		ancestor;
    AUDGenRepPtr	pGenRepData;


    /* all screens are hooked to the same data structure */
    if(ancestor = mainWidget)
    {
	GetUserData(ancestor, pGenRepData);

	if( pGenRepData )
	{
	    /* we want to display the crazy error condition.  The previous
	     * contents of the text field are acceptable at this point
	     */
    
	    /* if the status window is iconified, then pop it up */
	    DeIconify(XtParent(pGenRepData->ui.statusWindowFRM_W));
    
	    /* let's go... */
	    NEW_VUIT_Manage(ASK_WIDGET, &sw, pGenRepData->ui.form_W);
	}
    }
}



Boolean
AUDGenRepAuditingNow()

{
    char str[100];
    char tmp_str[PATH_MAX];
    char *tmp_file;
    FILE *fd;

    if (audcntl(GET_AUDSWITCH,(char *)0, 0, 0, 0, 0) == 0)
	return(False);
    else
    {
	if (tmp_file = tmpnam((char *)NULL))
	{
	    sprintf(str,"%s -dq > %s",AUDIT_DAEMON,tmp_file);
	    system(str);
	    /* '/' in first position of file indicates audit running */
	    if (fd = fopen(tmp_file,"r"))
	    {
		int count = fread(tmp_str, 1, PATH_MAX, fd);

		if (tmp_str[0] == '/')
		{
		    currentAuditLog = (char *) malloc(count);
		    if(currentAuditLog)
		    {
			strncpy(currentAuditLog, tmp_str, count);
			currentAuditLog[count-1] = '\0';
		    }
		    else
			MemoryError();

		    unlink(tmp_file);
		    free(tmp_file);
		    return(True);
		}
		else
		{
		    if(currentAuditLog != (char *) NULL)
		    {
			free(currentAuditLog);
			currentAuditLog = (char *) NULL;
		    }

		    unlink(tmp_file);
		    free(tmp_file);
		    return(False);
		}
	    }
	    unlink(tmp_file);
	    free(tmp_file);
	}
    }
    return(False);
}



Boolean
AUDGenRepBuildAuditToolString(pGenRepData, command_str)
AUDGenRepPtr	pGenRepData;
char *command_str;

{
    char		*str;
    Cardinal		n;
    Arg			args[4];
    int			selectedCount;
    XmStringTable	selectedItems;
    char		command_sub_str[10000];
    struct stat		buf;
    int			ret;
    char 		TO_SCREEN = True;
    char		*ptr;
    FILE		*fd;


    /* initialize command_str to be NULL terminated */
    strcpy(command_str,AUDIT_TOOL);
    strcat(command_str," ");

    if( pGenRepData )
    {
	/* Selection Files */
	n = 0;
	XtSetArg(args[n], XmNselectedItemCount, &selectedCount); n++;
	XtSetArg(args[n], XmNselectedItems, &selectedItems); n++;
	XtGetValues(pGenRepData->ui.selectionFileLST_W, args, n);
	if( selectedCount == 1)
	{
	    if( XmStringGetLtoR(selectedItems[0], charset, &str) )
	    {
		sprintf(command_sub_str,"%s/%s", AUDIT_SELDIR, str);
		ret = stat(command_sub_str,&buf);
		if( ret == 0)
		{
		    ptr = (char *) XtMalloc((buf.st_size+1) * sizeof(char));
		    if (ptr)
		    {
			if (fd = fopen(command_sub_str,"r"))
			{
			    fread(ptr,buf.st_size+1,1,fd);
			    sprintf(command_sub_str," %s",ptr);
			    strcat(command_str,command_sub_str);
			    fclose(fd);
			}
			XtFree(ptr);
		    }
		}
		
		XtFree(str);
	    }
	}

	/* Save to File */
	if (pGenRepData->ui.saveToFileTB_W && pGenRepData->ui.reportFilesTB_W)
	{
	    if (XmToggleButtonGetState(pGenRepData->ui.saveToFileTB_W))
	    {
		if (pGenRepData->data.fileName)
		    XtFree(pGenRepData->data.fileName);
		pGenRepData->data.fileName = StripWhiteSpace(XmTextGetString(pGenRepData->ui.fileNameTF_W));
		if (strcmp(pGenRepData->data.fileName,"") == 0)
		{
		    /* Post an Error Box */
		    PostErrorDialog("AUDmsg_err_need_filename",
				    pGenRepData->ui.form_W,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    NULL);
		    return(False);
		}

		TO_SCREEN = False;
	    }
	    else if (XmToggleButtonGetState(pGenRepData->ui.reportFilesTB_W))
	    {
		char *fileName;

		fileName = StripWhiteSpace(XmTextGetString(pGenRepData->ui.fileNameTF_W));
		if (strcmp(fileName,"") == 0)
		{
		    /* Post an Error Box */
		    PostErrorDialog("AUDmsg_err_need_filename",
				    pGenRepData->ui.form_W,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    NULL);
		    XtFree(fileName);
		    return(False);
		}
		else
		{
		    sprintf(command_sub_str," %s %s",
			    AUDREP_AUIDREP_COPT,
			    fileName);
		    strcat(command_str,command_sub_str);
		    TO_SCREEN = False;
		    XtFree(fileName);
		}
	    }
	    else
	    {
		if (pGenRepData->data.fileName)
		    XtFree(pGenRepData->data.fileName);
		pGenRepData->data.fileName = (char *) NULL;
	    }
	}

	/* Brief or Full Mode */
	if (pGenRepData->ui.briefRecTB_W)
	{
	    if (pGenRepData->data.brief = XmToggleButtonGetState(pGenRepData->ui.briefRecTB_W))
	    {
		/* If Output goes to a file use -B, if to screen - */
		if (TO_SCREEN)
		    sprintf(command_sub_str," %s",AUDREP_BOTH_COPT);
		else
		    sprintf(command_sub_str," %s",AUDREP_BRIEF_COPT);
		
		strcat(command_str,command_sub_str);
	    }
	}

	/* Follow Audit Log Change Records (negative selection) */
	if (pGenRepData->ui.followChangeTB_W)
	{
	    if (! XmToggleButtonGetState(pGenRepData->ui.followChangeTB_W))
	    {
		sprintf(command_sub_str," %s",AUDREP_OVERRIDE_COPT);
		strcat(command_str,command_sub_str);
	    }
	}

	/* Translate UID/GID to local names */
	if (pGenRepData->ui.translateIDsTB_W)
	{
	    if (XmToggleButtonGetState(pGenRepData->ui.translateIDsTB_W))
	    {
		sprintf(command_sub_str," %s",AUDREP_MAPIDS_COPT);
		strcat(command_str,command_sub_str);
	    }
	}

	/* Display Statistics on Events */
	if (pGenRepData->ui.displayStatsTB_W)
	{
	    if (XmToggleButtonGetState(pGenRepData->ui.displayStatsTB_W))
	    {
		sprintf(command_sub_str," %s",AUDREP_STATS_COPT);
		strcat(command_str,command_sub_str);
	    }
	}

	/* Deselection Files */
	n = 0;
	XtSetArg(args[n], XmNselectedItemCount, &selectedCount); n++;
	XtSetArg(args[n], XmNselectedItems, &selectedItems); n++;
	XtGetValues(pGenRepData->ui.deselectionFileLST_W, args, n);
	if( selectedCount == 1)
	{
	    Boolean printRules;

	    if ( pGenRepData->ui.printRulesetTB_W )
	    {
		printRules = XmToggleButtonGetState(pGenRepData->ui.printRulesetTB_W);
	    }

	    if( XmStringGetLtoR(selectedItems[0], charset, &str) )
	    {
		sprintf(command_sub_str," %s %s/%s",
			(printRules ? AUDREP_PRDESEL_COPT : AUDREP_DESEL_COPT),
			AUDIT_DESELDIR,
			str);
		strcat(command_str,command_sub_str);
		XtFree(str);
	    }
	}

	/* Display Current Activity */
	if (pGenRepData->ui.displayCurrentTB_W)
	{
	    if (XmToggleButtonGetState(pGenRepData->ui.displayCurrentTB_W))
	    {
		if(currentAuditLog != (char *) NULL)
		{
		    sprintf(command_sub_str," %s %s", AUDREP_FOLLOW_COPT, currentAuditLog);
		    strcat(command_str,command_sub_str);
		}
		else
		{
		    return(False);
		}
	    }
	    else
	    {
		n = 0;
		XtSetArg(args[n],XmNselectedItemCount,&selectedCount); n++;
		XtSetArg(args[n], XmNselectedItems, &selectedItems); n++;
		XtGetValues(pGenRepData->ui.auditLogLST_W, args, n);
		if( selectedCount == 1)
		{
		    if( XmStringGetLtoR(selectedItems[0], charset, &str) )
		    {
			if(currentAuditLogDir != (char *) NULL)
			{
			    sprintf(command_sub_str," %s/%s", currentAuditLogDir, str);
			    strcat(command_str,command_sub_str);
			    XtFree(str);
			}
			else
			    return(False);
		    }
		}
		else
		{
		    /* Post an Error Box */
		    PostErrorDialog("AUDmsg_err_need_audit_log",
				    pGenRepData->ui.form_W,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    NULL);
		    return(False);
		}
	    }
	}

    }

    return(True);
}



void
AUDGenRepCreateFormCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    AUDGenRepPtr	pGenRepData;

    pGenRepData = (AUDGenRepPtr) XtMalloc(sizeof(AUDGenRep));

    if( pGenRepData )
    {
	bzero( (char *)pGenRepData, (int)sizeof(AUDGenRep) );
	pGenRepData->ui.form_W = mainWidget = w;
	SetUserData( w, pGenRepData );
    }
    else
    {
	MemoryError();
    }

    /* Initialization */
    currentAuditLogDir = strdup(AUDIT_LOGDIR);
    systemPageSize = getpagesize();
}




void
AUDGenRepCreateWidgetsCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget 		ancestor;
    AUDGenRepPtr	pGenRepData;


    /* all screens are hooked to the same data structure */
    if(ancestor = mainWidget)
    {
	GetUserData(ancestor, pGenRepData);

	if( pGenRepData )
	{
	    switch (*flag)
	    {
	      case AUDGENREP_LOGS:
		pGenRepData->ui.auditLogLST_W = w;
		break;
	      case AUDGENREP_SELFILES:
		pGenRepData->ui.selectionFileLST_W = w;
		break;
	      case AUDGENREP_DESELFILES:
		pGenRepData->ui.deselectionFileLST_W = w;
		break;
	      case AUDGENREP_DISPNOW:
		pGenRepData->ui.displayNowTB_W = w;
		break;
	      case AUDGENREP_DISPCUR:
		pGenRepData->ui.displayCurrentTB_W = w;
		break;
	      case AUDGENREP_SAVETOFILE:
		pGenRepData->ui.saveToFileTB_W = w;
		break;
	      case AUDGENREP_REPORTS:
		pGenRepData->ui.reportFilesTB_W = w;
		break;
	      case AUDGENREP_FILENAME:
		pGenRepData->ui.fileNameTF_W = w;
		break;
	      case AUDGENREP_FILENAME_LBL:
		pGenRepData->ui.fileNameLBL_W = w;
		break;
	      case AUDGENREP_FOLLOWCHANGE:
		pGenRepData->ui.followChangeTB_W = w;
		break;
	      case AUDGENREP_TRANSLATEIDS:
		pGenRepData->ui.translateIDsTB_W = w;
		break;
	      case AUDGENREP_DISPSTATS:
		pGenRepData->ui.displayStatsTB_W = w;
		break;
	      case AUDGENREP_PRINTRULES:
		pGenRepData->ui.printRulesetTB_W = w;
		break;
	      case AUDGENREP_FULLREC:
		pGenRepData->ui.fullRecTB_W = w;
		break;
	      case AUDGENREP_BRIEFREC:
		pGenRepData->ui.briefRecTB_W = w;
		break;
	      case AUDGENREP_CHANGEDIR:
		pGenRepData->ui.changeDirPB_W = w;
		break;
	      case AUDGENREP_OK:
		pGenRepData->ui.OKPB_W = w;
		break;
	      case AUDGENREP_APPLY:
		pGenRepData->ui.ApplyPB_W = w;
		break;
	      case AUDGENREP_CANCEL:
		pGenRepData->ui.CancelPB_W = w;
		break;
	      case AUDGENREP_HELP:
		pGenRepData->ui.HelpPB_W = w;
	      break;
	      case AUDGENREP_CLD_FORM:
		pGenRepData->ui.changeLogDirFRMD_W = w;
		break;
	      case AUDGENREP_CLD_TEXT:
		pGenRepData->ui.changeLogDirTF_W = w;
		break;
	      case AUDGENREP_CLD_OK:
		pGenRepData->ui.changeLogDirOKPB_W = w;
		break;
	      case AUDGENREP_CLD_CANCEL:
		pGenRepData->ui.changeLogDirCancelPB_W = w;
		break;
	      case AUDGENREP_CLD_HELP:
		pGenRepData->ui.changeLogDirHelpPB_W = w;
		break;
	      case AUDGENREP_AR_FORM:
		pGenRepData->ui.actualReportFRM_W = w;
		break;
	      case AUDGENREP_AR_TEXT:
		pGenRepData->ui.actualReportTXT_W = w;
		break;
	      case AUDGENREP_AR_TITLE:
		pGenRepData->ui.actualReportTitleTXT_W = w;
		break;
	      case AUDGENREP_AR_SCALE:
		pGenRepData->ui.actualReportSCALE_W = w;
		break;
	      case AUDGENREP_AR_CANCEL:
		pGenRepData->ui.actualReportCancelPB_W = w;
		break;
	      case AUDGENREP_AR_HELP:
		pGenRepData->ui.actualReportHelpPB_W = w;
		break;
	      case AUDGENREP_SW_FORM:
		pGenRepData->ui.statusWindowFRM_W = w;
		break;
	      case AUDGENREP_SW_TEXT:
		pGenRepData->ui.statusWindowTXT_W = w;
		break;
	      case AUDGENREP_SW_CLEAR:
		pGenRepData->ui.statusWindowClearPB_W = w;
		break;
	      case AUDGENREP_SW_DISMISS:
		pGenRepData->ui.statusWindowDismissPB_W = w;
		break;
	      case AUDGENREP_SW_HELP:
		pGenRepData->ui.statusWindowHelpPB_W = w;
		break;
	      case AUDGENREP_AND_FORM:
		pGenRepData->ui.askNewDirFRMD_W = w;
		break;
	      case AUDGENREP_AND_TEXT:
		pGenRepData->ui.askNewDirTF_W = w;
		break;
	      case AUDGENREP_AND_OK:
		pGenRepData->ui.askNewDirOKPB_W = w;
		break;
	      case AUDGENREP_AND_CANCEL:
		pGenRepData->ui.askNewDirCancelPB_W = w;
		break;
	      case AUDGENREP_AND_HELP:
		pGenRepData->ui.askNewDirHelpPB_W = w;
		break;
	      case AUDGENREP_FAR_FORM:
		pGenRepData->ui.fullRecFRMD_W = w;
		break;
	      case AUDGENREP_FAR_TEXT:
		pGenRepData->ui.fullRecTXT_W = w;
		break;
	      case AUDGENREP_FAR_CANCEL:
		pGenRepData->ui.fullRecCancelPB_W = w;
		break;
	      case AUDGENREP_AUDLOG_DATE_TEXT:
		pGenRepData->ui.auditLogDateTF_W = w;
		break;
	      case AUDGENREP_AUDLOG_DATE_LBL:
		pGenRepData->ui.auditLogDateLBL_W = w;
		break;
	      case AUDGENREP_AUDLOG_SIZE_TEXT:
		pGenRepData->ui.auditLogSizeTF_W = w;
		break;
	      case AUDGENREP_AUDLOG_SIZE_LBL:
		pGenRepData->ui.auditLogSizeLBL_W = w;
		break;
	      case AUDGENREP_LOGS_LBL:
		pGenRepData->ui.auditLogLBL_W = w;
		break;
	    }
	}
    }
}




void
AUDGenRepMapFormCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDGenRepPtr	pGenRepData;
    static int		firstTime = True;

    /* Replace the WM_DELETE_RESPONSE from f.kill with 
       controlled close (to resensitive the Generate Reports
       screen. These routines should eventually
       live in Utilities instead of account/AccCommon.
     */
    AccCommonReplaceSystemMenuClose(XtParent(w),
				    AUDGenRepCancelCallback);

    ancestor = search_for_parent(w, MAIN_WIDGET);

    if (ancestor)
    {
	GetUserData(ancestor, pGenRepData);

	if (pGenRepData)
	{
	    /* Fill the scrolled lists with the audit logs, selection files, 
	     *  deselection files.
	     */
	    AUDGetFileList(currentAuditLogDir, 
			   &audglobal.auditLogDirList,
			   &audglobal.auditLogDirListCount,
			   !firstTime, 
			   True);

	    AUDGetFileList(AUDIT_SELDIR, 
			   &audglobal.selectionFiles,
			   &audglobal.numSelectionFiles,
			   (audglobal.numSelectionFiles != 0),
			   True);

	    AUDGetFileList(AUDIT_DESELDIR, 
			   &audglobal.deselectionFiles,
			   &audglobal.numDeselectionFiles,
			   (audglobal.numDeselectionFiles != 0),
			   True);

	    /* Load each widget */
	    (void)XmListDeleteAllItems(pGenRepData->ui.auditLogLST_W);
	    (void)XmListDeleteAllItems(pGenRepData->ui.selectionFileLST_W);
	    (void)XmListDeleteAllItems(pGenRepData->ui.deselectionFileLST_W);

	    if(audglobal.auditLogDirListCount > 0)
	    {
		XmString	item;
		int		i;
		
		for(i = 0; i < audglobal.auditLogDirListCount; i++)
		{
		    if(AUDIsAuditLog(audglobal.auditLogDirList[i]))
		    {
			/* ZZZ sort entries */
		    
			item = XmStringCreate(audglobal.auditLogDirList[i], charset);
			(void)XmListAddItem(pGenRepData->ui.auditLogLST_W,
					    item,
					    0);
			XmStringFree(item);
		    }
		}
	    }

	    if(audglobal.numSelectionFiles > 0)
	    {
		XmString	item;
		int		i;
		
		for(i = 0; i < audglobal.numSelectionFiles; i++)
		{
		    /* ZZZ sort entries */
		    
		    item = XmStringCreate(audglobal.selectionFiles[i], charset);
		    (void)XmListAddItem(pGenRepData->ui.selectionFileLST_W,
					item,
					0);
		    XmStringFree(item);
		}
	    }

	    if(audglobal.numDeselectionFiles > 0)
	    {
		XmString	item;
		int		i;
		
		for(i = 0; i < audglobal.numDeselectionFiles; i++)
		{
		    /* ZZZ sort entries */
		    
		    item = XmStringCreate(audglobal.deselectionFiles[i], charset);
		    (void)XmListAddItem(pGenRepData->ui.deselectionFileLST_W,
					item,
					0);
		    XmStringFree(item);
		}
	    }

	    /* initialize all fields to default values */
	    AUDGenRepClearFields(&pGenRepData->ui);
		
	    /* set the deslection rules option off until one is selected */
	    XtSetSensitive(pGenRepData->ui.printRulesetTB_W, False);

	    /* set current activity off if audit is not running */
	    if (AUDGenRepAuditingNow())
	    {
		if( pGenRepData->ui.displayCurrentTB_W )
		{
		    XtSetSensitive(pGenRepData->ui.displayCurrentTB_W, True);
		}
	    }
	    else
	    {
		if( pGenRepData->ui.displayCurrentTB_W )
		{
		    XmToggleButtonSetState(pGenRepData->ui.displayCurrentTB_W,
				           False,
				           False);
		    XtSetSensitive(pGenRepData->ui.displayCurrentTB_W, False);
		}
	    }

	    /* set filename based on current option */
	    if(XmToggleButtonGetState(pGenRepData->ui.saveToFileTB_W) ||
	       XmToggleButtonGetState(pGenRepData->ui.reportFilesTB_W))
	    {
		XtSetSensitive(pGenRepData->ui.fileNameTF_W, True);
		XtSetSensitive(pGenRepData->ui.fileNameLBL_W, True);
	    }
	    else
	    {
		XtSetSensitive(pGenRepData->ui.fileNameTF_W, False);
		XtSetSensitive(pGenRepData->ui.fileNameLBL_W, False);
	    }

	    /* OK, we are no longer a virgin */
	    firstTime = False;

	}
    }
}



XtEventHandler
AUDGenRepHandleMouseEvents(w, data, event, cont)
Widget		w;
XtPointer	data;
XEvent		*event;
Boolean		*cont;
{
    Widget		ancestor;
    AUDGenRepPtr	pGenRepData;
    static Time 	lastTimeStamp = -1;
    Time		diff;
    XmTextPosition	cursorPos;
    char		*message = "Expanded Record Not Found!!";

    switch(event->type)
    {
      case ButtonPress:
	if(event->xbutton.button == MB1)
	{
	    if((diff = event->xbutton.time - lastTimeStamp) 
	       <= MOUSE_CLICK_INTERVAL )
	    {
		cursorPos = XmTextGetInsertionPosition(w);

		if (ancestor = mainWidget)
		{
		    GetUserData(ancestor, pGenRepData);
		    
		    if (pGenRepData)
		    {
			if(pGenRepData->data.brief)
			{
			    /* retrieve full record base on cursor position */
			    if(OnScreenPage != (AUDPagePtr) NULL)
			    {
				if(OnScreenPage->map)
				{
				    int i;

				    i = 1;
				    while(i < OnScreenPage->map->numRecs)
				    {
					if(cursorPos < OnScreenPage->map->briefCursorPos[i])
					{
					    break;
					}
					i++;
				    }
				    XmTextSetString(
						    pGenRepData->ui.fullRecTXT_W, 
						    OnScreenPage->map->fullMemoryPos[i-1]);
				}
				else
				{
				    XmTextSetString(pGenRepData->ui.fullRecTXT_W, 
						    message);
				}
			    }
			    else
			    {
				XmTextSetString(pGenRepData->ui.fullRecTXT_W, 
						message);
			    }
			    if(!isManagedFullRecWindow)
				VUIT_Manage(FULLREC_WIDGET);
			}
		    }
		}
	    }

	    lastTimeStamp = event->xbutton.time;
	}
	break;
	
      default:
	break;
    }
}




void
AUDGenRepApplyCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    Widget		sw;
    static Widget	shell_report,shell_status;
    AUDGenRepPtr	pGenRepData;
    static int	        firstTime = True;
    char		command_str[10000], *fileName;
    Boolean		listenToSTDOUT;

    ancestor = search_for_parent(w, MAIN_WIDGET);
    if (ancestor)
    {
	GetUserData(ancestor, pGenRepData);

	if (pGenRepData)
	{
	    /* INITIALIZATION */
	    if( !createdStatusWindow )
	    {
		Cardinal	n;
		Arg		args[5];
		n = 0;
		XtSetArg(args[n], XmNx, 820); n++;
		XtSetArg(args[n], XmNy, 0); n++;
		shell_status = XtAppCreateShell("Audit Report Status Window",
						 "XIsso",
						 topLevelShellWidgetClass,
						 display, 
						 args,
						 n);

		NEW_VUIT_Create(STATUS_WIDGET, 
				&sw, 
				shell_status);

		createdStatusWindow = True;
	    }
    
	    if( !createdReportWindow )
	    {
		XmTextScanType	selectionArray[2];
		int		selectionCount;
		Cardinal	n;
		Arg		args[2];

		shell_report = XtAppCreateShell("Audit Report",
						"XIsso",
						topLevelShellWidgetClass,
						display, 
						(Arg *) NULL,
						0);

		NEW_VUIT_Create(REPORT_WIDGET, 
				&sw, 
				shell_report);

   		/* Replace the WM_DELETE_RESPONSE from f.kill with 
		   controlled close (to resensitive the Generate Reports
		   screen. These routines should eventually
		   live in Utilities instead of account/AccCommon.
		   */
		AccCommonReplaceSystemMenuClose(shell_report,
						AUDGenRepActualReportCancelCallback);

		/* Put event handler on the text widget to
		   catch mouse click events.  This will
		   allow us to detect selection of an
		   audit record.
		   */
		XtAddEventHandler(pGenRepData->ui.actualReportTXT_W,
				  ButtonPress,
				  False,
				  (XtEventHandler)AUDGenRepHandleMouseEvents,
				  (XtPointer) NULL);

		/* Change clicking mechanism to indicate
		 *
		 * 1 click  = set cursor position
		 * 2 clicks = select line
		 *
		 */
		selectionCount = 0;
		selectionArray[selectionCount++] = XmSELECT_POSITION;
		selectionArray[selectionCount++] = XmSELECT_LINE;
		
		n = 0;
		XtSetArg(args[n], XmNselectionArray	, selectionArray); n++;
		XtSetArg(args[n], XmNselectionArrayCount, selectionCount); n++;
		XtSetValues(pGenRepData->ui.actualReportTXT_W, args, n);

		createdReportWindow = True;
	    }

	    if( !createdFullRecWindow )
	    {
		NEW_VUIT_Create(FULLREC_WIDGET, 
				&sw,
				pGenRepData->ui.actualReportFRM_W);
	    }

	    /* Build the command line string */
	    if (AUDGenRepBuildAuditToolString(pGenRepData,
					      command_str))
	    {
		/*
		 * state will be one of the following:
		 *
		 *   i) Report goes to a file
		 *  ii) Report goes to files sorted by auid
		 * iii) Report goes to the screen
		 */
		int state;

		if(XmToggleButtonGetState(pGenRepData->ui.saveToFileTB_W))
		   state = TOFILE;
		else if(XmToggleButtonGetState(pGenRepData->ui.reportFilesTB_W))
		    state = TOAUID;
		else
		    state = TOSCREEN;

		/* pop up the status window */
		VUIT_Manage(STATUS_WIDGET);
		XtRealizeWidget(shell_status);
		isManagedStatusWindow = True;

		if(state == TOSCREEN)
		{
		    /* Initialize buffer stuff */
		    AUDInitPageScheme();
		}

		/* run audit_tool */
		listenToSTDOUT = (state == TOSCREEN);

		fileName = (state == TOFILE) ? 
			pGenRepData->data.fileName : (char *) NULL;

		/* XXX Bug on other side */
		if(state == TOFILE || state == TOAUID)
		    listenToSTDOUT = True;

		if(invoke_audit_tool(app_context,
				     pGenRepData->ui.form_W,
				     command_str, 
				     listenToSTDOUT,
				     fileName, 
				     &ioData) == -1)
		{
		    PostErrorDialog("AUDmsg_err_invoke_audit_tool",
				    pGenRepData->ui.form_W,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    NULL,
				    NULL);
		}
		else
		{
		    reportAtBeginning = True;
		    reportAtEnd       = False;

		    /* start collecting data */
		    ioData->timerID =
			    XtAppAddTimeOut(app_context,
					    AUD_TIME_SLICE,
					    (XtTimerCallbackProc) listen_to_audit_tool,
					    (XtPointer) ioData);

		    if(state == TOSCREEN)
		    {
			VUIT_Manage(REPORT_WIDGET);
			XtRealizeWidget(shell_report);

			/* XXX Call the map callback myself since Motif isn't */
			AUDGenRepActualReportMapCallback();
		    }

		    /* Set the Sensitivity of the Gen Reports Screen to False */
		    XtSetSensitive(pGenRepData->ui.form_W, False);

		    /* Must set the sensitivity of the lists also (Motif Bug?) */
		    XtSetSensitive(pGenRepData->ui.auditLogLST_W, False);
		    XtSetSensitive(pGenRepData->ui.selectionFileLST_W, False);
		    XtSetSensitive(pGenRepData->ui.deselectionFileLST_W, False);
		}
	    }
	}
    }
}



void
AUDGenRepCancelCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDGenRepPtr	pGenRepData;

    ancestor = mainWidget;

    if (ancestor)
    {
	GetUserData(ancestor, pGenRepData);

	if (pGenRepData)
	{
	    if (isManagedStatusWindow)
	    {
		/* Unrealize the parent shell of the Status Window */
		XtUnrealizeWidget(XtParent(pGenRepData->ui.statusWindowFRM_W));

		VUIT_Unmanage(STATUS_WIDGET);
		isManagedStatusWindow = False;
	    }
	    if (isManagedReportWindow)
	    {
		/* Unrealize the parent shell of the Status Window */
		XtUnrealizeWidget(XtParent(pGenRepData->ui.actualReportFRM_W));

		VUIT_Unmanage(REPORT_WIDGET);
		isManagedReportWindow = False;
	    }
	}
    }

    if (isManagedFullRecWindow)
    {
	VUIT_Unmanage(FULLREC_WIDGET);
	isManagedFullRecWindow = False;
    }

    VUIT_Unmanage(MAIN_WIDGET);
}




void
AUDGenRepChangeDirCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDGenRepPtr	pGenRepData;

    VUIT_Manage(CHANGE_WIDGET);
    createdChangeLogDirWindow = True;

    ancestor = mainWidget;

    if(ancestor)
    {
	GetUserData(ancestor, pGenRepData);

	if( pGenRepData )
	{
	    CenterDialog(pGenRepData->ui.form_W,
			 pGenRepData->ui.changeLogDirFRMD_W);
	}
    }
}



void
AUDGenRepChangeLogDirMapCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDGenRepPtr	pGenRepData;

    ancestor = mainWidget;

    if(ancestor)
    {
	GetUserData(ancestor, pGenRepData);

	if( pGenRepData )
	{
	    /* Fill the text field with the current location */
	    if(currentAuditLogDir)
		XmTextSetString(pGenRepData->ui.changeLogDirTF_W,
				currentAuditLogDir);
	    else
		XmTextSetString(pGenRepData->ui.changeLogDirTF_W,
				"");
	}
    }

    isManagedChangeLogDirWindow = True;
}


void
AUDGenRepChangeLogDirOKCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDGenRepPtr	pGenRepData;
    char		*directory = (char *)NULL;

    ancestor = mainWidget;

    if(ancestor)
    {
	GetUserData(ancestor, pGenRepData);

	if( pGenRepData )
	{
	    /* read back the new directory location */
	    directory = StripWhiteSpace(XmTextGetString(pGenRepData->ui.changeLogDirTF_W));

	    if(directory != (char *)NULL)
	    {
		if(strcmp(directory, currentAuditLogDir))
		{
		    /* get the list of files in the directory */
		    /* ZZZ Needs serial number logic */
		    errno = ESUCCESS;

		    AUDGetFileList(directory, 
				   &audglobal.auditLogDirList,
				   &audglobal.auditLogDirListCount,
				   True, 
				   True);

		    /* if we have no entries, something went wrong */
		    if(audglobal.auditLogDirListCount == 0 &&
		       errno != ESUCCESS)
		    {
			switch(errno)
			{
			  case EPERM:
			  case EACCES:
			    /* ERROR */
			    PostErrorDialog("AUDmsg_err_access_dir",
					    ancestor,
					    NULL,
					    NULL,
					    NULL,
					    NULL,
					    NULL,
					    NULL);
			    break;

			  case ENOENT:
			    /* ERROR */
			    PostErrorDialog("AUDmsg_err_no_such_dir",
					    ancestor,
					    NULL,
					    NULL,
					    NULL,
					    NULL,
					    NULL,
					    NULL);
			    break;

			  case ENOTDIR:
			    /* ERROR */
			    PostErrorDialog("AUDmsg_err_not_dir",
					    ancestor,
					    NULL,
					    NULL,
					    NULL,
					    NULL,
					    NULL,
					    NULL);
			    break;
			    
			  default:
			    /* ERROR */
			    PostErrorDialog("AUDmsg_err_generic_dir",
					    ancestor,
					    NULL,
					    NULL,
					    NULL,
					    NULL,
					    NULL,
					    NULL);
			    break;
			} /* end switch */
		    }
		    else
		    {
			/* Set the Date and Size field to "" */
			XmTextSetString(pGenRepData->ui.auditLogDateTF_W,"");
			XmTextSetString(pGenRepData->ui.auditLogSizeTF_W,"");

			/* The directory was cool and we may have a list of files */
			(void)XmListDeleteAllItems(pGenRepData->ui.auditLogLST_W);

			if(audglobal.auditLogDirListCount > 0)
			{
			    XmString	item;
			    int		i;

			    for(i = 0; i < audglobal.auditLogDirListCount; i++)
			    {
				if(AUDIsAuditLog(audglobal.auditLogDirList[i]))
				{
				    /* ZZZ sort entries */
			    
				    item = 
				    XmStringCreate(audglobal.auditLogDirList[i],
						   charset);
				    (void)XmListAddItem(
						pGenRepData->ui.auditLogLST_W,
						item,
						0);
				    XmStringFree(item);
				}
			    }
			}

			/* update our notion of the current Audit Log Directory */
			if(currentAuditLogDir)
				free(currentAuditLogDir);
			currentAuditLogDir = strdup(directory);
		    }
		}
	    }
	}
    }	
	
    if(directory)
	XtFree(directory);

    VUIT_Unmanage(CHANGE_WIDGET);
    isManagedChangeLogDirWindow = False;
}


void
AUDGenRepChangeLogDirCancelCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    VUIT_Unmanage(CHANGE_WIDGET);
    isManagedChangeLogDirWindow = False;
}


void
AUDGenRepActualReportMapCallback(/*w, flag, info*/)
/*
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
*/
{
    AUDGenRepPtr	pGenRepData;

    /* get the widget ID */
    GetUserData(mainWidget, pGenRepData);

    if(pGenRepData)
    {
	Arg 	 args[10];
	Cardinal n;
	int 	 value, minimum, maximum;

	value   = 1;
	minimum = 1;
	maximum = 2;

	n = 0;
	XtSetArg(args[n], XmNvalue  , value); n++;
	XtSetArg(args[n], XmNminimum, minimum); n++;
	XtSetArg(args[n], XmNmaximum, maximum); n++;
	XtSetValues(pGenRepData->ui.actualReportSCALE_W, args, n);

	XtSetSensitive(pGenRepData->ui.actualReportSCALE_W, False);

	/* change cursor to a watch until report is generated under
	 * the following conditions:
	 * 
	 * 1) report is displaying to screen
	 * 2) we are not in continuous mode
	 *
	 */

	if(XmToggleButtonGetState(pGenRepData->ui.displayNowTB_W))
	{
	    WorkingOpen(pGenRepData->ui.statusWindowFRM_W);
	}
    }
}


void
AUDGenRepARScaleChangedCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    AUDGenRepPtr	pGenRepData;
    AUDPagePtr          page;
    static int		pageNum = -1;
    int			newPageNum;

    /* get the widget ID */
    GetUserData(mainWidget, pGenRepData);

    if(pGenRepData)
    {
	(void) XmScaleGetValue(pGenRepData->ui.actualReportSCALE_W,
			       &newPageNum);
	
	/* go fetch this page from the pool */
	if(page = AUDGetPage(newPageNum-1))
	{
	    /* update the on screen tracker */
	    if(OnScreenPage != (AUDPagePtr) NULL)
	    {
		OnScreenPage->isOnScreen = False;
	    }

	    /* refresh the display */
	    XmTextSetString(pGenRepData->ui.actualReportTXT_W,
			    page->data);

	    /* be nice to the user and set the scroll bar appropriately */
	    if((pageNum-newPageNum) == 1)
	    {
		/* moved back 1 so set to bottom */
		XmTextShowPosition(pGenRepData->ui.actualReportTXT_W,
                    XmTextGetLastPosition(pGenRepData->ui.actualReportTXT_W));
	    }
	    else 
	    {
		/* moving ahead, so set to top */
		XmTextShowPosition(pGenRepData->ui.actualReportTXT_W,
				   0);
	    }

	    pageNum = newPageNum;

	    /* update the active page */
	    OnScreenPage = page;
	    OnScreenPage->isOnScreen = True;
	}
    }
}


void
AUDGenRepActualReportCancelCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDGenRepPtr	pGenRepData;

    ancestor = mainWidget;

    if (ancestor)
    {
	GetUserData(ancestor, pGenRepData);

	if (pGenRepData)
	{
	    /* free up the current mess */
	    XmTextSetString(pGenRepData->ui.actualReportTXT_W, "");
	    XmTextSetString(pGenRepData->ui.actualReportTitleTXT_W, "");

	    /* Set the Sensitivity of the Gen Reports Screen to True */
            XtSetSensitive(pGenRepData->ui.form_W, True);

	    /* Must set the sensitivity of the lists also (Motif Bug?) */
	    if(!XmToggleButtonGetState(pGenRepData->ui.displayCurrentTB_W))
		XtSetSensitive(pGenRepData->ui.auditLogLST_W, True);
            XtSetSensitive(pGenRepData->ui.selectionFileLST_W, True);
            XtSetSensitive(pGenRepData->ui.deselectionFileLST_W, True);

	    /* Unrealize the parent shell */
	    XtUnrealizeWidget(XtParent(pGenRepData->ui.actualReportFRM_W));
	}
    }

    /* Free up all the buffers */
    AUDTermPageScheme();

    if (isManagedFullRecWindow)
    {
	VUIT_Unmanage(FULLREC_WIDGET);
	isManagedFullRecWindow = False;
    }

    VUIT_Unmanage(REPORT_WIDGET);

    /* If the audit_tool is still running, let's kill it */
    if(!reportAtEnd)
    {
	XtRemoveTimeOut(ioData->timerID);
	shutdown_audit_tool(ioData);
    }	
}


void
AUDGenRepStatusWindowMapCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
}


void
AUDGenRepStatusWindowClearCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDGenRepPtr	pGenRepData;

    ancestor = mainWidget;

    if (ancestor)
    {
	GetUserData(ancestor, pGenRepData);

	if (pGenRepData)
	{
	    XmTextSetString(pGenRepData->ui.statusWindowTXT_W, "");
	}
    }
}


void
AUDGenRepStatusWindowDismissCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDGenRepPtr	pGenRepData;

    ancestor = mainWidget;

    if (ancestor)
    {
	GetUserData(ancestor, pGenRepData);

	if (pGenRepData)
	{
	    DeIconify(XtParent(pGenRepData->ui.statusWindowFRM_W));
	    Iconify(XtParent(pGenRepData->ui.statusWindowFRM_W));
	}
    }
}


void
AUDGenRepAskNewDirMapCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDGenRepPtr	pGenRepData;

    ancestor = mainWidget;

    if (ancestor)
    {
	GetUserData(ancestor, pGenRepData);

	if (pGenRepData)
	{
	    /* clear the text field */
	    XmTextSetString(pGenRepData->ui.askNewDirTF_W,
			    "");
	}
    }

    isManagedAskNewDirWindow = True;
}


void
AUDGenRepAskNewDirOKCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    Widget		ancestor;
    AUDGenRepPtr	pGenRepData;
    char 		*pathname;

    ancestor = mainWidget;

    if (ancestor)
    {
	GetUserData(ancestor, pGenRepData);

	if (pGenRepData)
	{
	    /* read back the user's response */
	    pathname = StripWhiteSpace
		    (XmTextGetString(pGenRepData->ui.askNewDirTF_W));
	}
    }

    write_pathname(pathname, ioData);

    /* clean-up */
    VUIT_Unmanage(ASK_WIDGET);
    isManagedAskNewDirWindow = False;
}


void
AUDGenRepAskNewDirCancelCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    char *pathname = "";

    /* reporting will terminate */
    write_pathname(pathname, ioData);

    /* clean-up */
    VUIT_Unmanage(ASK_WIDGET);
    isManagedAskNewDirWindow = False;
}


void
AUDGenRepFullRecMapCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    /* for now, nothing */
    isManagedFullRecWindow = True;
}


void
AUDGenRepFullRecCancelCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmAnyCallbackStruct 	*info;
{
    VUIT_Unmanage(FULLREC_WIDGET);
    isManagedFullRecWindow = False;
}



void 
AUDReportFinished()
{
    Widget 		ancestor;
    AUDGenRepPtr	pGenRepData;

    reportAtEnd = True;

    /* all screens are hooked to the same data structure */
    if(ancestor = mainWidget)
    {
	GetUserData(ancestor, pGenRepData);

	if( pGenRepData )
	{
	    /* Don't re-sensitize if in display mode */
	    if(XmToggleButtonGetState(pGenRepData->ui.saveToFileTB_W) ||
	       XmToggleButtonGetState(pGenRepData->ui.reportFilesTB_W))
	    {
		PostWorkingDialog("AUDmsg_info_report_done",
				  ancestor,
				  NULL,
				  NULL,
				  NULL,
				  NULL,
				  NULL,
				  NULL);

		/* Set the Sensitivity of the Gen Reports Screen to True */
		XtSetSensitive(pGenRepData->ui.form_W, True);

		/* Must set the sensitivity of the lists also (Motif Bug?) */
		if(!XmToggleButtonGetState(pGenRepData->ui.displayCurrentTB_W))
		    XtSetSensitive(pGenRepData->ui.auditLogLST_W, True);
		XtSetSensitive(pGenRepData->ui.selectionFileLST_W, True);
		XtSetSensitive(pGenRepData->ui.deselectionFileLST_W, True);
	    }

	    /* Free up text data */
	    if(pGenRepData->data.fileName != (char *) NULL)
	    {
		XtFree(pGenRepData->data.fileName);
		pGenRepData->data.fileName = (char *) NULL;
	    }
	}
	
	/* change pointer shape back to normal */
	if(XmToggleButtonGetState(pGenRepData->ui.displayNowTB_W))
	    WorkingClose();
    }
}



void
AUDGenRepStatFileCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmListCallbackStruct 	*info;
{
    Widget 		ancestor;
    AUDGenRepPtr	pGenRepData;
    char		*str;
    char		audlogfilename[101];
    char		time_str[101];
    char		size_str[101];
    int 		ret;
    struct stat		buf;
    struct tm		*tm;



    /* all screens are hooked to the same data structure */
    if(ancestor = mainWidget)
    {
	GetUserData(ancestor, pGenRepData);

	if( pGenRepData )
	{
            if( info->selected_item_count == 0 )
	    {
		XmTextSetString(pGenRepData->ui.auditLogDateTF_W,"");
		XmTextSetString(pGenRepData->ui.auditLogSizeTF_W,"");
	    }
	    else if( XmStringGetLtoR(info->item, charset, &str) )
	    {
		sprintf(audlogfilename,"%s/%s", currentAuditLogDir, str);
		ret = stat(audlogfilename,&buf);
		if( ret == 0)
		{
		    tm = localtime(&(buf.st_ctime));
		    ret = strftime(time_str,25,"%c",tm);
		    if( ret != 0)
		    {
			if( pGenRepData->ui.auditLogDateTF_W )
		    	    XmTextSetString(pGenRepData->ui.auditLogDateTF_W, 
					    time_str);
		    }

		    sprintf(size_str,"%d",buf.st_size);
		    if( pGenRepData->ui.auditLogSizeTF_W )
		    	XmTextSetString(pGenRepData->ui.auditLogSizeTF_W, 
					size_str);

		}
		XtFree(str);
	    }
	}
    }
}


void
AUDGenRepDeselectionFileCallback(w, flag, info)
Widget 			w;
int 			*flag;
XmListCallbackStruct 	*info;
{
    Widget 		ancestor;
    AUDGenRepPtr	pGenRepData;



    /* all screens are hooked to the same data structure */
    if(ancestor = mainWidget)
    {
	GetUserData(ancestor, pGenRepData);

	if( pGenRepData )
	{
            if( info->selected_item_count == 0 )
		{
		/* desensitize the print output option */
		XtSetSensitive(pGenRepData->ui.printRulesetTB_W, False);
		}
	    else
		/* sensitize the print output option */
		XtSetSensitive(pGenRepData->ui.printRulesetTB_W, True);
	}
    }
}


void
AUDGenRepDisplayRepNowCallback(w, flag, info)
Widget 				w;
int 				*flag;
XmToggleButtonCallbackStruct 	*info;
{
    Widget 		ancestor;
    AUDGenRepPtr	pGenRepData;



    /* all screens are hooked to the same data structure */
    if(ancestor = mainWidget)
    {
	GetUserData(ancestor, pGenRepData);

	if( pGenRepData )
	{
	/* sensitize the audit logs scrolled list and text field */
	XtSetSensitive(pGenRepData->ui.auditLogLBL_W, True);
	XtSetSensitive(pGenRepData->ui.auditLogLST_W, True);
	XtSetSensitive(pGenRepData->ui.auditLogDateTF_W, True);
	XtSetSensitive(pGenRepData->ui.auditLogDateLBL_W, True);
	XtSetSensitive(pGenRepData->ui.auditLogSizeTF_W, True);
	XtSetSensitive(pGenRepData->ui.auditLogSizeLBL_W, True);

	/* desensitize the file name label and text field */
	XtSetSensitive(pGenRepData->ui.fileNameLBL_W, False);
	XtSetSensitive(pGenRepData->ui.fileNameTF_W, False);
	}
    }
}


void
AUDGenRepDisplayCurActCallback(w, flag, info)
Widget 				w;
int 				*flag;
XmToggleButtonCallbackStruct 	*info;
{
    Widget 		ancestor;
    AUDGenRepPtr	pGenRepData;



    /* all screens are hooked to the same data structure */
    if(ancestor = mainWidget)
    {
	GetUserData(ancestor, pGenRepData);

	if( pGenRepData )
	{
	/* desensitize the audit logs scrolled list and text field */
	XtSetSensitive(pGenRepData->ui.auditLogLBL_W, False);
	XtSetSensitive(pGenRepData->ui.auditLogLST_W, False);
	XtSetSensitive(pGenRepData->ui.auditLogDateLBL_W, False);
	XtSetSensitive(pGenRepData->ui.auditLogDateTF_W, False);
	XtSetSensitive(pGenRepData->ui.auditLogSizeLBL_W, False);
	XtSetSensitive(pGenRepData->ui.auditLogSizeTF_W, False);

	/* desensitize the file name label and text field */
	XtSetSensitive(pGenRepData->ui.fileNameLBL_W, False);
	XtSetSensitive(pGenRepData->ui.fileNameTF_W, False);
	}
    }
}


void
AUDGenRepSaveToFileCallback(w, flag, info)
Widget 				w;
int 				*flag;
XmToggleButtonCallbackStruct 	*info;
{
    Widget 		ancestor;
    AUDGenRepPtr	pGenRepData;



    /* all screens are hooked to the same data structure */
    if(ancestor = mainWidget)
    {
	GetUserData(ancestor, pGenRepData);

	if( pGenRepData )
	{
	/* sensitize the audit logs scrolled list and text field */
	XtSetSensitive(pGenRepData->ui.auditLogLBL_W, True);
	XtSetSensitive(pGenRepData->ui.auditLogLST_W, True);
	XtSetSensitive(pGenRepData->ui.auditLogDateLBL_W, True);
	XtSetSensitive(pGenRepData->ui.auditLogDateTF_W, True);
	XtSetSensitive(pGenRepData->ui.auditLogSizeLBL_W, True);
	XtSetSensitive(pGenRepData->ui.auditLogSizeTF_W, True);

	/* sensitize the file name label and text field */
	XtSetSensitive(pGenRepData->ui.fileNameLBL_W, True);
	XtSetSensitive(pGenRepData->ui.fileNameTF_W, True);
	}
    }
}


void
AUDGenRepCreateRepAllIDsCallback(w, flag, info)
Widget 				w;
int 				*flag;
XmToggleButtonCallbackStruct 	*info;
{
    Widget 		ancestor;
    AUDGenRepPtr	pGenRepData;



    /* all screens are hooked to the same data structure */
    if(ancestor = mainWidget)
    {
	GetUserData(ancestor, pGenRepData);

	if( pGenRepData )
	{
	/* sensitize the audit logs scrolled list and text field */
	XtSetSensitive(pGenRepData->ui.auditLogLBL_W, True);
	XtSetSensitive(pGenRepData->ui.auditLogLST_W, True);
	XtSetSensitive(pGenRepData->ui.auditLogDateLBL_W, True);
	XtSetSensitive(pGenRepData->ui.auditLogDateTF_W, True);
	XtSetSensitive(pGenRepData->ui.auditLogSizeLBL_W, True);
	XtSetSensitive(pGenRepData->ui.auditLogSizeTF_W, True);

	/* sensitize the file name label and text field */
	XtSetSensitive(pGenRepData->ui.fileNameLBL_W, True);
	XtSetSensitive(pGenRepData->ui.fileNameTF_W, True);
	}
    }
}
