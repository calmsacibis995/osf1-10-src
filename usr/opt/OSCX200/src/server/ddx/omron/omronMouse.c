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
 * $XConsortium: omronMouse.c,v 1.1 91/06/29 13:49:03 xguest Exp $
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
#include "omronMouse.h"

static Bool omronMouseInit();
static void omronCrossScreen();
static void omronWarpCursor();
static void omronMouseControl(); 
static Bool omronCursorOffScreen();

miPointerScreenFuncRec omronPointerScreenFuncs = {
	omronCursorOffScreen,
	omronCrossScreen,
	omronWarpCursor,
};

int
omronMouseProc(pMouse,what)
DevicePtr 	pMouse;
int		what;
{
	static	Bool initFlag = FALSE;
	static	omronMousePrv	prv[1];
	BYTE    map[5];

	switch(what) {
	case DEVICE_INIT:
		pMouse->devicePrivate = (pointer)prv;
		if(initFlag == FALSE) {
			if (!omronMouseInit(prv))
				return (!Success);
#ifdef uniosu
			if (!omronResetTty(prv))
				return (!Success);
#endif
			initFlag = TRUE;
		}
		prv->button_state = 7;
		map[1] = Button1;
		map[2] = Button2;
		map[3] = Button3;
		InitPointerDeviceStruct( pMouse,map, 3, miPointerGetMotionEvents,
				omronMouseControl, miPointerGetMotionBufferSize());
#ifndef UNUSE_DRV_TIME
		omronSetDriverTimeMode(pMouse, NULL);
#endif
		break;	
	case DEVICE_ON:
#ifndef UNUSE_SIGIO_SIGNAL
		prv->ctl_flags |= FASYNC;
		if (fcntl(prv->fd, F_SETFL, prv->ctl_flags) < 0) {
			Error("Can't enable the mouse SIGIO.");
			return (!Success);
		}
#endif
		AddEnabledDevice (prv->fd);
		pMouse->on = TRUE;
		break;
	case DEVICE_OFF:
	case DEVICE_CLOSE:
#ifndef UNUSE_SIGIO_SIGNAL
		prv->ctl_flags &= ~FASYNC;
		if (fcntl(prv->fd, F_SETFL, prv->ctl_flags) < 0) {
			Error("Can't disable the mouse SIGIO.");
		}
#endif
		if (ioctl(prv->fd, MSDFLUSH, NULL) < 0) {
			Error("Mouse ioctl MSDFLUSH fault.");
		}
		RemoveEnabledDevice(prv->fd);
		pMouse->on = FALSE;
		break;
	}
	return (Success);
}

#ifdef uniosu
static Bool
omronResetTty(prv)
omronMousePrvPtr prv;
{
	int dummy = 0;

	if ((prv->ttyfd = open("/dev/tty",O_RDWR,0) ) == -1 ) {
		if ( errno != ENXIO ) {
			Error("Can't open tty.");
			return FALSE;
		}
	}
		
	if ( ioctl(prv->ttyfd,TIOCNOTTY,dummy) < 0 ) {
		Error("Can't ioctl TIOCNOTTY.");
		return FALSE;
	} 
	return TRUE;
}
#endif

static Bool
omronMouseInit(prv)
omronMousePrvPtr prv;
{
#ifdef	uniosu
	struct mssetlimt limit;
#endif	/* uniosu */
#ifdef UNUSE_SIGIO_SIGNAL
	int arg = 1;
#endif

	if ((prv->fd = open("/dev/mouse",O_RDONLY)) == -1) {
		Error("Can't open mouse device.");
		return FALSE;
	}

#ifdef UNUSE_SIGIO_SIGNAL
	ioctl(prv->fd, FIONBIO, &arg);
#else
	if ((prv->ctl_flags = fcntl(prv->fd, F_GETFL, NULL)) < 0) {
		Error("Mouse fcntl F_GETFL fault.");
		return FALSE;
	}
	prv->ctl_flags |= FNDELAY;
	if (fcntl(prv->fd, F_SETFL, prv->ctl_flags) < 0
		|| fcntl(prv->fd, F_SETOWN, getpid()) < 0) {
		Error("Can't set up mouse to receive SIGIO.");
		return FALSE;
	}
#endif

	if (ioctl(prv->fd,MSSETCURS,8) < 0) {
		Error("Mouse ioctl MSSETCURS fault.");
		return FALSE;
	}

	if(ioctl(prv->fd, MSSETMODE,1) < 0) {
		Error("Mouse ioctl MSSEMODE fault.");
		return FALSE;

	}
#ifdef	uniosu
	limit.mode = 1;
	if(ioctl(prv->fd, MSSETLIMT,&limit) < 0) {
		Error("Mouse ioctl MSSETLMT fault.");
		return FALSE;
	}
#endif	/* uniosu */
	return TRUE;
}

