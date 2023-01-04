
/*  module: notepad.h   "v1.0"
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
 *	Header file for Notepad
 *
 * NOTES:
 *	
 *
 * REVISION HISTORY:
 * [cjh]  07-Jul-93
 *        Inserted "Error codes" (#define's) to support CopyFile().
 */

/* BuildSystemHeader added automatically */
/* $Header: [notepad.h,v 1.4 91/08/17 05:59:46 rmurphy Exp ]$ */


/* Include files. */
#include <stdio.h>
#include <stdlib.h>

#if defined(VMS)
#include <stat.h>
#include <file.h>
#include <string.h>
#include <rms.h>
#include <descrip.h>
#else
#include <sys/types.h> 
#include <sys/file.h>
#include <sys/stat.h>
#include <strings.h>
#endif

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#if defined(VMS)
#include <decw$cursor.h>
#else
#include <X11/decwcursor.h>
#endif
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <DXm/DXmHelpB.h>
#include <Mrm/MrmAppl.h>
#include <X11/Xatom.h>
#include <Xm/TextP.h>
#include <X11/Vendor.h>
#include <Xm/PanedW.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#if defined(VMS)
#if 0
#include <cda$def.h>
#endif
#define R_OK    4/* test for read permission */
#define W_OK    2/* test for write permission */
#define X_OK    1/* test for execute (search) permission */
#define F_OK    0/* test for presence of file */
#define NormalStatus 1
#else
#define NormalStatus 0
#endif

#ifndef vax 
#define const 
#endif

extern MrmType	*dummy_class;

extern void DoFilter();
extern void DoFilterFromDialog();
extern void sigPipeAbort();
extern void enumerateFilters();
 
#define Feep()			XBell(CurDpy, 50)
#define GetHeight(widget)	((widget)->core.height)
#define GetWidth(widget)	((widget)->core.width)

#define MakeArg(n, v){  	args[numargs].name = n; 	\
			        args[numargs].value = v;	\
			        numargs++; 			\
		      }
 

typedef struct _QueueElement{
    struct _QueueElement *flink, *blink;
} QueueElement;

/* Sructure represting each text widget or "view". Notepad supports
 * an arbitrary number of views each separated by a pane in the 
 * main window. 
 */ 
typedef struct _view{
    struct _view *flink, *blink;	/* Forward/backward links */
    Widget  widget;
    Boolean is_file_view;
    Dimension height;
    XmTextPosition cursorPos;		/* Position of cursor in this view */
} View;

typedef struct {
    int count;
    char *out;
    XmTextPosition inPos;
    XmTextPosition begPos;
    char *selectionBuffer;
} FilterData;

/*	misc externs 	*/
extern XmTextSource PSourceCreate();
extern PSourceDestroy();
extern PSchanges();

extern XmTextSource _XmStringSourceCreate();
extern char *fontName, *setFontName;;

extern void DoSearchNext();
extern void DoSearchPrevious();

/*	externals in util.c 	*/
extern void BeginLongOperation();
extern void EndLongOperation();
extern void DoRevertFont();
extern void DoApplyFont();
extern XFontStruct *globalFontStruct;
extern XFontStruct *setFontStruct;
extern void DoSetFontFamily(), DoSetFontSize(), DoSetFontMisc();
extern void DoSetFont();
extern void DoCustomizeFont();
extern void ShowError();
extern void journalCallback();
extern char *decodeCS();
extern DoLine();
extern XeditPrintf();
extern setWidgetValue();
extern getWidgetValue();
extern Widget makeCommandButton();
extern Widget makeBooleanButton();
extern Widget makeStringBox();
extern FixScreen();
extern fileLock();
extern void setSourceEditable();
extern XmTextSource setSources();
 
extern DoQuit();
extern DoUndo();
extern DoRedo();
extern DoUndoMore();
extern DoSave();
extern DoLoad();
extern DoEdit();
extern void doSaveWithFile();

extern XmTextSource PseudoDiskSourceCreate();
extern void PseudoDiskSourceDestroy();

/*
#ifndef R_OK
#define R_OK 4
#define W_OK 2
#endif
*/
	/* Search stuff */
extern void makeSearchOptionsDialog();
extern void mkSimpleFindDialog();
extern void mkIncrDialog();
extern void mkLineDialog();
extern void mkSaveDialog();
extern void mkReplaceDialog();
extern void cancelDialog();
extern void hideSimpleDialog();
extern void hideSimpleDialog1();
extern void makeUndoDialog();
extern void _doUndo();
extern void mkFilterDialog();
extern void grabSelection();

typedef struct {
        Widget popupWidget;
	Widget dialogWidget;
	Widget stringWidget;
	Widget stringWidget_1;
	int (*createProc)();
        Widget buttons[10];  
	int pb_count;
} simpleDialog;

