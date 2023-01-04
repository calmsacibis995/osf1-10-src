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
/* $XConsortium: brf_phead.c,v 5.3 91/04/04 13:32:58 gildea Exp $ */
/***********************************************************
Copyright(c) 1989,1990, 1991 by Sun Microsystems, Inc. and the X Consortium at M.I.T.

						All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Sun Microsystems,
the X Consortium, and MIT not be used in advertising or publicity
pertaining to distribution of the software without specific, written
prior permission.

SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/*--------------------------------------------------------------------*\
|  Copyright (C) 1989,1990, 1991, National Computer Graphics Association
|
|  Permission is granted to any individual or institution to use, copy, or
|  redistribute this software so long as it is not sold for profit, provided
|  this copyright notice is retained.
|
|                         Developed for the
|                National Computer Graphics Association
|                         2722 Merrilee Drive
|                         Fairfax, VA  22031
|                           (703) 698-9600
|
|                                by
|                 SimGraphics Engineering Corporation
|                    1137 Huntington Drive  Unit A
|                      South Pasadena, CA  91030
|                           (213) 255-0900
|---------------------------------------------------------------------
|
| Author        :	Norman D. Evangelista
|
| File          :	brf_prtheader.c
| Date          :	Mon Jul  3 18:51:18 PDT 1989
| Project       :	BIF Benchmark Reporting Format
| Description   :	File header generation routines
| Status        :	Version 1.0
|
| Revisions     :	
|
\*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*\
|	Table of Contents
|
|	char *BRF_appendToColon( *char, *char )
|		:	Appends a character string after a colon.
|	void BRF_loopTiming( int, float, float, int, *char, *FILE )
|		:	Prints test loop timing information to BRF output.
|	void BRF_printHeader( *FILE )
|		:	Prints the BRF file header to the specified file.
|	char *BRF_getDate( void )
|		:	Gets a day/date string for timestamping.
|	char *BRF_appendEndBar( *char )
|		:	Appends the end bar to the end of a string
|
\*--------------------------------------------------------------------*/

#include <X11/Xos.h>
#include <stdio.h>
#include "brf_prt.h"
#include "biftypes.h"
#include "globals.h"
#include "brfexption.h"

#ifndef X_NOT_STDC_ENV
#include <time.h>
#else
struct tm *localtime();
#endif

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

char *BRF_appendEndBar();



/*--------------------------------------------------------------------*\
| Procedure     :	char *BRF_appendToColon( *char, *char )
|---------------------------------------------------------------------
| Description   :	Appends a character string after a colon.
|---------------------------------------------------------------------
| Return        :	String 1 with string 2 appended.
\*--------------------------------------------------------------------*/
char *BRF_appendToColon( str1, str2 )
char	*str1;				/* Target string	 */
char	*str2;				/* String to append  */

{/* BRF_appendToColon */

	char		toColon[ 80 ];
	int n_tocolon, sub_length;
	static int total_line_len = 72;

	n_tocolon = strcspn (str1,":");

	sub_length = strlen( str1 );
	if (n_tocolon == sub_length) 
		n_tocolon = N_TOCOLON;
		else n_tocolon += 4;
	strncpy( toColon, str1, n_tocolon );
	toColon[n_tocolon] = '\0';

	strcat( toColon, "   " );
	strcat( toColon, str2 );
	sub_length = strlen( toColon );
	if (sub_length < total_line_len)
		strncat(toColon,
"                                                                        ",
				(total_line_len-sub_length-1 ));
    strcat(toColon,"|\n");
	strcpy( str1, toColon );
	return( str1 );

}/* BRF_appendToColon */

/*--------------------------------------------------------------------*\
| Procedure     :	void BRF_loopTiming( int, float, float, int,
|						 *char, *FILE )
|---------------------------------------------------------------------
| Description   :	Prints test loop timing information to BRF output.
|---------------------------------------------------------------------
| Revisions	:	Moved epsilon from brf_stop_stopwatch to here
|		 	M.F.R. 2/6/90
|
|---------------------------------------------------------------------
| Return :	
\*--------------------------------------------------------------------*/
BRF_loopTiming( nFrames, elapsedTime, xmitDelay,
loopNumber, testName, brfFile )
int	nFrames;			/* Number of frames this loop	  */
float	elapsedTime;			/* Duration of this loop	  */
float	xmitDelay;			/* System transmission delay	  */
int	loopNumber;			/* Number of this test loop	  */
char	*testName;			/* Name of this PLB test	  */
FILE	*brfFile;			/* BRF output file pointer	  */

