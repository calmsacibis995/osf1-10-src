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
static char *rcsid = "@(#)$RCSfile: Messages.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/06 15:42:41 $";
#endif
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: Messages.c,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/06 15:42:41 $"
#endif
#endif
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <Xm/XmP.h>

/* Define _XmConst before including MessagesI.h, so that the
 * declarations will be in agreement with the definitions.
 */
#ifndef _XmConst
#define _XmConst XmConst
#endif /* _XmConst */

#include "MessagesI.h"


/**************** BaseClass.c ****************/

_XmConst char _XmMsgBaseClass_0000[] =
   "no context found for extension";

_XmConst char _XmMsgBaseClass_0001[] =
   "_XmPopWidgetExtData: no extension found with XFindContext";

_XmConst char _XmMsgBaseClass_0002[] =
   "XmFreeWidgetExtData is an unsupported routine";


/**************** BulletinB.c ****************/

_XmConst char _XmMsgBulletinB_0001[] =
   "Incorrect dialog style.";


/**************** CascadeB.c / CascadeBG.c ****************/

_XmConst char _XmMsgCascadeB_0000[] =
   "XmCascadeButton[Gadget] must have xmRowColumnWidgetClass parent with\n\
rowColumnType XmMENU_PULLDOWN, XmMENU_POPUP, XmMENU_BAR or XmMENU_OPTION.";

_XmConst char _XmMsgCascadeB_0001[] =
   "Only XmMENU_PULLDOWN XmRowColumnWidgets can be submenus.";

_XmConst char _XmMsgCascadeB_0002[] =
   "MapDelay must be >= 0.";

_XmConst char _XmMsgCascadeB_0003[] =
   "XtGrabPointer failed";


/**************** Command.c ****************/

_XmConst char _XmMsgCommand_0000[] =
   "Dialog type must be XmDIALOG_COMMAND.";

_XmConst char _XmMsgCommand_0001[] =
   "Invalid child type, Command widget does not have this child.";

_XmConst char _XmMsgCommand_0002[] =
   "Invalid XmString, check for invalid charset.";

_XmConst char _XmMsgCommand_0003[] =
   "NULL or empty string passed in to CommandAppendValue.";

_XmConst char _XmMsgCommand_0004[] =
   "mustMatch is always False for a Command widget.";

_XmConst char _XmMsgCommand_0005[] =
   "historyMaxItems must be a positive integer greater than zero.";


/**************** CutPaste.c ****************/

_XmConst char _XmMsgCutPaste_0000[] =
   "Must call XmClipboardStartCopy() before XmClipboardCopy()";

_XmConst char _XmMsgCutPaste_0001[] =
   "Must call XmClipboardStartCopy() before XmClipboardEndCopy()";

_XmConst char _XmMsgCutPaste_0002[] =
   "Too many formats in XmClipboardCopy()";

_XmConst char _XmMsgCutPaste_0003[] =
   "ClipboardBadDataType";

_XmConst char _XmMsgCutPaste_0004[] =
   "bad data type";

_XmConst char _XmMsgCutPaste_0005[] =
   "ClipboardCorrupt";

_XmConst char _XmMsgCutPaste_0006[] =
   "internal error - corrupt data structure";

_XmConst char _XmMsgCutPaste_0007[] =
   "ClipboardBadFormat";

_XmConst char _XmMsgCutPaste_0008[] =
   "Error - registered format length must be 8, 16, or 32";

_XmConst char _XmMsgCutPaste_0009[] =
   "Error - registered format name must be non-null";


/**************** DialogS.c ****************/

_XmConst char _XmMsgDialogS_0000[] =
   "DialogShell widget only supports one rectObj child";


/**************** DragBS.c ****************/

_XmConst char _XmMsgDragBS_0000[] =
   "_MOTIF_DRAG_WINDOW has been destroyed";

_XmConst char _XmMsgDragBS_0001[] =
   "we aren't at the same version level";

_XmConst char _XmMsgDragBS_0002[] =
   "unable to open display";

_XmConst char _XmMsgDragBS_0003[] =
   "empty atoms table";

_XmConst char _XmMsgDragBS_0004[] =
   "empty target table";

_XmConst char _XmMsgDragBS_0005[] =
   "inconsistent targets table property";

_XmConst char _XmMsgDragBS_0006[] =
   "invalid target table index";


/**************** DragICC.c ****************/

_XmConst char _XmMsgDragICC_0000[] =
   "unknown dnd message type";

_XmConst char _XmMsgDragICC_0001[] =
   "we aren't at the same version level";


/**************** DragIcon.c ****************/

_XmConst char _XmMsgDragIcon_0000[] =
   "no geometry specified for dragIcon pixmap";

