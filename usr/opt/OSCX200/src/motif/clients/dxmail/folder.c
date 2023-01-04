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
static char rcs_id[] = "@(#)$RCSfile: folder.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/09/21 17:40:35 $";
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

/* folder.c -- implement buttons relating to folders and other globals. */

#include "decxmail.h"
#include "msg.h"
#include "radio.h"
#include "toc.h"
#include "tocintrnl.h"
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>

Widget rename_option;
Widget rename_dialog;
Widget rename_label1, rename_label2;
Widget rename_edit1, rename_edit2;
Widget rename_button, cancel_button;

#define RENAME_RENAME             1
#define RENAME_CANCEL             2

/* Quit. */

void ExecQuit(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    extern void exit();
    Toc toc;
    register int i;
    TRACE(("@ExecQuit\n"));
    if ( check_bg_processes(w) ) return;

    for (i = numScrns - 1; i >= 0; i--)
	if (scrnList[i] != scrn) {
	    if (MsgSetScrn((Msg) NULL, scrnList[i]))
		return;
	}


    for (i = 0; i < numFolders; i++) {
	toc = folderList[i];
	if (TocConfirmCataclysm(toc))
	    return;
    }
    if (MsgSetScrn((Msg) NULL, scrn))
	return;
    for (i=0 ; i<numScrns ; i++)
	DestroyScrn(scrnList[i]);
    XCloseDisplay(theDisplay);
    theDisplay = NULL;
    for (i = 0; i < numFolders; i++) {
	toc = folderList[i];
	TocSaveCurMsg(toc);
    }
    toc_cache(); /* writes folder info into a cache file */
    exit(0);

}


void ExecForceQuit(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    extern void exit();
    Toc toc;
    register int i;
    TRACE(("@ExecForceQuit\n"));
    for (i = numScrns - 1; i >= 0; i--)
	if (scrnList[i] != scrn) {
	    MsgSetScrnForce((Msg) NULL, scrnList[i]);
	}
    for (i = 0; i < numFolders; i++) {
	toc = folderList[i];
	TocCommitChanges(toc);
    }
    MsgSetScrnForce((Msg) NULL, scrn);
    for (i=0 ; i<numScrns ; i++)
	DestroyScrn(scrnList[i]);
    XCloseDisplay(theDisplay);
    theDisplay = NULL;
    for (i = 0; i < numFolders; i++) {
	toc = folderList[i];
	TocSaveCurMsg(toc);
    }
    toc_cache(); /* writes folder info into a cache file */
    exit(0);
}

/* Close this scrn.  If this is the last scrn, quit. */

void ExecCloseScrn(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Scrn nscrn;
    register int i, count;
    Msg msg;
    TRACE(("@ExecCloseScrn\n"));
    ButtonSetRedo(ExecCloseScrn, (caddr_t)w);
    count = 0;
    for (i=0 ; i<numScrns ; i++) {
	nscrn = scrnList[i];
	if (nscrn->mapped && nscrn->tocwidget && nscrn != scrn)
	    count++;
    }
    if (count == 0) ExecQuit(w);
    else {
	msg = scrn->msg;
	if (MsgSetScrn((Msg) NULL, scrn)) return;
	if (msg) TocRedoButtonPixmaps(MsgGetToc(msg));
	DestroyScrn(scrn);
    }
}

/* Open the selected folder in this screen. */

void ExecOpenFolder(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = SelectedToc(scrn);
    TRACE(("@ExecOpenFolder %s\n", TocGetFolderName(toc)));
    if (toc) {
	TocChangeViewedSeq(toc, TocGetSeqNamed(toc, "all"));
	if (AutoCommit && scrn->toc == toc)
	    TocCommitChanges(toc);
	TocSetOpened(toc, True);
	TocSetScrn(toc, scrn);
    }
}

/* Make so that no folder is open in this screen.  How useless. */

void ExecCloseFolder(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecCloseFolder\n"));
    if (scrn->toc)
	TocSetOpened(scrn->toc, False);
    TocSetScrn((Toc) NULL, scrn);
}



/* Make a new scrn displaying the given folder. */

/* ARGSUSED */
void ExecOpenFolderInNewWindow(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc;
    toc = SelectedToc(scrn);
    TRACE(("@ExecOpenFolderInNewWindow %s\n",TocGetFolderName(toc)));
    if (toc) {
	scrn = CreateNewScrn(params[0]);
	TocSetScrn(toc, scrn);
	RadioSetOpened(scrn->folderradio, TocGetFolderName(toc));
	MapScrn(scrn, event->xbutton.time);
    }
}