void
omronMouseGiveUp()
{
	DevicePtr     pMouse;
	omronMousePrvPtr prv;

	if(pMouse = LookupPointerDevice()) {
		prv = (omronMousePrvPtr)(pMouse->devicePrivate);
		if(prv) {
			(void)close(prv->fd);
#ifdef uniosu
			(void)close(prv->ttyfd);
#endif
		}
	}
}

#define MAXEVENTS    1024

static Bool
omronCursorOffScreen(pScreen, x, y)
    ScreenPtr	*pScreen;
    int		*x, *y;
{
	return FALSE;
}

static void
omronCrossScreen(pScreen,x,y)
ScreenPtr	pScreen;
int		x;
int		y;
{
}


static short
omronMouseAccelerate (pMouse, delta)
DevicePtr	  pMouse;
int	    	  delta;
{
    register PtrCtrl *p;
    register int  s;


    p = &((DeviceIntPtr)pMouse)->ptrfeed->ctrl;
  
    if(delta > 0) {
	s = 1;
    } else {
	s = -1;
	delta = -delta;
    }
	
    if (delta > p->threshold) {
	return ((short)(s * (p->threshold +
		    ((delta - p->threshold) * p->num) / p->den)));
    } else {
	return ((short)(s * delta));
    }
}


static void
omronMouseControl()
{
}

static void
omronWarpCursor (pScreen, x, y)
	ScreenPtr   pScreen;
	int         x, y;
{
	int oldmask;

	oldmask = sigblock (sigmask(SIGIO));
	miPointerWarpCursor (pScreen, x, y);
	sigsetmask (oldmask);
}




struct msdata *
omronMouseGetEvents(pMouse, pNumEvents, pAgain)
DevicePtr     pMouse;
int           *pNumEvents;
Bool          *pAgain;
{
	struct msdgeta	msdata;
	static struct msdata data[MAXEVENTS];
	omronMousePrvPtr prv =(omronMousePrvPtr) (pMouse->devicePrivate);

	msdata.mode = 1;
	msdata.count = MAXEVENTS;
	msdata.msdatap = data;

	if (ioctl(prv->fd,MSDGETA,&msdata) < 0) {
		return FALSE;
	}

	*pNumEvents = msdata.retval;
	
	*pAgain = (msdata.retval == MAXEVENTS);
	
	return(data);
}

#ifndef UNUSE_DRV_TIME
struct msdatat *
omronMouseGetTEvents(pMouse, pNumEvents, pAgain)
DevicePtr     pMouse;
int           *pNumEvents;
Bool          *pAgain;
{
	struct msdgetat	msdatat;
	static struct msdatat datat[MAXEVENTS];
	omronMousePrvPtr prv =(omronMousePrvPtr) (pMouse->devicePrivate);

	msdatat.mode = 1;
	msdatat.count = MAXEVENTS;
	msdatat.msdatatp = datat;
	if (ioctl(prv->fd,MSDGETAT,&msdatat) < 0) {
		return FALSE;
	}

	*pNumEvents = msdatat.retval;
	
	*pAgain = (msdatat.retval == MAXEVENTS);
	
	return(datat);
}
#endif


void
omronMouseEnqueueEvent(pMouse, data)
DevicePtr     pMouse;
struct msdata           *data;
{
	register 	int i;
	register	int	button_state;
	xEvent              xE;
	short omronMouseAccelerate();
	int delta_X,delta_Y;
	omronMousePrvPtr prv =(omronMousePrvPtr) (pMouse->devicePrivate);

	button_state = prv->button_state;
	lastEventTime = GetTimeInMillis();
	xE.u.keyButtonPointer.time = lastEventTime;
		