{/* BRF_loopTiming */

	int			j = 0,k,i;
	float		framesPerSecond, timePerFrame;
	float		timingAccuracy;
	float		timingAccuracy2;
	char		strQuant[ 32 ], headLine[ 80 ];
	FILE		*brfHedFile;

	/*----------------------------------------------------------------*\
	|	Calculate timing stats
	\*----------------------------------------------------------------*/
	if (elapsedTime == 0.0) elapsedTime = 0.0001; 
	framesPerSecond = (float)nFrames / elapsedTime;
	timePerFrame    = elapsedTime / (float)nFrames;
#ifdef EXTERNALNOTE
	/* Old Chinese Proverb goes,"A man with one watch knows what time
	it is. Aman with two watches is never really sure!"
	Here we have two alternative methods of gauging the overall
	accuracy of the PLB run, sort of a benchmark on the benchmark.
	Method 1 is derived from the ratio of total elapsed time to transport
	delay time.
	Method two arbitrarily measures time and iterations against an
	empircal (arbitrary) scale of accuracy.
	*/
#endif

	timingAccuracy  = 100.0 * (elapsedTime / 
		(elapsedTime + (10 * xmitDelay)));

        timingAccuracy2 = ((100 * (elapsedTime / 10) 
	+ (float)nFrames / 1100.0)) * .5;

	timingAccuracy2 = MIN(100.0,timingAccuracy2);

	/*----------------------------------------------------------------*\
	|	Grab lines and pump through to BRF output file
	\*----------------------------------------------------------------*/
	i=0;
	k=0;
	while ( BRF_timeData[i][k] != (char *)NULL )
	{
		strcpy(headLine, BRF_timeData[i][k++]);

		switch( ++j )
		{/* Append info to proper lines */

			case 1	:
			case 3	:
				break;

			case 2	:
				fprintf( brfFile, "%s", headLine );
				sprintf( headLine, 
				"|   Test Loop of %d frames from File %s\n",
						   loopNumber, testName );
				BRF_appendEndBar(headLine);
				break;

			case 4	:
				sprintf( strQuant, "%d", nFrames );
				BRF_appendToColon( headLine, strQuant );
				break;

			case 5	:
				sprintf( strQuant, "%6.2f", elapsedTime );
				BRF_appendToColon( headLine, strQuant );
				break;

			case 6	:
				sprintf( strQuant, "%6.2f", xmitDelay );
				BRF_appendToColon( headLine, strQuant );
				break;

			case 7	:
				sprintf( strQuant, "%6.2f", framesPerSecond );
				BRF_appendToColon( headLine, strQuant );
				break;

			case 8	:
				sprintf( strQuant, "%6.2f", timePerFrame );
				BRF_appendToColon( headLine, strQuant );
				break;

			case 9	:
				sprintf( strQuant, "%6.2f%%", timingAccuracy );
				BRF_appendToColon( headLine, strQuant );
				break;

                        case 10  :
                                sprintf( strQuant, "%6.2f%%", timingAccuracy2 );
                                BRF_appendToColon( headLine, strQuant );
                                break;

		}/* Append info to proper lines */

		fprintf( brfFile, "%s", headLine );

	}/* Pipe output through */

	/*----------------------------------------------------------------*\
	|	Separate from next header, close current header
	\*----------------------------------------------------------------*/
	fprintf( brfFile, "%s\n",
"------------------------------------------------------------------------\n");


}/* BRF_loopTiming */

/*--------------------------------------------------------------------*\
| Procedure     :	void BRF_printHeader( *FILE )
|---------------------------------------------------------------------
| Description   :	Prints the BRF file header to the specified file.
|---------------------------------------------------------------------
| Return        :	
\*--------------------------------------------------------------------*/
BRF_printHeader( brfFile )
FILE		*brfFile;				/* BRF output file pointer		  */

