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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: WmResParse.c,v $ $Revision: 1.1.4.6 $ $Date: 1993/08/26 20:51:03 $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */
/*
 * (c) Copyright 1987, 1988 DIGITAL EQUIPMENT CORPORATION */
/*
 * (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY */

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmResNames.h"
#include "WmResource.h"

#include <Xm/VirtKeysP.h>

#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <ctype.h>

#ifdef MOTIF_ONE_DOT_ONE
#include <stdio.h>
#ifndef VMS
#include <pwd.h>
#endif
#else
#include <Xm/XmosP.h>
#endif

/* maximum string lengths */

#define MAX_KEYSYM_STRLEN    100
#define MAX_EVENTTYPE_STRLEN  20
#define MAX_MODIFIER_STRLEN   20
#define MAX_CONTEXT_STRLEN    20
#define MAX_GROUP_STRLEN      20

#define min(a,b)	((a)>(b) ? (b) : (a))

#define MAXLINE     4096
#define MAXPATH     1023

#define MBBSIZ	    4096

/*
 * include extern functions
 */
#ifdef DEC_MOTIF_BUG_FIX
#include "WmResParse.h"
#include "WmInitWs.h"
#endif
#include "WmError.h"
#include "WmFunction.h"
#include "WmImage.h"

#ifdef MOTIF_ONE_DOT_ONE
extern char   *getenv ();
#endif

/*
 * Global Variables And Tables:
 */
static FILE *cfileP = NULL;   /* fopen'ed configuration file or NULL */
static char cfileName[MAXPATH+1];
static unsigned char  line[MAXLINE+1]; /* line buffer */
static int   linec = 0;       /* line counter for parser */
static unsigned char *parseP = NULL;   /* pointer to parse string */


typedef struct {
   char         *name;
   unsigned int  mask;
} MaskTableEntry;

static MaskTableEntry modifierStrings[] = {

    {"none",    None},
    {"ctrl",	ControlMask},
    {"shift",	ShiftMask},
    {"alt",	Mod1Mask},
    {"meta",	Mod1Mask},
    {"lock",	LockMask},
    {"mod1",	Mod1Mask},
    {"mod2",	Mod2Mask},
    {"mod3",	Mod3Mask},
    {"mod4",	Mod4Mask},
    {"mod5",	Mod5Mask},
    {NULL,      (unsigned int)NULL},
};

#define ALT_INDEX 3
#define META_INDEX 4

typedef struct {
   char         *event;
   unsigned int  eventType;
   Boolean       (*parseProc)();
   unsigned int  closure;
   Boolean       fClick;
} EventTableEntry;

#ifdef _NO_PROTO
#ifdef MOTIF_ONE_DOT_ONE
void GetHomeDirName();
#endif
static Boolean   LookupModifier ();
static void      ParseButtonSet ();
static Boolean   ParseContext();
static Boolean   ParseEvent ();
static Boolean   ParseEventType ();
static Boolean   ParseImmed ();
static void      ParseKeySet ();
static Boolean   ParseKeySym ();
static MenuItem *ParseMenuItems ();
static void      ParseMenuSet ();
static Boolean   ParseModifiers ();
static void      ParseScreensArgument ();
static Boolean   ParseWmAccelerator ();
static Boolean   ParseWmFuncGrpArg ();
static Boolean   ParseWmFuncMaybeStrArg ();
static Boolean   ParseWmFuncNbrArg ();
static Boolean   ParseWmFuncNoArg ();
static Boolean   ParseWmFuncStrArg ();
static Boolean   ParseWmLabel ();
static void      ParseWmMnemonic ();
static void      ProcessAccelText ();
static unsigned int StrToHex ();
static unsigned int StrToNum ();
static unsigned int StrToOct ();
#else /* _NO_PROTO */
#ifdef MOTIF_ONE_DOT_ONE
void GetHomeDirName(String  fileName);
#endif
void ProcessWmFile (WmScreenData *pSD);
FILE *FopenConfigFile (void);
void SaveMenuAccelerators (WmScreenData *pSD, MenuSpec *newMenuSpec);
static void ParseMenuSet (WmScreenData *pSD, unsigned char *lineP);
MenuItem *ParseMwmMenuStr (WmScreenData *pSD, unsigned char *menuStr);
static MenuItem *ParseMenuItems (WmScreenData *pSD);
static Boolean ParseWmLabel (WmScreenData *pSD, MenuItem *menuItem, 
			     unsigned char *string);
static void ParseWmMnemonic (unsigned char **linePP, MenuItem *menuItem);
static Boolean ParseWmAccelerator (unsigned char **linePP, MenuItem *menuItem);
int ParseWmFunction (unsigned char **linePP, unsigned int res_spec, 
			    WmFunction *pWmFunction);
static Boolean ParseWmFuncMaybeStrArg (unsigned char **linePP, 
				       WmFunction wmFunction, String *pArgs);
static Boolean ParseWmFuncNoArg (unsigned char **linePP, WmFunction wmFunction,
				 String *pArgs);
static Boolean ParseWmFuncStrArg (unsigned char **linePP, 
				  WmFunction wmFunction, String *pArgs);
void FreeMenuItem (MenuItem *menuItem);
static Boolean ParseWmFuncGrpArg (unsigned char **linePP, 
				  WmFunction wmFunction, GroupArg *pGroup);
static Boolean ParseWmFuncNbrArg (unsigned char **linePP, 
				  WmFunction wmFunction, 
				  unsigned long *pNumber);
void ParseButtonStr (WmScreenData *pSD, unsigned char *buttonStr);
static void ParseButtonSet (WmScreenData *pSD, unsigned char *lineP);
static Boolean ParseContext (unsigned char **linePP, Context *context, 
			     Context *subContext);
void
ParseKeyStr (WmScreenData *pSD, unsigned char *keyStr);
static void ParseKeySet (WmScreenData *pSD, unsigned char *lineP);
unsigned char *
GetNextLine (void);
unsigned char *GetString (unsigned char **linePP);
Boolean ParseBtnEvent (unsigned char  **linePP,
		       unsigned int *eventType,
		       unsigned int *button,
		       unsigned int *state,
		       Boolean      *fClick);
Boolean ParseKeyEvent (unsigned char **linePP, unsigned int *eventType,
		       KeyCode *keyCode,  unsigned int *state);
static Boolean ParseEvent (unsigned char **linePP, EventTableEntry *table,
			   unsigned int *eventType, unsigned int *detail,
			   unsigned int *state, Boolean *fClick);
static Boolean ParseModifiers(unsigned char **linePP, unsigned int *state);
static Boolean LookupModifier (unsigned char *name, unsigned int *valueP);
static Boolean ParseEventType (unsigned char **linePP, EventTableEntry *table,
			       unsigned int *eventType, Cardinal *ix);
static Boolean ParseImmed (unsigned char **linePP, unsigned int closure,
			   unsigned int  *detail);
static Boolean ParseKeySym (unsigned char **linePP, unsigned int closure,
			    unsigned int *detail);
static unsigned int StrToNum(unsigned char *str);
static unsigned int StrToHex(unsigned char *str);
static unsigned int StrToOct(unsigned char *str);
void ScanAlphanumeric (unsigned char **linePP);
void ScanWhitespace(unsigned char  **linePP);
void ToLower (unsigned char  *string);
void
PWarning (char *message);
static void ProcessAccelText (unsigned char *startP, unsigned char *endP,
			      unsigned char *destP);
void ProcessCommandLine (int argc,  char *argv[]);
static void ParseScreensArgument (int argc, char *argv[], int *pArgnum,
				  unsigned char *lineP);
void ProcessMotifBindings (void);
#endif /* _NO_PROTO */

static EventTableEntry buttonEvents[] = {

    {"btn1down",    ButtonPress,    ParseImmed,    SELECT_BUTTON,  FALSE},
    {"btn1up",      ButtonRelease,  ParseImmed,    SELECT_BUTTON,  FALSE},
    {"btn1click",   ButtonRelease,  ParseImmed,    SELECT_BUTTON,  TRUE},
    {"btn1click2",  ButtonPress,    ParseImmed,    SELECT_BUTTON,  TRUE},
    {"btn2down",    ButtonPress,    ParseImmed,    DMANIP_BUTTON,  FALSE},
    {"btn2up",      ButtonRelease,  ParseImmed,    DMANIP_BUTTON,  FALSE},
    {"btn2click",   ButtonRelease,  ParseImmed,    DMANIP_BUTTON,  TRUE},
    {"btn2click2",  ButtonPress,    ParseImmed,    DMANIP_BUTTON,  TRUE},
    {"btn3down",    ButtonPress,    ParseImmed,    BMENU_BUTTON,  FALSE},
    {"btn3up",      ButtonRelease,  ParseImmed,    BMENU_BUTTON,  FALSE},
    {"btn3click",   ButtonRelease,  ParseImmed,    BMENU_BUTTON,  TRUE},
    {"btn3click2",  ButtonPress,    ParseImmed,    BMENU_BUTTON,  TRUE},
    {"btn4down",    ButtonPress,    ParseImmed,    Button4,  FALSE},
    {"btn4up",      ButtonRelease,  ParseImmed,    Button4,  FALSE},
    {"btn4click",   ButtonRelease,  ParseImmed,    Button4,  TRUE},
    {"btn4click2",  ButtonPress,    ParseImmed,    Button4,  TRUE},
    {"btn5down",    ButtonPress,    ParseImmed,    Button5,  FALSE},
    {"btn5up",      ButtonRelease,  ParseImmed,    Button5,  FALSE},
    {"btn5click",   ButtonRelease,  ParseImmed,    Button5,  TRUE},
    {"btn5click2",  ButtonPress,    ParseImmed,    Button5,  TRUE},
    { NULL, (unsigned int)NULL, (Boolean(*)())NULL, (unsigned int)NULL, FALSE}
};


static EventTableEntry keyEvents[] = {

    {"key",         KeyPress,    ParseKeySym,    0,  FALSE},
    { NULL, (unsigned int)NULL, (Boolean(*)())NULL, (unsigned int)NULL, FALSE}
};


unsigned int buttonModifierMasks[] = {
    0,
    SELECT_BUTTON_MASK,
    DMANIP_BUTTON_MASK,
    BMENU_BUTTON_MASK,
    Button4Mask,
    Button5Mask
};

/*
 * FUNCTION PARSER TABLE (function names must be in alphabetic order)
 */

typedef struct {
   char         * funcName;
   Context        greyedContext;
   unsigned int   resource;
   long           mgtMask;
   WmFunction     wmFunction;
   Boolean       (*parseProc)();
} FunctionTableEntry;


/*
 * NOTE: New functions MUST be added in ALPHABETICAL order.  A binary search
 *       is used to find the correct function name.
 */

