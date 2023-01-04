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
static char rcs_id[] = "@(#)$RCSfile: toc.c,v $ $Revision: 1.1.5.5 $ (DEC) $Date: 1994/01/11 22:06:49 $";
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

/* toc.c -- handle things in the toc widget. */

#include <sys/types.h>  /* for stat() call */
#include <sys/stat.h>
#ifdef __osf__
#define _POSIX_SOURCE
#define _BSD
#endif
#include <sys/dir.h>
#ifdef __osf__
#undef _POSIX_SOURCE
#undef _BSD
#endif
#include <sys/file.h>
#include "decxmail.h"
#include <Vendor.h>
#include <X11/Xatom.h>
/* #include <DXm/DXmSvn.h> included in externs.h */
#include "msg.h"
#include "mlist.h"
#include "radio.h"
#include "toc.h"
#include "actionprocs.h"
#include "tocintrnl.h"
#include "tocutil.h"

#define O_RDONLY 00000000
/*	PUBLIC ROUTINES 	*/



static char *parentDir;
static int IsDir(ent)
struct direct *ent;
{
    char str[PATH_MAX];
    struct stat buf;
/* PJS: If this is a "." or a ".." directory entry, ignore it! */
    if (ent->d_name[0] == '.')
	if (ent->d_name[1] == 0 || (ent->d_name[1] == '.' && ent->d_name[2] == 0))
	    return FALSE;
    (void) sprintf(str, "%s/%s", parentDir, ent->d_name);
    (void) stat(str, &buf);
    return (buf.st_mode & S_IFMT) == S_IFDIR;
}

static void MakeSureFolderExists(namelistptr, numfoldersptr, name)
struct direct ***namelistptr;
int *numfoldersptr;
char *name;
{
    register int i;
    extern alphasort();
    char str[200];
    for (i=0 ; i<*numfoldersptr ; i++)
	if (strcmp((*namelistptr)[i]->d_name, name) == 0) return;
    (void) sprintf(str, "%s/%s", mailDir, name);
    (void) mkdir(str, 0700);
    *numfoldersptr = scandir(mailDir, namelistptr, IsDir, alphasort);
    for (i=0 ; i<*numfoldersptr ; i++)
	if (strcmp((*namelistptr)[i]->d_name, name) == 0) return;
    Punt("Can't create new mail folder!");
}

/* A file '$HOME/.decxmailcache' may exist for directing DXMAIL where to
 * find 'inc' files for specific folders.
 * FORMAT:   ___folder___inc-file   (--- = white-space)
 */
static void LoadCheckFiles()
{
    FILE *fid;
    char str[1024], *ptr, *ptr2;
    register int i;
    extern char dxm_maildrop[];

DEBUG(("Loading inc file names...\n"));

    ptr2 = NULL;
    (void) sprintf(str, "%s/.decxmailcheck", homeDir);
    fid = myfopen(str, "r");
    if (fid) {
	while (ptr = ReadLine(fid)) {
	    while (*ptr == ' ' || *ptr == '\t') ptr++;
	    ptr2 = ptr;
	    while (*ptr2 && *ptr2 != ' ' && *ptr2 != '\t') ptr2++;
	    if (*ptr2 == 0) continue;
	    *ptr2++ = 0;
	    while (*ptr2 == ' ' || *ptr2 == '\t') ptr2++;
	    if (*ptr2 == 0) continue;
DEBUG(("D: +%s: %s\n",ptr,ptr2));
	    for (i=0 ; i<numFolders ; i++) {
		if (strcmp(ptr, folderList[i]->foldername) == 0) {
		    folderList[i]->incfile = MallocACopy(ptr2);
		    break;
		}
	    }
	}
	myfclose(fid);
     } else if (popHostName) {
       InitialFolder->inchost = MallocACopy(popHostName);
    } else {

/*      Bug fix for QAR # 16724:  Need to look up MailDrop as "inc" does.
 */
        InitialFolder->incfile = NULL;

        /* MAILDROP environment variable has first priority! */
        ptr = getenv("MAILDROP");

        /* If no mail drop then next check .mh_profile for MailDrop entry! */
        if ( !ptr && (strlen(dxm_maildrop) > 0) )
            ptr = &dxm_maildrop[0];

        /* If still no mail drop check env variables "MAIL" and "mail". */
	if (ptr == NULL) ptr = getenv("MAIL");
	if (ptr == NULL) ptr = getenv("mail");

        /* If still no mail drop ... use system default. */
	if (ptr == NULL) {
	    ptr = getenv("USER");
	    if (ptr) {
		(void) sprintf(str, "/usr/spool/mail/%s", ptr);
		ptr = str;
	    }
	}
	if (ptr)
	    InitialFolder->incfile = MallocACopy(ptr);

	if (ptr) ptr = "(nil)";
	if (ptr2) ptr2 = "(nil)";
	DEBUG(("E: +%s: %s\n",ptr,ptr2));
    }
}


static int totalNumFolders;
extern alphasort();
/* Find any subfolders */

static int TocLevel(name)
char *name;
{
    int i;
    int level;
    char *ptr;

    level = 0;

    if (!name) return level;

    ptr = name;
    while (*ptr) {
	if (*ptr == '/') level++;
	ptr++;
    }
    return level;
}
static int TocFindSubfolders(pos, parent)
int	pos;
char	*parent;
{
    Toc toc;
    struct direct **subfolderlist;
    register int i, j;
    char str[PATH_MAX];
    int numSubFolders;
    int numSubSubFolders = 0;
    int totalSubFolders = 0;

    parentDir = parent;
    numSubFolders = scandir(parent, &subfolderlist, IsDir, alphasort);
    if (numSubFolders) {
	totalSubFolders = numSubFolders;
	totalNumFolders += numSubFolders;
	folderList = (Toc *) XtRealloc((char *) folderList, 
				(unsigned) totalNumFolders * sizeof(Toc));
	j = pos;
	for (i = 0; i < numSubFolders; i++) {
	    toc = folderList[j++] = TUMalloc();
	    (void) sprintf(str, "%s/%s",
			folderList[pos-1]->foldername, subfolderlist[i]->d_name);
	    toc->foldername = MallocACopy(str);
	    toc->level = TocLevel(toc->foldername);
	    (void) sprintf(str, "%s/%s", mailDir, toc->foldername);
	    toc->path = MallocACopy(str);
	    toc->visible = False;
	    numSubSubFolders = TocFindSubfolders(j, toc->path);
	    j += numSubSubFolders;
	    totalSubFolders += numSubSubFolders;
	    toc->subFolderCount += numSubSubFolders;
	    parentDir = parent;
	    XtFree((char *)subfolderlist[i]);
	}
	XtFree((char *) subfolderlist);
    }
    return totalSubFolders;
}

void TocInit()
{
Toc toc;
struct direct **namelist;
register int i, j;
char str[PATH_MAX];
int numSubFolders;
extern alphasort();



if (!toc_cache_read())  /* could not find cache file */
  {
    parentDir = mailDir;
    numFolders = scandir(mailDir, &namelist, IsDir, alphasort);
    if (numFolders < 0) {
	(void) mkdir(mailDir, 0700);
	numFolders = scandir(mailDir, &namelist, IsDir, alphasort);
	if (numFolders < 0)
	    Punt("Can't create or read mail directory!");
    }
    MakeSureFolderExists(&namelist, &numFolders, initialFolderName);
    MakeSureFolderExists(&namelist, &numFolders, draftsFolderName);
    if (defUseWastebasket)
	MakeSureFolderExists(&namelist, &numFolders, wastebasketFolderName);
    totalNumFolders = numFolders;
    folderList = (Toc *) XtMalloc((unsigned) totalNumFolders * sizeof(Toc));
    j = 0;
    for (i=0 ; i<numFolders ; i++) {
	toc = folderList[j++] = TUMalloc();
	toc->foldername = MallocACopy(namelist[i]->d_name);
	toc->level = TocLevel(toc->foldername);
	(void) sprintf(str, "%s/%s", mailDir, toc->foldername);
	toc->path = MallocACopy(str);
	toc->visible = True;
	numSubFolders = TocFindSubfolders(j, toc->path);
	j += numSubFolders;
	toc->subFolderCount += numSubFolders;
	parentDir = mailDir;
	XtFree((char *)namelist[i]); 
    }
    numFolders = totalNumFolders;
    XtFree((char *)namelist);
  }
InitialFolder = TocGetNamed(initialFolderName);
DraftsFolder = TocGetNamed(draftsFolderName);
if (defUseWastebasket)
	WastebasketFolder = TocGetNamed(wastebasketFolderName);
DEBUG(("NewMailCheck: %s\n",(defNewMailCheck)?"TRUE":"FALSE"));
if (defNewMailCheck) LoadCheckFiles();
}


/* Create a new folder with the given name. */

