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
 * sanity test tool of the KM server extension
 */
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include "xkmeproto_include.h"
#include <stdio.h>

#define FALSE	0
#define TRUE	1

int KBModeSwitchSupport = TRUE;

static int
MyError(dpy, event)   
    Display *dpy;
    XErrorEvent *event;
{
    if(event->error_code == BadRequest) {
      printf("Server does not support keyboard mode switch\n");
      KBModeSwitchSupport = FALSE;
    }
    return KBModeSwitchInvalidCmd;
}


main()
{
    extern	XKMEDoKBModeSwitch();
    Display *dpy,*dpy2;
    int	command;
    int status = KBModeSwitchSuccess;
    int	(*old_handler)() = XSetErrorHandler(MyError);

    dpy = XOpenDisplay("");

    if(XKMEInitialize(dpy)) {
	printf("XKMEInitialize() succesful\n");
    } else {
	printf("XKMEInitialize() failes\n");
	printf("use xdpyinfo to check that extension is present\n");
	exit(1);
    }

    
    while(status == KBModeSwitchSuccess && KBModeSwitchSupport == TRUE ){
	printf("Command (1-2) : ");
        scanf("%d",&command);
	switch(command){
	  case 1:
	    status = XKMEDoKBModeSwitch(dpy,LockDownModeSwitch);
            break;

	  case 2: 
	    status = XKMEDoKBModeSwitch(dpy,UnlockModeSwitch);
	    break;

	  default:
	    break;
	}

       switch (status) {
         case(KBModeSwitchSuccess):
           printf("Server has successfully perform keyboard mode-switching\n");
	   break;
	   
	 case(KBModeSwitchFailure):
	   printf("Server has failed to perform keyboard mode-switching\n");
	   break;
	   
	 case(KBModeSwitchNoop):
	   /* It means the keyboard is already in the request state,
	      hence, no change is needed */
	   printf("No change for current keyboard mode-switch\n");
	   break;

	 default:
	   printf("Bad status - %d, program aborted");
	   break;
       }
    }
    XSetErrorHandler(old_handler);
    XCloseDisplay(dpy);
}

