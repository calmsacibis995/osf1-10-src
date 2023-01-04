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
/***********************************************************
Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* $XConsortium: ws_io.c,v 1.6 92/04/06 18:20:28 keith Exp $ */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/tty.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/devio.h>
#define  NEED_EVENTS
#include "misc.h"
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "pixmap.h"
#include "inputstr.h"
#include "cursorstr.h"
#include "regionstr.h"
#include "resource.h"
#include "dixstruct.h"
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include "ws.h"
#include "keynames.h"


#include "mi.h"

extern Bool InitKeyboardDeviceStruct();

#define MOTION_BUFFER_SIZE 100

#define ModeSwitchMask 	Mod3Mask
#define NumLockMask 	Mod4Mask
/* The following is taken from lib/X/keysymdef.h to initialize default modMasks */
#define XK_Mode_switch		0xFF7E	/* Character set switch */
#define XK_Num_Lock             0xFF7F

extern ws_descriptor 	wsinfo;
void 			wsCursorControl();
static Bool 		wsDisplayCursor();

extern int 		wsFd;
int 			rememberedScreen;
static int 		shiftLock;
static int 		numLock;
ScreenPtr		wsScreens[MAXSCREENS];
ScreenArgsRec		screenArgs[MAXSCREENS];
static DevicePtr	wsPointer;
static DevicePtr	wsKeyboard;
char 			*blackValue, *whiteValue;
extern	ws_event_queue	*queue;
int 			lastEventTime;
int 			wsNumButtons = -1;
static Bool 		cursorConfined = FALSE;
int 			wsTabletOverhang = -1;

#ifdef notdef
#define MAX_LED 3	/* only 3 LED's can be set by user; Lock LED
			   is controlled by server */
#endif

#ifndef __alpha
#define VSYNCFIXED
#endif
#ifdef VSYNCFIXED
#define CURRENT_TIME	queue->time
#else
#define CURRENT_TIME	GetTimeInMillis()
#endif

#define NoSuchClass -1
#define TrueColor12 6

static int
ParseClass(className)
    char *	className;
{
    static char *names[] = {
	"StaticGray", 
	"GrayScale", 
	"StaticColor",
	"PseudoColor", 
	"TrueColor",
	"DirectColor",
        "TrueColor12"};
    /* 
     * only the ones we support and must be in order from X.h, since
     * return value depends on index into array.
     */
    int i;
    for (i = 0; i < sizeof(names)/sizeof(char *); i++)
    {
	if (strcmp(names[i], className) == 0)
	    return i;
    }
    return NoSuchClass;
}

/* SaveScreen does blanking, so no need to worry about the interval timer */

Bool
wsSaveScreen(pScreen, on)
    ScreenPtr pScreen;
    int on;
{
    ws_video_control vc;

    vc.screen = WS_SCREEN(pScreen);

    if (on == SCREEN_SAVER_FORCER) {
	lastEventTime = CURRENT_TIME;
    } else if (on == SCREEN_SAVER_ON) {
	vc.control = SCREEN_OFF;

	if (ioctl(wsFd, VIDEO_ON_OFF, &vc) < 0)
	    ErrorF("VIDEO_ON_OFF: failed to turn screen off.\n");
    } else {
	vc.control = SCREEN_ON;
    	
	if (ioctl(wsFd, VIDEO_ON_OFF, &vc) < 0)
	    ErrorF("VIDEO_ON_OFF: failed to turn screen on.\n");
    }
    return TRUE;
}


wsPixelError(psn)
int psn;
{
    ErrorF("Only 0 through 255 are acceptable pixels for device %d\n", psn);
}

void
wsClick(click)
    int         click;
{
    ws_keyboard_control control;
    control.device_number = wsinfo.console_keyboard;
    control.flags = WSKBKeyClickPercent;
    control.click = click;
    
    if (ioctl (wsFd, SET_KEYBOARD_CONTROL, &control) == -1)
      ErrorF ("couldn't set click\n");
    return;
}

static void
wsChangeKeyboardControl(device, ctrl)
    DevicePtr device;
    KeybdCtrl *ctrl;
{
    int i;
    ws_keyboard_control control;
    control.device_number = wsinfo.console_keyboard;
    control.flags = 0;

    /* 
     * even though some of these are not implemented on the lk201/lk401,
     * we should pass these to the driver, as other hardware may not
     * lose so badly.  The new driver does do auto-repeat and up down
     * properly, however.
     */
    control.flags |=  WSKBKeyClickPercent| WSKBBellPercent | WSKBBellPitch | 
	WSKBBellDuration | WSKBAutoRepeatMode | WSKBAutoRepeats | WSKBLed;
    control.click = ctrl->click;
    control.bell = ctrl->bell;
    control.bell_pitch = ctrl->bell_pitch;
    control.bell_duration = ctrl->bell_duration;
    control.auto_repeat = ctrl->autoRepeat;
    control.leds = ctrl->leds;
    /* 
     * XXX a crock, but to have a byte interface would have implied the
     * driver did alot more work at interrupt time, so we made it 32 bits wide.
     */
    bcopy(ctrl->autoRepeats, control.autorepeats, 32);
#ifdef notdef
    /* LEDs */
    for (i=1; i<=MAX_LED; i++)
        ChangeLED(i, (ctrl->leds & (1 << (i-1))));
#endif

    if (ioctl (wsFd, SET_KEYBOARD_CONTROL, &control) == -1)
      ErrorF ("couldn't set global autorepeat\n");
    return;

}

static void
wsBell(loud, pDevice)
    int loud;
    DevicePtr pDevice;
{
    ws_keyboard_control control;
    control.device_number = wsinfo.console_keyboard;
    control.flags = WSKBBellPercent;
    control.bell = loud;

    ioctl(wsFd, SET_KEYBOARD_CONTROL, &control);

    ioctl(wsFd, RING_KEYBOARD_BELL, &wsinfo.console_keyboard);
}

/*
 * These serve protocol requests, setting/getting acceleration and threshold.
 * X10 analog is "SetMouseCharacteristics".
 */
void
wsChangePointerControl(device, ctrl)
    DevicePtr device;
    PtrCtrl   *ctrl;
{
    ws_pointer_control pc;
    pc.device_number = wsinfo.console_pointer;
    pc.numerator = ctrl->num;
    pc.denominator = ctrl->den;
    pc.threshold = ctrl->threshold;

    if (ioctl (wsFd, SET_POINTER_CONTROL, &pc) == -1) {
	ErrorF ("error setting mouse properties\n");
    }
}

int
wsGetMotionEvents(pDevice, buff, start, stop)
    CARD32 start, stop;
    DevicePtr pDevice;
    xTimecoord *buff;
{
    register int    i;		/* Number of events left to process in ring */
    ws_motion_buffer *mb = queue->mb;
    register ws_motion_history *mh = mb->motion; /*Beginning of ring storage*/
    int		    count;      /* Number of events that match conditions   */
    CARD32 temptime;

    /* Loop through entire ring buffer.  Technically, the driver may not have
       actually queued this many motion events, but since they are initialized
       to time 0 the non-events shouldn't match.  If the mouse isn't moved for
       25 days after startup this could be a problem...but who cares? */
    count = 0;
    for (i = mb->size; i != 0; i--) {
	temptime = mh->time;
	if (start <= temptime && temptime <= stop) {
	    buff[count].time = temptime;
	    buff[count].x    = mh->axis[0];
	    buff[count].y    = mh->axis[1];
	    if (temptime == mh->time) count++;	/* paranoid */
	}
	mh = (ws_motion_history *) ((caddr_t)mh + mb->entry_size);
    } 
    return count;
}