Toc TocCreateFolder(foldername)
char *foldername;
{
    char str[PATH_MAX];
    char *ptr;
    Toc toc;
    Boolean newVisible;

    if (TocGetNamed(foldername)) return NULL;
    (void) sprintf(str, "%s/%s", mailDir, foldername);
    if (mkdir(str, 0700) < 0) return NULL;
    newVisible = True;
/*
 * If a subfolder, adjust parent's folder count
 */
    strcpy(str, foldername);
    ptr = rindex(str, '/');
    if (ptr != NULL) {
	*ptr = '\0';
	toc = TocGetNamed(str);
	if (toc) {
	    toc->subFolderCount++;
	    newVisible = False;
	}
    }
    toc = TocIncludeFolderName(foldername);
    toc->visible = newVisible;
    return toc;
}


/* Show a previously hidden folder with the given name. */

Toc TocShowFolder(foldername)
char *foldername;
{
struct stat statb;
Toc toc;

    if ((toc = TocGetNamed(foldername)) == NULL) return NULL;
    if (stat(toc->path, statb) < 0 || (!(statb.st_mode & S_IFDIR))) return NULL;
    return(TocIncludeFolderName(foldername));
}

Toc TocIncludeFolderName(foldername)
char	*foldername;
{
    Toc toc;
    register int i, j, k;
    TagRec tag;
    int pos, addPos;
    Widget widget;
    TagRec addTag[1];
    char *ptr;
    int entries;
    Arg arg[1];
    Toc displayToc;
    char str[PATH_MAX];

    toc = TUMalloc();
    toc->foldername = MallocACopy(foldername);
    toc->level = TocLevel(toc->foldername);
    (void) sprintf(str, "%s/%s", mailDir, foldername);
    toc->path = MallocACopy(str);

    ptr = strrchr(foldername, '/');
    if (ptr) *ptr = '\001';
    for (i=0 ; i<numFolders ; i++)
	if (strcmp(foldername, folderList[i]->foldername) < 0) break;
    if (ptr) *ptr = '/';
    folderList = (Toc *) XtRealloc((char *) folderList,
				   (unsigned) ++numFolders * sizeof(Toc));
    for (j=numFolders - 1 ; j > i ; j--)
	folderList[j] = folderList[j-1];
    folderList[i] = toc;
    toc->visible = True;
    if (defSVNStyle) {
	for (j = numFolders - 1; j > i; j--) {
	    tag.tagFields.tocNumber = j;
	    tag.tagFields.msgNumber = 0;
	    for (k = 0; k < numScrns; k++) {
		widget = scrnList[k]->tocwidget;
		if (widget) {
		    DXmSvnDisableDisplay(widget);
		    pos = DXmSvnGetEntryNumber(widget, tag.tagValue);
		    if (pos) {
			tag.tagFields.tocNumber++;
			DXmSvnSetEntryTag(widget, pos, tag.tagValue);
			tag.tagFields.tocNumber--;
		    }
		    DXmSvnEnableDisplay(widget);
		}
	    }
	}
	for (k = 0; k < numScrns; k++) {
	    widget = scrnList[k]->tocwidget;
	    if (widget) {
		DXmSvnDisableDisplay(widget);
		tag.tagFields.tocNumber = i - 1;
		tag.tagFields.msgNumber = 0;
		pos = DXmSvnGetEntryNumber(widget, tag.tagValue);
		if (pos) {
		    addTag[0].tagFields.tocNumber = i;
		    addTag[0].tagFields.msgNumber = 0;
		    DXmSvnAddEntries(widget, pos, 1, toc->level, addTag, True);
		}
		DXmSvnEnableDisplay(widget);
	    }
	}
    } else {
	for (i = 0; i < numScrns; i++) {
	    widget = scrnList[i]->tocwidget;
	    displayToc = scrnList[i]->toc;
	    if (widget && displayToc) {
		DXmSvnDisableDisplay(widget);
		XtSetArg(arg[0], DXmSvnNnumberOfEntries, &entries);
		XtGetValues(widget, arg, 1);
		for (j = 0; j < entries; j++) {
		    tag.tagValue = DXmSvnGetEntryTag(widget, j);
		    if (tag.tagValue) {
			tag.tagFields.tocNumber = TUGetTocNumber(displayToc);
			DXmSvnSetEntryTag(widget, j, tag.tagValue);
		    }
		}
		DXmSvnEnableDisplay(widget);
	    }
	}
    }
    return toc;
}


/* Check to see if what folders have new mail, and highlight their
   folderbuttons appropriately. */

void TocCheckForNewMail()
{
    Toc toc;
    Scrn scrn;
    register int i, j, hasmail;
    TagRec tag;
    int pos;
    static Arg arglist[] = {
	{XtNiconPixmap, NULL},
    };
    if (!defNewMailCheck) return;
    for (i=0 ; i<numFolders ; i++) {
	toc = folderList[i];
	if (toc->incfile) {
	    hasmail =  (GetFileLength(toc->incfile) > 0);
	    if (hasmail != toc->mailpending) {
		toc->mailpending = hasmail;
		for (j=0 ; j<numScrns ; j++) {
		    scrn = scrnList[j];
		    if (scrn->tocwidget != NULL) {
			arglist[0].value = (XtArgVal) hasmail ? NewMailPixmap : NoMailPixmap;
			XtSetValues(scrn->parent, arglist, XtNumber(arglist));
		    } 
		    if (!defSVNStyle) {
			RadioSetHighlight(scrn->folderradio, toc->foldername, hasmail);
		    } else {
			if (scrn->tocwidget) {
			    tag.tagFields.tocNumber= TUGetTocNumber(toc);
			    tag.tagFields.msgNumber = 0;
			    TocDisableRedisplay(scrn->tocwidget);
			    pos = DXmSvnGetEntryNumber(scrn->tocwidget,
					 tag.tagValue);
			    if (pos != 0) {
				DXmSvnInvalidateEntry(scrn->tocwidget, pos);
			    }
			    TocEnableRedisplay(scrn->tocwidget);
		        }
		    }
		}
	    }
	}
    }
}


/* Destroy the given folder. */

void TocDeleteFolder(toc)
Toc toc;
{
    Toc toc2;
    register int i, j, k, w;
    char str[PATH_MAX];
    char *ptr;
    TagRec tag;
    int pos, addPos;
    Widget widget;
    TagRec delTag[1];

    TUGetFullFolderInfo(toc);
    if (TocConfirmCataclysm(toc)) return;
    TocSetScrn(toc, (Scrn) NULL);
    w = -1;
    for (i=0 ; i<numFolders ; i++) {
	toc2 = folderList[i];
	if (toc2 == toc)
	    w = i;
	else if (toc2->validity == valid)
	    for (j=0 ; j<toc2->nummsgs ; j++)
		if (toc2->msgs[j]->desttoc == toc)
		    MsgSetFate(toc2->msgs[j], Fignore, (Toc) NULL);
    }
    if (w < 0) Punt("Couldn't find it in TocDeleteFolder!");
/*
 * Adjust parent's subfolder count
 */
    strcpy(str, toc->foldername);
    ptr = rindex(str, '/');
    if (ptr != NULL) {
	*ptr = '\0';
	toc2 = TocGetNamed(str);
	if (toc2) {
	    toc2->subFolderCount--;
	}
    }
    NukeDirectory(toc->path);
    if (toc->validity == valid) {
	for (i=0 ; i<toc->nummsgs ; i++) {
	    MsgSetScrnForce(toc->msgs[i], (Scrn) NULL);
	    MsgFree(toc->msgs[i]);
	}
	XtFree((char *) toc->msgs);
        if ( toc->foldername ) {
            XtFree((char *)toc->foldername);
            toc->foldername = NULL;
        }
    }
    XtFree((char *)toc);
    toc = NULL;

    if (defSVNStyle) {
	for (k = 0; k < numScrns; k++) {
	    widget = scrnList[k]->tocwidget;
	    if (widget) {
		DXmSvnDisableDisplay(widget);
		tag.tagFields.tocNumber = w;
		tag.tagFields.msgNumber = 0;
		pos = DXmSvnGetEntryNumber(widget, tag.tagValue);
		if (pos > 0) {
		    DXmSvnDeleteEntries(widget, pos - 1, 1);
		}
		DXmSvnEnableDisplay(widget);
	    }
	}
	for (j = w; j < numFolders; j ++) {
	    tag.tagFields.tocNumber = j;
	    tag.tagFields.msgNumber = 0;
	    for (k = 0; k < numScrns; k++) {
		widget = scrnList[k]->tocwidget;
		if (widget) {
		    DXmSvnDisableDisplay(widget);
		    pos = DXmSvnGetEntryNumber(widget, tag.tagValue);
		    if (pos) {
			tag.tagFields.tocNumber--;
			DXmSvnSetEntryTag(widget, pos, tag.tagValue);
			tag.tagFields.tocNumber++;
		    }
		    DXmSvnEnableDisplay(widget);
		}
	    }
	}
    }
    numFolders--;
    for (i=w ; i<numFolders ; i++) folderList[i] = folderList[i+1];
}


/* Hide the given folder. */

