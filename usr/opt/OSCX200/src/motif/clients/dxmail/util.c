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
static char rcs_id[] = "@(#)$RCSfile: util.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/03 00:03:04 $";
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
 */

/* util.c -- little miscellaneous utilities. */

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <ctype.h>
#include "decxmail.h"
#include "mlist.h"
#include "radio.h"
#include "toc.h"
#include "capsar.h"
#include <X11/cursorfont.h>
#include <decwcursor.h>
#include <Xm/MessageB.h>
#include <Xm/Label.h>
#include <Xm/MainW.h>		/*SM*/
#include <Xm/Protocols.h>		/* for XmAddWmProtocolCallback */
#include <Xm/AtomMgr.h>			/* for XmInternAtom */

extern void abort();
static Widget puntwidget = NULL;
static int	cant_continue = FALSE;

static void DoQuit()
{
  toc_cache();
  exit(1);
}

static void DoDumpCore()
{
  toc_cache();
  abort();
}


static void DoContinue()
{
/*
    register int i, j;
#ifdef NODESTROY
    XtUnmanageChild(puntwidget);
#else
    XtDestroyWidget(puntwidget);
#endif
    puntwidget = NULL;
    for (j=0 ; j<10 ; j++) {
	for (i=0 ; i<numFolders ; i++)
	    TocStartUpdate(folderList[i]);
	EnableEnablingOfButtons();
    }
*/
    cant_continue = FALSE;
}


/* Something went wrong; panic and quit. */

Punt(str)
  char *str;
{
    static XtCallbackRec yescallback[2] = { (XtCallbackProc) DoContinue, 
					      NULL, NULL};
    static XtCallbackRec nocallback[2] = { (XtCallbackProc) DoQuit, 
					     NULL, NULL};
    static XtCallbackRec cancelcallback[2] = { (XtCallbackProc) DoDumpCore,
						 NULL, NULL};
    static Arg args[] = {
	{XmNmessageString, (XtArgVal) NULL},
/*	{XmNokCallback, (XtArgVal) yescallback},   */
	{XmNokCallback, (XtArgVal) NULL},   
	{XmNcancelCallback, (XtArgVal) nocallback},
	{XmNhelpCallback, (XtArgVal) cancelcallback},
	{XmNdialogStyle, (XtArgVal) XmDIALOG_APPLICATION_MODAL},
    };
    char info[1000];
    extern int sys_nerr;
    extern char *sys_errlist[];
    Scrn scrn;
    int i;
    XEvent	event;

    cant_continue = TRUE;
    sprintf(info, "FATAL ERROR: %s\n(errno = %d %s)\n", str, errno,
	    (errno > 0 && errno < sys_nerr) ? sys_errlist[errno] : "");
    fprintf(stderr, "%s", info);
    if (theDisplay == NULL) abort();
    args[0].value = (XtArgVal) XmStringCreateSimple(info);
    scrn = NULL;
    for (i=0 ; i<numScrns && scrn == NULL ; i++)
	if (scrnList[i]->mapped) scrn = scrnList[i];
    if (scrn == NULL) abort();
    if (puntwidget != NULL)
#ifdef NODESTROY
	XtUnmanageChild(puntwidget);
#else
	XtDestroyWidget(puntwidget);
#endif
    puntwidget = XmCreateWarningDialog(scrn->parent, "punt",
				      args, (int) XtNumber(args));
    XtManageChild(puntwidget);
    XtUnmanageChild(XmMessageBoxGetChild(puntwidget, XmDIALOG_OK_BUTTON));
    puntwidget = GetParent(puntwidget);
    XtRealizeWidget(puntwidget);
    XtPopup(puntwidget, XtGrabNonexclusive);
    XtFree((char *) args[0].value);

/* PJS: NO WAY! This does not allow to continue, and adds stack entries!
    XtMainLoop();
*/
    while (cant_continue) {
	XtNextEvent(&event);

#ifdef IGNOREDDIF
	XtDispatchEvent(&event); 
	DEBUG(("Calling XtDispatchEvent --- not DPS \n"));
#else
/* Precede all calls to XtDispatchEvent() with a call to XDPSDispatchEvent(): */
	if(!(dps_exists && XDPSDispatchEvent(&event)))
	  {
	    XtDispatchEvent(&event);
	    DEBUG(("XDPSDispatchEvent fails, calling XtDispatchEvent..\n"));
	  }
#endif /* IGNOREDDIF */	
      }
    DEBUG(("Exited Punt() to attempt to continue.\n"));
}