int
wsMouseProc(pDev, onoff, argc, argv)
    DevicePtr pDev;
    int onoff, argc;
    char *argv[];
{
    int     i, numButtons;
    BYTE    map[6];
    ws_hardware_type wht;

    switch (onoff)
    {
	case DEVICE_INIT: 
	    wht.device_number = wsinfo.console_pointer;

	    ioctl(wsFd, GET_DEVICE_TYPE, &wht);

	    wsPointer = pDev;
	    pDev->devicePrivate = (pointer) &queue;
	    map[1] = 1;  /* worst case is 5 buttons  - jmg */
	    map[2] = 2;
	    map[3] = 3;
	    map[4] = 4;
	    map[5] = 5;
	    if(wsNumButtons == -1) {
#ifdef WSPX
		/* multi-PX driver does DEC pointer autoconfig */
		if (! (numButtons = wht.buttons))
		    /*
		     * if not specified on cmd line and driver didn't recognize
		     * pointer device ID, then default as in the past...
		     */
		if(wht.hardware_type == VSXXX)
		    numButtons = 3;
		else 
		    numButtons = 4;
#else
		numButtons = wht.buttons; /* believe the Kernel :-) */
#endif
	    } 
	    else
		numButtons = wsNumButtons;
	    InitPointerDeviceStruct(
		wsPointer, map, numButtons, wsGetMotionEvents,
		wsChangePointerControl, MOTION_BUFFER_SIZE);
	    SetInputCheck(&queue->head, &queue->tail);
#ifdef WSPX
	    if (wsTabletOverhang >= 0) {
		ws_tablet_control ioc;
		ioc.screen =		/* "current" screen */
		    ioc.device = -1;	/* console pointer */
		ioc.data = wsTabletOverhang;
#ifdef DEBUG
	ErrorF("wsMouseProc: SET_TABLET_OVERHANG\n");
#endif
		if (ioctl(wsFd, SET_TABLET_OVERHANG, &ioc) < 0)
		    ErrorF("wsMouseProc: can't set tablet overhang\n");
	    }
#endif
	    break;
	case DEVICE_ON: 
	    pDev->on = TRUE;
	    AddEnabledDevice(wsFd); 
	    break;
	case DEVICE_OFF: 
	    pDev->on = FALSE;
	    RemoveEnabledDevice(wsFd);
	    break;
	case DEVICE_CLOSE:
	    break;
    }
    return Success;
}

/* since this driver does up/down autorepeat right, any key can be a modifier*/
/*ARGSUSED*/
Bool
LegalModifier(key, pDev)
    BYTE key;
    DevicePtr	pDev;
{
    return TRUE;
}

int
wsKeybdProc(pDev, onoff, argc, argv)
    DevicePtr pDev;
    int onoff, argc;
    char *argv[];
{
    KeySymsRec keySyms;
    CARD8 modMap[MAP_LENGTH];

    switch (onoff)
    {
	case DEVICE_INIT: 
/* Note that keyclick is off by default.  The QDSS MIT server sets it
   to 20 */	
	    wsKeyboard = pDev;
	    GetKeyboardMappings( &keySyms, modMap);
	    InitKeyboardDeviceStruct(
		    wsKeyboard, &keySyms, modMap, wsBell,
		    wsChangeKeyboardControl);
    /* Free the key sym mapping space allocated by GetKeyboardMappings. */
	    Xfree(keySyms.map);  
	    break;
	case DEVICE_ON: 
	    pDev->on = TRUE;
	    AddEnabledDevice(wsFd); 
	    break;
	case DEVICE_OFF: 
	    pDev->on = FALSE;
	    RemoveEnabledDevice(wsFd);
	    break;
	case DEVICE_CLOSE: 
	    break;
    }
    return Success;
}
extern int screenIsSaved;
static CursorPtr	currentCursor;


/* The code below is for backward compatibility with DEC R3 servers.
 * Load the keyboard map pointed to by the file "/usr/lib/X11/keymap_default"
 */

#define FileNameLength 256
#define MaxLineLength 256


KeySym *LoadKeymap();

/*
 * Load the default keymap file.
 */

int GetDefaultMap(ks, min_kc, max_kc)
    KeySymsPtr ks;
    int min_kc, max_kc;
{
    char keymap_name[FileNameLength];
    KeySym *keymap_pointer;
    int keymap_width;
    int keymap_loaded = FALSE;

    if (GetKeymapName (keymap_name)==TRUE) {
	if ((keymap_pointer = LoadKeymap (keymap_name, min_kc, max_kc,
					  &keymap_width)) !=NULL) {
	    ks->minKeyCode = min_kc;
	    ks->maxKeyCode = max_kc;
	    ks->mapWidth = keymap_width;
	    ks->map = keymap_pointer;
	    keymap_loaded=TRUE;
	}
    }
    return (keymap_loaded);
}


/*
 * Check for keymap type file "/usr/lib/X11/keymap_default"
 */

int GetKeymapName (name_return)
    char *name_return;
{
    char *filename = "/usr/lib/X11/keymap_default";
    int fd;
    int len;

    if ((access(filename, R_OK)) == -1)
	return (FALSE);
    len = strlen(filename);
    strcpy(name_return, filename);
    name_return[len] = '\0';
    return (TRUE);
}


#define EndLine(c) (((c)=='!' || (c) =='\n' || (c) == '\0') ? TRUE : FALSE )

/*
 * load the keymap file into  keysym table
 */

KeySym *LoadKeymap(keymap_name, minkc, maxkc, return_ks_per_kc)
    char *keymap_name;
    KeyCode minkc;
    KeyCode maxkc;
    int *return_ks_per_kc;
{
    FILE *fp;
    KeySym *keymap = NULL;
    char line[MaxLineLength];

    if ((fp=fopen (keymap_name, "r")) == NULL)
	return ( (KeySym *) NULL);

    while (fgets (line, MaxLineLength, fp) != NULL) {
	if (AddLineToKeymap (line, &keymap, minkc, maxkc, return_ks_per_kc)
								==FALSE) {
	    fclose (fp);
	    if (keymap != NULL) Xfree (keymap);
	    return ( (KeySym *)NULL);
	}
    }
    fclose (fp);
    return ( keymap );
}    


/*
 * decode keycode, and keysyms from line, allocate keymap first time round.
 */

int AddLineToKeymap (line, keymap, minkc, maxkc, return_ncols)
    char *line;
    KeySym **keymap;
    KeyCode minkc;
    KeyCode maxkc;
    int *return_ncols;
{
    int pos;
    KeyCode kc;
    KeySym *offset;
    KeySym ks;
    int ncols;
    int col;
    int map_size;
    int i;

    if (isspace(line[0]) || line[0] == '!' || line[0] == '\n' || line[0] == '\0')
	return (TRUE); /* ignore blank lines and comments */

    if ( *keymap ==NULL) {
	pos=0;
	if ((kc=GetToken (line, &pos)) == -1) return (FALSE);
	ncols=0;
	while (GetToken (line, &pos) != -1)
	    ncols++;
	if (ncols < 2) ncols = 2;
	(*return_ncols) = ncols;
	map_size = (maxkc-minkc+1) * ncols;
	(*keymap) = (KeySym *) Xalloc ( map_size * sizeof (KeySym));
	for (i = 0; i < map_size; i++)
	    (*keymap)[i] = NoSymbol;

    }
    pos = 0;
    if ((kc=GetToken (line, &pos)) == -1) return (FALSE);
    if ( kc < minkc || kc > maxkc ) return (TRUE); /* ignore, but keep going */
    offset = (*keymap) + (kc-minkc)* (*return_ncols);
    col=0;
    while (col < (*return_ncols) && ((ks=GetToken (line, &pos)) != -1)) {
	*(offset + col) = ks;
	col+=1;
    }
    return (TRUE);
}

/*
 * return hex value of next item on line (current position held in 'pos')
 */

int GetToken (line, pos)
    char *line;
    int *pos;
{
    int start;

    if (EndLine(line[*pos]) == TRUE) return (-1);
    while (isspace(line[*pos]) || EndLine(line[*pos])) {
	if (EndLine(line[*pos]) == TRUE)
		return (-1);
	(*pos)++;
    }
    start = *pos;
    while (!isspace (line[*pos]) && !EndLine (line[*pos])) {
	(*pos)++;
    }
    return (StringToHex(&line[start], (*pos)-start));
}


/*
 * convert null terminated hexadecimal string to integer
 * return 'value', or '-1' on error
 */

int StringToHex (str,nbytes)
    char *str;
    int nbytes;
{
    int i;
    int digit;
    int scale = 1;
    int value = 0;

    for (i=nbytes-1;i>=0;i--) {
	if (!isxdigit(str[i])) return (-1);
	if (isdigit(str[i]))
		digit=str[i]-'0';
	else
		digit=toupper(str[i])-'A'+10;
	value+=(digit*scale);
	scale*=16;
    }
    return(value);
}

#undef EndLine
#undef MaxLineLength
#undef FileNameLength