void TocHideFolder(toc)
Toc toc;
{
    Toc toc2;
    register int i, j, k, w;
    TagRec tag;
    int pos, addPos;
    Widget widget;
    TagRec delTag[1];

    TUGetFullFolderInfo(toc);
    if (TocConfirmCataclysm(toc)) return;
    TocSetScrn(toc, (Scrn) NULL);
    w = -1;
    for (i=0 ; i<numFolders ; i++) {
	toc2 = folderList[i];
	if (toc2 == toc)
	    w = i;
	else if (toc2->validity == valid)
	    for (j=0 ; j<toc2->nummsgs ; j++)
		if (toc2->msgs[j]->desttoc == toc)
		    MsgSetFate(toc2->msgs[j], Fignore, (Toc) NULL);
    }
    if (w < 0) Punt("Couldn't find it in TocHideFolder!");

/*  I don't know whether to keep the following. I've just copied this
        code from TocDeleteFolder.  I'm going to assume that the code 
        here cleans up internal data structures and that thats the right
        thing to do.
 */
    if (toc->validity == valid) {
	for (i=0 ; i<toc->nummsgs ; i++) {
	    MsgSetScrnForce(toc->msgs[i], (Scrn) NULL);
	    MsgFree(toc->msgs[i]);
	}
	XtFree((char *) toc->msgs);
    }
    XtFree((char *)toc);
    if (defSVNStyle) {
	for (k = 0; k < numScrns; k++) {
	    widget = scrnList[k]->tocwidget;
	    if (widget) {
		DXmSvnDisableDisplay(widget);
		tag.tagFields.tocNumber = w;
		tag.tagFields.msgNumber = 0;
		pos = DXmSvnGetEntryNumber(widget, tag.tagValue);
		if (pos > 0) {
		    DXmSvnDeleteEntries(widget, pos - 1, 1);
		}
		DXmSvnEnableDisplay(widget);
	    }
	}
	for (j = w; j < numFolders; j ++) {
	    tag.tagFields.tocNumber = j;
	    tag.tagFields.msgNumber = 0;
	    for (k = 0; k < numScrns; k++) {
		widget = scrnList[k]->tocwidget;
		if (widget) {
		    DXmSvnDisableDisplay(widget);
		    pos = DXmSvnGetEntryNumber(widget, tag.tagValue);
		    if (pos) {
			tag.tagFields.tocNumber--;
			DXmSvnSetEntryTag(widget, pos, tag.tagValue);
			tag.tagFields.tocNumber++;
		    }
		    DXmSvnEnableDisplay(widget);
		}
	    }
	}
    }
    numFolders--;
    for (i=w ; i<numFolders ; i++) folderList[i] = folderList[i+1];
}


/*
 * Display the given toc in the given scrn.  If scrn is NULL, then remove the
 * toc from all scrns displaying it.
 */

void TocSetScrn(toc, scrn)
Toc toc;
Scrn scrn;
{
    register int i;
    if (toc == NULL && scrn == NULL) return;
    if (scrn == NULL) {
	while (toc->num_scrns != 0)
	    TocSetScrn((Toc) NULL, toc->scrn[0]);
	return;
    }
    if (scrn->toc == toc) return;
    if (scrn->tocwidget == NULL) {
	scrn->toc = toc;
	return;
    }
    if (scrn->toc != NULL) {
	for (i=0 ; i<scrn->toc->num_scrns ; i++)
	    if (scrn->toc->scrn[i] == scrn) break;
	if (i >= scrn->toc->num_scrns)
	    Punt("Couldn't find scrn in TocSetScrn!");
	if (AutoCommit && scrn->toc->num_scrns == 1)
	    TocCommitChanges(scrn->toc);
	scrn->toc->scrn[i] = scrn->toc->scrn[--scrn->toc->num_scrns];
    }
    if (toc != NULL) {
	scrn->toc = toc;
	toc->num_scrns++;
	toc->scrn = (Scrn *)
	    XtRealloc((char *) toc->scrn,
		      (unsigned) toc->num_scrns * sizeof(Scrn));
	toc->scrn[toc->num_scrns - 1] = scrn;
	TUEnsureScanIsValidAndOpen(toc);
	if (unseenSeqName && TocCanIncorporate(toc))
	    (void) TocCreateNullSequence(toc, unseenSeqName);
    }
    scrn->lasttoc = scrn->toc;
    scrn->toc = toc;
    TURedisplayToc(scrn);
    TUResetTocLabel(scrn);
    EnableProperButtons(scrn);
    ScrnNeedsTitleBarChanged(scrn);
    ScrnNeedsIconNameChanged(scrn);
}



/* Remove the given message from the toc.  Doesn't actually touch the file.
   Also note that it does not free the storage for the msg. */

void TocRemoveMsg(toc, msg)
Toc toc;
Msg msg;
{
    Msg newcurmsg;
    MsgList mlist;
    register int i, j;
    int orig_pos, entry;
    TagRec tag;

    if (toc->validity == unknown)
	TUGetFullFolderInfo(toc);
    if (toc->validity != valid)
	return;
    newcurmsg = TocMsgAfter(toc, msg);
    if (newcurmsg) newcurmsg->changed = TRUE;
    if (msg == toc->curmsg) {
	TocSetCurMsg(toc, NULL);
	newcurmsg = TocMsgAfter(toc, msg);
	if (newcurmsg == NULL) newcurmsg = TocMsgBefore(toc, msg);
    } else {
	newcurmsg = NULL;
    }
    if (msg->visible) {
	TocUnsetSelection(toc);
	toc->lastPos -= msg->length;
    }

    if (msg->visible && toc->num_scrns != 0) {
	for (i = 0; i < toc->num_scrns; i++) {
	    TocDisableRedisplay(toc->scrn[i]->tocwidget);
	    tag.tagFields.tocNumber = TUGetTocNumber(toc);
	    tag.tagFields.msgNumber = TUGetMsgIndex(toc, msg);
	    entry = DXmSvnGetEntryNumber(toc->scrn[i]->tocwidget, tag.tagValue);
	    if (entry != 0)
	        DXmSvnDeleteEntries(toc->scrn[i]->tocwidget, entry - 1, 1);
	    TocEnableRedisplay(toc->scrn[i]->tocwidget);
	}
    }

    orig_pos = TUGetMsgPosition(toc, msg);
    tag.tagFields.tocNumber = TUGetTocNumber(toc);
    for(i = orig_pos, toc->nummsgs--; i<toc->nummsgs ; i++) {
	toc->msgs[i] = toc->msgs[i+1];
	tag.tagFields.msgNumber = i + 2;
	for (j = 0; j < toc->num_scrns; j++) {
	    entry = DXmSvnGetEntryNumber(toc->scrn[j]->tocwidget, tag.tagValue);
	    if (entry != 0) {
		tag.tagFields.msgNumber--;
		DXmSvnSetEntryTag(toc->scrn[j]->tocwidget, entry, tag.tagValue);
		tag.tagFields.msgNumber++;
	    }
	}
	if (msg->visible) toc->msgs[i]->position--;
    }
    for (i=0 ; i<toc->numsequences ; i++) {
	mlist = toc->seqlist[i]->mlist;
	if (mlist) DeleteMsgFromMsgList(mlist, msg);
    }

    if (newcurmsg)
	TocSetCurMsg(toc, newcurmsg);
    TUSaveTocFile(toc);
}
 


void TocRecheckValidity(toc)
  Toc toc;
{
    register int i;
    if (toc) {
	if (toc->validity != valid)
	    TUEnsureScanIsValidAndOpen(toc);
	if (TUScanFileOutOfDate(toc)) {
	    TUScanFileForToc(toc);
	    TULoadTocFile(toc);
	    for (i=0 ; i<toc->num_scrns ; i++)
		TURedisplayToc(toc->scrn[i]);
	}
    }
}

static Boolean WorkMakeVisible(msg)
Msg msg;
{
    Toc toc = msg->toc;
    register int i, j;
    int pos;
    int numDisplayed;
    int *entries;
    Boolean found;
    Widget widget;
    TagRec tag;

    if (toc == NULL || msg == NULL || msg->toc != toc || !msg->visible)
	return True;

    tag.tagFields.tocNumber = TUGetTocNumber(toc);
    tag.tagFields.msgNumber = TUGetMsgIndex(toc, msg);
    
    for (i=0 ; i<toc->num_scrns ; i++) {
	widget = toc->scrn[i]->tocwidget;
	TocDisableRedisplay(widget);
	pos = DXmSvnGetEntryNumber(widget, tag.tagValue);
	if (pos != 0) {
	    numDisplayed = DXmSvnGetNumDisplayed(widget);
	    entries = (int *) XtMalloc(numDisplayed * sizeof (int));
	    DXmSvnGetDisplayed(widget, entries, NULL, NULL, numDisplayed);
	    found = False;
	    for (;numDisplayed >=0; numDisplayed--)
	        if (entries[numDisplayed - 1] != 0) break;
	    for (j = 0; j < numDisplayed - 1; j++) {
	        if (entries[j] == pos) {
		    found = True;
		    break;
	        }
	    }
	    if (!found && numDisplayed != 0) {
	        if (pos < entries[0]) {
		    DXmSvnPositionDisplay(widget, pos, DXmSvnKpositionTop);
	        } 
	        if (pos >= entries[numDisplayed - 1]) {
		    DXmSvnPositionDisplay(widget, pos, DXmSvnKpositionBottom);
	        }
	    }
	    XtFree((char *)entries);
	}
	TocEnableRedisplay(widget);
    }
    return True;
}

