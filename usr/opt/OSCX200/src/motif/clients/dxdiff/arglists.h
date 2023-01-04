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
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/clients/dxdiff/arglists.h,v 1.1.2.2 92/08/03 09:47:25 Dave_Hill Exp $ */
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
 *	arglists.h - include file for widget instance data
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 26th April 1988
 *
 *
 *	Description
 *	-----------
 *
 *
 *	Modification History
 *	------------ -------
 *	
 *	27th Aug 1988	Laurence P. G. Cable
 *
 *	Modify TextArgList to remove insert cursor
 */

#ifndef	ARGLISTS_H
#define	ARGLISTS_H


#define	PointerToArg(p)	(ArgList)(((unsigned long)&(p)) - XtOffset(Arg *, value))

#define	NumberOfArgsBetween(start, finish)	\
	(int)(((Arg *)(finish) - (Arg *)(start)) + 1)

#define	NumberOfArgsInArgListStruct(struct)	\
	(int)(sizeof (struct) / sizeof (Arg))

/********************************
 *
 *     CoreArgList
 *
 ********************************/

typedef	struct _corearglist {
	Arg	x,			/* position */
		y,
		width,			/* dimensions */
		height,	
		borderwidth,		/* borderwidth */
		destroycallback;
} CoreArgList, *CoreArgListPtr;


#define	InitCoreArgListPtr(p)					\
		(p)->x.name = XmNx;				\
		(p)->y.name = XmNy;				\
		(p)->width.name = XmNwidth;			\
		(p)->height.name = XmNheight;			\
		(p)->borderwidth.name = XmNborderWidth,		\
		(p)->destroycallback.name = XmNdestroyCallback

#define	InitCoreArgList(p)		InitCoreArgListPtr(&(p))

#define	CorePtrX(p)			((p)->x.value)
#define	CoreX(p)			CorePtrX(&(p))


#define	CorePtrY(p)			((p)->y.value)
#define	CoreY(p)			CorePtrY(&(p))


#define	CorePtrWidth(p)			((p)->width.value)
#define	CoreWidth(p)			CorePtrWidth(&(p))


#define	CorePtrHeight(p)		((p)->height.value)
#define	CoreHeight(p)			CorePtrHeight(&(p))


#define	CorePtrBorderWidth(p)		((p)->borderwidth.value)
#define	CoreBorderWidth(p)		CorePtrBorderWidth(&(p))

#define	CorePtrDestroyCallBack(p)	((p)->destroycallback.value)
#define	CoreDestroyCallBack(p)		CorePtrDestroyCallBack(&(p))


#define	StaticInitCoreX(x)			{ XmNx, (x) }
#define	StaticInitCoreY(y)			{ XmNy, (y) }
#define	StaticInitCoreWidth(w)			{ XmNwidth, (w) }
#define	StaticInitCoreHeight(h)			{ XmNheight, (h) }
#define	StaticInitCoreBorderWidth(bw)		{ XmNborderWidth, (bw) }
#define	StaticInitCoreDestroyCallBack(dcb)	{ XmNdestroyCallback, (dcb) }

#define	StaticInitCoreArgList(x, y, w, h, bw, dcb)	\
	{						\
		StaticInitCoreX(x),			\
		StaticInitCoreY(y),			\
		StaticInitCoreWidth(w),			\
		StaticInitCoreHeight(h),		\
		StaticInitCoreBorderWidth(bw),		\
		StaticInitCoreDestroyCallBack(dcb)	\
	}



/********************************
 *
 *     ADBConstraintArgList
 *
 ********************************/


typedef	struct	_adbconstraintarglist {
	Arg	topattachment,		/* attachments */
		bottomattachment,
		leftattachment,
		rightattachment,
		topwidget,		/* widgets */
		bottomwidget,
		leftwidget,
		rightwidget,
		topoffset,		/* offsets */
		bottomoffset,
		leftoffset,
		rightoffset;
} ADBConstraintArgList, *ADBConstraintArgListPtr;

