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
/*  DEC/CMS REPLACEMENT HISTORY, Element TRANSLATE.H */
/*  *10   28-OCT-1988 16:54:39 LATHAM "add error message for bitonal image" */
/*  *9    20-OCT-1988 13:22:36 LATHAM "change some messages" */
/*  *8    12-AUG-1988 10:44:53 LATHAM "fix for FT2" */
/*  *7     1-JUL-1988 14:53:44 LATHAM "All set for FT2" */
/*  *6     9-JUN-1988 14:18:07 LATHAM "update elements" */
/*  *5    12-APR-1988 13:45:57 LATHAM "Check in for BL7.6" */
/*  *4    27-FEB-1988 20:36:50 GEORGE "Add ALL RIGHTS RESERVED" */
/*  *3     8-JAN-1988 09:50:11 LATHAM "FT1 code freeze" */
/*  *2    25-NOV-1987 21:57:48 LATHAM "Runs on BL5" */
/*  *1     3-NOV-1987 17:07:20 LATHAM "Initial entry" */
/*  DEC/CMS REPLACEMENT HISTORY, Element TRANSLATE.H */
/*
#module TRANSLATE "V1-000"
****************************************************************************
**                                                                          *
**  Copyright (c) 1987                                                      *
**  By DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                        *
**  All Rights Reserved                                                     *
**                                                                          *
**  This software is furnished under a license and may be used and  copied  *
**  only  in  accordance  with  the  terms  of  such  license and with the  *
**  inclusion of the above copyright notice.  This software or  any  other  *
**  copies  thereof may not be provided or otherwise made available to any  *
**  other person.  No title to and ownership of  the  software  is  hereby  *
**  transferred.                                                            *
**                                                                          *
**  The information in this software is subject to change  without  notice  *
**  and  should  not  be  construed  as  a commitment by DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL assumes no responsibility for the use or  reliability  of  its  *
**  software on equipment which is not supplied by DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**   DECpaint - VMS DECwindows paint program
**
**  AUTHOR
**
**   Daniel Latham, October 1987
**
**  ABSTRACT:
**
**   This module contains imbedded text which should be translated when
**   internationalization occurs.  
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**	dl	10/5/88
**	Add messages to complete shapes.
**
**	dl	10/27/88
**	Add file read error messages.
**--
**/           
/* Title of application */
#define T_DECPAINT "Paint"

/* Text found in menus */
#define T_FILE "File"
#define T_ZOOM "Zoom"
#define T_OK "OK"
#define T_CANCEL "Cancel"
#define T_YES "Yes"
#define T_NO "No"
#define T_APPLY "Apply"
#define T_DISMISS "Dismiss"
#define T_NONE "None"

/* Found in file menu */
#define T_OPEN "Open..."
#define T_CLOSE "Close"
#define T_SAVE "Save"
#define T_SAVE_AS "Save As..."
#define T_PRINT "Print"
#define T_PRINT_AS "Print..."
#define T_QUIT "Quit"
#define T_EXIT "Exit"

/* Edit menu text */
#define T_EDIT "Edit"
#define T_UNDO "Undo"
#define T_CUT "Cut"
#define T_COPY "Copy"
#define T_PASTE "Paste"
#define T_CLEAR "Clear"
#define T_INVERT "Invert"
#define T_SCALE "Scale..."
#define T_CROP "Crop"
#define T_SELECT_ALL "Select All"

#define T_SCALE_TITLE "Scale"
#define T_FLIP_X "Flip X-axis"
#define T_FLIP_Y "Flip Y-axis"
#define T_ROTATE_LEFT "Rotate Left"
#define T_ROTATE_RIGHT "Rotate Right"

/* Options menu text */
#define T_OPTIONS "Options"
#define T_LINE_ATTR "Line Attributes..."
#define T_BRUSH_SHAPE "Brush Shape..."
#define T_LINE_WIDTH "Line Width..."
#define T_PATTERNS "Patterns..."
#define T_EDIT_PATTERN "Edit Pattern..."
#define T_OPAQUE "Opaque"
#define T_TRANSPARENT "Transparent"
#define T_GRID_ON "Grid On"
#define T_GRID_OFF "Grid Off"
#define T_ZOOM_ON "Zoom On"
#define T_ZOOM_OFF "Zoom Off"