/*
 * Make sure the given message is visible in the toc.
 */

void TocMakeVisible(toc, msg)
Toc toc;
Msg msg;


{
    int pos;
    TagRec tag;
    int i;

    if (toc == NULL || msg == NULL || msg->toc != toc || !msg->visible)
      return;

    if (toc->num_scrns == 0) return;

    tag.tagFields.tocNumber = TUGetTocNumber(toc);
    tag.tagFields.msgNumber = TUGetMsgIndex(toc, msg);
    for (i = 0; i < toc->num_scrns; i++) { /* AJ 01 */
      TocDisableRedisplay(toc->scrn[i]->tocwidget);
      pos = DXmSvnGetEntryNumber(toc->scrn[i]->tocwidget, tag.tagValue);
      TocEnableRedisplay(toc->scrn[i]->tocwidget);
      if (pos == 0) continue;
      XtAddWorkProc((XtWorkProc)WorkMakeVisible, msg);
    }
  }




/* Set the current message. */

void TocSetCurMsg(toc, msg)
  Toc toc;
  Msg msg;
{
    Msg msg2;

    if (toc == NULL) return;
    if (toc->validity != valid) return;
    if (msg != toc->curmsg) {
	msg2 = toc->curmsg;
	toc->curmsg = msg;
	if (msg2)
	    MsgSetFate(msg2, msg2->fate, msg2->desttoc);
    }
    if (msg) {
	MsgSetFate(msg, msg->fate, msg->desttoc);
	TocMakeVisible(toc, msg);
    }
    TocRedoButtonPixmaps(toc);
}


/* Return the current message. */

Msg TocGetCurMsg(toc)
Toc toc;
{
    return (toc != NULL) ? toc->curmsg : NULL;
}


/*
 * Return the last messaeg in the given toc. 
 */

Msg TocGetFirstMsg(toc)
Toc toc;
{
    return (toc != NULL && toc->nummsgs > 0) ? toc->msgs[0] : NULL;
}

/*
 * Return the last messaeg in the given toc. 
 */

Msg TocGetLastMsg(toc)
Toc toc;
{
    return (toc != NULL && toc->nummsgs > 0) ? toc->msgs[toc->nummsgs - 1] : NULL;
}

/* Return the message after the given one.  (If none, return NULL.) */

Msg TocMsgAfter(toc, msg)
  Toc toc;
  Msg msg;
{
    register int i;
    i = TUGetMsgPosition(toc, msg);
    do {
	i++;
	if (i >= toc->nummsgs)
	    return NULL;
    } while (!(toc->msgs[i]->visible));
    return toc->msgs[i];
}



/* Return the message before the given one.  (If none, return NULL.) */

Msg TocMsgBefore(toc, msg)
  Toc toc;
  Msg msg;
{
    register int i;
    i = TUGetMsgPosition(toc, msg);
    do {
	i--;
	if (i < 0)
	    return NULL;
    } while (!(toc->msgs[i]->visible));
    return toc->msgs[i];
}



/* The caller KNOWS the toc's information is out of date; rescan it. */

void TocForceRescan(toc)
  Toc toc;
{
    register int i;
    char name[500];
    if (toc->num_scrns) {
	if (toc->viewedseq)
	    (void) strcpy(name, toc->viewedseq->name);
	else
	    name[0] = 0;
	toc->viewedseq = toc->seqlist[0];
	for (i=0 ; i<toc->num_scrns ; i++)
	    TUResetTocLabel(toc->scrn[i]);
	TUScanFileForToc(toc);
	TULoadTocFile(toc);
	TocChangeViewedSeq(toc, TocGetSeqNamed(toc, name));
	for (i=0 ; i<toc->num_scrns ; i++)
	    TURedisplayToc(toc->scrn[i]);
    } else {
	TUGetFullFolderInfo(toc);
	(void) unlink(toc->scanfile);
	DEBUG(("Force Rescan setting Invalid\n"));
	toc->validity = invalid;
    }
}



/*
 * For each scrn currently showing this toc, make sure the shown sequence
 * buttons correspond exactly to the sequences for this toc.  If not, then
 * rebuild the radiobox.
 */

void TocCheckSeqButtons(toc)
Toc toc;
{
    Scrn scrn;
    register int w, i, numinbox;
    int rebuild;
    char **names;

    DEBUG(("Checking sequence buttons...\n"));
    if (toc == NULL) return;
    for (w=0 ; w<numScrns ; w++) {
	scrn = scrnList[w];
	if (scrn->toc == toc || (scrn->msg && scrn->msg->toc == toc)) {
	    rebuild = FALSE;
	    numinbox = RadioGetNumChildren(scrn->seqradio);
	    if (numinbox != toc->numsequences)
		rebuild = TRUE;
	    else {
		for (i=0 ; i<toc->numsequences && !rebuild; i++)
		    rebuild = strcmp(toc->seqlist[i]->name,
				     RadioGetName(scrn->seqradio, i));
	    }
	    if (rebuild) {
		DEBUG(("Rebuilding sequence radio list...\n"));

/* Doesn't delete 0th ("all") sequence, if it exists... */
		for (i = numinbox - 1; i >= 1 ; i--)
		    RadioDeleteButton(scrn->seqradio,
				      RadioGetName(scrn->seqradio, i));
		names = (char **)
		    XtMalloc((Cardinal) (toc->numsequences * sizeof(char *)));
		for (i=0 ; i<toc->numsequences ; i++)
		    names[i] = toc->seqlist[i]->name;

/* If the "all" sequence existed before, don't bother adding it again... */
		i = (numinbox) ? 1 : 0;

/* otherwise add all of the other sequence name buttons. */
		RadioAddButtons(scrn->seqradio,names+i,toc->numsequences -i,i);

		XtFree((char *) names);
	    }
	    if (RadioGetCurrent(scrn->seqradio) == NULL)
		RadioSetCurrent(scrn->seqradio, toc->viewedseq->name);
	}
    }
}


/* The caller has just changed a sequence list.  Reread them from mh. */

void TocReloadSeqLists(toc)
Toc toc;
{
    register int i;
    TocSetCacheValid(toc);
    TULoadSeqLists(toc, (Msg *) NULL);
    TURefigureWhatsVisible(toc);
    for (i=0 ; i<toc->num_scrns ; i++) {
	TUResetTocLabel(toc->scrn[i]);
	EnableProperButtons(toc->scrn[i]);
    }
}

/*
 * The caller has just changed the in-memory sequence list.  Write it out to
 * mh.  NOTE: for safety's sake, this should only be called soon after reading
 * said list in!
 */

void TocWriteSeqLists(toc)
Toc toc;
{
    int i, j, msgid, first, last;
    FILE *fid;
    char name[500];
    Sequence seq;
    MsgList mlist;
    if (toc == NULL) return;
    (void) sprintf(name, "%s/.mh_sequences", toc->path);
    fid = FOpenAndCheck(name, "w");
    if (toc->curmsg != NULL)
	(void) fprintf(fid, "cur: %d\n", toc->curmsg->msgid);
    for (i=0 ; i<toc->numsequences ; i++) {
	seq = toc->seqlist[i];
	mlist = seq->mlist;
	if (mlist && mlist->nummsgs > 0 && strcmp(seq->name, "all") != 0) {
	    fprintf(fid, "%s:", seq->name);
	    first = last = mlist->msglist[0]->msgid;
	    for (j=1 ; j<mlist->nummsgs ; j++) {
		msgid = mlist->msglist[j]->msgid;
		if (msgid != last + 1) {
		    if (last == first) fprintf(fid, " %d", last);
		    else fprintf(fid, " %d-%d", first, last);
		    first = msgid;
		}
		last = msgid;
	    }
	    if (last == first) fprintf(fid, " %d", last);
	    else fprintf(fid, " %d-%d", first, last);
	    fprintf(fid, "\n");
	}
    }
    (void) myfclose(fid);
    TocSetCacheValid(toc);
    TURefigureWhatsVisible(toc);
    for (i=0 ; i<toc->num_scrns ; i++) {
	TUResetTocLabel(toc->scrn[i]);
	EnableProperButtons(toc->scrn[i]);
    }
}

/* Return TRUE if the toc has an interesting sequence. */

int TocHasSequences(toc)
Toc toc;
{
    return toc && toc->numsequences > 1;
}


/* Change which sequence is being viewed. */

void TocChangeViewedSeq(toc, seq)
  Toc toc;
  Sequence seq;
{
    register int i;
    if (seq == NULL) seq = toc->viewedseq;
    toc->viewedseq = seq;
    TURefigureWhatsVisible(toc);
    for (i=0 ; i<toc->num_scrns ; i++) {
	if (toc->scrn[i]->seqradio)
	    RadioSetCurrent(toc->scrn[i]->seqradio, seq->name);
	TUResetTocLabel(toc->scrn[i]);
    }
}


/* Return the sequence with the given name in the given toc. */

Sequence TocGetSeqNamed(toc, name)
Toc toc;
char *name;
{
    register int i;
    for (i=0 ; i<toc->numsequences ; i++)
	if (strcmp(toc->seqlist[i]->name, name) == 0)
	    return toc->seqlist[i];
    return (Sequence) NULL;
}


