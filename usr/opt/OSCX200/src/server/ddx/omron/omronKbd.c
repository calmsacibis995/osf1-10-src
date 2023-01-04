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
 * $XConsortium: omronKbd.c,v 1.1 91/06/29 13:49:01 xguest Exp $
 *
 * Copyright 1991 by OMRON Corporation
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of OMRON not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  OMRON makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * OMRON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL OMRON
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "omron.h"

#include "omronKbd.h"

#ifdef	uniosu
# include <sys/semienc.t>
#else /* not uniosu */
# ifdef luna88k
#  include <dev/semienc.t>
# else
#  ifdef luna2
#   include <dev/semienc.t>
#  else /* uniosb */
#   include <om68kdev/keymap.t>
#  endif
# endif
#endif

#if (!defined(uniosu)) && (!defined(luna88k)) && (!defined(luna2))
#define NOTREP 0
#define REPEAT 1
#endif

extern CARD8 *omronKeyModMap[];
extern KeySymsRec omronKeySyms[];
extern unsigned char *omronAutoRepeats[];

static Bool omronKbdInit();
static void omronBell();
static void omronKbdCtrl();
#ifndef USE_KANA_SWITCH
static void omronKbdModCheck();
#endif

int
omronKbdProc(pKeyboard,what)
DevicePtr     pKeyboard;
int           what;
{
	static Bool initFlag = FALSE;
	static omronKeyPrv prv[1];

	switch(what) {
		case DEVICE_INIT:
			pKeyboard->devicePrivate = (pointer)prv;
			if (initFlag == FALSE) {
#ifdef USE_KEYCOMPATI
				prv->offset = 0;
#else
				prv->offset = 7;
#endif
				if (!omronKbdInit(prv))
					return (!Success);
				initFlag = TRUE;
			}
			bcopy(prv->semiEncodeDef,prv->semiEncode,CODTBSZ);
#ifndef USE_KANA_SWITCH
			bcopy(prv->semiEncodeDef,prv->semiKanaEncode,CODTBSZ);
#endif
			bcopy (omronAutoRepeats[prv->type],
				defaultKeyboardControl.autoRepeats,AREPBUFSZ);
			prv->keybdCtrl = defaultKeyboardControl;
#ifndef USE_KANA_SWITCH
			prv->key_state = 0;
# ifdef luna2
			if (prv->type != KB_ASCII ) {
# endif
				prv->kana_offset =
					(omronKeySyms[prv->type].maxKeyCode +
					 omronKeySyms[prv->type].minKeyCode + 1) /2 - 
					 omronKeySyms[prv->type].minKeyCode ; 
# ifdef luna2
			}
# endif
#endif
			InitKeyboardDeviceStruct(pKeyboard,
				&(omronKeySyms[prv->type]), (omronKeyModMap[prv->type]),
				omronBell, omronKbdCtrl);
#ifndef UNUSE_DRV_TIME 
			omronSetDriverTimeMode(NULL, pKeyboard);
#endif
			break;
		case DEVICE_ON:
#ifndef UNUSE_SIGIO_SIGNAL
			prv->flags |= FASYNC;
			if (fcntl(prv->fd, F_SETFL, prv->flags) < 0 ) {
				Error("Can't enable the keyboard SIGIO.");
				return (!Success);
			}
#endif
			AddEnabledDevice(prv->fd);
			pKeyboard->on = TRUE;
			break;
		case DEVICE_OFF:
		case DEVICE_CLOSE:
#ifndef UNUSE_SIGIO_SIGNAL
			prv->flags &= ~FASYNC;
			if (fcntl(prv->fd, F_SETFL, prv->flags) < 0 ) {
				Error("Can't disable the keyboard SIGIO.");
			}
#endif
			if (ioctl(prv->fd, KBFLSH, NULL) < 0) {
				Error("Keyboard ioctl KBFLSH fault.");
			}
			RemoveEnabledDevice(prv->fd);
			pKeyboard->on = FALSE;
			break;
	}
	return (Success);
} 