/* Create a new folder. */

void ExecCreateFolder(w)
Widget w;
{
    Boolean CreateFolder();
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecCreateFolder\n"));
    MakePrompt(scrn, "folderCreate", CreateFolder, "", 
	"OnWindow OnFolders OnCreating_folders");
}



/* Show a hidden folder. */

void ExecShowFolder(w)
Widget w;
{
    Boolean ShowFolder();
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecShowFolder\n"));
    MakePrompt(scrn, "showFolder", ShowFolder, "",
	"OnWindow OnFolders OnShowing_folders");
}

/* Show a hidden folder. */

void ExecShowSubfolders(w)
Widget w;
{
    Boolean ShowSubFolders();
    Scrn scrn = ScrnFromWidget(w);
    char *foldername;
    Toc toc;
    TRACE(("@ExecShowSubfolders\n"));
    toc = SelectedToc(scrn);
    /* treat all folders the same for show/hide *
    if (toc == (Toc)NULL || toc == InitialFolder || 
	  (defUseWastebasket && toc == WastebasketFolder)) {
	foldername = "";
    } */
    if (toc == (Toc)NULL){
      foldername = "";
      MakePrompt(scrn, "showSubfolder", ShowSubFolders, foldername,
		 "OnWindow OnFolders OnShowing_folders");
    }
    else {
      foldername = TocGetFolderName(toc);
      ShowSubFolders( foldername, scrn );
    }
}


void ExecDeleteFolder(w)
Widget w;
{
    Boolean DeleteFolder();
    Scrn scrn = ScrnFromWidget(w);
    char *foldername;
    Toc toc;
    TRACE(("@ExecDeleteFolder\n"));
    toc = SelectedToc(scrn);

/* PJS: Added test for the NULL toc, just in case... */
    if (toc == (Toc)NULL || toc == InitialFolder || toc == DraftsFolder ||
	  (defUseWastebasket && toc == WastebasketFolder)) {
	foldername = "";
    }
    else
	foldername = TocGetFolderName(toc);
    MakePrompt(scrn, "deleteFolder", DeleteFolder, foldername,
	"OnWindow OnFolders OnDeleting_folders");
}



void ExecHideFolder(w)
Widget w;
{
    Boolean HideFolder();
    Scrn scrn = ScrnFromWidget(w);
    char *foldername;
    Toc toc;
    toc = SelectedToc(scrn);
    TRACE(("@ExecHideFolder\n"));
    if (toc == InitialFolder || toc == DraftsFolder ||
	  (defUseWastebasket && toc == WastebasketFolder)) {
	foldername = "";
    } else     foldername = TocGetFolderName(toc);
    MakePrompt(scrn, "hideFolder", HideFolder, foldername,
	"OnWindow OnFolders OnHiding_folders"); 
    
}

void ExecHideSubfolders(w)
Widget w;
{
    Boolean HideSubFolders();
    Scrn scrn = ScrnFromWidget(w);
    char *foldername;
    Toc toc;
    toc = SelectedToc(scrn);
    TRACE(("@ExecHideSubFolders\n"));

/* The 3 core folders get no special treatment, to   *
 * fix a QAR that prevents hiding of subfolders -aju *
    if (toc == InitialFolder || toc == DraftsFolder ||
	  (defUseWastebasket && toc == WastebasketFolder)) {
	foldername = "";
    } else */
/* Fix for if toc is null then prompt the user to get it */

    if (toc == (Toc)NULL){
      foldername = "";
      MakePrompt(scrn, "hideSubfolder", HideSubFolders, foldername,
		 "OnWindow OnFolders OnHiding_folders");
    } else {
    foldername = TocGetFolderName(toc);

/*   MakePrompt(scrn, "hideSubfolder", HideSubFolders, foldername,
	"OnWindow OnFolders OnHiding_folders"); */

    HideSubFolders(foldername, scrn);
	}
}



			/* Debugging stuff only. */
void ExecSyncOn()
{
    (void) XSynchronize(theDisplay, TRUE);
}
void ExecSyncOff()
{
    (void) XSynchronize(theDisplay, FALSE);
}



/* Create a new folder with the given name. */