void NoOp() {}

int myopen(path, flags, mode)
char *path;
int flags, mode;
{
int fid;

    if (path == NULL) Punt("Attempted to open NULL filename!");
    fid = open(path, flags, mode);
    DEBUG((" OPEN %d : %s\n", fid, path));
    return fid;
}


FILE *myfopen(path, mode)
char *path, *mode;
{
    FILE *result;
    if (path == NULL) Punt("Attempted to open NULL filename!");
    result = fopen(path, mode);
    DEBUG(("FOPEN %d : %s\n", (result == NULL) ? -1 : result->_file, path));
    return result;
}



int myclose(fid)
{
    DEBUG((" CLOSE %d\n", fid));
    if (close(fid) < 0) DEBUG(("Error in myclose!"));
}


int myfclose(file)
FILE *file;
{
    DEBUG(("FCLOSE %d\n", file->_file));
    if (fclose(file) < 0) DEBUG(("Error in myfclose!"));
}



/* Recursively delete an entire directory.  Nasty. */

void NukeDirectory(path)
char *path;
{
    char **argv;
    argv = MakeArgv(3);
    argv[0] = "rm";
    argv[1] = "-rf";
    argv[2] = path;
    DoCommand(argv, (char *) NULL, "/dev/null");
    XtFree((char *) argv);
}



/* Return a unique file name in the given directory. */

char *MakeNewTempFileNameInSameDir(filename)
char *filename;
{
    static char name[400], *ptr;
    static int  uniqueid = 0;
    ptr = rindex(filename, '/');
    if (ptr == NULL)
	Punt("Illegal argument passed to MakeNewTempFileNameInSameDir");
    else
        *ptr = 0;
    do {
	(void) sprintf(name, "%s/decxmail_%ld_%d", filename, getpid(),
		       uniqueid++);
    } while (FileExists(name));
    *ptr = '/';
    return name;
}

/* Return a unique file name. */

char *MakeNewTempFileName()
{
    char filename[500];
    sprintf(filename, "%s/foo", tempDir);
    return MakeNewTempFileNameInSameDir(filename);
}


/* Make an array of string pointers big enough to hold n+1 entries. */

char **MakeArgv(n)
  int n;
{
    char **result;
    result = ((char **) XtMalloc((unsigned) (n+1) * sizeof(char *)));
    result[n] = 0;
    return result;
}


char **ResizeArgv(argv, n)
  char **argv;
  int n;
{
    argv = ((char **)XtRealloc((char *)argv, (unsigned)(n+1) * sizeof(char *)));
    argv[n] = 0;
    return argv;
}

/* Open a file, and punt if we can't. */

FILE *FOpenAndCheck(name, mode)
char *name, *mode;
{
    FILE *result;
    char str[300];
    result = myfopen(name, mode);
    if (result == NULL) {
	sleep(2);
	result = myfopen(name, mode);
	if (result == NULL) {
	    sprintf(str, "Error in FOpenAndCheck on file %s", name);
	    Punt(str);
	}
    }
    return result;
}

int OpenAndCheck(path, flags, mode)
char *path;
int flags, mode;
{
    int result;
    char str[300];
    result = myopen(path, flags, mode);
    if (result < 0) {
	sleep(2);
	result = myopen(path, flags, mode);
	if (result < 0) {
	    sprintf(str, "Error in OpenAndCheck on file %s", path);
	    Punt(str);
	}
    }
    return result;
}


/* Read one line from a file. */

static char *DoReadLine(fid, lastchar)
  FILE *fid;
  char lastchar;
{
    static char *buf;
    static int  maxlength = 0;
    char   *ptr, c;
    register int     length = 0;
    ptr = buf;
    c = ' ';
    while (c != '\n' && !feof(fid)) {
	c = getc(fid);
	if (length++ > maxlength - 5) {
	    if (maxlength)
		buf = XtRealloc(buf, (unsigned) (maxlength *= 2));
	    else
		buf = XtMalloc((unsigned) (maxlength = 512));
	    ptr = buf + length - 1;
	}
	*ptr++ = c;
    }
    if (!feof(fid) || length > 1) {
	*ptr = 0;
	*--ptr = lastchar;
	return buf;
    }
    return NULL;
}