static Bool
omronKbdInit(prv)
omronKeyPrvPtr prv;
{
#ifdef uniosu
	struct termio new_term;
#else
	struct sgttyb new_term;
#endif
	struct kbssgeta segeta;
#if   (! defined(uniosu)) && (! defined(luna88k)) && (! defined(luna2))
	register int i;
#endif
#ifdef UNUSE_SIGIO_SIGNAL
	int arg = 1; 
#endif


	if((prv->fd = open("/dev/kbd",O_RDWR,0)) < 0) {
		Error("Can't open /dev/kbd");
		return FALSE;
	}

#ifdef luna2
	if( ioctl(prv->fd,KIOCTYPE,&(prv->type)) == -1 ) {
		Error("Can't get kbd type.");
		return FALSE;
	}	
#else
	prv->type = 0;
#endif

#ifdef UNUSE_SIGIO_SIGNAL
	ioctl (prv->fd, FIONBIO, &arg);
#else
	if ((prv->flags = fcntl(prv->fd, F_GETFL, NULL)) < 0) {
		Error("Keyboard fcntl F_GETFL fault.");
		return FALSE;
	}
	prv->flags |= FNDELAY;
	if (fcntl(prv->fd, F_SETFL, prv->flags) < 0
		|| fcntl(prv->fd, F_SETOWN, getpid()) < 0) {
		Error("Can't set up kbd to receive SIGIO.");
		return FALSE;
	}
#endif

	if(ioctl(prv->fd,KBSETM,SEMIENCODED) < 0) {
		Error("Keyboard ioctl SEMIENCODED fault.");
		return FALSE;
	}

	if (ioctl(prv->fd,KBRSTENABLE,RSTDISABLE) < 0) {
		Error("Keyboard ioctl RSTDISABLE fault.");
		return FALSE;
	}
#ifdef uniosu
	if(ioctl(prv->fd,TCGETA,&(prv->old_term)) < 0) {
		Error("Keyboard ioctl TCGETA fault.");
		return FALSE;
	}
#else
	if(ioctl(prv->fd,TIOCGETP,&(prv->old_term)) < 0) {
		Error("Keyboard ioctl TCGETA fault.");
		return FALSE;
	}
#endif

	new_term = prv->old_term;

#ifdef uniosu
	new_term.c_lflag &= ~ICANON;
	new_term.c_lflag &= ~ISIG;
	new_term.c_cc[VEOF] = 1;
	if (ioctl(prv->fd,TCSETA,&new_term) < 0) {	
		Error("Keyboard ioctl TCSETA fault.");
		return FALSE;
	}
#else
	new_term.sg_flags = RAW;
	if (ioctl(prv->fd,TIOCSETP,&new_term) < 0) {	
		Error("Keyboard ioctl TIOCSETP fault.");
		return FALSE;
	}
#endif
	
	segeta.kbsreptp = prv->semiEncodeDef;
	if(ioctl(prv->fd, KBSGETTA, &segeta) < 0) {
		Error("Kbd ioctl KBSGETTA fault.");
		return FALSE;
	}

#if   (! defined(uniosu)) && (! defined(luna88k)) && (! defined(luna2))
	for(i = 0 ; i < CODTBSZ ; i++ ) {
		prv->semiEncodeDef[i] = prv->semiEncodeDef[i] ? 0 : 1;
	}
#endif

	return TRUE;
}
		
void
omronKbdGiveUp()
{
	DevicePtr     pKeyboard;
	omronKeyPrvPtr prv;

	pKeyboard = LookupKeyboardDevice();

	if(!pKeyboard) return;

	prv = (omronKeyPrvPtr)(pKeyboard->devicePrivate);
	if(!prv) return;

	if (ioctl(prv->fd,KBSETM,ENCODED) < 0) {
		Error("Kbd ioctl fault(set encode mode).");
	}

	if (ioctl(prv->fd,KBRSTENABLE,RSTENABLE) < 0) {
		Error("Kbd ioctl fault.");
	}

#ifdef uniosu	
	if (ioctl(prv->fd,TCSETA,&(prv->old_term)) < 0) {
		Error("Kbd ioctl fault(reset term mode).");
	}
#else
	if (ioctl(prv->fd,TIOCSETP,&(prv->old_term)) < 0) {
		Error("Kbd ioctl fault(reset term mode).");
	}
#endif

#ifndef UNUSE_DRV_TIME
	if(ioctl(prv->fd, KBTIME,0) < 0) {
		if ( errno != EINVAL ) {
			Error("Kbd ioctl KBTIME fault.");
		}
	}
#endif

	close(prv->fd);
}
	