/* Return the sequence currently being viewed in the toc. */

Sequence TocViewedSequence(toc)
Toc toc;
{
    return toc->viewedseq;
}


/* 
 * Create a null sequence of the given name, provided such a one doesn't
 * already exist.  Return TRUE on success. 
 */

Boolean TocCreateNullSequence(toc, name)
Toc toc;
char *name;
{
    register int i, j, c;
    Sequence seq;
    if (toc == NULL) return FALSE;
    for (i=1 ; i<toc->numsequences ; i++) {
	c = strcmp(toc->seqlist[i]->name, name);
	if (c == 0) return FALSE;
	if (c > 0) break;
    }
    toc->numsequences++;
    toc->seqlist = (Sequence *)
	XtRealloc((char *) toc->seqlist,
		  (unsigned) toc->numsequences * sizeof(Sequence));
    for (j=toc->numsequences - 1; j > i ; j--)
	toc->seqlist[j] = toc->seqlist[j-1];
    seq = toc->seqlist[i] = XtNew(SequenceRec);
    seq->name = MallocACopy(name);
    seq->mlist = MakeNullMsgList();
    TocCheckSeqButtons(toc);
    return TRUE;
}


/*
 * Delete the given sequence from the toc, provided it doesn't contain
 * anything.  Return TRUE on success.
 */

Boolean TocDeleteNullSequence(toc, name)
Toc toc;
char *name;
{
    register int i, j;
    if (toc == NULL) return FALSE;
    for (i=1 ; i<toc->numsequences ; i++) {
	if (strcmp(toc->seqlist[i]->name, name) == 0) {
	    if (toc->seqlist[i]->mlist->nummsgs != 0) return FALSE;
	    if (toc->viewedseq == toc->seqlist[i])
		TocChangeViewedSeq(toc, toc->seqlist[0]);
	    toc->numsequences--;
	    for (j=i ; j<toc->numsequences ; j++)
		toc->seqlist[j] = toc->seqlist[j+1];
	    TocCheckSeqButtons(toc);
	    return TRUE;
	}
    }
    return FALSE;
}


/* Return the list of messages currently selected. */

MsgList TocCurMsgList(toc, scrn)
Toc toc;
Scrn scrn;
{
    MsgList result;
    register int i;
    Widget widget;
    int *entryNumbers;
    TagRec tag;
    int tocNumber = TUGetTocNumber(toc);
    Msg msg;
    int numSelections;

    if (toc == NULL || toc->num_scrns == NULL) return NULL;
    result = MakeNullMsgList();
    widget = scrn->tocwidget;    
    if (widget == NULL) return NULL;

    TocDisableRedisplay(widget);
    numSelections = DXmSvnGetNumSelections(widget);
    entryNumbers = (int *) XtMalloc(numSelections * sizeof(int *));
    DXmSvnGetSelections(widget, entryNumbers, NULL, NULL, numSelections);
    toc->numselections = 0;
    for (i=0 ; i < numSelections ; i++) {
	tag.tagValue = DXmSvnGetEntryTag(widget, entryNumbers[i]);
	if (tag.tagFields.tocNumber == tocNumber &&  
	    tag.tagFields.msgNumber != 0 &&
	    tag.tagFields.msgNumber <= toc->nummsgs) {
	    msg = toc->msgs[tag.tagFields.msgNumber - 1];
	    AppendMsgList(result, msg);
	    toc->numselections++;
	}
    }
    XtFree((char *)entryNumbers);
    TocEnableRedisplay(widget);
    return result;
}



/* Return TRUE if the given toc has the selection. */

Boolean TocHasSelection(toc, scrn)
Toc toc;
Scrn scrn;
{
    int numSelections;
    register int i;
    Widget widget;
    int *entryNumbers;
    TagRec tag;
    int tocNumber = TUGetTocNumber(toc);

    if (toc == NULL || scrn->tocwidget == NULL) return False;
    widget = scrn->tocwidget;    

    TocDisableRedisplay(widget);
    numSelections = DXmSvnGetNumSelections(widget);
    entryNumbers = (int *) XtMalloc(numSelections * sizeof(int *));
    DXmSvnGetSelections(widget, entryNumbers, NULL, NULL, numSelections);
    toc->numselections = 0;
    for (i=0 ; i < numSelections ; i++) {
	tag.tagValue = DXmSvnGetEntryTag(widget, entryNumbers[i]);
	if (tag.tagFields.tocNumber == tocNumber &&
	    tag.tagFields.msgNumber != 0) {
	    toc->numselections++;
	}
    }
    XtFree((char *)entryNumbers);
    TocEnableRedisplay(widget);
    return (toc->numselections != 0);
}

/* Unset the current selection. */

void TocUnsetSelection(toc)
Toc toc;
{
    register int i;
    for (i = 0; i < toc->num_scrns; i++) {
	TocDisableRedisplay(toc->scrn[i]->tocwidget);
	DXmSvnClearSelections(toc->scrn[i]->tocwidget);
	TocEnableRedisplay(toc->scrn[i]->tocwidget);
    }
}



/* Create a brand new, blank message. */

Msg TocMakeNewMsg(toc)
Toc toc;
{
    Msg msg;
    static attempts = 0; /* which try this is  - fixes infinite loop */

    TUEnsureScanIsValidAndOpen(toc);
    msg = TUAppendToc(toc, "####  empty\n"); /* %%% Shouldn't be hardcoded. */
    if (FileExists(MsgFileName(msg))) {
	DEBUG(("**** FOLDER %s WAS INVALID!!!\n",toc->foldername));
	switch (attempts) {
	    case 0:
		attempts = 1; /* only rescan and hope that fixes it */
		break;
	    case 1:
		attempts = 2; /* Second retry; unlink the hopefully bogus
				 message file */
		(void) unlink(MsgFileName(msg));
		break;
	    case 2:
		Error(toplevel, EFileCantOpen, MsgFileName(msg));
		return NULL;
	    }
#ifdef notdef
	MsgFree(msg);
#endif
	TocForceRescan(toc);
	return TocMakeNewMsg(toc); /* Try again.  Using recursion here is ugly,
				      but what the hack ... */
    }
    attempts = 0;		/* reset on success */

    CopyFileAndCheck("/dev/null", MsgFileName(msg));
    return msg;
}


/* Set things to not update cache or display until further notice. */

void TocStopUpdate(toc)
Toc toc;
{
    register int i;
    for (i=0 ; i<toc->num_scrns ; i++)
	TocDisableRedisplay(toc->scrn[i]->tocwidget);
    toc->stopupdate++;
}


/* Start updating again, and do whatever updating has been queued. */

void TocStartUpdate(toc)
Toc toc;
{
    register int i;
    if (toc->stopupdate && --(toc->stopupdate) == 0) {
	for (i=0 ; i<toc->num_scrns ; i++) {
	    if (toc->needsrepaint) 
		TURedisplayToc(toc->scrn[i]);
		TUResetTocLabel(toc->scrn[i]);
	}
	if (toc->needscachesave)
	    TUSaveTocFile(toc);
    }
    for (i=0 ; i<toc->num_scrns ; i++)
	TocEnableRedisplay(toc->scrn[i]->tocwidget);
}



/* Something has happened that could later convince us that our cache is out
   of date.  Make this not happen; our cache really *is* up-to-date. */

void TocSetCacheValid(toc)
Toc toc;
{
    TUSaveTocFile(toc);
}


/* Return the foldername of the given toc. */

char *TocGetFolderName(toc)
Toc toc;
{
    if (toc == NULL) return NULL;
    return toc->foldername;
}



/* Given a foldername, return the corresponding toc. */

Toc TocGetNamed(name)
char *name;
{
    register int i;
    for (i=0; i<numFolders ; i++)
	if (strcmp(folderList[i]->foldername, name) == 0) return folderList[i];
    return NULL;
}


/* Return whether given folder is a Drawer */

int
TocSubFolderCount(toc)
Toc toc;
{
    if (!toc) return 0;
    return toc->subFolderCount;
}

/* Return visible state of a button */
Boolean
TocGetVisible(toc)
Toc toc;
{
    return toc->visible;
}