_XmConst char _XmMsgDragIcon_0001[] =
   "dragIcon with no pixmap created";

_XmConst char _XmMsgDragIcon_0002[] =
   "String to Bitmap converter needs Screen argument";


/**************** DragOverS.c ****************/

_XmConst char _XmMsgDragOverS_0000[] =
   "depth mismatch";

_XmConst char _XmMsgDragOverS_0001[] =
   "unknown icon attachment";

_XmConst char _XmMsgDragOverS_0002[] =
   "unknown drag state";

_XmConst char _XmMsgDragOverS_0003[] =
   "unknown blendModel";


/**************** DragUnder.c ****************/

_XmConst char _XmMsgDragUnder_0000[] =
   "unable to get dropSite window geometry";

_XmConst char _XmMsgDragUnder_0001[] =
   "invalid animationPixmapDepth";


/**************** Form.c ****************/

_XmConst char _XmMsgForm_0000[] =
   "Fraction base cannot be zero.";

_XmConst char _XmMsgForm_0002[] =
   "Circular dependency in Form children.";

_XmConst char _XmMsgForm_0003[] =
   "Bailed out of edge synchronization after 10,000 iterations.\n\
Check for contradictory constraints on the children of this form.";

_XmConst char _XmMsgForm_0004[] =
   "Attachment widget must have same parent as widget.";


/**************** GetSecRes.c ****************/

_XmConst char _XmMsgGetSecRes_0000[] =
   "getLabelSecResData: Not enough memory \n";


/**************** Label.c / LabelG.c ****************/

_XmConst char _XmMsgLabel_0003[] =
   "Invalid XmNlabelString - must be a compound string";

_XmConst char _XmMsgLabel_0004[] =
   "Invalid XmNacceleratorText - must be a compound string";


/**************** List.c ****************/

_XmConst char _XmMsgList_0000[] =
   "When changed, XmNvisibleItemCount must be at least 1.";

_XmConst char _XmMsgList_0005[] =
   "Cannot change XmNlistSizePolicy after initialization.";

_XmConst char _XmMsgList_0006[] =
   "When changed, XmNitemCount must be non-negative.";

_XmConst char _XmMsgList_0007[] =
   "Invalid item(s) to delete.";

_XmConst char _XmMsgList_0008[] =
   "XmNlistSpacing must be non-negative.";

_XmConst char _XmMsgList_0009[] =
   "Cannot set XmNitems to NULL when XmNitemCount is positive.";

_XmConst char _XmMsgList_0010[] =
   "When changed, XmNselectedItemCount must be non-negative.";

_XmConst char _XmMsgList_0011[] =
   "Cannot set XmNselectedItemCount to NULL when XmNselectedItemCount \
is positive.";

_XmConst char _XmMsgList_0012[] =
   "XmNtopItemPosition must be non-negative.";

_XmConst char _XmMsgList_0013[] =
   "XmNitems and XmNitemCount mismatch!";


/**************** MainW.c ****************/

_XmConst char _XmMsgMainW_0000[] =
   "The Menu Bar cannot be changed to NULL.";

_XmConst char _XmMsgMainW_0001[] =
   "The Command Window cannot be changed to NULL.";


/**************** Manager.c ****************/

_XmConst char _XmMsgManager_0000[] =
   "widget class %s has invalid CompositeClassExtension record";


/**************** MenuShell.c ****************/

_XmConst char _XmMsgMenuShell_0000[] =
   "MenuShell widgets must have a xmRowColumnWidgetClass child.";

_XmConst char _XmMsgMenuShell_0001[] =
   "Attempting to manage an incomplete menu.";


/**************** MessageB.c ****************/

_XmConst char _XmMsgMessageB_0003[] =
   "Invalid Child Type.";

_XmConst char _XmMsgMessageB_0004[] =
   "PushButton Id cannot be changed directly.";


/**************** NavigMap.c ****************/

_XmConst char _XmMsgNavigMap_0000[] =
   "_XmNavigate called with invalid direction";


/**************** PanedW.c ****************/

_XmConst char _XmMsgPanedW_0000[] =
   "Invalid minimum value, must be > 0.";

_XmConst char _XmMsgPanedW_0001[] =
   "Invalid maximum value, must be > 0.";

_XmConst char _XmMsgPanedW_0002[] =
   "Invalid minimum/maximum value, minimum must be < maximum.";

_XmConst char _XmMsgPanedW_0003[] =
   "Constraints do not allow appropriate sizing.";

_XmConst char _XmMsgPanedW_0004[] =
   "Too few parameters.";

_XmConst char _XmMsgPanedW_0005[] =
   "Invalid 1st parameter.";