static void
omronBell(loudness, pKeyboard)
int           loudness;
DevicePtr     pKeyboard;
{
	register int 	hz, msec;
	char c;
	omronKeyPrvPtr prv;

	prv = (omronKeyPrvPtr)(pKeyboard->devicePrivate);

	if (loudness == 0) {
		return;
	}
	hz = prv->keybdCtrl.bell_pitch;
#ifdef uniosu
	if (hz <= 100){
		c = 0x47;
	} else  if (hz <= 150) {
		c = 0x46;
	} else  if (hz <= 300) {
		c = 0x45;
	} else  if (hz <= 600) {
		c = 0x44;
	} else  if (hz <= 1000) {
		c = 0x43;
	} else  if (hz <= 1500) {
		c = 0x42;
	} else  if (hz <= 3000) {
		c = 0x41;
	} else {
		c = 0x40;
	}

	msec = omronKeybdCtrl.bell_duration;
	if (msec <= 40) {
		i = 0;
	} else if (msec <= 150) {
		i = 1;
	} else if (msec <= 400) {
		i = 2;
	} else {
		i = 3;
	}
	c |= (i << 3);
#else
	if (hz <= 100){
		c = K_HZ100;
	} else  if (hz <= 150) {
		c = K_HZ150;
	} else  if (hz <= 300) {
		c = K_HZ300;
	} else  if (hz <= 600) {
		c = K_HZ600;
	} else  if (hz <= 1000) {
		c = K_HZ1000;
	} else  if (hz <= 1500) {
		c = K_HZ1500;
	} else  if (hz <= 3000) {
		c = K_HZ3000;
	} else {
		c = K_HZ6000;
	}

	msec = prv->keybdCtrl.bell_duration;
	if (msec <= 40) {
		c |= K_TIME40;
	} else if (msec <= 150) {
		c |= K_TIME150;
	} else if (msec <= 400) {
		c |= K_TIME400;
	} else {
		c |= K_TIME700;
	}
#endif
#ifdef uniosu
	if (  write(prv->fd,&c,1) < 0 ) {
		Error ("OmronBell can't write keyboard.");
	}
#else
	if (  ioctl(prv->fd, KBBELL, c) < 0 ) {
		Error ("OmronBell ioctl KBBELL error.");
	}
#endif
}

