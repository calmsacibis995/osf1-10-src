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
/* DEC/CMS REPLACEMENT HISTORY, Element GEHT.H*/
/* *3    19-MAR-1992 13:44:46 GOSSELIN "added new RAGS support"*/
/* *2     3-MAR-1992 17:23:37 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:49:24 PARMENTER "Rags"*/
/* DEC/CMS REPLACEMENT HISTORY, Element GEHT.H*/
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**	GEHT.H         	  	RAGS Half-tone stipples
**
**  ABSTRACT:
**
** Note: "On"  (1) = WHITE
**       "Off" (0) = BLACK
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 11/20/87 Created
**
**--
**/

/* static char GEHTSid[] = "@(#)geHT.h	1.2 1/31/90";                                        */

#define stip_w	16
#define stip_h	16
/*
 * Half-tones
 */ 
static short	ht000[] = {					/*   0% black */
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
		ht005[] = {					/*  05% black */
0x0000,0x4444,0x0000,0x0000,0x0000,0x1111,0x0000,0x0000,
0x0000,0x4444,0x0000,0x0000,0x0000,0x1111,0x0000,0x0000},
		ht010[] = {					/*  10% black */
0x0000,0x4444,0x0000,0x1111,0x0000,0x4444,0x0000,0x1111,
0x0000,0x4444,0x0000,0x1111,0x0000,0x4444,0x0000,0x1111},
		ht015[] = {					/*  15% black */
0x0000,0x4545,0x0000,0x1111,0x0000,0x5454,0x0000,0x1111,
0x0000,0x4545,0x0000,0x1111,0x0000,0x5454,0x0000,0x1111},
		ht020[] = {					/*  20% black */
0x0000,0x5555,0x0000,0x1111,0x0000,0x5555,0x0000,0x5151,
0x0000,0x5555,0x0000,0x1111,0x0000,0x5555,0x0000,0x5151},
		ht025[] = {					/*  25% black */
0x0000,0x5555,0x0000,0x5555,0x0000,0x5555,0x0000,0x5555,
0x0000,0x5555,0x0000,0x5555,0x0000,0x5555,0x0000,0x5555},
		ht030[] = {					/*  30% black */
0x8080,0x5555,0x8080,0x5555,0x0808,0x5555,0x0000,0x5555,
0x8080,0x5555,0x8080,0x5555,0x0808,0x5555,0x0000,0x5555},
		ht035[] = {					/*  35% black */
0x8080,0x5555,0x8888,0x5555,0x0808,0x5555,0x2222,0x5555,
0x8080,0x5555,0x8888,0x5555,0x0808,0x5555,0x2222,0x5555},
		ht040[] = {					/*  40% black */
0x8888,0x5555,0xA8A8,0x5555,0x8888,0x5555,0x2222,0x5555,
0x8888,0x5555,0xA8A8,0x5555,0x8888,0x5555,0x2222,0x5555},
		ht045[] = {					/*  45% black */
0x8A8A,0x5555,0xA8A8,0x5555,0x8A8A,0x5555,0x2A2A,0x5555,
0x8A8A,0x5555,0xA8A8,0x5555,0x8A8A,0x5555,0x2A2A,0x5555},
		ht050[] = {					/*  50% black */
0x5555,0xAAAA,0x5555,0xAAAA,0x5555,0xAAAA,0x5555,0xAAAA,
0x5555,0xAAAA,0x5555,0xAAAA,0x5555,0xAAAA,0x5555,0xAAAA},
		ht055[] = {					/*  55% black */
0x7575,0xAAAA,0x5757,0xAAAA,0x7575,0xAAAA,0xD5D5,0xAAAA,
0x7575,0xAAAA,0x5757,0xAAAA,0x7575,0xAAAA,0xD5D5,0xAAAA},
		ht060[] = {					/*  60% black */
0x7777,0xAAAA,0x5757,0xAAAA,0x7777,0xAAAA,0xDDDD,0xAAAA,
0x7777,0xAAAA,0x5757,0xAAAA,0x7777,0xAAAA,0xDDDD,0xAAAA},
		ht065[] = {					/*  65% black */
0x7F7F,0xAAAA,0x7777,0xAAAA,0xF7F7,0xAAAA,0xDDDD,0xAAAA,
0x7F7F,0xAAAA,0x7777,0xAAAA,0xF7F7,0xAAAA,0xDDDD,0xAAAA},
		ht070[] = {					/*  70% black */
0x7F7F,0xAAAA,0x7F7F,0xAAAA,0xF7F7,0xAAAA,0xFFFF,0xAAAA,
0x7F7F,0xAAAA,0x7F7F,0xAAAA,0xF7F7,0xAAAA,0xFFFF,0xAAAA},
		ht075[] = {					/*  75% black */
0xFFFF,0xAAAA,0xFFFF,0xAAAA,0xFFFF,0xAAAA,0xFFFF,0xAAAA,
0xFFFF,0xAAAA,0xFFFF,0xAAAA,0xFFFF,0xAAAA,0xFFFF,0xAAAA},
		ht080[] = {					/*  80% black */
0xFFFF,0xAAAA,0xFFFF,0xEEEE,0xFFFF,0xAAAA,0xFFFF,0xAEAE,
0xFFFF,0xAAAA,0xFFFF,0xEEEE,0xFFFF,0xAAAA,0xFFFF,0xAEAE},
		ht085[] = {					/*  85% black */
0xFFFF,0xBABA,0xFFFF,0xEEEE,0xFFFF,0xABAB,0xFFFF,0xEEEE,
0xFFFF,0xBABA,0xFFFF,0xEEEE,0xFFFF,0xABAB,0xFFFF,0xEEEE},
		ht090[] = {					/*  90% black */
0xFFFF,0xBFBF,0xFFFF,0xEEEE,0xFFFF,0xFBFB,0xFFFF,0xEEEE,
0xFFFF,0xBFBF,0xFFFF,0xEEEE,0xFFFF,0xFBFB,0xFFFF,0xEEEE},
		ht095[] = {					/*  95% BLACK */
0XFFFF,0XFFFF,0xFFFF,0xEFEF,0xFFFF,0xFFFF,0xFFFF,0xEEEE,
0xFFFF,0xFFFF,0xFFFF,0xEEEF,0xFFFF,0xFFFF,0xFFFF,0xEEEE},
		ht100[] = {					/* 100% black */
0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF};
/*
 * Patterns
 */
static short    pat000[] = {			/* 1 pix vert stripes	      */
~0xAAAA,~0xAAAA,~0xAAAA,~0xAAAA,~0xAAAA,~0xAAAA,~0xAAAA,~0xAAAA,
~0xAAAA,~0xAAAA,~0xAAAA,~0xAAAA,~0xAAAA,~0xAAAA,~0xAAAA,~0xAAAA},
		pat001[] = {			/* 1 pix vert stripes - spaced*/
~0x7777,~0x7777,~0x7777,~0x7777,~0x7777,~0x7777,~0x7777,~0x7777,
~0x7777,~0x7777,~0x7777,~0x7777,~0x7777,~0x7777,~0x7777,~0x7777},
		pat002[] = {			/* 2 pix vert stripes	      */
~0x3333,~0x3333,~0x3333,~0x3333,~0x3333,~0x3333,~0x3333,~0x3333,
~0x3333,~0x3333,~0x3333,~0x3333,~0x3333,~0x3333,~0x3333,~0x3333},
		pat003[] = {			/* 3 pix vert stripes	      */
~0x8888,~0x8888,~0x8888,~0x8888,~0x8888,~0x8888,~0x8888,~0x8888,
~0x8888,~0x8888,~0x8888,~0x8888,~0x8888,~0x8888,~0x8888,~0x8888},
		pat004[] = {			/* 4 pix vert stripes	      */
~0xFEFE,~0xFEFE,~0xFEFE,~0xFEFE,~0xFEFE,~0xFEFE,~0xFEFE,~0xFEFE,
~0xFEFE,~0xFEFE,~0xFEFE,~0xFEFE,~0xFEFE,~0xFEFE,~0xFEFE,~0xFEFE},
		pat005[] = {			/* 1 pix vert stripes - spaced*/
~0xFCFC,~0xFCFC,~0xFCFC,~0xFCFC,~0xFCFC,~0xFCFC,~0xFCFC,~0xFCFC,
~0xFCFC,~0xFCFC,~0xFCFC,~0xFCFC,~0xFCFC,~0xFCFC,~0xFCFC,~0xFCFC},
		pat006[] = {			/* 2 pix vert stripes - spaced*/
~0xF0F0,~0xF0F0,~0xF0F0,~0xF0F0,~0xF0F0,~0xF0F0,~0xF0F0,~0xF0F0,
~0xF0F0,~0xF0F0,~0xF0F0,~0xF0F0,~0xF0F0,~0xF0F0,~0xF0F0,~0xF0F0},
		pat007[] = {			/* 5 pix vert stripes	      */
~0xC0C0,~0xC0C0,~0xC0C0,~0xC0C0,~0xC0C0,~0xC0C0,~0xC0C0,~0xC0C0,
~0xC0C0,~0xC0C0,~0xC0C0,~0xC0C0,~0xC0C0,~0xC0C0,~0xC0C0,~0xC0C0},
		pat008[] = {			/* 1 pix horiz stripes	      */
~0xffff,~0x0000,~0xffff,~0x0000,~0xffff,~0x0000,~0xffff,~0x0000,
~0xffff,~0x0000,~0xffff,~0x0000,~0xffff,~0x0000,~0xffff,~0x0000},
		pat009[] = {			/* 4 pix vert stripes	      */
~0xffff,~0xffff,~0xffff,~0x0000,~0xffff,~0xffff,~0xffff,~0x0000,
~0xffff,~0xffff,~0xffff,~0x0000,~0xffff,~0xffff,~0xffff,~0x0000},
		pat010[] = {			/* Narrow vertical stripes    */
~0xffff,~0xffff,~0x0000,~0x0000,~0xffff,~0xffff,~0x0000,~0x0000,
~0xffff,~0xffff,~0x0000,~0x0000,~0xffff,~0xffff,~0x0000,~0x0000},
		pat011[] = {			/* Narrow vertical stripes    */
~0xffff,~0x0000,~0x0000,~0x0000,~0xffff,~0x0000,~0x0000,~0x0000,
~0xffff,~0x0000,~0x0000,~0x0000,~0xffff,~0x0000,~0x0000,~0x0000},
		pat012[] = {			/* Narrow vertical stripes    */
~0xfdfd,~0xfbfb,~0xf7f7,~0xefef,~0xdfdf,~0xbfbf,~0x7f7f,~0xfefe,
~0xfdfd,~0xfbfb,~0xf7f7,~0xefef,~0xdfdf,~0xbfbf,~0x7f7f,~0xfefe},
		pat013[] = {			/* Narrow vertical stripes    */
~0x8181,~0x0303,~0x0606,~0x0c0c,~0x1818,~0x3030,~0x6060,~0xc0c0,
~0x8181,~0x0303,~0x0606,~0x0c0c,~0x1818,~0x3030,~0x6060,~0xc0c0},
		pat014[] = {			/* Narrow vertical stripes    */
~0xf7f7,~0xf7f7,~0xf7f7,~0x0000,~0x7f7f,~0x7f7f,~0x7f7f,~0x0000,
~0xf7f7,~0xf7f7,~0xf7f7,~0x0000,~0x7f7f,~0x7f7f,~0x7f7f,~0x0000},
		pat015[] = {			/* Narrow vertical stripes    */
~0xdfdf,~0xbfbf,~0x7f7f,~0xfefe,~0x7d7d,~0xbbbb,~0xd7d7,~0xefef,
~0xdfdf,~0xbfbf,~0x7f7f,~0xfefe,~0x7d7d,~0xbbbb,~0xd7d7,~0xefef},
		pat016[] = {			/* Narrow vertical stripes    */
~0xeeee,~0xbbbb,~0xdddd,~0x7777,~0xeeee,~0xbbbb,~0xdddd,~0x7777,
~0xeeee,~0xbbbb,~0xdddd,~0x7777,~0xeeee,~0xbbbb,~0xdddd,~0x7777},
		pat017[] = {			/* Narrow vertical stripes    */
~0x4444,~0x1111,~0x2222,~0x8888,~0x4444,~0x1111,~0x2222,~0x8888,
~0x4444,~0x1111,~0x2222,~0x8888,~0x4444,~0x1111,~0x2222,~0x8888},
		pat018[] = {			/* Narrow vertical stripes    */
~0x7474,~0x3838,~0x5c5c,~0xeeee,~0xc5c5,~0x8383,~0x4747,~0xeeee,
~0x7474,~0x3838,~0x5c5c,~0xeeee,~0xc5c5,~0x8383,~0x4747,~0xeeee},
		pat019[] = {			/* Narrow vertical stripes    */
~0x8383,~0x7d7d,~0xfefe,~0xfefe,~0x3838,~0xd7d7,~0xefef,~0xefef,
~0x8383,~0x7d7d,~0xfefe,~0xfefe,~0x3838,~0xd7d7,~0xefef,~0xefef},
		pat020[] = {			/* Narrow vertical stripes    */
~0xc7c7,~0xb7b7,~0x7777,~0x7b7b,~0x7c7c,~0x7b7b,~0x7777,~0xb7b7,
~0xc7c7,~0xb7b7,~0x7777,~0x7b7b,~0x7c7c,~0x7b7b,~0x7777,~0xb7b7},
		pat021[] = {			/* Narrow vertical stripes    */
~0x7878,~0x3c3c,~0x1e1e,~0x0f0f,~0x8787,~0xc3c3,~0xe1e1,~0xf0f0,
~0x7878,~0x3c3c,~0x1e1e,~0x0f0f,~0x8787,~0xc3c3,~0xe1e1,~0xf0f0},
		pat022[] = {			/* Narrow vertical stripes    */
~0xf7f7,~0xf7f7,~0xf7f7,~0xf7f7,~0xf7f7,~0xf7f7,~0xf7f7,~0x0000,
~0xf7f7,~0xf7f7,~0xf7f7,~0xf7f7,~0xf7f7,~0xf7f7,~0xf7f7,~0x0000};