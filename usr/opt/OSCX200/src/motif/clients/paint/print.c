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
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Id: print.c,v 1.1.4.2 1993/06/25 22:47:21 Ronald_Hegli Exp $";
#endif		/* BuildSystemHeader */
/*
****************************************************************************
**                                                                          *
**  Copyright (c) 1987                                                      *
**  By DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                        *
**  All Rights Reserved
**                                                                          *
**  This software is furnished under a license and may be used and  copied  *
**  only  in  accordance  with  the  terms  of  such  license and with the  *
**  inclusion of the above copyright notice.  This software or  any  other  *
**  copies  thereof may not be provided or otherwise made available to any  *
**  other person.  No title to and ownership of  the  software  is  hereby  *
**  transferred.                                                            *
**                                                                          *
**  The information in this software is subject to change  without  notice  *
**  and  should  not  be  construed  as  a commitment by DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL assumes no responsibility for the use or  reliability  of  its  *
**  software on equipment which is not supplied by DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**   DECpaint - DECwindows paint program
**
**  AUTHOR
**
**   Daniel Latham, October 1987
**
**  ABSTRACT:
**
**   This module contains routines that handle printing to a file.
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**      12/8/88 dl
**      fix bug in creating print file name
**
**	10-DEC-1993	Dhiren M Patel
**		Fix ootb_bug 455. Enclosed the I18N fix in 
**		Get_Default_Pr_Filename_FP () in a if !defined (VMS) as 
**		the I18N bug fix was to a Unix specific bug. So the I18N 
**		fix will appear only on Unix systems.
**--
**/           

/* The following must come first on OSF/1 systems */
#include <time.h>

#include "paintrefs.h" 
#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <Xm/ToggleB.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif
#include "ps.h"
#ifdef PRINT
#define SIXEL_TYPE ".six"
#define POSTSCRIPT_TYPE ".ps"

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <DXm/DXmPrint.h>
#include <DXm/DXmCSText.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#if defined(VMS)
#include <unixlib.h>
#endif

#include <stdio.h>
#define OUTBUF_LENGTH 500

#define DEFAULT_ORIENTATION 0
#define PORTRAIT_ORIENTATION 1
#define LANDSCAPE_ORIENTATION 2

#define SCALE_WHOLE 0
#define SCALE_HORIZONTAL 1
#define SCALE_VERTICAL 2
#define SCALE_BOTH 3
#define CLIP_BOTH 4

/* color stuff -> */
/* Constants that describe the code bytes
 */

/* Deal with litte-endian or big-endian client (Postscript device
 * is big-endian). This macro reverses order if on little-endian machine.
 */
#define setorder_(c1, c2)                               \
{                                                       \
        if ((ones.ch[0]) && (bitspercomp != 8)) {       \
            if (bitspercomp == 1)                       \
                c1 = swap[(c2)&15]<<4 | swap[(c2)>>4];  \
            else if (bitspercomp == 4)                  \
                c1 = ((c2)&15)<<4 | (c2)>>4;            \
        }                                               \
        else c1 = c2;                                   \
}


#define Adj_modulo 8			/* Separation of adjust start pts */
#define Adj_maxlow (Adj_modulo*(Num_adj-1))
					/* Only need to get within 94 of 255 */
#define Num_colors 3			/* max # of color streams */
#define Num_arb 3			/* # of arbit-val strings maintained */
#define Num_adj 22			/* # of adj amts */
#define Num_hex 38			/* # of hex string lengths*/
#define Num_01	5			/* # of short strings */
#define Ten_same01 4			/* Val range of 10's digit in base95 */
#define Ten_samearb 2			/* Ditto for arbit-value strings */
#define Base_z 32			/* First code for repeated 0's */
#define Base_o (Base_z+Ten_same01)	/* Any # of 1's */
#define Base_a1 (Base_o+Ten_same01)	/* Any # in a1 */
#define Base_a2 (Base_a1+Ten_samearb)	/* Any # in a2 */
#define Base_a3 (Base_a2+Ten_samearb)	/* Any # in a3 */
#define Base_f1 (Base_a3+Ten_samearb)	/* Fill & any in a1 */
#define Base_f2 (Base_f1+Ten_samearb)	/* Fill & any in a2 */
#define Base_f3 (Base_f2+Ten_samearb)	/* Fill & any in a3 */
#define Base_noadj (Base_f3+Ten_samearb) /* Any # of ASCII chars */
#define Base_no1 (Base_noadj + 1)	/* 1 ASCII char */
#define Base_adj1 (Base_no1 + 1)	/* 1 adjusted char */
#define Base_adj (Base_adj1 + 3)	/* Any # of adjusted chars */
#define Base_hex (Base_adj + Num_adj)	/* 2 or more chars as hex string */
#define Base_z2 (Base_hex + Num_hex)	/* Short string of 0's */
#define Base_o2 (Base_z2 + Num_01)	/* Ditto, 1's */

/* Limits that control scan of image dat
 */
#define Min_same01 2			/* Shortest 1-char form of rpted 0/1 */
#define Max_same01 (95*Ten_same01-1)	/* Largest same-val 0 or 255 string */
#define Max_samearb (95*Ten_samearb-1)	/* Largest same-val arbit string */
#define Min_hex 2			/* Shortest regular hex string */
#define Max_hex 39			/* Largest hex string at a time */
#define Max_line 130			/* Largest line to have to read */
#define Min_adjust 4			/* Min chars to use adjust-compress */
#define Min_ascii 2			/* Min chars to use plain ascii */

/* Kinds of images we can process
 */
#define MONO_1 0			/* monochrome 1 bit per pixel */
#define MONO_4 (MONO_1 + 1)		/* monochrome 4 bits per pixel */
#define MONO_8 (MONO_4 + 1)		/* monochrome 8 bits per pixel */
#define RGB_begin (MONO_8 + 1)		/* color images */
#define RGB_8 RGB_begin			/* RGB 8 bits per pixel */
#define RGB_12 (RGB_8 + 1)		/* RGB 12 bits per pixel */
#define RGB_24 (RGB_12 + 1)		/* RGB 24 bits per pixel */

/* masks for 8 bit per pixel RGB on little endian machine */
#define l8mask1 7			/* 11100000 1'st 3 bits (red mask) */
#define l8mask2 56			/* 00011100 2'nd 3 bits (green mask) */
#define l8mask3 192			/* 00000011 last 2 bits (blue mask) */

/* masks for 8 bit per pixel RGB on big endian machine */
#define b8mask1 224			/* 11100000 1'st 3 bits (red mask) */
#define b8mask2 28			/* 00011100 2'nd 3 bits (green mask) */
#define b8mask3 3			/* 00000011 last 2 bits (blue mask) */

/* masks for 12 bit per pixel RGB on little endian machine */
#define l12mask1 15			/* 11110000 mask for 1'st nybble */
#define l12mask2 240			/* 00001111 mask for 2'nd nymmbe */

/* masks for 12 bit per pixel RGB on big endian machine */
#define b12mask1 240			/* 11110000 mask for 1'st nybble */
#define b12mask2 15			/* 00001111 mask for 2'nd nybble */

/* Data to keep track of what string was/is being output
 */
static union {long num; char ch[4];} ones;
					/* See setorder_ above */
static long min_same;			/* Controls compression throttle */
static long same;			/* Times it is repeated */
static unsigned char hex[]="0123456789ABCDEF";
static unsigned char swap[16]={0,8,4,12, 2,10,6,14, 1,9,5,13, 3,11,7,15};
					/* Swap bits */
static unsigned char cv34[8]={0,2,4,6,9,11,13,15};
					/* 3 bit code -> 4 bit code */
static unsigned char cv24[4]={0,5,10,15};	/* 2 bit code -> 4 bit code */
static int linelen;			/* Curr length of line */
static unsigned char qstring[Num_colors][Max_line];
					/* Room for q-ed diff chars */
static int qlen;			/* Curr # of chars in qstring */
static int arblast;			/* Simple FIFO ctl for arb strings */
static int arbmax[Num_arb];		/* Largest string so far of curr val */
static unsigned char arbval[Num_arb];		/* Curr val in this string */
static int arbbase[Num_arb*2] = {Base_a1, Base_a2, Base_a3,
				 Base_f1, Base_f2, Base_f3};
					/* Map arb string choice to code */
static int monochrome_output;		/* output designed form mono device */
static int convert_to_gray;		/* color -> grayscale */
static int zeromax;			/* image format is zero_max_intensity */
static int image_is_color;		/* pretty self explanatory */
static int image_data_format;		/* image type bpp and color vs mono */
static int begcstream;			/* stream index to begin with */
static int endcstream;			/* stream index to end with */
static unsigned long bitsperpix;		/* bits per pixel in image */
static unsigned long bitspercomp;		/* bits per component in image */
static int leftover;			/* number of chars left over */
/* monochrome case : begcstream = endcstream = 0 */		
/* rgb case : begcstream = 0, endcstream = 2 */		

FILE	*psfile;
static int pixel_ptr[MAX_COLORS];
static int pixel_gray[MAX_COLORS];
/* <- color stuff */

/* Print variables */
static char *form_ansi2 = "ANSI2"; 
static char *form_postscript = "PostScript(R)";
static char *tmp_file = "paint.tmp";
static XmString file_names[1];
static FILE *fid;
static char *outbuf;
static int porientation = DEFAULT_ORIENTATION;
static int fit_on_page = SCALE_WHOLE;
static int page_position_horizontal = JUSTIFY_LEFT;
static int page_position_vertical = JUSTIFY_TOP;

static int printer_count;
static Widget *printer_toggles;

static char pr_filename[256], pr_filename_new[256];
static XmString pr_printer_str, pr_printer_str_new;
static int pr_out_format, pr_out_format_new;
static int pr_out_dev, pr_out_dev_new;
static int pr_aspect_ratio, pr_aspect_ratio_new;
static int pr_h_alignment, pr_h_alignment_new;
static int pr_v_alignment, pr_v_alignment_new;
static int pr_send_to, pr_send_to_new;

static Widget pr_format_default;
static Widget pr_device_default;
static Widget pr_ar_default;
static Widget pr_h_align_default;
static Widget pr_v_align_default;
static Widget pr_send_to_default;

/* Scratch directory to write file into for printing */
#if !defined(VMS)
static char scratch_directory[] = "/tmp/";
#else
static char scratch_directory[] = "SYS$SCRATCH:";
#endif


/* color stuff -> */

void PsiLinend(len)
    int len;
{
	if (linelen + len > Max_line)
	{				/* About to overflow */
	    fprintf(psfile, "\n");
	    linelen = 0;
	}
	linelen += len;			/* Append (to possib re-inited) line */
	return;
}				 /* end routine */


void PsiDeqAscii(first, last, low, hi)
    int first, last, low, hi;