static void
omronKbdCtrl(pKeyboard, ctrl)
DevicePtr     pKeyboard;
KeybdCtrl     *ctrl;
{
	struct kbrepeat rep;
	struct kbssgeta seseta;
	register KeyCode key;
	register int mask,i;
	int shift;
	omronKeyPrvPtr prv;

	prv = (omronKeyPrvPtr)(pKeyboard->devicePrivate);

	if ( pKeyboard->on == FALSE ) {
		if ( ctrl->click == 0 ) {
			if(ioctl(prv->fd, KBCLICK, 0) < 0) {
				Error("Kbd ioctl KBCLICK error.");
			}
		} else {
			if(ioctl(prv->fd, KBCLICK, 5) < 0) {
				Error("Kbd ioctl KBCLICK error.");
			}
		}

		if ( ctrl->autoRepeat == TRUE )
			rep.repstart = REPCNT;
		else
			rep.repstart = 0;
		
		rep.repinterval = RPINTVL;
	
		if(ioctl(prv->fd, KBREPTIM, &rep) < 0) {
			Error("Kbd ioctl KBREPTIM error.");
		} 

		prv->minkey = omronKeySyms[prv->type].minKeyCode;
#ifndef USE_KANA_SWITCH
# ifdef luna2
		if (prv->type == KB_ASCII )
			prv->maxkey = omronKeySyms[prv->type].maxKeyCode; 
		else
# endif
			prv->maxkey = omronKeySyms[prv->type].minKeyCode +
						  prv->kana_offset - 1; 
#else
		prv->maxkey = omronKeySyms[prv->type].maxKeyCode; 
#endif

		for( key = prv->minkey ; key <= prv->maxkey ; key++) {
			if ( prv->semiEncodeDef[key - prv->offset] == REPEAT ) {
				i = (key >> 3) ; 
				mask = (1 << (key & 7));
				if ( ctrl->autoRepeats[i] & mask ) {
					prv->semiEncode[key - prv->offset] = REPEAT;	
				} else {
					prv->semiEncode[key - prv->offset] = NOTREP;	
				}
			}
		}

#ifndef USE_KANA_SWITCH
# ifdef luna2
		if ( prv->type != KB_ASCII ) {
# endif
			prv->kana_minkey = omronKeySyms[prv->type].minKeyCode +
							   prv->kana_offset;
			prv->kana_maxkey = omronKeySyms[prv->type].maxKeyCode; 
	
			for( key = prv->kana_minkey ; key <= prv->kana_maxkey ; key++) {
				if ( prv->semiEncodeDef[key - prv->kana_offset - prv->offset] == REPEAT ) {
					i = (key >> 3) ; 
					mask = (1 << (key & 7));
					if ( ctrl->autoRepeats[i] & mask ) {
						prv->semiKanaEncode[key - prv->kana_offset - prv->offset] = REPEAT;	
					} else {
						prv->semiKanaEncode[key - prv->kana_offset - prv->offset] = NOTREP;	
					}
				}
			}
# ifdef luna2
		}
# endif
#endif
		seseta.kbsreptp = prv->semiEncode;
		if(ioctl(prv->fd, KBSSETTA, &seseta) < 0) {
			Error("Kbd ioctl KBSSETTA error.");
		} 
	} else {
		if ( ctrl->click != prv->keybdCtrl.click ) {
			if( ctrl->click == 0 ) {
				if(ioctl(prv->fd, KBCLICK, 0) < 0) {
					Error("Kbd ioctl KBCLICK error.");
				}
			} else {
				if(ioctl(prv->fd, KBCLICK, 5) < 0) {
					Error("Kbd ioctl KBCLICK error.");
				}
			}
		}
		if ( ctrl->autoRepeat != prv->keybdCtrl.autoRepeat ) { 
			if ( ctrl->autoRepeat == TRUE )	 
				rep.repstart = REPCNT;
			else
				rep.repstart = 0;
			rep.repinterval = RPINTVL;
			if(ioctl(prv->fd, KBREPTIM, &rep) < 0) {
				Error("Kbd ioctl KBREPTIM error.");
			}
		}
		for ( i = 0 ; i < AREPBUFSZ ; i++) {
			if (mask = ctrl->autoRepeats[i] ^ prv->keybdCtrl.autoRepeats[i] ) {
				shift = ffs(mask) -1;
				key = ( i << 3) | shift;
				if ( key >= prv->minkey && key <= prv->maxkey ) {
					key -= prv->offset;
					if ( prv->semiEncodeDef[key] == REPEAT ) {
						if ( ctrl->autoRepeats[i] & (1 << shift) )
						{
							prv->semiEncode[key] = REPEAT;
						}else{
							prv->semiEncode[key] = NOTREP;
						}
#ifndef USE_KANA_SWITCH
						if ( !(prv->key_state & KS_KANA) ){
#endif
							seseta.kbsreptp = prv->semiEncode;
							if(ioctl(prv->fd, KBSSETTA, &seseta) < 0) {
								Error("Kbd ioctl fault(semi-encode table set).");
							}
#ifndef USE_KANA_SWITCH
						}
#endif
					}
#ifdef USE_KANA_SWITCH
				}
#else
# ifdef luna2
				} else if(prv->type != KB_ASCII && key >= prv->kana_minkey &&
												  key <= prv->kana_maxkey ) {
# else
				} else if ( key >= prv->kana_minkey &&
						    key <= prv->kana_maxkey ) {
# endif
					key -= (prv->kana_offset + prv->offset);
					if ( prv->semiEncodeDef[key] == REPEAT ) {
						if ( ctrl->autoRepeats[i] & (1 << shift) )
							prv->semiKanaEncode[key] = REPEAT;
						else
							prv->semiKanaEncode[key] = NOTREP;
						if ( prv->key_state & KS_KANA ) {
							seseta.kbsreptp = prv->semiKanaEncode;
							if(ioctl(prv->fd, KBSSETTA, &seseta) < 0) {
								Error("Kbd ioctl fault(semi-encode table set).");
							}
						}
					}
				}
#endif
				break;
			}
		}
	}
	prv->keybdCtrl = *ctrl;
}
	
