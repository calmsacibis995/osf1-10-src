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
 * @(#)$RCSfile: motifgif.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 17:58:18 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: motifgif.h,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 17:58:18 $ */

/*****************************************************************************
******************************************************************************
**
**       FILE:  gif.h
**
**       PROJECT:   Motif -- giff drawing selector
**
**
******************************************************************************
*****************************************************************************/

/*****************************************************
*                                                    *
*   Revision History:                                *
*   levine                                           *
*****************************************************/

/*  Standard C headers  */
#include <stdio.h>
#include <sys/signal.h>

/*  X headers  */
#include <X11/IntrinsicP.h>
/*#include <X11/Shell.h>
#include <X11/AtomMgr.h>
#include <X11/Protocols.h>
*/
/*  Xm headers  */
#include <Xm/Xm.h>
#include <Xm/ArrowB.h>
#include <Xm/ArrowBG.h>
#include <Xm/BulletinB.h>
#include <Xm/CascadeB.h>
#include <Xm/CascadeBG.h>
#include <Xm/Command.h>
#include <Xm/CutPaste.h>
#include <Xm/DialogS.h>
#include <Xm/DrawingA.h>
#include <Xm/DrawnB.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/MainW.h>
#include <Xm/MenuShell.h>
#include <Xm/MessageB.h>
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>
#include <Xm/SelectioB.h>
#include <Xm/SeparatoG.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>


#define WIDTH        200
#define HEIGHT       200
#define MAX_ARGS     100
/* below are three different types of pushbuttons (and reset) */
#define ACCEL        0x01
#define DEFAULT      0x02
#define INACTIVE     0x04
#define BITSOFF      0x00

/*  Global Variables  */

Display *display;
int     screen;

/* flag to select what type of pushbutton you want--
   a default one, one with an accelerator, or an inactive one */
unsigned int  bit_flag;
Widget    Dialog;
Widget    Shell;
Widget    MainWindow;
Widget    MenuBar;
Widget    PullDown1;
Widget    PullDown2;
Widget    PullDown3;
Widget    PopUp;
Widget    popup_label;
Widget    MenuBtn1;
Widget    MenuBtn2;
Widget    MenuBtn3;

/* contents of menu 1 */
Widget    Label1A;
Widget    Label1B;
Widget    Label1C;
Widget    Label1D;
Widget    Label1E;
Widget    Label1F;
Widget    Separator1A;
Widget    RadioBox1;
Widget    RadioBtn1A;
Widget    RadioBtn1B;
Widget    Separator1B;
Widget    ToggleBtn1A;
Widget    ToggleBtn1B;

/* contents of menu 2 */
Widget    Label2A;
Widget    Label2B;
Widget    Label2C;
Widget    Label2D;
Widget    Label2E;
Widget    PullDown2A;
Widget    Cascade2;
Widget    Label2AA;
Widget    Label2AB;
Widget    Separator2A;
Widget    RadioBox2;
Widget    RadioBtn2A;
Widget    RadioBtn2B;
Widget    Separator2B;
Widget    ToggleBtn2A;
Widget    ToggleBtn2B;

/* contents of menu 3 */
Widget    Label3A;
Widget    Label3B;
Widget    Label3C;
Widget    Label3D;
Widget    Label3E;
Widget    Label3F;
Widget    Separator3A;
Widget    RadioBox3;
Widget    RadioBtn3A;
Widget    RadioBtn3B;
Widget    Separator3B;
Widget    ToggleBtn3A;
Widget    ToggleBtn3B;

/* contents of popup menu */
Widget    LabelP1;
Widget    LabelP2;
Widget    LabelP3;
Widget    LabelP4;
Widget    PullDownP;
Widget    CascadeP;
Widget    LabelP5A;
Widget    LabelP5B;

Widget    HorizScrollBar;
Widget    VertScrollBar;
Widget    WorkRegion;
Widget    TextWin;
Widget    ScrollWin;
Widget    ScrollBar;
Widget    FormWin;
Widget    LabelW;
Widget    TBoardW;
Widget    TextMenuBar;
Widget    PullDownText1;
Widget    PullDownText2;
Widget    PullDownText3;
Widget    MenuBtnText1;
Widget    MenuBtnText2;
Widget    MenuBtnText3;
Widget    HierW;
Widget    HierW2;
Widget    LabelBoardW;
Widget    SWin;
Widget    ScrollBarText;
Widget    DialogSh;
Widget    FileDialog;
Widget    FileSelection;
Widget    ScrollBarList;
Widget    ResourceList;
Widget    ResourceDialog;
Widget    WidgetDList;
Widget    WidgetDialog;
Widget    FileOK;