char *ReadLine(fid)
  FILE *fid;
{
    return DoReadLine(fid, 0);
}


/* Read a line, and keep the CR at the end. */

char *ReadLineWithCR(fid)
  FILE *fid;
{
    return DoReadLine(fid, '\n');
}



/* Delete a file, and Punt if it fails. */

DeleteFileAndCheck(name)
  char *name;
{
    if (strcmp(name, "/dev/null") != 0 && unlink(name) == -1)
	if (errno != ENOENT)
	    Punt("DeleteFileAndCheck failed!");
}

CopyFileAndCheck(from, to)
  char *from, *to;
{
    int fromfid, tofid, n;
    char buf[512];
    fromfid = myopen(from, O_RDONLY, 0666);
    tofid = myopen(to, O_WRONLY | O_TRUNC | O_CREAT, 0666);
    if (fromfid < 0 || tofid < 0) Punt("CopyFileAndCheck failed!");
    do {
	n = read(fromfid, buf, 512);
	if (n) (void) write(tofid, buf, n);
    } while (n);
    (void) myclose(fromfid);
    (void) myclose(tofid);
}

RenameAndCheck(from, to)
  char *from, *to;
{
/* Non-system-V system call: is it also BSD or just ULTRIX...? */
    if (rename(from, to) == -1) {
	if (errno != EXDEV) Punt("RenameAndCheck failed!");
	CopyFileAndCheck(from, to);
	DeleteFileAndCheck(from);
    }
}


void WriteAndCheck(fid, buf, length)
int fid;
char *buf;
int length;
{
    if (write(fid, buf, length) != length)
	Punt("Error in WriteAndCheck!");
}

void LSeekAndCheck(fid, offset, whence)
int fid, offset, whence;
{
    off_t offset_t; /* 64 bit port */
    offset_t = offset;
    if (lseek(fid, offset_t, offset) < 0)
	Punt("Error in LSeekAndCheck!");
}

void MkDirAndCheck(name, mode)
char *name;
int mode;
{
    if (mkdir(name, mode) != 0)
	Punt("Error in MkDirAndCheck!");
}


char *MallocACopy(str)
  char *str;
{
    if (str != NULL)
    	return XtNewString(str);
    else
	return XtNewString("");
}



#ifndef F_OK
#define F_OK 0
#endif

FileExists(file)
  char *file;
{
    return (access(file, F_OK) == 0);
}

LastModifyDate(file)
  char *file;
{
    struct stat buf;
    if (stat(file, &buf)) return -1;
    return buf.st_mtime;
}

Boolean IsDirectory(file)
char *file;
{
    struct stat buf;
    if (stat(file, &buf) || !(buf.st_mode & S_IFDIR)) return FALSE;
    return TRUE;
}
    

GetFileLength(file)
char *file;
{
    struct stat buf;
    if (stat(file, &buf)) return -1;
    return buf.st_size;
}



ChangeLabel(widget, str)
Widget widget;
char *str;
{
    static Arg arglist[] = {XmNlabelString, NULL};
    arglist[0].value = (XtArgVal) XmStringCreateSimple(str);
    XtSetValues(widget, arglist, XtNumber(arglist));
    XmStringFree((char *) arglist[0].value);
}





Widget CreateTitleBar(scrn, name)
Scrn scrn;
char *name;
{
    Widget result;
    int height;
    static Arg arglist[] = {
	{XmNlabelString, NULL},
    };
    Arg args[2];
    arglist[0].value = (XtArgVal) XmStringCreateSimple(Version());
    result = XmCreateLabel(scrn->widget, name, arglist, XtNumber(arglist));
    height = GetHeight(result);
    XtSetArg(args[0], XmNpaneMinimum, height);
    XtSetArg(args[1], XmNpaneMaximum, height);
    XtSetValues(result, args, 2);
    XtManageChild(result);
    XmStringFree((char *) arglist[0].value);
    return result;
}



Feep()
{
    XBell(theDisplay, 50);
}