Bool
LegalModifier(key)
BYTE key;
{
	return(TRUE);
}

#define MAXEVENTS	1024

unsigned char *
omronKbdGetEvents(pKeyboard, pNumEvents, pAgain)
DevicePtr     pKeyboard;
int           *pNumEvents;
Bool          *pAgain;
{	
	int           nBytes;
	static unsigned char codebuf[MAXEVENTS];
	omronKeyPrvPtr prv;

	prv = (omronKeyPrvPtr)(pKeyboard->devicePrivate);

	nBytes = read(prv->fd,codebuf,sizeof(codebuf));

	if( nBytes < 0) {
		if (errno == EWOULDBLOCK) {
			*pNumEvents = 0;
			*pAgain = FALSE;
		} else {
			Error ("Can't read keyboard");
		}	
	} else {
		*pNumEvents = nBytes;
		*pAgain = (nBytes == sizeof (codebuf));
	}	
	return(codebuf);
}

#ifndef UNUSE_DRV_TIME
key_event *
omronKbdGetTEvents(pKeyboard, pNumEvents, pAgain)
DevicePtr     pKeyboard;
int           *pNumEvents;
Bool          *pAgain;
{	
	int           nBytes;
	static key_event codebuf[MAXEVENTS];
	omronKeyPrvPtr prv;

	prv = (omronKeyPrvPtr)(pKeyboard->devicePrivate);

	nBytes = read(prv->fd,codebuf,sizeof(codebuf));

	if( nBytes < 0) {
		if (errno == EWOULDBLOCK) {
			*pNumEvents = 0;
			*pAgain = FALSE;
		} else {
			Error ("Can't read keyboard");
		}	
	} else {
		*pNumEvents = nBytes / sizeof (key_event);
		*pAgain = (nBytes == sizeof (codebuf));
	}	
	return(codebuf);
}
#endif


