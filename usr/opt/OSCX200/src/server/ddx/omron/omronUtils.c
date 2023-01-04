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
 * $XConsortium: omronUtils.c,v 1.1 91/06/29 13:49:07 xguest Exp $
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
#include "omronFb.h"

int scrw = 0;
int scrh = 0;
char *fb_type = NULL;


extern	int	atoi();
extern	int	exit();

static Bool omronQueryFb();
static void omronQueryVersion();
#ifdef luna2
static void omronQueryKbd();
#endif

int
ddxProcessArgument (argc, argv, i)
int argc;
char *argv[];
int i;
{
	extern void UseMsg();

	if (strcmp( argv[i], "-scrw") == 0) {
		if (++i >= argc) UseMsg ();
		scrw = atoi(argv[i]);
		return 2;
	}
	if (strcmp( argv[i], "-scrh") == 0) {
		if (++i >= argc) UseMsg ();
		scrh = atoi(argv[i]);
		return 2;
	}
	if (strcmp( argv[i], "-fbtype") == 0) {
		if (++i >= argc) UseMsg ();
		fb_type = argv[i];
		return 2;
	}
	if (strcmp (argv[i], "-queryfb") == 0) {
		omronQueryFb();
		exit(0);
	}
#ifdef luna2
	if (strcmp (argv[i], "-querykbd") == 0) {
		omronQueryKbd();
		exit(0);
	}
#endif
	if (strcmp (argv[i], "-version") == 0) {
		omronQueryVersion();
		exit(0);
	}
	/* Do nothing now */
	return 0;
}

static Bool
omronQueryFb()
{
		char *query_type;
		int query_depth;
		int machine_type;
		int plasma = FALSE;

#ifdef	uniosu
	if (sys9100(S91MACHTYPE,&machine_type) < 0) {
		Error("sys9100 error.");
	}
#else
	if (sysomron(S91MACHTYPE,&machine_type) < 0) {
		Error("sysomron error.");
	}
#endif

	switch(machine_type & MACH_GRAPHIC_BOARD) {
	case MACH_BM : /* we call LUNA */ 
		query_type ="BITMAP";
		break;

	case  MACH_PLASMA  : /* LUNA support plasma display */  
		query_type ="PLASMA";
		plasma = TRUE;
		break;

	default : 
		ErrorF("Unknown fb type. (0x%x)\n",machine_type);
		return (FALSE);
	}

	switch(machine_type & MACH_PLANE){
	case MACH_1_PLANE :
		query_depth = 1;
		break;

	case MACH_4_PLANE :
		query_depth = 4;
		break;

	case MACH_8_PLANE :
		query_depth = 8;
		break;

	default :
		ErrorF("Unknown plane number. (0x%x)\n", machine_type); 
		return (FALSE);
	}

		ErrorF(" GRAPHIC  %s\n",query_type);
		if ( plasma ) {
			ErrorF(" SCREEN_WIDTH  %d\n",PLASMA_SCREEN_WIDTH);
			ErrorF(" SCREEN_HEIGHT %d\n",PLASMA_SCREEN_HEIGHT);
		} else {
			ErrorF(" SCREEN_WIDTH  %d\n",SCREEN_WIDTH);
			ErrorF(" SCREEN_HEIGHT %d\n",SCREEN_HEIGHT);
		}
		ErrorF(" SCREEN_DEPTH  %d\n",query_depth);

	return (TRUE);
}

void
ddxUseMsg()
{
	ErrorF("-scrw #         	set screen width  \n");
	ErrorF("-scrh #         	set screen height \n");
	ErrorF("-fbtype {DT_BM,DT_BM8,DT_PLASMA,FS_BM} set fb_type \n");
	ErrorF("-queryfb       		query framebuffer type \n");
#ifdef luna2
	ErrorF("-querykbd       	query keyboard type \n");
#endif
	ErrorF("-version       		query server version \n");

}

static void
omronQueryVersion()
{
	ErrorF("X11R5 OMRON Sample Server.\n");
}


#ifdef luna2
static	char *omronKbdTypeName[] = {
	"LUNA-II JisJis Keyboard"	/* 0 */
	"LUNA-II Ascii Keyboard"	/* 1 */
	"LUNA-II AsciiJis Keyboard"	/* 2 */
	"LUNA-II Old Keyboard"		/* 3 */
	"LUNA-II Unknown Keyboard"	/* 4 */
};

#define	MAX_KEYBOARD	3
#define	ERR_KEYBOARD	4

static void
omronQueryKbd()
{
	int keyboardfd;
	int kbd_type;

	if((keyboardfd = open("/dev/kbd",O_RDWR,0)) < 0) {
		Error("Can't Open Keyboard");
	} else if( ioctl(keyboardfd,KIOCTYPE,&kbd_type) == -1 ) {
		Error("Can't Get Keyboard Type.\n");
	}

	if(kbd_type < 0 || kbd_type > MAX_KEYBOARD) {
		kbd_type = ERR_KEYBOARD;
	}

	ErrorF("%s\n", omronKbdTypeName[ kbd_type ]);

	close(keyboardfd);
}
#endif