MsgList CurMsgListOrCurMsg(toc, scrn)
  Toc toc;
  Scrn scrn;
{
    MsgList result;
    Msg curmsg;
    if (toc == NULL) return MakeNullMsgList();
    result = TocCurMsgList(toc, scrn);
    if (result->nummsgs == 0 && defAffectCurIfNullSelection &&
	    (curmsg = TocGetCurMsg(toc))) {
	FreeMsgList(result);
	result = MakeSingleMsgList(curmsg);
    }
    return result;
}



/*
 * Get the list of selected messages, no matter what toc they're in.
 */

MsgList GetSelectedMsgs(scrn)
Scrn scrn;
{
    MsgList result;
    register int i;
    if (scrn->toc && scrn->mapped) {
      result = TocCurMsgList(scrn->toc, scrn);
      if (result) return result;
    }
    return NULL;
}


Toc SelectedToc(scrn)
Scrn scrn;
{
    char *name;
    int numSelections;
    register int i;
    Widget widget;
    int *entryNumbers;
    TagRec tag;

    if (LastPopupToc) return LastPopupToc;
    if (!defSVNStyle) {
	name = RadioGetCurrent(scrn->folderradio);
	if (name) return TocGetNamed(name);
	else return NULL;
    } else {
	if (scrn->tocwidget == NULL) return NULL;
	widget = scrn->tocwidget;
	TocDisableRedisplay(widget);
	numSelections = DXmSvnGetNumSelections(widget);
	entryNumbers = (int *) XtMalloc(numSelections * sizeof(int *));
	DXmSvnGetSelections(widget, entryNumbers, NULL, NULL, numSelections);
	for (i=0 ; i < numSelections ; i++) {
	    tag.tagValue = DXmSvnGetEntryTag(widget, entryNumbers[i]);
	    if (tag.tagFields.msgNumber == 0) {
		XtFree((char *)entryNumbers);
		TocEnableRedisplay(widget);
		return folderList[tag.tagFields.tocNumber];
	    }
	}
	XtFree((char *)entryNumbers);
	TocEnableRedisplay(widget);
    }
    return NULL;
}


char *SelectedSeqName(scrn)
Scrn scrn;
{
    char *result;
    result = RadioGetCurrent(scrn->seqradio);
    return (result) ? result : "all";
}


int strncmpIgnoringCase(str1, str2, length)
char *str1, *str2;
int length;
{
    register int i, diff;
    for (i=0 ; i<length ; i++, str1++, str2++) {
	diff = ((*str1 >= 'A' && *str1 <= 'Z') ? (*str1 + 'a' - 'A') : *str1) -
	       ((*str2 >= 'A' && *str2 <= 'Z') ? (*str2 + 'a' - 'A') : *str2);
	if (diff) return diff;
    }
    return 0;
}


void ExecOnDouble(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    static Widget lastwidget = NULL;
    static int lastx, lasty;
    static Time lasttime;
    XtActionProc func;
    Cardinal num;
    if (lastwidget == w && event->xbutton.time - lasttime < 500 &&
	  ABS(lastx - event->xbutton.x) < 5 &&
	  ABS(lasty - event->xbutton.y) < 5 &&
	  *num_params != 0) {
	func = NameToFunc(params[0]);
	params++;
	num = *num_params - 1;
	(*func)(w, event, params, &num);
    }
    lastwidget = w;
    lastx = event->xbutton.x;
    lasty = event->xbutton.y;
    lasttime = event->xbutton.time;
}


Cursor MakeCursor(widget, name)
Widget widget;
char *name;
{
    static Cursor cache[decw$c_num_glyphs / 2] = {NULL};
    static XColor fore = {0, 65535, 0, 0}; /* red */
    static XColor back = {0, 65535, 65535, 65535}; /* white */
    static fontfailed = FALSE;
    Cursor *result;
    XrmValue from, to;
    int num, half;
    XFontStruct *f;
    Font cfont;
    if (strncmp(name, "decw", 4) == 0) {
	num = atoi(name + 4);
	if (num >= 0 && num < decw$c_num_glyphs) {
	    half = num / 2;
	    if (cache[half] == NULL && !fontfailed) {
		f = XLoadQueryFont(XtDisplay(widget), "decw$cursor");
		if (f != NULL) {
		    cfont = f->fid;
		    cache[half] = XCreateGlyphCursor(XtDisplay(widget),
						     cfont, cfont, num, num+1,
						     &fore, &back);
		    XFreeFont(XtDisplay(widget), f);
		} else {
		    fontfailed = TRUE;
		}
	    }
	    if (cache[half]) return cache[half];
	    name = "watch";
	}
    }
    from.addr = name;
    from.size = strlen(name);
    XtConvert(widget, XtRString, &from, XtRCursor, &to);
    result = (Cursor *) to.addr;
    return *result;
}