FunctionTableEntry functionTable[] = {
    {"f.beep",		0,
			CRS_ANY,
			0,
			F_Beep,
			ParseWmFuncNoArg},
    {"f.circle_down",	F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Circle_Down,
			ParseWmFuncGrpArg},
    {"f.circle_up",	F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Circle_Up,
			ParseWmFuncGrpArg},
#ifdef DEC_MOTIF_EXTENSION
    {"f.dec_customize", 0,
                        CRS_ANY,
                        0,
	                F_DEC_Customize,
			ParseWmFuncStrArg},
    {"f.dec_help",      0,
                        CRS_ANY,
                        0,
                        F_DEC_Help,
			ParseWmFuncStrArg},
#endif
    {"f.exec",		0,
			CRS_ANY,
			0,
			F_Exec,
			ParseWmFuncStrArg},
    {"f.focus_color",	F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Focus_Color,
			ParseWmFuncNoArg},
    {"f.focus_key",	F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Focus_Key,
			ParseWmFuncNoArg},
    {"f.kill",		F_CONTEXT_ROOT,
			CRS_ANY,
			MWM_FUNC_CLOSE,
			F_Kill,
			ParseWmFuncNoArg},
    {"f.lower",		F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Lower,
                        ParseWmFuncMaybeStrArg},
    {"f.maximize",	F_CONTEXT_ROOT|F_CONTEXT_MAXIMIZE|
	                               F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			MWM_FUNC_MAXIMIZE,
			F_Maximize,
			ParseWmFuncNoArg},
    {"f.menu",		0,
			CRS_ANY,
			0,
			F_Menu,
			ParseWmFuncStrArg},
    {"f.minimize",	F_CONTEXT_ICON|F_CONTEXT_ROOT|F_SUBCONTEXT_IB_IICON,
			CRS_ANY,
			MWM_FUNC_MINIMIZE,
			F_Minimize,
			ParseWmFuncNoArg},
    {"f.move",		F_CONTEXT_ROOT,
			CRS_ANY,
			MWM_FUNC_MOVE,
			F_Move,
			ParseWmFuncNoArg},
    {"f.next_cmap",	0,
			CRS_ANY,
			0,
			F_Next_Cmap,
			ParseWmFuncNoArg},
    {"f.next_key",	0,
			CRS_ANY,
			0,
			F_Next_Key,
			ParseWmFuncGrpArg},
    {"f.nop",	        F_CONTEXT_ROOT|F_CONTEXT_ICON|F_CONTEXT_WINDOW|
	                    F_SUBCONTEXT_IB_WICON | F_SUBCONTEXT_IB_IICON,
			CRS_ANY,
			0,
			F_Nop,
			ParseWmFuncNoArg},
    {"f.normalize",	F_CONTEXT_ROOT|F_CONTEXT_NORMAL|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Normalize,
			ParseWmFuncNoArg},
    {"f.normalize_and_raise",
	                F_CONTEXT_ROOT|F_CONTEXT_NORMAL,
			CRS_ANY,
			0,
			F_Normalize_And_Raise,
			ParseWmFuncNoArg},
    {"f.pack_icons",	0,
			CRS_ANY,
			0,
			F_Pack_Icons,
			ParseWmFuncNoArg},
    {"f.pass_keys",	0,
			CRS_ANY,
			0,
			F_Pass_Key,
			ParseWmFuncNoArg},
    {"f.post_wmenu",	0,
			CRS_BUTTON|CRS_KEY,
			0,
			F_Post_SMenu,
			ParseWmFuncNoArg},
    {"f.prev_cmap",	0,
			CRS_ANY,
			0,
			F_Prev_Cmap,
			ParseWmFuncNoArg},
    {"f.prev_key",	0,
			CRS_ANY,
			0,
			F_Prev_Key,
			ParseWmFuncGrpArg},
#ifdef DEC_MOTIF_EXTENSION
    {"f.quit_mwm",      0,
#else
    {"f.quit_mwm",	F_CONTEXT_ICON|F_CONTEXT_WINDOW,
#endif
			CRS_ANY,
			0,
			F_Quit_Mwm,
			ParseWmFuncNoArg},
    {"f.raise",		F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Raise,
                        ParseWmFuncMaybeStrArg},
    {"f.raise_lower",	F_CONTEXT_ROOT |
    				F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Raise_Lower,
			ParseWmFuncNoArg},
    {"f.refresh",	0,
			CRS_ANY,
			0,
			F_Refresh,
			ParseWmFuncNoArg},
    {"f.refresh_win",	F_CONTEXT_ICON|F_CONTEXT_ROOT,
			CRS_ANY,
			0,
			F_Refresh_Win,
			ParseWmFuncNoArg},
    {"f.resize",	F_CONTEXT_ICON|F_CONTEXT_ROOT|
                                 F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			MWM_FUNC_RESIZE,
			F_Resize,
			ParseWmFuncNoArg},
#ifdef DEC_MOTIF_EXTENSION
    {"f.restart",	0,
#else
    {"f.restart",	F_CONTEXT_ICON|F_CONTEXT_WINDOW,
#endif
			CRS_ANY,
			0,
			F_Restart,
			ParseWmFuncNoArg},
    {"f.restore",	F_CONTEXT_ROOT|F_CONTEXT_NORMAL|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Restore,
			ParseWmFuncNoArg},
    {"f.restore_and_raise",
	                F_CONTEXT_ROOT|F_CONTEXT_NORMAL,
			CRS_ANY,
			0,
			F_Restore_And_Raise,
			ParseWmFuncNoArg},
    {"f.screen",	0,
			CRS_ANY,
			0,
			F_Screen,
			ParseWmFuncStrArg},
    {"f.send_msg",	F_CONTEXT_ROOT,
			CRS_ANY,
			0,
			F_Send_Msg,
			ParseWmFuncNbrArg},
    {"f.separator",	0,
			CRS_MENU,
			0,
			F_Separator,
			ParseWmFuncNoArg},
    {"f.set_behavior",	0,
			CRS_ANY,
			0,
			F_Set_Behavior,
			ParseWmFuncNoArg},
    {"f.title",		0,
			CRS_MENU,
			0,
			F_Title,
			ParseWmFuncNoArg},
};

/*
 * NOTE: New functions MUST be added in ALPHABETICAL order.  A binary search
 *       is used to find the correct function name.
 */

#define WMFUNCTIONTABLESIZE (sizeof(functionTable)/sizeof(functionTable[0]))

/*
 * Be sure to update these define, whenever adding/deleting a function.
 */
#ifdef DEC_MOTIF_EXTENSION
#define F_DEC_CUST_INDEX 3
#define F_DEC_HELP_INDEX 4
#define F_EXEC_INDEX 5
#define F_NOP_INDEX 16
#else
#define F_EXEC_INDEX 3          
#define F_NOP_INDEX 14
#endif /* DEC_MOTIF_EXTENSION */


#ifdef MOTIF_ONE_DOT_ONE
/*************************************<->*************************************
 *
 *  GetHomeDirName (fileName)
 *
 *  Description:
 *  -----------
 *  This function finds the "HOME" directory
 *
 *
 *  Inputs:
 *  ------
 *  fileName 
 *
 *  Outputs:
 *  -------
 *  fileName
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void GetHomeDirName(fileName)
String  fileName;
#else /* _NO_PROTO */
void GetHomeDirName(String  fileName)
#endif /* _NO_PROTO */
{
        int uid;
#ifndef VMS
        struct passwd *pw;
#endif
        char *ptr = NULL;

        if((ptr = getenv("HOME")) == NULL)
        {
#ifndef VMS
            if((ptr = getenv("USER")) != NULL)
	    {
		pw = getpwnam(ptr);
	    }
            else
            {
                uid = getuid();
                pw = getpwuid(uid);
            }

            if (pw)
	    {
                ptr = pw->pw_dir;
	    }
            else
	    {
                ptr = "";
	    }
#endif
        }
        strcpy(fileName, ptr);
}
#endif


/*************************************<->*************************************
 *
 *  SyncModifierStrings (fileName)
 *
 *  Description:
 *  -----------
 *  This function updates modifierStrings table so that Mwm uses the correct
 *  modifier to keysym mapping.  Specifically, fix up the Alt and Meta bindings.
 *
 *  Inputs:
 *  ------
 *  fileName 
 *
 *  Outputs:
 *  -------
 *  fileName
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void SyncModifierStrings()
#else /* _NO_PROTO */
void SyncModifierStrings(void)
#endif /* _NO_PROTO */
{
    XModifierKeymap *map;
    int i, j, k = 0;

    map = XGetModifierMapping (DISPLAY);

    for (i = 0; i < 8; i++)
    {
	for (j = 0; j < map->max_keypermod; j++)
	{
	    if (map->modifiermap[k])
	    {
		KeySym ks = XKeycodeToKeysym(DISPLAY, map->modifiermap[k], 0);
		char *nm = XKeysymToString(ks);

		/* Compare, ignoring the trailing '_L' or '_R' in keysym */
		if (nm && !strncmp("Alt", nm, 3))
		{
		    modifierStrings[ALT_INDEX].mask = (1<<i);
		}
		else if (nm && !strncmp("Meta", nm, 4))
		{
		    modifierStrings[META_INDEX].mask = (1<<i);
		}
	    }
	    k++;
	}
    }
}



/*************************************<->*************************************
 *
 *  ProcessWmFile ()
 *
 *
 *  Description:
 *  -----------
 *  This function reads the mwm resource description file and processes the
 *  resources that are described.
 *
 *
 *  Inputs:
 *  ------
 *  wmGD.bitmapDirectory = bitmapDirectory resource value
 *  pSD->buttonBindings = buttonBindings resource value
 *  wmGD.configFile = configuration file resource value
 *  pSD->keyBindings = keyBindings resource value
 *  wmGD.rootWindow = default root window of display
 *  HOME = environment variable for home directory
 *  functionTable = window manager function parse table
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD.buttonSpecs = list of button binding specifications
 *  wmGD.keySpecs = list of key binding specification
 *  wmGD.menuSpecs = list of menu specifications
 * *wmGD.acceleratorMenuSpecs = initialized array of (MenuSpec *)
 *  wmGD.acceleratorMenuCount = 0
 *
 *
 *  Comments:
 *  --------
 * If there are more than MAXLINE characters on a line the excess characters
 * are truncated.
 * 
 *************************************<->***********************************/
#define MENU_SPEC	"menu"
#define BUTTON_SPEC	"buttons"
#define KEY_SPEC	"keys"

#ifdef _NO_PROTO
void ProcessWmFile (pSD)

    WmScreenData *pSD;

#else /* _NO_PROTO */
void ProcessWmFile (WmScreenData *pSD)
#endif /* _NO_PROTO */
{
    unsigned char *lineP;
    unsigned char *string;
    unsigned int   n;
    MenuSpec      *menuSpec;

    /*
     * Initialize global data values that are set based on data in
     * the mwm resource description file.
     */

    pSD->buttonSpecs = NULL;
    pSD->keySpecs = NULL;
    pSD->menuSpecs = NULL;

    /*
     * Find and parse the default system menu string, if it exists.
     */

    cfileP = NULL;
    linec = 0;
    if (((parseP = (unsigned char *) builtinSystemMenu) != NULL) && 
	 (GetNextLine () != NULL))
    {
	lineP = line;
        ParseMenuSet (pSD, lineP);
    }

    linec = 0;
    if (((parseP = (unsigned char *) builtinRootMenu) != NULL) && 
	 (GetNextLine () != NULL))
    {
	lineP = line;
        ParseMenuSet (pSD, lineP);
    }

    /*
     * Find and associate a stream with the window manager resource 
     *   description file.
     */

    if ((cfileP = FopenConfigFile ()) == NULL)
    {
	if (!wmGD.useStandardBehavior)
	  Warning ("Cannot open configuration file");
	return;
    }

    /*
     * Parse the information in the configuration file.
     * If there are more than MAXLINE characters on a line the excess are
     *   truncated.
     */

    linec = 0;
    while ((GetNextLine () != NULL))  /* not EOF nor read error */
    {
	lineP = line;
	if ((*line == '!') || (string = GetString (&lineP)) == NULL)
	/* empty or comment line */
	{
	    continue;
	}

#ifndef VMS
        ToLower (string);
#else
        ToLower2 (string);
#endif
	if (!strcmp ((char *)string, MENU_SPEC))
	{
	    ParseMenuSet (pSD, lineP);
	}
	else if (!strcmp ((char *) string, BUTTON_SPEC))
	{
	    ParseButtonSet (pSD, lineP);
	}
	else if (!strcmp ((char *) string, KEY_SPEC))
	{
	    ParseKeySet (pSD, lineP);
	}
    }

    fclose (cfileP);

    /*
     * Create and initialize the pSD->acceleratorMenuSpecs array.
     * This assumes we create pointers to MenuSpecs within ProcessWmFile().
     * Set pSD->acceleratorMenuCount to 0.
     */

    /* count the number of menu specifications */
    n = 0;
    menuSpec = pSD->menuSpecs;
    while (menuSpec)
    {
	n++;
	menuSpec = menuSpec->nextMenuSpec;
    }

    /* allocate the array and initialize to zeros */
    pSD->acceleratorMenuSpecs = NULL;
    if (n)
    {
        pSD->acceleratorMenuSpecs = 
	    (MenuSpec **) XtCalloc (n, sizeof (MenuSpec *));
        if (pSD->acceleratorMenuSpecs == NULL)
        {
            Warning ("Insufficient memory for menu accelerators");
        }
    }
    pSD->acceleratorMenuCount = 0;
} /* END OF FUNCTION ProcessWmFile */


#ifdef DEC_MOTIF_EXTENSION
/*************************************<->*************************************
 *
 *  WmConfigFileChec
 *
 *
 *  Description:
 *  -----------
 *  This tests if the config file exists.
 *  If it exists and was not found, report a warning.
 *
 *  Inputs:
 *  ------
 *  new config file
 * 
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void WmConfigFileChec( fileName )

char *fileName;

#else /* _NO_PROTO */
void WmConfigFileChec( char *fileName )
#endif /* _NO_PROTO */
{

/********************************/

  if ((wmGD.configFile != (String)NULL) && (strlen( wmGD.configFile ) != 0 ))
    {
      fprintf( stderr, "The config file defined as %s was not found.\n %s was used instead.\n",    
               wmGD.configFile, fileName );
#ifndef VMS
      fprintf( stderr, "\nIf a non-default language is set,\n\
place the file in a subdirectory that matches the \n\
*xnlLanguage resource value in ~/.Xdefaults.\n\
For example, for French the config file should be placed in\n\
~/fr_FR.88591.\n" );
      fprintf( stderr, "Also, check the environment variable, XAPPLRESDIR.\n" );
#endif
    }

}

#endif
/*************************************<->*************************************
 *
 *  FopenConfigFile ()
 *
 *
 *  Description:
 *  -----------
 *  This function searches for, opens, and associates a stream with the mwm 
 *  resource description file,
 *
 *
 *  Inputs:
 *  ------
 *  wmGD.configFile = configuration file resource value.
 *  HOME = environment variable for home directory
 *
 *
 *  Outputs:
 *  -------
 *  Return = If successful, a pointer to the FILE structure associated with 
 *           the configuration file.  Otherwise, NULL.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
FILE *FopenConfigFile ()
#else /* _NO_PROTO */ 
FILE *FopenConfigFile (void)
#endif /* _NO_PROTO */
{
    char    *LANG, *LANGp;
#ifdef DEC_MOTIF_BUG_FIX
char *path, *res_path;
FILE *fileP = NULL;
#else
    FILE *fileP;
#ifndef MOTIF_ONE_DOT_ONE
    char *homeDir = _XmOSGetHomeDirName();
#endif

    /*
     * Get the LANG environment variable
     * make copy since another call to getenv will blast the value.
     */
    LANGp = getenv("LANG");
    if ((LANGp == NULL) || (strlen(LANGp) == 0))
      {
	 LANG = NULL;
      }
    else
      {
	 if ((LANG = (char *) XtMalloc(strlen(LANGp) +1)) == NULL)
	   {
	      PWarning ("Insufficient memory to get LANG environment variable.");
	      return(NULL);
	   }

	 strcpy(LANG, LANGp);
      }

#endif /* DEC_MOTIF_EXTENSION */                 
    /*
     * To get a name for the file first look at the value of the configFile
     * resource.  Interpret "~/.." as relative to the user's home directory.
     * Use the LANG variable if set and .mwmrc is in $HOME/$LANG/.mwmrc  
     */

    LANG = NULL;
    if ((wmGD.configFile != NULL) && (wmGD.configFile[0] != '\0'))
    /* pointer to nonNULL string */
    {
#ifdef DEC_MOTIF_BUG_FIX
/* Check for $HOME too. */
        if ((( wmGD.configFile[0] == '~') && (wmGD.configFile[1] == '/')) ||
             ( strncmp( wmGD.configFile, "$HOME", 5 ) == 0 ))
#else
        if ((wmGD.configFile[0] == '~') && (wmGD.configFile[1] == '/'))
#endif /* DEC_MOTIF_BUG_FIX */                 
	/* handle "~/..." */
	{
#ifdef DEC_MOTIF_EXTENSION
            /* It's home; so go through the usual path for the config file.
               The OSF code does not check xnls but only $LANG. */

            /* Find the beginning of the pathname.  Either $HOME/ or ~/ */
            res_path = (char *)strstr( wmGD.configFile, "/" );
            /* Get the user config file. */
            path = NULL;
            WmInitResUserGet( res_path, &path );
            /* Was it found ? */
            if ( path != NULL )
              /* Yes, open it. */
              {
                strcpy(cfileName, path );
                XtFree( path );
                WmConfigFileChec( cfileName );
                if ((fileP = fopen (cfileName, "r")) != NULL)
                    return (fileP);
                }
#else
#ifdef MOTIF_ONE_DOT_ONE
	    GetHomeDirName(cfileName);
#else
	    strcpy (cfileName, homeDir);
#endif
	    if (LANG != NULL)
	    {
		strncat(cfileName, "/", MAXPATH-strlen(cfileName));
		strncat(cfileName, LANG, MAXPATH-strlen(cfileName));
	    }
	    strncat(cfileName, &(wmGD.configFile[1]), MAXPATH-strlen(cfileName));
	    if ((fileP = fopen (cfileName, "r")) != NULL)
	    {
		if (LANG != NULL) XtFree(LANG);
		return (fileP);
	    }
	    else
	    {
		/* 
		 * Just try $HOME/.mwmrc
		 */
#ifdef MOTIF_ONE_DOT_ONE
		GetHomeDirName(cfileName);
#else
		strcpy (cfileName, homeDir);
#endif
		strncat(cfileName, &(wmGD.configFile[1]), 
			MAXPATH-strlen(cfileName));
		if ((fileP = fopen (cfileName, "r")) != NULL)
		{
		  if (LANG != NULL) XtFree(LANG);
		  return (fileP);
		}
	    }

#endif /* DEC_MOTIF_EXTENSION */
	}
	else
	/* relative to current directory or absolute */
	{
            if ((fileP = fopen (wmGD.configFile, "r")) != NULL)
	      {
		if (LANG != NULL) XtFree(LANG);
		return(fileP);
	      }
	}
    }

    /*
     * The configFile resource didn't do it for us.
     * First try HOME_MWMRC, then try SYS_MWMRC .
     */

/* Resource description file pathnames: */

#define SLASH_MWMRC "/system.mwmrc"

#ifdef DEC_MOTIF_EXTENSION
#ifndef VMS
#define HOME_MWMRC "/.mwmrc"
#define SYS_MWMRC  "/usr/lib/X11/system.mwmrc"
#else
#define HOME_MWMRC "sys$login:decw$mwm_rc.dat"
#define SYS_MWMRC  "decw$system_defaults:decw$mwm_rc.dat"
#endif /* VMS */
    path = NULL;
    /* Get the user config file. */
    WmInitResUserGet( mwm_rc_res_user_name, &path );
    /* Was it found ? */
    if ( path != NULL )
      /* Yes, open it. */
      {
        strcpy(cfileName, path );
        XtFree( path );
        WmConfigFileChec( cfileName );
        if ((fileP = fopen (cfileName, "r")) != NULL)
            return (fileP);
      }
#else
#ifdef MOTIF_ONE_DOT_ONE
    GetHomeDirName(cfileName);
#else
    strcpy (cfileName, homeDir);
#endif

    if (LANG != NULL)
    {
	strncat(cfileName, "/", MAXPATH-strlen(cfileName));
	strncat(cfileName, LANG, MAXPATH-strlen(cfileName));
    }
    strncat(cfileName, HOME_MWMRC, MAXPATH - strlen(cfileName));
    if ((fileP = fopen (cfileName, "r")) != NULL)
    {
        if (LANG != NULL) XtFree(LANG);
        return (fileP);
    }
    else

#endif /* DEC_MOTIF_EXTENSION */

#ifdef DEC_MOTIF_EXTENSION
    /* Check for the system config file */
    WmInitResSysGet( mwm_rc_res_sys_name, &path );
    /* Was one found ? */
    if ( path != NULL )
      /* Yes, get it */
      {
        strcpy( cfileName, path );
        XtFree( path );
        WmConfigFileChec( cfileName );
        if ((fileP = fopen ( cfileName, "r")) != NULL)
            return (fileP);
      }
    /* Use the default system config file if all else fails */
    return (fopen (SYS_MWMRC, "r"));
#else
    {  
       	/* 
	 * Just try $HOME/.mwmrc
	 */
#ifdef MOTIF_ONE_DOT_ONE
	GetHomeDirName(cfileName);
#else
    strcpy (cfileName, homeDir);
#endif
        strncat(cfileName, HOME_MWMRC, MAXPATH - strlen(cfileName));
	if ((fileP = fopen (cfileName, "r")) != NULL)
	{
	  if (LANG != NULL) XtFree(LANG);
	  return (fileP);
	}
    }

#ifndef LIBDIR
#define LIBDIR "/usr/lib/X11"
#endif
    if (LANG != NULL)
    {
       /*
	* Try /$LANG/system.mwmrc within the install tree
	*/
	strcpy(cfileName, LIBDIR);
	strncat(cfileName, "/", MAXPATH-strlen(cfileName));
	strncat(cfileName, LANG, MAXPATH-strlen(cfileName));
	strncat(cfileName, SLASH_MWMRC, MAXPATH - strlen(cfileName));
	if ((fileP = fopen (cfileName, "r")) != NULL)
	{
	  XtFree(LANG);
	  return (fileP);
	}
    }

    /*         
     * Try /system.mwmrc within the install tree
     */
    strcpy(cfileName, LIBDIR);
    strncat(cfileName, SLASH_MWMRC, MAXPATH - strlen(cfileName));

    if (LANG != NULL) XtFree(LANG);
    return (fopen (cfileName, "r"));
#endif /* DEC_MOTIF_EXTENSION */

} /* END OF FUNCTION FopenConfigFile */


/*************************************<->*************************************
 *
 *  SaveMenuAccelerators (pSD, newMenuSpec)
 *
 *
 *  Description:
 *  -----------
 *  This function saves the MenuSpec pointer in pSD->acceleratorMenuSpecs.
 *
 *
 *  Inputs:
 *  ------
 *  newMenuSpec = pointer to MenuSpec to be saved.
 *  pSD->acceleratorMenuSpecs = 
 *  pSD->acceleratorMenuCount = 
 *
 *
 *  Outputs:
 *  -------
 *  pSD->acceleratorMenuSpecs = possibly updated
 *  pSD->acceleratorMenuCount = possibly updated
 *
 *
 *  Comments:
 *  --------
 *  We assume only MenuSpecs created within ProcessWmFile() are to be saved.
 *  Otherwise, we may cause override the limits of pSD->acceleratorMenuSpecs.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void SaveMenuAccelerators (pSD, newMenuSpec)

    WmScreenData *pSD;
    MenuSpec  *newMenuSpec;

#else /* _NO_PROTO */
void SaveMenuAccelerators (WmScreenData *pSD, MenuSpec *newMenuSpec)
#endif /* _NO_PROTO */
{
    MenuSpec  **pMenuSpec;

    pMenuSpec = pSD->acceleratorMenuSpecs;

    if (pMenuSpec == NULL) 
	return;

    while ((*pMenuSpec != NULL) && (*pMenuSpec != newMenuSpec))
    {
	pMenuSpec++;
    }

    if (*pMenuSpec == NULL)
    {
	*pMenuSpec = newMenuSpec;
        pSD->acceleratorMenuCount++;
    }

} /* END OF FUNCTION SaveMenuAccelerators */


/*************************************<->*************************************
 *
 *  ParseMenuSet (pSD, lineP)
 *
 *
 *  Description:
 *  -----------
 *  Menu pane specification found.  Parse the following syntax:
 *
 *          v
 *     Menu menu_pane_name
 *     {
 *       label  [mnemonic]  [accelerator]  function
 *       label  [mnemonic]  [accelerator]  function
 *                 ...
 *       label  [mnemonic]  [accelerator]  function
 *     }
 *
 *
 *  Inputs:
 *  ------
 *  cfileP = (global) file pointer to fopened configuration file or NULL
 *  lineP = pointer to menu name in line buffer
 *  line   = (global) line buffer
 *  linec  = (global) line count
 *  parseP = (global) parse string pointer if cfileP == NULL
 *  pSD->rootWindow = default root window of display
 *  wmGD.bitmapDirectory = bitmapDirectory resource value
 *  HOME = environment variable for home directory
 *
 * 
 *  Outputs:
 *  -------
 *  linec  = (global) line count incremented
 *  parseP = (global) parse string pointer if cfileP == NULL
 *  pSD->menuSpecs = list of menu specifications
 *
 *
 *  Comments:
 *  --------
 *  Skips unnamed menu specifications.
 *  This means custom menu specifications can be distinguished by NULL name.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static void ParseMenuSet (pSD, lineP)

    WmScreenData *pSD;
    unsigned char *lineP;

#else /* _NO_PROTO */
static void ParseMenuSet (WmScreenData *pSD, unsigned char *lineP)
#endif /* _NO_PROTO */
{       
    unsigned char     *string;
    MenuSpec *menuSpec;
    
    /*
     * If menu name is NULL then skip this pane specification.
     */

    if ((string = GetString (&lineP)) == NULL)
    {
        return;
    }

    /*
     * Allocate space for the menu specification structure.
     */

    if ((menuSpec = (MenuSpec *)XtMalloc (sizeof (MenuSpec))) == NULL)
    {
#ifdef DEC_MOTIF_EXTENSION
	PWarningStr( "Insufficient memory for menu", (char *)lineP );
#else
        PWarning ("Insufficient memory for menu");
#endif
	return;
    }
    menuSpec->currentContext = 0;
    menuSpec->menuWidget = NULL;
    menuSpec->whichButton = SELECT_BUTTON;  /* Button1 selection default */
    menuSpec->menuItems = NULL;
    menuSpec->accelContext = 0;
    menuSpec->accelKeySpecs = NULL;
    menuSpec->nextMenuSpec = NULL;

    /*
     * Allocate and fill space for the menu name.
     */

    if ((menuSpec->name = 
	 (String)XtMalloc ((unsigned int) (strlen ((char *)string) + 1))) 
	 == NULL)
    {
#ifdef DEC_MOTIF_EXTENSION
	PWarningStr( "Insufficient memory for menu", (char *)lineP );
#else
        PWarning ("Insufficient memory for menu");
#endif
	XtFree ((char *)menuSpec);
	return;
    }
    strcpy (menuSpec->name, (char *)string);

    /* 
     * Add the empty structure to the head of the menu specification list.
     */

    menuSpec->nextMenuSpec = pSD->menuSpecs;
    pSD->menuSpecs = menuSpec;

    /*
     * Require leading '{' on the next line.
     */

    while ((GetNextLine () != NULL))  /* not EOF nor read error */
    {
        lineP = line;
	ScanWhitespace(&lineP);

	if ((*line == '!') || (*lineP == '\0') || (*lineP == '#'))
	/* ignore empty or comment line */
        {
            continue;
        }

        if (*lineP == '{')
	/* found '{' */
        {
            break;
        }

	/* not a '{' */
#ifdef DEC_MOTIF_EXTENSION
	PWarningStr( "Expected '{' after menu name", (char *)lineP );
#else
	PWarning ("Expected '{' after menu name");
#endif
        return;
    }

    /*
     * Found leading "{" or EOF.
     * Parse menu item specifications until "}" or EOF found.
     */

    menuSpec->menuItems = ParseMenuItems (pSD);

} /* END OF FUNCTION ParseMenuSet */


/*************************************<->*************************************
 *
 *  MenuItem *
 *  ParseMwmMenuStr (pSD, menuStr)
 *
 *
 *  Description:
 *  -----------
 *  This function parses a WMW_MENU string and returns a list of 
 *  MenuItems structures.  The string should have the syntax:
 *
 *       label  [mnemonic]  [accelerator]  function
 *       label  [mnemonic]  [accelerator]  function
 *                 ...
 *       label  [mnemonic]  [accelerator]  function
 *
 *
 *  Inputs:
 *  ------
 *  line = (global) line buffer
 *  pSD->rootWindow = default root window of display
 *  wmGD.bitmapDirectory = bitmapDirectory resource value
 *  HOME = environment variable for home directory
 *  functionTable = window manager function parse table
 *
 * 
 *  Outputs:
 *  -------
 *  Return = list of MenuItem structures or NULL
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
MenuItem *ParseMwmMenuStr (pSD, menuStr)

    WmScreenData *pSD;
    unsigned char  *menuStr;

#else /* _NO_PROTO */
MenuItem *ParseMwmMenuStr (WmScreenData *pSD, unsigned char *menuStr)
#endif /* _NO_PROTO */
{

    cfileP = NULL;
    linec = 0;
    parseP = menuStr;

    return (ParseMenuItems (pSD));

} /* END OF FUNCTION ParseMwmMenuStr */


/*************************************<->*************************************
 *
 *  static MenuItem *
 *  ParseMenuItems (pSD)
 *
 *
 *  Description:
 *  -----------
 *  Parse menu item specifications:
 *
 *       label  [mnemonic]  [accelerator]  function
 *       label  [mnemonic]  [accelerator]  function
 *                 ...
 *       label  [mnemonic]  [accelerator]  function
 *     [}]
 *
 *
 *  Inputs:
 *  ------
 *  pSD    = pointer to screen data
 *  cfileP = (global) file pointer to fopened configuration file or NULL
 *  line   = (global) line buffer
 *  linec  = (global) line count
 *  parseP = (global) parse string pointer if cfileP == NULL
 *  pSD->rootWindow = default root window of display
 *  wmGD.bitmapDirectory = bitmapDirectory resource value
 *  HOME = environment variable for home directory
 *
 * 
 *  Outputs:
 *  -------
 *  linec  = (global) line count incremented
 *  parseP = (global) parse string pointer if cfileP == NULL
 *  Return = list of MenuItem structures or NULL
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static MenuItem *ParseMenuItems (pSD)

    WmScreenData *pSD;

#else /* _NO_PROTO */
static MenuItem *ParseMenuItems (WmScreenData *pSD)
#endif /* _NO_PROTO */
{
    unsigned char *string;
    unsigned char *lineP;
    MenuItem      *firstMenuItem;
    MenuItem      *lastMenuItem;
    MenuItem      *menuItem;
    register int   ix;
    
    /*
     * Parse "label [mnemonic] [accelerator] function"  lines until "}" or EOF
     *  found.
     */

    firstMenuItem = lastMenuItem = NULL;
    while ((GetNextLine () != NULL))
    {
	lineP = line;
	if ((*line == '!') || (*line == '#') || (string = GetString (&lineP)) == NULL)
	/* ignore empty or comment lines */
        {
            continue;
        }
        if (*string == '}')  /* finished with menu set. */
        {
	    break;
        }

	/*
	 * Allocate space for the menu item structure. 
	 */

        if ((menuItem = (MenuItem *)XtMalloc (sizeof (MenuItem))) == NULL)
	{
#ifdef DEC_MOTIF_EXTENSION
	    PWarningStr( "Insufficient memory for menu item", (char *)lineP );
#else
            PWarning ("Insufficient memory for menu item");
#endif
            continue;
	}
	menuItem->nextMenuItem = NULL;
	menuItem->wmFunction = (WmFunction)NULL;
	menuItem->wmFuncArgs = NULL;

        /*
	 * Parse the menu item label.
	 */

	if (!ParseWmLabel (pSD, menuItem, string))
	{
	    XtFree ((char *)menuItem);
	    continue;
	}

        /*
	 * Parse any menu function mnemonic.
	 */

	ParseWmMnemonic (&lineP, menuItem);

        /*
	 * Parse any menu function accelerator.
	 */

	if (!ParseWmAccelerator (&lineP, menuItem))
	{
	    XtFree ((char *)menuItem);
	    continue;
	}

        /*
	 * Parse the menu function name.
	 * Determine context sensitivity and applicability mask.
	 */

	ix = ParseWmFunction (&lineP, CRS_MENU, &menuItem->wmFunction);
	menuItem->greyedContext = functionTable[ix].greyedContext;
	menuItem->mgtMask = functionTable[ix].mgtMask;

        /* 
	 * Apply the function argument parser.
	 */
        if (!(*(functionTable [ix].parseProc)) 
		   (&lineP, menuItem->wmFunction, &menuItem->wmFuncArgs))
        {
	    FreeMenuItem (menuItem);
	    continue;  /* skip this menu item */
        }

	/*
	 * Add this item to the menu specification.
	 */

	if (lastMenuItem != NULL)  /* not first */
	{
	    lastMenuItem->nextMenuItem = menuItem;
	}
	else
	{
	    firstMenuItem = menuItem;
	}
	lastMenuItem = menuItem;
    }

    return (firstMenuItem);

} /* END OF FUNCTION ParseMenuItems */



/*************************************<->*************************************
 *
 *  ParseWmLabel (pSD, menuItem, string)
 *
 *
 *  Description:
 *  -----------
 *  Parse a menu label string.
 *
 *
 *  Inputs:
 *  ------
 *  pSD      = pointer to screen data
 *  menuItem = pointer to MenuItem structure
 *  string   = label string
 *
 * 
 *  Outputs:
 *  -------
 *  menuItem->label
 *  menuItem->labelType
 *  menuItem->labelBitmapCache
 *  Return   = boolean, FALSE iff insufficient memory
 *
 *
 *  Comments:
 *  --------
 * We have two label types:  XmSTRING and XmPIXMAP
 * We allocate and fill the label field with string and set the type to 
 * XmSTRING.  If string = "@<bitmap_file>", and <bitmap_file> contains a
 * with which to build a label image we save the bitmap in the MenuItem 
 * structure and set the type to XmPIXMAP.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static Boolean ParseWmLabel (pSD, menuItem, string)

    WmScreenData *pSD;
    MenuItem *menuItem;		/* RETURNED */
    unsigned char *string;

#else /* _NO_PROTO */
static Boolean ParseWmLabel (WmScreenData *pSD, MenuItem *menuItem, 
			     unsigned char *string)
#endif /* _NO_PROTO */
{

    /*
     * Allocate the label field and copy string.
     */

    if ((menuItem->label = (String)
        XtMalloc ((unsigned int)(strlen ((char *)string) + 1))) == NULL)
    {
#ifdef DEC_MOTIF_EXTENSION
	PWarningStr( "Insufficient memory for menu item", (char *)string );
#else
        PWarning ("Insufficient memory for menu item");
#endif
        return (FALSE);
    }

    strcpy (menuItem->label, (char *)string);
    menuItem->labelType = XmSTRING;

    if (*string == '@')
    /*
     * Here:  string  = "@<bitmap file>"
     * Try to find the label bitmap in the bitmap cache or read the label 
     * bitmap file.
     */
    {
        string++;  /* skip "@" */
        if ((menuItem->labelBitmapIndex = GetBitmapIndex (pSD, 
					       (char *)string)) >= 0)
	{
	    menuItem->labelType = XmPIXMAP;
	}
    }
    return (TRUE);

} /* END OF FUNCTION ParseWmLabel */



/*************************************<->*************************************
 *
 *  ParseWmMnemonic (linePP, menuItem)
 *
 *
 *  Description:
 *  -----------
 *  Parse an optional menu function mnemonic.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  menuItem = pointer to MenuItem structure
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = pointer to revised line buffer pointer.
 *  menuItem->mnemonic = valid mnemonic character or NULL.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static void ParseWmMnemonic (linePP, menuItem)

    unsigned char    **linePP;
    MenuItem *menuItem;

#else /* _NO_PROTO */
static void ParseWmMnemonic (unsigned char **linePP, MenuItem *menuItem)
#endif /* _NO_PROTO */
{
    unsigned char *lineP = *linePP;
    unsigned char *mnemonic;

    /*
     * Skip leading white space.
     */
    ScanWhitespace (&lineP);
    menuItem->mnemonic = (KeySym)NULL;

    if (*lineP == '_')
    /* 
     * We have a mnemonic specification. 
     * Get the next string (we only use the first character).
     * If no string exists, the labelType is not XmSTRING, or does not contain 
     * the first character, then skip the string and return.
     * Otherwise, accept the first character as a mnemonic.
     */
    {
	KeySym ks;
        lineP++;
        mnemonic = GetString(&lineP);

#ifndef NO_MULTIBYTE
	if (menuItem->labelType == XmSTRING &&
	    mnemonic != NULL &&
	    (ks = XStringToKeysym((char *)mnemonic)) != NoSymbol &&
	    strchr(menuItem->label, (char)(ks & 0xff)) != NULL)
	{
	    menuItem->mnemonic = ks;
	}
#else
        if ((mnemonic != NULL) &&
            (*mnemonic != '\0') &&
            (menuItem->labelType == XmSTRING) &&
	    (strchr (menuItem->label, *mnemonic) != NULL))
        /* valid mnemonic */
        {
            menuItem->mnemonic = *mnemonic;
        }
#endif
	else
	{
#ifdef DEC_MOTIF_EXTENSION
        PWarningStr( "Invalid mnemonic specification", (char *)mnemonic );
#else
        PWarning ("Invalid mnemonic specification");
#endif
	}
    }

    *linePP = lineP;  /* consume any string */

} /* END OF FUNCTION ParseWmMnemonic */


/*************************************<->*************************************
 *
 *  ParseWmAccelerator (linePP, menuItem)
 *
 *
 *  Description:
 *  -----------
 *  Parse an optional menu function accelerator.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  menuItem = pointer to MenuItem structure
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = pointer to revised line buffer pointer.
 *  menuItem->accelText = pointer to an accelerator string or NULL.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static Boolean ParseWmAccelerator (linePP, menuItem)

    unsigned char    **linePP;
    MenuItem *menuItem;

#else /* _NO_PROTO */
static Boolean ParseWmAccelerator (unsigned char **linePP, MenuItem *menuItem)
#endif /* _NO_PROTO */
{
    unsigned char *lineP;
    String        string;
    unsigned int  eventType;
    unsigned int  state;
    KeyCode       keycode;
    Boolean       status;

    /*
     * Skip leading white space.
     */
    ScanWhitespace (linePP);
    lineP = *linePP;
    menuItem->accelState = 0;
    menuItem->accelKeyCode = 0;
    menuItem->accelText = NULL;
    status = TRUE;

    /*
     * If the second character is not ".", and an accelerator specification 
     * exists, then process and save the specification string.
     */

    if ((*lineP != '\0') &&     /* something follows */
	(*lineP != '!')  &&     /* skip if we have the ! WmFunction */
	(*lineP != 'f')  &&
	(*(lineP+1) != '.'))    /* skip if we have f.xxx WmFunction */
    {
        if (ParseKeyEvent(&lineP, &eventType, &keycode, &state))
        {
            if ((string = (String) XtMalloc 
		 ((unsigned int) (lineP - *linePP + 1))) == NULL)
            {
#ifdef DEC_MOTIF_EXTENSION
	        PWarningStr( "Insufficient memory for accelerator specification", 
                          (char *)lineP );                                               
#else
	        PWarning ("Insufficient memory for accelerator specification");
#endif
                status = FALSE;
            }
	    else
	    /*
	     * Save the accelerator state and keycode.
	     * Process and save the accelerator text.
	     */
            {
	        ProcessAccelText (*linePP, lineP, (unsigned char *) string);
                menuItem->accelState = state;
                menuItem->accelKeyCode = keycode;
                menuItem->accelText = string;
            }
        }
	else
	{
#ifdef DEC_MOTIF_EXTENSION
	    PWarningStr( "Invalid accelerator specification", (char *)lineP );
#else
            PWarning("Invalid accelerator specification");
#endif
            status = FALSE;
	}

        *linePP = lineP;  /* consume the specification */
    }

    return (status);

} /* END OF FUNCTION ParseWmAccelerator */


/*************************************<->*************************************
 *
 *  int
 *  ParseWmFunction (linePP, res_spec, pWmFunction)
 *
 *
 *  Description:
 *  -----------
 *  Parse a button, key, or menu function name and return its function table
 *  index.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  res_spec = resource specification type (key, button, or menu).
 *  pWmFunction = pointer to window manager function destination.
 *  functionTable = window manager function parse table
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = pointer to revised line buffer pointer.
 *  pWmFunction = pointer to parsed window manager function.
 *  Return = function table index of parsed function.
 *
 *
 *  Comments:
 *  --------
 *  Uses F_Nop if the function name or resource type is invalid.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
int ParseWmFunction (linePP, res_spec, pWmFunction)

    unsigned char        **linePP;	/* RETURNED */
    unsigned int  res_spec;
    WmFunction   *pWmFunction;	/* RETURNED */

#else /* _NO_PROTO */
int ParseWmFunction (unsigned char **linePP, unsigned int res_spec, 
			    WmFunction *pWmFunction)
#endif /* _NO_PROTO */
{
    unsigned char *lineP = *linePP;
    unsigned char *string;
    register int  low, mid, high, cmp;

    /*
     * Skip leading white space.
     */
    ScanWhitespace (&lineP);

    /* 
     * Have function string (may be NULL or a comment).
     * Handle the special case of '!' 
     */

    if (*lineP == '!')
    {
	*linePP = ++lineP;
	*pWmFunction = F_Exec;
	return (F_EXEC_INDEX);
    }

    /*
     * Identify the function corresponding to the specified name.
     * Try binary search of the window manager function parse table.
     * Assume f.nop if the function and resource type cannot be matched.
     * This handles NULL and comment strings, bad function names, and functions
     *   in inappropriate resource sets.
     */
    string = GetString (&lineP);
    *linePP = lineP;

    if (string != NULL)
    {                            
#ifndef VMS
        ToLower (string);
#else
        ToLower2 (string);
#endif
	low = 0;
        high = WMFUNCTIONTABLESIZE - 1;

#ifdef DEC_MOTIF_EXTENSION
        if (!(strcmp ((char *)string, "f.dec_customize")))
          {                  
            *pWmFunction = F_DEC_Customize;
            return(F_DEC_CUST_INDEX);
          }

        if (!(strcmp ((char *)string, "f.dec_help")))
          {
            *pWmFunction = F_DEC_Help;  
            return(F_DEC_HELP_INDEX);
          }

#endif
        while (low <= high)
        {
	    mid = (low + high)/2;
            cmp = strcmp (functionTable[mid].funcName, (char *)string);

            if (!cmp)
	    /*
	     * Function name match 
	     * Require proper resource type for the function.
	     */
	    {
	        if (res_spec & functionTable[mid].resource)
	        {
		    *pWmFunction = functionTable[mid].wmFunction;
	            return (mid);
                }

	        /* invalid resource:  use F_Nop */
  	        break;
            }

	    /*
	     * Function name mismatch 
	     */
            if (cmp > 0)
	    {
	        high = mid - 1;
	    }
	    else
	    {
	        low = mid + 1;
	    }
        }
    }

    /* 
     * Not found:  assume f.nop
     */
    *pWmFunction = F_Nop;
    return (F_NOP_INDEX);

} /* END OF FUNCTION ParseWmFunction */


/*************************************<->*************************************
 *
 *  ParseWmFuncMaybeStrArg (linePP, wmFunction, pArgs)
 *
 *
 *  Description:
 *  -----------
 *  Parses a window manager function with a null or string argument.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  wmFunction = function (not used).
 *  pArgs = pointer to argument destination.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = pointer to revised line buffer pointer.
 *  pArgs    = pointer to parsed argument string.
 *  Return   = FALSE iff insufficient memory
 *
 *
 *  Comments:
 *  --------
 *  Only used to parse arguments for F_Lower, F_Raise, and F_Raise_Lower.
 *  If it is used for any other function, be sure to change FreeMenuItem ()
 *  accordingly.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static Boolean ParseWmFuncMaybeStrArg (linePP, wmFunction, pArgs)

    unsigned char **linePP;	/* RETURNED */
    WmFunction    wmFunction;
    String       *pArgs;	/* RETURNED */

#else /* _NO_PROTO */
static Boolean ParseWmFuncMaybeStrArg (unsigned char **linePP, 
				       WmFunction wmFunction, String *pArgs)
#endif /* _NO_PROTO */
{
    unsigned char *string = *linePP;
    unsigned int  len;

    ScanWhitespace (&string);
/*
    if (*lineP == '-')
    {
	*linePP = ++lineP;
	return (ParseWmFuncStrArg (linePP, wmFunction, pArgs));
    }
*/
    if (len = strlen ((char *)string))
    {
	if ((*pArgs = (String)XtMalloc (len + 1)) == NULL)
	{
	    PWarning ("Insufficient memory");
	    return (FALSE);
	}
	strcpy (*pArgs, (char *)string);
	return (TRUE);
    }
    else
    /* Do ParseWmFuncNoArg () */
    {
        *pArgs = NULL;
        return (TRUE);
    }

} /* END OF FUNCTION ParseWmFuncMaybeStrArg */


/*************************************<->*************************************
 *
 *  ParseWmFuncNoArg (linePP, wmFunction, pArgs)
 *
 *
 *  Description:
 *  -----------
 *  Parses a window manager function null argument.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  wmFunction = function (not used).
 *  pArgs = pointer to argument destination.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = unchanged
 *  pArgs    = NULL
 *  Return   = TRUE
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static Boolean ParseWmFuncNoArg (linePP, wmFunction, pArgs)

    unsigned char **linePP;	/* RETURNED */
    WmFunction    wmFunction;
    String        *pArgs;	/* RETURNED */

#else /* _NO_PROTO */
static Boolean ParseWmFuncNoArg (unsigned char **linePP, WmFunction wmFunction,
				 String *pArgs)
#endif /* _NO_PROTO */
{

    *pArgs = NULL;
    return (TRUE);

} /* END OF FUNCTION ParseWmFuncNoArg */


/*************************************<->*************************************
 *
 *  ParseWmFuncStrArg (linePP, wmFunction, pArgs)
 *
 *
 *  Description:
 *  -----------
 *  Parses a window manager function string argument.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  wmFunction = function for which the argument string is intended.
 *  pArgs = pointer to argument string destination.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = pointer to revised line buffer pointer.
 *  pArgs    = pointer to parsed argument string.
 *  Return   = FALSE iff insufficient memory
 *
 *
 *  Comments:
 *  --------
 *  Insures that an argument for F_Exec() ends in '&' .
 *  Only used to parse arguments for F_Exec, F_Menu, F_Lower, F_Raise, 
 *  F_Raise_Lower, and F_Screen.  If it is used for any other function, be
 *  sure to change FreeMenuItem () accordingly.
 * 
 *  #ifdef DEC_MOTIF_EXTENSION
 *  Add these to the list
 *
 *  F_DEC_Customize, F_DEC_Help    
 *  #endif DEC_MOTIF_EXENSION              
 *          
 *************************************<->***********************************/

#ifdef _NO_PROTO
static Boolean ParseWmFuncStrArg (linePP, wmFunction, pArgs)

    unsigned char **linePP;	/* RETURNED */                
    WmFunction    wmFunction;
    String        *pArgs;	/* RETURNED */

#else /* _NO_PROTO */
static Boolean ParseWmFuncStrArg (unsigned char **linePP, 
				  WmFunction wmFunction, String *pArgs)
#endif /* _NO_PROTO */
{
    unsigned char *string;
    unsigned int  len;
#ifndef NO_MULTIBYTE
    char *p;
    wchar_t last;
    int lastlen;
#endif

    if ((string = GetString (linePP)) != NULL)
    /* nonNULL string argument */
    {
        len = strlen ((char *)string);
        if ((*pArgs = (String)XtMalloc (len + 2)) == NULL)
        {
#ifdef DEC_MOTIF_EXTENSION
	    PWarningStr( "Insufficient memory",  (char *)string );
#else
            PWarning ("Insufficient memory");
#endif
	    return (FALSE);
        }
        strcpy (*pArgs, (char *)string);

        /*
         *  Insure that an argument for F_Exec ends in '&' .
         */

#ifndef NO_MULTIBYTE
	if ((wmFunction == F_Exec))
	{
	    lastlen = 0;
	    p = *pArgs;
	    while (*p &&
		   ((len = mblen(p, MB_CUR_MAX)) > 0))
	    {
		mbtowc(&last, p, MB_CUR_MAX);
		lastlen = len;
		p += len;
	    }
	    if (lastlen == 1 && last != (wchar_t)'&')
	    {
		*p++ = '&';
		*p   = '\0';
	    }
	}
#else
        if ((wmFunction == F_Exec) && ((*pArgs)[len - 1] != '&'))
        {
	    (*pArgs)[len] = '&';
	    (*pArgs)[len + 1] = '\0';
        }
#endif
    }
    else
    /* NULL string argument */
    {
        *pArgs = NULL;
    }

    return (TRUE);

} /* END OF FUNCTION ParseWmFuncStrArg */


/*************************************<->*************************************
 *
 *  FreeMenuItem (menuItem)
 *
 *
 *  Description:
 *  -----------
 *  This procedure destroys a MenuItem structure.
 *
 *
 *  Inputs:
 *  ------
 *  menuItem = to be destroyed.
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  Assumes that ParseWmFuncStrArg () has parsed a menu item's function
 *  argument only for F_Exec, F_Menu, F_Lower, F_Raise, F_Raise_Lower, and
 *  F_Screen. If it is used for other functions, be sure to include them here!
 *
 *  #ifdef DEC_MOTIF_EXTENSION
 *  Add these to the list
 *
 *  F_DEC_Customize, F_DEC_Help    
 *  #endif DEC_MOTIF_EXENSION              
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void FreeMenuItem (menuItem)

    MenuItem  *menuItem;

#else /* _NO_PROTO */
void FreeMenuItem (MenuItem *menuItem)
#endif /* _NO_PROTO */
{
    if (menuItem->label != NULL)
    {
        XtFree ((char *)menuItem->label);
    }

    if (menuItem->accelText != NULL)
    {
	XtFree ((char *)menuItem->accelText);
    }

    /*
     * If menuItem->wmFuncArgs is nonNULL, we assume that it is a string that
     * was malloc'ed in ParseWmFuncStrArg () and we free it now.
     */
    if ((menuItem->wmFuncArgs != NULL) &&
        ((menuItem->wmFunction == F_Exec)  || 
         (menuItem->wmFunction == F_Menu)  || 
         (menuItem->wmFunction == F_Lower) || 
#ifdef DEC_MOTIF_EXTENSION    
/* Extension for Help and customize */
         (menuItem->wmFunction == F_DEC_Customize) || 
         (menuItem->wmFunction == F_DEC_Help) || 
#endif /* DEC_MOTIF_EXTENSION */
         (menuItem->wmFunction == F_Raise) || 
	 (menuItem->wmFunction == F_Raise_Lower) ||
	 (menuItem->wmFunction == F_Screen)))
    {
	XtFree ((char *)menuItem->wmFuncArgs);
    }

    XtFree ((char *)menuItem);

} /* END OF FUNCTION FreeMenuItem */



/*************************************<->*************************************
 *
 *  ParseWmFuncGrpArg (linePP, wmFunction, pGroup)
 *
 *
 *  Description:
 *  -----------
 *  Parses a window manager function group argument.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  wmFunction = function for which the group argument is intended.
 *  pGroup = pointer to group argument destination.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = pointer to revised line buffer pointer.
 *  pGroup    = pointer to parsed group argument.
 *  Return   = FALSE iff invalid group argument.
 *
 *
 *  Comments:
 *  --------
 *  The only valid nonNULL arguments are "icon", "window", and "transient".
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static Boolean ParseWmFuncGrpArg (linePP, wmFunction, pGroup)

    unsigned char **linePP;	/* RETURNED */
    WmFunction    wmFunction;
    GroupArg	  *pGroup;	/* RETURNED */

#else /* _NO_PROTO */
static Boolean ParseWmFuncGrpArg (unsigned char **linePP, 
				  WmFunction wmFunction, GroupArg *pGroup)
#endif /* _NO_PROTO */
{
    unsigned char  *lineP = *linePP;
    unsigned char  *startP;
    unsigned char   grpStr[MAX_GROUP_STRLEN+1];
    int    len;


    /*
     * Parse groups while each is followed by "|".
     */

    *pGroup = 0;
    while (1)
    {
        /* 
	 * Skip whitespace and find next group string.
	 */

        ScanWhitespace (&lineP);
	startP = lineP;
        ScanAlphanumeric (&lineP);
        if (startP == lineP)
        /* Group missing => use default or complain */
	{
	    if (*pGroup)
	    {
#ifdef DEC_MOTIF_EXTENSION
	        PWarningStr( "Missing group specification", (char *)lineP );
#else
                PWarning ("Missing group specification");
#endif
                return (FALSE);
	    }
	    else
	    {
                *pGroup = F_GROUP_DEFAULT;
		break;
	    }
        }

	/*
	 * Found a group string; compare it with valid groups.
	 */

        len = min (lineP - startP, MAX_GROUP_STRLEN);
#ifdef DEC_MOTIF_EXTENSION
        (void) memcpy ((char *)grpStr, (char *)startP, len);   
#else
        (void) strncpy ((char *)grpStr, (char *)startP, len);
#endif
        grpStr[len] = '\0';
#ifndef VMS
        ToLower (grpStr);
#else
        ToLower2 (grpStr);
#endif

        if (!strcmp ("icon", (char *)grpStr))
        {
            *pGroup |= F_GROUP_ICON;
        }
        else if (!strcmp ("window", (char *)grpStr))
        {
            *pGroup |= F_GROUP_WINDOW;
        }
        else if (!strcmp ("transient", (char *)grpStr))
        {
            *pGroup |= F_GROUP_TRANSIENT;
        }
        else 
        /* Unknown group name */
        {
#ifdef DEC_MOTIF_EXTENSION
	    PWarningStr( "Invalid group specification", (char *)grpStr );
#else
            PWarning ("Invalid group specification");
#endif
            return (FALSE);
        }

        /*
	 *  Continue processing until the line is exhausted.
	 *  Skip any '|' .
	 */

        ScanWhitespace (&lineP);

        if (lineP == NULL || *lineP == '\0')
	{
	    break; 
        }
        else if (*lineP == '|')
	{
            lineP++;
        }
    }

    *linePP = lineP;
    return (TRUE);

} /* END OF FUNCTION ParseWmFuncGrpArg */



/*************************************<->*************************************
 *
 *  ParseWmFuncNbrArg (linePP, wmFunction, pNumber)
 *
 *
 *  Description:
 *  -----------
 *  Parses a window manager function number argument.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  wmFunction = function 
 *  pNumber = pointer to number argument destination.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = pointer to revised line buffer pointer.
 *  pNumber  = pointer to parsed number argument.
 *  Return   = FALSE iff invalid number argument.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static Boolean ParseWmFuncNbrArg (linePP, wmFunction, pNumber)

    unsigned char **linePP;	/* RETURNED */
    WmFunction     wmFunction;
    unsigned long *pNumber;	/* RETURNED */

#else /* _NO_PROTO */
static Boolean ParseWmFuncNbrArg (unsigned char **linePP, 
				  WmFunction wmFunction, 
				  unsigned long *pNumber)
#endif /* _NO_PROTO */
{
    int  val;

    val = StrToNum (GetString (linePP));
    if (val == -1)
    {
#ifdef DEC_MOTIF_EXTENSION
	PWarningStr( "Invalid number specification", (char *)GetString( linePP ));
#else
        PWarning ("Invalid number specification");
#endif
        *pNumber = 0;
        return (FALSE);
    }

    *pNumber = val;
    return (TRUE);

} /* END OF FUNCTION ParseWmFuncNbrArg */


/*************************************<->*************************************
 *
 *  ParseButtonStr ()
 *
 *
 *  Description:
 *  -----------
 *  This function parses a button set specification string:
 *
 *     bindings_name
 *     {
 *       button   context   function
 *       button   context   function
 *                 ...
 *       button   context   function
 *     }
 *
 *
 *
 *  Inputs:
 *  ------
 *  pSD->buttonBindings = buttonBindings resource value
 *  functionTable = window manager function parse table
 *
 * 
 *  Outputs:
 *  -------
 *  pSD->buttonSpecs = list of button binding specifications.
 *
 *
 *  Comments:
 *  --------
 *  The button set specification name must match pSD->buttonBindings.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void ParseButtonStr (pSD, buttonStr)

    WmScreenData *pSD;
    unsigned char *buttonStr;

#else /* _NO_PROTO */
void ParseButtonStr (WmScreenData *pSD, unsigned char *buttonStr)
#endif /* _NO_PROTO */
{
    unsigned char *lineP;

    cfileP = NULL;
    linec = 0;
    if (((parseP = buttonStr) != NULL) && (GetNextLine () != NULL))
    {
	lineP = line;
        ParseButtonSet (pSD, lineP);
    }

} /* END OF FUNCTION ParseButtonStr */


/*************************************<->*************************************
 *
 *  ParseButtonSet (pSD, lineP)
 *
 *
 *  Description:
 *  -----------
 *  Button set specification found.  Parse the following syntax:
 *
 *             v
 *     Buttons bindings_name
 *     {
 *       button   context   function
 *       button   context   function
 *                 ...
 *       button   context   function
 *     }
 *
 *
 *  Inputs:
 *  ------
 *  cfileP = (global) file pointer to fopened configuration file or NULL
 *  line =  (global) line buffer
 *  lineP = pointer to current character in line buffer
 *  pSD->buttonBindings = buttonBindings resource value
 *
 * 
 *  Outputs:
 *  -------
 *  lineP = pointer to current character in line buffer
 *  pSD->buttonSpecs = list of button binding specifications.
 *
 *
 *  Comments:
 *  --------
 *  Skips unnamed button binding set and sets with names that don't match
 *  the buttonBindings resource.
 *  Skips bad button binding specifications.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static void ParseButtonSet (pSD, lineP)
             
    WmScreenData  *pSD;
    unsigned char *lineP;	/* RETURNED */

#else /* _NO_PROTO */
static void ParseButtonSet (WmScreenData *pSD, unsigned char *lineP)
#endif /* _NO_PROTO */
{
    unsigned char *string;
    ButtonSpec    *buttonSpec;
    ButtonSpec    *lastButtonSpec;
    int            ix;
    
    /*
     * Parse the button set bindings from the configuration file.
     * If either the button set name or buttonBindings resource is NULL or 
     *   they don't match, then skip this button specification.
     */

    if (((string = GetString (&lineP)) == NULL) ||
        (pSD->buttonBindings == NULL) ||
        strcmp ((char *)string, pSD->buttonBindings))
    {
        return;
    }

    /* 
     * Require leading '{' on the next line.
     */
    while ((GetNextLine () != NULL))  /* not EOF nor read error */
    {
        lineP = line;
	ScanWhitespace(&lineP);

	if ((*line == '!') || (*lineP == '\0') || (*lineP == '#'))
	/* ignore empty or comment line */
        {
            continue;
        }

        if (*lineP == '{')
        /* found '{' */
        {
            break;
        }

        /* not a '{' */
#ifdef DEC_MOTIF_EXTENSION
	PWarningStr( "Expected '{' after button set name", (char *)lineP );
#else
	PWarning ("Expected '{' after button set name");
#endif
        return;
    }

    /*         
     * Found leading "{" or EOF.
     * Prepare to accumulate button bindings by finding the end of
     *   the button specification list.
     * lastButtonSpec will be NULL only if no prior bindings exist.
     */

    lastButtonSpec = pSD->buttonSpecs;
    if (lastButtonSpec != NULL)
    {
        while (lastButtonSpec->nextButtonSpec != NULL)
        {
            lastButtonSpec = (lastButtonSpec->nextButtonSpec);
        }
    }

    /*
     * Parse "button context function"  until "}" or EOF found.
     * Skips bad button binding specifications.
     */

    while ((GetNextLine () != NULL))  /* not EOF nor read error */
    {
	lineP = line;
	ScanWhitespace(&lineP);
	if ((*line == '!') || (*lineP == '\0') || (*lineP == '#'))
	/* ignore empty or comment lines */
        {
            continue;
        }
        if (*lineP == '}')  /* finished with button set */
        {
	    break;
        }

	/*
	 * Allocate space for the button binding specification. 
	 */
        if ((buttonSpec = (ButtonSpec *)XtMalloc (sizeof (ButtonSpec))) == NULL)
	{
#ifdef DEC_MOTIF_EXTENSION
    	    PWarningStr( "Insufficient memory for button specification",
                              (char *)lineP );
#else
            PWarning ("Insufficient memory for button specification");
#endif
            continue;
	}
	buttonSpec->wmFunction = (WmFunction)NULL;
	buttonSpec->wmFuncArgs = NULL;
	buttonSpec->nextButtonSpec = NULL;

	/*
	 * Parse the button specification "button".
	 */
	lineP = line;
	if (!ParseBtnEvent(&lineP,
			   &buttonSpec->eventType,
			   &buttonSpec->button,
	                   &buttonSpec->state,
	                   &buttonSpec->click))
	{
#ifdef DEC_MOTIF_EXTENSION
            PWarningStr( "Invalid button specification", (char *)lineP );
#else
            PWarning ("Invalid button specification");
#endif
	    XtFree ((char *)buttonSpec);
	    continue;  /* skip this button specification */
	}


	/*
	 * Parse the button context.
	 */
	if (!ParseContext(&lineP, &buttonSpec->context, 
			  &buttonSpec->subContext))
	{
#ifdef DEC_MOTIF_EXTENSION
    	    PWarningStr( "Invalid button context", (char *)lineP );
#else
            PWarning ("Invalid button context");
#endif
	    XtFree ((char *)buttonSpec);
	    continue;  /* skip this button specification */
	}

        /*
	 * Parse the button function and any arguments.
	 */

	ix = ParseWmFunction (&lineP, CRS_BUTTON, &buttonSpec->wmFunction);

	/*
	 * remove any subContexts that don't apply to this function
	 */

	if ((functionTable[ix].greyedContext & F_SUBCONTEXT_IB_IICON) &&
	    (buttonSpec->subContext & F_SUBCONTEXT_IB_IICON))
	{
	    buttonSpec->subContext &= ~F_SUBCONTEXT_IB_IICON;
	}

	if ((functionTable[ix].greyedContext & F_SUBCONTEXT_IB_WICON) &&
	    (buttonSpec->subContext & F_SUBCONTEXT_IB_WICON))
	{
	    buttonSpec->subContext &= ~F_SUBCONTEXT_IB_WICON;
	}


        /* 
	 * Apply the function argument parser.
	 */
        if (!(*(functionTable [ix].parseProc)) 
		   (&lineP, buttonSpec->wmFunction, &buttonSpec->wmFuncArgs))
        {
	    XtFree ((char *)buttonSpec);
	    continue;  /* skip this button specification */
	}

        /* 
	 * Add the button specification to the button specification list.
	 */
        if (lastButtonSpec != NULL)
	/* a prior specification exists */
        {
            lastButtonSpec->nextButtonSpec = buttonSpec;
        }
	else
        {
            pSD->buttonSpecs = buttonSpec;
        }
        lastButtonSpec = buttonSpec;
    }

} /* END OF FUNCTION ParseButtonSet */



/*************************************<->*************************************
 *
 *  ParseContext (linePP, context, subContext)
 *
 *
 *  Description:
 *  -----------
 *  Parses a general context string.
 *
 *
 *  Inputs:
 *  ------
 *  linePP =  pointer to current line buffer pointer.
 * 
 *  Outputs:
 *  -------
 *  linePP =  pointer to revised line buffer pointer.
 *  context =    context field value
 *  subContext = subContext field value
 *  Return = (Boolean) true iff valid context string
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static Boolean ParseContext (linePP, context, subContext)

    unsigned char **linePP;	/* RETURNED */
    Context       *context;	/* RETURNED */
    Context	  *subContext;	/* RETURNED */

#else /* _NO_PROTO */
static Boolean ParseContext (unsigned char **linePP, Context *context, 
			     Context *subContext)
#endif /* _NO_PROTO */
{
    unsigned char   *lineP = *linePP;
    unsigned char   *startP;
    unsigned char    ctxStr[MAX_CONTEXT_STRLEN+1];
    int     len;

    /*
     * Parse contexts while each is followed by "|".
     */

    *context = 0;
    *subContext = 0;
    while (1) {

        /* 
	 * Skip whitespace and find next context string.
	 */
        ScanWhitespace (&lineP);
	startP = lineP;
        ScanAlphanumeric (&lineP);
        if (startP == lineP)
        /* ERROR: Context missing */
	{
           return (FALSE);
        }

	/*
	 * Found nonNULL string; compare it with valid contexts.
	 */

        len = min(lineP - startP, MAX_CONTEXT_STRLEN);
#ifdef DEC_MOTIF_EXTENSION
        (void) memcpy ((char *)ctxStr, (char *)startP, len);
#else
        (void) strncpy ((char *)ctxStr, (char *)startP, len);
#endif
        ctxStr[len] = '\0';
#ifndef VMS
        ToLower (ctxStr);
#else
        ToLower2 (ctxStr);
#endif

        if (!strcmp ("root", (char *)ctxStr))
        {
            *context |= F_CONTEXT_ROOT;
	    *subContext |= F_SUBCONTEXT_R_ALL;
        }
        else if (!strcmp ("icon", (char *)ctxStr))
        {
            *context |= (F_CONTEXT_ICON        |
			 F_CONTEXT_ICONBOX     |
			 F_SUBCONTEXT_IB_IICON | 
			 F_SUBCONTEXT_IB_WICON );
	    *subContext |= (F_SUBCONTEXT_I_ALL    | 
			    F_SUBCONTEXT_IB_IICON | 
			    F_SUBCONTEXT_IB_WICON );
        }
        else if (!strcmp ("window", (char *)ctxStr))
        {
            *context |= F_CONTEXT_WINDOW;
	    *subContext |= F_SUBCONTEXT_W_ALL;
        }
        else if (!strcmp ("frame", (char *)ctxStr))
        {
            *context |= F_CONTEXT_WINDOW;
	    *subContext |= F_SUBCONTEXT_W_FRAME;
        }
        else if (!strcmp ("title", (char *)ctxStr))
        {
            *context |= F_CONTEXT_WINDOW;
	    *subContext |= F_SUBCONTEXT_W_TITLE;
        }
        else if (!strcmp ("border", (char *)ctxStr))
        {
            *context |= F_CONTEXT_WINDOW;
	    *subContext |= F_SUBCONTEXT_W_BORDER;
        }
        else if (!strcmp ("app", (char *)ctxStr))
        {
            *context |= F_CONTEXT_WINDOW;
	    *subContext |= F_SUBCONTEXT_W_APP;
        }
        else 
        /* Unknown context name */
        {
           return (FALSE);
        }

        /* continue only if followed by '|' */
        ScanWhitespace (&lineP);
        if (*lineP != '|')
	{
	    break; 
        }
        lineP++;
    }

    *linePP = lineP;
    return (TRUE);

} /* END OF FUNCTION ParseContext */


/*************************************<->*************************************
 *
 *  ParseKeyStr ()
 *
 *
 *  Description:
 *  -----------
 *  This function parses a key set specification string:
 *
 *     bindings_name
 *     {
 *        key   context   function
 *        key   context   function
 *                     ...
 *        key   context   function
 *     }
 *
 *
 *  Inputs:
 *  ------
 *  pSD->keyBindings = keyBindings resource value
 *  functionTable = window manager function parse table
 *
 * 
 *  Outputs:
 *  -------
 *  pSD->keySpecs = list of key binding specification
 *
 *
 *  Comments:
 *  --------
 *  The key set specification name must match pSD->keyBindings.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void
ParseKeyStr (pSD, keyStr)

    WmScreenData *pSD;
    unsigned char *keyStr;

#else /* _NO_PROTO */
void
ParseKeyStr (WmScreenData *pSD, unsigned char *keyStr)
#endif /* _NO_PROTO */
{
    unsigned char *lineP;

    cfileP = NULL;
    linec = 0;
    if (((parseP = keyStr) != NULL) && (GetNextLine () != NULL))
    {
	lineP = line;
        ParseKeySet (pSD, lineP);
    }

} /* END OF FUNCTION ParseKeyStr */



/*************************************<->*************************************
 *
 *  ParseKeySet (pSD, lineP)
 *
 *
 *  Description:
 *  -----------
 *  Key set specification found.  Parse the following syntax:
 *
 *          v
 *     Keys bindings_name
 *     {
 *        key   context   function
 *        key   context   function
 *                     ...
 *        key   context   function
 *     }
 *
 *
 *  Inputs:
 *  ------
 *  cfileP = (global) file pointer to fopened configuration file or NULL
 *  line =  (global) line buffer
 *  lineP = pointer to current character in line buffer
 *  pSD->keyBindings = keyBindings resource value
 *
 * 
 *  Outputs:
 *  -------
 *  lineP = pointer to current character in line buffer
 *  pSD->keySpecs = list of key binding specifications.
 *
 *
 *  Comments:
 *  --------
 *  Skips unnamed key binding set and sets with names that don't match the 
 *  keyBindings resource.
 *  Skips bad key binding specifications.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static void ParseKeySet (pSD, lineP)

    WmScreenData *pSD;
    unsigned char *lineP;

#else /* _NO_PROTO */
static void ParseKeySet (WmScreenData *pSD, unsigned char *lineP)
#endif /* _NO_PROTO */
{
    unsigned char         *string;
    KeySpec      *keySpec;
    KeySpec      *lastKeySpec;
    unsigned int  eventType;
    int           ix;
#ifdef DEC_MOTIF_EXTENSION
    Boolean       done;
    Boolean       morelines;
#endif
    
    /*
     * Parse the key set bindings from the configuration file.
     * If either the key set name or keyBindings resource is NULL or they
     *   don't match then skip this key specification.
     */

    if (((string = GetString (&lineP)) == NULL) ||
        (pSD->keyBindings == NULL) ||
        strcmp ((char *)string, pSD->keyBindings))
    {
        return;
    }

    /*
     * Require leading '{' on next line.
     */
    while ((GetNextLine () != NULL))  /* not EOF nor read error */
    {
        lineP = line;
	ScanWhitespace(&lineP);

	if ((*line == '!') || (*lineP == '\0') || (*lineP == '#'))
	/* ignore empty or comment line */
        {
            continue;
        }

        if (*lineP == '{')
	/* found '{' */
        {
            break;
        }

	/* not a '{' */
#ifdef DEC_MOTIF_EXTENSION
	PWarningStr( "Expected '{' after key set name", (char *)lineP );
#else
	PWarning ("Expected '{' after key set name");
#endif
        return;
    }

    /*
     * Found leading "{" or EOF.
     * Prepare to accumulate key bindings by finding the end of
     *   the key specification list.
     * lastKeySpec will be NULL only if no prior bindings exist.
     */

    lastKeySpec = pSD->keySpecs;
    if (lastKeySpec != NULL)
    {
        while (lastKeySpec->nextKeySpec != NULL)
        {
            lastKeySpec = (lastKeySpec->nextKeySpec);
        }
    }

    /*
     * Parse "key context function"  until "}" or EOF found.
     * Skip bad key bindings.
     */

#ifdef DEC_MOTIF_EXTENSION
    morelines = TRUE;
    done = FALSE;
    while ( !done )
    {
      /* Are we done parsing ? */
      if ( morelines )
          /* Set to done to true if no more lines */
          morelines = GetNextLine() != NULL;
      /* More lines ? */
      if ( !morelines ) 
        /* No, check for alt space. */
        {
          /* Last time through */
          done = TRUE;
          /* Force alt space to be post wmenu  ? */
          if ( wmGD.forceAltSpace )
            /* Yup */
            {
              strcpy( (char *)line, "Meta<Key>space window|icon f.post_wmenu" );
            }
          /* No, done */
          else break;
        }
#else
    while ((GetNextLine () != NULL))  /* not EOF nor read error */
    {
#endif
	lineP = line;
	ScanWhitespace (&lineP);
	if ((*line == '!') || (*lineP == '\0') || (*lineP == '#'))
	/* ignore empty or comment lines */
        {
            continue;
        }
        if (*lineP == '}')  /* finished with key set */
        {
#ifdef DEC_MOTIF_EXTENSION
            morelines = FALSE;
            continue;
#else
	    break;
#endif
        }

	/*
	 * Allocate space for the key specification.
	 */
        if ((keySpec = (KeySpec *)XtMalloc (sizeof (KeySpec))) == NULL)
	{
#ifdef DEC_MOTIF_EXTENSION
	    PWarningStr( "Insufficient memory for key specification",
                         (char *)lineP );
#else
            PWarning ("Insufficient memory for key specification");
#endif
            continue;
	}

	keySpec->wmFunction = (WmFunction)NULL;
	keySpec->wmFuncArgs = NULL;
	keySpec->nextKeySpec = NULL;

	/*
	 * Parse the key specification.
	 */
	if (!ParseKeyEvent(&lineP,
			   &eventType,
			   &keySpec->keycode,
	                   &keySpec->state))
	{
#ifdef DEC_MOTIF_EXTENSION
	    PWarningStr( "Invalid key specification", (char *)lineP );
#else
            PWarning ("Invalid key specification");
#endif
	    XtFree ((char *)keySpec);
	    continue;  /* skip this key specification */
	}

	/*
	 * Parse the key context.
	 *   Here lineP points to the candidate context string.
	 */

	if (!ParseContext(&lineP, &keySpec->context, 
			  &keySpec->subContext))
	{
#ifdef DEC_MOTIF_EXTENSION
	    PWarningStr( "Invalid key context", (char *)lineP );
#else
            PWarning ("Invalid key context");
#endif
	    XtFree ((char *)keySpec);
	    continue;  /* skip this key specification */
	}


        /*
	 * Parse the key function and any arguments.
	 */

	ix = ParseWmFunction (&lineP, CRS_KEY, &keySpec->wmFunction);
	
	/*
	 * remove any subContexts that don't apply to this function
	 */
	if ((functionTable[ix].greyedContext & F_SUBCONTEXT_IB_IICON) &&
	    (keySpec->subContext & F_SUBCONTEXT_IB_IICON))
	{
	    keySpec->subContext &= ~F_SUBCONTEXT_IB_IICON;
	}

	if ((functionTable[ix].greyedContext & F_SUBCONTEXT_IB_WICON) &&
	    (keySpec->subContext & F_SUBCONTEXT_IB_WICON))
	{
	    keySpec->subContext &= ~F_SUBCONTEXT_IB_WICON;
	}

        /* 
	 * Apply the function argument parser.
	 */
        if (!(*(functionTable [ix].parseProc)) 
		   (&lineP, keySpec->wmFunction, &keySpec->wmFuncArgs))
        {
	    XtFree ((char *)keySpec);
	    continue;  /* skip this key specification */
	}

        /* 
	 * Add the key specification to the key specification list. 
	 */
        if (lastKeySpec != NULL)
	/* a prior specification exists */
        {
            lastKeySpec->nextKeySpec = keySpec;
        }
	else
        {
            pSD->keySpecs = keySpec;
        }
        lastKeySpec = keySpec;
    }

} /* END OF FUNCTION ParseKeySet */


/*************************************<->*************************************
 *
 *  GetNextLine ()
 *
 *
 *  Description:
 *  -----------
 *  Returns the next line from an fopened configuration file or a newline-
 *  embedded configuration string.
 *
 *
 *  Inputs:
 *  ------
 *  cfileP = (global) file pointer to fopened configuration file or NULL
 *  line   = (global) line buffer
 *  linec  = (global) line count
 *  parseP = (global) parse string pointer if cfileP == NULL
 *
 * 
 *  Outputs:
 *  -------
 *  line =    (global) next line 
 *  linec =   (global) line count incremented
 *  parseP =  (global) parse string pointer incremented
 *  Return =  line or NULL if file or string is exhausted.
 *
 *
 *  Comments:
 *  --------
 *  If there are more than MAXLINE characters on a line in the file cfileP the
 *  excess are truncated.  
 *  Assumes the line buffer is long enough for any parse string line.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
unsigned char *
GetNextLine ()
#else /* _NO_PROTO */
unsigned char *
GetNextLine (void)
#endif /* _NO_PROTO */
{
    register unsigned char	*string;
    int				len;

#ifndef NO_MULTIBYTE
    int   chlen;
    wchar_t last;
    int lastlen;
#endif

    if (cfileP != NULL)
    /* read fopened file */
    {
	if ((string = (unsigned char *) 
		      fgets ((char *)line, MAXLINE, cfileP)) != NULL)
	{
#ifndef NO_MULTIBYTE

	    lastlen = 0;
	    while (*string &&
		   ((len = mblen((char *)string, MB_CUR_MAX)) > 0))
	    {
		mbtowc(&last, (char *)string, MB_CUR_MAX);
		lastlen = len;
		string += len;
	    }
	    if (lastlen == 1 && last == (wchar_t)'\\')
	    {
		do
		{
		    if (!fgets((char *)string, MAXLINE - (string - line), cfileP))
			break;

		    lastlen = 0;
		    while (*string &&
			   ((len = mblen((char *)string, MB_CUR_MAX)) > 0))
		    {
			mbtowc(&last, (char *)string, MB_CUR_MAX);
			lastlen = len;
			string += len;
		    }
		    linec++;
		}
		while (lastlen == 1 && last == (wchar_t)'\\');
	    }
	    string = line;
#else
	    len = strlen((char *)string) - 2;
	    if ((len > 0) && string[len] == '\\')
	    {
		do {
		    string = &string[len];
		    if (fgets((char *)string, 
		 	      MAXLINE - (string-line), cfileP) == NULL)
		       break;
		    len = strlen((char *)string) - 2;
		    linec++;
		} while ((len >= 0) && string[len] == '\\');
		string = line;
	    }
#endif
	}
    }
    else if ((parseP != NULL) && (*parseP != '\0'))
    /* read parse string */
    {
	string = line;
#ifndef NO_MULTIBYTE
	while ((*parseP != '\0') &&
               ((chlen = mblen ((char *)parseP, MB_CUR_MAX)) > 0) &&
	       (*parseP != '\n'))
	/* copy all but NULL and newlines to line buffer */
	{
	    while (chlen--)
	    {
	        *(string++) = *(parseP++);
	    }
        }
#else
	while ((*parseP != '\0') && (*parseP != '\n'))
	/* copy all but end-of-line and newlines to line buffer */
	{
	    *(string++) = *(parseP++);
        }
#endif
	*string = '\0';
	if (*parseP == '\n')
	{
	    parseP++;
	}
    }
    else
    {
	string = NULL;
    }

    linec++;
    return (string);

} /* END OF FUNCTION GetNextLine */


/*************************************<->*************************************
 *
 *  GetString (linePP)
 *
 *
 *  Description:
 *  -----------
 *  Returns the next quoted or whitespace-terminated nonquoted string in the
 *  line buffer.
 *
 *
 *  Inputs:
 *  ------
 *  linePP =  pointer to current line buffer pointer.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP =  pointer to revised line buffer pointer.
 *  Return =  string 
 *
 *
 *  Comments:
 *  --------
 *  May alter the line buffer contents.
 *  Handles quoted strings and characters, removing trailing whitespace from
 *  quoted strings.
 *  Returns NULL string if the line is empty or is a comment.
 * 
 *************************************<->***********************************/

#ifdef  _NO_PROTO
unsigned char *GetString (linePP)

    unsigned char **linePP;	/* RETURNED */

#else /* _NO_PROTO */
unsigned char *GetString (unsigned char **linePP)
#endif /* _NO_PROTO */
{
    unsigned char *lineP = *linePP;
    unsigned char *endP;
    unsigned char *curP;
    unsigned char *lnwsP;
#ifndef NO_MULTIBYTE
    int            chlen;

    /* get rid of leading white space */
    ScanWhitespace (&lineP);

    /*
     * Return NULL if line is empty, a comment, or invalid.
     */
    if (
	*lineP == '\0' ||
	((chlen = mblen ((char *)lineP, MB_CUR_MAX)) < 1) ||
        ((chlen > 0) && (*lineP == '!'))
       )
    {
        *linePP = lineP;
        return (NULL);
    }

    if ((chlen == 1) && (*lineP == '"'))
    /* Quoted string */
    {
	/*
	 * Start beyond double quote and find the end of the quoted string.
	 * '\' quotes the next character.
	 * Otherwise,  matching double quote or NULL terminates the string.
	 *
	 * We use lnwsP to point to the last non-whitespace character in the
	 * quoted string.  When we have found the end of the quoted string,
	 * increment lnwsP and if lnwsP < endP, write NULL into *lnwsP.
	 * This removes any trailing whitespace without overwriting the 
	 * matching quote, needed later.  If the quoted string was all 
	 * whitespace, then this will write a NULL at the beginning of the 
	 * string that will be returned -- OK.
	 */
	lnwsP = lineP++;                /* lnwsP points to first '"' */
	curP = endP = lineP;            /* other pointers point beyond */

        while ((*endP = *curP) &&
               ((chlen = mblen ((char *)curP, MB_CUR_MAX)) > 0) &&
	       ((chlen > 1) || (*curP != '"')))
	/* Haven't found matching quote yet.
	 * First byte of next character has been copied to endP.
	 */
        {
	    curP++;
	    if ((chlen == 1) && (*endP == '\\') && 
		((chlen = mblen ((char *)curP, MB_CUR_MAX)) > 0))
	    /* character quote:
	     * copy first byte of quoted nonNULL character down.
	     * point curP to next byte
	     */
	    {
		*endP = *curP++;
            }

	    if (chlen == 1)
	    /* Singlebyte character:  character copy finished. */
	    {
	        if (isspace (*endP))
	        /* whitespace character:  leave lnwsP unchanged. */
	        {
	            endP++;
	        }
	        else
	        /* non-whitespace character:  point lnwsP to it. */
	        {
	            lnwsP = endP++;
	        }
	    }
	    else if (chlen > 1)
	    /* Multibyte (nonwhitespace) character:  point lnwsP to it.
	     * Finish character byte copy.
	     */
	    {
	        lnwsP = endP++;
		while (--chlen)
		{
		    *endP++ = *curP++;
		    lnwsP++;
		}
	    }
        }
#else

    /* get rid of leading white space */
    ScanWhitespace (&lineP);

    /* Return NULL if line is empty, whitespace, or begins with a comment. */
    if ((*lineP == '\0') || (*lineP == '#'))
    {
        *linePP = lineP;
        return (NULL);
    }

    if (*lineP == '"')
    /* Quoted string */
    {
	/*
	 * Start beyond double quote and find the end of the quoted string.
	 * '\' quotes the next character.
	 * Otherwise,  matching double quote or NULL terminates the string.
	 *
	 * We use lnwsP to point to the last non-whitespace character in the
	 * quoted string.  When we have found the end of the quoted string,
	 * increment lnwsP and if lnwsP < endP, write NULL into *lnwsP.
	 * This removes any trailing whitespace without overwriting the 
	 * matching quote, needed later.  If the quoted string was all 
	 * whitespace, then this will write a NULL at the beginning of the 
	 * string that will be returned -- OK.
	 */
	lnwsP = lineP++;                /* lnwsP points to first '"' */
	curP = endP = lineP;            /* other pointers point beyond */

        while ((*endP = *curP) && (*endP != '"'))
	/* haven't found matching quote yet */
        {
	    /* point curP to next character */
	    curP++;
	    if ((*endP == '\\') && (*curP != '\0'))
	    /* shift quoted nonNULL character down and curP ahead */
	    {
		*endP = *curP++;
            }
	    if (isspace (*endP))
	    /* whitespace character:  leave lnwsP unchanged. */
	    {
	        endP++;
	    }
	    else
	    /* non-whitespace character:  point lnwsP to it. */
	    {
	        lnwsP = endP++;
	    }
        }
#endif

	/*
	 *  Found matching quote or NULL.  
	 *  NULL out any trailing whitespace.
	 */

	lnwsP++;
	if (lnwsP < endP)
        {
	    *lnwsP = '\0';
        }
    }

    else
    /* Unquoted string */
    {
        /* 
	 * Find the end of the nonquoted string.
	 * '\' quotes the next character.
	 * Otherwise,  whitespace, end-of-line, or '#' terminates the string.
	 */
        curP = endP = lineP;

#ifndef NO_MULTIBYTE
        while ((*endP = *curP) &&
               ((chlen = mblen ((char *)curP, MB_CUR_MAX)) > 0) &&
               ((chlen > 1) || (!isspace (*curP) && (*curP != '#'))))
	/* Haven't found whitespace or '#' yet.
	 * First byte of next character has been copied to endP.
	 */
        {
	    curP++;
	    if ((chlen == 1) && (*endP == '\\') && 
		((chlen = mblen ((char *)curP, MB_CUR_MAX)) > 0))
	    /* character quote:
	     * copy first byte of quoted nonNULL character down.
	     * point curP to next byte
	     */
	    {
		*endP = *curP++;
            }
	    endP++;
	    if (chlen > 1)
	    /* Multibyte character:  finish character copy. */
	    {
		while (--chlen)
		{
		    *endP++ = *curP++;
		}
	    }
        }
#else
        while ((*endP = *curP) && !isspace (*endP) && (*endP != '#'))
        {
	    /* point curP to next character */
	    curP++;
	    if ((*endP == '\\') && (*curP != '\0'))
	    /* shift quoted nonNULL character down and curP ahead */
	    {
		*endP = *curP++;
            }
	    endP++;
        }
#endif
    }

    /*
     * Three cases for *endP:
     *   '#' --> write NULL over # and point to NULL
     *   whitespace or
     *     matching quote -> write end-of-line over char and point beyond
     *   NULL -> point to NULL 
     */

    if (*endP == '#')
    {
	*endP = '\0';       /* write '\0' over '#' */
	*linePP = endP;     /* point to '\0' */
    }
    else if (*endP != '\0')
    {
	*endP = '\0';       /* write NULL over terminator */
	*linePP = ++curP;   /* point beyond terminator */
    }
    else
    {
	*linePP = endP;
    }
    return ((unsigned char *)lineP);

} /* END OF FUNCTION GetString */



/*************************************<->*************************************
 *
 *  ParseBtnEvent (linePP, eventType, button, state, fClick)
 *
 *
 *  Description:
 *  -----------
 *  Parse a button event specification.
 *
 *
 *  Inputs:
 *  ------
 *  linePP =          pointer to current line buffer pointer
 *  buttonEvents =    (global) button event parse table
 *  modifierStrings = (global) modifier string/mask table
 *
 * 
 *  Outputs:
 *  -------
 *  linePP =    pointer to revised line buffer pointer.
 *  eventType = type of event
 *  button =    parsed button number
 *  state =     composite modifier mask
 *  fClick =    is click?
 *
 *  Return = (Boolean) true iff valid button event specification
 * 
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean ParseBtnEvent (linePP, eventType, button, state, fClick)

    unsigned char       **linePP;	/* RETURNED */
    unsigned int *eventType;	/* RETURNED */
    unsigned int *button;	/* RETURNED */
    unsigned int *state;	/* RETURNED */
    Boolean      *fClick;	/* RETURNED */

#else /* _NO_PROTO */
Boolean ParseBtnEvent (unsigned char  **linePP,
		       unsigned int *eventType,
		       unsigned int *button,
		       unsigned int *state,
		       Boolean      *fClick)
#endif /* _NO_PROTO */
{
    if (!ParseEvent (linePP, buttonEvents, eventType, button, state, fClick))
    {
       return (FALSE);
    }

    /* 
     * The following is a fix for an X11 deficiency in regards to 
     * modifiers in grabs.
     */
    if (*eventType == ButtonRelease)
    {
	/* the button that is going up will always be in the modifiers... */
	*state |= buttonModifierMasks[*button];
    }

    return (TRUE);

} /* END OF FUNCTION ParseBtnEvent */



/*************************************<->*************************************
 *
 *  ParseKeyEvent (linePP, eventType, keyCode, state)
 *
 *
 *  Description:
 *  -----------
 *  Parse a key event specification.
 *
 *
 *  Inputs:
 *  ------
 *  linePP =          pointer to current line buffer pointer
 *  keyEvents =       (global) key event parse table
 *  modifierStrings = (global) modifier string/mask table
 *
 * 
 *  Outputs:
 *  -------
 *  linePP =    pointer to revised line buffer pointer.
 *  eventType = type of event
 *  keyCode =   parsed KeyCode
 *  state =     composite modifier mask
 *
 *  Return = (Boolean) true iff valid key event specification
 * 
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean ParseKeyEvent (linePP, eventType, keyCode, state)

    unsigned char        **linePP;	/* RETURNED */
    unsigned int *eventType;	/* RETURNED */
    KeyCode      *keyCode;	/* RETURNED */
    unsigned int *state;	/* RETURNED */

#else /* _NO_PROTO */
Boolean ParseKeyEvent (unsigned char **linePP, unsigned int *eventType,
		       KeyCode *keyCode,  unsigned int *state)
#endif /* _NO_PROTO */


{
    Boolean      fClick;
    unsigned int keySym = 0;

    if (!ParseEvent (linePP, keyEvents, eventType, &keySym, state, &fClick))
    {
       return (FALSE);
    }

    /* 
     * Here keySym is a KeySym.  Convert it to a KeyCode.
     * KeyCode will be set to 0 if keySym is not defined for any KeyCode
     *  (e.g. 0x001).
     */

    *keyCode = XKeysymToKeycode(DISPLAY, (KeySym) keySym);

    if (*keyCode == 0)
    {
        if (keySym == XK_F9)
        {
	    keySym = XK_KP_F1;
        }
        else if (keySym == XK_F10)
        {
	    keySym = XK_KP_F2;
        }
        else if (keySym == XK_F11)
        {
	    keySym = XK_KP_F3;
        }
        else if (keySym == XK_F12)
        {
	    keySym = XK_KP_F4;
        }
#ifdef DEC_MOTIF_EXTENSION
/* For DEC keyboards without escape, map the escape key to F11 */
        else if (keySym == XK_Escape)               
        {
            keySym = XK_F11;                                     
        }
#endif /* DEC_MOTIF_EXTENSION */
        *keyCode = XKeysymToKeycode(DISPLAY, (KeySym) keySym);
    }

    return (*keyCode != 0);

} /* END OF FUNCTION ParseKeyEvent */


/*************************************<->*************************************
 *
 *  ParseEvent (linePP, table, eventType, detail, state, fClick)
 *
 *
 *  Description:
 *  -----------
 *  Parse an event specification.
 *
 *
 *  Inputs:
 *  ------
 *  linePP =          pointer to current line buffer pointer.
 *  table =           event parse table
 *  modifierStrings = (global) modifier string/mask table
 *
 * 
 *  Outputs:
 *  -------
 *  linePP =    pointer to revised line buffer pointer.
 *  eventType = type of event
 *  detail =    dependent upon parse table detail procedure and closure
 *  state =     composite modifier mask
 *  fClick =    click flag
 *
 *  Return = (Boolean) true iff valid event specification
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static Boolean ParseEvent (linePP, table, eventType, detail, state, fClick)
 
 unsigned char            **linePP;		/* RETURNED */
    EventTableEntry  *table;
    unsigned int     *eventType;	/* RETURNED */
    unsigned int     *detail;		/* RETURNED */
    unsigned int     *state;		/* RETURNED */
    Boolean          *fClick;		/* RETURNED */

#else /*  _NO_PROTO */
static Boolean ParseEvent (unsigned char **linePP, EventTableEntry *table,
			   unsigned int *eventType, unsigned int *detail,
			   unsigned int *state, Boolean *fClick)
#endif /*  _NO_PROTO */
{
    unsigned char    *lineP = *linePP;
    Cardinal ix;
    Boolean  status;
 
    /* Parse the modifiers */
    if (!ParseModifiers (&lineP, state) || *lineP != '<')
    {
       return (FALSE);
    }
    lineP++;  /* skip '<' */

    /* Parse the event type */
    if (!ParseEventType (&lineP, table, eventType, &ix) || *lineP != '>') 
    {
       return (FALSE);
    }
    lineP++;  /* skip '>' */

    /*
     *  Compute detail and fClick.
     *  Status will be False for a invalid KeySym name.
     */
    status = (*(table[ix].parseProc))(&lineP, table[ix].closure, detail);
    *fClick = table[ix].fClick;

    if (status)
    {
        *linePP = lineP;
    }
    return (status);

} /* END OF FUNCTION ParseEvent */



/*************************************<->*************************************
 *
 *  ParseModifiers(linePP, state)
 *
 *
 *  Description:
 *  -----------
 *  Parses a modifier specification.
 *
 *
 *  Inputs:
 *  ------
 *  linePP = pointer to current line buffer pointer.
 *  modifierStrings = (global) modifier string/mask table
 *
 * 
 *  Outputs:
 *  -------
 *  linePP =    pointer to revised line buffer pointer.
 *  state  = composite modifier mask
 *  Return = (Boolean) true iff valid modifier name
 *
 *
 *  Comments:
 *  --------
 *  If successful, will be followed by NULL or '<'.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static Boolean ParseModifiers(linePP, state)

    unsigned char         **linePP;	/* RETURNED */
    unsigned int  *state;	/* RETURNED */

#else /* _NO_PROTO */
static Boolean ParseModifiers(unsigned char **linePP, unsigned int *state)
#endif /* _NO_PROTO */
{
    unsigned char         *lineP = *linePP;
    unsigned char         *startP;
    unsigned char          modStr[MAX_MODIFIER_STRLEN+1];
    Boolean       fNot;
    unsigned int  maskBit;
    int           len;

    *state = 0;
 
    /*
     * Parse modifiers until the event specifier is encountered.
     */

    ScanWhitespace (&lineP);
    while ((*lineP != '\0') && (*lineP != '<'))
    {
        if (*lineP == '~') 
	{
            fNot = TRUE;
            lineP++;
        }
	else 
	{
            fNot = FALSE;
	}

	startP = lineP;
        ScanAlphanumeric (&lineP);
        if (startP == lineP)
        /* ERROR: Modifier or '<' missing */
	{
            return (FALSE);
        }
        len = min(lineP - startP, MAX_MODIFIER_STRLEN);
#ifdef DEC_MOTIF_EXTENSION
        (void) memcpy ((char *)modStr, (char *)startP, len);
#else
        (void) strncpy ((char *)modStr, (char *)startP, len);
#endif
        modStr[len] = '\0';

        if (!LookupModifier (modStr, &maskBit))
        /* Unknown modifier name */
        {
            return (FALSE);
        }

	if (fNot) 
	{
            *state &= ~maskBit;
        }
	else 
	{
            *state |= maskBit;
        }
        ScanWhitespace(&lineP);
    }

    *linePP = lineP;
    return (TRUE);  /* must have '<' or NULL following */

} /* END OF FUNCTION ParseModifiers */


/*************************************<->*************************************
 *
 *  LookupModifier (name, valueP)
 *
 *
 *  Description:
 *  -----------
 *  Return the modifier mask for the provided modifier name.
 *
 *
 *  Inputs:
 *  ------
 *  name = modifier string
 *  modifierStrings = modifier string/mask table
 *
 * 
 *  Outputs:
 *  -------
 *  valueP = modifier mask
 *  Return = (Boolean) true iff valid modifier name
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef  _NO_PROTO
static Boolean LookupModifier (name, valueP)

    unsigned char         *name;
    unsigned int *valueP;	/* RETURNED */

#else /* _NO_PROTO */
static Boolean LookupModifier (unsigned char *name, unsigned int *valueP)
#endif /* _NO_PROTO */
{
    register int i;

    if (name != NULL)
    {
#ifndef VMS
        ToLower (name);
#else
        ToLower2 (name);
#endif
        for (i=0; modifierStrings[i].name != NULL; i++)
	{
	    if (!strcmp (modifierStrings[i].name, (char *)name))
            {
	        *valueP = modifierStrings[i].mask;
	        return (TRUE);
            }
        }
    }

    return (FALSE);

} /* END OF FUNCTION LookupModifier */


/*************************************<->*************************************
 *
 *  ParseEventType(linePP, table, eventType, ix)
 *
 *
 *  Description:
 *  -----------
 *  Parses the event type string.
 *
 *
 *  Inputs:
 *  ------
 *  linePP = pointer to current line buffer pointer.
 *  table =  event parse table
 *
 * 
 *  Outputs:
 *  -------
 *  linePP = pointer to revised line buffer pointer.
 *  eventType = type of event
 *  ix = table index for matched event
 *
 *  Return = (Boolean) true iff valid event
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static Boolean ParseEventType(linePP, table, eventType, ix)

    unsigned char             **linePP;		/* RETURNED */
    EventTableEntry   *table;
    unsigned int      *eventType;	/* RETURNED */
    Cardinal          *ix;		/* RETURNED */

#else /* _NO_PROTO */
static Boolean ParseEventType (unsigned char **linePP, EventTableEntry *table,
			       unsigned int *eventType, Cardinal *ix)
#endif /* _NO_PROTO */
{
    unsigned char *lineP = *linePP;
    unsigned char *startP = *linePP;
    unsigned char eventTypeStr[MAX_EVENTTYPE_STRLEN+1];
    register int  len;

    /* Parse out the event string */
    ScanAlphanumeric (&lineP);

    /*
     * Attempt to match the parsed event against our supported event set.
     */

    if (startP != lineP)
    {
        len = min (lineP - startP, MAX_EVENTTYPE_STRLEN);
#ifdef DEC_MOTIF_EXTENSION
        (void) memcpy ((char *)eventTypeStr, (char *)startP, len);
#else
        (void) strncpy ((char *)eventTypeStr, (char *)startP, len);
#endif
        eventTypeStr[len] = '\0';
#ifndef VMS
        ToLower (eventTypeStr);
#else
        ToLower2 (eventTypeStr);
#endif

        for (len = 0; table[len].event != NULL; len++)
            if (!strcmp (table[len].event, (char *)eventTypeStr))
            {
               *ix = len;
               *eventType = table[*ix].eventType;
               *linePP = lineP;
               return (TRUE); 
            }
    }

    /* Unknown event specified */
    return (FALSE);

} /* END OF FUNCTION ParseEventType */


/*************************************<->*************************************
 *
 *  ParseImmed (linePP, closure, detail)
 *
 *
 *  Description:
 *  -----------
 *  Button event detail procedure.
 *
 *
 *  Inputs:
 *  ------
 *  linePP =  not used
 *  closure = table entry
 *
 * 
 *  Outputs:
 *  -------
 *  detail = pointer to closure
 *
 *  Return = TRUE
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static Boolean ParseImmed (linePP, closure, detail)

    unsigned char         **linePP;
    unsigned int   closure;
    unsigned int  *detail;	/* RETURNED */

#else /* _NO_PROTO */
static Boolean ParseImmed (unsigned char **linePP, unsigned int closure,
			   unsigned int  *detail)
#endif /* _NO_PROTO */
{
    *detail = closure;
    return (TRUE);

} /* END OF FUNCTION ParseImmed */


/*************************************<->*************************************
 *
 *  ParseKeySym (linePP, closure, detail)
 *
 *
 *  Description:
 *  -----------
 *  Key event detail procedure.  Parses a KeySym string.
 *
 *
 *  Inputs:
 *  ------
 *  linePP = pointer to current line buffer pointer
 *
 *  closure = not used.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP = pointer to revised line buffer pointer
 *  detail = pointer to parsed KeySym
 *
 *  Return = (Boolean) true iff valid KeySym string
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static Boolean ParseKeySym (linePP, closure, detail)

    unsigned char **linePP;	/* RETURNED */
    unsigned int    closure;
    unsigned int   *detail;	/* RETURNED */

#else /* _NO_PROTO */
static Boolean ParseKeySym (unsigned char **linePP, unsigned int closure,
			    unsigned int *detail)
#endif /* _NO_PROTO */
{
    unsigned char *lineP = *linePP;
    unsigned char *startP;
    char           keySymName[MAX_KEYSYM_STRLEN+1];
    int            len;
#ifndef NO_MULTIBYTE
    int            chlen;
#endif

    ScanWhitespace (&lineP);
    startP = lineP;

#ifndef NO_MULTIBYTE
    while (*lineP &&
	   ((chlen = mblen ((char *)lineP, MB_CUR_MAX)) > 0) &&
           ((chlen > 1) ||
	   (!isspace (*lineP) && *lineP != ',' && *lineP != ':')))
    {
	/* Skip next character */
        lineP += chlen;
    }
#else
    while (*lineP && !isspace (*lineP) && *lineP != ',' && *lineP != ':' )
    {
	/* Skip next character */
        lineP++;
    }
#endif

    len = min (lineP - startP, MAX_KEYSYM_STRLEN);
#ifdef DEC_MOTIF_EXTENSION
    (void) memcpy (keySymName, (char *)startP, len);
#else
    (void) strncpy (keySymName, (char *)startP, len);
#endif
    keySymName[len] = '\0';

#ifndef NO_MULTIBYTE
    if ((*detail = XStringToKeysym(keySymName)) == NoSymbol &&
	 (mblen (keySymName, MB_CUR_MAX) == 1))
#else
    if ((*detail = XStringToKeysym(keySymName)) == NoSymbol)
#endif
    {
        if (!isdigit (keySymName[0]) ||
            ((*detail = StrToNum ((unsigned char *)&keySymName[0])) == -1))
        {
            *detail = NoSymbol;
            return (FALSE);
        }
    }
    *linePP = lineP;
    return (TRUE);

} /* END OF FUNCTION ParseKeySym */



/*************************************<->*************************************
 *
 *  StrToNum(str)
 *
 *
 *  Description:
 *  -----------
 *  Converts a string to an unsigned hexadecimal, decimal, or octal integer.
 *
 *
 *  Inputs:
 *  ------
 *  str = character string
 *
 * 
 *  Outputs:
 *  -------
 *  Return = unsigned integer 
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static unsigned int StrToNum(str)

    unsigned char *str;

#else /* _NO_PROTO */
static unsigned int StrToNum(unsigned char *str)
#endif /* _NO_PROTO */
{
    unsigned char c;
    unsigned int  val = 0;

#ifdef DEC_MOTIF_BUG_FIX
/* If the string is empty, return -1 */
    if ( str == NULL )
       	return (-1);               
#endif /* DEC_MOTIF_BUG_FIX */
    if (*str == '0')
    {
	str++;
	if (*str == 'x' || *str == 'X')
	{
	    return (StrToHex(++str));
	}
	return (StrToOct(str));
    }

    while (c = *str)
    {
	if ('0' <= c && c <= '9')
	{
	    val = val*10+c-'0';
	}
	else
	{
	    return (-1);
	}
	str++;
    }

    return (val);

} /* END OF FUNCTION StrToNum */



/*************************************<->*************************************
 *

 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static unsigned int StrToHex(str)

    unsigned char *str;

#else /* _NO_PROTO */
static unsigned int StrToHex(unsigned char *str)
#endif /* _NO_PROTO */
{
    unsigned char c;
    unsigned int  val = 0;

#ifdef DEC_MOTIF_BUG_FIX
/* If the string is empty, return -1 */
    if ( str == NULL )
       	return (-1);               
#endif /* DEC_MOTIF_BUG_FIX */
    while (c = *str)
    {
	if ('0' <= c && c <= '9')
	{
	    val = val*16+c-'0';
	}
	else if ('a' <= c && c <= 'f')
	{
	    val = val*16+c-'a'+10;
	}
	else if ('A' <= c && c <= 'F')
	{
	    val = val*16+c-'A'+10;
	}
	else
	{
	    return (-1);
	}
	str++;
    }

    return (val);

} /* END OF FUNCTION StrToHex */



/*************************************<->*************************************
 *

 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static unsigned int StrToOct(str)

    unsigned char *str;

#else /* _NO_PROTO */
static unsigned int StrToOct(unsigned char *str)
#endif /* _NO_PROTO */
{
    unsigned char c;
    unsigned int  val = 0;

#ifdef DEC_MOTIF_BUG_FIX
/* If the string is empty, return -1 */
    if ( str == NULL )
       	return (-1);               
#endif /* DEC_MOTIF_BUG_FIX */
    while (c = *str)
    {
	if ('0' <= c && c <= '7')
	{
	    val = val*8+c-'0';
	}
	else
	{
	    return (-1);
	}
	str++;
    }

    return (val);

} /* END OF FUNCTION StrToOct */



/*************************************<->*************************************
 *
 *  ScanAlphanumeric (linePP)
 *
 *
 *  Description:
 *  -----------
 *  Scan string until a non-alphanumeric character is encountered.
 *
 *
 *  Inputs:
 *  ------
 *  linePP = nonNULL pointer to current line buffer pointer
 *
 * 
 *  Outputs:
 *  -------
 *  linePP = nonNULL pointer to revised line buffer pointer
 *
 *
 *  Comments:
 *  --------
 *  Assumes linePP is nonNULL
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void ScanAlphanumeric (linePP)

    unsigned char  **linePP;

#else /* _NO_PROTO */
void ScanAlphanumeric (unsigned char **linePP)
#endif /* _NO_PROTO */
{
#ifndef NO_MULTIBYTE
    int            chlen;

    while (*linePP &&
	   ((chlen = mblen ((char *) *linePP, MB_CUR_MAX)) > 0) &&
           ((chlen > 1) || isalnum (**linePP)))
    {
        (*linePP) += chlen;
    }
#else
    while (*linePP && isalnum (**linePP))
    {
        (*linePP)++;
    }
#endif

} /* END OF FUNCTION ScanAlphanumeric */



/*************************************<->*************************************
 *
 *  ScanWhitespace(linePP)
 *
 *
 *  Description:
 *  -----------
 *  Scan the string, skipping over all white space characters.
 *
 *
 *  Inputs:
 *  ------
 *  linePP = nonNULL pointer to current line buffer pointer
 *
 * 
 *  Outputs:
 *  -------
 *  linePP = nonNULL pointer to revised line buffer pointer
 *   
 *
 *  Comments:
 *  --------
 *  Assumes linePP is nonNULL
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void ScanWhitespace(linePP)

    unsigned char  **linePP;

#else /* _NO_PROTO */
void ScanWhitespace(unsigned char  **linePP)
#endif /* _NO_PROTO */
{
#ifndef NO_MULTIBYTE
    while (*linePP && (mblen ((char *)*linePP, MB_CUR_MAX) == 1) && isspace (**linePP))
#else
    while (*linePP && isspace (**linePP))
#endif
    {
        (*linePP)++;
    }

} /* END OF FUNCTION ScanWhitespace */


/*************************************<->*************************************
 *
 *  ToLower (string)
 *
 *
 *  Description:
 *  -----------
 *  Lower all characters in a string.
 *
 *
 *  Inputs:
 *  ------
 *  string = NULL-terminated character string or NULL
 *
 * 
 *  Outputs:
 *  -------
 *  string = NULL-terminated lower case character string or NULL
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
#ifndef VMS
void ToLower (string)
#else
/*
 * VMS can't differential between ToLower with an uppercase "T" and
 * the CRTL defined function tolower with a lowercase "t", so rename
 * the ToLower function to ToLower2 on VMS.
 */
void ToLower2 (string)
#endif

    unsigned char  *string;

#else /* _NO_PROTO */
#ifndef VMS
void ToLower (unsigned char  *string)
#else
void ToLower2 (unsigned char  *string)
#endif
#endif /* _NO_PROTO */
{
    unsigned char *pch = string;
#ifndef NO_MULTIBYTE
    int            chlen;

    while (*pch && ((chlen = mblen ((char *)pch, MB_CUR_MAX)) > 0))
    {
        if ((chlen == 1) && (isupper (*pch)))
	{
	    *pch = tolower(*pch);
	}
	pch += chlen;
    }
#else
    while (*pch != '\0')
    {
        if (isupper (*pch))
	{
	    *pch = tolower(*pch);
	}
	pch++;
    }
#endif

} /* END OF FUNCTION ToLower */


/*************************************<->*************************************
 *
 *  PWarning (message)
 *
 *
 *  Description:                                                   
 *  -----------
 *  This function lists a resource description parse message to stderr.
 *
 *
 *  Inputs:
 *  ------
 *  message = pointer to a message string
 *  cfileP  = (global) file pointer to fopened configuration file or NULL
 *  linec   = (global) line counter
 * 
 *************************************<->***********************************/

#ifdef  _NO_PROTO
void
PWarning (message)

    char * message;

#else /* _NO_PROTO */
void
PWarning (char *message)
#endif /* _NO_PROTO */
{

    if (cfileP != NULL)
    {
        fprintf (stderr, "%s: %s on line %d of configuration file \"%s\"\n",
		 wmGD.mwmName, message, linec,
		 wmGD.configFile ? wmGD.configFile : cfileName);
    }
    else
    {
        fprintf (stderr, "%s: %s on line %d of specification string\n",
                     wmGD.mwmName, message, linec);
    }
    fflush (stderr);


} /* END OF FUNCTION PWarning */

#ifdef DEC_MOTIF_EXTENSION
/* Add more information to the warning messages.*/

/*************************************<->*************************************
 *
 *  PWarningStr (message,string)
 *
 *
 *  Description:
 *  -----------
 *  This function lists a resource description parse message to stderr.   
 *
 *
 *  Inputs:
 *  ------
 *  message = pointer to a message string
 *  string  = to append to message
 *  cfileP  = (global) file pointer to fopened configuration file or NULL
 *  linec   = (global) line counter
 * 
 *************************************<->***********************************/

#ifdef  _NO_PROTO
void
PWarningStr (message, string)

    char * message;
    char * string;

#else /* _NO_PROTO */
void
PWarningStr (char *message, char *string)
#endif /* _NO_PROTO */
{

    if (cfileP != NULL)
    {
        fprintf( stderr, "%s: %s on line %d of configuration file \n%s\n",
                 wmGD.mwmName, message, linec,
		 wmGD.configFile ? wmGD.configFile : cfileName);
    }
    else
    {
        fprintf( stderr, "%s: %s on line %d of specification string\n",
                 wmGD.mwmName, message, linec );
    }
    /* Is there information to append to the message ? */
    if ( string != NULL )
        fprintf( stderr, "%s\n", string );      

    fflush (stderr);


} /* END OF FUNCTION PWarningStr */                         
#endif /* DEC_MOTIF_EXTENSION */

/*************************************<->*************************************
 *
 *  ProcessAccelText (startP, endP, destP)
 *
 *
 *  Description:
 *  -----------
 *  Process accelerator text and copy into string.
 *
 *
 *  Inputs:
 *  ------
 *  startP = pointer to start of valid accelerator specification
 *  endP =   pointer past end of accelerator specification
 *  destP =  pointer to destination buffer
 *
 * 
 *  Outputs:
 *  -------
 *  Destination buffer has processed accelerator text.
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
static void ProcessAccelText (startP, endP, destP)

    unsigned char *startP;
    unsigned char *endP;
    unsigned char *destP;

#else /* _NO_PROTO */
static void ProcessAccelText (unsigned char *startP, unsigned char *endP,
			      unsigned char *destP)
#endif /* _NO_PROTO */
{
#ifndef NO_MULTIBYTE
    int   chlen;
#endif

    /*
     * Copy modifiers
     */

    ScanWhitespace (&startP);

    while (*startP != '<')
    {
        if (*startP == '~') 
	{
            *destP++ = *startP++;
        }

#ifndef NO_MULTIBYTE
        while (*startP &&
	       (((chlen = mblen ((char *)startP, MB_CUR_MAX)) > 1)
		|| isalnum (*startP)))
        {
	    while (chlen--)
	    {
	        *destP++ = *startP++;
	    }
	}
#else
        while (isalnum (*startP))
        {
	    *destP++ = *startP++;
	}
#endif
	*destP++ = '+';

        ScanWhitespace (&startP);
    }

    /*
     * Skip the key event type.
     */
    startP++;  /* skip '<' */
    while (*startP != '>')
    {
#ifndef NO_MULTIBYTE
        startP += mblen ((char *)startP, MB_CUR_MAX);
#else
        startP++;
#endif
    }
    startP++;  /* skip '>' */

    /*
     * Copy the KeySym string.
     */

    ScanWhitespace (&startP);
    while (startP != endP)
    {
        *destP++ = *startP++;
    }
    *destP = '\0';

} /* END OF FUNCTION ProcessAccelText */



/*************************************<->*************************************
 *
 *  ProcessCommandLine (argc, argv)
 *
 *
 *  Description:
 *  -----------
 *  This function looks for and processes mwm options in the command line
 *
 *  Inputs:
 *  ------
 *  argc =   argument count.
 *  argv =   argument vector.
 *
 * 
 *  Outputs:
 *  -------
 *  Changes global data to based on command line options recognized
 *
 *
 *************************************<->***********************************/
#define SCREENS_OPT		"-screens"
#define MULTI_SCREEN_OPT	"-multiscreen"

#ifdef _NO_PROTO
void ProcessCommandLine (argc, argv)

    int argc;
    char *argv[];

#else /* _NO_PROTO */
void ProcessCommandLine (int argc,  char *argv[])
#endif /* _NO_PROTO */
{
    unsigned char *string;
    int argnum;
    unsigned char *lineP;

    for (argnum = 1; argnum < argc; argnum++)
    {
        lineP = (unsigned char *) argv[argnum];
        if ((string = GetString (&lineP)) == NULL) 
        /* empty or comment line */
	{
            continue;
        }
	if (!strcmp((char *)string, MULTI_SCREEN_OPT))
	{
	    wmGD.multiScreen = True;
	    wmGD.numScreens = ScreenCount (DISPLAY);
        }
	else if (!strcmp((char *)string, SCREENS_OPT))
	{
	    argnum++;		/* skip to next arg */
            ParseScreensArgument (argc, argv, &argnum, lineP);
        }
    }

} /* END OF FUNCTION ProcessCommandLine */


/*************************************<->*************************************
 *
 *  ParseScreensArgument (argc, argv, pArgnum, lineP)
 *
 *
 *  Description:
 *  -----------
 *  This function processes the ``-screens'' command line argument
 *
 *  Inputs:
 *  ------
 *  argc =   argument count.
 *  argv =   argument vector.
 *  pArgnum = pointer to argument number where processing left off
 *  lineP  = pointer into argv[*pArgnum] where processing left off
 *
 * 
 *  Outputs:
 *  -------
 *  Changes global data to based on command line options recognized
 *      + wmGD.screenNames
 *      + wmGD.numScreens
 *  Assumes default screenNames are already in place
 *
 *************************************<->***********************************/

#ifdef  _NO_PROTO
static void ParseScreensArgument (argc, argv, pArgnum, lineP)

    int argc;
    char *argv[];
    int *pArgnum;
    unsigned char *lineP;

#else /* _NO_PROTO */
static void ParseScreensArgument (int argc, char *argv[], int *pArgnum,
				  unsigned char *lineP)
#endif /* _NO_PROTO */
{
    unsigned char *string;
    int sNum = 0;
    int lastLen;
    int nameCount = 0;

    for (; (*pArgnum < argc) && (sNum < ScreenCount(DISPLAY)); 
					    (*pArgnum)++, sNum++)
    {
	lineP = (unsigned char *)argv[*pArgnum];
	if (*argv[*pArgnum] == '"')
	{
	    /*
	     * if Quote, use GetString to strip it
	     */
	    if ((string = GetString (&lineP)) == NULL) 
		/* empty or comment line */
	    {
		continue;
	    }
	}
	else 
	{
	    string = (unsigned char *)argv[*pArgnum];
	    if (*string == '-')
	    {
		/* another option, end of screens names */
		break;
	    }
	}
	
	if (!(wmGD.screenNames[sNum] = (unsigned char *) 
	      XtRealloc ((char*)wmGD.screenNames[sNum], 
		         1 + strlen((char *)string))))
	{
	    Warning ("Insufficient memory for screen names");
	    exit(WM_ERROR_EXIT_VALUE);
	}
	else 
	{
	    strcpy((char *)wmGD.screenNames[sNum], (char *)string);
	    nameCount++;
	}
    }

    (*pArgnum)--;

    /*
     * remaining screens (if any) get first name specified 
     */
    if (nameCount > 0)
    {
	lastLen = 1 + strlen((char *)wmGD.screenNames[0]);
	for (; sNum < ScreenCount(DISPLAY); sNum++)
	{
	    if (!(wmGD.screenNames[sNum] = (unsigned char *) 
		XtRealloc ((char*)wmGD.screenNames[sNum], lastLen)))
	    {
		Warning ("Insufficient memory for screen names");
		exit(WM_ERROR_EXIT_VALUE);
	    }
	    else 
	    {
		strcpy((char *)wmGD.screenNames[sNum], 
		       (char *)wmGD.screenNames[0]);
	    }
	}
    }

} /* END OF FUNCTION ParseScreensArgument */


/*************************************<->*************************************
 *
 *  ProcessMotifBindings ()
 *
 *
 *  Description:
 *  -----------
 *  This function is used retrieve the motif input bindings
 *  and put them into a property on the root window.
 *
 *
 *************************************<->***********************************/
#ifdef _NO_PROTO
void ProcessMotifBindings ()
#else /* _NO_PROTO */
void ProcessMotifBindings (void)
#endif /* _NO_PROTO */
{
    char           fileName[MAXPATH+1];
    char	  *bindings = NULL;
#ifndef MOTIF_ONE_DOT_ONE
    char	  *homeDir = _XmOSGetHomeDirName();
#else
    FILE          *fileP;
#endif

    /*
     *  Look in the user's home directory for .motifbind
     */

#ifdef VMS             
    /* The VMS motif bindings file is
       decw$user_defaults:decw$motifbind.dat. */
    strcpy( fileName, "decw$user_defaults:decw$motifbind.dat" );
#else
#ifdef MOTIF_ONE_DOT_ONE
    GetHomeDirName(fileName);
#else
    strcpy (fileName, homeDir);
#endif
    strncat(fileName, "/", MAXPATH-strlen(fileName));
    strncat(fileName, MOTIF_BINDINGS_FILE, MAXPATH-strlen(fileName));
#endif /* VMS */

#ifdef MOTIF_ONE_DOT_ONE
    if ((fileP = fopen (fileName, "r")) != NULL)
    {
        unsigned char   buffer[MBBSIZ];
        int             count;
        Boolean         first = True;
        int             mode = PropModeReplace;
        Window          propWindow;

        /*
         * Get the atom for the property.
         */
        wmGD.xa_MOTIF_BINDINGS =
                XInternAtom (DISPLAY, _XA_MOTIF_BINDINGS, False);

        /*
         * The property goes on the root window of screen zero
         */
        propWindow = RootWindow(DISPLAY, 0);

        /*
         * Copy file contents to property on root window of screen 0.
         */
        while ( (count=fread((char *) &buffer[0], 1, MBBSIZ, fileP)) > 0)
        {
            XChangeProperty (DISPLAY, propWindow, wmGD.xa_MOTIF_BINDINGS,
                                XA_STRING, 8, mode,
                                &buffer[0], count);

            if (first)
            {
                first = False;
                mode = PropModeAppend;
            }
        }
    }

#else
    XDeleteProperty (DISPLAY, RootWindow (DISPLAY, 0),
		XInternAtom (DISPLAY, "_MOTIF_BINDINGS", False));
    XDeleteProperty (DISPLAY, RootWindow (DISPLAY, 0),
		XInternAtom (DISPLAY, "_MOTIF_DEFAULT_BINDINGS", False));

    if (_XmVirtKeysLoadFileBindings (fileName, &bindings) == True) {
	XChangeProperty (DISPLAY, RootWindow(DISPLAY, 0),
		XInternAtom (DISPLAY, "_MOTIF_BINDINGS", False),
		XA_STRING, 8, PropModeReplace,
		(unsigned char *)bindings, strlen(bindings));
    }
    else {
	_XmVirtKeysLoadFallbackBindings (DISPLAY, &bindings);
    }
    XtFree (bindings);
#endif
} /* END OF FUNCTION ProcessMotifBindings */

