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
 * $XConsortium: x386Config.c,v 1.3 91/12/09 16:38:24 converse Exp $
 *
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "X.h"
#include "Xmd.h"
#include "input.h"
#include "servermd.h"
#include "scrnintstr.h"

#include "compiler.h"

#include "x386OSD.h"
#include "x386Procs.h"

#ifndef X_NOT_POSIX
#ifdef _POSIX_SOURCE
#include <limits.h>
#else
#define _POSIX_SOURCE
#include <limits.h>
#undef _POSIX_SOURCE
#endif
#endif
#ifndef PATH_MAX
#include <sys/param.h>
#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif
#endif
#endif

#ifndef SERVER_CONFIG_FILE
#define SERVER_CONFIG_FILE "/usr/X386/Xconfig"
#endif

typedef struct {
  int           token;                /* id of the token */
  char          *name;                /* pointer to the LOWERCASED name */
} SymTabRec, *SymTabPtr;

typedef union {
  int           num;                  /* returned number */
  char          *str;                 /* private copy of the return-string */
} LexRec, *LexPtr;

#define LOCK_TOKEN  -3
#define ERROR_TOKEN -2
#define NUMBER      10000                  
#define STRING      10001


#define FONTPATH   0
#define RGBPATH    1
#define SHAREDMON  2

#define KEYBOARD   10

#define MICROSOFT  20
#define MOUSESYS   21
#define MMSERIES   22
#define LOGITECH   23
#define BUSMOUSE   24
#define XQUE       25

#define VGA256     30
#define VGA16      31
#define WGA        32
#define XGA        33

#define MODEDB     40

static SymTabRec SymTab[] = {
  { FONTPATH,   "fontpath" },
  { RGBPATH,    "rgbpath" },
  { SHAREDMON,  "sharedmonitor" },

  { KEYBOARD,   "keyboard" },

  { MICROSOFT,  "microsoft" },
  { MOUSESYS,   "mousesystems" },
  { MMSERIES,   "mmseries" },
  { LOGITECH,   "logitech" }, /* that's eveybodys favourite !!! */
  { MICROSOFT,  "mouseman" },
  { BUSMOUSE,   "busmouse" },
  { XQUE,       "xqueue" },

  { VGA256,     "vga256" },

#ifdef notyet
  { VGA16,      "vga16" },
  { WGA,        "wga" },
  { XGA,        "xga" },
#endif

  { MODEDB,     "modedb" },

  { -1,         "" },
};

#define AUTOREPEAT 0
#define DONTZAP    1
#define SERVERNUM  2
#define XLEDS      3

static SymTabRec KeyboardTab[] = {
  { AUTOREPEAT, "autorepeat" },
  { DONTZAP,    "dontzap" },
  { SERVERNUM,  "servernumlock" },
  { XLEDS,      "xleds" },
  { -1,         "" },
};


#define EMULATE3   0
#define BAUDRATE   1
#define SAMPLERATE 2

static SymTabRec MouseTab[] = {
  { BAUDRATE,   "baudrate" },
  { EMULATE3,   "emulate3buttons" },
  { SAMPLERATE, "samplerate" },
  { -1,         "" },
};


#define STATICGRAY  0
#define GRAYSCALE   1
#define STATICCOLOR 2
#define PSEUDOCOLOR 3
#define TRUECOLOR   4
#define DIRECTCOLOR 5

#define CHIPSET     10
#define CLOCKS      11
#define DISPLAYSIZE 12
#define MODES       13
#define SCREENNO    14
#define VENDOR      15
#define VIDEORAM    16
#define VIEWPORT    17
#define VIRTUAL     18

static SymTabRec GraphicsTab[] = {
  { STATICGRAY, "staticgray" },
  { GRAYSCALE,  "grayscale" },
  { STATICCOLOR,"staticcolor" },
  { PSEUDOCOLOR,"pseudocolor" },
  { TRUECOLOR,  "truecolor" },
  { DIRECTCOLOR,"directcolor" },

  { CHIPSET,    "chipset" },
  { CLOCKS,     "clocks" },
  { DISPLAYSIZE,"displaysize" },
  { MODES,      "modes" },
  { SCREENNO,   "screenno" },
  { VENDOR,     "vendor" },
  { VIDEORAM,   "videoram" },
  { VIEWPORT,   "viewport" },
  { VIRTUAL,    "virtual" },
  { -1,         "" },
};

#define INTERLACE 0
#define PHSYNC    1
#define NHSYNC    2
#define PVSYNC    3
#define NVSYNC    4

