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
/* Resource and command-line junk
 *
 * DXkeycaps, Copyright (c) 1991 Jamie Zawinski <jwz@lucid.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

static char *DXkeycapsDefaults[] = {
  /*
   *  The default colors are specified rather completely here to avoid
   *  losing badly when the user specifies conflicting values for
   *  "*Foreground" and "*Background" in their personal resources.
   *  The downside of this is that it's more work for the user to change
   *  the colors of DXkeycaps.  If you want white-on-black, instead of
   *  the black-on-light-shades-of-gray default configuration, do
   *  something like
   *			DXkeycaps*Background: black
   *			DXkeycaps*Foreground: white
   *			DXkeycaps*BorderColor: black
   *
   *  "DXkeycaps -fg white -bg black -bd white" will do the same thing.
   *
   */
  "DXkeycaps*EditKey*title                      Edit Key",

  "*buttons.borderWidth:	0",
  "*info.borderWidth:		0",
  "*Keyboard.borderWidth:	0",
  "*Key.borderWidth:		1",
  
  "*info.labels.borderWidth:	 0",
  "*info.line.spacing:		 0",
  "*info.line.borderWidth:	 0",
  "*Label.borderWidth:		 0",

  "*EditKey*keysymLine.borderWidth:	0",

  "*EditKey*autoRepeatValue.borderWidth:	0",

  "*EditKey*XmList.listMarginWidth:	 1",
  "*EditKey*XmList.listSpacing:		 1",

  "*keyboardMenu*leftMargin:		15",
  "*keyboardMenu*rightMargin:		15",
  
  "*CommandsMenu.quit.labelString:		Exit",
  "*CommandsMenu.keyboard.labelString:		Keyboard",
  "*CommandsMenu.restore.labelString:		Reset to Default",
  "*CommandsMenu.write.labelString:		Save",  
  "*HelpMenu.overview.labelString:		Overview",

  "*keyMenu.editKeysyms.labelString:		Edit KeySyms of Key",
  "*keyMenu.swapKey.labelString:		Exchange Keys",
  "*keyMenu.cloneKey.labelString:		Duplicate Key",
  "*keyMenu.disableKey.labelString:		Disable Key",
  "*keyMenu.restoreKey.labelString:		Reset Key to Default",

  "*EditKey*keysym1.labelString:		KeySym 1",
  "*EditKey*keysym2.labelString:		KeySym 2",
  "*EditKey*keysym3.labelString:		KeySym 3",
  "*EditKey*keysym4.labelString:		KeySym 4",
  "*EditKey*keysym5.labelString:		KeySym 5",
  "*EditKey*keysym6.labelString:		KeySym 6",
  "*EditKey*keysym7.labelString:		KeySym 7",
  "*EditKey*keysym8.labelString:		KeySym 8",
  "*EditKey*autoRepeat.labelString:		Auto Repeat",
  "*EditKey*symsOfCode.labelString:		KeySyms of KeyCode",
  "*EditKey*modifiers.labelString:		Modifiers",
  "*EditKey*allKeySets.labelString:		Character Set",
  "*EditKey*keySymsOfSet.labelString:		KeySym",
  "*EditKey*modifierBox.modShift.labelString:	Shift",
  "*EditKey*modifierBox.modControl.labelString:Control",
  "*EditKey*modifierBox.modLock.labelString:	Lock",

  "*Save Session*messageString:		\\n\
All current bindings will be saved into the file ~/.dxkeycaps.\\n\
Add the line 'xmodmap ~/.dxkeycaps' to\\n\
.X11Startup and restart your session. \\n\
\\n\
Save the bindings?",

  "*Save Session*buttons*full.labelString:		All Keys",
  "*Save Session*partial.labelString:		Changed Keys",
  "*Save Session*abort.labelString:		Cancel",

  "*Restore Bindings*messageString:		\\n\
Restore Default Keymap?",

  "*Restore Bindings*yes.labelString:		Restore",
  "*Restore Bindings*no.labelString:		Cancel",