Boolean CreateFolder(name, scrn)
char *name;
Scrn scrn;
{
    Toc toc, thistoc;
    register int i, position;

    for (i=0 ; name[i] > ' ' ; i++) ;
    name[i] = 0;
    toc = TocGetNamed(name);
    if (toc || i == 0) {
	if (i == 0) {
	    Error(scrn->parent, EFolderEmptyName, (char *) NULL);
	} else {
	    Error(scrn->parent, EFolderExists, name);
	}
	return FALSE;
    }
    toc = TocCreateFolder(name);
    if (toc == NULL) {
	Error(scrn->parent, EFolderCantCreate, name);
	return FALSE;
    }
    if (TocGetVisible(toc)) {
	position = 2;
	for (i = 0; i < numFolders; i++) {
	    thistoc = folderList[i];
	    if (thistoc == toc)
		break;
	    if (thistoc != InitialFolder && thistoc != DraftsFolder &&
	       (!defUseWastebasket || thistoc != WastebasketFolder) &&
	       (TocGetVisible(thistoc)))
		position++;
	}
	RadioAddFolders(scrn->folderradio, &name, 1, position);
    }
    RadioFixFolders(scrn->folderradio);
    TocCheckForNewMail();
    return TRUE;
}

/* Show an existing folder with the given name. */

Boolean ShowFolder(name, scrn)
char *name;
Scrn scrn;
{
    Toc toc, thistoc;
    register int i, position;

    for (i=0 ; name[i] > ' ' ; i++) ;
    name[i] = 0;
    toc = TocGetNamed(name);
    if (toc || i == 0) {
	if (i == 0) {
	    Error(scrn->parent, EFolderEmptyName, (char *) NULL);
	} else {
	    Error(scrn->parent, EFolderExists, name);
	}
	return FALSE;
    }
    toc = TocShowFolder(name);
    if (toc == NULL) {
	Error(scrn->parent, EFolderCantShow, name);
	return FALSE;
    }
    position = 2;
    for (i = 0; i < numFolders; i++) {
	thistoc = folderList[i];
	if (thistoc == toc)
	    break;
	if (thistoc != InitialFolder && thistoc != DraftsFolder &&
	      (!defUseWastebasket || thistoc != WastebasketFolder) &&
	      (TocGetVisible(thistoc)))
	    position++;
    }
    RadioAddFolders(scrn->folderradio, &name, 1, position);
    TocCheckForNewMail();
    return TRUE;
}

/* Show subfolders of a folder */

Boolean ShowSubFolders(name, scrn)
char *name;
Scrn scrn;
{
    Toc toc, thistoc;
    register int i, position;
    int namesize;
    char *thisName;

    for (i=0 ; name[i] > ' ' ; i++) ;
    name[i] = 0;
    toc = TocGetNamed(name);
    if (!toc || !TocGetVisible(toc) || i == 0) {
	if (i == 0) {
	    Error(scrn->parent, EFolderEmptyName, (char *) NULL);
	} else {
	    Error(scrn->parent, EFolderCantShow, name);
	}
	return FALSE;
    }
    namesize = strlen(name);
    for (i = 0; i < numFolders; i++) {
	toc = folderList[i];
	thisName = TocGetFolderName(toc);
	if (strncmp(name, thisName, namesize) == 0 &&
	    strlen(thisName) > namesize &&
	    index(thisName+namesize+1, '/') == NULL &&
	    !TocGetVisible(toc)) {
	    toc = TocMakeFolderVisible(thisName);
	    if (toc == NULL) {
		Error(scrn->parent, EFolderCantShow, name);
		return FALSE;
	    }
	    position = 2;
	    for (i = 0; i < numFolders; i++) {
		thistoc = folderList[i];
		if (thistoc == toc)
		    break;
		if (thistoc != InitialFolder && thistoc != DraftsFolder &&
		    (!defUseWastebasket || thistoc != WastebasketFolder) &&
		    TocGetVisible(thistoc))
		    position++;
	    }
	    RadioAddFolders(scrn->folderradio, &thisName, 1, position);
	}
    }
    TocCheckForNewMail();
    return TRUE;
}
/* Show the subfolders of the named folder */

void TocRevealSubfolders(w, name)
Widget w;
char *name;
{
    Scrn scrn = ScrnFromWidget(w);
    ShowSubFolders(name, scrn);
}

/* Delete the folder with the given name. */