#define	InitADBConstraintArgListPtr(p)					\
		(p)->topattachment.name = XmNtopAttachment;		\
		(p)->bottomattachment.name = XmNbottomAttachment;	\
		(p)->leftattachment.name = XmNleftAttachment;	\
		(p)->rightattachment.name = XmNrightAttachment;	\
		(p)->topwidget.name = XmNtopWidget;			\
		(p)->bottomwidget.name = XmNbottomWidget;		\
		(p)->leftwidget.name = XmNleftWidget;		\
		(p)->rightwidget.name = XmNrightWidget;		\
		(p)->topoffset.name = XmNtopOffset;			\
		(p)->bottomoffset.name = XmNbottomOffset;		\
		(p)->leftoffset.name = XmNleftOffset;		\
		(p)->rightoffset.name = XmNrightOffset

#define	InitADBConstraintArgList(p)	InitADBConstraintArgListPtr(&(p))

	

#define	ADBConstraintPtrTopAttachment(p)	((p)->topattachment.value)
#define	ADBConstraintTopAttachment(p)		\
	ADBConstraintPtrTopAttachment(&(p))

#define	ADBConstraintPtrBottomAttachment(p)	((p)->bottomattachment.value)
#define	ADBConstraintBottomAttachment(p)	\
	ADBConstraintPtrBottomAttachment(&(p))

#define	ADBConstraintPtrLeftAttachment(p)	((p)->leftattachment.value)
#define	ADBConstraintLeftAttachment(p)		\
	ADBConstraintPtrLeftAttachment(&(p))

#define	ADBConstraintPtrRightAttachment(p)	((p)->rightattachment.value)
#define	ADBConstraintRightAttachment(p)		\
	ADBConstraintPtrRightAttachment(&(p))

#define	ADBConstraintPtrTopWidget(p)		((p)->topwidget.value)
#define	ADBConstraintTopWidget(p)		\
	ADBConstraintPtrTopWidget(&(p))

#define	ADBConstraintPtrBottomWidget(p)		((p)->bottomwidget.value)
#define	ADBConstraintBottomWidget(p)		\
	ADBConstraintPtrBottomWidget(&(p))

#define	ADBConstraintPtrLeftWidget(p)		((p)->leftwidget.value)
#define	ADBConstraintLeftWidget(p)		\
	ADBConstraintPtrLeftWidget(&(p))

#define	ADBConstraintPtrRightWidget(p)		((p)->rightwidget.value)
#define	ADBConstraintRightWidget(p)		\
	ADBConstraintPtrRightWidget(&(p))

#define	ADBConstraintPtrTopOffset(p)		((p)->topoffset.value)
#define	ADBConstraintTopOffset(p)		\
	ADBConstraintPtrTopOffset(&(p))

#define	ADBConstraintPtrBottomOffset(p)		((p)->bottomoffset.value)
#define	ADBConstraintBottomOffset(p)		\
	ADBConstraintPtrBottomOffset(&(p))

#define	ADBConstraintPtrLeftOffset(p)		((p)->leftoffset.value)
#define	ADBConstraintLeftOffset(p)		\
	ADBConstraintPtrLeftOffset(&(p))

#define	ADBConstraintPtrRightOffset(p)		((p)->rightoffset.value)
#define	ADBConstraintRightOffset(p)		\
	ADBConstraintPtrRightOffset(&(p))


#define	StaticInitADBConstraintTopAttachment(ta)	\
		{ XmNtopAttachment, (ta) }

#define	StaticInitADBConstraintBottomAttachment(ba)	\
		{ XmNbottomAttachment, (ba) }

#define	StaticInitADBConstraintLeftAttachment(la)	\
		{ XmNleftAttachment, (la) }

#define	StaticInitADBConstraintRightAttachment(ra)	\
		{ XmNrightAttachment, (ra) }

#define	StaticInitADBConstraintTopWidget(tw)		\
		{ XmNtopWidget, (tw) }

#define	StaticInitADBConstraintBottomWidget(bw)		\
		{ XmNbottomWidget, (bw) }

#define	StaticInitADBConstraintLeftWidget(lw)		\
		{ XmNleftWidget, (lw) }