void
omronKbdEnqueueEvent(pKeyboard, data)
DevicePtr     pKeyboard;
unsigned char   *data;
{

	unsigned char key;
	xEvent xE;
	CARD8 keyModifiers;
	KeySymsPtr pKeys;
	int lock_key = 0;
#ifndef USE_KANA_SWITCH
	struct kbssgeta seseta;
	unsigned long keysym; 
	static unsigned long modstate = 0;
	int	omron_key_state;
#endif
	omronKeyPrvPtr prv;

	prv = (omronKeyPrvPtr)(pKeyboard->devicePrivate);
#ifndef USE_KANA_SWITCH
	omron_key_state = prv->key_state;
#endif

	lastEventTime = GetTimeInMillis();
	xE.u.keyButtonPointer.time = lastEventTime;

	key = *data;

#ifndef USE_KANA_SWITCH
	if (!(key & 0x80)) {
		key &= 0x7f;
		if (key == KANA_KEY) {
			omron_key_state |= KS_KANA;
			prv->key_state = omron_key_state;
			seseta.kbsreptp = prv->semiKanaEncode;
			if(ioctl(prv->fd, KBSSETTA, &seseta) < 0) {
				Error("Kbd ioctl fault(semi-encode table set).");
			}
			return;
		}
		xE.u.u.type = KeyPress;
	} else {
		key &= 0x7f;
		if (key == KANA_KEY) {
			omron_key_state &= ~KS_KANA;
			prv->key_state = omron_key_state;
			seseta.kbsreptp = prv->semiEncode;
			if(ioctl(prv->fd, KBSSETTA, &seseta) < 0) {
				Error("Kbd ioctl fault(semi-encode table set).");
			}
			return;
		}
		xE.u.u.type = KeyRelease;
	}
	if (key == CAPSLOCK_KEY) {
		lock_key = 1;
	}
	key += prv->offset;
	keyModifiers = ((DeviceIntPtr)pKeyboard)->key->modifierMap[key];
	if ((omron_key_state & KS_KANA) && (!modstate) && (keyModifiers == 0)) {
		key += prv->kana_offset;
	}
	if (keyModifiers != 0) {
		pKeys = &((DeviceIntPtr)pKeyboard)->key->curKeySyms;
		if (!(lock_key ) && ((keyModifiers & LockMask) ||
			((keyModifiers & (Mod1Mask| Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask))
			&& (pKeys->map[(key - pKeys->minKeyCode) * pKeys->mapWidth] == XK_Mode_switch)))) {
			if (xE.u.u.type == KeyRelease)
				return;
			if (BitIsOn(((DeviceIntPtr)pKeyboard)->key->down,key))
				xE.u.u.type = KeyRelease;
		} else if ((keyModifiers & ControlMask) ||
			(keyModifiers & (Mod1Mask| Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask))) {
			keysym = pKeys->map[(key - pKeys->minKeyCode) * pKeys->mapWidth];
			omronKbdModCheck(keysym, xE.u.u.type, &modstate);
		}
	}
#else
	if (!(key & 0x80)) {
		xE.u.u.type = KeyPress;
	} else {
		xE.u.u.type = KeyRelease;
	}
	key &= 0x7f;
	if ((key == CAPSLOCK_KEY) || (key == KANA_KEY)) {
		lock_key = 1;
	}
	key += prv->offset;
	if (!lock_key) {
		keyModifiers = ((DeviceIntPtr)pKeyboard)->key->modifierMap[key];
		pKeys = &((DeviceIntPtr)pKeyboard)->key->curKeySyms;
		if ((keyModifiers & LockMask) ||
			((keyModifiers & (Mod1Mask| Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask))
			&& (pKeys->map[(key - pKeys->minKeyCode) * pKeys->mapWidth] == XK_Mode_switch))) {
			if (xE.u.u.type == KeyRelease)
				return;
			if (BitIsOn(((DeviceIntPtr)pKeyboard)->key->down,key))
				xE.u.u.type = KeyRelease;
		}
	}
#endif
	xE.u.u.detail = key;
#ifndef USE_KANA_SWITCH
	prv->key_state = omron_key_state;
#endif
	mieqEnqueue(&xE);
}