static Boolean DeleteFolder(name, scrn)
char *name;
Scrn scrn;
{
   int subfolder_count = 0;

   Toc toc = TocGetNamed(name);
    if (strlen(name) == 0) {
	Error(scrn->parent, EFolderEmptyName, (char *) NULL);
	return FALSE;
    }
    if (toc == NULL) {
	Error(scrn->parent, EFolderNoSuch, name);
	return FALSE;
    }
    if (toc == InitialFolder || toc == DraftsFolder ||
	  (defUseWastebasket && toc == WastebasketFolder)) {
	Error(scrn->parent, EFolderCantDelete, TocGetFolderName(toc));
	return FALSE;
    }
    subfolder_count = TocSubFolderCount(toc);
    if (subfolder_count){
         Error(scrn->parent, ESubFoldersPresent, name);
         return FALSE;
    }

    if (!AutoCommit && TocConfirmCataclysm(toc))
	return FALSE;

    TocSetScrn(toc, (Scrn) NULL);
    if (AutoCommit)
	TocCommitChanges(toc);
    TocDeleteFolder(toc);
    if (!defSVNStyle)
	RadioDeleteButton(scrn->folderradio, name);

/* PJS: Set the scrn to have the initial folder as TOC, as rmf(1mh) does. */
    TocSetScrn(InitialFolder,scrn);
    if (!defSVNStyle) {
	RadioSetOpened(scrn->folderradio, TocGetFolderName(InitialFolder));
	RadioFixFolders(scrn->folderradio);
    }
    return TRUE;
}


/* Hide the folder with the given name. */

static Boolean HideFolder(name, scrn)
char *name;
Scrn scrn;
{
    Toc toc = TocGetNamed(name);
    if (strlen(name) == 0) {
	Error(scrn->parent, EFolderEmptyName, (char *) NULL);
	return FALSE;
    }
    if (toc == NULL) {
	Error(scrn->parent, EFolderNoSuch, name);
	return FALSE;
    }
    if (toc == InitialFolder || toc == DraftsFolder ||
	  (defUseWastebasket && toc == WastebasketFolder)) {
	Error(scrn->parent, EFolderCantHide, TocGetFolderName(toc));
	return FALSE;
    }
    if (!AutoCommit && TocConfirmCataclysm(toc))
	return FALSE;

    TocSetScrn(toc, (Scrn) NULL);
    if (AutoCommit)
	TocCommitChanges(toc);
    TocHideFolder(toc);
    if (!defSVNStyle)
	RadioDeleteButton(scrn->folderradio, name);
    return TRUE;
}

/* Hide subfolders of the given folder */

static Boolean HideSubFolders(name, scrn)
char *name;
Scrn scrn;
{
    Toc toc = TocGetNamed(name);
    register int i;
    int namesize;
    char *thisName;

    for (i=0 ; name[i] > ' ' ; i++) ;
    name[i] = 0;

    if (strlen(name) == 0) {
	Error(scrn->parent, EFolderEmptyName, (char *) NULL);
	return FALSE;
    }
    if (toc == NULL) {
	Error(scrn->parent, EFolderNoSuch, name);
	return FALSE;
    }
/* allow hiding of subfolders in drafts, inbox, wastebasket
 * so this part is commented out to fix a QAR -- aju

    if (toc == InitialFolder || toc == DraftsFolder ||
	  (defUseWastebasket && toc == WastebasketFolder)) {
	Error(scrn->parent, EFolderCantHide, TocGetFolderName(toc));
	return FALSE;
    }
 *
 * End of commented part                                 */

    namesize = strlen(name);
    for (i = 0; i < numFolders; i++) {
	toc = folderList[i];
	thisName = TocGetFolderName(toc);
	if (strncmp(name, thisName, namesize) == 0 &&
	    strlen(thisName) > namesize &&
	    thisName[namesize] == '/' &&
	    TocGetVisible(toc)) {
	    TocSetScrn(toc, (Scrn) NULL);
	    TocCommitChanges(toc);
	    TocMakeFolderInvisible(thisName);
	    if (!defSVNStyle)
		RadioDeleteButton(scrn->folderradio, thisName);
	}
    }
    return TRUE;
}

void TocHideSubfolders(w, name)
Widget w;
char *name;
{
    Scrn scrn = ScrnFromWidget(w);
    HideSubFolders(name, scrn);
}