/**************** Protocols.c ****************/

_XmConst char _XmMsgProtocols_0000[] =
   "must be a vendor shell";

_XmConst char _XmMsgProtocols_0001[] =
   "protocol mgr already exists";

_XmConst char _XmMsgProtocols_0002[] =
   "more protocols than I can handle";


/**************** Region.c ****************/

_XmConst char _XmMsgRegion_0000[] =
   "memory error";


/**************** ResConvert.c ****************/

_XmConst char _XmMsgResConvert_0000[] =
   "FetchUnitType: bad widget class";


/**************** RowColumn.c ****************/

_XmConst char _XmMsgRowColumn_0000[] =
   "Attempt to set width to zero ignored";

_XmConst char _XmMsgRowColumn_0001[] =
   "Attempt to set height to zero ignored";

_XmConst char _XmMsgRowColumn_0002[] =
   "XmNhelpWidget not used by PopUps: forced to NULL";

_XmConst char _XmMsgRowColumn_0003[] =
   "XmNhelpWidget not used by Pulldowns: forced to NULL";

_XmConst char _XmMsgRowColumn_0004[] =
   "XmNhelpWidget not used by Option menus: forced to NULL";

_XmConst char _XmMsgRowColumn_0005[] =
   "XmNhelpWidget not used by Work Areas: forced to NULL";

_XmConst char _XmMsgRowColumn_0007[] =
   "Widget hierarchy not appropriate for this XmNrowColumnType:\n\
defaulting to WorkArea";

_XmConst char _XmMsgRowColumn_0008[] =
   "Attempt to change XmNrowColumnType after initialization: ignored";

_XmConst char _XmMsgRowColumn_0015[] =
   "Attempt to set XmNisHomogenous to FALSE for a RowColumn widget of type \
XmMENU_BAR ignored";

_XmConst char _XmMsgRowColumn_0016[] =
   "Attempt to change XmNentryClass for a RowColumn widget of type \
XmMENU_BAR ignored";

_XmConst char _XmMsgRowColumn_0017[] =
   "Attempt to change XmNwhichButton via XtSetValues for a RowColumn widget \
of type XmMENU_PULLDOWN ignored";

_XmConst char _XmMsgRowColumn_0018[] =
   "Attempt to change XmNmenuPost via XtSetValues for a RowColumn widget \
of type XmMENU_PULLDOWN ignored";

_XmConst char _XmMsgRowColumn_0019[] =
   "Attempt to set XmNmenuPost to an illegal value ignored";

_XmConst char _XmMsgRowColumn_0020[] =
   "Attempt to change XmNshadowThickness for a RowColumn widget not of type \
XmMENU_PULLDOWN or XmMENU_POPUP ignored";

_XmConst char _XmMsgRowColumn_0022[] =
   "Attempt to add wrong type child to a menu (i.e. RowColumn) widget";

_XmConst char _XmMsgRowColumn_0023[] =
   "Attempt to add wrong type child to a homogeneous RowColumn widget";

_XmConst char _XmMsgRowColText_0024[] =
   "XtGrabKeyboard failed";

_XmConst char _XmMsgRowColumn_0025[] =
   "Attempt to change XmNisHomogeneous for a RowColumn widget of type \
XmMENU_OPTION ignored";

_XmConst char _XmMsgRowColumn_0026[] =
   "Tear off enabled on a shared menupane: allowed but not recommended";

_XmConst char _XmMsgRowColumn_0027[] =
   "Illegal mnemonic character;  Could not convert X KEYSYM to a keycode";


/**************** Scale.c ****************/

_XmConst char _XmMsgScale_0000[] =
   "The scale minumum value is greater than or equal to the scale maximum \
value.";

_XmConst char _XmMsgScale_0001[] =
   "The specified scale value is less than the minimum scale value.";

_XmConst char _XmMsgScale_0002[] =
   "The specified scale value is greater than the maximum scale value.";

_XmConst char _XmMsgScaleScrBar_0004[] =
   "Incorrect processing direction.";

_XmConst char _XmMsgScale_0005[] =
   "Invalid highlight thickness.";

_XmConst char _XmMsgScale_0006[] =
   "Invalid scaleMultiple; greater than (max - min)";

_XmConst char _XmMsgScale_0007[] =
   "Invalid scaleMultiple; less than zero";

_XmConst char _XmMsgScale_0008[] =
   "(Maximum - minimum) cannot be greater than INT_MAX / 2;\n\
minimum has been set to zero, maximum may have been set to (INT_MAX/2).";


/**************** Screen.c ****************/

_XmConst char _XmMsgScreen_0000[] =
   "icon screen mismatch";


/**************** ScrollBar.c ****************/

