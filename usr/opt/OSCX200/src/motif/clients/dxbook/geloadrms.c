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
static char SccsId[] = "@(#)geloadrms.c	1.11\t8/23/89";
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**	GELOADGEO			Loads widget geometry
**	GELOADACC			Loads accelerators
**	GELOADSETTINGS			Loads customize settings
**	GESETCUSTOMIZE			Sets customize pulldown menu items
**	GESETPANEL     			Sets panel customize settings
**
**  ABSTRACT:
**
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	DAA 01/23/90 Created
**
**--
**/
extern double atof();

#include "geGks.h"
#include <X11/keysym.h>

static int 	horz, vert, interactive, clockwise, gravrad;
static float	scalef, rotangle, ft;
static char     buf[50];

#ifdef GERAGS
         
geLoadGeo()
{                       
static char     	*b;
static int      	PixWidth, MMWidth;
XrmValue		v;

PixWidth = XDisplayWidth  (geDispDev, geScreen);
MMWidth  = XDisplayWidthMM(geDispDev, geScreen);

/* Main
 */
GEGET_RMGEO(gra.main.x, 			geRM.Main.x);
GEGET_RMGEO(gra.main.y, 			geRM.Main.y);
GEGET_RMGEO(gra.main.width, 			geRM.Main.width);
GEGET_RMGEO(gra.main.height, 			geRM.Main.height);
geRM.Main.manage_flag = 1;

/* Panel
 */
GEGET_RMGEO(gra.panel.x, 			geRM.Panel.x);
GEGET_RMGEO(gra.panel.y, 			geRM.Panel.y);
GEGET_RMGEO(gra.panel.width, 			geRM.Panel.width);
GEGET_RMGEO(gra.panel.height, 			geRM.Panel.height);
geRM.Panel.manage_flag = 1;

/* Hints
 */
GEGET_RMGEO(gra.hints.x, 			geRM.Hints.x);
GEGET_RMGEO(gra.hints.y, 			geRM.Hints.y);
GEGET_RMGEO(gra.hints.width, 			geRM.Hints.width);
GEGET_RMGEO(gra.hints.height,			geRM.Hints.height);
GEGET_RMI(gra.hints.manage_flag,		geRM.Hints.manage_flag);

/* Grid
 */
GEGET_RMGEO(gra.grid.x, 			geRM.Grid.x);
GEGET_RMGEO(gra.grid.y, 	       		geRM.Grid.y);
GEGET_RMGEO(gra.grid.width, 			geRM.Grid.width);
GEGET_RMGEO(gra.grid.height,			geRM.Grid.height);
GEGET_RMI(gra.grid.manage_flag,	 		geRM.Grid.manage_flag);
                                                
/* Units
 */
GEGET_RMGEO(gra.units.x, 			geRM.Units.x);
GEGET_RMGEO(gra.units.y,   	 		geRM.Units.y);
GEGET_RMGEO(gra.units.width, 			geRM.Units.width);
GEGET_RMGEO(gra.units.height,			geRM.Units.height);
GEGET_RMI(gra.units.manage_flag,		geRM.Units.manage_flag);

/* Customize Settings
 */
GEGET_RMGEO(gra.customize.x, 			geRM.Customize.x);
GEGET_RMGEO(gra.customize.y,   	 		geRM.Customize.y);
GEGET_RMGEO(gra.customize.width, 		geRM.Customize.width);
GEGET_RMGEO(gra.customize.height,		geRM.Customize.height);
GEGET_RMI(gra.customize.manage_flag,		geRM.Customize.manage_flag);

/* Attributes
 */
XrmGetResource (geRM.SysDb, "gra.attr.type", NULL, &b, &v);
strcpy(geUtilBuf, v.addr);
*geUtilBuf = (char)tolower(*geUtilBuf);
     if (!strcmp(geUtilBuf, "object_interior"))   geAttr.MnCur = GEWOBJINTATTR;
else if (!strcmp(geUtilBuf, "text_fg"))           geAttr.MnCur = GEWTEXTFGATTR;
else if (!strcmp(geUtilBuf, "text_bg"))   	  geAttr.MnCur = GEWTEXTBGATTR;
else if (!strcmp(geUtilBuf, "page")) 		  geAttr.MnCur = GEWPAGEATTR;
else if (!strcmp(geUtilBuf, "font")) 		  geAttr.MnCur = GEWTEXTFONTS;
else 						  geAttr.MnCur = GEWOBJEXTATTR;

GEGET_RMGEO(gra.attr.x, 	    		geRM.Attr.x);
GEGET_RMGEO(gra.attr.y,   	 		geRM.Attr.y);
GEGET_RMGEO(gra.attr.width, 			geRM.Attr.width);
GEGET_RMGEO(gra.attr.height,			geRM.Attr.height);
GEGET_RMI(gra.attr.manage_flag,			geRM.Attr.manage_flag);

/* Load Default User Font for the UI and obtain the width and height.
 */
XrmGetResource (geRM.SysDb, "gra.fontList", NULL, &b, &v);
if (!(geFontMenu = XLoadQueryFont(geDispDev, v.addr)))
      geFontMenu = XLoadQueryFont(geDispDev, GEXFIXED_NAME);

geFontMenuWidth = geFontMenu->max_bounds.rbearing - 
		  geFontMenu->max_bounds.lbearing;
geFontMenuHeight = geFontMenu->ascent + geFontMenu->descent;

}
#endif