/*
 * FUNCTION:
 *	Output the specified chunk of qstring as Ascii string
 * INPUTS:
 *	first = 1st index in qstring
 *	last = last index in qstring
 *	low/hi = range of values in this chunk
 * RESULTS:
 *	
 * NOTES:
*/
{
	unsigned char printing[Max_line], *pc, *qc;
	int adjust, code, i;

/* Determine the transform needed to make chars printable
 */
	low = MIN (low, Adj_maxlow);	/* Insure not past highest-needed low */
	if (low >= 32 && hi <= 126)
	    adjust = 0;			/* Naturally printable */
	else adjust = low - 32;		/* From ps's point of view */

/* Copy and transform, and null term the string
 */
	qc = &qstring[begcstream][first];
					/* Start pos's for copy */
	pc = &printing[0];
	for (i=first; i<=last; i++) {
	    *pc++ = *qc++ - adjust;	/* Go opposite dir as postscript */
	}
	*pc = 0;			/* Make C happy */

/* Do actual output
 */
	PsiLinend(last-first+1+1);	/* Second +1 for code */
	if (!adjust)			/* Plain ASCII? */
	    code = Base_noadj;		/* Yes */
	else code = Base_adj + low/Adj_modulo;
					/* No, enum + base */
	fprintf(psfile, "%c%s\n", code, printing);
					/* No, do code + string itself */
	linelen = 0;			/* Always ends a line */
	return;
}				 /* end routine */



void PsiDeqHex(first, last)
    int first, last;
/*
 * FUNCTION:
 *	Output the specified chunk of qstring as hex string, or
 *	do special cases for 1-char strings
 * INPUTS:
 *	first/last = bounds of chunk
 * RESULTS:
 *	
 * NOTES:
*/
{
	unsigned char sc;
	int i, num, rest, h1, h2;
	int j, jnum;

/* Do 1-char perf special cases
 */
	if (first != last) goto usual;

	for (i = begcstream; i <= endcstream; i++)
					/* for each color stream */
	{
	    PsiLinend(2);		/* Put out Return if line too long */
	    sc = qstring[i][first];	/* Get the char */
	    if (sc >= 32 && sc <= 126)	/* Vanilla ASCII? */
		fprintf(psfile, "%c%c", Base_no1, sc);	/* Yes */
	    else {			/* No, an adjust case */
		if (sc < 32)		/* Low? */
		    fprintf(psfile, "%c%c", Base_adj1, sc+32);
		else if (sc < 127+95)	/* Medium? */
		    fprintf(psfile, "%c%c", Base_adj1+1, sc-95);
		else fprintf(psfile, "%c%c", Base_adj1+2, sc-190);
					/* High */
	    }
	}
	return;

/* Process chars 1 by 1, but generate chunk no bigger than Max_hex at a time
 */
usual:
	num = 0;
	j = endcstream;
	for (i=first; i<=last; i++) {
	    if (num <= 0)		/* Start of new chunk? */
	    {				/* Yes init things */
		if (j >= endcstream) {
		    j = begcstream;
		    rest = last-i+1;	/* Total left to process */
		    jnum = num = MIN (rest, Max_hex);
					/* Size of next chunk: rest or max */
		    rest -= num;	/* Reduce by just calc chunk, unless */
		    if (rest>0 && rest<Min_hex)
			jnum = (num -= Min_hex);	
					/* Was patho residue of <Min_hex */
		}
		else {
		    num = jnum;		/* process same sized chunk for each */
					/* color stream */
		    j++;		/* move to next stream */
		}
		PsiLinend(num+num + 1);	/* Code, + 2 hex digits/char */
		fprintf(psfile, "%c", Base_hex + num-Min_hex);
					/* Output cnt at start */
	    }

/* Generate next pair of hex digits
 */
	    sc = qstring[j][i];		/* Get curr char */
	    h1 = sc / 16;		/* High-order bits */
	    h2 = sc & 15;		/* Low order bits */
	    fprintf(psfile, "%c%c", hex[h1], hex[h2]);
					/* The 2 hex digits for curr char */
	    num -= 1;			/* Appended another char */
	    if (num <= 0) {		/* if out of chars */
		if (j < endcstream)	/* if there is another color stream */
		    i -= jnum;		/* process the next color stream */
	    }
	}
	return;
}				 /* end routine */



void PsiProlog()
{

/* Setup common variables. Note that the z(i) and o(i) strings share space
 * with the long strings of zeros and ones (ie. z and o).
 */
fprintf(psfile, "/bd{bind def}def /sd{string def}bd /U{0 exch getinterval def}bd\n");
fprintf(psfile, "/cf currentfile def /imstr 130 sd /h1 1 sd ");
fprintf(psfile, "/a1 190 sd /a2 190 sd /a3 190 sd /z 380 sd /o 380 sd\n");
if (image_is_color) 
    fprintf(psfile, "/imstr1 130 sd /imstr2 130 sd /strct 0 def /h2 1 sd /h3 1 sd /hct 2 def\n");
fprintf(psfile, "/z2 z 2 U /z3 z 3 U /z4 z 4 U /z5 z 5 U /z6 z 6 U\n");
fprintf(psfile, "/o2 o 2 U /o3 o 3 U /o4 o 4 U /o5 o 5 U /o6 o 6 U\n");


/* {I} eats image data. Image format is [code] [data] per chunk,
 * where code is a single character and data is code dependent.
 * Groop 0 -- skip control codes (eg. Return) in the stream.
 * Group 1 -- repeat same value:
 *	Data is 1 byte indicating one's digit of repeat count in base 95.
 *	The code embodies the value of the 10's digit.
 *	a) String of 0 bytes.
 *	b) String of all-ones bytes
 *	c) The value currently in a1
 *	d) The value currently in a2
 *	e) The value currently in a3
 * Group 2 -- setup and repeat same value:
 *	Data is 1 byte indicating value to set each byte to,
 *	plus 2 hex digits for val to fill width.
 *	a) Fill in a1, then use it
 *	b) Fill in a2, then use it
 *	c) Fill in a3, then use it
 * Group 3 -- diff values (read arbit hex string, & cases to avoid hex string):
 *	a) Arbit string of ASCII chars
 *	b) String of 1 ASCII char
 *	c) String of 1 char, outside range of 32-126
 *	d) Arbit string of chars, with adjustment factor (22 cases)
 *	e) Hex string, 2-39 long (the totally uncompressed case)
 * Group 4 -- short specific-length strings of 0's and 1's
 *	a) z(i) -- zeroes
 *	b) o(i) -- ones
 */
fprintf(psfile, "/I {codes cf read pop get exec} bd\n");
					/* Get code's proc from array & exec */
fprintf(psfile, "/codes [{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}{I}\n");
						/* 0-31 */
fprintf(psfile, "{z 0 -32 S}{z 0 63 S}{z 0 158 S}{z 0 253 S}");
						/* Sp ! " # */
fprintf(psfile, "{o 0 -32 S}{o 0 63 S}{o 0 158 S}{o 0 253 S}\n");
						/* $ % & ' */
fprintf(psfile, "{a1 0 -32 S}{a1 0 63 S}{a2 0 -32 S}{a2 0 63 S}{a3 0 -32 S}{a3 0 63 S}\n");
						/* ( ) * + , - */
fprintf(psfile, "{a1 -32 F}{a1 63 F}{a2 -32 F}{a2 63 F}{a3 -32 F}{a3 63 F}\n");
						/* . / 0 1 2 3 */
fprintf(psfile, "{Nn}{N1}{%s 0 -32 C}{%s 0 95 C}{%s 0 190 C}\n",
   image_is_color ? "newh" : "h1", image_is_color ? "newh" : "h1",
   image_is_color ? "newh" : "h1");
						/* 4 thru 8 */
fprintf(psfile, "{-32 A}{-24 A}{-16 A}{-8 A}{0}{8 A}{16 A}{24 A}{32 A}{40 A}{48 A}{56 A}\n");
fprintf(psfile, "{64 A}{72 A}{80 A}{88 A}{96 A}{104 A}{112 A}{120 A}{128 A}{136 A}\n");
						/* 9 thru N */
fprintf(psfile, "{2 H}{3 H}{4 H}{5 H}{6 H}{7 H}{8 H}{9 H}");
fprintf(psfile, "{10 H}{11 H}{12 H}{13 H}{14 H}{15 H}{16 H}{17 H}{18 H}{19 H}\n");
fprintf(psfile, "{20 H}{21 H}{22 H}{23 H}{24 H}{25 H}{26 H}{27 H}{28 H}{29 H}\n");
fprintf(psfile, "{30 H}{31 H}{32 H}{33 H}{34 H}{35 H}{36 H}{37 H}{38 H}{39 H}\n");
						/* O thru t */
fprintf(psfile, "z2 z3 z4 z5 z6 o2 o3 o4 o5 o6] def\n");
						/* u thru ~ */

/* Read a long hex string (at least 2 chars, up thru 39 chars)
 */
fprintf(psfile, "/H {cf %s 0 4 -1 roll getinterval readhexstring pop} bd\n",
   image_is_color ? "getstr" : "imstr");
				/* Read desired len hex string, and done */

/* The main adjust code-proc:
 * adjusts each char of arbitrarily long string into actual range of 95 to use.
 * Data is string of bytes ending in Return.
 * Arg is how much to adjust bytes of string to get true value in image.
 * When "string 0 string" left on stack, the top string is arg to forall,
 * the other 2 are for rpted puts.
 */
fprintf(psfile, "/A {/val exch def cf %s readline pop dup 0 exch\n",
   image_is_color ? "getstr" : "imstr");
				/* Setup adj amt & put "str 0 str" on stack */
fprintf(psfile,	"{val add 3 copy put pop 1 add} forall ");
				/* Adjust, dup put's 3 args, do put, &
				 leave "string i+1" on stack for next iter */
fprintf(psfile,	"pop} bd\n");
				/* Pop "i", returning string on top for image */

/* The other avoid-hex-string procs:
 * nn (arbit long string all in 32-126 range). Data is: string.
 * n1 (1 char in 32-126 range). Data is: 1 char.
 * C (any other 1 char). Usage is h1 0 amt C. Data is: 1 char.
 */
fprintf(psfile, "/Nn {cf %s readline pop} bd\n", image_is_color ? "getstr" : "imstr");
fprintf(psfile, "/N1 {cf %s readstring pop} bd\n", image_is_color ? "newh" : "h1");
fprintf(psfile, "/C {cf read pop add put %s} bd\n", image_is_color ? "curh" : "h1");

/* The procedure for reading same val n times:
 * Args: string-to-get-from 0 val-to-add-to-data-byte-to-get-n
 * Return value: string for image-op to process
 * The procedure for initing string with n vals & reading n vals out of it:
 * Args: string-to-get-from  val-to-add-to-data-byte-to-get-n
 * Return value: string for image-op to process
*/
fprintf(psfile, "/S {cf read pop add getinterval} bd\n");
				/* Gen substring for image machinery */
fprintf(psfile, "/F {cf read pop add ");
				/* Leave str len on stack */
fprintf(psfile,	"cf %s readhexstring pop 0 get exch dofill} bd\n",
   image_is_color ? "newh" : "h1");
				/* Get val to fill with and fill in string */

/* This proc generates a string containing all the same char, eg. "QQQQQ"
 * Usage: string-to-fill  val-to-fill-with  length dofill
 * Returns: filled substring
 */
fprintf(psfile, "/dofill {/len exch def 2 copy ");
				/* Save len & setup "string val" */
fprintf(psfile, "0 1 len 1 sub {exch put 2 copy} for ");
				/* Loop thru string, exchanging index & val
				 for put, & setup "string val" for next iter */
fprintf(psfile, "pop pop pop 0 len getinterval} bd\n");
				/* Get rid of "val string val" & finish def */

/* This proc picks one of the imstr's to use.  If processing a color image,
 *  must use a different string for each of RGB 
 */
if (image_is_color) {
    fprintf(psfile, "/getstr {strct 0 eq {imstr} {strct 1 eq {imstr1} {imstr2} ifelse} ifelse\n");
				/* use cur string based on strct */
    fprintf(psfile, "	       /strct strct 1 add def strct 3 eq {/strct 0 def} if} bd\n");
				/* increment strct by 1 use cur string */
				/* strct wraps to 0 after 2 */

    fprintf(psfile, "/newh {/hct hct 1 add def hct 3 eq {/hct 0 def} if curh} bd\n");
    fprintf(psfile, "/curh {hct 0 eq {h1} {hct 1 eq {h2} {h3} ifelse} ifelse} bd\n");
}

fprintf(psfile, "o 255 380 dofill pop\n");	/* Init "ones" string */

/* If the image is color, put in the fallback for the colorimage operator
 * for b/w printers
 */
if (image_is_color) {
    fprintf(psfile, "/colorimage where\n{ pop }\n{ /colorimage\n  { pop pop pop pop pop\n");
				/* pop colorimage args off the stack */
    fprintf(psfile, "    { /rstr I def /gstr I def /bstr I def\n");
				/* read three chunks one for each of RGB */
				/* place them in strings rstr, gstr, bstr */
    fprintf(psfile, "      /grstr rstr length string def\n      0 1 grstr length 1 sub\n");
				/* create the gray string, loop thru */
    fprintf(psfile, "      { /idx exch def\n");

/* create the gray value byte for each 3 bytes of color info 
 * using the GRAY = 0.299(R) + 0.587(G) + 0.114(B) algorithm
 */
    if (bitspercomp == 8) {
	fprintf(psfile, "        rstr idx get 0.299 mul\n");
	fprintf(psfile, "        gstr idx get 0.587 mul add\n");
	fprintf(psfile, "        bstr idx get 0.114 mul add\n");
	fprintf(psfile, "        0.5 add cvi grstr idx 3 -1 roll put\n");
    }
    else if (bitspercomp == 4) {
	fprintf(psfile, "        rstr idx get dup 16 idiv dup .299 mul /hi4 exch def 16 mul sub .299 mul /lo4 exch def\n");
	fprintf(psfile, "        gstr idx get dup 16 idiv dup .587 mul hi4 add /hi4 exch def 16 mul sub .587 mul lo4 add /lo4 exch def\n");
	fprintf(psfile, "        bstr idx get dup 16 idiv dup .114 mul hi4 add /hi4 exch def 16 mul sub .114 mul lo4 add /lo4 exch def\n");
	fprintf(psfile, "        grstr idx hi4 0.5 add cvi 16 mul lo4 0.5 add cvi add put\n");
    }
    fprintf(psfile, "      } for\n      grstr\n    } image\n  } bind def\n}\nifelse\n");
}
	return;
}				 /* end routine */


