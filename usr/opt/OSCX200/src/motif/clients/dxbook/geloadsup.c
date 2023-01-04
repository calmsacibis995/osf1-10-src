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
static char SccsId[] = "@(#)geloadsup.c	1.11\t8/23/89";
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
**	GELOADSUP			Loads set-up file
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
**	GNE 05/01/86 Created
**	DAA 01/05/89 Revised
**
**--
**/

#include "geGks.h"
#include "geFnt.h"
#include <math.h>
         
extern char 		*geMalloc();
extern XFontStruct 	*XLoadQueryFont();

geLoadSup()
{                       
int		PixWidth, MMWidth, col1, col2, col3, col4;
int		i, j, itemp, isave;
char		*msg_string, *col_string, *error_string, *b, *ptr;
char 		msgbuf[50], colbuf[100], unitbuf[50], fontbuf[50], model[5];
float		res, resfact;
XrmValue	v, unit_def;

struct          GE_COL_LIST *ColListP;

#ifdef GERAGS
XmString	string;
#endif


/*
 * Make sure everything starts out with NULL
 */
for (i = 0; i < GE_MAX_FNTS;  i++) geFntAlloced[i] = NULL;
for (i = 0; i < GE_MAX_FNTS;  i++) geFntPtr[i] 	   = NULL;
for (i = 0; i < GE_MAX_UNITS; i++) geU.Str[i]      = NULL;
for (i = 0; i < GE_MAX_MSGS;  i++) geMsgPtr[i]     = NULL;

/* Get the Hints Messages from the UIL file. (1-based array)
 */
#ifdef GERAGS

for (i=1; i < GE_MAX_MSGS; i++)
  {sprintf(msgbuf, "GE_MSG_%d", i); 
   msg_string = (char *) geFetchLiteral(msgbuf, MrmRtypeChar8);
   GEMSGALLOC(geMsgPtr[i], msg_string);
   XtFree(msg_string);
  }

#endif

/* Next get the Fonts (from geFnt.h). Add the fonts to the Fonts ListBox.  
 */
i = 0;
while (GenFnts[i])
  {geFntPtr[i] = GenFnts[i]; 

#ifdef GERAGS
   string = XmStringCreateSimple(geFntPtr[i]);
   XmListAddItem(geUI.WidgetArray[GEWFONTSLIST], string, i+1);
#endif

   i++;
  }

if (!geRun.RagsCalling && !geRun.MopsCalling && !geRun.VoyerCalling)
  {geAttr.Font 	     = GEFONT_USER_DEFAULT;
   geFontUser[geAttr.Font] = XLoadQueryFont(geDispDev, GEXFIXED_NAME);
   return;
  }

/*
 * Get the Users Fonts from the resource file - first the X server names
 */ 
itemp = isave = i;
for (i=1, j=0; i < GE_MAX_USER_FNTS; i++)
   {sprintf(fontbuf, "gra.font.terminal_%d", i);
    XrmGetResource (geRM.SysDb, fontbuf, NULL, &b, &v);
    if (v.addr)
      {GEMSGALLOC(ptr, v.addr);
       geFntXName[itemp++] = geFntAlloced[j++] = ptr;
      }
   }
/*
 * Now get the PostScript names corresponding to the above list
 */ 
itemp = isave;
for (i=1, j=0; i < GE_MAX_USER_FNTS; i++)
   {sprintf(fontbuf, "gra.font.postscript_%d", i);
    XrmGetResource (geRM.SysDb, fontbuf, NULL, &b, &v);
    if (v.addr)
      {GEMSGALLOC(ptr, v.addr);
       geFntPtr[itemp] = geFntAlloced[j++] = ptr;
/*
 * Add the Users Fonts to the Fonts ListBox.  
 */
#ifdef GERAGS
       string = XmStringCreateSimple(geFntPtr[itemp]);
       XmListAddItem(geUI.WidgetArray[GEWFONTSLIST], string, itemp+1);
#endif
       itemp++;	
      }
   }
/*
 * None of the user selectable colors need to be loaded except for interactive
 * selection from the list box within RAGS itself.
 */
#ifdef GERAGS

/* Get the Users Colors from the resource file. 
 */
for (i=0, j=1; j < GE_MAX_COLS; j++)
   {sprintf(colbuf, "gra.col_%d", j);
    XrmGetResource (geRM.SysDb, colbuf, NULL, &b, &v);
    if (v.addr)
      {if (!geColList0)
	 {geColList0 = ColListP =
	    (struct GE_COL_LIST *)geMalloc(sizeof(struct GE_COL_LIST));
	 }
       else
	 {ColListP->Next =
	    (struct GE_COL_LIST *)geMalloc(sizeof(struct GE_COL_LIST));
	  ColListP = ColListP->Next;
	 }
       sscanf(v.addr, "%s", model);
       if (!strcmp(model, "RGB") || !strcmp(model, "rgb"))
         {sscanf(v.addr, "%s %d %d %d %s", 
 			model, &col1, &col2, &col3, colbuf);
	  col1 = (col1 * GEMAXUSHORT) / 100;
	  col2 = (col2 * GEMAXUSHORT) / 100;
	  col3 = (col3 * GEMAXUSHORT) / 100;
	  ColListP->Col.RGB.red    = (unsigned short)(min(col1, GEMAXUSHORT));
	  ColListP->Col.RGB.green  = (unsigned short)(min(col2, GEMAXUSHORT));
	  ColListP->Col.RGB.blue   = (unsigned short)(min(col3, GEMAXUSHORT));
	  geGenRgbToCmyk(&(ColListP->Col.RGB), &(ColListP->Col.CMYK));
	 }
       else if (!strcmp(model, "CMYK") || !strcmp(model, "cmyk"))
         {sscanf(v.addr, "%s %d %d %d %d %s", 
		 model, &col1, &col2, &col3, &col4, colbuf);
	  col1 = (col1 * GEMAXUSHORT) / 100;
	  col2 = (col2 * GEMAXUSHORT) / 100;
	  col3 = (col3 * GEMAXUSHORT) / 100;
	  col4 = (col4 * GEMAXUSHORT) / 100;
	  ColListP->Col.CMYK.C = (unsigned short)(min(col1, GEMAXUSHORT));
	  ColListP->Col.CMYK.M = (unsigned short)(min(col2, GEMAXUSHORT));
	  ColListP->Col.CMYK.Y = (unsigned short)(min(col3, GEMAXUSHORT));
	  ColListP->Col.CMYK.K = (unsigned short)(min(col4, GEMAXUSHORT));
	  geGenCmykToRgb(&(ColListP->Col.CMYK), &(ColListP->Col.RGB));
	 }
       ColListP->Col.Name = geMalloc(strlen(colbuf) + 1);
       strcpy(ColListP->Col.Name, colbuf);
       ColListP->Next = NULL;
/*
 * Add the Users Colors to the Color ListBox.  
 */
       string = XmStringCreateSimple(ColListP->Col.Name);
       XmListAddItem(geUI.WidgetArray[GEWCOLORLIST], string, i+1);
       i++;
      }	
  }

geGlobin2();                                    /* RAGS globals              */
geInitHT();  					/* Stipples for h-tone fill  */
/*
 * Load default font now. Must be done after geGlobin is called, because
 * the font defaults are set up there.
 */
XrmGetResource (geRM.SysDb, "gra.fonts.default", NULL, &b, &v);
i = 0;
while (i < GE_MAX_FNTS && geFntPtr[i])
   {if (!strcmp(geFntPtr[i], v.addr)) break;
    else
  	{strcpy(geUtilBuf, v.addr);
  	 *geUtilBuf = (char)toupper(*geUtilBuf);
   	 if (!strcmp(geFntPtr[i], geUtilBuf)) break;
  	}		
    i++;
   }
if (geFntPtr[i]) geAttr.Font = i;

geFontUser[geAttr.Font] = XLoadQueryFont(geDispDev, geFntXName[geAttr.Font]);
geSampleAttr.Font = geAttr.Font;

/* Get screen resolution and compute units conversion factor.
 */
PixWidth = XDisplayWidth  (geDispDev, geScreen);
MMWidth  = XDisplayWidthMM(geDispDev, geScreen);
res      = (float)PixWidth / ((float)MMWidth / 25.4);
if (fabs(res - GEVSDPI) > GEMIN_DPI_DELTA)
  resfact  = (float)GEVSDPI / res;
else
  resfact  = 1.0;

/* Get the default unit and unit base from the resource file.
 */
XrmGetResource (geRM.SysDb, "gra.units.default", NULL, &b, &unit_def);
for (i=0, j=1; j < GE_MAX_UNITS; j++)
   {sprintf(unitbuf, "gra.unit_%d", j);
    XrmGetResource (geRM.SysDb, unitbuf, NULL, &b, &v);
    if (v.addr)
      {sscanf(v.addr, "%f %s", &geU.Con[i], unitbuf);
       if (geU.Con[i] != 1.0)
	geU.Con[i] = resfact / geU.Con[i];
       GEMSGALLOC(ptr, unitbuf);
       geU.Str[i] = ptr;
       geU.LMap   = i;
/*
 * Add the unit base type to the Unit ListBox.  Select and hilight
 * the pixels entry.
 */
       string = XmStringCreateSimple(geU.Str[i]);
       XmListAddItem(geUI.WidgetArray[GEWUNITSLIST], string, i+1);
       if (!strcmp(geU.Str[i], unit_def.addr))			
         geU.DMap = i;
       else
         {strcpy(geUtilBuf, unit_def.addr);
          *geUtilBuf = (char)toupper(*geUtilBuf);
          if (!strcmp(geU.Str[i], geUtilBuf))			
             geU.DMap = i;
         }
       i++;	
      }
   }

if (geRM.Units.manage_flag)
     geU.Map = geU.DMap;
else geU.Map = -1;

#endif

return;

}