/****************************************************************************/
geLoadSettings()
{

static char     *b;
XrmValue	v;

#ifdef GERAGS

GEGET_RMI(gra.initialState, 		geStartIconified);

/*  Customize Menu
 */
GEGET_RMI(gra.cust.autoscroll, 		geState.ScrAuto);
GEGET_RMI(gra.cust.crosswindow, 	geState.CutOn);
GEGET_RMI(gra.cust.hilite_mode, 	geState.HilightMode);
GEGET_RMI(gra.cust.display_mode, 	geState.AnimDispMode);
GEGET_RMI(gra.cust.chg_linewght, 	geState.ZoomLineThick);
GEGET_RMI(gra.cust.linewght_points, 	geState.LinWPoints);
GEGET_RMI(gra.cust.anim_pointer, 	geState.AnimCursor);
GEGET_RMI(gra.cust.damage_repair, 	geState.DamageRepair);
GEGET_RMI(gra.cust.cropmode,  		geState.CropMode);
GEGET_RMI(gra.cust.checkpt, 		geState.ChkPt.On);
GEGET_RMI(gra.cust.checkpt_freq, 	geState.ChkPt.DoItAt);
GEGET_RMI(gra.cust.roundcoords, 	geState.RoundCoordsOnSave);
GEGET_RMI(gra.cust.pagebg, 		geState.PrintPgBg);
GEGET_RMI(gra.cust.store_image, 	geState.ImgStorage);
GEGET_RMI(gra.cust.auto_apply_attr,	geState.AutoApply);
GEGET_RMI(gra.cust.object_action_mode,	geState.ObjAction);
GEGET_RMI(gra.cust.confirm_dialogs, 	geState.Confirm);
GEGET_RMI(gra.cust.text_mode, 		geState.Insert);
GEGET_RMI(gra.cust.text_justify,	geJustTxt);
GEGET_RMI(gra.cust.tabs,		geState.Tab);
GEGET_RMI(gra.cust.automove,		geState.AutoMove);
GEGET_RMI(gra.cust.display_ps,	 	geDrawPS);

/* Control Panel
 */
XrmGetResource (geRM.SysDb, "gra.panel.object", NULL, &b, &v);
strcpy(geUtilBuf, v.addr);
*geUtilBuf = (char)tolower(*geUtilBuf);
     if (!strcmp(geUtilBuf, "arc"))             geObjSel = GEOBJ_ARC;
else if (!strcmp(geUtilBuf, "circle")) 		geObjSel = GEOBJ_CIR;
else if (!strcmp(geUtilBuf, "ellipse")) 	geObjSel = GEOBJ_ELP;
else if (!strcmp(geUtilBuf, "roundrect"))  	geObjSel = GEOBJ_BXR;
else if (!strcmp(geUtilBuf, "polygon")) 	geObjSel = GEOBJ_POL;
else if (!strcmp(geUtilBuf, "rectangle")) 	geObjSel = GEOBJ_BOX;
else if (!strcmp(geUtilBuf, "pie")) 		geObjSel = GEOBJ_PIE;
else 						geObjSel = GEOBJ_LIN;

XrmGetResource (geRM.SysDb, "gra.panel.editop", NULL, &b, &v);
strcpy(geUtilBuf, v.addr);
*geUtilBuf = (char)tolower(*geUtilBuf);
     if (!strcmp(geUtilBuf, "edit"))      	geState.EditCmd = GEEDIT;
else if (!strcmp(geUtilBuf, "group")) 	  	geState.EditCmd = GEJOIN;
else if (!strcmp(geUtilBuf, "ungroup")) 	geState.EditCmd = GESEP;
else if (!strcmp(geUtilBuf, "top")) 	  	geState.EditCmd = GEZT;
else if (!strcmp(geUtilBuf, "bottom")) 	  	geState.EditCmd = GEZB;
else if (!strcmp(geUtilBuf, "hide")) 	  	geState.EditCmd = GEXVISIBLE;
else if (!strcmp(geUtilBuf, "rotate")) 	  	geState.EditCmd = GEROTX;
else if (!strcmp(geUtilBuf, "align")) 	  	geState.EditCmd = GEALN;
else if (!strcmp(geUtilBuf, "delete")) 	  	geState.EditCmd = GEDEL;
else if (!strcmp(geUtilBuf, "copy")) 	  	geState.EditCmd = GECOPY;
else if (!strcmp(geUtilBuf, "mirror")) 	  	geState.EditCmd = GEFLIP;
else if (!strcmp(geUtilBuf, "copy/mirror"))  	geState.EditCmd = GECOPYFLP;
else if (!strcmp(geUtilBuf, "above")) 	  	geState.EditCmd = GEZU;
else if (!strcmp(geUtilBuf, "below")) 	  	geState.EditCmd = GEZD;
else if (!strcmp(geUtilBuf, "complement"))   	geState.EditCmd = GECOMP;
else if (!strcmp(geUtilBuf, "scale")) 	  	geState.EditCmd = GESCALE;
else if (!strcmp(geUtilBuf, "none")) 	  	geState.EditCmd = GENOOP;
else 						geState.EditCmd = GEMOVE;

XrmGetResource (geRM.SysDb, "gra.panel.alignfrom", NULL, &b, &v);
strcpy(geUtilBuf, v.addr);
*geUtilBuf = (char)tolower(*geUtilBuf);
     if (!strcmp(geUtilBuf, "bottom")) 	  	geAln.AlnFrom = GEALNBOT;
else if (!strcmp(geUtilBuf, "center")) 	  	geAln.AlnFrom = GEALNCHV;
else if (!strcmp(geUtilBuf, "left")) 	  	geAln.AlnFrom = GEALNLFT;
else if (!strcmp(geUtilBuf, "right")) 	  	geAln.AlnFrom = GEALNRT;
else 						geAln.AlnFrom = GEALNTOP;

XrmGetResource (geRM.SysDb, "gra.panel.alignto", NULL, &b, &v);
strcpy(geUtilBuf, v.addr);
*geUtilBuf = (char)tolower(*geUtilBuf);
     if (!strcmp(geUtilBuf, "bottom")) 	  	geAln.AlnTo = GEALNBOT;
else if (!strcmp(geUtilBuf, "center")) 	  	geAln.AlnTo = GEALNCHV;
else if (!strcmp(geUtilBuf, "left")) 	  	geAln.AlnTo = GEALNLFT;
else if (!strcmp(geUtilBuf, "right")) 	  	geAln.AlnTo = GEALNRT;
else if (!strcmp(geUtilBuf, "horizontal")) 	geAln.AlnTo = GEALNCH;
else if (!strcmp(geUtilBuf, "vertical")) 	geAln.AlnTo = GEALNCV;
else 						geAln.AlnTo = GEALNTOP;

GEGET_RMI(gra.panel.createtosize, 	geState.ObjEdCr);
GEGET_RMI(gra.panel.whiteout,     	geState.WhiteOut);
GEGET_RMI(gra.panel.stackontop,   	geState.StackOnTop);
GEGET_RMI(gra.panel.horizontal,   	horz);
GEGET_RMI(gra.panel.vertical,     	vert);
GEGET_RMI(gra.panel.editpoints,	geState.EditPts);
GEGET_RMI(gra.panel.ignoregroups,	geState.GrpEdit);
GEGET_RMI(gra.panel.gravity,      	geState.Grav.Align);
GEGET_RMI(gra.panel.gravradius,   	gravrad); 
GEGET_RMI(gra.panel.clockwise,  	clockwise);
GEGET_RMF(gra.panel.rotateangle,	rotangle);
GEGET_RMI(gra.panel.interactive,  	interactive);
GEGET_RMF(gra.panel.scalepercent,	scalef);

/* Zoom Factor
 */
GEGET_RMF(gra.cust.zoom_factor, 	geRM.ZoomUserDef);

/* Digitize Options
 */
GEGET_RMI(gra.cust.digmode, 		geState.DigMode);
GEGET_RMI(gra.cust.digconvcol, 		geState.DigConvCol);

/* Open/Include Dialog Box
 */
GEGET_RMI(gra.cust.group_open_drawing, geState.GroupOnOpen);
GEGET_RMI(gra.cust.group_include_drawing, geState.GroupOnImport);
GEGET_RMI(gra.cust.manual_placement, geState.ManualPlacement);

/* Grid Dialog Box
 */
GEGET_RMI(gra.grid.visible, 		geState.Grid.On);
GEGET_RMI(gra.grid.front,	 	geState.Grid.Top);
GEGET_RMI(gra.grid.snap.x,	 	geState.Grid.XAlign);
GEGET_RMI(gra.grid.snap.y,	 	geState.Grid.YAlign);
GEGET_RMF(gra.grid.major.x,   		geState.Grid.FMajorX);
GEGET_RMF(gra.grid.major.y, 		geState.Grid.FMajorY);
GEGET_RMI(gra.grid.div.x,	 	geState.Grid.DivX);
GEGET_RMI(gra.grid.div.y,	 	geState.Grid.DivY);

#endif

/* File Extensions
 */
if (geRM.SysDb)
    {XrmGetResource (geRM.SysDb, "gra.ext.meta", NULL, &b, &v);
     strcpy(geDefMeta,         	 v.addr);
     XrmGetResource (geRM.SysDb, "gra.ext.ddif", NULL, &b, &v);
     strcpy(geDefDDIF,    	 v.addr);
     XrmGetResource (geRM.SysDb, "gra.ext.sdml", NULL, &b, &v);
     strcpy(geDefSDML,    	 v.addr);
     XrmGetResource (geRM.SysDb, "gra.ext.list", NULL, &b, &v);
     strcpy(geDefLIST,    	 v.addr);
     XrmGetResource (geRM.SysDb, "gra.ext.ps", NULL, &b, &v);
     strcpy(geDefProofPs,    v.addr);
     XrmGetResource (geRM.SysDb, "gra.ext.six", NULL, &b, &v);
     strcpy(geDefProofSix,   v.addr);
     XrmGetResource (geRM.SysDb, "gra.ext.ddifi", NULL, &b, &v);
     strcpy(geDefProofDDIF,  v.addr);
     XrmGetResource (geRM.SysDb, "gra.ext.chkpt", NULL, &b, &v);
     strcpy(geDefChkPt,    	v.addr);
     XrmGetResource (geRM.SysDb, "gra.ext.fse", NULL, &b, &v);
     strcpy(geDefProofFSE,   v.addr);
     XrmGetResource (geRM.SysDb, "gra.ext.x", NULL, &b, &v);
     strcpy(geDefProofX,    	v.addr);
     XrmGetResource (geRM.SysDb, "gra.ext.text", NULL, &b, &v);
     strcpy(geDefProofText,	v.addr);
     XrmGetResource (geRM.SysDb, "gra.default.filename", NULL, &b, &v);
     strcpy(geDefRags,    	v.addr);

     /* Directory Specs
      */
     XrmGetResource (geRM.SysDb, "gra.indir.meta", NULL, &b, &v);
     if (*v.addr != '-')     strcpy(geDefDirIn, v.addr);
     else *geDefDirIn        = '\0';

#ifdef VMS
     XrmGetResource (geRM.SysDb, "gra.indir.clipart_vms", NULL, &b, &v);
#else
     XrmGetResource (geRM.SysDb, "gra.indir.clipart_ultrix", NULL, &b, &v);
#endif
     if (*v.addr != '-')     strcpy(geDefDirInClip, v.addr);
     else *geDefDirInClip    = '\0';

     XrmGetResource (geRM.SysDb, "gra.outdir.meta", NULL, &b, &v);
     if (*v.addr != '-')     strcpy(geDefDirOut, v.addr);
     else *geDefDirOut       = '\0';

     XrmGetResource (geRM.SysDb, "gra.indir.image", NULL, &b, &v);
     if (*v.addr != '-')     strcpy(geDefDirInImg, v.addr);
     else *geDefDirInImg     = '\0';

     XrmGetResource (geRM.SysDb, "gra.outdir.image", NULL, &b, &v);
     if (*v.addr != '-')     strcpy(geDefDirOutScrDmp, v.addr);
     else *geDefDirOutScrDmp = '\0';
    }
 else
    {strcpy(geDefMeta,           "gra");
    }
}