static int PsiAsciiOk(numok, low, hi)
/*
 * FUNCTION:
 *	Det if string is long enough to not use hex string
 * INPUTS:
 *	numok = len of string
 *	low/hi = range of values
 * RESULTS:
 *	Returns 1 if long enough, else 0
 * NOTES:
*/
{
	if (low >= 32 && hi <= 126)
	{				/* Lower min applies */
	    if (numok >= Min_ascii)
		return 1;
	}
	else {				/* Higher min applies */
	    if (numok >= Min_adjust)
		return 1;
	}
	return 0;			/* To few, just use hex */
}				 /* end routine */


void PsiDeqString()
{
	unsigned char sc;
	int i, numok, first, last, low, hi;
	int basemask;			/* See Bas_adj's cases */

	if (qlen <= 0) return;		/* No chars */
	low = hi = qstring[begcstream][0];
					/* First inrange with itself */
	numok = 1;
	first = 0;

/* Do escape for low_res (avoid short chunks)
 */
/* If image is color then just print hex chars
*/
	if (image_is_color)
	{					/* Yes */
	    PsiDeqHex(0, qlen-1);		/* Dump whole thing as hex */
	    qlen = 0;				/* Start afresh */
	    return;
	}
	    
/* Scan the q-ed chars
 */
	basemask = ~(Adj_modulo - 1);	/* Detect aligned range of 95 */
	for (i=1; i<qlen; i++) {
	    sc = qstring[begcstream][i];
	    if (hi-(sc&basemask) > 94 || sc-(low&basemask) > 94)
	    {				/* Out of 95-wide range */
		if (PsiAsciiOk(numok, low, hi))
		{			/* Enough inrange to use (aaa) */
		    last = i-1 - numok;	/* Last of: must do hex */
		    if (last >= first) PsiDeqHex(first, last);
					/* Do the "bad" if any */
		    PsiDeqAscii(last+1, i-1, low&basemask, hi);
		    first = i;		/* Deq-ed it all */
		}
		low = hi = sc;		/* First inrange with itself */
		numok = 1;
	    }

/* Continue q-ing an in-range run
 */
	    else {			/* No */
		if (sc < low) low = sc;	/* Stretch down */
		if (sc > hi) hi = sc;	/* Stretch up */
		numok += 1;
	    }
	}

/* Dump the residue
 */
	if (PsiAsciiOk(numok, low, hi))
	    last = qlen-1 - numok;	/* Enough inrange to use (aaa) */
	else last = qlen-1;		/* No */
	if (last >= first) PsiDeqHex(first, last);
					/* Do the "bad" if any */
	if (last!=qlen-1) PsiDeqAscii(last+1, qlen-1, low&basemask, hi);
					/* Do the "good" if any */
	qlen = 0;
	return;
}				 /* end routine */


int PsiRepeat(same, base, c)
    int same, *base;
    unsigned char *c;
/*
 * FUNCTION:
 *	Output needed # of coded char-pairs to represent indicated rpt count
 * INPUTS:
 *	same = # to output
 *	base = case that applies (eg. 0's)
 * RESULTS:
 *	Returns residue could not output (usually 0)
 * NOTES:
 *	Conceptually the loop at the end could be replace by recursive call
 *	In practice, same can be so high that the recursion would blow the stack
*/
{
	int chunk, currmax, cur_arb;
	int tens, residue;
	int first_time = TRUE;
	int first_fill = FALSE;
	int h1, h2;
	int i;

/* Decide how many at a time can be done
 */
	currmax = Max_same01;		/* assume all 0's or 1's */ 
	for (i = begcstream; i <= endcstream; i++) {
	    if ((base[i] != Base_o) && (base[i] != Base_z)) {
		currmax = Max_samearb;	/* not all 0's or 1's */
	    }
	}

/* Do actual output
 */
	while (same > 0) {
	    chunk = MIN (same, currmax);	/* Do as much as can at a time */
	    tens = chunk / 95;		/* # of tens in base95 */
	    residue = chunk % 95;	/* # left over */
	    for (i = begcstream; i <= endcstream; i++) {
		if (((base[i] == Base_o) || (base[i] == Base_z)) &&
		    ((chunk >= Min_same01) && (chunk <= Min_same01+Num_01-1))) {
		    PsiLinend(1);
		    if (base[i] == Base_z)
			fprintf (psfile, "%c", Base_z2 + same-Min_same01);
		    else fprintf (psfile, "%c", Base_o2 + same-Min_same01);
		}
		else {
		    if (first_time && c[i]) {
			cur_arb = (base[i] - Base_a1) / Ten_samearb;
			if (arbmax[cur_arb] < chunk) {
			    PsiLinend(4);
			    arbmax[cur_arb] = chunk;
					/* save largest so far */
			    base[i] += Num_arb * Ten_samearb;
					/* use fill base */
			    first_fill = TRUE;
			}
		    }
		    PsiLinend(2);	/* Put out Return if line too long */
		    fprintf (psfile, "%c%c", base[i]+tens, 32+residue);
		    if (first_fill) {
			h1 = c[i] /16;	/* High-order bits */
			h2 = c[i] & 15;	/* Low order bits */
			fprintf (psfile, "%c%c", hex[h1], hex[h2]);
					 /* The 2 hex digits for char to rpt */
			base[i] -= Num_arb * Ten_samearb;
					 /* use rpt base for the rest */
			first_fill = FALSE;
		    }
		}
	    }
	    first_time = FALSE;
	    same -= chunk;	/* Acct for what just output */
	}
	return same;
}				 /* end routine */



int PsiGenSame (pix, same)
    unsigned char pix;
    int same;
/*
 * FUNCTION:
 *	Dequeue last string chunk and gen compressed chunk
 * INPUTS:
 *	pix = pixel to be repeated
 *	same = number of them
 * RESULTS:
 *	Returns residue (usually 0)
 * NOTES:
*/
{
	unsigned char c[Num_colors], sc[Num_colors];
	int i, j, chunk;
	int base[Num_colors];
	int arbused[Num_arb];		/* true if if the arb is used by a */
					/* prev color component in the pixel */

/* Chk for easy special cases: 0's or 1's
 * PsiRepeat outputs as many code-count pairs as needed for same
 */
	PsiDeqString();			/* Dequeue whatever was there */
/* c contains the values to repeat for each color - 0 signifies no repeat
 * value (ie. all 0's byte or all 1's byte)
 */
	c[0] = c[1] = c[2] = 0;
	arbused[0] = arbused[1] = arbused[2] = FALSE;

	switch (image_data_format) {
	    case MONO_1 :
		sc[0] = pix;
		break;
	    case MONO_8 :
		if (convert_to_gray) {
		    sc[0] = pixel_gray[pix];
		}
		else {
		    sc[0] = colormap[pixel_ptr[pix]].i_green >> 8;
		}
		break;
	    case RGB_24 :
		sc[0] = colormap[pixel_ptr[pix]].i_red >> 8;
		sc[1] = colormap[pixel_ptr[pix]].i_green >> 8;
		sc[2] = colormap[pixel_ptr[pix]].i_blue >> 8;
		break;
	}

	for (j = begcstream; j <= endcstream; j++) {
	    if (sc[j] == 0)		/* Chunk of 0's? */
		base[j] = Base_z;
	    else if (sc[j] == 255)
		base[j] = Base_o;
	    else {
/* Setup to use one of the variable strings, filling it 1st if neces
 */
		setorder_(c[j], sc[j]);	/* Reverse bits as needed */
		for (i=0; i<Num_arb; i++) {
		    if (arbval[i] == c[j]) break;
		}
		if (i>=Num_arb)		/* No match */
		{			/* Choose one to zap */
		    do {
			arblast = i = arblast + 1;
			if (i >= Num_arb)	/* Need to recycle? */
			    arblast = i = 0;	/* Yes */
		    } while (arbused[i]);
		    arbmax[i] = 0;	/* Ensure fill-case done */
		    arbval[i] = c[j];	/* Zap old value */
		}
		base[j] = arbbase[i];
		arbused[i] = TRUE;
	    }
	}

	return PsiRepeat(same, base, c);
}					/* end routine */