#define	StaticInitADBConstraintRightWidget(rw)		\
		{ XmNrightWidget, (rw) }

#define	StaticInitADBConstraintTopOffset(to)		\
		{ XmNtopOffset, (to) }

#define	StaticInitADBConstraintBottomOffset(bo)		\
		{ XmNbottomOffset, (bo) }

#define	StaticInitADBConstraintLeftOffset(lo)		\
		{ XmNleftOffset, (lo) }

#define	StaticInitADBConstraintRightOffset(ro)		\
		{ XmNrightOffset, (ro) }

#define	StaticInitADBConstraintArgList(ta, ba, la, ra, tw, bw, lw, rw, to, bo, lo, ro) 	\
	{							\
		StaticInitADBConstraintTopAttachment(ta),	\
		StaticInitADBConstraintBottomAttachment(ba),	\
		StaticInitADBConstraintLeftAttachment(la),	\
		StaticInitADBConstraintRightAttachment(ra),	\
		StaticInitADBConstraintTopWidget(tw),		\
		StaticInitADBConstraintBottomWidget(bw),	\
		StaticInitADBConstraintLeftWidget(lw),		\
		StaticInitADBConstraintRightWidget(rw),		\
		StaticInitADBConstraintTopOffset(to),		\
		StaticInitADBConstraintBottomOffset(bo),	\
		StaticInitADBConstraintLeftOffset(lo),		\
		StaticInitADBConstraintRightOffset(ro)		\
	}

/********************************
 *
 *     LabelArgList
 *
 ********************************/


typedef	struct	_labelarglist	{
	Arg	labeltype,
		label,
		marginwidth,
		marginheight,
		alignment,
		marginleft,
		marginright,
		margintop,
		marginbottom,
		conformtotext,
		shadow;
} LabelArgList, *LabelArgListPtr;

#define	InitLabelArgListPtr(p)					\
		(p)->labeltype.name = XmNlabelType;		\
		(p)->label.name = XmNlabelString;		\
		(p)->marginwidth.name = XmNmarginWidth;		\
		(p)->marginheight.name = XmNmarginHeight;	\
		(p)->alignment.name = XmNalignment;		\
		(p)->marginleft.name = XmNmarginLeft;		\
		(p)->marginright.name = XmNmarginRight;		\
		(p)->margintop.name = XmNmarginTop;		\
		(p)->marginbottom.name = XmNmarginBottom;	\
		(p)->conformtotext.name = XmNrecomputeSize;

#define	InitLabelArgList(p)		InitLabelArgListPtr(&(p))
		
#define	LabelPtrLabelType(p)		((p)->labeltype.value)
#define	LabelLabelType(p)		LabelPtrLabelType(&(p))
		
#define	LabelPtrLabel(p)		((p)->label.value)
#define	LabelLabel(p)			LabelPtrLabel(&(p))
		
#define	LabelPtrMarginWidth(p)		((p)->marginwidth.value)
#define	LabelMarginWidth(p)		LabelPtrMarginWidth(&(p))
		
#define	LabelPtrMarginHeight(p)		((p)->marginheight.value)
#define	LabelMarginHeight(p)		LabelPtrMarginHeight(&(p))
		
#define	LabelPtrAlignment(p)		((p)->alignment.value)
#define	LabelAlignment(p)		LabelPtrAlignment(&(p))
		
#define	LabelPtrMarginLeft(p)		((p)->marginleft.value)
#define	LabelMarginLeft(p)		LabelPtrMarginLeft(&(p))
		
#define	LabelPtrMarginRight(p)		((p)->marginright.value)
#define	LabelMarginRight(p)		LabelPtrMarginRight(&(p))
		
#define	LabelPtrMarginTop(p)		((p)->margintop.value)
#define	LabelMarginTop(p)		LabelPtrMarginTop(&(p))
		
#define	LabelPtrMarginBottom(p)		((p)->marginbottom.value)
#define	LabelMarginBottom(p)		LabelPtrMarginBottom(&(p))
		
#define	LabelPtrConformToText(p)	((p)->conformtotext.value)
#define	LabelConformToText(p)		LabelPtrConformToText(&(p))
		
