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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: ffbgencirclebits.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:10:18 $";
#endif
/*
 */
/* Generate bitmaps for solid and 0-width circles up to SFBBUSBITS in diameter.
*/

#include <stdio.h>
#include "Xproto.h"
#include "ffb.h"
#include "ffbcirclebits.h"
#include "mifillarc.h"
#include "mizerarc.h"

/************ Code from mizerarc.c ********************/

static miZeroArcPtRec oob = {65536, 65536, 0};

Bool
miZeroArcSetup(arc, info, ok360)
    register xArc *arc;
    register miZeroArcRec *info;
    Bool ok360;
{
    int l;
    int angle1, angle2;
    int startseg, endseg;
    int startAngle, endAngle;
    int i, overlap;
    miZeroArcPtRec start, end;

    l = arc->width & 1;
    if (arc->width == arc->height)
    {
	info->alpha = 4;
	info->beta = 4;
	info->k1 = -8;
	info->k3 = -16;
	info->b = 12;
	info->a = (arc->width << 2) - 12;
	info->d = 17 - (arc->width << 1);
	if (l)
	{
	    info->b -= 4;
	    info->a += 4;
	    info->d -= 7;
	}
    }
    else if (!arc->width || !arc->height)
    {
	info->alpha = 0;
	info->beta = 0;
	info->k1 = 0;
	info->k3 = 0;
	info->a = -arc->height;
	info->b = 0;
	info->d = -1;
    }
    else
    {
	/* initial conditions */
	info->alpha = (arc->width * arc->width) << 2;
	info->beta = (arc->height * arc->height) << 2;
	info->k1 = info->beta << 1;
	info->k3 = info->k1 + (info->alpha << 1);
	info->b = l ? 0 : -info->beta;
	info->a = info->alpha * arc->height;
	info->d = info->b - (info->a >> 1) - (info->alpha >> 2);
	if (l)
	    info->d -= info->beta >> 2;
	info->a -= info->b;
	/* take first step, d < 0 always */
	info->b -= info->k1;
	info->a += info->k1;
	info->d += info->b;
	/* octant change, b < 0 always */
	info->k1 = -info->k1;
	info->k3 = -info->k3;
	info->b = -info->b;
	info->d = info->b - info->a - info->d;
	info->a = info->a - (info->b << 1);
    }
    info->dx = 1;
    info->dy = 0;
    info->w = (arc->width + 1) >> 1;
    info->h = arc->height >> 1;
    info->xorg = arc->x + (arc->width >> 1);
    info->yorg = arc->y;
    info->xorgo = info->xorg + l;
    info->yorgo = info->yorg + arc->height;
    if (!arc->width)
    {
	if (!arc->height)
	{
	    info->x = 0;
	    info->y = 0;
	    info->initialMask = 1;
	    info->start = oob;
	    info->end = oob;
	    return FALSE;
	}
	info->x = 0;
	info->y = 1;
    }
    else
    {
	info->x = 1;
	info->y = 0;
    }
    info->startAngle = 0;
    info->endAngle = 0;
    info->initialMask = 0xf;
    info->start = oob;
    info->end = oob;
    return TRUE;
}


#define Pixelate(xval, yval) \
    bits[(yval)] |= (1 << (xval))

#define DoPix(idx,xval,yval) if (mask & (1 << idx)) Pixelate(xval, yval);