void PsiQueueSame(pix, same)
    unsigned char pix;
    int same;
{
	unsigned char c[Num_colors], sc[Num_colors];
	int i, j;

/* Append chars to end up q-ed stuff
 */
	switch (image_data_format) {
	    case MONO_1 :
		sc[0] = pix;
		break;
	    case MONO_8 :
		if (convert_to_gray) {
		    sc[0] = pixel_gray[pix];
		}
		else {
		    sc[0] = colormap[pixel_ptr[pix]].i_green >> 8;
		}
		break;
	    case RGB_24 :
		sc[0] = colormap[pixel_ptr[pix]].i_red >> 8;
		sc[1] = colormap[pixel_ptr[pix]].i_green >> 8;
		sc[2] = colormap[pixel_ptr[pix]].i_blue >> 8;
		break;
	}


	for (i = begcstream; i <= endcstream; i++ ) {
	    setorder_(c[i], sc[i]);	/* Reverse bits as needed */
	}
	for (i=0; i<same; i++) {
	    if (qlen >= Max_line)	/* About to overflow? */
		PsiDeqString();		/* Yes, dequeue the stuff */
	    for (j = begcstream; j <= endcstream; j++ ) {
		qstring[j][qlen] = c[j];
	    }
	    qlen += 1;
	}
	return;
}				 /* end routine */




void PsiReceive (ximage)
    XImage *ximage;
{
    unsigned char sc;		/* Value of last char scanned */
    unsigned char c, *cp, mask;
    int i, j, k;
    int wd, ht, step = 1;

    ones.num = 0xffff;		/* For little/big-endian chk */

    for (i = 0; i < num_colors; i++) {
	pixel_ptr[colormap[i].pixel] = i;
    }
    for (i = 0; i < MAX_COLORS; i++) {
	pixel_gray[i] = i;
    }

/* if converting color to grey */
    if (convert_to_gray) {
	for (i = 0; i < num_colors; i++) {
	    pixel_gray[colormap[i].pixel] = 
		Convert_Color_To_Grey (&(colormap[i]));
	}
    }

    min_same = 2;
    arblast = 2;
    arbmax[0] = arbmax[1] = arbmax[2] = 0;
    arbval[0] = arbval[1] = arbval[2] = 0;
    linelen = qlen = same = 0;

    ht = ximage->height;
    if (pdepth == 1) {
	wd = ximage->width / 8 + ((ximage->width % 8) ? 1 : 0);
    }
    else {
	wd = ximage->width;
	if (image_data_format == MONO_1)
	    step = 8;
    }

/* Scan thru curr line of bits, a byte at a time
 */
    for (i = 0; i < ht; i++) {
	cp = (unsigned char *)ximage->data + (i * ximage->bytes_per_line);
	for (j = 0; j < wd; j += step) {
	    if (image_data_format == MONO_1) {
		if (pdepth == 1) {
		    c = ~*cp;
		}
		else {
		    c = 0;
		    if (ones.ch[0])
			mask = (ones.ch[0]) ? 1 : 128;
		    for (k = 0; (k < 8) && (j + k < wd); k++) {
			if (pixel_ptr[*(cp + k)] == WHITE)
			    c += mask;
			if (ones.ch[0])
			    mask <<= 1;
			else 
			    mask >>= 1;
		    }
		}
	    }
	    else {
		c = *cp;
	    }

	    if (!same)		/* Starting new seq? */
	    {			/* Yes */
		sc = c;
		same = 1;
	    }

/* More of same
 */
	    else if (pixel_gray[sc] == pixel_gray[c])
				/* Curr char = same as before? */
		same += 1;	/* Yes, bump cnt */

/* A byte that is not the same
 */
	    else {		/* No, start anew */
		if (same >= min_same)
		    same = PsiGenSame (sc, same);
		if (same)
		    PsiQueueSame (sc, same);
		sc = c;		/* Diff curr 1st of next chunk */
		same = 1;
	    }
	    cp += step;
	}			/* End of loop */
    }
    if (same >= min_same)
	same = PsiGenSame(sc, same);
    if (same) {
	PsiQueueSame(sc, same);
	PsiDeqString();
    }
    fprintf (psfile, "\n");
}				 /* end routine */

/* <- color stuff */

void Get_Default_Pr_Filename (format, filename)
    int format;
    char *filename;
{
    char ftype[10];
    char *period;
    int i;

    if (format == PR_PS_FORMAT)
	strcpy (ftype, POSTSCRIPT_TYPE);
    else 
	if (format == PR_SIXEL_FORMAT)
	   strcpy (ftype, SIXEL_TYPE);

    i = 0;
/* dl - 12/8/88 check to see if there is a period in the cur_name.  If there
 * is only copy the name up to and not including the period.
 */
    period = strchr (cur_name, (int)'.');
    if (period) {
        strncpy (&filename[i], cur_name, period - cur_name);
        i += period - cur_name;
    }
    else{
	strncpy (&filename[i], cur_name, strlen(cur_name));
        i += strlen(cur_name);
    }

    strncpy (&filename[i], ftype, strlen(ftype));
    i += strlen(ftype);
    filename[i] = '\0';
}



void Get_Default_Pr_Filename_FP (format, fullpath)
    int format;
    char *fullpath;
{
    char filename[256];
    int i;

    Get_Default_Pr_Filename (format, filename);

/* get the current working directory */
    getcwd (fullpath, 256);
    i = strlen(fullpath);
 
#if !defined (VMS) 	/* a unix specific bug i.e. put a "/" only on unix */
#ifdef I18N_BUG_FIX /* fix corporate bug */
    fullpath[i]='/';
    i++;
#endif /* I18N_BUG_FIX */
#endif
    strncpy (&fullpath[i], filename, strlen(filename));
    i += strlen(filename);
    fullpath[i] = '\0';
}


/* changes the printer choice in the print_2 dialog */
XtCallbackProc Change_Printer (printer)
    XmString printer;
{
    XmString tmp_printer;

    Get_Attribute (print_2_dialog, DXmNprinterChoice, &tmp_printer);
    if (!XmStringByteCompare (printer, tmp_printer))
	Set_Attribute (print_2_dialog, DXmNprinterChoice, printer);
/*
    XmStringFree (tmp_printer);
*/
}


/* changes the print format in the print_2 dialog */
void Change_Print_Format (format)
    int format;
{
    XmString new_format_str, cur_format_str;
    Arg args [5];
    long bc, status;

    if (format == PR_PS_FORMAT)
	new_format_str = DXmCvtOStoCS (form_postscript, &bc, &status);
/*	new_format_str = XmStringLtoRCreate(form_postscript , "ISO8859-1"); */
    else
	new_format_str = DXmCvtOStoCS (form_ansi2, &bc, &status);
/*	new_format_str = XmStringLtoRCreate(form_ansi2 , "ISO8859-1"); */

    Get_Attribute (print_2_dialog, DXmNprintFormatChoice, &cur_format_str);
    if (!XmStringByteCompare (new_format_str, cur_format_str)) {
	XtSetArg (args[0], DXmNprintFormatList, &new_format_str);
	XtSetArg (args[1], DXmNprintFormatCount, 1);
	XtSetArg (args[2], DXmNprintFormatChoice, new_format_str);
	XtSetValues (print_2_dialog, args, 3);
    }
    XmStringFree (new_format_str);
/*
    XmStringFree (cur_format_str);
*/
}


void Change_Pr_Printer (w, button_id, r)
    Widget w;
    int *button_id;
    XmToggleButtonCallbackStruct *r;
{
    XmString cs;

    if (r->reason == XmCR_VALUE_CHANGED)
        if (r->set) {
	    Get_Attribute (w, XmNlabelString, &cs);
	    if (pr_printer_str_new)
		XmStringFree (pr_printer_str_new);
	    pr_printer_str_new = XmStringCopy(cs);
	    Change_Printer (cs);
	    XmStringFree (cs);
	}
}


/* removes the toggle buttons which are children of the printer option menu
 * in the print dialog box.
 */
void Delete_Printer_Toggles ()
{
    Arg args [3];
    int i;
    int count = printer_count;

    if (printer_count == 0) {
	XtSetSensitive (widget_ids[PR_PRINTERS_OPTION_MENU], SENSITIVE);
	XtSetSensitive (widget_ids[PR_SEND_TO_RADIO_BOX], SENSITIVE);
	count = 1;
    }

    XtUnmanageChildren (printer_toggles, count);
    for (i = 0; i < count; i++) {
        XtDestroyWidget (printer_toggles[i]);
        printer_toggles[i] = 0;
    }
    XtFree ((char*)printer_toggles);
    printer_count = 0;
/* set width and height of pulldown menu to be 0 */
/*
    XtSetArg (args[0], XmNwidth, 0);
    XtSetArg (args[1], XmNheight, 0);
    XtSetValues (widget_ids[PR_PRINTERS_PULLDOWN_MENU], args, 2);
*/
}


static XtCallbackRec change_printer_callback [2] =  /* change printer callback */
{
    { (XtCallbackProc) Change_Pr_Printer, NULL}, 
    NULL
};

/* Adds toggles to the printer option menu in the print dialog box - the names
 * of the printers are in cs_list and count tells how many there are.
 */
void Add_Printer_Toggles (cs_list, count)
    XmString *cs_list;
    int count;
{
    Arg args [5];
    int i;
    int tmp_count = count;
    XmString tmp_xmstr;
    long bc, status;

    if (count == 0)
	tmp_count = 1;
    printer_toggles = (Widget *) XtCalloc (tmp_count, sizeof (Widget));
    XtSetArg (args[0], XmNindicatorOn, 0);
    XtSetArg (args[1], XmNvalueChangedCallback, change_printer_callback);
    for (i = 0; i < count; i++) {
        XtSetArg (args[2], XmNlabelString, cs_list[i]);
        printer_toggles[i] = 
	    XmCreateToggleButton (widget_ids[PR_PRINTERS_PULLDOWN_MENU], "",
				   args, 3);
    }
    if (count == 0) {
/* Set up a placeholder toggle */
	tmp_xmstr = DXmCvtOStoCS ("          ", &bc, &status);
	XtSetArg (args[2], XmNlabelString, tmp_xmstr);
	XmStringFree (tmp_xmstr);
	printer_toggles [0] = 
	    XmCreateToggleButton (widget_ids[PR_PRINTERS_PULLDOWN_MENU], "",
				  args, 3);
	XtSetSensitive (widget_ids[PR_PRINTERS_OPTION_MENU], INSENSITIVE);
	if (pr_send_to_new != PR_SEND_TO_FILE) {
	    if (pr_send_to_new == PR_SEND_TO_PRINTER) {
		XmToggleButtonSetState (widget_ids[PR_SEND_TO_PRINTER_TOGGLE],
					FALSE, TRUE);
	    }
	    else {
		XmToggleButtonSetState (widget_ids[PR_SEND_TO_BOTH_TOGGLE],
					FALSE, TRUE);
	    }
	    pr_send_to_new = PR_SEND_TO_FILE;
	    XmToggleButtonSetState (widget_ids[PR_SEND_TO_FILE_TOGGLE], TRUE,
				    TRUE);
	    Set_Attribute (widget_ids[PR_SEND_TO_RADIO_BOX], XmNmenuHistory,
			   widget_ids[PR_SEND_TO_FILE_TOGGLE]);
	}
	XtSetSensitive (widget_ids[PR_SEND_TO_RADIO_BOX], INSENSITIVE);
    }
    printer_count = count;
    XtManageChildren (printer_toggles, tmp_count);
}