{/* BRF_printHeader */

	int			i, j, k;
	char		headLine[ 80 ];
	char		*BRF_appendToColon(), *BRF_getDate();
	FILE		*brfHedFile;

	for ( i = 0, j = 0; i < N_HEADFILES; i++ )
	{/* Pipe header files through */

		/*------------------------------------------------------------*\
		|	Grab lines and pump through to BRF output file
		\*------------------------------------------------------------*/
		k=0;
		while ( BRF_hedrData[i][k] != (char *)NULL )
		{/* Append timestamp to last line of first header */
			strcpy(headLine, BRF_hedrData[i][k++]);

			if ( ++j == N_TITLELINES )
				BRF_appendToColon( headLine, BRF_getDate() );
			fprintf( brfFile, "%s", headLine );

		}/* Append timestamp to last line of first header */
#if 0
		/*------------------------------------------------------------*\
		|	Separate from next header, close current header
		\*------------------------------------------------------------*/
		fprintf( brfFile, "\n" );
#endif /* if  0 */

	}/* Pipe header files through */

}/* BRF_printHeader */

/*--------------------------------------------------------------------*\
| Procedure     :	char *BRF_getDate( void )
|---------------------------------------------------------------------
| Description   :	Gets a day/date string for timestamping.
|---------------------------------------------------------------------
| Return        :	Character string of form:
|					Mon Jul  3 19:46:39 PDT 1989, corrected for
|					local time zone and prevailing alternate zone.
\*--------------------------------------------------------------------*/
char *BRF_getDate()

{/* BRF_getDate */
#ifdef EXTERNALNOTE

        ATTENTION END PORT PROGRAMMERS!!!!!!!

	  The date function used in this block is standard to BSD UNIX
	systems and to most UNIX systems in general. It may NOT be
	supported on some UNIX systems, and will probably not
	exist on non_UNIX C compilers. 
#endif


#ifndef sgi		       /* SGI compiler complains about redeclaration */
	int gettimeofday();
#endif
        struct timeval tval, *tp;
        struct timezone tzone, *tzp;
	struct tm	*calendar;
	char *asctime_pt;
	char *line_ret;

	int status;

	tp = &tval;
	tzp = &tzone;

	tzone.tz_minuteswest = 0; /* Use system default here */
	tzone.tz_dsttime = 0; /* Use system default here */
        status = gettimeofday(tp, tzp);
	if(status == -1)
	{
		fprintf(stderr,"OOPS: Failure on gettimeofday.\n");
		fflush(stderr);
	
	}
	calendar = localtime(&tp->tv_sec);
 	asctime_pt = asctime( calendar );  
	/* remove the line return in this string */
	if ( (line_ret = index( asctime_pt, '\n')) != NULL)
		line_ret[0] = ' ';
 	return( asctime_pt );  
}/* BRF_getDate */



/*--------------------------------------------------------------------*\
| Procedure     :	char *BRF_appendEndBar( *char )
|---------------------------------------------------------------------
| Description   :	Appends the end bar to the end of a string
|---------------------------------------------------------------------
| Return        :	String with end bar in column 72
\*--------------------------------------------------------------------*/
char *BRF_appendEndBar( str1)
char	*str1;				/* Target string	 */
{/* BRF_appendToColon */

	char		toColon[ 80 ];
	int 		n_tocolon, sub_length;
	char		*line_ret;
	static int 	total_line_len = 72;
	
	/* remove any and all line return in this string */
	while ( (line_ret = index( str1, '\n')) != NULL)
		line_ret[0] = ' ';

	sub_length = strlen( str1 );
	if (sub_length < total_line_len)
	{
		strncat(str1,
"                                                                        ",
			(total_line_len-sub_length-1 ));
    	strcat(str1,"|\n");
	} else
	{
		str1[total_line_len-1] = '|';
		str1[total_line_len  ] = '\n';
		str1[total_line_len+1] = '\0';
	}
	return(str1);

}/* BRF_appendToColon */