#ifdef GERAGS

/****************************************************************************/
geLoadAcc()
{                       
static char	*b;
XrmValue	v;

/*
 * Accelerators
 */
GEGET_RMK(gra.acc.move,       		geRM.Acc.Move);
GEGET_RMK(gra.acc.scale,      		geRM.Acc.Scale);
GEGET_RMK(gra.acc.edit,      		geRM.Acc.Edit);
GEGET_RMK(gra.acc.copy,      		geRM.Acc.Copy);
GEGET_RMK(gra.acc.delete,     		geRM.Acc.Delete);
GEGET_RMK(gra.acc.mirror,     		geRM.Acc.Mirror);
GEGET_RMK(gra.acc.copymirror, 		geRM.Acc.CopyMirror);
GEGET_RMK(gra.acc.group,      		geRM.Acc.Group);
GEGET_RMK(gra.acc.ungroup,    		geRM.Acc.Ungroup);
GEGET_RMK(gra.acc.top,        		geRM.Acc.Top);
GEGET_RMK(gra.acc.bottom,     		geRM.Acc.Bottom);
GEGET_RMK(gra.acc.above,      		geRM.Acc.AboveObj);
GEGET_RMK(gra.acc.below,      		geRM.Acc.BelowObj);
GEGET_RMK(gra.acc.hide,      		geRM.Acc.Hide);
GEGET_RMK(gra.acc.complement, 		geRM.Acc.Complement);
GEGET_RMK(gra.acc.rotate,     		geRM.Acc.Rotate);
GEGET_RMK(gra.acc.align,      		geRM.Acc.Align);
GEGET_RMK(gra.acc.none,      		geRM.Acc.NoOp);
GEGET_RMK(gra.acc.horizontal, 		geRM.Acc.Horizontal);
GEGET_RMK(gra.acc.vertical,   		geRM.Acc.Vertical);
GEGET_RMK(gra.acc.editpoints, 		geRM.Acc.EditPoints);
GEGET_RMK(gra.acc.ignoregroups,		geRM.Acc.IgnoreGroups);
GEGET_RMK(gra.acc.gravity,     		geRM.Acc.Gravity);
GEGET_RMK(gra.acc.clockwise,   		geRM.Acc.Clockwise);
GEGET_RMK(gra.acc.interactive, 		geRM.Acc.Interactive);
GEGET_RMK(gra.acc.createtosize,		geRM.Acc.CreateToSize);
GEGET_RMK(gra.acc.stackontop,  		geRM.Acc.StackOnTop);
GEGET_RMK(gra.acc.arc,      		geRM.Acc.Arc);
GEGET_RMK(gra.acc.circle,     		geRM.Acc.Circle);
GEGET_RMK(gra.acc.ellipse,    		geRM.Acc.Ellipse);
GEGET_RMK(gra.acc.line,      		geRM.Acc.Line);
GEGET_RMK(gra.acc.rectangle,  		geRM.Acc.Rectangle);
GEGET_RMK(gra.acc.roundrect,  		geRM.Acc.RoundRect);
GEGET_RMK(gra.acc.pie,      		geRM.Acc.Pie);
GEGET_RMK(gra.acc.polygon,   		geRM.Acc.Polygon);
}
/****************************************************************************/
geSetCustomize()
{
Arg	al[2];

/* Toggle the appropriate buttons in the customize settings popup to
 * reflect the resource settings. 
 */
if (geState.ScrAuto)				/* autocscroll */
     XmToggleButtonSetState(geUI.WidgetArray[GEWSCRAUTO], 1, 0);	
else XmToggleButtonSetState(geUI.WidgetArray[GEWSCRAUTO], 0, 0);	

if (geState.CutOn)                      	/* cross window */
     XmToggleButtonSetState(geUI.WidgetArray[GEWCROSSWINDOW], 1, 0);	
else XmToggleButtonSetState(geUI.WidgetArray[GEWCROSSWINDOW], 0, 0);	

if (geState.HilightMode == GEHILIGHT_BLINK)     /* blink highlight mode */
     XmToggleButtonSetState(geUI.WidgetArray[GEWHILIGHTMODE], 1, 0);	
else XmToggleButtonSetState(geUI.WidgetArray[GEWHILIGHTMODE], 0, 0);	

if (geState.ZoomLineThick)			/* change line weight (zoom) */
     XmToggleButtonSetState(geUI.WidgetArray[GEWZOOMLINETHICK], 1, 0);	
else XmToggleButtonSetState(geUI.WidgetArray[GEWZOOMLINETHICK], 0, 0);	

if (geState.LinWPoints)				/* line weight in points */
     XmToggleButtonSetState(geUI.WidgetArray[GEWLINEPOINTS], 1, 0);	
else XmToggleButtonSetState(geUI.WidgetArray[GEWLINEPOINTS], 0, 0);	

if (geState.DamageRepair)                    	/* damage repair */
     XmToggleButtonSetState(geUI.WidgetArray[GEWDAMAGEREPAIR], 1, 0);	
else XmToggleButtonSetState(geUI.WidgetArray[GEWDAMAGEREPAIR], 0, 0);	

if (geState.AnimCursor)                   	/* pointer on in animation */
     XmToggleButtonSetState(geUI.WidgetArray[GEWANIMCURSOR], 1, 0);	
else XmToggleButtonSetState(geUI.WidgetArray[GEWANIMCURSOR], 0, 0);	

switch (geState.AnimDispMode)	     		/* animation display mode */
  {case GEANIM_LIN:
   default:	
     GESET_ARG(geUI.WidgetArray[GEWANIMDISPMENU], XmNmenuHistory, 
      			  	    geUI.WidgetArray[GEWANIM_LIN]);
     break;
   case GEANIM_TRUE:
     GESET_ARG(geUI.WidgetArray[GEWANIMDISPMENU], XmNmenuHistory, 
      			  	    geUI.WidgetArray[GEWANIM_TRUE]);
     break;
   case GEANIM_BOX:
     GESET_ARG(geUI.WidgetArray[GEWANIMDISPMENU], XmNmenuHistory, 
      			  	    geUI.WidgetArray[GEWANIM_BOX]);
     break;
   case GEANIM_NONE:
     GESET_ARG(geUI.WidgetArray[GEWANIMDISPMENU], XmNmenuHistory, 
     			  	    geUI.WidgetArray[GEWANIM_NONE]);
     break;
  }

if (geState.AutoApply)				/* auto apply attributes */
     XmToggleButtonSetState(geUI.WidgetArray[GEWAUTOAPPLY], 1, 0);	
else XmToggleButtonSetState(geUI.WidgetArray[GEWAUTOAPPLY], 0, 0);	

if (geState.ObjAction) 				/* object->action mode */
     XmToggleButtonSetState(geUI.WidgetArray[GEWOBJACTION], 1, 0);	
else XmToggleButtonSetState(geUI.WidgetArray[GEWOBJACTION], 0, 0);	

if (geState.Confirm)   				/* show confirmations  */
     XmToggleButtonSetState(geUI.WidgetArray[GEWCONFIRM], 1, 0);	
else XmToggleButtonSetState(geUI.WidgetArray[GEWCONFIRM], 0, 0);	

if (geState.AutoMove)                         	/* Auto Move */
     XmToggleButtonSetState(geUI.WidgetArray[GEWAUTOMOVE], 1, 0);	
else XmToggleButtonSetState(geUI.WidgetArray[GEWAUTOMOVE], 0, 0);	

if (geState.RoundCoordsOnSave)			/* round coords on save */
     XmToggleButtonSetState(geUI.WidgetArray[GEWROUNDCOORDS], 1, 0);	
else XmToggleButtonSetState(geUI.WidgetArray[GEWROUNDCOORDS], 0, 0);	

if (geState.PrintPgBg)				/* print page bg on output */
     XmToggleButtonSetState(geUI.WidgetArray[GEWPRINTPGBG], 1, 0);	
else XmToggleButtonSetState(geUI.WidgetArray[GEWPRINTPGBG], 0, 0);	

if (geState.ImgStorage == GEIMGSTORAGE_DATA)	/* image storage in metafile */
     XmToggleButtonSetState(geUI.WidgetArray[GEWIMGSTORAGE], 1, 0);	
else XmToggleButtonSetState(geUI.WidgetArray[GEWIMGSTORAGE], 0, 0);	

if (geDrawPS) 					/* display EPS objects */
     XmToggleButtonSetState(geUI.WidgetArray[GEWDISPLAYEPS], 1, 0);	
else XmToggleButtonSetState(geUI.WidgetArray[GEWDISPLAYEPS], 0, 0);	

if (geStartIconified)				/* start iconififed */
     XmToggleButtonSetState(geUI.WidgetArray[GEWSTARTICON], 1, 0);	
else XmToggleButtonSetState(geUI.WidgetArray[GEWSTARTICON], 0, 0);	

if (geState.CropMode == GECROPAUTO)	     	/* cropping */
  { GESET_ARG(geUI.WidgetArray[GEWCROPMENU], XmNmenuHistory, 
	      geUI.WidgetArray[GEWAUTOCROP]); }
else if (geState.CropMode == GECROPMAN)
  { GESET_ARG(geUI.WidgetArray[GEWCROPMENU], XmNmenuHistory, 
	      geUI.WidgetArray[GEWMANUALCROP]); }
else if (geState.CropMode == GECROPWIN)
  { GESET_ARG(geUI.WidgetArray[GEWCROPMENU], XmNmenuHistory, 
	      geUI.WidgetArray[GEWWINDOWCROP]); }

if (!geState.ChkPt.On)                   	/* checkpointing */
  { GESET_ARG(geUI.WidgetArray[GEWCHECKPTMENU], XmNmenuHistory, 
      			  	    geUI.WidgetArray[GEWCHKPT_OFF]); }
else 
    {switch (geState.ChkPt.DoItAt)           
       {case 5:
          GESET_ARG(geUI.WidgetArray[GEWCHECKPTMENU], XmNmenuHistory, 
      			  	    geUI.WidgetArray[GEWCHKPT_5]);
	  break;
        case 15:
        default:
          GESET_ARG(geUI.WidgetArray[GEWCHECKPTMENU], XmNmenuHistory, 
      			  	    geUI.WidgetArray[GEWCHKPT_15]);
	  break;
        case 50:
          GESET_ARG(geUI.WidgetArray[GEWCHECKPTMENU], XmNmenuHistory, 
      			  	    geUI.WidgetArray[GEWCHKPT_50]);
	  break;
        case 100:
          GESET_ARG(geUI.WidgetArray[GEWCHECKPTMENU], XmNmenuHistory, 
      			  	    geUI.WidgetArray[GEWCHKPT_100]);
	  break;
        case 200:
          GESET_ARG(geUI.WidgetArray[GEWCHECKPTMENU], XmNmenuHistory, 
      			  	    geUI.WidgetArray[GEWCHKPT_200]);
	  break;
       }
    }

switch (geJustTxt)		                /* Text Justification */
  {case GEJUSTTXTC:
     GESET_ARG(geUI.WidgetArray[GEWTEXTJUSTMENU], XmNmenuHistory, 
      			  	    geUI.WidgetArray[GEWJUSTTXTC]);
     break;
   case GEJUSTTXTL:
   default:	
     GESET_ARG(geUI.WidgetArray[GEWTEXTJUSTMENU], XmNmenuHistory, 
      			  	    geUI.WidgetArray[GEWJUSTTXTL]);
     break;	
   case GEJUSTTXTR:
     GESET_ARG(geUI.WidgetArray[GEWTEXTJUSTMENU], XmNmenuHistory, 
      			  	    geUI.WidgetArray[GEWJUSTTXTR]);
    break;
  }

if (geState.Insert)		                /* Text Edit Mode */
     { GESET_ARG(geUI.WidgetArray[GEWTEXTMODEMENU], XmNmenuHistory, 
      			  	    geUI.WidgetArray[GEWINSERT]); }
else { GESET_ARG(geUI.WidgetArray[GEWTEXTMODEMENU], XmNmenuHistory, 
      			  	    geUI.WidgetArray[GEWOVERSTRIKE]); }
	
sprintf(buf, "%d", geState.Tab);		/* Tab Setting */
XmTextSetString(geUI.WidgetArray[GEWTAB], buf);

}