#define	LabelPtrShadow(p)		((p)->shadow.value)
#define	LabelShadow(p)			LabelPtrShadow(&(p))


#define	StaticInitLabelType(tx)			\
	{ XmNlabelType, (tx) }

#define	StaticInitLabelLabel(lb)		\
	{ XmNlabelString, (lb) }

#define	StaticInitLabelMarginWidth(mw)		\
	{ XmNmarginWidth, (mw) }

#define	StaticInitLabelMarginHeight(mh)		\
	{ XmNmarginHeight, (mh) }

#define	StaticInitLabelAlignment(al)		\
	{ XmNalignment, (al) }

#define	StaticInitLabelMarginLeft(ml)		\
	{ XmNmarginLeft, (ml) }

#define	StaticInitLabelMarginRight(mr)		\
	{ XmNmarginRight, (mr) }

#define	StaticInitLabelMarginTop(mt)		\
	{ XmNmarginTop, (mt) }

#define	StaticInitLabelMarginBottom(mb)		\
	{ XmNmarginBottom, (mb) }

#define	StaticInitLabelMarginConfromToText(ctt)	\
	{ XmNrecomputeSize, (ctt) }

#define StaticInitLabelShadow(sh)		\
	{ XmNshowAsDefault, (sh) }

#define	StaticInitLabelArgList(tx,lb,mw,mh,al,ml,mr,mt,mb,ctt,sh)	\
	{						\
		StaticInitLabelType(tx),		\
		StaticInitLabelLabel(lb),		\
		StaticInitLabelMarginWidth(mw),		\
		StaticInitLabelMarginHeight(mh),	\
		StaticInitLabelAlignment(al),		\
		StaticInitLabelMarginLeft(ml),		\
		StaticInitLabelMarginRight(mr),		\
		StaticInitLabelMarginTop(mt),		\
		StaticInitLabelMarginBottom(mb),	\
		StaticInitLabelMarginConfromToText(ctt),\
		StaticInitLabelShadow(sh)		\
	}

/********************************
 *
 *     ScrollArgList
 *
 ********************************/

typedef	struct	_textarglist {
	Arg	marginwidth,
		marginheight,
		units,
		resizeheight,
		resizewidth,
		displaypolicy,
		scrollpolicy,
		visualpolicy,
		editmode,
		vsb,
		hsb,
		cols,
		rows,
		wordwrap,
		value,
		editable,
		autoshowinsert,
		insertvisible,
		scrollleftside,
		highlightthickness;
} TextArgList, *TextArgListPtr;

#define	InitTextArgListPtr(p)					\
	(p)->marginwidth.name = XmNmarginWidth;			\
	(p)->marginheight.name = XmNmarginHeight;		\
	(p)->units.name = XmNunitType;				\
	(p)->resizeheight.name = XmNresizeHeight;		\
	(p)->resizewidth.name = XmNresizeWidth;			\
	(p)->displaypolicy.name = XmNscrollBarDisplayPolicy,	\
	(p)->scrollpolicy.name = XmNscrollingPolicy,		\
	(p)->visualpolicy.name = XmNvisualPolicy,		\
	(p)->editmode = XmNeditMode,				\
	(p)->vsb.name = XmNscrollVertical,			\
	(p)->hsb.name = XmNscrollHorizontal,			\
	(p)->cols.name = XmNcolumns;				\
	(p)->rows.name = XmNrows;				\
	(p)->wordwrap.name = XmNwordWrap;			\
	(p)->value.name = XmNvalue,				\
	(p)->editable.name = XmNeditable,			\
	(p)->autoshowinsert.name = XmNautoShowCursorPosition,	\
	(p)->insertvisible.name = XmNcursorPositionVisible,	\
	(p)->scrollleftside.name = XmNscrollLeftSide,		\
	(p)->highlightthickness.name = XmNhighlightThickness;


#define	InitTextArgList(p)		InitTextArgListPtr(&(p))

#define	TextPtrMarginWidth(p)		((p)->marginwidth.value)
#define	TextMarginWidth(p)		TextPtrMarginWidth(&(p))