Toc
TocMakeFolderVisible(foldername)
char *foldername;
{
    Toc toc;
    TagRec tag[1];
    TagRec findTag;
    int i, parentPos;
    Widget widget;
    int parentToc;

    if ((toc = TocGetNamed(foldername)) == NULL) return NULL;
    toc->visible = True;
    if (defSVNStyle) {
	for (i = 0; i < numScrns; i++) {
	    widget = scrnList[i]->tocwidget;
	    if (widget) {
		DXmSvnDisableDisplay(widget);
		parentToc = tag[0].tagFields.tocNumber = 
			TUGetTocNumber(toc) - 1;
		tag[0].tagFields.msgNumber = 0;
		parentPos = DXmSvnGetEntryNumber(widget, tag[0].tagValue);
		while (parentPos != 0) {
		    findTag.tagValue = DXmSvnGetEntryTag(widget, parentPos);
		    if (findTag.tagFields.tocNumber == parentToc) {
			parentPos++;
		    } else {
			parentPos--;
			break;
		    }
		}
		if (parentPos) {
		    tag[0].tagFields.tocNumber++;
		    DXmSvnAddEntries(widget, parentPos, 1, toc->level,
			tag, True);
		}
		DXmSvnEnableDisplay(widget);
	    }
	}
    }
    return toc;
}
Toc
TocMakeFolderInvisible(foldername)
char *foldername;
{
    Toc toc;
    TagRec tag[1];
    int i, pos;
    Widget widget;

    if ((toc = TocGetNamed(foldername)) == NULL) return NULL;
    toc->visible = False;
    if (defSVNStyle) {
	for (i = 0; i < numScrns; i++) {
	    widget = scrnList[i]->tocwidget;
	    if (widget) {
		DXmSvnDisableDisplay(widget);
		tag[0].tagFields.tocNumber = TUGetTocNumber(toc);
		tag[0].tagFields.msgNumber = 0;
		pos = DXmSvnGetEntryNumber(widget, tag[0].tagValue);
		if (pos) {
		    DXmSvnDeleteEntries(widget, pos - 1, 1);
		}
		DXmSvnEnableDisplay(widget);
	    }
	}
    }
    return toc;
}
/*
 * Throw out all changes to this toc, and close all views of msgs in it.
 * Requires confirmation by the user.  Note that if we are auto-committing,
 * then this will just commit, and always return as if the user confirmed it.
 */

int TocConfirmCataclysm(toc)
Toc toc;
{
    register int i;
    int found = FALSE;
    char str[500];
    if (AutoCommit) {
	TocCommitChanges(toc);
	return 0;
    }
    for (i=0 ; i<toc->nummsgs && !found ; i++)
	if (toc->msgs[i]->fate != Fignore) found = TRUE;
    if (found) {
	(void)sprintf(str,"Are you sure you want to remove all changes to %s?",
		      toc->foldername);
	if (!Confirm(toc->scrn[0], str, "OnWindow OnFolders OnDeleting_folders"))
	    return DELETEABORTED;
    }
    for (i=0 ; i<toc->nummsgs ; i++)
	MsgSetFate(toc->msgs[i], Fignore, (Toc)NULL);
    for (i=0 ; i<toc->nummsgs ; i++)
	if (MsgSetScrn(toc->msgs[i], (Scrn) NULL)) return DELETEABORTED;
    return 0;
}



/*
 * Commit all the changes in this toc; all messages will meet their 'fate'.
 */

void TocCommitChanges(toc)
Toc toc;
{
    Msg msg, newmsg;
    register int i, cur;
    char str[100], **argv;
    FateType curfate, fate; 
    Toc desttoc;
    Toc curdesttoc;
    Boolean didsomething = FALSE;
    Boolean fixedprofile = FALSE;
    Boolean found = FALSE;
    char *tempprofilename;
    FILE *fid;

    if (toc == NULL) return;
    for (i=0 ; i<toc->nummsgs ; i++) {
	msg = toc->msgs[i];
	fate = MsgGetFate(msg, (Toc *)NULL);
	if (fate != Fignore) {
	    found = TRUE;
	    if (MsgSetScrn(msg, (Scrn) NULL))
		return;
	}
    }
    if (!found) return;		/* Nothing to commit. */
    for (i=0 ; i<numFolders ; i++)
	TocStopUpdate(folderList[i]);
    toc->haschanged = TRUE;
    do {
	curfate = Fignore;
	i = 0;
	while (i < toc->nummsgs) {
	    msg = toc->msgs[i];
	    fate = MsgGetFate(msg, &desttoc);
	    if (fate == Fdelete && defUseWastebasket &&
		   toc != WastebasketFolder) {
		fate = Fmove;
		desttoc = WastebasketFolder;
	    }
	    if (curfate == Fignore && fate != Fignore) {
		curfate = fate;
		argv = MakeArgv(2);
		cur = 0;
		if (fate == Fdelete) {
		    if (!fixedprofile && !hasRmmProc) {
			tempprofilename =
				MakeNewTempFileNameInSameDir(profileFileName);
			tempprofilename = MallocACopy(tempprofilename);
			RenameAndCheck(profileFileName, tempprofilename);
			CopyFileAndCheck(tempprofilename, profileFileName);
			fid = FOpenAndCheck(profileFileName, "a");
			(void) fprintf(fid, "rmmproc: /bin/rm\n");
			(void) fclose(fid);
			fixedprofile = TRUE;
		    }
		    argv[cur++] = MallocACopy(rmmCmd);
		    (void) sprintf(str, "+%s", toc->foldername);
		    argv[cur++] = MallocACopy(str);
		} else {
		    argv[cur++] = MallocACopy(refileCmd);
		}
		curdesttoc = desttoc;
		if (curdesttoc && !curdesttoc->haschanged &&
		      curdesttoc->validity == valid &&
		      TUScanFileOutOfDate(curdesttoc)) {
		    DEBUG(("Out of date, setting invalid\n"));
		    curdesttoc->validity = invalid;
		    if (curdesttoc->num_scrns > 0)
			TocRecheckValidity(curdesttoc);
		}
	    }
	    if (curfate != Fignore &&
		  curfate == fate && desttoc == curdesttoc) {
		argv = ResizeArgv(argv, cur + 1);
		(void) sprintf(str, "%d", MsgGetId(msg));
		argv[cur++] = MallocACopy(str);
		MsgSetFate(msg, Fignore, (Toc)NULL);
		if (curdesttoc) {
		    if (msg->handle) {
			if (curdesttoc->validity != valid)
			    TUEnsureScanIsValidAndOpen(curdesttoc);
			newmsg = TUAppendToc(curdesttoc, MsgGetScanLine(msg));
			if (newmsg) {
			    newmsg->handle = msg->handle;
			    newmsg->handle->msg = newmsg;
			    msg->handle = NULL;
			}
		    } else
			(void) TUAppendToc(curdesttoc, MsgGetScanLine(msg));
		    curdesttoc->haschanged = TRUE;
		}
		TocRemoveMsg(toc, msg);
		MsgFree(msg);
		i--;
		if (cur > 40)
		    break;	/* Do only 40 at a time, just to be safe. */
	    } 
	    i++;
	}
	if (curfate != Fignore) {
	    if (!didsomething) {
		didsomething = TRUE;
		BeginLongOperation();
	    }
	    if (curfate == Fmove) {
		argv = ResizeArgv(argv, cur + 4);
		argv[cur++] = MallocACopy("-nolink");
		argv[cur++] = MallocACopy("-src");
		(void) sprintf(str, "+%s", toc->foldername);
		argv[cur++] = MallocACopy(str);
		(void) sprintf(str, "+%s", curdesttoc->foldername);
		argv[cur++] = MallocACopy(str);
	    }
	    if (debug) {
		for (i = 0; i < cur; i++)
		    (void) fprintf(stderr, "%s ", argv[i]);
		(void) fprintf(stderr, "\n");
	    }
	    DoCommand(argv, (char *) NULL, "/dev/null");
	    for (i = 0; argv[i]; i++)
		XtFree((char *) argv[i]);
	    XtFree((char *) argv);
	}
    } while (curfate != Fignore);
    if (AutoPack && toc != WastebasketFolder)
	TocPack(toc);
    if (defSVNStyle) {
	for (i = 0; i < toc->num_scrns; i++) {
	    TURedisplayToc(toc->scrn[i]);
 	}
    }
    for (i=0 ; i<numFolders ; i++) {
	if (folderList[i]->haschanged) {
	    TocReloadSeqLists(folderList[i]);
	    folderList[i]->haschanged = FALSE;
	}
	TocStartUpdate(folderList[i]);
    }
    if (fixedprofile) {
	RenameAndCheck(tempprofilename, profileFileName);
	XtFree(tempprofilename);
    }
    if (didsomething) EndLongOperation();
}


/*
 * Copy the given messages from one toc to another.
 */

void TocCopyMessages(toc, mlist, desttoc)
Toc toc;
MsgList mlist;
Toc desttoc;
{
    char str[300], **argv;
    register int i, p;
    if (mlist->nummsgs == 0 || desttoc == toc) return;
    BeginLongOperation();
    TocStopUpdate(desttoc);
    argv = MakeArgv(mlist->nummsgs + 5);
    p = 0;
    argv[p++] = MallocACopy(refileCmd);
    for (i=0 ; i<mlist->nummsgs ; i++) {
	sprintf(str, "%d", MsgGetId(mlist->msglist[i]));
	argv[p++] = MallocACopy(str);
	(void) TUAppendToc(desttoc, MsgGetScanLine(mlist->msglist[i]));
    }
    argv[p++] = MallocACopy("-link");
    argv[p++] = MallocACopy("-src");
    sprintf(str, "+%s", TocGetFolderName(toc));
    argv[p++] = MallocACopy(str);
    sprintf(str, "+%s", TocGetFolderName(desttoc));
    argv[p++] = MallocACopy(str);
    DoCommand(argv, (char *) NULL, "/dev/null");
    for (i=0 ; i<p ; i++) XtFree(argv[i]);
    XtFree((char *) argv);
    TocStartUpdate(desttoc);
    EndLongOperation();
}    



/* Return whether the given toc can incorporate mail. */