/****************************************************************************/
geSetPanel()
{
Arg	al[2];

/* Toggle the appropriate buttons in the control panel to reflect the
 * resource settings. 
 */

switch (geObjSel)
  {case GEOBJ_LIN:
   default:
     XmToggleButtonSetState(geUI.WidgetArray[GEWLINOBJECT], 1, 1);	break;
   case GEOBJ_ARC:
     XmToggleButtonSetState(geUI.WidgetArray[GEWARCOBJECT], 1, 1);	break;
   case GEOBJ_CIR:
     XmToggleButtonSetState(geUI.WidgetArray[GEWCIROBJECT], 1, 1);	break;
   case GEOBJ_ELP:
     XmToggleButtonSetState(geUI.WidgetArray[GEWELPOBJECT], 1, 1);	break;
   case GEOBJ_BXR:
     XmToggleButtonSetState(geUI.WidgetArray[GEWBXROBJECT], 1, 1);	break;
   case GEOBJ_POL:                                                      
     XmToggleButtonSetState(geUI.WidgetArray[GEWPOLYOBJECT], 1, 1);	break;
   case GEOBJ_BOX:
     XmToggleButtonSetState(geUI.WidgetArray[GEWBOXOBJECT], 1, 1);	break;
   case GEOBJ_PIE:                                                   	
     XmToggleButtonSetState(geUI.WidgetArray[GEWPIEOBJECT], 1, 1);   	break;
  }

if (geState.ObjEdCr)
  XmToggleButtonSetState(geUI.WidgetArray[GEWCREATETOSIZE], 1, 1);	
if (geState.WhiteOut)
  XmToggleButtonSetState(geUI.WidgetArray[GEWWHITEOUT], 1, 1);	
if (geState.StackOnTop)
  XmToggleButtonSetState(geUI.WidgetArray[GEWSTACKONTOP], 1, 1);	
if (horz)
  XmToggleButtonSetState(geUI.WidgetArray[GEWHORZCONSTR], 1, 1);	
if (vert)
  XmToggleButtonSetState(geUI.WidgetArray[GEWVERTCONSTR], 1, 1);	
if (geState.EditPts)
  XmToggleButtonSetState(geUI.WidgetArray[GEWEDITPOINTS], 1, 1);	
if (geState.GrpEdit)
  XmToggleButtonSetState(geUI.WidgetArray[GEWGRPEDIT], 1, 1);	

sprintf(buf, "%d", gravrad);
XmTextSetString(geUI.WidgetArray[GEWGRAVVAL], buf);
if (geState.Grav.Align)
  XmToggleButtonSetState(geUI.WidgetArray[GEWGRAVON], 1, 1);	

sprintf(buf, "%.1f", rotangle);
XmTextSetString(geUI.WidgetArray[GEWROTATEANGLE], buf);
if (clockwise)
  XmToggleButtonSetState(geUI.WidgetArray[GEWCLOCKWISE], 1, 1);	

sprintf(buf, "%.1f", scalef);
XmTextSetString(geUI.WidgetArray[GEWSCALEPERCENT], buf);
if (interactive)
  XmToggleButtonSetState(geUI.WidgetArray[GEWINTERACTIVE], 1, 1);	

switch (geState.EditCmd)
  {case GEMOVE:
   default:
     XmToggleButtonSetState(geUI.WidgetArray[GEWMOVE_OBJ], 1, 1);	break;
   case GEEDIT:
     XmToggleButtonSetState(geUI.WidgetArray[GEWEDIT_OBJ], 1, 1);	break;
   case GEJOIN:
     XmToggleButtonSetState(geUI.WidgetArray[GEWGROUP_OBJ], 1, 1);	break;
   case GESEP:
     XmToggleButtonSetState(geUI.WidgetArray[GEWUNGROUP_OBJ], 1, 1);	break;
   case GEZT:
     XmToggleButtonSetState(geUI.WidgetArray[GEWTOP_OBJ], 1, 1);	break;
   case GEZB:
     XmToggleButtonSetState(geUI.WidgetArray[GEWBOTTOM_OBJ], 1, 1);	break;
   case GEXVISIBLE:
     XmToggleButtonSetState(geUI.WidgetArray[GEWHIDE_OBJ], 1, 1);	break;
   case GEROTX:
     XmToggleButtonSetState(geUI.WidgetArray[GEWROTATE_OBJ], 1, 1);	break;
   case GEALN:
     XmToggleButtonSetState(geUI.WidgetArray[GEWALIGN_OBJ], 1, 1);	break;
   case GEDEL:
     XmToggleButtonSetState(geUI.WidgetArray[GEWDELETE_OBJ], 1, 1);	break;
   case GECOPY:
     XmToggleButtonSetState(geUI.WidgetArray[GEWCOPY_OBJ], 1, 1);	break;
   case GEFLIP:
     XmToggleButtonSetState(geUI.WidgetArray[GEWMIRROR_OBJ], 1, 1);	break;
   case GECOPYFLP:
     XmToggleButtonSetState(geUI.WidgetArray[GEWCOPYMIRROR_OBJ], 1, 1); break;
   case GEZU:
     XmToggleButtonSetState(geUI.WidgetArray[GEWABOVE_OBJ], 1, 1);	break;
   case GEZD:
     XmToggleButtonSetState(geUI.WidgetArray[GEWBELOW_OBJ], 1, 1);	break;
   case GECOMP:
     XmToggleButtonSetState(geUI.WidgetArray[GEWCOMPLEMENT_OBJ], 1, 1); break;
   case GESCALE:
     XmToggleButtonSetState(geUI.WidgetArray[GEWSCALE_OBJ], 1, 1);	break;
   case GENOOP:
     XmToggleButtonSetState(geUI.WidgetArray[GEWANIMATE_OBJ], 1, 1);	break;
  }

switch(geAln.AlnFrom)
  {case GEALNTOP:
   default:
     GESET_ARG(geUI.WidgetArray[GEWALNFROMMENU], XmNmenuHistory, 
                 geUI.WidgetArray[GEWALNFROMTOP]);	break;
   case GEALNBOT:
     GESET_ARG(geUI.WidgetArray[GEWALNFROMMENU], XmNmenuHistory, 
                 geUI.WidgetArray[GEWALNFROMBOT]);	break;
   case GEALNCHV:
     GESET_ARG(geUI.WidgetArray[GEWALNFROMMENU], XmNmenuHistory, 
                 geUI.WidgetArray[GEWALNFROMCTR]);	break;
   case GEALNLFT:
     GESET_ARG(geUI.WidgetArray[GEWALNFROMMENU], XmNmenuHistory, 
                 geUI.WidgetArray[GEWALNFROMLFT]);      break; 
   case GEALNRT:
     GESET_ARG(geUI.WidgetArray[GEWALNFROMMENU], XmNmenuHistory, 
                 geUI.WidgetArray[GEWALNFROMRT]);	break;
  }	

switch(geAln.AlnTo)
  {case GEALNTOP:
   default:
     GESET_ARG(geUI.WidgetArray[GEWALNTOMENU], XmNmenuHistory, 
                 geUI.WidgetArray[GEWALNTOTOP]);	break;
   case GEALNBOT:
     GESET_ARG(geUI.WidgetArray[GEWALNTOMENU], XmNmenuHistory, 
                 geUI.WidgetArray[GEWALNTOBOT]);	break;
   case GEALNCHV:
     GESET_ARG(geUI.WidgetArray[GEWALNTOMENU], XmNmenuHistory, 
                 geUI.WidgetArray[GEWALNTOCTR]);	break;
   case GEALNLFT:
     GESET_ARG(geUI.WidgetArray[GEWALNTOMENU], XmNmenuHistory, 
                 geUI.WidgetArray[GEWALNTOLFT]);	break;
   case GEALNRT:
     GESET_ARG(geUI.WidgetArray[GEWALNTOMENU], XmNmenuHistory, 
                 geUI.WidgetArray[GEWALNTORT]);		break;
   case GEALNCH:
     GESET_ARG(geUI.WidgetArray[GEWALNTOMENU], XmNmenuHistory, 
                 geUI.WidgetArray[GEWALNTOHORZ]);	break;
   case GEALNCV:
     GESET_ARG(geUI.WidgetArray[GEWALNTOMENU], XmNmenuHistory, 
                 geUI.WidgetArray[GEWALNTOVERT]);	break;
  }	

}

#endif