#define	TextPtrMarginHeight(p)		((p)->marginheight.value)
#define	TextMarginHeight(p)		TextPtrMarginHeight(&(p))

#define	TextPtrUnits(p)			((p)->units.value)
#define	TextUnits(p)			TextPtrUnits(&(p))

#define	TextPtrResizeHeight(p)		((p)->resizeheight.value)
#define	TextResizeHeight(p)		TextPtrResizeHeight(&(p))

#define	TextPtrResizeWidth(p)		((p)->resizewidth.value)
#define	TextResizeWidth(p)		TextPtrResizeWidth(&(p))

#define TextPtrDisplayPolicy(p)		((p)->displaypolicy.value)
#define TextDisplayPolicy(p)		TextPtrDisplayPolicy(&(p))

#define TextPtrScrollPolicy(p)		((p)->scrollpolicy.value)
#define TextScrollPolicy(p)		TextPtrScrollPolicy(&(p))

#define TextPtrVisualPolicy(p)		((p)->visualpolicy.value)
#define TextVisualPolicy(p)		TextPtrVisualPolicy(&(p))

#define TextPtrEditMode(p)		((p)->editmode.value)
#define TextEditMode(p)			TextPtrEditMode(&(p))

#define TextPtrVerticalScroll(p)	((p)->vsb.value)
#define TextVerticalScroll(p)		TextPtrVerticalScroll(&(p))

#define TextPtrHorizontalScroll(p)	((p)->hsb.value)
#define TextHorizontalScroll(p)		TextPtrHorizontalScroll(&(p))

#define	TextPtrCols(p)			((p)->cols.value)
#define	TextCols(p)			TextPtrCols(&(p))
                                                
#define	TextPtrRows(p)			((p)->rows.value)
#define	TextRows(p)			TextPtrRows(&(p))

#define	TextPtrWordWrap(p)		((p)->wordwrap.value)
#define	TextWordWrap(p)			TextPtrWordWrap(&(p))

#define	TextPtrValue(p)			((p)->value.value)
#define	TextValue(p)			TextPtrValue(&(p))

#define	TextPtrEditable(p)		((p)->editable.value)
#define	TextEditable(p)			TextPtrEditable(&(p))

#define	TextPtrAutoShowInsert(p)	((p)->autoshowinsert.value)
#define	TextAutoShowInsert(p)		TextPtrAutoShowInsert(&(p))

#define	TextPtrInsertVisible(p)		((p)->insertvisible.value)
#define	TextInsertVisible(p)		TextPtrInsertVisible(&(p))

#define	TextPtrScrollLeftSide(p)	((p)->scrollleftside.value)
#define	TextScrollLeftSide(p)		TextPtrScrollLeftSide(&(p))

#define	TextPtrHighlightThickness(p)	((p)->highlightthickness.value)
#define	TextHighlightThickness(p)	TextPtrHighlightThickness(&(p))

#define	StaticInitTextMarginWidth(mw)		\
	{ XmNmarginWidth, (mw) }

#define	StaticInitTextMarginHeight(mh)	\
	{ XmNmarginHeight, (mh) }

#define	StaticInitTextUnits(un)		\
	{ XmNunitType, (un) }

#define	StaticInitTextResizeWidth(rw)		\
	{ XmNresizeWidth, (rw) }

#define	StaticInitTextResizeHeight(rh)	\
	{ XmNresizeHeight, (rh) }

#define StaticInitTextDisplayPolicy(dp)	\
	{ XmNscrollBarDisplayPolicy, (dp) }

#define StaticInitTextScrollPolicy(sp)	\
	{ XmNscrollingPolicy, (sp) }

#define StaticInitTextVisualPolicy(vp)	\
	{ XmNvisualPolicy, (vp) }

#define StaticInitTextEditMode(em)	\
	{XmNeditMode, (em) }

#define StaticInitTextVerticalScroll(vs) 	\
	{ XmNscrollVertical, (vs) }

#define StaticInitTextHorizontalScroll(hs) 	\
	{ XmNscrollHorizontal, (hs) }

#define	StaticInitTextCols(co)		\
	{ XmNcolumns, (co) }