int TocCanIncorporate(toc)
Toc toc;
{
     return (toc && (toc == InitialFolder || toc->incfile || toc -> inchost));
}


/*
 * Incorporate new messages into the given toc.  Returns the msgid of the
 * first new msg, or NULL if none were found.
 */

Msg TocIncorporate(toc)
Toc toc;
{
    char **argv;
    char str[100], str2[10], *file, *ptr;
    Msg msg, firstmessage;
    FILE *fid;
    register int p, num;
    Boolean needsrescan = FALSE;
    Boolean wasonunseen;
    wasonunseen = (strcmp(toc->viewedseq->name, unseenSeqName) == 0);
    TocStopUpdate(toc);
    TUGetFullFolderInfo(toc);
    TUEnsureScanIsValidAndOpen(toc);
    if (toc->validity == invalid || TUScanFileOutOfDate(toc))
	needsrescan = TRUE;
    argv = MakeArgv(9);
    p = 0;
    argv[p++] = incCmd;
    (void) sprintf(str, "+%s", toc->foldername);
    argv[p++] = str;
    argv[p++] = "-width";
    (void) sprintf(str2, "%d", defTocWidth);
    argv[p++] = str2;
     if (toc->inchost) {
        argv [p++] = "-host";
        argv [p++] = toc->inchost;
     } else if (toc->incfile) {
	argv[p++] = "-file";
	argv[p++] = toc->incfile;
    }
    if (ScanFormatFile && *ScanFormatFile) {
	argv[p++] = "-form";
	argv[p++] = ScanFormatFile;
    }
    argv[p++] = "-truncate";
    argv[p++] = NULL;
    file = DoCommandToFile(argv);
    XtFree((char *)argv);
    firstmessage = NULL;
    fid = FOpenAndCheck(file, "r");
    while (ptr = ReadLineWithCR(fid)) {
	num = atoi(ptr + ScanIDCols[0]);
	if (num > 0) {
	    if (toc->viewedseq != toc->seqlist[0])
		TocChangeViewedSeq(toc, toc->seqlist[0]);
	    msg = TUAppendToc(toc, ptr);
	    if (msg->msgid != num)
		needsrescan = TRUE;
	    if (firstmessage == NULL) firstmessage = msg;
	}
    }
    if (needsrescan) {
	TocSetCurMsg(toc, firstmessage);
	TocForceRescan(toc);
	firstmessage = TocGetCurMsg(toc);
    }
    if (firstmessage && firstmessage->visible)
	TocReloadSeqLists(toc);
    if (wasonunseen)
	TocChangeViewedSeq(toc, TocGetSeqNamed(toc, unseenSeqName));
    TocStartUpdate(toc);
    (void) myfclose(fid);
    DeleteFileAndCheck(file);
    return firstmessage;
}



/*
 * The given message has changed.  Rescan it and change the scanfile.  This is
 * done as much as possible in the background and as work procs.
 */

typedef struct _MsgChangedDataRec {
    MsgHandle handle;
    char *foldername;
    int msgid;
    char *tempfile;
} MsgChangedDataRec, *MsgChangedData;

static Boolean WorkMsgChangedStart(), WorkMsgChangedFinished();

void TocMsgChanged(toc, msg)
Toc toc;
Msg msg;
{
    MsgChangedData data;
    if (toc == NULL || msg == NULL || toc->validity != valid) return;
    data = XtNew(MsgChangedDataRec);
    data->handle = MsgGetHandle(msg);
    XtAddWorkProc((XtWorkProc)WorkMsgChangedStart, (Opaque) data);
}
    

static Boolean WorkMsgChangedStart(op)
Opaque op;
{
    MsgChangedData data = (MsgChangedData) op;
    Msg msg = MsgFromHandle(data->handle);
    Toc toc = MsgGetToc(msg);
    char **argv, str[100], str2[10], str3[10];
    register int i;

    if (msg == NULL) {
        if ( data->handle ) {
            MsgFreeHandle(data->handle);
            data->handle = (MsgHandle) NULL;
        }
        XtFree((char *) data);
	return TRUE;
    }
    data->foldername = MallocACopy(toc->foldername);
    data->msgid = msg->msgid;
    data->tempfile = MallocACopy(MakeNewTempFileName());
    argv = MakeArgv(7);
    i = 0;
    argv[i++] = scanCmd;
    (void) sprintf(str, "+%s", toc->foldername);
    argv[i++] = str;
    (void) sprintf(str2, "%d", msg->msgid);
    argv[i++] = str2;
    argv[i++] = "-width";
    (void) sprintf(str3, "%d", defTocWidth);
    argv[i++] = str3;
    if (ScanFormatFile && *ScanFormatFile) {
	argv[i++] = "-form";
	argv[i++] = ScanFormatFile;
    }
    argv[i++] = NULL;
    DoCommandInBackground(argv, (char *) NULL, data->tempfile,
			  WorkMsgChangedFinished, op);
    XtFree((char *) argv);
    return TRUE;
}


static Boolean WorkMsgChangedFinished(op)
Opaque op;
{
    MsgChangedData data = (MsgChangedData) op;
    Msg msg = MsgFromHandle(data->handle);
    Toc toc = MsgGetToc(msg);
    char *ptr;
    int fid, length, delta, i, j;
    FateType fate;
    Toc desttoc;
    TagRec tag;
    int pos;

    if (msg && strcmp(TocGetFolderName(toc), data->foldername) == 0 &&
	    MsgGetId(msg) == data->msgid) {
	length = GetFileLength(data->tempfile);
	ptr = XtMalloc((unsigned) length + 1);
	fid = myopen(data->tempfile, O_RDONLY, 0666);
	if (length != read(fid, ptr, length))
	    Punt("Couldn't read result in WorkMsgChangedFinished!");
	ptr[length] = 0;
	(void) myclose(fid);
	fate = MsgGetFate(msg, &desttoc);
	if (strcmp(ptr, msg->origbuf) != 0) {
	    msg->changed = TRUE;
	    length = strlen(ptr);
	    delta = length - msg->length;
	    if (msg->curbuf != msg->origbuf) XtFree(msg->curbuf);
	    XtFree(msg->origbuf);
	    msg->curbuf = msg->origbuf = ptr;
	    msg->length = length;
	    if (msg->visible) {
		if (delta != 0) {
		    for (i=TUGetMsgPosition(toc, msg)+1; i<toc->nummsgs ; i++)
			toc->msgs[i-1]->position++;
		    toc->lastPos += delta;
		}
		for (j=0 ; j<toc->num_scrns ; j++) {
		    TocDisableRedisplay(toc->scrn[j]->tocwidget);
		    tag.tagFields.tocNumber = TUGetTocNumber(toc);
		    tag.tagFields.msgNumber = TUGetMsgIndex(toc, msg);
		    pos = DXmSvnGetEntryNumber(toc->scrn[j]->tocwidget,
			tag.tagValue);
		    if (pos != 0)
		        DXmSvnInvalidateEntry(toc->scrn[j]->tocwidget, pos);
		    TocEnableRedisplay(toc->scrn[j]->tocwidget);
#ifdef notdef
		    TURedisplayToc(toc->scrn[j]);
#endif
		}
	    }
	    MsgSetFate(msg, fate, desttoc);
	    TUSaveTocFile(toc);
	} else XtFree(ptr);
    } else if (msg) {
	DEBUG(("Msg moved; redoing scan.\n"));
	TocMsgChanged(toc, msg);
    }
    DeleteFileAndCheck(data->tempfile);
    MsgFreeHandle(data->handle);
    XtFree(data->foldername);
    XtFree(data->tempfile);
    XtFree((char *) data);
    return TRUE;
}



Msg TocMsgFromId(toc, msgid)
Toc toc;
int msgid;
{
    register int h, l, m;
    if (toc == NULL) return NULL;
    l = 0;
    h = toc->nummsgs - 1;
    if (h < 0) return NULL;
    while (l < h - 1) {
	m = (l + h) / 2;
	if (toc->msgs[m]->msgid > msgid)
	    h = m;
	else
	    l = m;
    }
    if (toc->msgs[l]->msgid == msgid) return toc->msgs[l];
    if (toc->msgs[h]->msgid == msgid) return toc->msgs[h];
    DEBUG(("TocMsgFromId search failed! hi=%d, lo=%d, msgid=%d\n",h,l,msgid));
    return NULL;
}


/* Tell mh our concept of the current message, if we have one. */

void TocSaveCurMsg(toc)
Toc toc;
{
    Msg oldmsg;
    if (toc->curmsg) {
	TULoadSeqLists(toc, &oldmsg);
	if (oldmsg != toc->curmsg)
	    TocWriteSeqLists(toc);
    }
#ifdef OLDANDSLOW
    char plus[100], str[20], **argv;
    if (toc->curmsg) {
	argv = MakeArgv(7);
	argv[0] = "mark";
	(void) sprintf(plus, "+%s", TocGetFolderName(toc));
	argv[1] = plus;
	(void) sprintf(str, "%d", toc->curmsg->msgid);
	argv[2] = str;
	argv[3] = "-sequence";
	argv[4] = "cur";
	argv[5] = "-add";
	argv[6] = "-zero";
	DoCommand(argv, (char *) NULL, "/dev/null");
	XtFree((char *) argv);
	TocSetCacheValid(toc);
    }
#endif OLDANDSLOW
}