_XmConst char _XmMsgScrollBar_0000[] =
   "The scrollbar minimum value is greater than or equal to\n\
the scrollbar maximum value.";

_XmConst char _XmMsgScrollBar_0001[] =
   "The specified slider size is less than 1";

_XmConst char _XmMsgScrollBar_0002[] =
   "The specified scrollbar value is less than the minimum\n\
scroll bar value.";

_XmConst char _XmMsgScrollBar_0003[] =
   "The specified scrollbar value is greater than the maximum\n\
scrollbar value minus the scrollbar slider size.";

_XmConst char _XmMsgScrollBar_0004[] =
   "The scrollbar increment is less than 1.";

_XmConst char _XmMsgScrollBar_0005[] =
   "The scrollbar page increment is less than 1.";

_XmConst char _XmMsgScrollBar_0006[] =
   "The scrollbar initial delay is less than 1.";

_XmConst char _XmMsgScrollBar_0007[] =
   "The scrollbar repeat delay is less than 1.";

_XmConst char _XmMsgScrollBar_0008[] =
   "Specified slider size is greater than the scrollbar maximum\n\
value minus the scrollbar minimum value.";


/**************** ScrolledW.c ****************/

_XmConst char _XmMsgScrolledW_0004[] =
   "Cannot change scrolling policy after initialization.";

_XmConst char _XmMsgScrolledW_0005[] =
   "Cannot change visual policy after initialization.";

_XmConst char _XmMsgScrolledW_0006[] =
   "Cannot set AS_NEEDED scrollbar policy with a\nvisual policy of VARIABLE.";

_XmConst char _XmMsgScrolledW_0007[] =
   "Cannot change scrollbar widget in AUTOMATIC mode.";

_XmConst char _XmMsgScrolledW_0008[] =
   "Cannot change clip window";

_XmConst char _XmMsgScrolledW_0009[] =
   "Cannot set visual policy of CONSTANT in APPLICATION_DEFINED mode.";

_XmConst char _XmMsgScrollVis_0000[] =
   "Wrong parameters passed to the function.";

/**************** SelectioB.c ****************/

_XmConst char _XmMsgSelectioB_0001[] =
   "Dialog type cannot be modified.";

_XmConst char _XmMsgSelectioB_0002[] =
   "Invalid child type.";


/**************** Text.c ****************/

_XmConst char _XmMsgText_0000[] =
   "Invalid source, source ignored.";

_XmConst char _XmMsgText_0002[] =
   "Text widget is editable, Traversal_on must be true.";


/**************** TextF.c ****************/

_XmConst char _XmMsgTextF_0000[] =
   "Invalid cursor position, must be >= 0.";

_XmConst char _XmMsgTextF_0001[] =
   "Invalid columns, must be > 0.";

_XmConst char _XmMsgTextF_0002[] =
   "XmFontListInitFontContext Failed.";

_XmConst char _XmMsgTextF_0003[] =
   "XmFontListGetNextFont Failed.";

_XmConst char _XmMsgTextF_0004[] =
   "Character '%c', not supported in font.  Discarded.";

_XmConst char _XmMsgTextF_0005[] =
   "Traversal_on must always be true.";

_XmConst char _XmMsgTextF_0006[] =
   "Invalid columns, must be >= 0.";

_XmConst char _XmMsgTextFWcs_0000[] =
   "Character '%s', not supported in font.  Discarded.";

_XmConst char _XmMsgTextFWcs_0001[] =
   "Cannot use multibyte locale without a fontset.  Value discarded.";


/**************** TextIn.c ****************/

_XmConst char _XmMsgTextIn_0000[] =
   "Can't find position in MovePreviousLine().";


/**************** TextOut.c ****************/

_XmConst char _XmMsgTextOut_0000[] =
   "Invalid rows, must be > 0";


/**************** Vendor.c ****************/

_XmConst char _XmMsgVendor_0000[] =
   "invalid value for delete response";


/**************** VendorE.c ****************/

_XmConst char _XmMsgVendorE_0000[] =
   "String to noop conversion needs no extra arguments";

_XmConst char _XmMsgVendorE_0005[] =
   "FetchUnitType called without a widget to reference";


/**************** Visual.c ****************/

_XmConst char _XmMsgVisual_0000[] =
   "Invalid color requested from _XmAccessColorData";

_XmConst char _XmMsgVisual_0001[] =
   "Cannot allocate colormap entry for default background";

_XmConst char _XmMsgVisual_0002[] =
   "Cannot parse default background color spec";


/**************** XmIm.c *******************/

_XmConst char _XmMsgXmIm_0000[] =
    "Cannot open input method - using XLookupString";