/* ARGSUSED */
void ExecDumpScrnWidgetHierarchy(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    FILE *fid;
    fid = myfopen("dump", "w");
    if (fid) {
	DumpIt(fid, scrn->parent, 0);
	(void) myfclose(fid);
    }
}



/**** rename folder code --aju *************/

void renameCB(w,client_data,call_data)
    Widget w;
    XtPointer client_data;
    XmAnyCallbackStruct *call_data;
{
  int command = (int) client_data;
    switch (command)
    {
        case RENAME_RENAME:
            do_rename(w);
            XtUnmanageChild(rename_dialog);
            break;
        case RENAME_CANCEL:
            XtUnmanageChild(rename_dialog);	    
            break;
    }
}


void make_rename_dialog(folder_name, scrn)
char *folder_name;
Scrn scrn;
{

    Arg al[10];
    int ac;
    int client_data;
    ac=0;


    XtSetArg(al[ac],XmNautoUnmanage,False); ac++;
    rename_dialog=XmCreateBulletinBoardDialog(scrn->parent,"rename_dialog",al,ac);

    rename_label1=XmCreateLabel(rename_dialog,"rename_label1",NULL,0);
    XtManageChild(rename_label1);

    
    rename_label2=XmCreateLabel(rename_dialog,"rename_label2",NULL,0);
    XtManageChild(rename_label2);


    rename_edit1=XmCreateText(rename_dialog,"rename_edit1",al,ac);
    XtManageChild(rename_edit1);
    XmTextSetString(rename_edit1, folder_name);

    rename_edit2=XmCreateText(rename_dialog,"rename_edit2",NULL,0);
    XtManageChild(rename_edit2);

    rename_button=XmCreatePushButton(rename_dialog,"rename_button",NULL,0);
    XtManageChild(rename_button);
    client_data = RENAME_RENAME;
    XtAddCallback (rename_button, XmNactivateCallback, (XtCallbackProc)renameCB, (XtPointer) client_data);

    cancel_button=XmCreatePushButton(rename_dialog,"cancel_button",NULL,0);
    XtManageChild(cancel_button);
    client_data = RENAME_CANCEL;
    XtAddCallback (cancel_button, XmNactivateCallback,(XtCallbackProc) renameCB,   (XtPointer) client_data);


}



Boolean RenameThisFolder(name, scrn)
char *name;
Scrn scrn;
{
    Boolean RenameFolder();
    Arg al[10];

    /* create the file error dialog. */


    make_rename_dialog(name, scrn);

    XtManageChild(rename_dialog);

    return;
}



void ExecRenameFolder(w)
Widget w;
{
    Boolean RenameThisFolder();
    Scrn scrn = ScrnFromWidget(w);
    char *foldername;
    Toc toc;
    toc = SelectedToc(scrn);
    TRACE(("@ExecRenameFolder\n"));

    
    if (toc == (Toc)NULL || toc == InitialFolder || toc == DraftsFolder ||
	  (defUseWastebasket && toc == WastebasketFolder)) {
      /* don't let anyone rename these */
      foldername = "";
    }
    else
	foldername = TocGetFolderName(toc);
    RenameThisFolder(foldername, scrn);

}



do_rename(w)
Widget w;
/* renames the folder to the specified name */
{
    Arg al[10];
    int ac, i;
    int result;
    XmTextPosition cursor_pos;
    char *rename_string,*replace_string;
    Scrn scrn = ScrnFromWidget(w);
    rename_string=XmTextGetString(rename_edit1);
    replace_string=XmTextGetString(rename_edit2);

    for (i=0 ; rename_string[i] > ' ' ; i++) 
      if (rename_string[i] == '\n') break;
    rename_string[i] = 0;

    for (i=0 ; replace_string[i] > ' ' ; i++) 
      if (replace_string[i] == '\n') break;
    replace_string[i] = 0;

    result = (int) RenameFolder( rename_string, replace_string, scrn);

    XtFree(rename_string);
    XtFree(replace_string);
    if (!result)
	DEBUG (( "Rename failed\n"));
    return result;
}