#define	StaticInitTextRows(ro)		\
	{ XmNrows, (ro) }

#define	StaticInitTextWordWrap(ww)	\
	{ XmNwordWrap, (ww) }

#define	StaticInitTextValue(v)		\
	{ XmNvalue, (v) }

#define	StaticInitTextEditable(ed)	\
	{ XmNeditable, (ed) }

#define	StaticInitTextAutoShowInsert(asip)	\
	{ XmNautoShowCursorPosition, (asip) }

#define	StaticInitTextInsertVisible(iv)	\
	{ XmNcursorPositionVisible, (iv) }

#define	StaticInitTextScrollLeftSide(sls)	\
	{ XmNscrollLeftSide, (sls) }

#define	StaticInitHighlightThickness(ht)	\
	{ XmNhighlightThickness, (ht) }

#define	StaticInitTextArgList(mw,mh,un,rw,rh,dp,sp,vp,em,vs,hs,co,ro,ww,v,ed,asip,iv,sls,ht) \
	{					\
		StaticInitTextMarginWidth(mw),		\
		StaticInitTextMarginHeight(mh),		\
		StaticInitTextUnits(un),		\
		StaticInitTextResizeWidth(rw),		\
		StaticInitTextResizeHeight(rh),		\
		StaticInitTextDisplayPolicy(dp),	\
		StaticInitTextScrollPolicy(sp),		\
		StaticInitTextVisualPolicy(vp),		\
		StaticInitTextEditMode(em),		\
		StaticInitTextVerticalScroll(vs), 	\
		StaticInitTextHorizontalScroll(hs), 	\
		StaticInitTextCols(co),			\
		StaticInitTextRows(ro),			\
		StaticInitTextWordWrap(ww),		\
		StaticInitTextValue(v),			\
		StaticInitTextEditable(ed),		\
		StaticInitTextAutoShowInsert(asip),	\
		StaticInitTextInsertVisible(iv), 	\
		StaticInitTextScrollLeftSide(sls),	\
		StaticInitHighlightThickness(ht)	\
	}

/********************************
 *
 *     FontArgList
 *
 ********************************/

typedef	struct	_fontarglist {
	Arg	fontx,
		fonty,
		font;
} FontArgList, *FontArgListPtr;

#define InitFontArgListPtr(p)			\
		(p)->fontx.name = "fontX";	\
		(p)->fonty.name = "fonty";	\
		(p)->font.name = XmNfontList


#define	InitFontArgList(p)		InitFontArgListPtr(&(p))
	
#define FontPtrFontX(p)		((p)->fontx.value)
#define	FontFontX(p)		FontPtrFontX(p)
	
#define FontPtrFontY(p)		((p)->fonty.value)
#define	FontFontY(p)		FontPtrFontY(p)
	
#define FontPtrFont(p)		((p)->font.value)
#define	FontFont(p)		FontPtrFont(p)

#define	StaticInitFontArgListfontX(fx)		{ "fontX", (fx) }
#define	StaticInitFontArgListfontY(fy)		{ "fontY", (fy) }
#define	StaticInitFontArgListfont(fn)		{ XmNfontList, (fn) }

#define	StaticInitFontArgList(fx, fy, fn)	\
	{					\
		StaticInitFontArgListfontX(fx),	\
		StaticInitFontArgListfontY(fy),	\
		StaticInitFontArgListfont(fn)	\
	}

/********************************
 *
 *     DialogBoxArgList
 *
 ********************************/

typedef	struct	_dialogboxarglist {
	Arg	units,
		titletype,
		title,
		style,
		resize,
		childoverlap,
		marginwidth,
		marginheight;
} DialogBoxArgList, *DialogBoxArgListPtr;

#define	InitDialogBoxArgListPtr(p)				\
		(p)->units.name = XmNunitType;			\
		(p)->titletype.name = "titleType";		\
		(p)->title.name = XmNtitle;			\
		(p)->style.name = XmNdialogStyle;		\
		(p)->resize.name = XmNresize;			\
		(p)->childoverlap.name = XmNallowOverlap;	\
		(p)->marginwidth.name = XmNmarginWidth;		\
		(p)->marginheight.name = XmNmarginHeight

