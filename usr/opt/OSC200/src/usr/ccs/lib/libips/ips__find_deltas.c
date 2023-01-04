/***************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990-1991 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
***************************************************************************/
/* 
** this is just a test driver for ips__geometric and does NOT represent
** final code - used for demo purposes.
*/
/****************************************************************************
**
**              point 0                                 point 1
**                 +----------------------------------------+
**                /                                          \
**               /                                            \ 
**              /                                              \ 
**             +------------------------------------------------+
**          point 2                                           point 3
**
*****************************************************************************/

#include <math.h>

void _IpsFindDeltas (x, y, dst_x, dst_y, dx, dy, d_dx, d_dy, 
    dxr, dyr, d_dxr, d_dyr)
long		x[4], y[4];
unsigned long	dst_x,	dst_y;
float		*dx,	*dy;
float		*d_dx,	*d_dy;
float		*dxr,	*dyr;
float		*d_dxr,	*d_dyr;
    {
    float src_x, src_y;
    float scale_factor;
    double angle;
    float dx_l, dy_l;
    double  z;

/* calculate scale factor of top row */
 
    *dx = (float)(x[1] - x[0]);
    *dy = (float)(y[1] - y[0]);
    src_x = sqrt (*dx * *dx + *dy * *dy);
    scale_factor = src_x / (float)dst_x;

    printf ("scale of top row = %f\n",scale_factor);

    if (x[1] == x[0])
	{
	*dx = 0.0;
	*dy = scale_factor;
	}
    else
	{
	/* determine angle of inclination */
	z = *dy / *dx;
	z= fabs(z);
	angle = atan (z);
	printf ("angle of inclination = %f\n",angle);
	*dx = cos(angle) * scale_factor;
	*dy = sin(angle) * scale_factor;
	}

/* signs can cancel you out - quick fix */

    *dx = fabs(*dx);
    *dy = fabs(*dy);
    if (x[1] < x[0])
	*dx *= -1.0;
    if (y[1] < y[0])
	*dy *= -1.0;

    printf ("top row dx and dy = %f %f\n",*dx,*dy);
    printf ("\n");

/* calculate scale factor of bottom row */

    dx_l = (float)(x[3] - x[2]);
    dy_l = (float)(y[3] - y[2]);
    src_x = sqrt (dx_l * dx_l + dy_l * dy_l);
    scale_factor = src_x / (float)dst_x;
    printf ("scale of bottom row = %f\n",scale_factor);

    if (x[3] == x[2])
	{
	dx_l = 0.0;
	dy_l = scale_factor;
	}
    else
	{
	/* determine angle of inclination */
	z = dy_l / dx_l;
	z = fabs(z);
	angle = atan (z);
	printf ("angle of inclination = %f\n",angle);
	dx_l = cos(angle) * scale_factor;
	dy_l = sin(angle) * scale_factor;
	}

    dx_l = fabs(dx_l);
    dy_l = fabs(dy_l);
    if (x[3] < x[2])
	dx_l *= -1.0;
    if (y[3] < y[2])
	dy_l *= -1.0;

    printf ("bottom row dx and dy = %f %f\n",dx_l,dy_l);
    
/* differences of two rows over number of rows */
    *d_dxr = (dx_l - *dx) / (dst_y - 1);
    *d_dyr = (dy_l - *dy) / (dst_y - 1);

    printf ("diff of row deltas/rows = %f %f\n",*d_dxr, *d_dyr);

    printf ("\n");

/* calculate scale factor of left column for deltas over number of rows */
 
    *dxr = (float)(x[2] - x[0]);
    *dyr = (float)(y[2] - y[0]);
    src_y = sqrt (*dxr * *dxr + *dyr * *dyr);
    scale_factor = src_y / (float)dst_y;

    printf ("scale of left column = %f\n",scale_factor);

    if (x[2] == x[0])
	{
	*dxr = 0.0;
	*dyr = scale_factor;
	}
    else
	{
	/* determine angle of inclination */
	z = *dxr / *dyr;
	z= fabs(z);
	angle = atan (z);
	printf ("angle of inclination = %f\n",angle);
	*dxr = sin(angle) * scale_factor;
	*dyr = cos(angle) * scale_factor;
	}

    *dxr = fabs(*dxr);
    *dyr = fabs(*dyr);
    if (x[2] < x[1])
	*dxr *= -1.0;
    if (y[2] < y[1])
	*dyr *= -1.0;

    printf ("left column dxr and dyr = %f %f\n",*dxr,*dyr);
    printf ("\n");

    *d_dx = 0.0;
    *d_dy = 0.0;
    /*    printf ("diff of col deltas/rows = %f %f\n",*d_dx, *d_dy);*/
    }