static int longopcnt = 0;

void BeginLongOperation()
{
    register int i;
    Scrn scrn;
    if (longopcnt == 0 && numScrns > 0) {
	for (i=0 ; i<numScrns ; i++) {
	    scrn = scrnList[i];
	    if (scrn->mapped) {
		XDefineCursor(XtDisplay(scrn->parent), XtWindow(scrn->parent),
			      scrn->sleepcursor);
	    }
	}
	PickBeginLongOperation();
	XFlush(XtDisplay(scrn->parent));
    }
    longopcnt++;
}


void EndLongOperation()
{
    register int i;
    Scrn scrn;
    if (longopcnt <= 1) {
	for (i=0 ; i<numScrns ; i++) {
	    scrn = scrnList[i];
	    if (scrn->mapped) {
/* RHM: Change to use default cursor rather than force it */
		XDefineCursor(XtDisplay(scrn->parent), XtWindow(scrn->parent),
			      NULL);
	    }
	}
	PickEndLongOperation();
	longopcnt = 0;
    } else longopcnt--;
}
	



/*
 * Called by a work proc that wants to really make sure no events are
 * outstanding.  Returns TRUE if it's ok.
 */


Boolean CheckWorkOK()
{
    XSync(theDisplay, FALSE);
    return (XPending(theDisplay) == 0);
}


typedef struct _EventQueueRec {
    XEvent event;
    struct _EventQueueRec *next;
} EventQueueRec, *EventQueue;

static EventQueue queue = NULL;


/*
 * Take any events that are queued up and put them into a temporary
 * local queue. 
 */

void SaveQueuedEvents()
{
    EventQueue temp;
    while (XPending(theDisplay)) {
	temp = XtNew(EventQueueRec);
	XNextEvent(theDisplay, &(temp->event));
	switch (temp->event.type) {
	  case KeyPress:
	  case KeyRelease:
	  case ButtonPress:
	  case ButtonRelease:
	  case MotionNotify:
	  case EnterNotify:
	  case LeaveNotify:
	    temp->next = queue;
	    queue = temp;
	    break;
	  default:
#ifdef IGNOREDDIF
	    XtDispatchEvent(&(temp->event));
	    DEBUG(("Calling XtDispatchEvent --- not DPS \n"));
#else
/* Precede all calls to XtDispatchEvent() with a call to 
   XDPSDispatchEvent(): */
	    if(!(dps_exists && XDPSDispatchEvent(&(temp->event)))){
	      XtDispatchEvent(&(temp->event));
	      DEBUG(("XDPSDispatchEvent fails, calling XtDispatchEvent..\n"));
	    }
#endif /* IGNOREDDIF */
	    XtFree((char *) temp);
	}
    }
}

/*
 * Restore events that were queued up by SaveQueuedEvents. 
 */

void RestoreQueuedEvents()
{
    EventQueue temp;

    while (queue) {
	temp = queue;
	queue = queue->next;
	XPutBackEvent(theDisplay, &(temp->event));
	XtFree((char *) temp);
    }
}


/*
 * Extract the first segment of a compound string, and return it as a normal C
 * string.
 */

char *ExtractStringFromCompoundString(cstr)
    XmString cstr;
{
    XmStringContext ctx;
    XmStringCharSet charset;
    XmStringDirection direction;
    Boolean separator;
    char *text;

    if(FALSE == XmStringInitContext (&ctx, cstr)) {
	DEBUG(("init failed\n"));
        return (char *) NULL;
    }
    if (FALSE == XmStringGetNextSegment (ctx, &text, &charset, &direction,
                &separator)) {
	DEBUG(("decode failed\n"));
	return (char *)NULL;
    }
    return text;	
}


char *GetApplicationResourceAsString(name, class)
char *name, *class;
{
    char rname[500], rclass[500], *type;
    XrmValue value;

    (void) sprintf(rname, "%s.%s", progName, name);
    (void) sprintf(rclass, "%s.%s", PROGCLASS, class);
    XrmGetResource(DefaultDB, rname, rclass, &type, &value);
    return (char *) value.addr;
}
    


