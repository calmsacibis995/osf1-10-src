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
	@(#)$RCSfile: externs.h,v $ $Revision: 1.1.6.5 $ (DEC) $Date: 1993/09/09 16:30:35 $
*/

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

#define PROTO

extern int errno;

extern char *getenv();
extern void bcopy();

#include <DXm/DXmSvn.h> 

#ifdef PROTO

/* actions.c */
extern int InitActions(XtActionList actions, Cardinal num);
extern int ActionNewScrn(Scrn scrn);
extern void TieButtonToFunc(Button button, XtActionProc func);
extern void FuncChangeEnabled(XtActionProc func, Scrn scrn, int enabled);
extern Boolean FuncGetEnabled(XtActionProc func, Scrn scrn);
extern XtActionProc NameToFunc(char *name);

/* button.c */
extern Button ButtonCreate(Scrn scrn, Widget parent, char *name);
extern Button ButtonMake(Widget widget);
extern void ButtonAddFunc(Button button, XtActionProc func, char **params, Cardinal num_params);
extern void ButtonChangeEnabled(Button button, int enabled);
extern Widget ButtonGetWidget(Button button);
extern Scrn ButtonGetScrn(Button button);
extern void RedoLastButton(void);
extern void ButtonSetRedo(void (*proc)(), caddr_t param);

/* command.c */
extern int ChildDone(void);
extern int DoCommand(char **argv, char *inputfile, char *outputfile);
extern int DoExecToFile(char **argv, char *inputfile, char *outputfile);
extern char *DoCommandToString(char **argv);
extern char *DoCommandToFile(char **argv);
extern void DoCommandInBackground(char **argv, char *inputfile, char *outputfile, XtWorkProc proc, Opaque param);
extern int sleep_for_child(void);
extern int pr_argv(char **argv);

/* compfuncs.c */
extern void ExecCompReset(Widget w);
extern void ExecComposeMessage(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecSendDraft(Widget w);
extern void ExecSaveDraft(Widget w);
extern int CreateForward(MsgList mlist, char *name, Time eventtime);
extern void ExecComposeUsingFile(Widget w, XEvent *event, char **params, Cardinal *num_params);

/* customize.c */
extern void CustomizeCreate(void);

/* folder.c */
extern void ExecQuit(Widget w);
extern void ExecCloseScrn(Widget w);
extern void ExecOpenFolder(Widget w);
extern void ExecCloseFolder(Widget w);
extern void ExecOpenFolderInNewWindow(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecCreateFolder(Widget w);
extern void ExecShowFolder(Widget w);
extern void ExecShowSubfolders(Widget w);
extern void ExecDeleteFolder(Widget w);
extern void ExecHideFolder(Widget w);
extern void ExecHideSubfolders(Widget w);
extern void ExecSyncOn(void);
extern void ExecSyncOff(void);
extern Boolean CreateFolder(char *name, Scrn scrn);
extern Boolean ShowFolder(char *name, Scrn scrn);
extern Boolean ShowSubFolders(char *name, Scrn scrn);
extern void TocRevealSubfolders(Widget w, char *name);
extern void TocHideSubfolders(Widget w, char *name);
extern void ExecDumpScrnWidgetHierarchy(Widget w, XEvent *event, char **params, Cardinal *num_params);

/* icon.c */
extern void SetIconifyIcon(Widget w, Pixmap iconifyPixmap);
extern Pixmap GetPixmapNamed(char *str);
extern Pixmap GetBitmapNamed(char *str);
extern void IconInit(void);

/* init.c */
extern int Syntax(void);
extern int InitializeWorld(unsigned int argc, char **argv);
extern void MapStructureToGlobals();
extern int get_first_number(char *range_str);
extern int get_last_number(char *range_str);

/* main.c */
extern int main(unsigned int argc, char **argv);

/* menus.c */
extern int ParseToFuncAndParams(char *ptr, XtActionProc *func, char ***params, Cardinal *num_params);
extern void CreateWidgetFromLayout(Scrn scrn, Widget parent, char *name);
extern void MakeMenu(Scrn scrn, char *name);
extern Widget GetAppShell(Widget client);
extern void ExecPopupHelpMenu(Widget client, char *closure, int call_data);
extern void ExecCreateHelpMenu(Widget client, XEvent *event, char **params, int *num_params);
extern Widget GetMainWindow(Widget client);
extern void ExecOnContext(Widget client, XEvent *event, char **params, Cardinal *num_params);
extern void destroyHelpDialog(Widget client, XmAnyCallbackStruct *cb);
extern int InitMenu(void);
extern Boolean ResizeFolderBox(Widget widget, XConfigureEvent *event);

/* mlist.c */
extern MsgList MakeNullMsgList(void);
extern void AppendMsgList(MsgList mlist, Msg msg);
extern void DeleteMsgFromMsgList(MsgList mlist, Msg msg);
extern MsgList MakeSingleMsgList(Msg msg);
extern void FreeMsgList(MsgList mlist);
extern MsgList StringToMsgList(Toc toc, char *str);

/* msg.c */
extern char *MsgGetName(Msg msg);
extern void RedisplayMsg(Scrn scrn);
extern char *MsgFileName(Msg msg);
extern MsgType MsgGetMsgType(Msg msg);
extern void MsgCreateSource(Msg msg, int editable,Scrn scrn);
extern void MsgDestroySource(Msg msg);
extern void MsgDestroyPS(Msg msg);
extern void MsgSaveChanges(Msg msg);
extern int MsgSetScrn(Msg msg, Scrn scrn);
extern int MsgSetScrnForComp(Msg msg, Scrn scrn);
extern void gSetScrnForce(Msg msg, Scrn scrn);
extern void MsgRepaintLabels(Msg msg);
extern void MsgSetFate(Msg msg, FateType fate, Toc desttoc);
extern void MsgBatchSetFate(MsgList mlist, FateType fate, Toc desttoc);
extern FateType MsgGetFate(Msg msg, Toc *toc);
extern void MsgSetTemporary(Msg msg);
extern Boolean MsgGetTemporary(Msg msg);
extern void MsgSetPermanent(Msg msg);
extern int MsgGetId(Msg msg);
extern char *MsgGetScanLine(Msg msg);
extern Toc MsgGetToc(Msg msg);
extern void MsgSetReapable(Msg msg);
extern void MsgClearReapable(Msg msg);
extern Boolean MsgGetReapable(Msg msg);
extern void MsgSetEditable(Msg msg);
extern void MsgClearEditable(Msg msg);
extern Boolean MsgGetEditable(Msg msg);
extern Boolean MsgChanged(Msg msg);
extern void MsgSetChanged(Msg msg, int value);
extern void MsgSetCallOnChange(Msg msg, void (*func)(), Opaque param);
extern void MsgClearCallOnChange(Msg msg);
extern void MsgSend(Msg msg);
extern Boolean sendres(char *file);
extern void MsgLoadComposition(Msg msg);
extern void MsgLoadReply(Msg msg, Msg frommsg);
extern void MsgLoadForward(Msg msg, MsgList mlist,Scrn scrn);
extern void MsgLoadCopy(Msg msg, Msg frommsg);
extern Boolean MsgLoadFromFile(Msg msg, char *filename, Scrn scrn);
extern void MsgCheckPoint(Msg msg);
extern Msg MsgMalloc(void);
extern void MsgFree(Msg msg);
extern void MsgRemoveFromSeq(Msg msg, char *seqname);
extern XmTextSource MsgGetSource(Msg msg);
extern MsgHandle MsgGetHandle(Msg msg);
extern void MsgFreeHandle(MsgHandle handle);
extern Msg MsgFromHandle(MsgHandle handle);
extern char *MsgGetDDIFFile(Msg msg);
extern void InitMsg(void);
extern void MsgSetMsgText(Scrn scrn, Msg msg);

/* pick.c */

extern void DoPickYes(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void CreatePick(char *foldername, char *from, char *to);
extern void PickBeginLongOperation(void);
extern void PickEndLongOperation(void);

/* popup.c */
extern void DoPromptYes(void);
extern void MakePrompt(Scrn scrn, char *name, Boolean (*func)(), char *initvalue, char *helpString);
extern void MakeFileSelect(Scrn scrn, Boolean (*func)(), char *label, char *helpString);
extern void DestroyConfirmWindow(void);
extern Boolean Confirm(Scrn scrn, char *str, char *helpString);
extern void Warning(Widget parent, char *name, char *ptr);
extern void Error(Widget parent, char *name, char *ptr);
extern void NoNewMailWarning(Scrn scrn);
extern void ExecPopupMenu(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void PopupSvnMenu(Widget w, int closure, DXmSvnCallbackStruct *data);
extern void PopupFinished(void);
extern void InitPopup(void);
extern void mesgDisableRedisplay(void);
extern void mesgEnableRedisplay(void);
extern int Message(Widget parent, char *newText);

/* radio.c */
#define _radio.h
extern Radio RadioCreate(void);
extern void InitFolderPixmaps(Widget widget);
extern void RadioAddWidget(Radio radio, Widget widget, XtActionProc func, char **params, Cardinal num_params, int isFolder);
extern void RadioSetCurrent(Radio radio, char *name);
extern void RadioSetOpened(Radio radio, char *name);
extern void RadioFixFolders(Radio radio);
extern void RadioAddButtons(Radio radio, char **names, int num_names, int position);
extern void RadioAddFolders(Radio radio, char **names, int num_names, int position);
extern void RadioDeleteButton(Radio radio, char *name);
extern char *RadioGetCurrent(Radio radio);
extern Cardinal RadioGetNumChildren(Radio radio);
extern char *RadioGetName(Radio radio, int i);

/* screen.c */
extern void DisableEnablingOfButtons(void);
extern void EnableEnablingOfButtons(void);
extern void EnableScrnsButtons(void);
extern void ForceEnableScrnsButtons(void);
extern void ForceEnableScrnsDeleteButtons(void);
extern void EnableProperButtons(Scrn scrn);
extern Scrn CreateNewScrn(char *name);
extern Scrn CreateReadScrn(char *name);
extern void DestroyScrn(Scrn scrn);
extern void MapScrn(Scrn scrn, Time mapTime);
extern Scrn ScrnFromWidget(Widget w);
extern void ScrnNeedsTitleBarChanged(Scrn scrn);
extern void ScrnNeedsIconNameChanged(Scrn scrn);
extern void MakeDDIFWidgets(Scrn scrn);
extern void FreeDDIFInfo(DDIFFileInfo info);
extern void DDIFShowFile(Scrn scrn, char *filename);
extern void PSShowFile(Scrn scrn, char *filename);
extern void ExecViewInDDIFViewer(Widget w);
extern void ExecPSOrient(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern Boolean IsDeleteGrey(Scrn scrn, int msgsselected, Msg msg);

/* toc.c */
extern void TocInit(void);
extern Toc TocCreateFolder(char *foldername);
extern Toc TocShowFolder(char *foldername);
extern Toc TocIncludeFolderName(char *foldername);
extern void TocCheckForNewMail(void);
extern void TocDeleteFolder(Toc toc);
extern void TocHideFolder(Toc toc);
extern void TocSetScrn(Toc toc, Scrn scrn);
extern void TocRemoveMsg(Toc toc, Msg msg);
extern void TocRecheckValidity(Toc toc);
extern void TocMakeVisible(Toc toc, Msg msg);
extern void TocSetCurMsg(Toc toc, Msg msg);
extern Msg TocGetCurMsg(Toc toc);
extern Msg TocGetFirstMsg(Toc toc);
extern Msg TocGetLastMsg(Toc toc);
extern Msg TocMsgAfter(Toc toc, Msg msg);
extern Msg TocMsgBefore(Toc toc, Msg msg);
extern void TocForceRescan(Toc toc);
extern void TocCheckSeqButtons(Toc toc);
extern void TocReloadSeqLists(Toc toc);
extern void TocWriteSeqLists(Toc toc);
extern int TocHasSequences(Toc toc);
extern void TocChangeViewedSeq(Toc toc, Sequence seq);
extern Sequence TocGetSeqNamed(Toc toc, char *name);
extern Sequence TocViewedSequence(Toc toc);
extern Boolean TocCreateNullSequence(Toc toc, char *name);
extern Boolean TocDeleteNullSequence(Toc toc, char *name);
extern MsgList TocCurMsgList(Toc toc, Scrn scrn);
extern Boolean TocHasSelection(Toc toc, Scrn scrn);
extern void TocUnsetSelection(Toc toc);
extern Msg TocMakeNewMsg(Toc toc);
extern void TocStopUpdate(Toc toc);
extern void TocStartUpdate(Toc toc);
extern void TocSetCacheValid(Toc toc);
extern char *TocGetFolderName(Toc toc);
extern Toc TocGetNamed(char *name);
extern int TocSubFolderCount(Toc toc);
extern Boolean TocGetVisible(Toc toc);
extern Toc TocMakeFolderVisible(char *foldername);
extern Toc TocMakeFolderInvisible(char *foldername);
extern int TocConfirmCataclysm(Toc toc);
extern void TocCommitChanges(Toc toc);
extern void TocCopyMessages(Toc toc, MsgList mlist, Toc desttoc);
extern int TocCanIncorporate(Toc toc);
extern Msg TocIncorporate(Toc toc);
extern void TocMsgChanged(Toc toc, Msg msg);
extern Msg TocMsgFromId(Toc toc, int msgid);
extern void TocSaveCurMsg(Toc toc);
extern void TocEmptyWastebasket(void);
extern void TocExpungeOldMessages(Toc toc, int days);
extern Boolean TocNeedsPacking(Toc toc);
extern void TocPack(Toc toc);
extern int TocSetOpened(Toc toc, int value);
extern int toc_cache(void);
extern int toc_cache_read(void);
extern void TocResetAll(Widget w);

/* tocfuncs.c */
extern void ExecNextView(Widget w);
extern void ExecPrevView(Widget w);
extern void ExecViewNew(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecViewInSpecified(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern Scrn GetDefaultViewScrn(void);
extern void ExecViewInDefault(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecViewMsg(Widget w, Msg msg);
extern void ExecTocForward(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecTocUseAsComposition(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecMarkDelete(Widget w);
extern void ExecMarkCopy(Widget w);
extern void ExecMarkMove(Widget w);
extern void ExecMarkCopyDialog(Widget w);
extern void ExecMarkMoveDialog(Widget w);
extern void ExecMarkUnmarked(Widget w);
extern void ExecCommitChanges(Widget w);
extern void ExecEmptyWastebasket(Widget w);
extern void ExecPrintMessages(Widget w);
extern void ExecPrintWidget(Widget w);
extern void stripMessage(char *from, char *to);
extern void ExecPrintStripped(Widget w);
extern void ExecPrintWidgetStripped(Widget w);
extern void ExecPack(Widget w);
extern void ExecSort(Widget w);
extern void ExecForceRescan(Widget w);
extern void ExecIncorporate(Widget w);
extern void ExecReadNewMail(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecReadNewMailInNew(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecShowUnseen(Widget w);
extern void ExecTocReply(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecPick(Widget w);
extern void ExecPickInSelected(Widget w);
extern void ExecOpenSeq(Widget w);
extern void TwiddleSequence(Scrn scrn, MsgList mlist, TwiddleOperation op);
extern void ExecAddToSeq(Widget w);
extern void ExecRemoveFromSeq(Widget w);
extern void ExecDeleteSeq(Widget w);
extern void ExecCreateSeq(Widget w);
extern void ExecSelectAll(Widget w, XEvent *event, char **params, Cardinal *num_params);

/* tocout.c */
extern Pixmap MsgGetPixmap(Toc toc, Msg msg);
extern void RedoButtonPixmaps(Scrn scrn);
extern void TocRedoButtonPixmaps(Toc toc);
extern void ExecSelectThisMsg(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecExtendThisMsg(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecExtendThisToc(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void TocDisableRedisplay(Widget w);
extern void TocEnableRedisplay(Widget w);

/* tocutil.c */
extern Toc TUMalloc(void);
extern int TUScanFileOutOfDate(Toc toc);
extern void TUScanFileForToc(Toc toc);
extern int TUGetTocNumber(Toc toc);
extern int TUGetMsgPosition(Toc toc, Msg msg);
extern int TUGetMsgIndex(Toc toc, Msg msg);
extern void TUResetTocLabel(Scrn scrn);
extern void TURedisplayToc(Scrn scrn);
extern void TULoadSeqLists(Toc toc, Msg *curmsg);
extern void TURefigureWhatsVisible(Toc toc);
extern void TULoadTocFile(Toc toc);
extern void TUSaveTocFile(Toc toc);
extern void TUEnsureScanIsValidAndOpen(Toc toc);
extern void TURefigureTocPositions(Toc toc);
extern void TUGetFullFolderInfo(Toc toc);
extern Msg TUAppendToc(Toc toc, char *ptr);

/* util.c */
extern int Punt(char *str);
extern void NoOp(void);
extern int myopen(char *path, int flags, int mode);
extern FILE *myfopen(char *path, char *mode);
extern int myclose(int fid);
extern int myfclose(FILE *file);
extern void NukeDirectory(char *path);
extern char *MakeNewTempFileNameInSameDir(char *filename);
extern char *MakeNewTempFileName(void);
extern char **MakeArgv(int n);
extern char **ResizeArgv(char **argv, int n);
extern FILE *FOpenAndCheck(char *name, char *mode);
extern int OpenAndCheck(char *path, int flags, int mode);
extern char *ReadLine(FILE *fid);
extern char *ReadLineWithCR(FILE *fid);
extern int DeleteFileAndCheck(char *name);
extern int CopyFileAndCheck(char *from, char *to);
extern int RenameAndCheck(char *from, char *to);
extern void WriteAndCheck(int fid, char *buf, int length);
extern void LSeekAndCheck(int fid, int offset, int whence);
extern void MkDirAndCheck(char *name, int mode);
extern char *MallocACopy(char *str);
extern int FileExists(char *file);
extern int LastModifyDate(char *file);
extern Boolean IsDirectory(char *file);
extern int GetFileLength(char *file);
extern int ChangeLabel(Widget widget, char *str);
extern Widget CreateTitleBar(Scrn scrn, char *name);
extern int Feep(void);
extern MsgList CurMsgListOrCurMsg(Toc toc, Scrn scrn);
extern MsgList GetSelectedMsgs(Scrn scrn);
extern Toc SelectedToc(Scrn scrn);
extern char *SelectedSeqName(Scrn scrn);
extern int strncmpIgnoringCase(char *str1, char *str2, int length);
extern void ExecOnDouble(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern Cursor MakeCursor(Widget widget, char *name);
extern void BeginLongOperation(void);
extern void EndLongOperation(void);
extern Boolean CheckWorkOK(void);
extern void SaveQueuedEvents(void);
extern void RestoreQueuedEvents(void);
extern char *ExtractStringFromCompoundString(XmString cstr);
extern char *GetApplicationResourceAsString(char *name, char *class);
extern int GetMagicNumber(char *filename);
extern Boolean DDIFFileCheck(int fid);
extern int WasteFolder(char *foldername);
extern void AddProtocols(Widget widget, XtCallbackProc delete, XtCallbackProc save);

/* version.c */
extern char *Version(void);

/* viewfuncs.c */
extern void SkipToMsg(Scrn scrn, int wantnew);
extern void ExecViewBefore(Widget w);
extern void ExecViewAfter(Widget w);
extern void ExecNextSelected(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecPrevSelected(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecThisReply(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecThisForward(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecThisInDefault(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecThisInNew(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecThisUseAsComposition(Widget w, XEvent *event, char **params, Cardinal *num_params);
extern void ExecEditView(Widget w);
extern void ExecSaveView(Widget w);
extern void ExecThisPrintStripped(Widget w);
extern void ExecThisPrint(Widget w);
extern void ExecThisPrintSelective(Widget w);
extern void MakeMoveOrCopyDialog(Scrn scrn, MsgList mlist, int copy);
extern void ExecThisMoveDialog(Widget w);
extern void ExecThisMove(Widget w);
extern void ExecThisCopyDialog(Widget w);
extern void ExecThisCopy(Widget w);
extern void ExecThisDelete(Widget w);
extern void ExecThisUnmark(Widget w);
extern void ExecNewMailInView(Widget w);
extern void ExecMsgAddToSeq(Widget w);
extern void ExecMsgRemoveFromSeq(Widget w);
extern void ExecMakeDefaultView(Widget w);
extern void ExecInsertFile(Widget w);
extern void ExecExtractMsg(Widget w);
extern void ExecExtractMsgStrip(Widget w);
extern void ExecCopy(Widget w);
extern void ExecCut(Widget w);
extern void ExecPaste(Widget w);

/* widgetinfo.c */
extern int GetShadowThickness(Widget widget);
extern char *GetWidgetName(Widget widget);
extern unsigned int GetBorderWidth(Widget widget);
extern Widget GetParent(Widget widget);
extern unsigned int GetHeight(Widget widget);
extern unsigned int GetWidth(Widget widget);
extern int GetX(Widget widget);
extern int GetY(Widget widget);
extern void SetInsertPosition(CompositeWidget widget, XtOrderProc func);
extern void SetX(Widget widget, int x);
extern void SetY(Widget widget, int y);
extern void SetBorderPixmap(Widget widget, Pixmap border_pixmap);
extern void SetFocus(Widget widget, Time focusTime);
extern Boolean IsScrollbarWidget(Widget widget);
extern void DumpIt(FILE *fid, Widget widget, int indent);
extern Widget GetFirstChild(Widget widget);

#endif /* PROTO */



#ifndef PROTO


/* command.c */

extern char *DoCommandToFile();
extern char *DoCommandToString();
extern void DoCommandInBackground();
extern int  please_fork();

/* util.c */

extern void NoOp();
extern int myopen();
extern FILE *myfopen();
extern int myclose();
extern int myfclose();
extern void NukeDirectory();
extern char *MakeNewTempFileNameInSameDir();
extern char *MakeNewTempFileName();
extern char **MakeArgv();
extern char **ResizeArgv();
extern FILE *FOpenAndCheck();
extern char *ReadLine();
extern char *ReadLineWithCR();
extern char *MallocACopy();
extern char *MakeFileName();
extern Boolean IsDirectory();
extern Widget CreateTitleBar();
extern MsgList CurMsgListOrCurMsg();
extern MsgList GetSelectedMsgs();
extern Toc SelectedToc();
extern char *SelectedSeqName();
extern Cursor MakeCursor();
extern void BeginLongOperation();
extern void EndLongOperation();
extern Boolean CheckWorkOK();
extern void SaveQueuedEvents();
extern void RestoreQueuedEvents();
extern char *ExtractStringFromCompoundString();
extern char *GetApplicationResourceAsString();
extern int GetMagicNumber();
extern Boolean DDIFFileCheck();

extern void     doHelp();			/*SM*/
extern void     doOnContext();			/*SM*/
extern void     destroyHelpDialog();	    	/*SM*/
/* screen.c */

extern void DisableEnablingOfButtons();
extern void EnableEnablingOfButtons();
extern void EnableScrnsButtons();
extern void ForceEnableScrnsButtons();
extern void EnableProperButtons();
extern Scrn CreateNewScrn();
extern Scrn CreateReadScrn();
extern void MapScrn();
extern void DestroyScrn();
extern Scrn ScrnFromWidget();
extern void ScrnNeedsTitleBarChanged();
extern void ScrnNeedsIconNameChanged();
extern void MakeDDIFWidgets();
extern Boolean IsDeleteGrey();

/* button.c */

extern void RedoLastButton();

/* icon.c */

extern Pixmap GetPixmapNamed();
extern Pixmap GetBitmapNamed();
extern void IconInit();

/* version.c */

extern char *Version();

/* folder.c */

extern void OpenFolder();

/* tocfuncs.c */

extern void OpenSequence();

/* actions.c */

extern XtActionProc NameToFunc();
extern void TieButtonToFunc();
extern void FuncChangeEnabled();

/* menus.c */

extern void CreateWidgetFromLayout();
extern void MakeMenu();
extern Boolean ResizeFolderBox();

/* popup.c */

extern void MakePrompt();
extern Boolean Confirm();
extern void Warning();
extern void Error();
extern void NoNewMailWarning();
extern void DoPromptYes();
extern void DestroyConfirmWindow();
extern void PopupFinished();
extern void InitPopup();

/* viewfuncs.c */

extern void SkipToMsg();
extern void MakeMoveOrCopyDialog();

/* tocfuncs.c */

extern Scrn GetDefaultViewScrn();
extern void TwiddleSequence();

/* tocout.c */

#ifdef notdef
extern Msg WhichMsg();
#endif
extern void RedoButtonPixmaps();
extern void TocReButtonPixmaps();

/* toc.c */

extern Toc TocMakeFolderVisible();
extern void TocRevealSubfolders();
extern void TocHideSubfolders();

/* customize.c */

extern void CustomizeCreate();

/* pick.c */

extern void CreatePick();
extern void PickBeginLongOperation();
extern void PickEndLongOperation();
extern void DoPickYes();

/* widgetinfo.c */

extern char *GetWidgetName();
extern unsigned int GetBorderWidth();
extern unsigned int GetHeight();
extern unsigned int GetWidth();
extern Widget GetParent();
extern int GetX();
extern int GetY();
extern void SetInsertPosition();
extern void SetX();
extern void SetY();
extern void SetBorderPixmap();
extern void SetFocus();
extern Boolean IsScrollbarWidget();
extern Widget GetFirstChild();
extern void DumpIt();



#endif /* ifndef PROTO */