/*
 * Empty the wastebasket.
 */

void TocEmptyWastebasket()
{
    Toc toc = WastebasketFolder;
    register Msg msg;
    register int i, c;
    char str[300];
    if (!defUseWastebasket) return;
    BeginLongOperation();
    TUEnsureScanIsValidAndOpen(toc);
    TocStopUpdate(toc);
    TocCommitChanges(toc);
    c = 0;
    for (i=0 ; i<toc->nummsgs ; i++) {
	if (MsgSetScrn(toc->msgs[i], (Scrn) NULL))
	    goto CLEANUP;
    }
    for (i=toc->nummsgs - 1 ; i >= 0 ; i--) {
	msg = toc->msgs[i];
	(void) sprintf(str, "%s/%d", toc->path, MsgGetId(msg));
	TocRemoveMsg(toc, msg);
	MsgFree(msg);
	(void) unlink(str);
	if (c++ == 100) {
	    SaveQueuedEvents();
	}
    }
    RestoreQueuedEvents();
  CLEANUP:
    TocStartUpdate(toc);
    EndLongOperation();
}


#define TEMPSEQNAME	"MaIlTeMp"

static Boolean FinishExpunge(data)
Opaque data;
{
    Toc toc = (Toc) data;
    Sequence seq;
    Msg msg;
    MsgList mlist;
    char str[300], **argv;
    register int i;
    int c = 0;
    TocStopUpdate(toc);
    TocReloadSeqLists(toc);
    seq = TocGetSeqNamed(toc, TEMPSEQNAME);
    if (seq == NULL) goto CLEANUP; /* None found. */
    mlist = seq->mlist;
    DEBUG(("...%d messages found to expunge.\n", mlist->nummsgs));
    for (i=0 ; i<mlist->nummsgs ; i++) {
	if (MsgChanged(mlist->msglist[i]))
	    goto CLEANUP;
	if (MsgSetScrn(mlist->msglist[i], (Scrn) NULL))
	    goto CLEANUP;
    }
    for (i=mlist->nummsgs - 1 ; i >= 0 ; i--) {
	msg = mlist->msglist[i];
	(void) sprintf(str, "%s/%d", toc->path, MsgGetId(msg));
	TocRemoveMsg(toc, msg);
	MsgFree(msg);
	(void) unlink(str);
	if (c++ == 100) {
	    SaveQueuedEvents();
	}
    }
    RestoreQueuedEvents();
  CLEANUP:
    TocStartUpdate(toc);
    argv = MakeArgv(6);
    argv[0] = markCmd;
    (void) sprintf(str, "+%s", toc->foldername);
    argv[1] = str;
    argv[2] = "-sequence";
    argv[3] = TEMPSEQNAME;
    argv[4] = "-delete";
    argv[5] = "all";
    DoCommand(argv, (char *) NULL, "/dev/null");
    XtFree((char *) argv);
    TocReloadSeqLists(toc);
    TocDeleteNullSequence(toc, TEMPSEQNAME);
    DEBUG(("Expunging completed.\n"));
    return TRUE;
}
    


void TocExpungeOldMessages(toc, days)
Toc toc;
int days;
{
    char str[100], str2[100], **argv;
    if (!toc) return;
    DEBUG(("Expunging messages from %s...\n", TocGetFolderName(toc)));
    TocRecheckValidity(toc);
    argv = MakeArgv(6);
    argv[0] = pickCmd;
    (void) sprintf(str, "+%s", TocGetFolderName(toc));
    argv[1] = str;
    argv[2] = "-sequence";
    argv[3] = TEMPSEQNAME;
    argv[4] = "-before";
    (void) sprintf(str2, "-%d", days);
    argv[5] = str2;
    DoCommandInBackground(argv, (char *) NULL, debug ? NULL : "/dev/null",
			  FinishExpunge, (Opaque) toc);
    XtFree((char *) argv);
}

    


Boolean TocNeedsPacking(toc)
Toc toc;
{
    return (toc != NULL && toc->nummsgs > 0 &&
	    toc->msgs[toc->nummsgs - 1]->msgid != toc->nummsgs);
}


/*
 * Pack the messages in the given folder.
 */

void TocPack(toc)
Toc toc;
{
    char **argv, str[100], format[100];
    register int i, j, k, pos;
    Msg msg;
    TagRec tag;

    if (!TocNeedsPacking(toc)) return;
    BeginLongOperation();
    TocStopUpdate(toc);
    argv = MakeArgv(4);
    argv[0] = folderCmd;
    (void) sprintf(str, "+%s", TocGetFolderName(toc));
    argv[1] = str;
    argv[2] = "-pack";
    argv[3] = "-fast";
    DoCommand(argv, (char *) NULL, "/dev/null");
    XtFree((char *) argv);
    (void) sprintf(format, "%%%dd", ScanIDCols[1]);
    TRACE(("@TocPack: nummsgs = %d\n",toc->nummsgs));
    for (i=0 ; i<toc->nummsgs ; i++) {
	msg = toc->msgs[i];
	if (msg->msgid != i + 1) {
	    msg->msgid = i + 1;
	    (void) sprintf(str, format, msg->msgid);
	    TRACE(("@    orig = %x, cur = %x str = %s\n",msg->origbuf,msg->curbuf,msg->curbuf));
	    for (j=0; j<ScanIDCols[1] ; j++) {
		msg->origbuf[j + ScanIDCols[0]] = str[j];
		msg->curbuf[j + ScanIDCols[0]] = str[j];
	    }
	    tag.tagFields.tocNumber = TUGetTocNumber(toc);
	    tag.tagFields.msgNumber = TUGetMsgIndex(toc, msg);
	    for (k = 0; k < toc->num_scrns; k++) {
	        Widget w = toc->scrn[k]->tocwidget;
		TocDisableRedisplay(w);
		pos = DXmSvnGetEntryNumber(w, tag.tagValue);
		if (pos != 0)
		    DXmSvnInvalidateEntry(w, pos);
		TocEnableRedisplay(w);
	    }
	    MsgRepaintLabels(msg);
	}
    }
    TocReloadSeqLists(toc);
    TocStartUpdate(toc);
    EndLongOperation();
}
TocSetOpened(toc, value)
Toc toc;
Boolean value;
{
    if (toc) toc->opened = value;
}

/* added by aju. The following routine creates a cache file
   in the MH home dir, on all folders and subfolders, before
   exiting dxmail gracefully. Next time over, dxmail reads\
   this cache file instead of scanning MH tree, using next routine.
   Toc_cache writes the information in the folderList[] structure
   to a file in MH directory called .decxmailTOCcache */

int do_not_save_cache = 0;  /* global flag. */
int toc_cache()

{
    FILE *result;
    char str[300];
    int i;
    Toc bfToc;

    if (do_not_save_cache)
      return;

    (void) sprintf(str, "%s/%s", mailDir, ".decxmailTOCcache");
    result = myfopen(str, "w");
    if (result == NULL) {
      sprintf(str, "Error in openToWrite on file %s", str);
      Punt(str);
    }
    sprintf( str,  "%d", numFolders);
    fprintf(result, "%s \n", str);
    for(i=0; i<numFolders;i++)
      {
	bfToc = folderList[i];
	sprintf ( str, "%s %s %d %d\n", bfToc->foldername, bfToc->path, bfToc->subFolderCount, bfToc->level  );
	fprintf( result, "%s", str);
      }
    fclose(result);


}


int toc_cache_read()

{
    FILE *result;
    char str[300], FolderStr[5];
    int i, lvl, count;
    Toc toc;
    unsigned indx;
    char bufoldername[150], bufpath[150];


    do_not_save_cache = 0;
    (void) sprintf(str, "%s/%s", mailDir, ".decxmailTOCcache");
    result = myfopen(str, "r");
    if (result == NULL) {
          printf ( "Scanning MH folders..\n");
          return (0);
	}

    fscanf(result, "%s \n", FolderStr);
    numFolders = atoi (FolderStr);
    folderList = (Toc *) XtMalloc((unsigned) numFolders * sizeof(Toc));
    for(i=0; i<numFolders;i++)
      {
	toc = folderList[i] =  TUMalloc();/* (Toc)&bulk_toc[i]; */
	fscanf ( result, "%s %s %d %d\n", bufoldername, bufpath, &count, &lvl);
	toc->subFolderCount = count;
	toc->level  = lvl;
	toc->path = MallocACopy( bufpath);

	if (lvl)
	  toc->visible = False;
	else
	  toc->visible = True;
	toc->foldername= MallocACopy( bufoldername);
      }
    fclose(result);
    unlink (str);
    return(1);

/*** cache code ends **/
}


void TocResetAll(w)
Widget w;
{
  Scrn scrn = ScrnFromWidget(w);
  do_not_save_cache = 0; /* default is to save cache on exit */
	    if (Confirm(scrn, "Invalidate folder cache when exiting?", 
			 "OnWindow OnFolders OnInvalidatingCache")) 
	      do_not_save_cache = 1; /* invalidate cache */


}