Bool GetKeyboardMappings(pKeySyms, pModMap)
    KeySymsPtr pKeySyms;
    CARD8 *pModMap;
{
    int i;
    ws_keyboard_definition def;
    KeySym *map;
    KeySym rawsyms[256];		/* more than we'll ever need! */
    unsigned char rawcodes[256];	/* more than we'll ever need! */
    ws_keycode_modifiers mods[32];	/* more than we'll ever need! */
    ws_keysyms_and_modifiers km;
    int min_keycode = 256, max_keycode = 0;
    int keymap_loaded = FALSE;

    def.device_number = wsinfo.console_keyboard;
    if (ioctl (wsFd, GET_KEYBOARD_DEFINITION, &def) == -1) {
	ErrorF ("error getting keyboard definition\n");
    }
    km.device_number = wsinfo.console_keyboard;
    km.modifiers = mods;
    *((KeySym **)(&km.keysyms)) = rawsyms; /* XXX bad type in inputdevice.h */
    km.keycodes = rawcodes;
    if (ioctl (wsFd, GET_KEYSYMS_AND_MODIFIERS, &km) == -1) {
	ErrorF ("error getting keysyms and modifiers\n");
    }

    /*
     * Always see what kernel thinks are the minimum and maximum keycodes,
     *  and use them later when parsing the default keymap file (if any).
     */
    for (i = 0; i < def.keysyms_present; i++) {
	    if (rawcodes[i] > max_keycode) max_keycode = rawcodes[i];
	    if (rawcodes[i] < min_keycode) min_keycode = rawcodes[i];
    }

   for (i = 0; i < MAP_LENGTH; i++)
        pModMap[i] = NoSymbol;  /* make sure it is restored */

    /* first set up modifier keys */
    for (i = 0; i < def.modifier_keycode_count; i++)
        pModMap[mods[i].keycode] = mods[i].modbits;

    /* If it exists, use special keysym map from file instead of driver.
	This is for backward compatibility with the i18n stuff from the
	DEC R3 servers.
    */
    keymap_loaded = GetDefaultMap(pKeySyms, min_keycode, max_keycode);

    if (!keymap_loaded) {
        map = (KeySym *)Xalloc(sizeof(KeySym) * 
	  	        (MAP_LENGTH * def.keysyms_per_keycode));
        if (!map)
	    return FALSE;

        /* initialize the keysym array */
        for (i = 0; i < (MAP_LENGTH * def.keysyms_per_keycode); i++)
	    map[i] = NoSymbol;
        pKeySyms->minKeyCode = min_keycode;
        pKeySyms->maxKeyCode = max_keycode;
        pKeySyms->mapWidth   = def.keysyms_per_keycode;
        pKeySyms->map = map;
#define INDEX(in) ((in - min_keycode) * def.keysyms_per_keycode)
        for (i = 0; i < def.keysyms_present; i++) {
	    register int j;
	    for (j = 0; j < def.keysyms_per_keycode; j++) {
		    if (map[INDEX(rawcodes[i]) + j] == NoSymbol) {
		            map[INDEX(rawcodes[i]) + j] = rawsyms[i];
			    break;
		    }
	    }
        }
#undef INDEX
    }
    /* 
     * Now If we find Num_Lock or Mode_Switch keysyms, add the
     * appropriate default modifier mask to its entry.
     */
    map = pKeySyms->map;
#define INDEX(in) ((in - pKeySyms->minKeyCode) * pKeySyms->mapWidth)
    for (i=pKeySyms->minKeyCode; i < pKeySyms->maxKeyCode; i++) {
        register int j;
        register KeySym ks;
        for (j=0; j< pKeySyms->mapWidth; j++) {
            ks = map[INDEX(i)+j];
            if (ks == XK_Num_Lock) {
                pModMap[i] = NumLockMask;
            }
            if (ks == XK_Mode_switch) {
                pModMap[i] = ModeSwitchMask;
            }
        }
    }
#undef INDEX
    return TRUE;
}


void
SetLockLED (on)
    Bool on;
{
    ws_keyboard_control kc;
    kc.flags = WSKBLed;
    kc.device_number = wsinfo.console_keyboard;
    if (ioctl (wsFd, GET_KEYBOARD_CONTROL, &kc) == -1) {
        ErrorF ("error getting keyboard control\n");
    }
    if(on)
        kc.leds |= WSKBLed_CapsLock;
    else
	kc.leds &= ~WSKBLed_CapsLock;
    kc.flags = WSKBLed;
    if (ioctl (wsFd, SET_KEYBOARD_CONTROL, &kc) == -1) {
        ErrorF ("error setting keyboard control\n");
    }
}

void
SetNumLockLED (on)
    Bool on;
{
    ws_keyboard_control kc;
    kc.flags = WSKBLed;
    kc.device_number = wsinfo.console_keyboard;
    if (ioctl (wsFd, GET_KEYBOARD_CONTROL, &kc) == -1) {
        ErrorF ("error getting keyboard control\n");
    }
    if(on)
        kc.leds |= WSKBLed_NumLock;
    else
	kc.leds &= ~WSKBLed_NumLock;
    kc.flags = WSKBLed;
    if (ioctl (wsFd, SET_KEYBOARD_CONTROL, &kc) == -1) {
        ErrorF ("error setting keyboard control\n");
    }
}

/*
 * The driver has been set up to put events in the queue that are identical
 * in shape to the events that the DDX layer has to deliver to ProcessInput
 * in DIX.
 */
extern void * GetSpriteCursor();

void
ProcessInputEvents()
{
    xEvent x;
    register ws_event *e;
    register int    i;
    int screen;
    DeviceIntPtr dev = (DeviceIntPtr) wsKeyboard;
    i = queue->head;
    while (i != queue->tail)  {
	e = (ws_event *)((caddr_t)(queue->events) + queue->event_size * i);

	if (screenIsSaved == SCREEN_SAVER_ON)
	    SaveScreens(SCREEN_SAVER_OFF, ScreenSaverReset);

	if (wsScreens[e->screen] == NULL)
	{
	    if (i >= queue->size - 1)   i = queue->head = 0;
	    else                        i = ++queue->head;
	    continue;
	}

   	if(e->screen != rememberedScreen)
	{
		ScreenPtr	pScreen;
		int		x, y;
		CursorPtr cursor = GetSpriteCursor();

		if (i >= queue->size - 1)   i = queue->head = 0;
		else			    i = ++queue->head;
		if (cursorConfined)
		{
		    /* OS doesn't work right -- we have to confine here */
		    GetSpritePosition (&x, &y);
		    pScreen = wsScreens[rememberedScreen];
		    (void) (*pScreen->SetCursorPosition) (pScreen, x, y, FALSE);
		    wspCursorControl(e->screen, CURSOR_OFF);
		    wspCursorControl(rememberedScreen, CURSOR_ON);
		    if (cursor)
			(*pScreen->DisplayCursor)(pScreen, cursor);
		}
		else
		{
		    /* assumption -- this is a motion event */
		    wspCursorControl(rememberedScreen, CURSOR_OFF);
		    wspCursorControl(e->screen, CURSOR_ON);
		    rememberedScreen = e->screen;
		    pScreen = wsScreens[e->screen];
		    if (cursor)
			(*pScreen->DisplayCursor)(pScreen, cursor);
		    x = e->e.key.x;
		    y = e->e.key.y;
		    NewCurrentScreen(pScreen, x, y);
		}
		i = queue->head;
		continue;
    	}

	x.u.keyButtonPointer.rootX = e->e.key.x;
	x.u.keyButtonPointer.rootY = e->e.key.y;
	x.u.keyButtonPointer.time = lastEventTime = e->time;
	x.u.u.detail = e->e.key.key;

	switch (e->device_type) {
	  case KEYBOARD_DEVICE:
		    switch (e->type) {
			case BUTTON_DOWN_TYPE: 
			    x.u.u.type = KeyPress;
			    /* if key is a lock modifier then ... */
			    if (dev->key->modifierMap[e->e.key.key] & LockMask){
				if (shiftLock) {
				    x.u.u.type = KeyRelease;
				    SetLockLED (FALSE);
				    shiftLock = FALSE;
				} else {
				    x.u.u.type = KeyPress;
				    SetLockLED (TRUE);
				    shiftLock = TRUE;
				}
			    }
			    /* if key is a num-lock modifier then ... */
			    if (dev->key->modifierMap[e->e.key.key] & NumLockMask) {
       			        if (numLock) {
        			    x.u.u.type = KeyRelease;
	        		    SetNumLockLED (FALSE);
		        	    numLock = FALSE;
			        } else {
			            x.u.u.type = KeyPress;
			            SetNumLockLED (TRUE);
			            numLock = TRUE;
			        }
			    }
#ifdef MODE_SWITCH
			    ProcessKeyboardInput(&x, wsKeyboard);
#else
			    (*wsKeyboard->processInputProc) 
				(&x, wsKeyboard, 1);
#endif
			    break;
			case BUTTON_UP_TYPE: 
                            /* if key is a lock modifier then ignore */
                            if (dev->key->modifierMap[e->e.key.key] & LockMask)
                                break;
			    /* if key is a numlock modifier and numLock mod is on then ignore */
                           if (dev->key->modifierMap[e->e.key.key] & NumLockMask)
				break;
			    x.u.u.type = KeyRelease;
#ifdef MODE_SWITCH
			    ProcessKeyboardInput(&x, wsKeyboard);
#else
			    (*wsKeyboard->processInputProc)
				(&x, wsKeyboard, 1);
#endif
			    break;
			default: 	       /* hopefully BUTTON_RAW_TYPE */
			    break;
		    }
		    break;
	    case MOUSE_DEVICE:
	    /* someday tablet will be handled differently than a mouse */
	    case TABLET_DEVICE:
		    switch (e->type) {
			case BUTTON_DOWN_TYPE: 
			    x.u.u.type = ButtonPress;
			    break;
			case BUTTON_UP_TYPE: 
			    x.u.u.type = ButtonRelease;
			    break;
			case MOTION_TYPE: 
			    x.u.u.type = MotionNotify;
			    break;
			default: 
#ifdef DEBUG
			    ErrorF("Unknown mouse or tablet event = %d\n",
				e->type);
#endif
			    goto out;
		    }
		    (*wsPointer->processInputProc) (&x, wsPointer, 1);
		    break;
	    /* new input devices go here (or are ignored) */
	    default:
#ifdef XINPUT
		    if (!ExtProcessInputEvents(&x, e))
#  ifdef DEBUG
		      ErrorF("Unknown device type = %d\n",e->device_type);
#  else
		      ; /* do nothing */
#  endif
#else
#  ifdef DEBUG
		    ErrorF("Unknown device type = %d\n",e->device_type);
#  endif
#endif
		break;
	}
out:
	if (i >= queue->size - 1)   i = queue->head = 0;
	else			    i = ++queue->head;

    }
}