/* Sets the printer choice in the print dialog box - to be printer denoted by
 * cs.
 */
void Set_Printer_Choice (cs)
    XmString cs;
{
    int i;
    XmString label;
    char *label_ascii, *cs_ascii;
    long bc, status;

    cs_ascii = (char *)DXmCvtCStoOS (cs, &bc, &status);
    for (i = 0; i < printer_count; i++)
    {
	Get_Attribute (printer_toggles[i], XmNlabelString, &label);
	label_ascii = (char *)DXmCvtCStoOS (label, &bc, &status);
	if (!strcmp (cs_ascii, label_ascii))
	{
	    XmStringFree (label);
	    XtFree (label_ascii);
	    break;
	}
	XmStringFree (label);
	XtFree (label_ascii);
    }
    XtFree (cs_ascii);

    if (i < printer_count) {
	Set_Attribute (widget_ids[PR_PRINTERS_OPTION_MENU], XmNmenuHistory,
		       printer_toggles[i]);
	if (pr_printer_str_new)
	    XmStringFree (pr_printer_str_new);
	pr_printer_str_new = XmStringCopy(cs);
    }

    if (printer_count == 0) {
	Set_Attribute (widget_ids[PR_PRINTERS_OPTION_MENU], XmNmenuHistory,
		       printer_toggles[0]);
    }
}


/* changes printers option menu in print dialog box - based on contents of 
 * print2 dialog box.
 */
void Change_Printer_Toggles ()
{
    Arg args[5];
    XmString cs, *cs_list;
    int count;
    int i;

    XtSetArg (args[0], DXmNprinterCount, &count);
    XtSetArg (args[1], DXmNprinterList, &cs_list);
    XtSetArg (args[2], DXmNprinterChoice, &cs);
    XtGetValues (print_2_dialog, args, 3);

    Delete_Printer_Toggles ();
    Add_Printer_Toggles (cs_list, count);
    Set_Printer_Choice (cs);
/*
    XmStringFree (cs);
    for (i = 0; i < count; i++)
	XmStringFree (cs_list[i]);
*/
}


/* Induces a change in output format in print_2 dialog - based on format.
 * Induces a change in printers option menu in print dialog.
 */
void Set_Output_Format (format)
    int format;
{
    Change_Print_Format (format);
    Change_Printer_Toggles ();
}


void Change_Aspect_Ratio (w, button_id, r)
    Widget w;
    int *button_id;
    XmToggleButtonCallbackStruct *r;
{
    if (r->reason == XmCR_VALUE_CHANGED)
	if (r->set)
	    pr_aspect_ratio_new = *button_id;
}


void Change_Pr_H_Alignment (w, button_id, r)
    Widget w;
    int *button_id;
    XmToggleButtonCallbackStruct *r;
{
    if (r->reason == XmCR_VALUE_CHANGED)
	if (r->set)
	    pr_h_alignment_new = *button_id;
}


void Change_Pr_V_Alignment (w, button_id, r)
    Widget w;
    int *button_id;
    XmToggleButtonCallbackStruct *r;
{
    if (r->reason == XmCR_VALUE_CHANGED)
	if (r->set)
	    pr_v_alignment_new = *button_id;
}


void Change_Pr_Output_Device (w, button_id, r)
    Widget w;
    int *button_id;
    XmToggleButtonCallbackStruct *r;
{
    if (r->reason == XmCR_VALUE_CHANGED)
	if (r->set)
	    pr_out_dev_new = *button_id;
}


void Change_Pr_Send_To (w, button_id, r)
    Widget w;
    int *button_id;
    XmToggleButtonCallbackStruct *r;
{
    if (r->reason == XmCR_VALUE_CHANGED)
        if (r->set)
            pr_send_to_new = *button_id;
}




/* Sensitize or desensitize option menus based on format */
void Set_Sensitive_Option_Menus (format)
    int format;
{

    switch (format) {
	case PR_PS_FORMAT :
/* sensitize output device, h alignment, v alignment.
 * desensitize aspect ratio.
 */
	    XtSetSensitive (widget_ids[PR_AR_OPTION_MENU], INSENSITIVE);
	    XtSetSensitive (widget_ids[PR_DEVICE_OPTION_MENU], SENSITIVE);
	    XtSetSensitive (widget_ids[PR_H_ALIGN_OPTION_MENU], SENSITIVE);
	    XtSetSensitive (widget_ids[PR_V_ALIGN_OPTION_MENU], SENSITIVE);
	    break;
	case PR_SIXEL_FORMAT :
/* sensitize aspect ratio.
 * desensitize output device, h alignment, v alignment.
 */
	    XtSetSensitive (widget_ids[PR_DEVICE_OPTION_MENU], INSENSITIVE);
	    XtSetSensitive (widget_ids[PR_H_ALIGN_OPTION_MENU], INSENSITIVE);
	    XtSetSensitive (widget_ids[PR_V_ALIGN_OPTION_MENU], INSENSITIVE);
	    XtSetSensitive (widget_ids[PR_AR_OPTION_MENU], SENSITIVE);
	    break;
    }
}


void Change_Pr_Output_Format (w, button_id, r)
    Widget w;
    int *button_id;
    XmToggleButtonCallbackStruct *r;
{
    XmString tmp_xmstr;
    long bc, status;

    if (r->reason == XmCR_VALUE_CHANGED)
	if (r->set) {
	    pr_out_format_new = *button_id;
	    switch (pr_out_format_new) {
		case PR_PS_FORMAT :
/* For PS set ratio to 1:1  */
		    Set_Attribute (widget_ids[PR_AR_OPTION_MENU],
				   XmNmenuHistory, 
				   widget_ids[PR_AR_1_TO_1_TOGGLE]);
		    break;
		case PR_SIXEL_FORMAT :
/* For Sixel, set output device to monochrome, set h alignment to left,
 * set v alignment to top.
 */
		    Set_Attribute (widget_ids[PR_DEVICE_OPTION_MENU],
				   XmNmenuHistory, 
				   widget_ids[PR_MONO_DEVICE_TOGGLE]);
		    Set_Attribute (widget_ids[PR_H_ALIGN_OPTION_MENU],
				   XmNmenuHistory, 
				   widget_ids[PR_ALIGN_LEFT_TOGGLE]);
		    Set_Attribute (widget_ids[PR_V_ALIGN_OPTION_MENU],
				   XmNmenuHistory, 
				   widget_ids[PR_ALIGN_TOP_TOGGLE]);

		    break;
	    }
/* Set option menus to be sensitive or insensitive as necessary */
	    Set_Sensitive_Option_Menus (pr_out_format_new);
/* change the printer queues */
	    Get_Default_Pr_Filename_FP (pr_out_format_new, pr_filename_new);

	    tmp_xmstr = DXmCvtOStoCS (pr_filename_new, &bc, &status);
	    DXmCSTextSetString ((DXmCSTextWidget)widget_ids[PR_FILENAME_TEXT], tmp_xmstr);
	    XmStringFree (tmp_xmstr);

	    Set_Output_Format (pr_out_format_new);
	}
}


void Update_Print_Filename ()
{
    XmString tmp_xmstr;
    long bc, status;

    if (print_dialog) {
	Get_Default_Pr_Filename_FP (pr_out_format, pr_filename);
	if (XtIsManaged (print_dialog)) {
	    Get_Default_Pr_Filename_FP (pr_out_format_new, pr_filename_new);
	    tmp_xmstr = DXmCvtOStoCS (pr_filename_new, &bc, &status);
	}
	else {
	    tmp_xmstr = DXmCvtOStoCS (pr_filename, &bc, &status);
	}
	DXmCSTextSetString ((DXmCSTextWidget)widget_ids[PR_FILENAME_TEXT], tmp_xmstr);
	XmStringFree (tmp_xmstr);
    }
}

void Reset_Print_Defaults ()
{
    XmString tmp_xmstr;
    long bc, status;
    Widget tmp;

    Set_Attribute (widget_ids[PR_FORMAT_OPTION_MENU], XmNmenuHistory,
		   pr_format_default);    
    Set_Attribute (widget_ids[PR_DEVICE_OPTION_MENU], XmNmenuHistory,
		   pr_device_default);    
    Set_Attribute (widget_ids[PR_AR_OPTION_MENU], XmNmenuHistory,
		   pr_ar_default);    
    Set_Attribute (widget_ids[PR_H_ALIGN_OPTION_MENU], XmNmenuHistory,
		   pr_h_align_default);    
    Set_Attribute (widget_ids[PR_V_ALIGN_OPTION_MENU], XmNmenuHistory,
		   pr_v_align_default);    

/* Set option menus to be sensitive or insensitive as necessary */
    Set_Sensitive_Option_Menus (pr_out_format);

    Get_Attribute (widget_ids[PR_SEND_TO_RADIO_BOX], XmNmenuHistory, &tmp);
    Set_Attribute (tmp, XmNset, FALSE);
    Set_Attribute (pr_send_to_default, XmNset, TRUE);
    Set_Attribute (widget_ids[PR_SEND_TO_RADIO_BOX], XmNmenuHistory,
		   pr_send_to_default);    

    if (pr_out_format != pr_out_format_new) {
	Set_Output_Format (pr_out_format);
    }

    Change_Printer (pr_printer_str);
    Set_Printer_Choice (pr_printer_str);
/* reset the filename */
    tmp_xmstr = DXmCvtOStoCS (pr_filename, &bc, &status);
    DXmCSTextSetString ((DXmCSTextWidget)widget_ids[PR_FILENAME_TEXT], tmp_xmstr);
    XmStringFree (tmp_xmstr);

    strcpy (pr_filename_new, pr_filename);
    pr_out_format_new = pr_out_format;
    pr_send_to_new = pr_send_to;
}

void Set_Print_Defaults ()
{
    Get_Attribute (widget_ids[PR_FORMAT_OPTION_MENU], XmNmenuHistory,
		   &pr_format_default);    
    Get_Attribute (widget_ids[PR_DEVICE_OPTION_MENU], XmNmenuHistory,
		   &pr_device_default);    
    Get_Attribute (widget_ids[PR_AR_OPTION_MENU], XmNmenuHistory,
		   &pr_ar_default);    
    Get_Attribute (widget_ids[PR_H_ALIGN_OPTION_MENU], XmNmenuHistory,
		   &pr_h_align_default);    
    Get_Attribute (widget_ids[PR_V_ALIGN_OPTION_MENU], XmNmenuHistory,
		   &pr_v_align_default);    
    Get_Attribute (widget_ids[PR_SEND_TO_RADIO_BOX], XmNmenuHistory,
		   &pr_send_to_default);
}