static SymTabRec TimingTab[] = {
  { INTERLACE,  "interlace"},
  { PHSYNC,     "+hsync"},
  { NHSYNC,     "-hsync"},
  { PVSYNC,     "+vsync"},
  { NVSYNC,     "-vsync"},
  { -1,         "" },
};


#define CONFIG_BUF_LEN     1024

static FILE * configFile   = NULL;
static int    configStart  = 0;           /* start of the current token */
static int    configPos    = 0;           /* current readers position */
static int    configLineNo = 0;           /* linenumber */
static char   *configBuf,*configRBuf;     /* buffer for lines */
static char   *configPath;                /* path to config file */
static int    pushToken = LOCK_TOKEN;
static LexRec val;                        /* global return value */

static DisplayModePtr pModes = NULL;

static int screenno = -100;      /* some little number ... */

extern char *getenv();
extern char *defaultFontPath;
extern char *rgbPath;



/*
 * getToken --
 *      Read next Token form the config file. Handle the global variable
 *      pushToken.
 */

static int
getToken(tab)
     SymTabRec tab[];
{
  int          c, i;

  /*
   * First check whether pushToken has a different value than LOCK_TOKEN.
   * In this case rBuf[] contains a valid STRING/TOKEN/NUMBER. But in the other
   * case the next token must be read from the input.
   */
  if (pushToken == EOF) return(EOF);
  else if (pushToken == LOCK_TOKEN)
    {
      
      c = configBuf[configPos];
      
      /*
       * Get start of next Token. EOF is handled, whitespaces & comments are
       * skipped. 
       */
      do {
	if (!c)  {
	  if (fgets(configBuf,CONFIG_BUF_LEN-1,configFile) == NULL)
	    {
	      return( pushToken = EOF );
	    }
	  configLineNo++;
	  configStart = configPos = 0;
	}
	while (((c=configBuf[configPos++])==' ') || ( c=='\t') || ( c=='\n'));
	if (c == '#') c = '\0'; 
      } while (!c);
      configStart = configPos;
      
      /*
       * Numbers are returned immediately ...
       */
      if (isdigit(c))
	{
	  configRBuf[0] = c; i = 1;
	  while (isdigit(c = configBuf[configPos++])) configRBuf[i++] = c;
	  configRBuf[i] = '\0';
	  val.num = atoi(configRBuf);
	  return(NUMBER);
	}
      
      /*
       * All Strings START with a \" ...
       */
      else if (c == '\"')
	{
	  i = -1;
	  do {
	    configRBuf[++i] = (c = configBuf[configPos++]);
	  } while ((c != '\"') && (c != '\n') && (c != '\0'));
	  configRBuf[i] = '\0';
	  val.str = (char *)xalloc(strlen(configRBuf) + 1);
	  strcpy(val.str, configRBuf);      /* private copy ! */
	  return(STRING);
	}
      
      /*
       * ... and now we MUST have a valid token. Since all tokens are handled
       * caseinsenitive, they are all lowercased internally. The search is
       * handled later along with the pushed tokens.
       */
      else
	{
	  configRBuf[0] = tolower(c); i = 0;
	  do {
	    configRBuf[++i] = tolower(c=configBuf[configPos++]);
	  } while ((c != ' ') && (c != '\t') && (c != '\n') && (c != '\0'));
	  configRBuf[i] = '\0'; i=0;
	}
      
    }
  else
    {
    
      /*
       * Here we deal with pushed tokens. Reinitialize pushToken again. If
       * the pushed token was NUMBER || STRING return them again ...
       */
      int temp = pushToken;
      pushToken = LOCK_TOKEN;
    
      if (temp == NUMBER || temp == STRING) return(temp);
    }
  
  /*
   * Joop, at last we have to lookup the token ...
   */
  if (tab)
    {
      i = 0;
      while (tab[i].token != -1)
	if (strcmp(configRBuf,tab[i].name) == 0)
	  return(tab[i].token);
	else
	  i++;
    }
  
  return(ERROR_TOKEN);       /* Error catcher */
}



/*
 * configError --
 *      Print are READABLE ErrorMessage !!! All informations that are 
 *      interesting are printed. Even a pointer to the erroanous place is
 *      printed. Maybe my e-mails will be fewer :-)
 */

static void
configError(msg)
     char *msg;
{
  int i;

  ErrorF( "\nConfig Error: %s:%d\n\n%s", configPath, configLineNo, configBuf);
  for (i = 1; i < configStart; i++) ErrorF(" ");
  for (i = configStart; i < configPos; i++) ErrorF("^");
  ErrorF("\n%s\n", msg);
  exit(-1);                 /* simple exit ... */
}