/* Font menu */
#define T_FONT "Font"
#define T_COURIER "Courier"
#define T_TIMES "Times"
#define T_HELVETICA "Helvetica"
#define T_NORMAL "Normal"
#define T_BOLD "Bold"
#define T_ITALIC "Italic"
#define T_10 "10"
#define T_12 "12"
#define T_14 "14"
#define T_18 "18"
#define T_24 "24"
#define T_FAMILY "Family"
#define T_SIZE "Size"
#define T_STYLE "Style"

/* Customize menu text */
#define T_CUSTOMIZE "Customize"
#define T_PIC_SIZE "Picture Size..."
#define T_GRID_SIZE "Grid Size"

/* Help menu text */
#define T_HELP "Help"
#define T_OVERVIEW "Overview"
#define T_ABOUT "About"

/* Actions text */
#define UNDO_ACTION_LENGTH 40	/* used as buffer length for combined strings*/
#define T_UNDO_ACTION "Undo "	/* undo + space */
#define T_REDO_ACTION "Redo "	/* redo + space */
#define T_LINE "Line"
#define T_RECTANGLE   "Rectangle"
#define T_ELLIPSE     "Ellipse"
#define T_STROKE      "Stroke"
#define T_ARC         "Arc"
#define T_POLYLINE    "Polyline"
#define T_BRUSH       "Brush"
#define T_ERASE       "Erase"
#define T_PENCIL      "Pencil"
#define T_SELECT      "Select"
#define T_MOVE        "Move"
#define T_FLOOD       "Flood"
#define T_TEXT        "Text"
#define T_SPRAY       "Spray"
#define T_CIRCLE      "Circle"
#define T_SQUARE      "Square"
#define T_CUT         "Cut"
#define T_COPY        "Copy"
#define T_PASTE       "Paste"

#define T_ASPECT "Aspect Ratio"
#define T_1_TO_1 "1:1"
#define T_2_TO_1 "2:1"
#define T_BSHAPES "Brush Shapes"
#define T_HEIGHT_LABEL "Height: "
#define T_WIDTH_LABEL "Width: "
#define T_PAGE "Letter"
#define T_SCREEN "Screen"
#define T_SIZE "Size"
#define T_NON_STANDARD "Non-Standard"
#define T_PSIZE_TITLE "Picture Size"

/* Miscellaneous */
#define T_NO_EDIT_PATTERN "The current pattern cannot be edited."          
#define T_EXIT_MSG "Do you want to save the picture before exiting?"
#define T_SCALE_BY "Scale By:"
#define T_HELP_MSG  "DECpaint is a bitmap-oriented graphics drawing program\nthat allows you to create pictures. The pictures\nare saved as DDIF files."
#define T_CROP_TITLE "Crop"
#define T_CROP_MSG "Draw a rectangle to specify the new size or\nenter the width and height in pixels."
#define T_COPYRIGHT "Digital Equipment Corporation.  1988.  All Rights Reserved."
#define T_PICTURE_NAME "Picture name:"
#define T_DDIF "DDIF"
#define T_X11 "X11"
#define T_SAVE_PICTURE "Save As"
#define T_OPEN_PICTURE "Open"
#define T_PIXELS "Pixels"
#define T_INCHES "Inches"
#define T_CENTIMETERS "Centimeters"
#define T_PATTERNS_TITLE "Patterns"

/* Error strings */
#define T_ERROR_STRING "Paint Error: %s\n"
#define DISPLAY_NOT_OPENED "Error: Display could not be opened\n"
#define T_FILE_READ_ERROR "The picture cannot be opened.\nCheck the directory and file name."
#define T_ILLEGAL_FILE_FORMAT "The picture's file format is not supported.\nOnly DDIF images and X bitmaps are permissible."
#define T_FILE_NOT_BITONAL "The picture cannot be opened.\nOnly bitonal images can be edited."
#define T_FILE_WRITE_ERROR "The picture cannot be saved.\nCheck the directory and file name."
#define T_PICTURE_SCALED_DOWN "In order to display the picture, it has been\nscaled down.  All changes will be done\nat screen resolution."
#define T_SCALE_UP_ERROR "The scale factor is too large.\nThe enlarged selected area will not fit in the picture."
#define T_INVALID_SIZE "The requested picture size is invalid."
#define T_FINISH_POLYGON "The polyline must be ended before choosing another shape.\nTo end a polyline, double click at the last endpoint\nor single click near the first point of the polyline."
#define T_FINISH_SHAPE "The current action must be completed before you can choose another shape."
#define T_NO_FILL_PATTERN "The fill pattern is off and no drawing will occur.\nSelect a fill pattern from the patterns menu."