Boolean RenameFolder(oldname, newname, scrn)
char *oldname;
char *newname;
Scrn scrn;
{
    Toc toc, thistoc, oldtoc, newtoc, subfoldertoc;
    Toc TocRenameCreateFolder();
    char **argv, plus[100],  seqname[100];
    register int i, w,  position;
    int oldnamesize;
    char *thisOldname, *ptr;
    char newSubFoldername[256];
    char from[256], to[256] , str[256];
    Boolean newVisible;
    HideSubFolders(oldname, scrn);
    for (i=0 ; oldname[i] > ' ' ; i++) ;
    oldname[i] = 0;
    oldtoc = toc = TocGetNamed(oldname);
    if (!toc  || i == 0) {
	if (i == 0) {
	    Error(scrn->parent, EFolderEmptyName, (char *) NULL);
	} else {
	    Error(scrn->parent, EFolderNoSuch, oldname);
	}
	return FALSE;
    }
    if (toc == InitialFolder || toc == DraftsFolder ||
	  (defUseWastebasket && toc == WastebasketFolder)) {
	Error(scrn->parent, EFolderCantDelete, TocGetFolderName(toc));
	return FALSE;
    }
    BeginLongOperation();
    TocForceRescan(toc);
    (void) sprintf(to,  "%s/%s", mailDir, newname);
    (void) sprintf(from, "%s/%s", mailDir, oldname);

    if (rename(from, to) == -1)
      {
	DEBUG(("Cannot rename folder\n"));
	Error(scrn->parent, EFolderExists, newname);
	EndLongOperation();
	return FALSE;
      }

    newtoc = TocRenameCreateFolder(newname);
    if (newtoc == NULL)
      {
	Error(scrn->parent, EFolderCantCreate, newname);	
	rename(to, from); /* undo changes */
	return FALSE;
      }


    oldnamesize = strlen(oldname);
    for (i = 0; i < numFolders; i++) {
	toc = folderList[i];
	thisOldname = TocGetFolderName(toc);
	if (strncmp(oldname, thisOldname, oldnamesize) == 0 &&
	    strlen(thisOldname) > oldnamesize &&
	    thisOldname[oldnamesize] == '/'){
	   thisOldname+=oldnamesize+1;
	   (void) sprintf(to, "%s/%s",  newname, thisOldname);
	   subfoldertoc =  TocRenameCreateFolder(to);
	   

 	}
    }


    for (i = numFolders-1; i >0;  i--) {
	toc = folderList[i];
	if (toc == NULL)
	  {
	    DEBUG (( "NULL toc encountered\n"));
	    continue;
	  }
	thisOldname = TocGetFolderName(toc);
	if ( (strncmp(oldname, thisOldname, oldnamesize) == 0 &&
	    strlen(thisOldname) > oldnamesize &&
	    thisOldname[oldnamesize] == '/') ||
	    (strcmp(oldname, thisOldname)==0)){
	  if ( i < numFolders-1) {
	    for (w=i ; w<numFolders ; w++) folderList[w] = folderList[w+1];
	  }
	  numFolders--;
	  /*
	   * If a subfolder, adjust parent's folder count
	   */
	  strcpy (str, thisOldname);
	  ptr = rindex(str, '/');
	  if (ptr != NULL) {
	    *ptr = '\0';
	    toc = TocGetNamed(str);
	    if (toc) {
	      toc->subFolderCount--;
	    }
	  }
	  if (!defSVNStyle)
	    RadioDeleteButton(scrn->folderradio, thisOldname);

	}
      }

    if (!defSVNStyle) {
      RadioDeleteButton(scrn->folderradio, oldname);
      if (TocGetVisible(newtoc)) {
	position = 2;
	for (i = 0; i < numFolders; i++) {
	  thistoc = folderList[i];
	  if (thistoc == newtoc)
	    break;
	  if (thistoc != InitialFolder && thistoc != DraftsFolder &&
	      (!defUseWastebasket || thistoc != WastebasketFolder) &&
	      (TocGetVisible(thistoc)))
	    position++;
	}
	RadioAddFolders(scrn->folderradio, &newname, 1, position);
      }
      RadioFixFolders(scrn->folderradio);    
      RadioSetOpened(scrn->folderradio, TocGetFolderName(InitialFolder));
    }

    TocSetScrn(InitialFolder,scrn);
    EndLongOperation();
    return TRUE;
}


Toc TocRenameCreateFolder(foldername)
char *foldername;
{
    char str[500];
    char *ptr;
    Toc toc;
    Boolean newVisible;
    void TUGetFullFolderInfo();

    if (TocGetNamed(foldername)) return NULL;
    (void) sprintf(str, "%s/%s", mailDir, foldername);
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
    TUGetFullFolderInfo(toc);
    return toc;
}