/*
 * configKeyboard --
 *      Configure all keyboard related parameters
 */

static void
configKeyboard()
{
  int token;

  x386Info.dontZap       = FALSE;
  x386Info.serverNumLock = FALSE;
  x386Info.xleds         = 0L;
  x386Info.kbdDelay      = 500;
  x386Info.kbdRate       = 30;

  for (;;) {
    
    switch (token = getToken(KeyboardTab)) {

    case AUTOREPEAT:
      if (getToken(NULL) != NUMBER) configError("Autorepeat delay expected");
      x386Info.kbdDelay = val.num;
      if (getToken(NULL) != NUMBER) configError("Autorepeat rate expected");
      x386Info.kbdRate = val.num;
      break;

    case DONTZAP:
      x386Info.dontZap = TRUE;
      break;

    case SERVERNUM:
      x386Info.serverNumLock = TRUE;
      break;

    case XLEDS:
      while ((token= getToken(NULL)) == NUMBER)
	x386Info.xleds |= 1L << (val.num-1);
      pushToken = token;
      break;

    default:
      pushToken = token;
      return;
    }
  }
}


/*
 * configMouse --
 *      Configure all mouse related parameters
 */

static void
configMouse()
{
  int token;

  x386Info.baudRate        = 1200;
  x386Info.sampleRate      = 0;
  x386Info.emulate3Buttons = FALSE;

  for (;;) {

    switch (token = getToken(MouseTab)) {

    case BAUDRATE:
      if (getToken(NULL) != NUMBER) configError("Baudrate expected");
      x386Info.baudRate = val.num;
      break;

    case SAMPLERATE:
      if (getToken(NULL) != NUMBER) configError("Sample rate expected");
      x386Info.sampleRate = val.num;
      break;

    case EMULATE3:
      x386Info.emulate3Buttons = TRUE;
      break;

    default:
      pushToken = token;
      return;
    }
  }
}


/*
 * configGraphics --
 *      Set up all parameters for the graphics drivers. These may be changed
 *      by the driver during device-probe ...
 */

static void
configGraphics(index)
     int index;
{
  int token,i;
  DisplayModePtr pNew, pLast;

  x386Screens[index]->configured = TRUE;
  x386Screens[index]->index = screenno++;
  x386Screens[index]->frameX0 = -1;
  x386Screens[index]->frameY0 = -1;
  x386Screens[index]->virtualX = -1;
  x386Screens[index]->virtualY = -1;
  x386Screens[index]->defaultVisual = -1;
  x386Screens[index]->chipset = NULL;
  x386Screens[index]->modes = NULL;
  x386Screens[index]->vendor = NULL;
  x386Screens[index]->videoRam = 0;
  x386Screens[index]->width = 240;
  x386Screens[index]->height = 180;

  for (;;) {

    switch (token = getToken(GraphicsTab)) {

    case STATICGRAY:
    case GRAYSCALE:
    case STATICCOLOR:
    case PSEUDOCOLOR:
    case TRUECOLOR:
    case DIRECTCOLOR:
      x386Screens[index]->defaultVisual = token - STATICGRAY;
      break;

    case CHIPSET:
      if (getToken(NULL) != STRING) configError("Chipset string expected");
      x386Screens[index]->chipset = val.str;
      break;

    case CLOCKS:
      for (i=0; (token = getToken(NULL)) == NUMBER && i < MAXCLOCKS; i++)
	x386Screens[index]->clock[i] = val.num;

      x386Screens[index]->clocks = i;
      pushToken = token;
      break;

    case DISPLAYSIZE:
      if (getToken(NULL) != NUMBER) configError("Display Width expected");
      x386Screens[index]->width = val.num;
      if (getToken(NULL) != NUMBER) configError("Height expected");
      x386Screens[index]->height = val.num;
      break;

    case MODES:
      for (pLast=NULL; (token = getToken(NULL)) == STRING; pLast = pNew)
	{
	  pNew = (DisplayModePtr)xalloc(sizeof(DisplayModeRec));
	  pNew->name = val.str;

	  if (pLast) 
	    {
	      pLast->next = pNew;
	      pNew->prev  = pLast;
	    }
	  else
	    x386Screens[index]->modes = pNew;
	}
      pNew->next = x386Screens[index]->modes;
      x386Screens[index]->modes->prev = pLast;
      pushToken = token;
      break;

    case SCREENNO:
      if (getToken(NULL) != NUMBER) configError("Screen No expected");
      x386Screens[index]->index = val.num;

    case VENDOR:
      if (getToken(NULL) != STRING) configError("Vendor string expected");
      x386Screens[index]->vendor = val.str;
      break;

    case VIDEORAM:
      if (getToken(NULL) != NUMBER) configError("Video RAM size expected");
      x386Screens[index]->videoRam = val.num;
      break;

    case VIEWPORT:
      if (getToken(NULL) != NUMBER) configError("Viewport X expected");
      x386Screens[index]->frameX0 = val.num;
      if (getToken(NULL) != NUMBER) configError("Viewport Y expected");
      x386Screens[index]->frameY0 = val.num;
      break;

    case VIRTUAL:
      if (getToken(NULL) != NUMBER) configError("Virtual X expected");
      x386Screens[index]->virtualX = val.num;
      if (getToken(NULL) != NUMBER) configError("Virtual Y expected");
      x386Screens[index]->virtualY = val.num;
      break;

    default:
      pushToken = token;
      return;
    }
  }
}