	switch(data->type) {
	case COORDEVNT:
		xE.u.u.type = MotionNotify;
		delta_X = omronMouseAccelerate(pMouse,
				data->event.coordinate.X_coord);
		delta_Y = omronMouseAccelerate(pMouse,
				data->event.coordinate.Y_coord);
		miPointerDeltaCursor (delta_X, delta_Y, lastEventTime);
		break;
	case BUTONEVNT:
		i = ((data->event.button.L_button == 0) << 2) |
		    ((data->event.button.M_button == 0) << 1) |
		    (data->event.button.R_button == 0);
		button_state = (((button_state & ~i) & 0x7) << 6) |
			    (((~button_state & i) & 0x7) << 3) |
			    (i & 0x7);

		if (BUTTON_PRESSED(button_state)) {
			xE.u.u.type = ButtonPress;
			if (LEFT_PRESSED(button_state)) {
				xE.u.u.detail = Button1; 
				mieqEnqueue(&xE);
			}
			if (RIGHT_PRESSED(button_state)) {
				xE.u.u.detail = Button3; 
				mieqEnqueue(&xE);
			}
			if (MIDDLE_PRESSED(button_state)) {
				xE.u.u.detail = Button2; 
				mieqEnqueue(&xE);
			}
		}
		if (BUTTON_RELEASED(button_state)) {
			xE.u.u.type = ButtonRelease;
			if (LEFT_RELEASED(button_state)) {
				xE.u.u.detail = Button1; 
				mieqEnqueue(&xE);
			}
			if (RIGHT_RELEASED(button_state)) {
				xE.u.u.detail = Button3; 
				mieqEnqueue(&xE);
			}
			if (MIDDLE_RELEASED(button_state)) {
				xE.u.u.detail = Button2; 
				mieqEnqueue(&xE);
			}
		}
		break;
	}
	prv->button_state = button_state;
}

#ifndef UNUSE_DRV_TIME
void
omronMouseEnqueueTEvent(pMouse, data)
DevicePtr     pMouse;
struct msdatat           *data;
{
	register 	int i;
	register	int	button_state;
	xEvent              xE;
	short omronMouseAccelerate();
	int delta_X,delta_Y;
	register unsigned long  time;
	omronMousePrvPtr prv =(omronMousePrvPtr) (pMouse->devicePrivate);

	button_state = prv->button_state;

	time = xE.u.keyButtonPointer.time = data->time;	

	switch(data->type) {
	case COORDEVNT:
		xE.u.u.type = MotionNotify;
		delta_X = omronMouseAccelerate(pMouse,
				data->event.coordinate.X_coord);
		delta_Y = omronMouseAccelerate(pMouse,
				data->event.coordinate.Y_coord);
		miPointerDeltaCursor (delta_X, delta_Y, time);
		break;
	case BUTONEVNT:
		i = ((data->event.button.L_button == 0) << 2) |
		    ((data->event.button.M_button == 0) << 1) |
		    (data->event.button.R_button == 0);
		button_state = (((button_state & ~i) & 0x7) << 6) |
			    (((~button_state & i) & 0x7) << 3) |
			    (i & 0x7);

		if (BUTTON_PRESSED(button_state)) {
			xE.u.u.type = ButtonPress;
			if (LEFT_PRESSED(button_state)) {
				xE.u.u.detail = 1; 
				mieqEnqueue(&xE);
			}
			if (RIGHT_PRESSED(button_state)) {
				xE.u.u.detail = 3; 
				mieqEnqueue(&xE);
			}
			if (MIDDLE_PRESSED(button_state)) {
				xE.u.u.detail = 2; 
				mieqEnqueue(&xE);
			}
		}
		if (BUTTON_RELEASED(button_state)) {
			xE.u.u.type = ButtonRelease;
			if (LEFT_RELEASED(button_state)) {
				xE.u.u.detail = 1; 
				mieqEnqueue(&xE);
			}
			if (RIGHT_RELEASED(button_state)) {
				xE.u.u.detail = 3; 
				mieqEnqueue(&xE);
			}
			if (MIDDLE_RELEASED(button_state)) {
				xE.u.u.detail = 2; 
				mieqEnqueue(&xE);
			}
		}
		break;
	}
	prv->button_state = button_state;
}
#endif