MakeZerCircle(diameter)
    int diameter;
{
    CommandWord     bits[FFBBUSBITS];
    int		    i;
    xArc	    arcRec;
    xArc	    *arc = &arcRec;

    miZeroArcRec info;
    register int x, y, a, b, d, mask;
    register int k1, k3, dx, dy;
    Bool do360;

    for (i = diameter; i >= 0; i--) {
	bits[i] = 0;
    }
    if (diameter > 0) {
	arcRec.x = 0;
	arcRec.y = 0;
	arcRec.width = diameter;
	arcRec.height = diameter;
	arcRec.angle1 = 0;
	arcRec.angle2 = FULLCIRCLE;
    
	do360 = miZeroArcSetup(arc, &info, TRUE);
	MIARCSETUP();
	mask = info.initialMask;
	if (!(arc->width & 1))
	{
	    DoPix(1, info.xorgo, info.yorg);
	    DoPix(3, info.xorgo, info.yorgo);
	}
	if (!info.end.x || !info.end.y)
	{
	    mask = info.end.mask;
	    info.end = info.altend;
	}
	if (do360 && (arc->width == arc->height) && !(arc->width & 1))
	{
	    int yorgh = info.yorg + info.h;
	    int xorghp = info.xorg + info.h;
	    int xorghn = info.xorg - info.h;
    
	    while (1)
	    {
		Pixelate(info.xorg + x, info.yorg + y);
		Pixelate(info.xorg - x, info.yorg + y);
		Pixelate(info.xorg - x, info.yorgo - y);
		Pixelate(info.xorg + x, info.yorgo - y);
		if (a < 0)
		    break;
		Pixelate(xorghp - y, yorgh - x);
		Pixelate(xorghn + y, yorgh - x);
		Pixelate(xorghn + y, yorgh + x);
		Pixelate(xorghp - y, yorgh + x);
		MIARCCIRCLESTEP(;);
	    }
	    x = info.w;
	    y = info.h;
	}
	else if (do360)
	{
	    while (y < info.h || x < info.w)
	    {
		MIARCOCTANTSHIFT(;);
		Pixelate(info.xorg + x, info.yorg + y);
		Pixelate(info.xorgo - x, info.yorg + y);
		Pixelate(info.xorgo - x, info.yorgo - y);
		Pixelate(info.xorg + x, info.yorgo - y);
		MIARCSTEP(;,;);
	    }
	}
	else
	{
	    fprintf(stderr, "Oops, how did we get !do350?\n");
	    exit(1);
	}
	if ((x == info.start.x) || (y == info.start.y))
	    mask = info.start.mask;
	DoPix(0, info.xorg + x, info.yorg + y);
	DoPix(2, info.xorgo - x, info.yorgo - y);
	if (arc->height & 1)
	{
	    DoPix(1, info.xorgo - x, info.yorg + y);
	    DoPix(3, info.xorg + x, info.yorgo - y);
	}
    }

    printf("CommandWord zeroCircle%d[%d] = {", diameter, diameter/2+1);
    for (i = 0; i <= diameter/2; i++) {
	if ((i&3) == 0) printf("\n");
	printf("    0x%08lx", bits[i]);
	if (i == diameter/2) printf("\n"); else printf(",");
    }
    printf("};\n\n");
}

/******************** Code from mifillarc.c ****************************/

void
miFillArcSetup(arc, info)
    register xArc *arc;
    register miFillArcRec *info;
{
    info->y = arc->height >> 1;
    info->dy = arc->height & 1;
    info->yorg = arc->y + info->y;
    info->dx = arc->width & 1;
    info->xorg = arc->x + (arc->width >> 1) + info->dx;
    info->dx = 1 - info->dx;
    if (arc->width == arc->height)
    {
	/* (2x - 2xorg)^2 = d^2 - (2y - 2yorg)^2 */
	/* even: xorg = yorg = 0   odd:  xorg = .5, yorg = -.5 */
	info->ym = 8;
	info->xm = 8;
	info->yk = info->y << 3;
	if (!info->dx)
	{
	    info->e = -1;
	    info->xk = 0;
	}
	else
	{
	    info->y++;
	    info->yk += 4;
	    info->xk = -4;
	    info->e = - (info->y << 3);
	}
    }
    else
    {
	/* h^2 * (2x - 2xorg)^2 = w^2 * h^2 - w^2 * (2y - 2yorg)^2 */
	/* even: xorg = yorg = 0   odd:  xorg = .5, yorg = -.5 */
	info->ym = (arc->width * arc->width) << 3;
	info->xm = (arc->height * arc->height) << 3;
	info->yk = info->y * info->ym;
	if (!info->dy)
	    info->yk -= info->ym >> 1;
	if (!info->dx)
	{
	    info->xk = 0;
	    info->e = - (info->xm >> 3);
	}
	else
	{
	    info->y++;
	    info->yk += info->ym;
	    info->xk = -(info->xm >> 1);
	    info->e = info->xk - info->yk;
	}
    }
}