/*
 * x386Config --
 *	Fill some internal structure with userdefined setups. Many internal
 *      Structs are initialized. The drivers are selected and initialized.
 */

void
x386Config ()
{
  char           *home;
  int            index = 0;
  int            token;
  int            i,j;
  DisplayModePtr pNew, pLast;

  configBuf  = (char*)xalloc(CONFIG_BUF_LEN);
  configRBuf = (char*)xalloc(CONFIG_BUF_LEN);
  configPath = (char*)xalloc(PATH_MAX);
  
  configBuf[0] = '\0';                    /* sanity ... */
  
  /*
   * First open if necessary the config file. Note, that every user may have
   * a private Xconfig file in his home.
   */
  while (!configFile) {
    
    /*
     * ~/Xconfig ...
     */
    if (home = getenv("HOME")) {
      strcpy(configPath,home);
      strcat(configPath,"/Xconfig");
      if (configFile = fopen( configPath, "r" )) break;
    }
    
    /*
     * /usr/lib/X11/X386/Xconfig.<hostname>
     */
    strcpy(configPath, SERVER_CONFIG_FILE);
    strcat(configPath, ".");
    gethostname(configPath+strlen(configPath), MAXHOSTNAMELEN);
    if (configFile = fopen( configPath, "r" )) break;
    
    /*
     * /usr/lib/X11/X386/Xconfig
     */
    strcpy(configPath, SERVER_CONFIG_FILE);
    if (configFile = fopen( configPath, "r" )) break;
    
    FatalError("No config file found!\n");
  }


  x386Info.sharedMonitor = FALSE;
  x386Info.kbdProc = NULL;

  
  while ((token = getToken(SymTab)) != EOF)
    
    switch (token) {
      
    case SHAREDMON:
      x386Info.sharedMonitor = TRUE;
      break;
      
    case FONTPATH:
      if (getToken(NULL) != STRING) configError("Font path expected");
      defaultFontPath = val.str;
      break;
      
    case RGBPATH:
      if (getToken(NULL) != STRING) configError("RGB path expected");
      rgbPath = val.str;
      break;
      
      
    case KEYBOARD:
      configKeyboard();
      break;
      
      
    case MICROSOFT:
    case MOUSESYS:
    case MMSERIES:
    case LOGITECH:
    case BUSMOUSE:
      if (getToken(NULL) != STRING) configError("Mouse device expected");
      x386Info.kbdProc    = x386KbdProc;
      x386Info.kbdEvents  = x386KbdEvents;
      x386Info.mseProc    = x386MseProc;
      x386Info.mseEvents  = x386MseEvents;
      x386Info.mseType    = token - MICROSOFT;
      x386Info.mseDevice  = val.str;
      configMouse();
      break;
      
#ifdef XQUEUE
    case XQUE:
      x386Info.kbdProc   = x386XqueKbdProc;
      x386Info.kbdEvents = x386XqueEvents;
      x386Info.mseProc   = x386XqueMseProc;
      x386Info.mseEvents = x386XqueEvents;
      x386Info.xqueSema  = 0;
      configKeyboard();
      configMouse();
      break;
#endif
      
      
    case VGA256:
    case VGA16:
    case WGA:
    case XGA:
      configGraphics(token - VGA256);
      break;


    case MODEDB:
      for (pLast=NULL, token = getToken(NULL);
	   token == STRING || token == NUMBER;
	   pLast = pNew)
	{
	  pNew = (DisplayModePtr)xalloc(sizeof(DisplayModeRec));
	  if (pLast) 
	    pLast->next = pNew;
	  else
	    pModes = pNew;
	  
	  if (token == STRING)
	    {
	      pNew->name = val.str;
	      if ((token = getToken(NULL)) != NUMBER)
		FatalError("Dotclock expected");
	    }
	  else if (pLast)
	    {
	      pNew->name = (char *)strdup(pLast->name);
	    }
	  else
	    FatalError("Mode name expected");

	  pNew->Flags = 0;
	  pNew->Clock = val.num;
	  
	  if (getToken(NULL) == NUMBER) pNew->HDisplay = val.num;
	  else configError("Horizontal display expected");
	  
	  if (getToken(NULL) == NUMBER) pNew->HSyncStart = val.num;
	  else configError("Horizontal sync start expected");
	  
	  if (getToken(NULL) == NUMBER) pNew->HSyncEnd = val.num;
	  else configError("Horizontal sync end expected");
	  
	  if (getToken(NULL) == NUMBER) pNew->HTotal = val.num;
	  else configError("Horizontal total expected");
	  
	  
	  if (getToken(NULL) == NUMBER) pNew->VDisplay = val.num;
	  else configError("Vertical display expected");
	  
	  if (getToken(NULL) == NUMBER) pNew->VSyncStart = val.num;
	  else configError("Vertical sync start expected");
	  
	  if (getToken(NULL) == NUMBER) pNew->VSyncEnd = val.num;
	  else configError("Vertical sync end expected");
	  
	  if (getToken(NULL) == NUMBER) pNew->VTotal = val.num;
	  else configError("Vertical total expected");

	  while (((token=getToken(TimingTab)) != EOF) &&
		 (token != STRING) &&
		 (token != NUMBER))
	    
	    switch(token) {
	      
	    case INTERLACE: pNew->Flags |= V_INTERLACE;  break;
	    case PHSYNC:    pNew->Flags |= V_PHSYNC;     break;
	    case NHSYNC:    pNew->Flags |= V_NHSYNC;     break;
	    case PVSYNC:    pNew->Flags |= V_PVSYNC;     break;
	    case NVSYNC:    pNew->Flags |= V_NVSYNC;     break;
	    default:
	      configError("Videomode special flag expected");
	      break;
	    }
	}
      pushToken = token;
      break;

    default:
      configError("Keyword expected");
      break;
    }

  /*
   * Probe all configured screens for letting them resolve their modes
   */
  for ( i=0; i < x386MaxScreens; i++ )
    if (x386Screens[i]->configured &&
	(x386Screens[i]->configured = (x386Screens[i]->Probe)()))
      x386InitViewport(x386Screens[i]);

  /*
   * Now sort the drivers to match the order of the ScreenNumbers
   * requested by the user. (sorry, slow bubble-sort here)
   * Note, that after this sorting the first driver that is not configured
   * can be used as last-mark for all configured ones.
   */
  for ( j = 0; j < x386MaxScreens-1; j++)
    for ( i=0; i < x386MaxScreens-j; i++ )
      if (!x386Screens[i]->configured ||
	  (x386Screens[i+1]->configured &&
	   (x386Screens[i+1]->index < x386Screens[i]->index)))
	{
	  ScrnInfoPtr temp = x386Screens[i+1];
	  x386Screens[i+1] = x386Screens[i];
	  x386Screens[i] = temp;
	}

  /*
   * free up mode info...
   */
  for (pLast = pModes, pNew = pModes->next;
       pLast;
       pLast = pNew, pNew = pNew->next)
    {
      Xfree(pLast->name);
      Xfree(pLast);
    }
  
  fclose(configFile);
  Xfree(configBuf);
  Xfree(configRBuf);
  Xfree(configPath);
  
  if (!x386Info.kbdProc) configError("You must specify the input device(s)");
}


void 
x386LookupMode(target, driver)
     DisplayModePtr target;
     ScrnInfoPtr    driver;
{
  DisplayModePtr p;
  int            i;
  Bool           found = FALSE;

  for (p = pModes; p != NULL; p = p->next)    /* scan list */
    if (!strcmp(p->name, target->name))       /* names equal ? */
      for (i=0; i < driver->clocks; i++)      /* scan clocks */
	if (p->Clock == driver->clock[i])     /* clock found */
	  {
	    found = TRUE;
	    target->Clock      = i;
	    target->HDisplay   = p->HDisplay;
	    target->HSyncStart = p->HSyncStart;
	    target->HSyncEnd   = p->HSyncEnd;
	    target->HTotal     = p->HTotal;
	    target->VDisplay   = p->VDisplay;
	    target->VSyncStart = p->VSyncStart;
	    target->VSyncEnd   = p->VSyncEnd;
	    target->VTotal     = p->VTotal;
	    target->Flags      = p->Flags;
	  }
  if (!found)
    FatalError("Mode couldn't be resolved: \"%s\"\n", target->name);
}
