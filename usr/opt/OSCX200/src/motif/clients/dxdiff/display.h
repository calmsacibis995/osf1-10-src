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
/* BuildSystemHeader added automatically */
/* $Header: /usr/sde/x11/rcs/x11/src/./motif/clients/dxdiff/display.h,v 1.1 90/01/01 00:00:00 devrcs Exp $ */
/*
 * Copyright 1988 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
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
 *
 *	dxdiff
 *
 *	display.h - display handler include file
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 27th April 1988
 *
 *
 *	Description
 *	-----------
 *
 *
 *	Modification History
 *	------------ -------
 *	
 *	14th June 1988	Laurence P. G. Cable
 *
 *			added parent field to MainADB
 *
 *	23rd June 1988	Laurence P. G. Cable
 *
 *			added fileselection stuff
 */

#ifndef	DISPLAY_H
#define	DISPLAY_H



/********************************
 *
 *     MainADB
 *
 ********************************/

typedef	struct	_mainadb {
	CoreArgList		core;
	DialogBoxArgList	adb;
	ADBConstraintArgList	constraints;

	Widget		w,
			parent;
} MainADB, *MainADBPtr;

#define	InitMainADBPtrArgList(p)		\
	InitCoreArgList((p)->core);		\
	InitDialogBoxArgList((p)->adb)

#define	InitMainADBArgList(p)		InitMainADBPtrArgLIst(&(p))

#define	MainADBPtrCoreArgList(p)	((p)->core)
#define	MainADBCoreArgList(p)		MainADBPtrCoreArgList(&(p))

#define	MainADBPtrDialogBoxArgList(p)	((p)->adb)
#define	MainADBDialogBoxArgList(p)	MainADBPtrDialogBoxArgList(&(p))

#define	MainADBPtrConstraintArgList(p)	((p)->constraints)
#define	MainADBConstraintArgList(p)	MainADBPtrConstraintArgList(&(p))

#define	MainADBPtrWidget(p)		((p)->w)
#define MainADBWidget(p)		MainADBPtrWidget(&(p))

#define	MainADBPtrParent(p)		((p)->parent)
#define MainADBParent(p)		MainADBPtrParent(&(p))



/********************************
 *
 *     HVScrollBar
 *
 ********************************/

typedef	struct	_hvscrollbar	{
	Widget			w;
	Boolean			dragging;
} HVScrollBar, *HVScrollBarPtr;


#define	HVScrollBarPtrWidget(p)		((p)->w)
#define	HVScrollBarWidget(p)		HVScrollBarPtrWidget(&(p))

#define	HVScrollBarPtrDragging(p)	((p)->dragging)
#define	HVScrollBarDragging(p)		HVScrollBarPtrWidget(&(p))


/********************************
 *
 *     FileName
 *
 ********************************/

typedef	struct	_filename {
	CoreArgList		core;
	ADBConstraintArgList	constraints;
	LabelArgList		label;
	Arg			sensitivity;
	FontArgList		font;		/* not used */

	Widget			w;
	char			*file;
	XmFontList		fontlist;
} FileName, *FileNamePtr;