#if LONG_BIT == 64
int
#else
long
#endif
GetTimeInMillis()
{
    struct timeval  tp;
#ifdef VSYNCFIXED
    if (queue)
	return queue->time;
#endif
    gettimeofday(&tp, 0);
    return(tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}

TimeSinceLastInputEvent()
{
    if (lastEventTime == 0)
	lastEventTime = CURRENT_TIME;
    return CURRENT_TIME -  lastEventTime;
}

SetTimeSinceLastInputEvent ()
{
    lastEventTime = CURRENT_TIME;
}

/*
 * set the bounds in the device for this particular cursor
 */
static void
wsConstrainCursor( pScr, pBox)
    ScreenPtr	pScr;
    BoxPtr	pBox;
{
    ws_pointer_box wsbox;
    wsbox.screen = WS_SCREEN(pScr);
    wsbox.enable = PointerConfinedToScreen();
    cursorConfined = wsbox.enable;
    wsbox.device_number = wsinfo.console_pointer;
    wsbox.box.bottom = pBox->y2;
    wsbox.box.right = pBox->x2;
    wsbox.box.top = pBox->y1;
    wsbox.box.left = pBox->x1;

    if (ioctl(wsFd, SET_POINTER_BOX, &wsbox) == -1)
	    ErrorF("SET_POINTER_BOX: failed to set pointer box.\n");
}

static Bool
wsSetCursorPosition( pScr, newx, newy, generateEvent)
    ScreenPtr	pScr;
    unsigned int	newx;
    unsigned int	newy;
    Bool		generateEvent;
{
    ws_pointer_position pos;
    xEvent motion;

    pos.screen = WS_SCREEN(pScr);
    pos.device_number = wsinfo.console_pointer;
    pos.x = newx;
    pos.y = newy;

    /* if this is on a different screen, then we need to switch... */
    if (pos.screen != rememberedScreen) {
		wspCursorControl(rememberedScreen, CURSOR_OFF);
		wspCursorControl(pos.screen, CURSOR_ON);
		rememberedScreen = pos.screen;
    }
    if (ioctl (wsFd, SET_POINTER_POSITION, &pos) == -1) {
	ErrorF ("error warping cursor\n");
	return FALSE;
    }

    if (generateEvent) {
	if (queue->head != queue->tail)
	    ProcessInputEvents ();
	motion.u.keyButtonPointer.rootX = newx;
	motion.u.keyButtonPointer.rootY = newy;
	motion.u.keyButtonPointer.time = currentTime.milliseconds;
	motion.u.u.type = MotionNotify;
	(wsPointer->processInputProc)(&motion, wsPointer, 1);
    }
    return TRUE;
}

static Bool
wsDisplayCursor( pScr, pCurs)
    ScreenPtr	pScr;
    CursorPtr	pCurs;
{
    ws_cursor_data cd;
    ws_cursor_color cc;
    cd.screen = WS_SCREEN(pScr);
    cd.width = pCurs->bits->width;
    cd.height = pCurs->bits->height;
    cd.x_hot =  pCurs->bits->xhot;
    cd.y_hot =  pCurs->bits->yhot;
    if (sizeof (long) != 4 && cd.width <= 32 ) {
	register unsigned int source[64], mask[64];
	register int i;
	for( i = 0 ; i < cd.height ; i++ ) {
	    source[i] = (unsigned int)
		(((unsigned long *)pCurs->bits->source)[i]);
	    mask[i] = (unsigned int)
		(((unsigned long *)pCurs->bits->mask)[i]);
	}
        cd.cursor = (unsigned int *) source;
        cd.mask =   (unsigned int *) mask;
        if ( ioctl( wsFd, LOAD_CURSOR, &cd) == -1)    {
	    ErrorF( "error loading bits of new cursor\n");
            return FALSE;
        }
    }
    else {
        cd.cursor = (unsigned int *) pCurs->bits->source;
        cd.mask =   (unsigned int *) pCurs->bits->mask;
        if ( ioctl( wsFd, LOAD_CURSOR, &cd) == -1)    {
	    ErrorF( "error loading bits of new cursor\n");
            return FALSE;
        }
    }
    cc.screen = WS_SCREEN(pScr);
    cc.background.red = pCurs->backRed;
    cc.background.green = pCurs->backGreen;
    cc.background.blue = pCurs->backBlue;
    cc.foreground.red = pCurs->foreRed;
    cc.foreground.green  = pCurs->foreGreen;
    cc.foreground.blue = pCurs->foreBlue;
    if ( ioctl(wsFd, RECOLOR_CURSOR, &cc) == -1) {
	ErrorF( "error writing colors of new cursor\n");
        return FALSE;
    }
    currentCursor = pCurs;
    return (TRUE);
}

void 
wsCursorControl(screen, control)
    int screen;
    int control;
{
    ws_cursor_control cc;
    cc.screen = screen;
    cc.control = control;
    if (ioctl(wsFd, CURSOR_ON_OFF, &cc) == -1) {
	ErrorF( "error enabling/disabling cursor\n");
    }
    return;
}
static void
wsRecolorCursor (pScr, pCurs, displayed)
    ScreenPtr	pScr;
    CursorPtr	pCurs;
    Bool	displayed;
{
    ws_cursor_color cc;
    if (!displayed)
	return;
    cc.screen = WS_SCREEN(pScr);
    cc.background.red = pCurs->backRed;
    cc.background.green = pCurs->backGreen;
    cc.background.blue = pCurs->backBlue;
    cc.foreground.red = pCurs->foreRed;
    cc.foreground.green  = pCurs->foreGreen;
    cc.foreground.blue = pCurs->foreBlue;
    if ( ioctl(wsFd, RECOLOR_CURSOR, &cc) == -1)    {
	ErrorF( "error writing colors of new cursor\n");
    }
}

static Bool
wsRealizeCursor( pScr, pCurs)
    ScreenPtr	pScr;
    CursorPtr	pCurs;	/* The new driver makes this easy */
{
    return TRUE;
}

static Bool
wsUnrealizeCursor( pScr, pCurs)
    ScreenPtr	pScr;
    CursorPtr	pCurs;
{
    if (pCurs == currentCursor)
	currentCursor = 0;
    return TRUE;
}

static void
wsCursorLimits( pScr, pCurs, pHotBox, pPhysBox)
    ScreenPtr	pScr;
    CursorPtr	pCurs;
    BoxPtr	pHotBox;
    BoxPtr	pPhysBox;	/* return value */
{
    wsScreenPrivate *wsp = (wsScreenPrivate *)
		pScr->devPrivates[wsScreenPrivateIndex].ptr;
    pPhysBox->x1 = max( pHotBox->x1, 0);
    pPhysBox->y1 = max( pHotBox->y1, 0);
    pPhysBox->x2 = min( pHotBox->x2, wsp->screenDesc->width - 1);
    pPhysBox->y2 = min( pHotBox->y2, wsp->screenDesc->height - 1);
}

void
wsPointerNonInterestBox( pScr, pBox)
    ScreenPtr	pScr;
    BoxPtr	pBox;
{
    ws_pointer_box wsbox;
    wsbox.screen = WS_SCREEN(pScr);
    wsbox.device_number = wsinfo.console_pointer;
    wsbox.enable = TRUE;
    wsbox.box.top = pBox->x1;
    wsbox.box.bottom =  pBox->x2;
    wsbox.box.left = pBox->y1;
    wsbox.box.right = pBox->y2;;
    if (ioctl(wsFd, SET_ESCAPE_BOX, wsbox) == -1)
	    ErrorF("SET_ESCAPE_BOX: failed to set non interest box.\n");
}

#define PROC_LIST_SIZE 100
typedef void (*VoidProc) ();
static VoidProc 	wsGiveUpProcList[PROC_LIST_SIZE];
static VoidProc 	wsAbortProcList[PROC_LIST_SIZE];
static int 		wsGiveUpProcIndex = 0;
static int 		wsAbortProcIndex = 0;

void wsRegisterGiveUpProc(proc)
    VoidProc proc;
{
    if ( wsGiveUpProcIndex < PROC_LIST_SIZE )
	wsGiveUpProcList[wsGiveUpProcIndex++] = proc;
    else
	ErrorF("Out of ws give up proc handler space.\n");
}

void wsRegisterAbortProc(proc)
    VoidProc proc;
{
    if ( wsAbortProcIndex < PROC_LIST_SIZE )
	wsAbortProcList[wsAbortProcIndex++] = proc;
    else
	ErrorF("Out of ws give up proc handler space.\n");
}
/*
 * DDX - specific abort routine.  Called by AbortServer().
 */
void
AbortDDX()
{
    register int i;
    for (i = 0; i < wsAbortProcIndex; i++)
	if (wsAbortProcList[i])
	    (* wsAbortProcList[i])();
}

/* Called by GiveUp(). */
void
ddxGiveUp()
{
    register int i;
    for (i = 0; i < wsGiveUpProcIndex; i++)
	if (wsGiveUpProcList[i])
	    (* wsGiveUpProcList[i])();

#ifdef DEBUG
    ErrorF("ddxGiveUp(): closing /dev/mouse\n");
#endif DEBUG
    /*
     * The following statements are needed to give time for
     * the PX[G] accelerators to terminate gracefully.
     */
#ifndef NO_CLOSE
    close(wsFd);
    sleep(2);
#endif /*~NO_CLOSE*/
}

int
ArgMatch(arg, template, screen)
    char *arg;
    char *template;
    int *screen;
{
    int tlen = strlen(template);
    char next = *(arg + tlen);
    if(strncmp(arg, template, tlen) == 0) {
        if(tlen == strlen(arg)) {
	    /* exact match - applies to all screens */
	    *screen = -1;
	    return TRUE;
	}
	if(isdigit(next)) {
	    /* parse off screen number */
	    *screen = atoi(arg + tlen);
	    if(*screen < MAXSCREENS)
		return TRUE;
	    else
	        return FALSE;
	}
	else
	    /* non-digit stuff at end of arg.  not ours. */
	    return FALSE;
    }
    else 
	return FALSE;
}


int
ddxProcessArgument (argc, argv, i)
    register int argc;
    register char *argv[];
    register int i;
{
    int			argind=i;
    int			skip;
    static int		Once=0;
    void		ddxUseMsg();
    int 		screen;

    skip = 0;
    if (!Once) {
	register int i;
	bzero(wsDisabledScreens, sizeof(int) * MAXSCREENS);
        blackValue = NULL;
        whiteValue = NULL;
	Once = 1;
    }
    if (ArgMatch( argv[argind], "-dpix", &screen)) {
	if (++argind < argc) {
	    if(screen == -1) {
		for(i = 0; i < MAXSCREENS; i++) {
		    screenArgs[i].flags |= ARG_DPIX;
		    screenArgs[i].dpix = atoi(argv[argind]);
		}
	    }
	    else {
		screenArgs[screen].flags |= ARG_DPIX;
		screenArgs[screen].dpix = atoi(argv[argind]);
	    }
	    skip = 2;
	}
	else return 0;	/* failed to parse */
    }
    else if (ArgMatch( argv[argind], "-dpiy", &screen)) {
	if (++argind < argc) {
	    if(screen == -1) {
		for(i = 0; i < MAXSCREENS; i++) {
		    screenArgs[i].flags |= ARG_DPIY;
		    screenArgs[i].dpiy = atoi(argv[argind]);
		}
	    }
	    else {
		screenArgs[screen].flags |= ARG_DPIY;
		screenArgs[screen].dpiy = atoi(argv[argind]);
	    }
	    skip = 2;
	}
	else return 0;
    }
    else if (ArgMatch( argv[argind], "-dpi", &screen)) {
	if (++argind < argc) {
	    if(screen == -1) {
		for(i = 0; i < MAXSCREENS; i++) {
		    screenArgs[i].flags |= (ARG_DPI | ARG_DPIX | ARG_DPIY) ;
		    screenArgs[i].dpi = screenArgs[i].dpix =
			screenArgs[i].dpiy = atoi(argv[argind]);
		}
	    }
	    else {
		screenArgs[screen].flags |= (ARG_DPI | ARG_DPIX | ARG_DPIY);
		screenArgs[screen].dpi = screenArgs[screen].dpix  =
		       screenArgs[screen].dpiy = atoi(argv[argind]);
	    }
	    skip = 2;
	}
	else return 0;
    }
    else if(ArgMatch( argv[argind], "-bp", &screen)) {
	if (++argind < argc) {
	    if(screen == -1) {
		for(i = 0; i < MAXSCREENS; i++) {
		    screenArgs[i].flags |= ARG_BLACKVALUE;
		    screenArgs[i].blackValue = argv[argind];
		}
	    }
	    else {
		screenArgs[screen].flags |=  ARG_BLACKVALUE;
		screenArgs[screen].blackValue = argv[argind];
	    }
	    skip = 2;
	}
	else return 0;
    }
    else if (ArgMatch( argv[argind], "-wp", &screen)) {
	if (++argind < argc) {
	    if(screen == -1) {
		for(i = 0; i < MAXSCREENS; i++) {
		    screenArgs[i].flags |= ARG_WHITEVALUE;
		    screenArgs[i].whiteValue = argv[argind];
		}
	    }
	    else {
		screenArgs[screen].flags |=  ARG_WHITEVALUE;
		screenArgs[screen].whiteValue = argv[argind];
	    }
	    skip = 2;
	}
	else return 0;
    }
    else if (ArgMatch(argv[argind], "-vclass", &screen))  {
	if(++argind < argc)  {
	    int class = ParseClass(argv[argind]);
	    if (class == NoSuchClass)
		return 0;
	    if(screen == -1) {
		for(i = 0; i < MAXSCREENS; i++) {
		    screenArgs[i].flags |= ARG_CLASS;
		    screenArgs[i].class = class;
		}
	    }
	    else {
		screenArgs[screen].flags |=  ARG_CLASS;
		screenArgs[screen].class = class;
	    }
	    skip = 2;
        }
	else return 0;
    }
    else if (ArgMatch(argv[argind], "-edge_left", &screen))  {
	if(++argind < argc)  {
	    if(screen == -1) {
		return 0;
	    }
	    else {
		screenArgs[screen].flags |=  ARG_EDGE_L;
		screenArgs[screen].edge_left = atoi(argv[argind]);
	    }
	    skip = 2;
        }
	else return 0;
    }
    else if (strcmp( argv[argind], "-forceDepth") == 0)
    {
	if (++argind < argc) {
	    forceDepth = atoi (argv[argind]);
	    skip = 2;
	}
    }
    else if (ArgMatch(argv[argind], "-edge_right", &screen))  {
	if(++argind < argc)  {
	    if(screen == -1) {
		return 0;
	    }
	    else {
		screenArgs[screen].flags |=  ARG_EDGE_R;
		screenArgs[screen].edge_right = atoi(argv[argind]);
	    }
	    skip = 2;
        }
	else return 0;
    }
    else if (ArgMatch(argv[argind], "-edge_top", &screen))  {
	if(++argind < argc)  {
	    if(screen == -1) {
		return 0;
	    }
	    else {
		screenArgs[screen].flags |=  ARG_EDGE_T;
		screenArgs[screen].edge_top = atoi(argv[argind]);
	    }
	    skip = 2;
        }
	else return 0;
    }
    else if (ArgMatch(argv[argind], "-edge_bottom", &screen))  {
	if(++argind < argc)  {
	    if(screen == -1) {
		return 0;
	    }
	    else {
		screenArgs[screen].flags |=  ARG_EDGE_B;
		screenArgs[screen].edge_bottom = atoi(argv[argind]);
	    }
	    skip = 2;
        }
	else return 0;
    }

    else if (ArgMatch(argv[argind], "-monitor", &screen))  {
	if(++argind + 4 < argc)  {
	    if(strlen(argv[argind]) == 5) {
		strcpy(screenArgs[screen].monitor.type, argv[argind++]);
		screenArgs[screen].monitor.mm_width =  atoi(argv[argind++]);
		screenArgs[screen].monitor.mm_height =  atoi(argv[argind++]);
		screenArgs[screen].monitor.color_or_mono =atoi(argv[argind++]);
		screenArgs[screen].monitor.phosphor_type =atoi(argv[argind++]);
		screenArgs[screen].flags |=  ARG_MONITOR;
	        skip = 6;
	    }
	    else return 0;
        }
	else return 0;
    }
    else if (strcmp( argv[argind], "-btn") == 0)
    {
	if (++argind < argc)
	{
	    wsNumButtons = atoi(argv[argind]);
	    skip = 2;
	    if(wsNumButtons < 1 || wsNumButtons > 5)
		return 0;
	}
	else return 0;
    }
#ifdef WSPX
    else if (strcmp( argv[argind], "-tablet_overhang") == 0) {
	/*
	 * tablet borders apply to all screens (global in the
	 * driver) so we don't need to parse for screen #.
	 */
	if (++argind < argc)
	{
	    wsTabletOverhang = atoi(argv[argind]);
	    /*
	     * This is about all the checking we can do now, since wsFd
	     * ain't open yet...
	     */
	    if (wsTabletOverhang < 0 || wsTabletOverhang > 10)
		return 0;
	    skip = 2;
	}
	else return 0;
    }
#endif
    else if (ArgMatch(argv[argind], "-enableScreen", &screen))  {
	    if(screen == -1) {
		return 0;
	    }
	    else {
		wsEnableScreen(screen);
	    }
	    skip = 1;
    }
    else if (ArgMatch(argv[argind], "-disableScreen", &screen))  {
	    if(screen == -1) {
		return 0;
	    }
	    else {
		wsDisableScreen(screen);
	    }
	    skip = 1;
    }
    else if (ArgMatch(argv[argind], "-onlyScreen", &screen))  {
	    if(screen == -1) {
		return 0;
	    }
	    else {
		/* Override any disable previously processed */
		wsEnableScreen(screen);
		wsMakeScreenOnly(screen);
	    }
	    skip = 1;
    }
    else if (strcmp(argv[argind], "-screenOrder") == 0 ) {
	    if (++argind < argc) {
		if (wsParseScreenList(argv[argind]) == FALSE )
			UseMsg();
	        skip = 2;
	    } else return 0;
    }
#ifdef XINPUT
    else
    skip = ExtddxProcessArgument(argc, argv, argind);
#endif

    return skip;
}

#define ARGFILE "/etc/screens"

void getFileArguments()
{
    FILE *f =  fopen(ARGFILE, "r");
    if(f) {
        struct stat stats;
	if(stat(ARGFILE, &stats) == 0) {
	    int i, argcount = 0, skip;
	    char *ptr, *buf = (char *)Xalloc(stats.st_size);
	    char **arguments;
	    while(EOF != fscanf(f, "%s", buf)) argcount++;
	    arguments =  (char **) Xalloc(argcount * sizeof(char *));
    	    rewind(f);
	    for(ptr = buf, i = 0; i < argcount; i++) {
	        arguments[i] = ptr;
		fscanf(f, "%s", ptr);
		ptr += strlen(arguments[i]) + 1;
	    }
	    fclose(f);
	    for(i = 0; i < argcount; i++) 
		if(skip = ddxProcessArgument(argcount, arguments, i))
		    i +=  (skip - 1);
	    Xfree(arguments);
	    Xfree(buf);
	}	
    }
}


void
ddxUseMsg()
{
    ErrorF ("\n");
    ErrorF ("\n");
    ErrorF ("Device Dependent Usage\n");
    ErrorF ("Note - most ddx arguments can take an optional screen number ``s''\n");
    ErrorF ("The screen number refers to the physical screen number.\n");
    ErrorF ("\n");
    ErrorF ("-btn <n>              Number of buttons on pointer device\n");
    ErrorF ("-dpix[s] <n>          Dots per inch, x coordinate\n");
    ErrorF ("-dpiy[s] <n>          Dots per inch, y coordinate\n");
    ErrorF ("-dpi[s] <n>           Dots per inch, x and y coordinates\n");
    ErrorF ("                   (overrides -dpix and -dpiy above)\n");
    ErrorF ("-bp[s] #XXX           color for BlackPixel for screen\n");
    ErrorF ("-wp[s] #XXX           color for WhitePixel for screen\n");
    ErrorF ("-vclass[s] <classname> type of Visual for root window\n");
    ErrorF ("       one of StaticGray, StaticColor, PseudoColor,\n");
    ErrorF ("       GrayScale, or even TrueColor!\n");
    ErrorF ("-edge_left<s1> <s2> Attach left edge of screen s1 to screen s2\n");
    ErrorF ("-edge_right<s1> <s2> Attach right edge of screen s1 to screen s2\n");
    ErrorF ("-edge_top<s1> <s2> Attach top edge of screen s1 to screen s2\n");
    ErrorF ("-edge_bottom<s1> <s2> Attach bottom edge of screen s1 to screen s2\n");
    ErrorF ("-onlyScreen<s>        make screen s only enabled screen\n");
    ErrorF ("-disableScreen<s>     disable screen s\n");
    ErrorF ("-enableScreen<s>      override screen disabling\n");
    ErrorF ("-screenOrder <list>    list of physical screens to place in logical order\n");
    ErrorF ("     where <list> is a comma separated list.\n");
    ErrorF ("     if the list does not end in a period, all unlisted physical\n");
    ErrorF ("     screens will be added to the end of the logical order.\n");
    ErrorF ("     If the list ends in a period, all remaining physical screens\n");
    ErrorF ("     will be disabled\n");

#ifdef XINPUT
    ExtddxUseMsg();
#endif
}

/* ARGSUSED */
int wsScreenInit(index, pScreen, argc, argv)
    int index;
    register ScreenPtr pScreen;
    int argc;
    char **argv;
{

    wsScreenPrivate *wsp = WSP_PTR(pScreen);
    int psn = WS_SCREEN(pScreen);

    wsp->CursorControl = wsCursorControl;
    pScreen->SaveScreen = wsSaveScreen;
    pScreen->RealizeCursor = wsRealizeCursor;
    pScreen->UnrealizeCursor = wsUnrealizeCursor;
    pScreen->DisplayCursor = wsDisplayCursor;
    pScreen->SetCursorPosition = wsSetCursorPosition;
    pScreen->CursorLimits = wsCursorLimits;
    pScreen->PointerNonInterestBox = wsPointerNonInterestBox;
    pScreen->ConstrainCursor = wsConstrainCursor;
    pScreen->RecolorCursor = wsRecolorCursor;
    pScreen->StoreColors = wsStoreColors;
    pScreen->InstallColormap = wsInstallColormap;
    pScreen->UninstallColormap = wsUninstallColormap;
    pScreen->ListInstalledColormaps = wsListInstalledColormaps;

    if(screenArgs[psn].flags & ARG_MONITOR) {
	ws_monitor_type wmt;
	wmt.screen = psn;
	wmt.monitor_type = screenArgs[psn].monitor;
	if (ioctl(wsFd, SET_MONITOR_TYPE, &wmt) == -1)
	    ErrorF("SET_MONITOR_TYPE, failed to set monitor type.\n");
    }

    return index;
}

int wsOnlyScreen = -1;
int wsDisabledScreens[MAXSCREENS];
int wsPhysToLogScreens[MAXSCREENS];
int wsRemapPhysToLogScreens = FALSE;

void wsEnableScreen(psn)
    int psn;
{
    wsDisabledScreens[psn] = 0;
}
void wsDisableScreen(psn)
    int psn;
{
    wsDisabledScreens[psn] = 1;
}
void wsMakeScreenOnly(psn)
    int psn;
{
    if ( wsDisabledScreens[psn] == 0 )
        wsOnlyScreen = psn;
}
int wsPhysScreenNum(pScreen)
    ScreenPtr pScreen;
{
    if ( pScreen )
        return(WS_SCREEN(pScreen));
    else
	return -1;
}

/* Format:
 * comma separated list
 * each "slot" in the list represents a logical screen slot, starting
 *    at logical screen 0.
 * Each number represents a physical screen number (someday there can
 *    be a symbolic representation of this, such as fb0, fb1, px1, pv2)
 * If the list ends unterminated, then any unspecified physical screens
 *    will be filled in at the end.
 * If the list ends terminated in a period, then only those physical
 *    screens will be enabled.
 * If a physical screen is specified and it ends up not being enabled,
 *    it will be configured out later. Logical slots will be packed,
 *    i.e., no empty logical slots will exist.
 * If a logical slot is beyond the number of screens enabled, then that
 *    screen just ges slid down to pack the list.
 */
Bool wsParseScreenList(list)
    char * list;
{
    register int i;
    char * ptr, * baseptr;
    Bool terminated = FALSE;
    char * tail;
    int lsn = 0;
    Bool last = FALSE;

    ptr = (char *)Xalloc(sizeof(char)*(strlen(list)+1));
    strcpy(ptr, list);
    baseptr = ptr;

    /* initialize...*/
    for ( i = 0; i < MAXSCREENS; i++ )
	wsPhysToLogScreens[i] = -1;

    /* Can't have an empty list. */
    if ( *ptr == '.' ) {
	ErrorF("ws ScreenOrder list: bad format: list is empty\n");
	return FALSE;
    }
    while ( *ptr ) {
	tail = ptr;

	/* gether up some digits */
	while(isdigit(*tail)) tail++;

	/* If there isn't a digit here, complain */
	if ( ptr == tail ) {
	    ErrorF("ws ScreenOrder list: bad format: unexpected character\n");
	    ErrorF("     expecting a screen number\n");
	    ErrorF("     %s\n",list);
	    ErrorF("     %*s\n",ptr-baseptr+1,"^");
	    return FALSE;
        }

	/* After a digit, there should only be a period, comma, or end 
	 * of line
	 */
	if ( *tail == '.' ) terminated = TRUE;
	else if ( *tail != ',' && *tail != '\0' ) {
	    ErrorF("ws ScreenOrder list: bad format: unexpected character\n");
	    ErrorF("     list can only contain digits, commas, and periods\n");
	    ErrorF("     %s\n",list);
	    ErrorF("     %*s\n",tail-baseptr+1,"^");
	    return FALSE;
        }

	if ( *tail == '\0' ) 
	    last = TRUE;
	else
	    *tail = '\0';

	/* Make sure we have a good screen number */
	if ( atoi(ptr) > MAXSCREENS || atoi(ptr) < 0 ) {
	    ErrorF("ws ScreenOrder list: bad format\n");
	    ErrorF("    invalid screen %d: screen number greater than maximum (%d)\n",atoi(ptr), MAXSCREENS);
	    return FALSE;
	}

	/* check that there aren't duplicates in the list */
	if ( wsPhysToLogScreens[atoi(ptr)] != -1 ) {
	    ErrorF("ws ScreenOrder list: duplicate physical screens specified\n");
	    ErrorF("     %s\n",list);
	    ErrorF("     %*s\n",ptr-baseptr+1,"^");
	    return FALSE;
	}

	/* enter the logical mapping */
	wsPhysToLogScreens[atoi(ptr)] = lsn++;


	if ( last == TRUE )
	    break;
	ptr = tail+1;
    }

    /* If there was a terminated list, disable any screens that
     * haven't been specified */
    if ( terminated == TRUE ) {
	for ( i = 0; i < MAXSCREENS; i++ ) 
	    if ( wsPhysToLogScreens[i] < 0 )
	        wsDisabledScreens[i] = 1;
    }
    wsRemapPhysToLogScreens = TRUE;
    return TRUE;

}

/* This is borrowed from the dix. Don't know if the dix should be
 * changed or not, probably should, but for now we'll leave 
 * the dix as is.
 */
static void
FreeScreen(pScreen)
    ScreenPtr pScreen;
{
    if ( !pScreen ) return;
    xfree(pScreen->WindowPrivateSizes);
    xfree(pScreen->GCPrivateSizes);
    xfree(pScreen->devPrivates);
    xfree(pScreen);
}

/* Finish up things.
 * Do: final disabling of screens
 *     physical to logical remapping
 *     redo screenInfo and wsScreens to map new mapping
 *     tie up edge attachments so they make sense.
 */
void wsInputOutputFinish(numScreens)
    int numScreens;
{
    ws_edge_connection wec;
    int i;
    ScreenInfo newScreenInfo;
    ws_video_control vc;
    int disable;
    int screen;
    int NewRemap[MAXSCREENS];
    int NewNumScreens=0;
    int modify = 0;
    int mask;
    int lsn;		/* logical screen number */
    int psn;		/* physical screen number */

    /* determine which screens need to be disabled 
     * If only is done, then all are disabled except the only one.
     * otherwise, just use what's in the disabled list
     */
    if ( wsOnlyScreen >= 0 && wsOnlyScreen < numScreens ) {
        for ( i = 0; i < numScreens; i++ ) 
            wsDisabledScreens[i] = 1;
	wsDisabledScreens[wsOnlyScreen] = 0;
    } 

    /* Count the number of remaining screens and get ready to remap screens */
    for ( i = 0; i < numScreens; i++ ) {
	NewRemap[i] = -1;
	if ( wsDisabledScreens[i] == 0 ) NewNumScreens++;
    }

    if ( NewNumScreens == 0 )  {
	ErrorF("No screens left enabled\n");
	screenInfo.numScreens = 0;
	return;
    }
    else
	newScreenInfo.numScreens = NewNumScreens;

    /* make sure no physical numbers beyond those we have are set */
    for ( i = numScreens; i < MAXSCREENS; i++ )
            wsDisabledScreens[i] = 1;

    /* Turn off video to all disabled screens */
    /* NOTE: this is not really disabling them, but at this
     * point we don't have enough of a handle to go back and
     * unconfigure things, unload libraries, etc. The screens
     * should have been turned off on the command line if 
     * the user didn't want them in which case we would
     * not have to turn them off here, but if some ddx
     * comes up and wants to run alone and issues a MakeOnly
     * then we have to at least appear to disable everything 
     * else.
     * If we had some way of doing this in the driver, it
     * might make things better. We might also consider issuing
     * a CloseScreen and then unloading the library. TBD
     */
    vc.control = SCREEN_OFF;
    for ( i = 0; i < numScreens; i++ )  {
	if ( wsDisabledScreens[i] == 1 ) {
            vc.screen = i;
	    if (ioctl(wsFd, VIDEO_ON_OFF, &vc) < 0)
	        ErrorF("VIDEO_ON_OFF: failed to turn screen off.\n");
	}
    }

    /* Remove all unnessary entries in the dix notion of screens */
    /* NOTE: again, some type of close screen should be used here */
    for ( i = 0; i < screenInfo.numScreens; i++ ) {
	newScreenInfo.screens[i] = (ScreenPtr) NULL;    /* init newScreen */
	screen = WS_SCREEN(screenInfo.screens[i]);
	if ( wsDisabledScreens[screen] == 1 ) {
	    FreeScreen(screenInfo.screens[i]);
	    screenInfo.screens[i] = (ScreenPtr) NULL;
	}
    }

    /* Check the physical to logical remapping to make sure it is valid */
    if ( wsRemapPhysToLogScreens == TRUE ) {
	int ltp[MAXSCREENS];
	/* specified changes exist */
	/* check things out */
        for ( psn = 0; psn < MAXSCREENS; psn++ ) {
	    if ( wsPhysToLogScreens[psn] != -1 && wsDisabledScreens[psn] == 1 ){
	      ErrorF("Physical to logical screen mapping warning.\n");
	      ErrorF(
		"Physical screen %d is disabled but a mapping was provided.\n", 
		psn);
	      wsPhysToLogScreens[psn] = -1;
	    }
	    else if ( wsDisabledScreens[psn] == 0 && 
		 wsPhysToLogScreens[psn] >= NewNumScreens ) {
	          ErrorF("Physical to logical screen mapping warning.\n");
		  ErrorF("Mapping physical screen %d to logical screen %d: \n",
		      psn,wsPhysToLogScreens[psn]);
		  ErrorF("Logical screen is beyond current number of enabled screens (%d)\n",
		      NewNumScreens);
		  wsPhysToLogScreens[psn] = -1;
	    }
	}
	for (lsn = 0; lsn < MAXSCREENS; lsn++ )
	    ltp[lsn] = -1;
	/* convert physical to logical to logical to physical mapping */
	for (psn = 0; psn < numScreens; psn++ )
	    if ( wsPhysToLogScreens[psn] >= 0 )
	        ltp[wsPhysToLogScreens[psn]] = psn;
	
	/* now shift things down to pack in the logicals */
	lsn = 0;
	for (i = 0; i < MAXSCREENS; i++ )
	    if ( ltp[i] >= 0 )
		ltp[lsn++] = ltp[i];
	/* now add any physical screens left */
	for (psn = 0; psn < numScreens; psn++ )
	    if ( wsPhysToLogScreens[psn] < 0 && wsDisabledScreens[psn] == 0 )
		ltp[lsn++] = psn;

	/* now convert back to phys to logical mapping */
	for (lsn = 0; lsn < numScreens; lsn++ )
	    NewRemap[ltp[lsn]] = lsn;
    }

    /* Remap all the screens */
    if ( wsRemapPhysToLogScreens == TRUE ) {
        /* first, make a full copy to get non-remapped screens (e.g., "0,2") */
        for ( psn = 0; psn < numScreens; psn++ ) {
	    newScreenInfo.screens[psn] = screenInfo.screens[psn];
	}
	/* rearrange them all */
        for ( psn = 0; psn < numScreens; psn++ ) {
	    /* If there is a remapping for this phys screen...*/
	    if ( NewRemap[psn] >= 0 )  {
        	for ( lsn = 0; lsn < screenInfo.numScreens; lsn++ ) {
		    /* find the corresponding logical screen */
		    if ( screenInfo.screens[lsn] &&
		         WS_SCREEN(screenInfo.screens[lsn]) == NewRemap[psn]) {
			/* make new logical screen this old one */
    	        	newScreenInfo.screens[psn] = screenInfo.screens[lsn];
	    		newScreenInfo.screens[psn]->myNum = psn;
			break;
		    }
		}
	    }
	}
    } else {
	/* merely slide all the valid screen down to the lowest slot */
	int lastScreen = 0;
        for ( lsn = 0; lsn < screenInfo.numScreens; lsn++ ) {
	    if ( screenInfo.screens[lsn] ) {
		newScreenInfo.screens[lastScreen] = screenInfo.screens[lsn];
		newScreenInfo.screens[lastScreen]->myNum = lastScreen;
		lastScreen++;
	    }
	}
        for ( lsn = lastScreen; lsn < MAXSCREENS; lsn++ ) {
	    newScreenInfo.screens[lsn] = (ScreenPtr)NULL;
	}
    }

    /* Make sure each disabled screen is attached to screen 0, just in
     * case the pointer ever drifts
     */
    wec.adj_screens.left 	= -1;
    wec.adj_screens.right 	= WS_SCREEN(newScreenInfo.screens[0]);
    wec.adj_screens.top 	= -1;
    wec.adj_screens.bottom 	= -1;
    for ( psn = 0; psn < numScreens; psn++ )
	if ( wsDisabledScreens[psn] == 1 ) {
	    wec.screen = psn;
            if (ioctl(wsFd, SET_EDGE_CONNECTION, &wec) == -1)
	      ErrorF("SET_EDGE_CONNECTION, failed to set edge attachment.\n");
	}

    /* Make sure all other edge attachments make sense. If not,
     * the default is to attach from left to right, 0 through N logical,
     * nothing to each top and bottom, left most and rightmost with
     * nothing.
     */
    for ( lsn = 0; lsn < newScreenInfo.numScreens; lsn++ ) {
        ScreenArgsRec *args;
        int mask;

	wec.screen = screen = WS_SCREEN(newScreenInfo.screens[lsn]);
        args = &screenArgs[screen];
        mask = args->flags;

        if (ioctl(wsFd, GET_EDGE_CONNECTION, &wec) == -1)
	    ErrorF("GET_EDGE_CONNECTION, failed to get edge attachment.\n");

	if ( mask & ARG_EDGE_L && wsDisabledScreens[args->edge_left] == 0) 
	    wec.adj_screens.left = args->edge_left;
	else 
	    if ( lsn == 0 ) 
		wec.adj_screens.left = -1;
	    else
		wec.adj_screens.left = WS_SCREEN(newScreenInfo.screens[lsn-1]);

	if ( mask & ARG_EDGE_R && wsDisabledScreens[args->edge_right] == 0 )
	    wec.adj_screens.right = args->edge_right;
	else 
	    if ( lsn == (NewNumScreens-1) ) 
		wec.adj_screens.right = -1;
	    else
		wec.adj_screens.right = WS_SCREEN(newScreenInfo.screens[lsn+1]);

    	if(mask & ARG_EDGE_T && wsDisabledScreens[args->edge_top] == 0 )
	    wec.adj_screens.top = args->edge_top;
    	else
	    wec.adj_screens.top = -1;

    	if(mask & ARG_EDGE_B && wsDisabledScreens[args->edge_bottom] == 0 )
	    wec.adj_screens.bottom = args->edge_bottom;
    	else
	    wec.adj_screens.bottom = -1;

        if (ioctl(wsFd, SET_EDGE_CONNECTION, &wec) == -1)
	    ErrorF("SET_EDGE_CONNECTION, failed to set edge attachment.\n");
    }

    /* transfer new screen info to global structure */
    for ( lsn = 0; lsn < MAXSCREENS; lsn++ )  {
        screenInfo.screens[lsn] = newScreenInfo.screens[lsn];
    }
    screenInfo.numScreens = NewNumScreens;

    /* Remap the wsScreens */
    for ( psn = 0; psn < MAXSCREENS; psn++ ) 
	wsScreens[psn] = (ScreenPtr)NULL;
    for ( lsn = 0; lsn < screenInfo.numScreens; lsn++ ) 
	wsScreens[WS_SCREEN(screenInfo.screens[lsn])] = screenInfo.screens[lsn];

    /* We should have turned off the cursor to all screens.
     * This guarentees that it will get turned on
     */
    rememberedScreen = -1;

    /* call the screen saver here so that anyone could have 
     * wrapped it in their screen init
     */
    for ( lsn = 0; lsn < screenInfo.numScreens; lsn++ )
    	(screenInfo.screens[lsn]->SaveScreen)
		(screenInfo.screens[lsn],  SCREEN_SAVER_OFF);

    /*
     * Turn the cursor on in logical screen 0.
     */
    wsCursorControl(WS_SCREEN(screenInfo.screens[0]), CURSOR_ON);

    return;
}
