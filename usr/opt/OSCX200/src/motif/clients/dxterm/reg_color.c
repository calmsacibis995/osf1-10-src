/*****  COLOR  *****/
/* #module color.c "X0.0" */
/*
 *  Title:	color.c
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1988, 1993                                                 |
 *  | By Digital Equipment Corporation, Maynard, Mass.                       |
 *  | All Rights Reserved.                                                   |
 *  |                                                                        |
 *  | This software is furnished under a license and may be used and  copied |
 *  | only  in  accordance  with  the  terms  of  such  license and with the |
 *  | inclusion of the above copyright notice.  This software or  any  other |
 *  | copies  thereof may not be provided or otherwise made available to any |
 *  | other person.  No title to and ownership of  the  software  is  hereby |
 *  | transfered.                                                            |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  Module Abstract:
 *
 *	<short description of module contents>
 *
 *  Procedures contained in this module:
 *
 *	<list of procedure names and abstracts>
 *
 *  Author:	<original author>
 *
 *  Modification history:
 *
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Bob Messenger		 7-Apr-1989	X2.0-6
 *	- removed rgbhls.  rgb to hls conversion is now done in reg_regis3.c.
 *
 *	R. Messenger	20-Oct-1987
 * Re-written for DECWindows, based on col332.c from Pegasus/Pro 350 code.
 * col332.c produced 3 bis red, 3 bits green, 2 bits blue.  color.c produces
 * 16 bits each (0 to 65535).
 **/

#include "regstruct.h"

#define MAX_RGB 65535

/* hlsrgb - convert HLS color to RGB */

hlsrgb( hue, lightness, saturation, red, green, blue, mono )
    int hue;		/* 0 <= hue < 360; blue at 0 */
    int lightness;	/* 0 <= lightness <= 100 */
    int saturation;	/* 0 <= saturation <= 100 */
    int *red;	/* output red value, from 0 to MAX_RGB */
    int *green;
    int *blue;
    int *mono;	/* output monochrome value, from 0 to MAX_RGB */
{
    int prime;	/* value of primary color */
    int comp;	/* value of complementary color */
    int ramp;	/* value partially between comp and prime */

/* check inputs against allowable ranges -- correct if needed */

    if ( saturation <= 0 )
	{  /* no color - gray scale only */
	*red = lightness * ( MAX_RGB + 1 ) / 100;
	if ( *red < 0 )
	    *red = 0;
	if ( *red > MAX_RGB )
	    *red = MAX_RGB;
	*green = *red;
	*blue = *red;
	*mono = *red;
	return;
	}

    if ( saturation > 100 )
	saturation = 100;

    if ( lightness <= 0 )
	{  /* black */
	*red = 0;
	*green = 0;
	*blue = 0;
	*mono = 0;
	return;
	}

    if ( lightness > 100 )
	lightness = 100;

    if ( hue <= -360 )
	hue %= 360;	/* force it to -359 to 0 */

    if ( hue < 0 )
	hue += 360;	/* put it in range 0 to 359 */
    else if ( hue >= 360 )
	hue %= 360;

/* compute values for prime and comp -- the value for the desired
   color and the value for the complement of the desired color */

    if ( lightness <= 50 )
	prime = lightness * ( 100 + saturation ) / 100;
    else
	prime = lightness + saturation - ( lightness * saturation / 100 );

    comp = 2 * lightness - prime;

    ramp = ( prime - comp ) * ( hue % 60 ) / 60;

/* compute red, green and blue vales from prime, comp and ramp, based on
   which 60 degree arc of the circle we are in */

    switch ( hue / 60 )
	{
	case 0:		/* 0 to 60 = blue to magenta */
	    *red = comp + ramp;
	    *green = comp;
	    *blue = prime;
	    break;
	case 1:		/* 60 to 120 = magenta to red */
	    *red = prime;
	    *green = comp;
	    *blue = prime - ramp;
	    break;
	case 2:		/* 120 to 180 = red to yellow */
	    *red = prime;
	    *green = comp + ramp;
	    *blue = comp;
	    break;
	case 3:		/* 180 to 240 = yellow to green */
	    *red = prime - ramp;
	    *green = prime;
	    *blue = comp;
	    break;
	case 4:		/* 240 to 300 = green to cyan */
	    *red = comp;
	    *green = prime;
	    *blue = comp + ramp;
	    break;
	case 5:		/* 300 to 360 = cyan to blue */
	    *red = comp;
	    *green = prime - ramp;
	    *blue = prime;
	    break;
	}

    *red = *red * ( MAX_RGB + 1 ) / 100;
    if ( *red > MAX_RGB )
	*red = MAX_RGB;
    *green = *green * ( MAX_RGB + 1 ) / 100;
    if ( *green > MAX_RGB )
	*green = MAX_RGB;
    *blue = *blue * ( MAX_RGB + 1 ) / 100;
    if ( *blue > MAX_RGB )
	*blue = MAX_RGB;
    *mono = ( *green * 59 + *red * 30 + *blue * 11 ) / 100;
}