extern void showSimpleDialog();

#define st_number		7	/* how many toggle bits */
#define st_Forward		0
#define st_Backward		1
#define st_WithinSelected	2
#define st_CaseSensitive	3
#define st_WordWrap		4
#define st_Incremental		5
#define st_RegExp		6

/* error messages */
#define ERR_NOERR     0    /* operation succeeded, no errors */
#define ERR_NOORIG    1    /* original object doesn't exist */
#define ERR_NOCOPY    2    /* original object exists, but copy failed */

extern void callClosureWithSelection();

extern void SetSearchToggle();

extern void DoReplaceOne();
extern void DoReplaceAll();
extern void DoSearchRight();
extern void DoSearchLeft();
extern void DoSearchNextAndFinish();
extern void DoSearchPrevAndFinish();
extern void DoSearchPreviousForSelection();
extern void DoSearchNextForSelection();
extern void SearchProc();
extern void TopProc();
extern void BottomProc();
extern void DoJump();
extern void GotoLineProc();
extern void GotoPositionProc();
extern void ReplaceOnceProc();
extern void ReplaceSelectedProc();
extern void ReplaceAllProc();
extern void DismissSearchProc();
extern void SearchStringModified();
extern void DoShowLine();
extern void DoGotoLine();
extern void DoInclude();

/*clipboard */
extern void DoCut();
extern void DoCopy();
extern void DoPaste();


#ifdef VMS
struct fileID{
	short id;
	short seq;
	short vol;
} ;
struct lsb{
	short cond;
	short reserved;
	int id;
} ;

#endif /* VMS */


/* 
 * Huge global structure holding most of Notepad's state
 */
typedef struct{
    Widget _workArea;
    Widget _workAreaPane;
    Widget _helpWidget;
    Widget _toplevel;
    Widget _mainWin;
    Widget _saveWidget;
    Widget _findWidget;
    Widget _menuBar;
    Display *_CurDpy; 
    Time _globalTime;
    
    char *_NextString;
    char *_PrevString;
    char *_NextIncrString;
    char *_PrevIncrString;
    char *_OnceString;
    char *_SelectedString;
    char *_AllString;
    
    int _lock;
    int _read_only;
    int _editInPlace;
    int _enableBackups;
    int _exitWithSave;
    int _overrideLocking;
    int _expert;
    char *_backupNameSuffix;
    char *_backupNamePrefix;
    char *_journalNamePrefix;
    char *_journalNameSuffix;
    char *_fileFilter;
    char *_fontFilter;
    char *_geometry;
    int _Width, _Height;
    XmTextSource _source, _rsource, _asource;
    int _changesUntilCompress;
    int _incr_direction;
    
    int _state[st_number];
    
    int _FileMode;
    int _modified;
    char *_loadedfile, *_savedfile;
    int _backedup;
    
    long _clipItemID;
    int _clipDataID;
    
    FILE *_journalFile;
    int _recover;
    char *_jnlName;
    
#ifdef VMS
    struct fileID fibb;
    struct lsb _mylsb;
    struct FAB          fabb;
    struct NAM          namm;
    struct dsc$descriptor_s _foobarDesc;
    char _exp_str[255];
#endif /* VMS */
    
    char _lockname[32];
    Pixmap _NotepadPixmap, _iconifyPixmap;
    
    Widget  _AllButton, _OnceButton, _SelectedButton, _SkipButton;
    Widget _SearchNextButton, _SearchPrevButton, _SearchDismissButton;
    char _geometryString[64];
    
    simpleDialog _saveDialog, 			/* 1 */
    		_undoDialog, 			/* 2 */
    		_seaDialog, 			/* 3 */
    		_seaNextIncrDialog, 		/* 4 */
    		_LineDialog, 			/* 5 */
    		_replaceDialog,			/* 6 */
    		_filterDialog, 			/* 7 */
    		_searchOptionsDialog,		/* 8 */
    		_messageDialog, 		/* 9 */
    		_openDialog,			/* 10 */
    		_fontDialog;			/* 11 */

    int _searchIncrIndex;
    
    /* Notepad supports several "views" of a file using multiple
     * text widgets and the "shared source" feature of the text widget
     */
    View *viewHead;				/* Initial view */
    View *viewTail;				/* Last view in a linked list */
    int viewCount;				/* Number of views */
    View *focusView;			/* pointer to "current" view */
    
    int abortFilterOutput;
    char *_filter;
    MrmHierarchy _DRM_hierarchy;
    
    /* Widget id's for all our dialog boxes */
    Widget _QuitWarnBox, _RecoverWarnBox,  _LoadWarnBox, _FilterWarnBox;
    Widget _noJournalAccessMessage, _badFilename, _nothingToSave, _noWriteMessage;
    Widget _noBackupMessage, _noTempMessage, _writeErrorMessage, _readOnlyMessage;
    Widget _reopenMessage, _noAccessMessage, _noFilenameMessage;
    Widget _fileNotFoundMessage;
    
    Widget FontFamilyList, FontSizeList, FontMiscList;
    
    /* Strings fetch from the UID file */
    char *_untitledString, *_modifiedString, *_notepadString, *_readonlyString;
    
    
} notepadStuff;
    
