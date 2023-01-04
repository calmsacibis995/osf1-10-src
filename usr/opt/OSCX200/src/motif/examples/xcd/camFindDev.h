/******************************************************************************
*******************************************************************************
*
*   Copyright (c) 1993 by Digital Equipment Corporation
*
*   Permission to use, copy, modify, and distribute this software for any
*   purpose and without fee is hereby granted, provided that the above
*   copyright notice and this permission notice appear in all copies, and that
*   the name of Digital Equipment Corporation not be used in advertising or
*   publicity pertaining to distribution of the document or software without
*   specific, written prior permission.
*
*   Digital Equipment Corporation makes no representations about the
*   suitability of the software described herein for any purpose.  It is
*   provided "as is" without express or implied warranty.
*  
*  DEC is a registered trademark of Digital Equipment Corporation
*  DIGITAL is a registered trademark of Digital Equipment Corporation
*  X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/

/*
 * File:	camFindDev.c
 * Author:	Henry R. Tumblin
 * Date:	April 15,1993
 *
 * Description:
 *	Generic cam scan for devices routine.
 *
 */

typedef struct {
	u_char bus;
	u_char target;
	u_char lun;
	char * name;
	ALL_INQ_DATA * inq;
}  _camdevtbl, *camDevTbl;

camDevTbl camFindDev();