void Update_Print_Defaults ()
{
    char *tmp_str;
    XmString tmp_xmstr;
    long bc, status;

    pr_out_format = pr_out_format_new;
    pr_out_dev = pr_out_dev_new;
    pr_aspect_ratio = pr_aspect_ratio_new;
    pr_h_alignment = pr_h_alignment_new;
    pr_v_alignment = pr_v_alignment_new;
    pr_send_to = pr_send_to_new;
/* set the invisible delete button */
    if (pr_send_to == PR_SEND_TO_PRINTER)
	Set_Attribute (print_2_dialog, DXmNdeleteFileChoice, TRUE);
    else 
	Set_Attribute (print_2_dialog, DXmNdeleteFileChoice, FALSE);


    tmp_xmstr = DXmCSTextGetString (widget_ids[PR_FILENAME_TEXT]);
    if (tmp_xmstr)
    {
	tmp_str = (char *)DXmCvtCStoOS (tmp_xmstr, &bc, &status);
	XmStringFree (tmp_xmstr);
#ifdef I18N_BUG_FIX
	if (tmp_str) {
#endif /* I18N_BUG_FIX */
	strcpy (pr_filename, tmp_str);
	XtFree (tmp_str);
#ifdef I18N_BUG_FIX
	} else
	    strcpy (pr_filename, "");
#endif /* I18N_BUG_FIX */
    }
    else {
	strcpy (pr_filename, "");
    }

/*    strcpy (pr_filename, pr_filename_new);	*/

    if (pr_printer_str)
	XmStringFree (pr_printer_str);
    pr_printer_str = XmStringCopy(pr_printer_str_new);

    Change_Print_Format (pr_out_format);
    Change_Printer (pr_printer_str);
    Set_Print_Defaults ();
}

int Print_File (); /* jj-port */

void Print_2_File_Callback (w, button_id, d)
    Widget w;					/* pulldown menu */
    int *button_id;
    char *d;		/* data */
{
    XmString cs;
    
    if (Finished_Action()) {
	switch (*button_id) {
	    case PRINT_2_OK_ID :
		Get_Attribute (print_2_dialog, DXmNprinterChoice, &cs);
		Set_Printer_Choice (cs);
/*
		XmStringFree (cs);
*/
		break;
	    case PRINT_2_CANCEL_ID :
		break;
	}
	XtUnmanageChild (print_2_dialog);
    }
}


/* Create the print dialog */
void Manage_Print_2_Dialog ()
{
    XtManageChild (print_2_dialog);
}


void Create_Print_2_Dialog ()
{
    if (!print_2_dialog) {
        if (Fetch_Widget ("print_2_dialog_box", main_widget,
		          &print_2_dialog) != MrmSUCCESS)
	    DRM_Error ("can't fetch print dialog");
    }
}


void Print_File_Callback (w, button_id, d)
    Widget w;					/* pulldown menu */
    int *button_id;
    char *d;		/* data */
{
    int status;
    
    if (Finished_Action()) {
	status == K_SUCCESS;
	switch (*button_id) {
	    case PRINT_OK_ID :
		Update_Print_Defaults ();
		if ((status = Print_File ()) == K_SUCCESS) {
		    XtUnmanageChild (print_dialog);
		    XtUnmanageChild (print_2_dialog);
		}
 		break;
	    case PRINT_APPLY_ID :
		Update_Print_Defaults ();
		status = Print_File ();
		break;
	    case PRINT_OPTIONS_ID :
		Manage_Print_2_Dialog ();
		break;
	    case PRINT_CANCEL_ID :
		XtUnmanageChild (print_dialog);
		XtUnmanageChild (print_2_dialog);
		Reset_Print_Defaults ();
 		break;
	}
/* print any error message */
	switch (status) {
	    case K_FAILURE :
		Display_Message ("T_NO_MEM_FOR_PRINT");
		break;
	    case K_PICTURE_NOT_BW :
		Display_Message ("T_NO_PRINT_COLOR_SIXELS");
		break;
	    case K_CANNOT_OPEN :
		Display_Message ("T_CANNOT_OPEN_PRINT_FILE");
		break;
	}
    }
}


void Create_Print_Dialog()
{
    long bc, status;
    XmString tmp_xmstr;
    XmString cs;
    int ht1, ht2, y = 0, yoff;
    int wd1, wd2, sp;

    Set_Cursor_Watch (pwindow);
    if (!print_dialog) {
/* set print defaults */
	pr_out_format = pr_out_format_new = PR_PS_FORMAT;
	pr_out_dev = pr_out_dev_new = PR_COLOR_DEVICE;
	pr_aspect_ratio = pr_aspect_ratio_new = ASPECT_1_TO_1_ID;
	pr_h_alignment = pr_h_alignment_new = JUSTIFY_LEFT;
	pr_v_alignment = pr_v_alignment_new = JUSTIFY_TOP;
	pr_send_to = pr_send_to_new = PR_SEND_TO_PRINTER;
	Get_Default_Pr_Filename_FP (pr_out_format, pr_filename);
	strcpy (pr_filename_new, pr_filename);

        if (Fetch_Widget ("print_dialog_box", main_widget,
		          &print_dialog) != MrmSUCCESS)
	    DRM_Error ("can't fetch print dialog");

	Create_Print_2_Dialog ();
/* set proper fields in print dialog */
	Get_Attribute (print_2_dialog, DXmNprinterChoice, &cs);
	pr_printer_str = XmStringCopy (cs);
	pr_printer_str_new = XmStringCopy (cs);

	printer_count = 1;
	printer_toggles = (Widget *) XtCalloc (printer_count, sizeof (Widget));
	printer_toggles[0] = widget_ids[PR_DUMMY_TOGGLE];

	Change_Printer_Toggles ();	
	pr_send_to = pr_send_to_new;
	Set_Print_Defaults ();

	tmp_xmstr = DXmCvtOStoCS (pr_filename, &bc, &status);
	DXmCSTextSetString ((DXmCSTextWidget)widget_ids[PR_FILENAME_TEXT], tmp_xmstr);
	XmStringFree (tmp_xmstr);

/* grey out the aspect ratio option menu */
	XtSetSensitive (widget_ids[PR_AR_OPTION_MENU], INSENSITIVE);

/* add tab groups */
	XmAddTabGroup (widget_ids[PR_FILENAME_TEXT]);
	XmAddTabGroup (widget_ids[PR_FORMAT_OPTION_MENU]);
	XmAddTabGroup (widget_ids[PR_DEVICE_OPTION_MENU]);
	XmAddTabGroup (widget_ids[PR_H_ALIGN_OPTION_MENU]);
	XmAddTabGroup (widget_ids[PR_V_ALIGN_OPTION_MENU]);
	XmAddTabGroup (widget_ids[PR_AR_OPTION_MENU]);

	XmAddTabGroup (widget_ids[PR_SEND_TO_RADIO_BOX]);
	XmAddTabGroup (widget_ids[PR_PRINTERS_OPTION_MENU]);

	XmAddTabGroup (widget_ids[PR_BUTTONS_ROW_COLUMN]);

	XtManageChild (print_dialog);

/* Center the labels with respect to the option menus */
	Get_Attribute (widget_ids[PR_FORM_1], XmNverticalSpacing, &y);
	ht1 = XtHeight (widget_ids[PR_FORMAT_LABEL]);
	ht2 = XtHeight (widget_ids[PR_FORMAT_OPTION_MENU]);
	yoff = y + ((ht2 - ht1) / 2);
	Set_Attribute (widget_ids[PR_FORMAT_LABEL], XmNtopOffset, yoff);
	yoff = y + (ht2 - ht1);
	Set_Attribute (widget_ids[PR_DEVICE_LABEL], XmNtopOffset, yoff);
	Set_Attribute (widget_ids[PR_H_ALIGN_LABEL], XmNtopOffset, yoff);
	Set_Attribute (widget_ids[PR_V_ALIGN_LABEL], XmNtopOffset, yoff);
	Set_Attribute (widget_ids[PR_AR_LABEL], XmNtopOffset, yoff);

/* center the push buttons */
	wd1 = XtWidth (widget_ids[PR_BUTTONS_ROW_COLUMN]);
	wd2 = XtWidth (widget_ids[PR_OK_BUTTON]);
	sp = (wd1 - (4 * wd2)) / 3;
	Set_Attribute (widget_ids[PR_BUTTONS_ROW_COLUMN], XmNspacing, sp);
    }
    else {
	XtManageChild (print_dialog);
    }
    Set_Cursor (pwindow, current_action);
}


/*
 *
 * ROUTINE:  Write_To_Temp
 *
 * ABSTRACT: 
 *                                
 *   Write the output stream to the temp file
 *
 */
long Write_To_Temp( bufptr, buflen, usrparam )
char *bufptr;
int buflen;
int usrparam;
{
int st;

	st = fwrite( bufptr, buflen, 1, fid );
#if defined(VMS)
	fprintf (fid, "\n"); /* dont let the records get too bif */
#endif 
	return(1);
}


/*
 *
 * ROUTINE:  Write_Sixels
 *
 * ABSTRACT: 
 *                                
 *   Write out a sixel file to print
 *
 */
