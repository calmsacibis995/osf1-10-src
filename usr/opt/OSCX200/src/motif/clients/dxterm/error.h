/*
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © Digital Equipment Corporation, 1988 - 1990 All Rights      |
 *  | Reserved.  Unpublished rights reserved under the copyright laws of     |
 *  | the United States.                                                     |
 *  |                                                                        |
 *  | The software contained on this media is proprietary to and embodies    |
 *  | the confidential technology of Digital Equipment Corporation.          |
 *  | Possession, use, duplication or dissemination of the software and      |
 *  | media is authorized only pursuant to a valid written license from      |
 *  | Digital Equipment Corporation.                                         |
 *  |                                                                        |
 *  | RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the      |
 *  | U.S. Government is subject to restrictions as set forth in             |
 *  | Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19,    |
 *  | as applicable.                                                         |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 */
/*

	Update history

	Mark Woodbury 	25-May-1990 X3.0-3M
	- update for Motif
*/
/* @(#)error.h	X10/6.6	11/6/86 */
/* main.c */
#define	ERROR_KMALLOC	10	/* main: malloc() failed for keyboardtype */
#define	ERROR_FIONBIO	11	/* main: ioctl() failed on FIONBIO */
#define	ERROR_TSLOT	12	/* spawn: tslot() failed and getty */
#define	ERROR_TSLOT2	13	/* spawn: tslot() failed and am_slave */
#define	ERROR_OPDEVTTY	14	/* spawn: open() failed on /dev/tty */
#define	ERROR_TIOCGETP	15	/* spawn: ioctl() failed on TIOCGETP */
#define	ERROR_TIOCGETC	16	/* spawn: ioctl() failed on TIOCGETC */
#define	ERROR_TIOCGETD	17	/* spawn: ioctl() failed on TIOCGETD */
#define	ERROR_TIOCGLTC	18	/* spawn: ioctl() failed on TIOCGLTC */
#define	ERROR_TIOCLGET	19	/* spawn: ioctl() failed on TIOCLGET */
#define	ERROR_TIOCCONS	20	/* spawn: ioctl() failed on TIOCCONS */
#define	ERROR_OPDEVTTY2	21	/* spawn: second open() failed on /dev/tty */
#define	ERROR_NOTTY	22	/* spawn: ioctl() failed on TIOCNOTTY */
#define	ERROR_TIOCSETP	23	/* spawn: ioctl() failed on TIOCSETP */
#define	ERROR_TIOCSETC	24	/* spawn: ioctl() failed on TIOCSETC */
#define	ERROR_TIOCSETD	25	/* spawn: ioctl() failed on TIOCSETD */
#define	ERROR_TIOCSLTC	26	/* spawn: ioctl() failed on TIOCSLTC */
#define	ERROR_TIOCLSET	27	/* spawn: ioctl() failed on TIOCLSET */
#define	ERROR_TSLOT3	28	/* spawn: tslot() failed  */
#define	ERROR_FORK	29	/* spawn: fork() failed */
#define	ERROR_EXEC	30	/* spawn: exec() failed */
#define	ERROR_OPDEVTTY3	31	/* spawn: third open() failed on /dev/tty */
#define	ERROR_PTYS	32	/* get_pty: not enough ptys */
#define	ERROR_NOX	33	/* get_terminal: can't connect to server */
#define	ERROR_NOX2	34	/* get_terminal: can't connect and getty */
#define	ERROR_INIT	36	/* spawn: can't initialize window */
#define	ERROR_NOCO	37	/* resize: no `co' in termcap */
#define	ERROR_NOLI	38	/* resize: no `li' in termcap */
#define	ERROR_BORDER	39	/* get_terminal: can't make border tile */
#define	ERROR_BACK	40	/* get_terminal: can't make background tile */
#define ERROR_NOX3      43      /* get_terminal: bad pty from display server */
/* charproc.c */
#define	ERROR_SELECT	50	/* in_put: select() failed */
#define	ERROR_VINIT	54	/* VTInit: can't initialize window */
#define	ERROR_CNMALLOC1	55	/* Changename: malloc failed */
#define	ERROR_CNMALLOC2	56	/* Changename: malloc failed */
/* Tekproc.c */
#define	ERROR_TSELECT	60	/* Tinput: select() failed */
#define	ERROR_TINIT	64	/* TekInit: can't initialize window */
#define	ERROR_TBACK	65	/* TekBackground: can't make background */
#define	ERROR_TWINNAME	66	/* TekInit: malloc failed */
/* button.c */
#define	ERROR_BMALLOC2	71	/* SaltTextAway: malloc() failed */
#define ERROR_BADMENU   72      /* ModeMenu: don't know what menu to use */
/* misc.c */
#define	ERROR_LOGEXEC	80	/* StartLog: exec() failed */
#define	ERROR_XERROR	83	/* xerror: XError event */
#define	ERROR_XIOERROR	84	/* xioerror: X I/O error */
#define ERROR_WINNAME   85      /* get_terminal: malloc failed */
/* screen.c */
#define	ERROR_SCALLOC	90	/* Alloc: calloc() failed on base */
#define	ERROR_SCALLOC2	91	/* Alloc: calloc() failed on rows */
#define	ERROR_SREALLOC	92	/* ScreenResize: realloc() failed on alt base */
#define	ERROR_SREALLOC2	93	/* ScreenResize: realloc() failed on alt rows */
#define	ERROR_SREALLOC3	94	/* ScreenResize: realloc() failed on rows */
#define	ERROR_SREALLOC4	95	/* ScreenResize: realloc() failed on rows */
#define	ERROR_RESIZE	96	/* ScreenResize: malloc() or realloc() failed */
#define	ERROR_RESIZE2	97	/* ScreenResize: malloc() or realloc() failed */
#define	ERROR_RESIZROW	98	/* ScreenResize: realloc() failed on alt char */
#define	ERROR_RESIZROW2	99	/* ScreenResize: realloc() failed on alt attr */
#define	ERROR_RESIZROW3	100	/* ScreenResize: realloc() failed on attr */
#define	ERROR_RESIZROW4	101	/* ScreenResize: realloc() failed on attr */
/* scrollbar.c */
#define	ERROR_SBRALLOC	110	/* ScrollBarOn: realloc() failed on base */
#define	ERROR_SBRALLOC2	111	/* ScrollBarOn: realloc() failed on rows */
/* util.c */
#define	ERROR_UBACK	120	/* ReverseVideo: can't make background */