/* Define this struct in notepad.c (storage allocated there ) and then 
 * it's declared extern for all other modules
 */
#if !defined(NOTEPAD_MAIN)
extern notepadStuff Stuff;
#endif

#define globalTime         Stuff._globalTime
#define DRM_hierarchy	   Stuff._DRM_hierarchy

/* global Widgets  */
#define filter			Stuff._filter
#define workArea                Stuff._workArea
#define workAreaPane                Stuff._workAreaPane
#define helpWidget                Stuff._helpWidget 
#define toplevel                Stuff._toplevel 
#define mainWin                Stuff._mainWin
#define saveWidget                Stuff._saveWidget 
#define textwindow                Stuff.focusView->widget
#define findWidget                Stuff._findWidget
#define menuBar                Stuff._menuBar
#define AllButton              Stuff._AllButton
#define OnceButton            Stuff._OnceButton
#define SelectedButton            Stuff._SelectedButton 
#define SkipButton                Stuff._SkipButton 
#define SearchNextButton         Stuff._SearchNextButton
#define SearchPrevButton            Stuff._SearchPrevButton
#define SearchDismissButton              Stuff._SearchDismissButton   
#define CurDpy                 Stuff._CurDpy  

#define lock            Stuff._lock 
#define read_only            Stuff._read_only 
#define editInPlace            Stuff._editInPlace
#define enableBackups            Stuff._enableBackups
#define exitWithSave            Stuff._exitWithSave
#define overrideLocking            Stuff._overrideLocking 
#define expert            Stuff._expert 
#define Width            Stuff._Width 
#define Height            Stuff._Height
#define changesUntilCompress            Stuff._changesUntilCompress
#define incr_direction            Stuff._incr_direction
#define state            Stuff._state
#define FileMode           Stuff._FileMode
#define modified            Stuff._modified 
#define backedup            Stuff._backedup 
#define clipItemID            Stuff._clipItemID 
#define clipDataID            Stuff._clipDataID 
#define recover            Stuff._recover 
#define searchIncrIndex            Stuff._searchIncrIndex 
#define loadedfile            Stuff._loadedfile 
#define savedfile            Stuff._savedfile 
#define NextString            Stuff._NextString
#define PrevString            Stuff._PrevString
#define NextIncrString            Stuff._NextIncrString 
#define PrevIncrString            Stuff._PrevIncrString
#define OnceString            Stuff._OnceString 
#define SelectedString            Stuff._SelectedString 
#define AllString            Stuff._AllString 
#define backupNameSuffix            Stuff._backupNameSuffix 
#define backupNamePrefix            Stuff._backupNamePrefix 
#define journalNamePrefix            Stuff._journalNamePrefix
#define journalNameSuffix            Stuff._journalNameSuffix 
#define fileFilter            Stuff._fileFilter
#define fontFilter            Stuff._fontFilter
#define geometry            Stuff._geometry 
#define exp_str            Stuff._exp_str 
#define lockname            Stuff._lockname
#define geometryString            Stuff._geometryString
#define jnlName            Stuff._jnlName 
#define Psource            Stuff._source 
#define rsource            Stuff._rsource  
#define asource            Stuff._asource 
#define  journalFile            Stuff._journalFile  

#define  NotepadPixmap          Stuff._NotepadPixmap 
#define  iconifyPixmap            Stuff._iconifyPixmap

#define seaDialog               Stuff._seaDialog 
#define seaNextIncrDialog           Stuff._seaNextIncrDialog 
#define LineDialog           Stuff._LineDialog   
#define replaceDialog          Stuff._replaceDialog  
#define messageDialog           Stuff._messageDialog
#define undoDialog           Stuff._undoDialog
#define openDialog           Stuff._openDialog 
#define saveDialog           Stuff._saveDialog
#define searchOptionsDialog          Stuff._searchOptionsDialog 
#define fontDialog			Stuff._fontDialog
#define filterDialog			Stuff._filterDialog
#define modifiedString	Stuff._modifiedString
#define untitledString	Stuff._untitledString
#define notepadString	Stuff._notepadString
#define readonlyString	Stuff._readonlyString


#ifdef VMS
#define a_fib            Stuff.fibb 
#define mylsb            Stuff._mylsb  
#define a_fab            Stuff.fabb   
#define a_nam            Stuff.namm        
#define foobarDesc            Stuff._foobarDesc 
#endif /* VMS */