int Write_Sixels (fid, ximage)
    FILE *fid;
    XImage *ximage;
{
    unsigned int bytcnt;
    unsigned long image_id;
    unsigned long new_id;
    int set_index;
    int i;
    struct PUT_ITMLST set_attributes[7];
    char *image_data;
    int context;
    int image_size;
    int status;
    float xscale, yscale;
    float	pagelen = 11;
    float	pagewid = 8.5;
    float	img_ht, img_wd;
    unsigned char LineFeedCharCode = 10;
    unsigned char SpaceCharCode = 32;

    unsigned char *Convert_Image_Data_Cmap_To_BW();
    
    static unsigned char ansi_head[] =  /* jj-port */
	{
		155,  50, 119, 155, 51, 122, 
	/*  	CSI   2    w    CSI  3  z    	*/	
	/*	DECSHORP 	DECVERP		*/
	};

		/* 	DCS   0   ;   0  ;   9   q	*/
		/*sixelhead[5] is the grid size in decipoints; 9 dp = 80 DPI */
		/*	"   1   ;   1   ;   0   ;   0	*/
/*
unsigned char sixel_head[] =
		{	144, 48, 59, 48, 59, 57, 113,
			34, 49, 59, 49, 59, 48, 59, 48};
*/

	/*	CSI!p * reset the printer */
	/*      CSI7 I * set size unit to pixels */
	/* 	DCS 9;;4q"1;1; *sets pixel size of 4*(1/300) */

/*
unsigned char sixel_head[] ={
	155, 33, 112,
	155, 55, 32, 73,
	144, 57, 59, 59, 52, 113, 34, 49, 59, 49, 59 };
*/

/*	CSI!p * reset the printer 
	ESC[1r * !Set margin to top
	ESC[d * Move to top
	ESCP9q * start sixels
*/
static unsigned char sixel_head[] = {  /* jj-port */
	155, 33, 112,
	27, 91, 49, 114,
	27, 91, 100,
	27, 80, 57, 113 };

static unsigned char sixel_tail[] = {  /* jj-port */
	/* ESC\ *end sixel stream*/
	/* CSI2 I *set back to decipoints*/
		27, 92,
		155, 50, 32, 73 };


/* if depth is greater than 1, convert to depth 1 */
	if (ximage->depth > 1) {
	    image_data = 
		(char *) Convert_Image_Data_Cmap_To_BW (ximage->data,
							ximage->width,
							ximage->height,
							ximage->bytes_per_line,
							&bytes, 4, &status);
	    if (status != K_SUCCESS) {
		return (status);
	    }
	}	
	else {
	    image_data = ximage->data;
	}

/* Create frame using ISL */
	bits_per_pixel = 1;
	pixel_order = ImgK_StandardPixelOrder; 

	pixels_per_scanline = ximage-> width;
	scanline_count = ximage -> height;
	scanline_stride = 8 * bytes;	/* 8 bits per byte */
	image_size = bytes * scanline_count;

	start_set_itemlist (set_attributes, set_index);
	put_set_item (set_attributes, set_index, Img_PixelOrder, pixel_order);
	put_set_item (set_attributes, set_index, Img_BitsPerPixel, bits_per_pixel);
	put_set_item (set_attributes, set_index, Img_PixelsPerLine, pixels_per_scanline);
	put_set_item (set_attributes, set_index, Img_NumberOfLines, scanline_count);
	put_set_item (set_attributes, set_index, Img_ScanlineStride, scanline_stride);
	end_set_itemlist (set_attributes, set_index);

	image_id = ImgCreateFrame(set_attributes, ImgK_StypeBitonal);

	ImgImportBitmap(image_id, image_data, image_size, 0, 0, 0, 0);
	
/* Scale if necessary for LA50 */
	if (pr_aspect_ratio == ASPECT_2_TO_1_ID) {
	    xscale = 2.0;
	    yscale = 1.0;
	    new_id = ImgScale (image_id, &xscale, &yscale, 0, 0, 0);
	    ImgDeleteFrame (image_id);
	    image_id = new_id;
	}
		
	outbuf = (char *) XtMalloc( OUTBUF_LENGTH ); /* jj-port */

/* Write the header */
	fwrite( ansi_head, sizeof(ansi_head), 1, fid );

/* spaces and linefeeds to center on the page
	for (i=0; i< numLFs; i++)
		fwrite( &LineFeedCharCode, sizeof(LineFeedCharCode), 1, fid );
	for (i=0; i< numSPs; i++)
		fwrite( &SpaceCharCode, sizeof(SpaceCharCode), 1, fid );
*/	     
	fwrite (sixel_head, sizeof(sixel_head), 1, fid);

	ImgExportSixels (image_id, NULL, (unsigned char *)outbuf, OUTBUF_LENGTH,
			 &bytcnt, NULL, Write_To_Temp, NULL);

	/*
	 * write trailer
	 */
	fwrite (sixel_tail, sizeof(sixel_tail), 1, fid);
	XtFree (outbuf);
	ImgDeleteFrame (image_id);
/* If new image data was created, free it. */
	if (image_data != ximage->data)
	{
	    XtFree (image_data);
	    image_data = NULL;
	}

	return (K_SUCCESS);
}

/*
 *	swapbits( buf, n) 
 *		reverse bits of n bytes starting at buf
 *	negbits( buf, n)
 *		negate bits of n bytes starting at buf
 *	swapshort( buf, n )
 *		swap bytes, two at a time for n bytes starting at buf
 * 	swapthree( buf, n )
 *		swap bytes, three at a time for n bytes starting at buf
 *	swaplong( buf, n)
 *		swap bytes, four at at time for n bytes starting at buf
 *
 *		buf	- pointer to buffer which contains bytes to be 
 *			modified
 *		n 	- integer value of number of bytes to be modified
 *
 *
 *	other:
 *		no restrictions, implicit parameters, etc.
 *		
 */

unsigned char reverse_byte[0x100] = {
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
	0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
	0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
	0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
	0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
	0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
	0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
	0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
	0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
	0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
	0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
	0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
	0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
	0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
	0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
	0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};
unsigned char negate_byte[0x100] = {
	0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8,
	0xf7, 0xf6, 0xf5, 0xf4, 0xf3, 0xf2, 0xf1, 0xf0,
	0xef, 0xee, 0xed, 0xec, 0xeb, 0xea, 0xe9, 0xe8,
	0xe7, 0xe6, 0xe5, 0xe4, 0xe3, 0xe2, 0xe1, 0xe0,
	0xdf, 0xde, 0xdd, 0xdc, 0xdb, 0xd0, 0xd9, 0xd8,
	0xd7, 0xd6, 0xd5, 0xd4, 0xd3, 0xd2, 0xd1, 0xd0,
	0xcf, 0xce, 0xcd, 0xcc, 0xcb, 0xca, 0xc9, 0xc8,
	0xc7, 0xc6, 0xc5, 0xc4, 0xc3, 0xc2, 0xc1, 0xc0,
	0xbf, 0xbe, 0xbd, 0xbc, 0xbb, 0xba, 0xb9, 0xb8,
	0xb7, 0xb6, 0xb5, 0xb4, 0xb3, 0xb2, 0xb1, 0xb0,
	0xaf, 0xae, 0xad, 0xac, 0xab, 0xaa, 0xa9, 0xa8,
	0xa7, 0xa6, 0xa5, 0xa4, 0xa3, 0xa2, 0xa1, 0xa0,
	0x9f, 0x9e, 0x9d, 0x9c, 0x9b, 0x9a, 0x99, 0x98,
	0x97, 0x96, 0x95, 0x94, 0x93, 0x92, 0x91, 0x90,
	0x8f, 0x8e, 0x8d, 0x8c, 0x8b, 0x8a, 0x89, 0x88,
	0x87, 0x86, 0x85, 0x84, 0x83, 0x82, 0x81, 0x80,
	0x7f, 0x7e, 0x7d, 0x7c, 0x7b, 0x7a, 0x79, 0x78,
	0x77, 0x76, 0x75, 0x74, 0x73, 0x72, 0x71, 0x70,
	0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x69, 0x68,
	0x67, 0x66, 0x65, 0x64, 0x63, 0x62, 0x61, 0x60,
	0x5f, 0x5e, 0x5d, 0x5c, 0x5b, 0x5a, 0x59, 0x58,
	0x57, 0x56, 0x55, 0x54, 0x53, 0x52, 0x51, 0x50,
	0x4f, 0x4e, 0x4d, 0x4c, 0x4b, 0x4a, 0x49, 0x48,
	0x47, 0x46, 0x45, 0x44, 0x43, 0x42, 0x41, 0x40,
	0x3f, 0x3e, 0x3d, 0x3c, 0x3b, 0x3a, 0x39, 0x38,
	0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30,
	0x2f, 0x2e, 0x2d, 0x2c, 0x2b, 0x2a, 0x29, 0x28,
	0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20,
	0x1f, 0x1e, 0x1d, 0x1c, 0x1b, 0x1a, 0x19, 0x18,
	0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10,
	0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
	0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00
};

/* 
 * swap the bits of n bytes 
 */
swapbits (b, n)
	register unsigned char *b;
	register int n;
{
	do {
		*b = reverse_byte[*b];
		b++;
	    } while (--n);
	
}

/* 
 * negate the bits of n bytes 
 */
negbits (b, n)
	register unsigned char *b;
	register int n;
{
	do {
		*b = negate_byte[*b];
		b++;
	    } while (--n);
	
}

/* 
 * swap two bytes at a time 
 */
swapshort (bp, n)
     register char *bp;
     register int n;
{
	register char c;
	register char *ep = bp + n;
	do {
		c = *bp;
		*bp = *(bp + 1);
		bp++;
		*bp = c;
		bp++;
	}
	while (bp < ep);
}

/* 
 * swap four bytes at a time 
 */
swaplong (bp, n)
     register char *bp;
     register int n;
{
	register char c;
	register char *ep = bp + n;
	register char *sp;
	do {
	  	sp = bp + 3;
		c = *sp;
		*sp = *bp;
		*bp = c;
		bp++;
		sp = bp + 1;
		c = *sp;
		*sp = *bp;
		*bp = c;
		bp++;
		bp += 2;
	}
	while (bp < ep);
}

/* 
 * swap three bytes at a time 
 */
swapthree (bp, n)
     register char *bp;
     register int n;
{
	register char c;
	register char *ep = bp + n;
	do {
	  	c = *bp;
		*(bp + 2) = *bp;
		*bp = c;
		bp += 3;
	}
	while (bp < ep);
}
/*
 *
 * ROUTINE:  Write_PS
 *
 * ABSTRACT: 
 *                                
 *   Write PostScript into the given file
 *		file 	- (WO) (*FILE) opened output file
 *		filename - (W0) ascii filename
 *		ximage 	- (RO) (*Ximage) image to be converted
 *	requires:
 *		swap routines - negbits, invertbits, swaplong, swapshort
 *
 */