#define	InitDialogBoxArgList(p)			InitDialogBoxArgListPtr(&(p))
		
#define	DialogBoxPtrUnits(p)		((p)->units.value)
#define	DialogBoxUnits(p)		DialogBoxPtrUnits(&(p))
		
#define	DialogBoxPtrTitleType(p)	((p)->titletype.value)
#define	DialogBoxTitleType(p)		DialogBoxPtrTitleType(&(p))
		
#define	DialogBoxPtrTitle(p)		((p)->title.value)
#define	DialogBoxTitle(p)		DialogBoxPtrTitle(&(p))
		
#define	DialogBoxPtrStyle(p)		((p)->style.value)
#define	DialogBoxStyle(p)		DialogBoxPtrStyle(&(p))
		
#define	DialogBoxPtrResize(p)		((p)->resize.value)
#define	DialogBoxResize(p)		DialogBoxPtrResize(&(p))
		
#define	DialogBoxPtrChildOverlap(p)	((p)->childoverlap.value)
#define	DialogBoxChildOverlap(p)	DialogBoxPtrChildOverlap(&(p))
		
#define	DialogBoxPtrMarginWidth(p)	((p)->marginwidth.value)
#define	DialogBoxMarginWidth(p)		DialogBoxPtrMarginWidth(&(p))
		
#define	DialogBoxPtrMarginHeight(p)	((p)->marginheight.value)
#define	DialogBoxMarginHeight(p)	DialogBoxPtrMarginHeight(&(p))


#define	StaticInitDialogBoxUnits(un)		{ XmNunitType, (un) }
#define	StaticInitDialogBoxTitleType(tt)	{ "titleType", (tt) }
#define	StaticInitDialogBoxTitle(tt)		{ XmNtitle, (tt) }
#define	StaticInitDialogBoxStyle(t)		{ XmNdialogStyle, (t) }
#define	StaticInitDialogBoxResize(re)		{ XmNresize, (re) }
#define	StaticInitDialogBoxChildOverlap(co)	{ XmNallowOverlap, (co) }
#define	StaticInitDialogBoxMarginWidth(mw)	{ XmNmarginWidth, (mw) }
#define	StaticInitDialogBoxMarginHeight(mh)	{ XmNmarginHeight, (mh) }

#define	StaticInitDialogBoxArgList(un,tt,t,st,re,co,mw,mh)	\
	{						\
		StaticInitDialogBoxUnits(un),		\
		StaticInitDialogBoxTitleType(tt),	\
		StaticInitDialogBoxTitle(t),		\
		StaticInitDialogBoxStyle(st),		\
		StaticInitDialogBoxResize(re),		\
		StaticInitDialogBoxChildOverlap(co),	\
		StaticInitDialogBoxMarginWidth(mw),	\
		StaticInitDialogBoxMarginHeight(mh)	\
	}

/********************************
 *
 *     MenuBarArgList
 *
 ********************************/

typedef	struct	_menubararglist	{
	Arg 	orientation,
		type;
} MenuBarArgList, *MenuBarArgListPtr;

#define	InitMenuBarArgListPtr(p)	\
				(p)->orientation.name = XmNorientation; \
				(p)->type.name = XmNrowColumnType

#define	InitMenuBarArgList(p)	InitMenuBarArgListPtr(&(p))

#define	MenuBarPtrOrientation(p)	((p)->orientation.value)
#define	MenuBarOrientation(p)		MenuBarPtrOrientation(&(p))

#define	MenuBarPtrType(p)		((p)->type.value)
#define	MenuBarType(p)			MenuBarPtrType(&(p))

#define	StaticInitMenuBarOrientation(or)	{ XmNorientation, (or) }
#define	StaticInitMenuBarType(ty)		{ XmNrowColumnType, (ty) }

#define	StaticInitMenuBarArgList(or,ty)			\
	{						\
		StaticInitMenuBarOrientation(or),	\
		StaticInitMenuBarType(ty)		\
	}
#endif	ARGLISTS_H