#define ADDSPANS()					    \
{							    \
    CommandWord mask;					    \
    mask = FFBLEFTBUSMASK(xorg - x, FFBBUSALL1)		    \
	    & FFBRIGHTBUSMASK(xorg - x + slw, FFBBUSALL1);  \
    bits[yorg - y] |= mask;				    \
    if (miFillArcLower(slw)) {				    \
	bits[yorg + y + dy] |= mask;			    \
    }							    \
}


MakeFillCircle(diameter)
    int diameter;
{
    CommandWord bits[FFBBUSBITS];
    int		i;
    xArc	arcRec;
    xArc	*arc = &arcRec;
    miFillArcRec info;
    register int x, y, e, ex;
    int yk, xk, ym, xm, dx, dy, xorg, yorg;
    int slw;

    for (i = diameter-1; i >= 0; i--) {
	bits[i] = 0;
    }

    if (diameter > 0) {
	arcRec.x = 0;
	arcRec.y = 0;
	arcRec.width = diameter;
	arcRec.height = diameter;
	arcRec.angle1 = 0;
	arcRec.angle2 = FULLCIRCLE;

	miFillArcSetup(arc, &info);
	MIFILLARCSETUP();
	if (arc->width == arc->height)
	{
	    while (y)
	    {
		MIFILLCIRCSTEP(slw);
		ADDSPANS();
	    }
	}
	else
	{
	    while (y > 0)
	    {
		MIFILLELLSTEP(slw);
		ADDSPANS();
	    }
	}
    } else {
	bits[0] = 0;
    }

    printf("CommandWord solidCircle%d", diameter);
    if (diameter == 0) diameter = 1;
    printf("[%d] = {", diameter);
    for (i = 0; i < diameter; i++) {
	if ((i&3) == 0) printf("\n");
	printf("    0x%08lx", bits[i]);
	if (i == diameter-1) printf("\n"); else printf(",");
    }
    printf("};\n\n");
}

main() {
    int diameter;

    printf(
      "/* DO NOT MODIFY.  AUTOMATICALLY GENERATED BY ffbgencirclebits.c */\n");
    printf("\n");
    printf("#include \"ffb.h\"\n");
    printf("#include \"ffbcirclebits.h\"\n");
    printf("\n");
    printf("\n");

    for (diameter = 0; diameter < FFBBUSBITS; diameter++) {
	MakeZerCircle(diameter);
    }

    printf("\n");
    printf("CommandWord *zeroCircles[FFBBUSBITS] = {");
    for (diameter = 0; diameter < FFBBUSBITS; diameter++) {
	if ((diameter&3) == 0) printf("\n");
	printf("    zeroCircle%d", diameter);
	if (diameter == FFBBUSBITS-1) printf("\n"); else printf(",");
    }
    printf("};\n\n");

    for (diameter = 0; diameter <= FFBBUSBITS; diameter++) {
	MakeFillCircle(diameter);
    }

    printf("\n");
    printf("CommandWord *solidCircles[FFBBUSBITS+1] = {");
    for (diameter = 0; diameter <= FFBBUSBITS; diameter++) {
	if ((diameter&3) == 0) printf("\n");
	printf("    solidCircle%d", diameter);
	if (diameter == FFBBUSBITS) printf("\n"); else printf(",");
    }
    printf("};\n\n");
}

/*
 * HISTORY
 */
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/server/ddx/dec/ffb/ffbgencirclebits.c,v 1.1.2.2 1993/11/19 21:10:18 Robert_Lembree Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