int Write_PS (file, ximage)
    FILE	*file;
    XImage	*ximage;
{
    unsigned char	*rbuf;	/* buffer to hold row of image		*/
    unsigned char	*brbuf;	/* always points to beginning of rbuf	*/
    unsigned char 	*dst;   /* temporary pointer */
    unsigned char	*src;	/* temporary pointer */
    unsigned short	*sbuf;	/* buffer for output ascii hex data	*/
    unsigned short	*bsbuf;	/* always points to beginning of sbuf	*/

    int	i;
    int		row,column;	/* image row and column indices		*/
    char	*rowptr;	/* pointer to a row of image data 	*/
    char	*ctemp;		/* temporary string pointer	 	*/
    int		itemp;		/* temporary integer			*/
    int		xsca, ysca;	/* x and y scale factors		*/
    float	xs, ys;	
    int		xpos, ypos;	/* coordinates of image origin		*/
    int		rotate;		/* true when image should be rotated	*/
    struct  tm	*timest;
    time_t	time_val;
    static char *month[12]= {"JAN","FEB", "MAR", "APR", "MAY", "JUN", "JUL",
			     "AUG","SEP","OCT","NOV","DEC"};

    begcstream = 0;
    convert_to_gray = FALSE;

    if ((visual_info->class == StaticGray) ||( num_colors <= 2)) {
	image_data_format = MONO_1;
	endcstream = 0;
	image_is_color = 0;
	bitspercomp = 1;
    }
    else if ((visual_info->class == GrayScale) || 
	     (pr_out_dev == PR_MONO_DEVICE)) {
	if (visual_info->class != GrayScale)
	    convert_to_gray = TRUE;
	image_data_format = MONO_8;
	endcstream = 0;
	image_is_color = 0;
	bitspercomp = 8;
    }
    else {
	image_data_format = RGB_24;
	endcstream = 2;
	image_is_color = 1;
	bitspercomp = 8;
    }

    /* 
     * Find amounts to scale, translate and rotate image for best 
     * fit on 8.5 x 11 piece of paper
     */
    xs = (ximage->width * 72) / resolution;
    ys = (ximage->height * 72) / resolution;
    xsca = xs + 0.5;
    ysca = ys + 0.5;

    switch (porientation) {
	case DEFAULT_ORIENTATION :
	    if (xsca > UPSPAGEWID) {
		if (ximage->width > ximage->height)
		    rotate = TRUE;
		else
		    rotate = FALSE;
	    }
	    else
		rotate = FALSE;
	    break;
	case PORTRAIT_ORIENTATION :
	    rotate = FALSE;
	    break;
	case LANDSCAPE_ORIENTATION :
	    rotate = TRUE;
	    break;
    }

    if (!rotate) {
	switch (fit_on_page) {
	    case SCALE_WHOLE :
		if (xsca > UPSPAGEWID) {
		    ysca = ((ys * UPSPAGEWID) / xs) + 0.5;
		    xsca = UPSPAGEWID;
		}
		if (ysca > UPSPAGELEN) {
		    xsca = ((xs * UPSPAGELEN) / ys) + 0.5;
		    ysca = UPSPAGELEN;
		}
 		break;
	    case SCALE_BOTH :
	    case SCALE_HORIZONTAL :
		if (xsca > UPSPAGEWID)
		    xsca = UPSPAGEWID;
		if (fit_on_page != SCALE_BOTH)
		    break;
	    case SCALE_VERTICAL :
		if (ysca > UPSPAGELEN)
		    ysca = UPSPAGELEN;
		break;
	    case CLIP_BOTH :
		break;
	}

	switch (pr_h_alignment) {
	    case JUSTIFY_LEFT :
		xpos = 18;
		break;
	    case JUSTIFY_RIGHT :
		xpos = PSPAGEWID - xsca - 18;
		break;
	    case CENTER_HORIZONTAL :
		xpos = (PSPAGEWID - xsca) / 2;
		break;
	}

	switch (pr_v_alignment) {
	    case JUSTIFY_TOP :
		ypos = PSPAGELEN - ysca - 18;
		break;
	    case JUSTIFY_BOTTOM :
		ypos =  18;
		break;
	    case CENTER_VERTICAL :
		ypos = (PSPAGELEN - ysca) / 2;
		break;
	}
    }

    else {					    /* if (rotate) */
	switch (fit_on_page) {
	    case SCALE_WHOLE :
		if (ysca > UPSPAGEWID) {
		    xsca = ((xs * UPSPAGEWID) / ys) + 0.5;
		    ysca = UPSPAGEWID;
		}
		if (xsca > UPSPAGELEN) {
		    ysca = ((ys * UPSPAGELEN) / xs) + 0.5;
		    xsca = UPSPAGELEN;
		}
 		break;
	    case SCALE_BOTH :
	    case SCALE_HORIZONTAL :
		if (xsca > UPSPAGELEN)
		    xsca = UPSPAGELEN;
		if (fit_on_page != SCALE_BOTH)
		    break;
	    case SCALE_VERTICAL :
		if (ysca > UPSPAGEWID)
		    ysca = UPSPAGEWID;
		break;
	    case CLIP_BOTH :
		break;
	}

	switch (pr_h_alignment) {
            case JUSTIFY_LEFT :
                ypos = 18;
                break;
            case JUSTIFY_RIGHT :
                ypos = PSPAGELEN - xsca - 18;
                break;
            case CENTER_HORIZONTAL :
                ypos = (PSPAGELEN - xsca) / 2;
                break;
	}

	switch (pr_v_alignment) {
            case JUSTIFY_TOP :
                xpos = 18 + ysca;
                break;
            case JUSTIFY_BOTTOM :
                xpos = PSPAGEWID - 18;
                break;
            case CENTER_VERTICAL :
                xpos = (PSPAGEWID + ysca) / 2;
                break;
	}
    }


    /* 
     * Header as described in Encapulated Postscript File specification
     */
    time(&time_val);
    timest = localtime(&time_val);

    fprintf(file, "%%!PS-Adobe-2.0 EPSF-1.2\n");
    fprintf(file, "%%%%BoundingBox:");
    if (!rotate)
	fprintf(file, "%d %d %d %d\n", xpos, ypos, xpos + xsca, ypos + ysca);
    else
	fprintf(file, "%d %d %d %d\n", xpos- ysca, ypos, xpos, ypos + xsca);

    fprintf(file, "%%%%Creator: DECW$PAINT \n");
/*
    fprintf(file, "%%%%Title: %s\n", filename);
*/
    fprintf(file, "%%%%CreationDate: %d-%s-19%d  \n",
	    timest->tm_mday,month[timest->tm_mon], timest->tm_year);
    fprintf(file, "%%%%Pages: 1 \n");
    fprintf(file, "%%%%EndComments \n");
    fprintf(file, "%%%%EndProlog \n");	
    fprintf(file, "%%%%Page: 1 1 \n");
	

    /*
     * creat dictionary and store definitions
     */
    fprintf(file, "55 dict begin\n");
    fprintf(file, "/savobj save def\n");
    fprintf(file, "/picstr %d string def\n", ROUNDUP(ximage->width,BITSINBYTE));

    /* 
     * Define clipping rectangle (1/4 inch margins on every side)
     */
    fprintf(file, "newpath\n");
    fprintf(file, "%d %d moveto\n%d %d lineto\n%d %d lineto\n%d %d lineto\n",
	    18, 18, PSPAGEWID - 18, 18, PSPAGEWID - 18, PSPAGELEN - 18, 18,
	    PSPAGELEN - 18);
    fprintf(file, "closepath\nclip\n");

    /*
     * Write out scaling and rotating comands
     */
    fprintf(file,"%d %d translate\n",xpos, ypos);
    if (rotate)
	fprintf(file, "90 rotate\n");
    fprintf(file, "%d %d scale\n", xsca, ysca );

    /*
     * use basic Postscript print image algorithm, assume
     * image is scanned left-to-right, top-to-bottom
     */

    PsiProlog();


    fprintf(file, "%d %d %d [%d 0 0 -%d 0 %d]\n",
	    ximage->width, ximage->height,
	    (image_data_format == MONO_1) ? 1 : 8,
	    ximage->width, ximage->height, ximage->height);
    if (image_is_color) {
	fprintf(file, "{I} {I} {I} true 3 colorimage\n");
    }
    else {
	fprintf(file, "{I} image\n");
    }


  if (TRUE) {
    PsiReceive (ximage);
  }
  else {
    /*
     * process data row by row, making appropriate conversions
     * if bytes or bits need to be reversed 
     */
    rbuf = (unsigned char*)XtMalloc( ximage->bytes_per_line );
    sbuf = (unsigned short*)XtMalloc
		( 2 * ROUNDUP( ximage->width, BITSINBYTE ) + 2);
    bsbuf = sbuf;
    brbuf = rbuf;
    rowptr = ximage->data;
    for (row = 0; row < ximage->height; row++,
	 rowptr += ximage->bytes_per_line ) {
	src = (unsigned char *) rowptr; /* jj-port */
	dst = brbuf;
	for( i = 0; i < ximage->bytes_per_line; ++i )
	    *dst++ = *src++;

	if( ximage->byte_order != LSBFirst ) {
	    switch( ximage->bitmap_unit ) {
		case 8: 
		    break;
		case 16: 
		    swapshort( brbuf, ximage->bytes_per_line);
		    break;
		default:
		    swaplong( brbuf, ximage->bytes_per_line );
	    }
	}

	if( ximage->bitmap_bit_order != MSBFirst ) {
	    swapbits( brbuf, ximage->bytes_per_line );
	}
		
/* reverse image */
	negbits( brbuf, ximage->bytes_per_line );

	for (column = 0, rbuf = brbuf, sbuf = bsbuf;
	     column < ROUNDUP( ximage->width, BITSINBYTE ); 
	     rbuf++, sbuf++, column++) {
	    *sbuf = ps[*rbuf];
	}
	sprintf( (char *)sbuf, "\n" );
	fwrite( bsbuf, ( 2 * ROUNDUP( ximage->width, BITSINBYTE ) + 1),
	    	1, file );
    }

    /* 
     * write final postscript command 
     */
	 
    XtFree((char *)brbuf);
    XtFree((char *)bsbuf);
  }


    fprintf(file, "savobj restore\n");
    fprintf(file, "end\n");
    fprintf(file, "showpage\n");
    fprintf(file, "%%%%Trailer \n");	

    return (K_SUCCESS);
}


/*
 *
 * ROUTINE:  Print_File
 *
 * ABSTRACT: 
 *                                
 *   Print file callback
 *
 */
int Print_File ()
{
    int i;
    int st;
    int dummy;
    float scale_value;
    char *scale_str;
    char tmp[256], tmp2[256];
    long bc, stat;
    int status;
    FILE *fid;

    if (print_dialog != 0) {
        if (XtIsManaged (print_dialog)) {
	    Set_Cursor_Watch (XtWindow (print_dialog));
	}
    }
    Set_Cursor_Watch (pwindow);
    Get_Attribute (print_2_dialog, DXmNorientationChoice, &porientation);

/*
    if ((picture_image->width != pimage_wd) ||
	(picture_image->height != pimage_ht) ||
	(undo_action == CROP)) {
 * there has been a crop and image in memory is larger than picture *
	ximage = (XImage *) New_Image ();  * jj-port *
    }
    else {
*/

    if (entering_text)
	End_Text ();
    if (paint_view == NORMAL_VIEW) {
	Update_Image (0, 0, picture_wd, picture_ht, picture_x, picture_y,
		     picture, ImgK_Src);
    }

/*
    if (ximage == 0) {
	Display_Message ("NO_MEM_FOR_PRINT");
    }
    else {
*/

    fid = fopen (pr_filename, "w" );

/* if could not open the print file and the user doesn't care about saving */
/* the file, just open a file in the default directory. */
    if ((fid == NULL) && (pr_send_to == PR_SEND_TO_PRINTER)) {
	Get_Default_Pr_Filename (pr_out_format, tmp2);
	strcpy (tmp, scratch_directory);
	i = strlen(tmp);
	strncpy (&tmp[i], tmp2, strlen(tmp2));
        i += strlen(tmp2);
	tmp[i] = '\0';
	fid = fopen (tmp, "w");
    }
    else {
	strcpy (tmp, pr_filename);
    }

    if (fid == NULL) {
	status = K_CANNOT_OPEN;
    }
    else {
	psfile = fid;
	if (pr_out_format == PR_PS_FORMAT)
	    status = Write_PS (fid, picture_image);
	else
	    if (pr_out_format == PR_SIXEL_FORMAT)
		status = Write_Sixels (fid, picture_image);
	fclose (fid);
/* If sending to printer */
	if (status == K_SUCCESS) {
	    if ((pr_send_to == PR_SEND_TO_PRINTER) ||
		(pr_send_to == PR_SEND_TO_BOTH)) {
		file_names[0] = DXmCvtOStoCS (tmp, &bc, &stat);
/*		file_names[0] = XmStringLtoRCreate(tmp , "ISO8859-1"); */
		st = DXmPrintWgtPrintJob(print_2_dialog, file_names, 1); /* jj-port */
		XmStringFree (file_names[0]);
	    }
	}
	else {
/* delete the file */
#if !defined(VMS)
	    unlink (tmp);
#else
	    delete (tmp);
#endif
	}
    }

/*
    if (ximage != picture_image)
    {
	XtFree (ximage->data);
	XDestroyImage (ximage);
	ximage = NULL;
    }
*/

    if (print_dialog != 0)
	XUndefineCursor (disp, XtWindow (print_dialog));
    Set_Cursor (pwindow, current_action);
    return (status);
}
#endif