#define	InitFileNamePtrCoreArgList(p)				\
		InitCoreArgList((p)->core);			\
		InitADBConstraintArgList((p)->constraints;	\
		InitLabelArgList((p)->label);			\
		(p)->sensitivity.name = XmNsensitive;		\
		InitFontArgList((p)->font

#define	InitFileNameArgList(p)		InitFileNamePtrArgList(&(p))

#define	FileNamePtrCoreArgList(p)		((p)->core)
#define	FileNameCoreArgList(p)		FileNamePtrCoreArgList(&(p))

#define	FileNamePtrConstraintArgList(p)	((p)->constraints)
#define	FileNameConstraintArgList(p)	FileNamePtrConstraintArgList(&(p))

#define	FileNamePtrLabelArgList(p)		((p)->label)
#define	FileNameLabelArgList(p)		FileNamePtrLabelArgList(&(p))

#define	FileNamePtrSensitivity(p)	((p)->sensitivity.value)
#define	FileNameSensitivity(p)		FileNamePtrSensitivity(&(p))

#define	FileNamePtrFontArgList(p)		((p)->font)
#define	FileNameFontArgList(p)		FileNamePtrFontArgList(&(p))

#define	FileNamePtrWidget(p)		((p)->w)
#define	FileNameWidget(p)			FileNamePtrWidget(&(p))

#define	FileNamePtrFile(p)		((p)->file)
#define	FileNameFile(p)			FileNamePtrFile(&(p))

#define	FileNamePtrFontList(p)	((p)->fontlist)
#define	FileNameFontList(p)		FontListNamePtrFontList(&(p))

#define	StaticInitFileNameSensitivity(s)	\
{ XmNsensitive, (s) }

/********************************
 *
 *     Filler
 *
 ********************************/

typedef	struct	_filler {
	CoreArgList		core;
	ADBConstraintArgList	constraints;

	Widget			w;
} Filler, *FillerPtr;

#define	InitFillerPtrArgList(p)				\
		InitCoreArgList((p)->core);		\
		InitADBConstraintArgList((p)->constraints)

#define	InitFillerArgList(p)	InitFillerPtrArgList(&(p))

#define	FillerPtrCoreArgList(p)		((p)->core)
#define	FillerCoreArgList(p)		FillerPtrCoreArgList(&(p))

#define	FillerPtrADBConstraintArgList(p)	((p)->constraints)
#define	FillerADBConstraintArgList(p)		\
					FillerPtrADBConstraintArgList(&(p))

#define	FillerPtrWidget(p)		((p)->w)
#define	FillerWidget(p)			FillerPtrWidget(&(p))


/********************************
 *
 *     ALabel
 *
 ********************************/

typedef	struct	_alabel {
	CoreArgList		core;
	ADBConstraintArgList	constraints;
	LabelArgList		label;

	Widget			w;
} ALabel, *ALabelPtr;

#define	InitAALabelPtrArgList(p)				\
		InitCoreArgList((p)->core);			\
		InitADBConstraintArgList((p)->constraints)	\
		InitLabelArgList((p)->label)

#define	InitALabelArgList(p)	InitALabelPtrArgList(&(p))

#define	ALabelPtrCoreArgList(p)		((p)->core)
#define	ALabelCoreArgList(p)		ALabelPtrCoreArgList(&(p))

#define	ALabelPtrADBConstraintArgList(p)	((p)->constraints)
#define	ALabelADBConstraintArgList(p)		ALabelPtrADBConstraintArgList(&(p))

#define	ALabelPtrLabelArgList(p)	((p)->label)
#define	ALabelLabelArgList(p)		ALabelPtrLabelArgList(&(p))

#define	ALabelPtrWidget(p)		((p)->w)
#define	ALabelWidget(p)			ALabelPtrWidget(&(p))


/********************************
 *
 *     DiffRegionADB
 *
 ********************************/

typedef	struct	_diffregionadb {
	CoreArgList			core;
	DialogBoxArgList		adb;
	ADBConstraintArgList		constraints;

	Widget				w;
	FillerPtr			topfiller,
					bottomfiller;

	DifferenceBoxPtr		differencebox;
} DiffRegionADB, *DiffRegionADBPtr;

#define	InitDiffRegionADBPtrArgList(p)				\
		InitCoreArgList((p)->core);			\
		InitDialogBoxArgList((p)->adb);			\
		InitADBConstraintArgList((p)->constraints

#define	InitDiffRegionADBArgList(p)	InitDiffRegionADBPtrArgList(&(p))

#define	DiffRegionADBPtrCoreArgList(p)	((p)->core)
#define	DiffRegionADBCoreArgList(p)	DiffRegionADBPtrCoreArgList(&(p))

#define	DiffRegionADBPtrDialogBoxArgList(p)	((p)->adb)
#define	DiffRegionADBDialogBoxArgList(p)	DiffRegionADBPtrDialogBoxArgList(&(p))

#define	DiffRegionADBPtrConstraintArgList(p)	((p)->constraints)
#define	DiffRegionADBConstraintArgList(p)	\
				DiffRegionADBPtrADBConstraintArgList(&(p))

#define	DiffRegionADBPtrWidget(p)		((p)->w)
#define	DiffRegionADBWidget(p)		DiffRegionADBPtrWidget(&(p))

#define	DiffRegionADBPtrTopFiller(p)	((p)->topfiller)
#define	DiffRegionADBTopFiller(p)		DiffRegionADBPtrTopFiller(&(p))

#define	DiffRegionADBPtrBottomFiller(p)	((p)->bottomfiller)
#define	DiffRegionADBBottomFiller(p)	DiffRegionADBPtrBottomFiller(&(p))

#define	DiffRegionADBPtrDifferenceBox(p)	((p)->differencebox)
#define	DiffRegionADBDifferenceBox(p)	DiffRegionADBPtrDifferenceBox(&(p))



/********************************
 *
 *	TextDisplayADB
 *
 ********************************/

typedef	struct	_textdisplayadb	{
	CoreArgList		core;
	DialogBoxArgList	adb;
	ADBConstraintArgList	constraints;

	Widget			w;
	WhichFile		whatfile;
	FileNamePtr		filename;
	TextDisplayPtr		textdisplay;
	HVScrollBarPtr		vscroll,
				hscroll;
	ALabelPtr		label;
	AMenuBarPtr		menubar;
} TextDisplayADB, *TextDisplayADBPtr;

#define	InitTextDisplayADBPtrArgList(p)					\
		InitCoreArgList((p)->core);				\
		InitDialogBoxArgList((p)->adb);				\
		InitADBConstraintsArgList((p)->constraints)

#define	InitTextDisplayADBArgList(p)	InitTextDisplayADBPtrArgList(&(p))

#define	TextDisplayADBPtrCoreArgList(p)	((p)->core)
#define	TextDisplayADBCoreArgList(p)	TextDisplayADBPtrCoreArgList(&(p))

#define	TextDisplayADBPtrDialogBoxArgList(p)	((p)->adb)
#define	TextDisplayADBDialogBoxArgList(p)	\
				TextDisplayADBPtrDialogBoxArgList(&(p))

#define	TextDisplayADBPtrADBConstraintArgList(p)	((p)->constraints)
#define	TextDisplayADBADBConstraintArgList(p)		\
				TextDisplayADBPtrADBConstraintArgList(&(p))

#define	TextDisplayADBPtrWidget(p)		((p)->w)
#define	TextDisplayADBWidget(p)			TextDisplayADBPtrWidget(&(p))

#define	TextDisplayADBPtrWhatFile(p)		((p)->whatfile)
#define	TextDisplayADBWhatFile(p)		TextDisplayADBPtrWhatFile(&(p))

#define	TextDisplayADBPtrFilename(p)		((p)->filename)
#define	TextDisplayADBFilename(p)		TextDisplayADBPtrFilename(&(p))

#define	TextDisplayADBPtrTextDisplay(p)	((p)->textdisplay)
#define	TextDisplayADBTextDisplay(p)	TextDisplayADBPtrTextDisplay(&(p))

#define	TextDisplayADBPtrVScroll(p)	((p)->vscroll)
#define	TextDisplayADBVScroll(p)	TextDisplayADBPtrVScroll(&(p))

#define	TextDisplayADBPtrHScroll(p)	((p)->hscroll)
#define	TextDisplayADBHScroll(p)	TextDisplayADBPtrHScroll(&(p))

#define	TextDisplayADBPtrLabel(p)	((p)->label)
#define	TextDisplayADBLabel(p)		TextDisplayADBPtrLabel(&(p))

#define	TextDisplayADBPtrMenuBar(p)	((p)->menubar)
#define	TextDisplayADBMenuBar(p)	MenuBarADBPtrMenuBar(&(p))





/********************************
 *
 *     FileSelector
 *
 ********************************/

typedef	struct	_fileselector {
	CoreArgList		core;
	DialogBoxArgList	db;
	Arg			autounmanage,
				defaultpositioning,
				dirmaskarg,
				activatecallback,
				cancelcallback,	/* not used currently */
				helpcallback;

	Widget			fileselector;
	XmStringContext		context;
	XmString		value, dirmask;
	char			*file, *dir;
} FileSelector, *FileSelectorPtr;
				
#define	InitFileSelectorPtrArgList(p)					\
		InitCoreArgListPtr(p);					\
		InitDialogBoxArgList(p);				\
		(p)->autounmanage.name = XmNautoUnmanage;		\
		(p)->defaultpositioning.name = XmNdefaultPosition;	\
		(p)->valuearg.name = XmNdirMask;			\
		(p)->cancelcallback.name = XmNcancelCallback;		\
		(p)->activatecallback.name = XmNactivateCallback;	\
		(p)->helpcallback.name = XmNhelpCallback

#define	InitFileSelectorArgList(p)	InitFileSeletectorPtrArgList(&(p))

#define	FileSelectorPtrCoreArgList(p)		((p)->core)
#define	FileSelectorCoreArgList(p)		FileSelectorPtrCoreArgList(&(p))

#define	FileSelectorPtrDialogBoxArgList(p)	((p)->db)
#define	FileSelectorDialogBoxArgList(p)		FileSelectorPtrDialogBoxArgList(&(p))

#define	FileSelectorPtrAutoUnmanage(p)		((p)->autounmanage.value)
#define	FileSelectorAutoUnmanage(p)		FileSelectorPtrAutoUnmanage(&(p))

#define	FileSelectorPtrDefaultPositioning(p)	((p)->defaultpositioning.value)
#define	FileSelectorDefaultPositioning(p)	FileSelectorPtrDefaultPositioning(&(p))

#define	FileSelectorPtrCancelCallBack(p)	((p)->cancelcallback.value)
#define	FileSelectorCancelCallBack(p)		FileSelectorPtrCancelCallBack(&(p))

#define	FileSelectorPtrDirMaskArg(p)		((p)->dirmaskarg.value)
#define	FileSelectorDirMaskArg(p)		FileSelectorPtrDirMaskArg(&(p))

#define	FileSelectorPtrActivateCallBack(p)	((p)->activatecallback.value)
#define	FileSelectorActivateCallBack(p)		FileSelectorPtrActivateCallBack(&(p))

#define	FileSelectorPtrHelpCallBack(p)		((p)->helpcallback.value)
#define	FileSelectorHelpCallBack(p)		FileSelectorPtrHelpCallBack(&(p))

#define	FileSelectorPtrFileSelector(p)		((p)->fileselector)
#define	FileSelectorFileSelector(p)		FileSelectorPtrFileSelector(&(p))

#define	FileSelectorPtrContext(p)		((p)->context)
#define	FileSelectorContext(p)			FileSelectorPtrContext(&(p))

#define	FileSelectorPtrValue(p)			((p)->value)
#define	FileSelectorValue(p)			FileSelectorPtrValue(&(p))

#define	FileSelectorPtrDirMask(p)		((p)->dirmask)
#define	FileSelectorDirMask(p)			FileSelectorPtrDirMask(&(p))

#define	FileSelectorPtrFile(p)			((p)->file)
#define	FileSelectorFile(p)			FileSelectorPtrFile(&(p))

#define	FileSelectorPtrDir(p)			((p)->dir)
#define	FileSelectorDir(p)			FileSelectorPtrDir(&(p))


/********************************
 *
 *     MessageBox
 *
 ********************************/

typedef	struct _messagebox {
	CoreArgList		core;
	DialogBoxArgList	dialog;
	LabelArgList		label;
	Arg			defaultpositioning,
				activatecallback;

	Widget			w;
	caddr_t			closure;	/* as you like it ! */
} MessageBox, *MessageBoxPtr;

#define	InitMessageBoxPtrArgList(p)			\
	InitCoreArgList((p)->core);			\
	InitDialogBoxArgList((p)->dialog);		\
	(p)->activatecallback.name = XmNactivateCallback

#define	MessageBoxPtrCoreArgList(p)		((p)->core)
#define	MessageBoxCoreArgList(p)		MessageBoxPtrCoreArgList(&(p))

#define	MessageBoxPtrDialogBoxArgList(p)	((p)->dialog)
#define	MessageBoxDialogBoxArgList(p)		MessageBoxPtrDialogBoxArgList(&(p))

#define	MessageBoxPtrLabelArgList(p)		((p)->label)
#define	MessageBoxLabelArgList(p)		MessageBoxPtrLabelArgList(&(p))

#define	MessageBoxPtrActivateCallBack(p)	((p)->activatecallback.value)
#define	MessageBoxActivateCallBack(p)		MessageBoxPtrActivateCallBack(&(p))

#define	MessageBoxPtrWidget(p)			((p)->w)
#define	MessageBoxWidget(p)			MessageBoxPtrWidget(&(p))

#define	MessageBoxPtrClosure(p)			((p)->closure)
#define	MessageBoxClosure(p)			MessageBoxPtrClosure(&(p))


/********************************
 *
 *     DxDiffDisplay
 *
 ********************************/

typedef	struct _dxdiffdisplay	{
	MainADBPtr		mainadb,
				displayadb;
	AMenuBarPtr		menubar;
	TextDisplayADBPtr	lefttextadb,
				righttextadb;
	DiffRegionADBPtr	diffregionadb;
	DiffListBlkPtr		difflist;
	int			displayidx;	/* for multi dxdiff displays */
	FileSelectorPtr		lfileselector,
				rfileselector;
	Boolean			horizontalslavescroll;
	WhichFile		horizontalscrolledlast;
} DxDiffDisplay, *DxDiffDisplayPtr;

#define	InitDxDiffDisplayPtrArgList(p)					\
		InitMainADBArgList((p)->mainadb);			\
		InitAMenuBarArgList((p)->menubar);			\
		InitTextDisplayADBArgList((p)->lefttextadb);		\
		InitTextDisplayADBArgList((p)->righttextadb);		\
		InitDiffRegionADBArgLiist((p)->diffregionadb)

#define	InitDxDiffDisplayArgList(p)	InitDxDiffDisplayPtrArgList(&(p))

#define	DxDiffDisplayPtrMainADB(p)	((p)->mainadb)
#define	DxDiffDisplayMainADB(p)		DxDiffDisplayPtrMainADB(&(p))

#define	DxDiffDisplayPtrDisplayADB(p)	((p)->displayadb)
#define	DxDiffDisplayDisplayADB(p)	DxDiffDisplayPtrDisplayADB(&(p))

#define	DxDiffDisplayPtrMenuBar(p)	((p)->menubar)
#define	DxDiffDisplayMenuBar(p)		DxDiffDisplayPtrMenuBar(&(p))

#define	DxDiffDisplayPtrLeftTextADB(p)	((p)->lefttextadb)
#define	DxDiffDisplayLeftTextADB(p)	\
				DxDiffDisplayPtrLeftTextADB(&(p))

#define	DxDiffDisplayPtrRightTextADB(p)	((p)->righttextadb)
#define	DxDiffDisplayRightTextADB(p)		\
					DxDiffDisplayPtrRightTextADB(&(p))

#define	DxDiffDisplayPtrDiffRegionADB(p)	((p)->diffregionadb)
#define	DxDiffDisplayDiffRegionADB(p)		\
					DxDiffDisplayPtrDiffRegionADB(&(p))

#define	DxDiffDisplayPtrDiffList(p)	((p)->difflist)
#define	DxDiffDisplayDiffList(p)		\
					DxDiffDisplayPtrDiffList(&(p))

#define	DxDiffDisplayPtrDisplayIdx(p)	\
					((p)->displayidx)
#define	DxDiffDisplayDisplayIdx(p)	\
					DxDiffDisplayPtrDisplayIdx(&(p))

#define	DxDiffDisplayPtrLFileSelector(p)	\
					((p)->lfileselector)
#define	DxDiffDisplayLFileSelector(p)	\
					DxDiffDisplayPtrLFileSelector(&(p))

#define	DxDiffDisplayPtrRFileSelector(p)	\
					((p)->rfileselector)
#define	DxDiffDisplayRFileSelector(p)	\
					DxDiffDisplayPtrRFileSelector(&(p))
#define	DxDiffDisplayPtrHorizontalSlaveScroll(p)	((p)->horizontalslavescroll)
#define	DxDiffDisplayHorizontalSlaveScroll(p)		DxDiffDisplayPtrHorizontalSlaveScroll(&(p))

#define	DxDiffDisplayPtrHorizontalScrolledLast(p)	((p)->horizontalscrolledlast)
#define	DxDiffDisplayHorizontalScrolledLast(p)		DxDiffDisplayPtrHorizontalScrolledLast(&(p))

#endif	DISPLAY_H