#ifndef UNUSE_DRV_TIME
void
omronKbdEnqueueTEvent(pKeyboard, data)
DevicePtr     pKeyboard;
key_event     *data;
{

	unsigned char key;
	xEvent xE;
	CARD8 keyModifiers;
	KeySymsPtr pKeys;
	int lock_key = 0;
#ifndef USE_KANA_SWITCH
	struct kbssgeta seseta;
	unsigned long keysym; 
	static unsigned long modstate = 0;
	int	omron_key_state;
#endif
	omronKeyPrvPtr prv;

	prv = (omronKeyPrvPtr)(pKeyboard->devicePrivate);
#ifndef USE_KANA_SWITCH
	omron_key_state = prv->key_state;
#endif

	xE.u.keyButtonPointer.time = data->time;
	key = data->code;

#ifndef USE_KANA_SWITCH
	if (!(key & 0x80)) {
		key &= 0x7f;
		if (key == KANA_KEY) {
			omron_key_state |= KS_KANA;
			prv->key_state = omron_key_state;
			seseta.kbsreptp = prv->semiKanaEncode;
			if(ioctl(prv->fd, KBSSETTA, &seseta) < 0) {
				Error("Kbd ioctl fault(semi-encode table set).");
			}
			return;
		}
		xE.u.u.type = KeyPress;
	} else {
		key &= 0x7f;
		if (key == KANA_KEY) {
			omron_key_state &= ~KS_KANA;
			prv->key_state = omron_key_state;
			seseta.kbsreptp = prv->semiEncode;
			if(ioctl(prv->fd, KBSSETTA, &seseta) < 0) {
				Error("Kbd ioctl fault(semi-encode table set).");
			}
			return;
		}
		xE.u.u.type = KeyRelease;
	}
	if (key == CAPSLOCK_KEY) {
		lock_key = 1;
	}
	key += prv->offset;
	keyModifiers = ((DeviceIntPtr)pKeyboard)->key->modifierMap[key];
	if ((omron_key_state & KS_KANA) && (!modstate) && (keyModifiers == 0)) {
		key += prv->kana_offset;
	}
	if (keyModifiers != 0) {
		pKeys = &((DeviceIntPtr)pKeyboard)->key->curKeySyms;
		if (!(lock_key ) && ((keyModifiers & LockMask) ||
			((keyModifiers & (Mod1Mask| Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask))
			&& (pKeys->map[(key - pKeys->minKeyCode) * pKeys->mapWidth] == XK_Mode_switch)))) {
			if (xE.u.u.type == KeyRelease)
				return;
			if (BitIsOn(((DeviceIntPtr)pKeyboard)->key->down,key))
				xE.u.u.type = KeyRelease;
		} else if ((keyModifiers & ControlMask) ||
			(keyModifiers & (Mod1Mask| Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask))) {
			keysym = pKeys->map[(key - pKeys->minKeyCode) * pKeys->mapWidth];
			omronKbdModCheck(keysym, xE.u.u.type, &modstate);
		}
	}
#else
	if (!(key & 0x80)) {
		xE.u.u.type = KeyPress;
	} else {
		xE.u.u.type = KeyRelease;
	}
	key &= 0x7f;
	if ((key == CAPSLOCK_KEY) || (key == KANA_KEY)) {
		lock_key = 1;
	}
	key += prv->offset;
	if (!lock_key) {
		keyModifiers = ((DeviceIntPtr)pKeyboard)->key->modifierMap[key];
		pKeys = &((DeviceIntPtr)pKeyboard)->key->curKeySyms;
		if ((keyModifiers & LockMask) ||
			((keyModifiers & (Mod1Mask| Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask))
			&& (pKeys->map[(key - pKeys->minKeyCode) * pKeys->mapWidth] == XK_Mode_switch))) {
			if (xE.u.u.type == KeyRelease)
				return;
			if (BitIsOn(((DeviceIntPtr)pKeyboard)->key->down,key))
				xE.u.u.type = KeyRelease;
		}
	}
#endif
	xE.u.u.detail = key;
#ifndef USE_KANA_SWITCH
	prv->key_state = omron_key_state;
#endif
	mieqEnqueue(&xE);
}
#endif

#ifndef USE_KANA_SWITCH
static void
omronKbdModCheck(keysym, type, modstate)
unsigned long keysym;
BYTE type;
unsigned long *modstate;
{
		unsigned long modst;

		modst = *modstate;
		switch(keysym) {
		case XK_Control_L:
			if (type == KeyPress) {
				modst |= KS_CTRL_L;
			} else {
				modst &= ~KS_CTRL_L;
			}
			break;
		case XK_Control_R:
			if (type == KeyPress) {
				modst |= KS_CTRL_R;
			} else {
				modst &= ~KS_CTRL_R;
			}
			break;
		case XK_Meta_L:
			if (type == KeyPress) {
				modst |= KS_META_L;
			} else {
				modst &= ~KS_META_L;
			}
			break;
		case XK_Meta_R:
			if (type == KeyPress) {
				modst |= KS_META_R;
			} else {
				modst &= ~KS_META_R;
			}
			break;
		case XK_Alt_L:
			if (type == KeyPress) {
				modst |= KS_ALT_L;
			} else {
				modst &= ~KS_ALT_L;
			}
			break;
		case XK_Alt_R:
			if (type == KeyPress) {
				modst |= KS_ALT_R;
			} else {
				modst &= ~KS_ALT_R;
			}
			break;
		case XK_Super_L:
			if (type == KeyPress) {
				modst |= KS_SUPER_L;
			} else {
				modst &= ~KS_SUPER_L;
			}
			break;
		case XK_Super_R:
			if (type == KeyPress) {
				modst |= KS_SUPER_R;
			} else {
				modst &= ~KS_SUPER_R;
			}
			break;
		case XK_Hyper_L:
			if (type == KeyPress) {
				modst |= KS_HYPER_L;
			} else {
				modst &= ~KS_HYPER_L;
			}
			break;
		case XK_Hyper_R:
			if (type == KeyPress) {
				modst |= KS_HYPER_R;
			} else {
				modst &= ~KS_HYPER_R;
			}
			break;
		default:
			break;
		}
		*modstate = modst;
}
#endif