#ifdef JENSEN_INTERNAT  
  "*internatMenu*leftMargin:		15",
  "*internatMenu*rightMargin:		15",
  "*internatMenu*fontList:		*-helvetica-bold-r-*-*-*-100-*-*-*-*-*-*",
#endif 

  NULL
}; 



static XrmOptionDescRec options [] = {
  { "-foreground",	"*Foreground",			XrmoptionSepArg, 0 },
  { "-background",	"*Background",			XrmoptionSepArg, 0 },
  { "-fg",		"*Foreground",			XrmoptionSepArg, 0 },
  { "-bg",		"*Background",			XrmoptionSepArg, 0 },
  { "-gutterwidth",	"*Keyboard.Key.gutterWidth",	XrmoptionSepArg, 0 },
  { "-gw",		"*Keyboard.Key.gutterWidth",	XrmoptionSepArg, 0 },
  { "-font",		"*Keyboard.Key.keycapFont",	XrmoptionSepArg, 0 },
  { "-fn",		"*Keyboard.Key.keycapFont",	XrmoptionSepArg, 0 },
  { "-keyboard",	"*Keyboard.keyboard",		XrmoptionSepArg, 0 },
  { "-kbd",		"*Keyboard.keyboard",		XrmoptionSepArg, 0 }
};






/*
  "*Save Session*label.fontList:	*-helvetica-bold-r-*-*-*-140-*-*-*-*-*-*",
  "*Restore Bindings*label.fontList:	*-helvetica-bold-r-*-*-*-140-*-*-*-*-*-*",

  "*EditKey*buttons*fontList:	*-helvetica-bold-r-*-*-*-140-*-*-*-*-*-*",
  "*EditKey*label.fontList:		*-helvetica-bold-r-*-*-*-140-*-*-*-*-*-*",
  "*EditKey*XmLabel.fontList:		*-helvetica-bold-r-*-*-*-100-*-*-*-*-*-*",
  "*EditKey*keysets.fontList:		*-helvetica-medium-r-*-*-*-120-*-*-*-*-*-*",
  "*EditKey*autoRepeatValue.fontList:*-helvetica-medium-r-*-*-*-100-*-*-*-*-*-*",

  "*labels.Label.fontList:		*-helvetica-bold-r-*-*-*-100-*-*-*-*-*-*",
  "*Label.fontList: 		*-courier-medium-r-*-*-*-120-*-*-*-*-*-*",
  "*info*message.fontList:		*-helvetica-medium-r-*-*-*-100-*-*-*-*-*-*",
  "*info*message2.fontList:		*-helvetica-medium-r-*-*-*-100-*-*-*-*-*-*",
  "*keyMenu*fontList:		*-helvetica-bold-r-*-*-*-100-*-*-*-*-*-*",
  "*keyMenu.menuLabel.fontList:	*-helvetica-bold-o-*-*-*-120-*-*-*-*-*-*",
  "*modifiers*label.fontList:	*-helvetica-bold-r-*-*-*-140-*-*-*-*-*-*",
  "*modifiers*XmCommand.fontList:	*-helvetica-bold-r-*-*-*-140-*-*-*-*-*-*",
  "*buttons.XmPushButton.fontList:	*-helvetica-bold-r-*-*-*-100-*-*-*-*-*-*",

  "DXkeycaps*EditKey*label.foreground:		gray93",
  "DXkeycaps*EditKey*label.background:		black",
  "DXkeycaps*EditKey*Toggle.background:		gray88",
  "DXkeycaps*EditKey*Viewport*background:	gray93",
  "DXkeycaps*EditKey*autoRepeatValue.background:	gray88",

  "DXkeycaps*Foreground:				black",
  "DXkeycaps*borderColor:			black",
  "DXkeycaps*background: 			gray88",
  "DXkeycaps*Command.background:			gray93",
  "DXkeycaps*MenuButton.background:		gray93",
  "DXkeycaps*Toggle.background:			gray93",
  "DXkeycaps*Key.background:			gray93",
  "DXkeycaps*Key.highlight:			white",
*/