/*
 * Returns what the magic number for the given file is.  Blech.
 */

int GetMagicNumber(filename)
char *filename;
{
    int fid = OpenAndCheck(filename, O_RDONLY, 0666);
    int data;
    if (sizeof(int) != read(fid, (char *) &data, sizeof(int)))
	data = 0;
    (void) myclose(fid);
    return data;
}


/*
 * Determine if the given file descriptor is one containing ddif data.
 * Returns TRUE if it does.  (Assumes that we're currently set up to read from
 * the beginning of the file, and leaves it that way too.)
 */

Boolean DDIFFileCheck(fid)
int fid;
{
    int data;
    
    if (sizeof(int) != read(fid, (char *) &data, sizeof(int)))
	data = 0;
#ifndef L_SET
#define L_SET 0
#endif
    LSeekAndCheck(fid, 0, L_SET);
    return (data == DDIF_MAGICNO || data == DOTS_MAGICNO || data == DTIF_MAGICNO);
}
WasteFolder(foldername)
char	*foldername;
{
char	**argv, *rmf_result;

/* PJS: Move ('refile') ALL messages from this folder into the wastebasket... */
    argv = MakeArgv(7);
    argv[0] = refileCmd;
    argv[1] = "-src";
    argv[2] = XtMalloc(strlen(foldername)+2);
    sprintf(argv[2], "+%s",foldername);
    argv[3] = "+wastebasket";
    argv[4] = "all";
    argv[5] = "-nolink";
    argv[6] = 0;
    DoCommand(argv, (char *) NULL, "/dev/null");

/* ...then attempt to do a 'rmf'. */
    argv[0] = rmfCmd;
    argv[1] = "-nointera";
/*  argv[2] from the above... */
    argv[3] = 0;
    rmf_result = DoCommandToString(argv);
    DEBUG(("Rmf   : '%s'\n",rmf_result));
    if (rmf_result[0] != '\0')
	Warning(toplevel,NULL,rmf_result);

    XtFree((char *)argv[2]);
    XtFree((char *)argv);
    XtFree((char *)rmf_result);
}

#ifdef __osf__
/*
 * NAME: ftime
 *                                                                    
 * FUNCTION: returns the number of seconds (up to 1000 milliseconds) since epoch.
 *                                                                    
 * RETURNS:  
 */  

/* from old timeb.h */
struct timeb {
	time_t	time;
	u_short	millitm;
	short	timezone;
	short	dstflag;
};

ftime(tp)
	register struct timeb *tp;
{
	struct timeval t;
	struct timezone tz;

	if (gettimeofday(&t, &tz) < 0)
		return (-1);
	tp->time = t.tv_sec;
	tp->millitm = t.tv_usec / 1000;
	tp->timezone = tz.tz_minuteswest;
	tp->dstflag = tz.tz_dsttime;
}
#endif

void AddProtocols (widget, delete, save)
Widget	widget;
XtCallbackProc	delete;
XtCallbackProc	save;
{
    static Atom	DwcWmDeleteWindowAtom = None;
    static Atom	DwcWmSaveYourselfAtom = None;

    /*
    ** Try to get the atoms necessary to do this job.
    */
    if (DwcWmDeleteWindowAtom == None)
    {
	DwcWmDeleteWindowAtom = XmInternAtom
	    (XtDisplay(widget), "WM_DELETE_WINDOW", False);
    }
    if (DwcWmSaveYourselfAtom == None)
    {
	DwcWmSaveYourselfAtom = XmInternAtom
	    (XtDisplay(widget), "WM_SAVE_YOURSELF", False);
    }

    /*
    ** Add the callbacks that do what's needed.
    */
    if (delete != NULL)
    {
	XmAddWMProtocolCallback
	    (widget, DwcWmDeleteWindowAtom, (XtCallbackProc)delete, NULL);
	XmActivateWMProtocol (widget, DwcWmDeleteWindowAtom);
    }

    if (save != NULL)
    {
	XmAddWMProtocolCallback
	    (widget, DwcWmSaveYourselfAtom, (XtCallbackProc)save, NULL);
	XmActivateWMProtocol (widget, DwcWmSaveYourselfAtom);
    }
}

