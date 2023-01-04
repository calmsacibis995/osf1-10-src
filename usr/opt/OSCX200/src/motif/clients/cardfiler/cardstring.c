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
 * Changed quotes to brackets for include of dwi18n_lib.h
 */
#if defined(OSF1) || defined(__osf__)
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
#ifndef lint				
static char *BuildSystemHeader = 
  "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/cardfiler/cardstring.c,v 1.1.2.3 92/12/11 08:30:29 devrcs Exp $";
#endif
#endif
/*  DEC/CMS REPLACEMENT HISTORY, Element CARDSTRING.C */
/*  *49   30-JAN-1992 14:41:54 PONTE "Add I18N Library" */
/*  *48   13-NOV-1991 11:48:08 PONTE "Merge Asian I18N and Performance Chnages" */
/*  *47   21-OCT-1991 09:24:41 PONTE "Problem with certain files being reserved" */
/*  *46   18-OCT-1991 15:22:37 PONTE "Change strncpy to memcpy for performance issues" */
/*  *45   15-MAY-1991 17:04:10 POON "Checkin from notes request # 746,747,748" */
/*  *44   10-JAN-1991 17:06:56 POON "Check in EFT changes." */
/*  *43   30-NOV-1990 13:21:42 POON "made style guide changes and help submenu change." */
/*  *42   26-JUL-1990 15:12:23 POON "Post BL5 checkin, with Icon supportted vlist and Memex BL5 conversion" */
/*  *41   29-JUN-1990 14:44:05 POON "Post bl4 checkins, with memex support in vlist." */
/*  *40   22-JUN-1990 16:20:40 POON "bl4 checkin" */
/*  *39   15-JUN-1990 09:53:52 POON "Added Change XmStringByteCompare if statement." */
/*  *38   21-MAY-1990 15:13:02 POON "Change the order of externdef.h, and globaldefs.h" */
/*  *37   21-MAY-1990 14:47:07 POON "Changes made." */
/*  *36   18-MAY-1990 17:45:51 POON "changes made." */
/*  *35   18-MAY-1990 15:07:50 POON "XNLS changes added" */
/*  *34    9-MAY-1990 23:21:14 POON "ISL ifdef changed." */
/*  *33   26-APR-1990 14:10:12 POON "Add back in modules with the Motif changes." */
/*  *32    6-NOV-1989 20:05:05 FINNEGAN "Hickory: Integrate in new vlist widget" */
/*  *31    2-NOV-1989 17:16:27 FINNEGAN "Hickory changes, xnls sorting" */
/*  *30   17-AUG-1989 09:49:42 FINNEGAN "BL6: Search large, pop DB's, human factors, .img, help in VLIST, new help, file filter, */
/* accelerator" */
/*  *29   25-MAY-1989 13:16:12 FINNEGAN "IFT1: pixmap, graying, undo, file select, icon name, focus, watch, lg bitmap, font name" */
/*  *28    1-MAY-1989 08:35:35 FINNEGAN "BL4: Add cut/paste images, fixup UIL files" */
/*  *27   10-APR-1989 16:18:11 FINNEGAN "Fix Help problem, fix autorepeat problem, save resources" */
/*  *26   21-MAR-1989 16:50:11 FINNEGAN "BL2 Fix tab proble, context sens help, all text in UIL" */
/*  *25   18-OCT-1988 09:46:46 FINNEGAN "BL 11.1 changes: 4000 char limit, better focus, clipboard fix, new VLIST, new help, */
/* minWidth, minHeight, autorepeat Prev/Next, fix parenting" */
/*  *24    3-OCT-1988 14:42:52 FINNEGAN "BL 10.1 changes, pcc compatability" */
/*  *23   22-AUG-1988 14:39:48 FINNEGAN "BL9.2 changes: ISl image cleanup, error messages typo's, sys$scratch" */
/*  *22   12-AUG-1988 17:00:09 FINNEGAN "BL9.1 fixes: set focus, rename problem" */
/*  *21   12-AUG-1988 14:31:07 FINNEGAN "BL9 change: fix set focus, strings in UIL" */
/*  *20   12-AUG-1988 12:41:58 FINNEGAN "BL9.1 changes: Ultrix Synch, small card, large pic., set focus" */
/*  *19   19-JUL-1988 16:22:03 FINNEGAN "Latin1 on clipboard, @Help, VList fix" */
/*  *18    1-JUL-1988 11:30:19 FINNEGAN "FT2 changes: cleanup strings, fix bugs" */
/*  *17   22-JUN-1988 15:10:27 FINNEGAN "BL8.5: New Vlist, Better prev/next, gadgets" */
/*  *16   10-JUN-1988 14:56:31 HOOVER "performance/interface changes -McEvoy for Finnegan" */
/*  *15    1-JUN-1988 15:50:57 FINNEGAN "BL8.4: copyright, new interface, ddif, vue fixes" */
/*  *14   23-MAY-1988 15:01:21 FINNEGAN "BL8.3 Fixes: text widget, display card, fewer widgets" */
/*  *13   18-MAY-1988 13:21:37 FINNEGAN "BL8.2 changes: integrate Vue List box,, plus leaks, fix card display" */
/*  *12   11-MAY-1988 16:57:16 FINNEGAN "BL8.1, fix save, add open card, new help" */
/*  *11   29-APR-1988 16:47:55 FINNEGAN "BL8 changes, set focus, help" */
/*  *10   22-APR-1988 15:27:47 FINNEGAN "FT2 bug fixes and bitmap support" */
/*  *9    12-APR-1988 15:12:54 FINNEGAN "FT2 cleanup" */
/*  *8    11-APR-1988 15:09:14 FINNEGAN "FT2 compatability changes" */
/*  *7    22-MAR-1988 10:28:49 FINNEGAN "New UIL version of the cardfiler" */
/*  *6    27-FEB-1988 19:29:59 GEORGE "Add copyright" */
/*  *5     8-JAN-1988 09:10:06 REILLY "BL6.3 changes" */
/*  *4     7-JAN-1988 14:14:06 REILLY "Replace with completely new version" */
/*  *3     6-JAN-1988 15:23:20 REILLY "More BL6.2 stuff" */
/*  *2    30-DEC-1987 10:30:11 REILLY "Update to BL6.2" */
/*  *1    25-NOV-1987 10:22:57 GEORGE "Initial Entry" */
/*  DEC/CMS REPLACEMENT HISTORY, Element CARDSTRING.C */
/*
**++

  Copyright (c) Digital Equipment Corporation, 
  1987, 1988, 1989, 1990, 1991, 1992
  All Rights Reserved.  Unpublished rights reserved
  under the copyright laws of the United States.

  The software contained on this media is proprietary
  to and embodies the confidential technology of
  Digital Equipment Corporation.  Possession, use,
  duplication or dissemination of the software and
  media is authorized only pursuant to a valid written
  license from Digital Equipment Corporation.

  RESTRICTED RIGHTS LEGEND   Use, duplication, or
  disclosure by the U.S. Government is subject to
  restrictions as set forth in Subparagraph (c)(1)(ii)
  of DFARS 252.227-7013, or in FAR 52.227-19, as
  applicable.

**--
**/

/*
**++
**  MODULE NAME:
**	cardstring.c
**
**  FACILITY:
**      OOTB Cardfiler
**
**  ABSTRACT:
**	Currently contains the string handling routines for the Cardfiler.
**
**  AUTHORS:
**      Paul Reilly
**
**  RELEASER:
**
**  CREATION DATE:     01-DEC-1987
**
**  MODIFICATION HISTORY:
**
**    Version:
**
**	Nov 1990 - V3.0 	(ASP) - Motif conversion and added memex support.
**      May 1992 - V3.1         (SP)  - I18N and maintenance changes.
**--
**/
#include "cardglobaldefs.h"
#include "cardexterndefs.h"
#include <stdio.h>
#include <string.h>

#ifdef VMS
#include "dwi18n_lib.h"
#else
#include <dwi18n_lib.h>
#endif

char *myindex();

#define GET_VALUE(base,offset,nbit) \
   (*((int *)(base + ( offset >> 3))) >> \
   (offset & 0x7) & IMG_AL_ONES_ASCENDING[nbit])

/* put value nbits long (up to 25) described by the base and bit offset */
#define PUT_VALUE(base,offset,nbit,value) \
   *((int *)(base + (offset >> 3))) ^= \
   (value & IMG_AL_ONES_ASCENDING[nbit]) << (offset & 0x7)

static int IMG_AL_ONES_ASCENDING[26] = {
	0X0, 0X1, 0X3, 0X7,
        0X0F, 0X1F, 0X3F, 0X7F,
        0X0FF, 0X1FF, 0X3FF, 0X7FF,
        0X0FFF, 0X1FFF, 0X3FFF, 0X7FFF,
        0X0FFFF, 0X1FFFF, 0X3FFFF, 0X7FFFF,
        0X0FFFFF, 0X1FFFFF, 0X3FFFFF, 0X7FFFFF,
        0X0FFFFFF, 0X1FFFFFF };

/*
**++
**  ROUTINE NAME: search
**
**  FUNCTIONAL DESCRIPTION:
**	This routine searches for a string inside another string. If it
**	is found, the offset into the string is returned. Otherwise, a
**	value of -1 is returned.
**
**  FORMAL PARAMETERS:
**	string - string to search.
**	target - string to look for.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
int search(string, target)
    char string[], target[];
{
    char *tempstring;
    char lc_string[TEXT_LENGTH + 1], lc_target[TEXT_LENGTH + 1];
    int found = -1, length;

    if ((string == NULL) || (target == NULL))
	return (-1);

    /* Convert both strings to lower case for case insensitivity. */
    tempstring = DWI18n_ToLower(string);
    strcpy(lc_string, tempstring);
    XtFree(tempstring);

    tempstring = DWI18n_ToLower(target);
    strcpy(lc_target, tempstring);
    XtFree(tempstring);
    tempstring = lc_string;
    length = strlen(lc_target);

    for (;;) {
	tempstring = myindex(tempstring, lc_target[0]);
	if (tempstring != 0) {
	    found = strncmp(tempstring, lc_target, length);
	    if (found == 0)
		return (tempstring - lc_string);
	    else {
		found = -1;
		DWI18n_CharIncrement(&tempstring);
	    }
	} else
	    return (-1);
    }
}

/*
**++
**  ROUTINE NAME: lower
**
**  FUNCTIONAL DESCRIPTION:
**	This function accepts a character and returns it lower's case value.
**	If the letter is not a capital letter (A - Z) then whatever was
**	passed into the function is returned.
**
**  FORMAL PARAMETERS:
**	c - character to convert to lower case.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
lower(c)
    int c;
{
    if (c >= 'A' && c <= 'Z')
	return (c + 'a' - 'A');
    else
	return (c);
}

/*
**++
**  ROUTINE NAME: comparethem
**
**  FUNCTIONAL DESCRIPTION:
**	This routine accepts two strings and compares them. It converts each
**	character to a lower case value before comparing. It is therefore
**	case insensitive. After converting to lower case, strcmp is called
**	and it's value is returned.
**
**  FORMAL PARAMETERS:
**	one - string one for comparison
**	two - string two for comparison
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
comparethem(one, two)
    char *one, *two;
{
    char *temp1, *temp2;
    char string1[TEXT_LENGTH + 1], string2[TEXT_LENGTH + 1];
    int flags = 0, ret_status = -1;
    XmString sort_array[2];
#ifndef NO_XNLS
    xnl_status status;
    Boolean xnl_error = FALSE;
#endif

    if (!DWI18n_IsXnlLangISOLatin1())
	use_c_sort = TRUE;

#ifndef NO_XNLS
    if (!use_c_sort) {
	flags = flags | XNL_COMPFLAGDIAC;

	sort_array[0] = XmStringCreate(one, XmSTRING_DEFAULT_CHARSET);
	sort_array[1] = XmStringCreate(two, XmSTRING_DEFAULT_CHARSET);

	if (XmStringByteCompare(sort_array[0], sort_array[1]))
	    ret_status = 0;
	else {
	    status = xnl_sort(language_id, 2, flags, sort_array);

	    if (status != XnlSuccess) {
		    /* XNL failed, switch to C sort as backup */
		    xnl_error = TRUE;
		    use_c_sort = TRUE;
		    printf("Xnl_sort routine failed, status is %d \n", status);
	    }
	    else {
		if (XmStringByteCompare(sort_array[0],
		  XmStringCreate(two, XmSTRING_DEFAULT_CHARSET)))
		    ret_status = 1;
	    }
	    XmStringFree(sort_array[0]);
	    XmStringFree(sort_array[1]);
	}
	if (!xnl_error)
	    return (ret_status);
    }
#endif

    if (use_c_sort) {
	strcpy(string1, one);
	strcpy(string2, two);
	temp1 = string1;
	temp2 = string2;

	temp1 = DWI18n_ToLower(string1);
	strcpy(string1, temp1);
	XtFree(temp1);

	temp2 = DWI18n_ToLower(string2);
	strcpy(string2, temp2);
	XtFree(temp2);
	return (strcmp(string1, string2));
    }
}

/*
**++
**  ROUTINE NAME: ncompare
**
**  FUNCTIONAL DESCRIPTION:
**	This routine compares two strings for up to n characters after
**	converting them both to lower case.
**
**  FORMAL PARAMETERS:
**	one - string to compare to.
**	two - string being compared.
**	n   - number of characters to compare.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
ncompare(one, two, n)
    char *one, *two;
    int n;
{

    char *temp1, *temp2;
    char string1[TEXT_LENGTH + 1], string2[TEXT_LENGTH + 1];

    strcpy(string1, one);
    strcpy(string2, two);
    temp1 = string1;
    temp2 = string2;

    while (*temp1) {
	*temp1 = lower(*temp1);
	temp1++;
    }
    while (*temp2) {
	*temp2 = lower(*temp2);
	temp2++;
    }
    return (strncmp(string1, string2, n));
}

/*
**++
**  ROUTINE NAME: uptolow
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure converts a string of uppercase characters to lower
**	case using the lower procedure.
**
**  FORMAL PARAMETERS:
**	up  - string to convert to lower case.
**	low - buffer to return lower case string in.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
uptolow(up, low)
    char *up;
    char *low;
{
    char *temp1, *temp2;

    temp1 = up;
    temp2 = low;

    while (*temp1) {
	*temp2 = lower(*temp1);
	temp1++;
	temp2++;
    }
    *temp2 = '\0';
}

/*
**++
**  ROUTINE NAME: getnum
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure reads in a specified number of characters from a file
**	and places them into the buffer whose address is passed to the
**	routine. A generic routine to read in any type of data.
**
**  FORMAL PARAMETERS:
**	s  - buffer for characters
**	n  - number of characters to be read
**	fp - Pointer to file to be read from
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
char *getnum(s, n, fp)
    char *s;				/* buffer for characters */
    int n;				/* number of characters to be read */
    FILE *fp;				/* Pointer to file to be read from */
{
    int i, c;
    char *cs;

    cs = s;
    while (n-- > 0 && (c = getc(fp)) != EOF) {
	*cs++ = c;
	i++;
    }
    *cs = '\0';
    return ((c == EOF && cs == s) ? (char *) NULL : s);
}

/*
**++
**  ROUTINE NAME: myindex
**
**  FUNCTIONAL DESCRIPTION:
**	Replaces the C index routine. Searches for the first occurence
**	of a character in a string and then returns a pointer to that
**	character in the string.
**
**  FORMAL PARAMETERS:
**	s - string to search.
**	c - character to look for.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
char *myindex(s, c)
    char *s;
    char c;
{
    /* For searching cards with a blank index or a blank card. */
    if ((c == NULL) && (*s == NULL))
	return (s);

    while ((*s != c) && (*s != NULL))
	DWI18n_CharIncrement(&s);
    if (*s == NULL)
	return ((char *) 0);
    else
	return (s);
}

/*
**++
**  ROUTINE NAME: mybcopy
**
**  FUNCTIONAL DESCRIPTION:
**	Replaces the C bcopy routine. Copies an area of memory to another.
**
**  FORMAL PARAMETERS:
**	b1 - buffer to copy from.
**	b2 - buffer to copy into.
**	n  - number of bytes to copy.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
mybcopy(b1, b2, n)
    char *b1;
    char *b2;
    int n;
{
    int i;

    for (i = 0; i < n; i++) {
	*b2 = *b1;
	b1++;
	b2++;
    }
}

/*
**++
**  ROUTINE NAME: imagecopy
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**	image  - image to copy from.
**	bitmap - bitmap to copy into.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
imagecopy(image, bitmap)
    XImage *image;
    char *bitmap;
{
#ifndef NO_ISL
    char *image_data;
    unsigned int bits;
    int remainder;
    int image_count = 0, bitmap_count = 0;
    int start_image_count;
    int i, j;
    int block_size = 16;

    for (i = 0; i < ((image->width * image->height / 8) + 1); i++)
	bitmap[i] = 0;

    image_data = image->data;

    for (i = 0; i < image->height; i++) {
	remainder = image->width;
	start_image_count = image_count;

	while (remainder >= block_size) {
	    bits = 0;
	    bits = GET_VALUE(image_data, image_count, block_size);
	    PUT_VALUE(bitmap, bitmap_count, block_size, bits);
	    image_count += block_size;
	    bitmap_count += block_size;
	    remainder -= block_size;
	}

	if (remainder != 0) {
	    bits = 0;
	    bits = GET_VALUE(image_data, image_count, remainder);
	    PUT_VALUE(bitmap, bitmap_count, remainder, bits);
	    bitmap_count += remainder;
	}

	image_count = start_image_count + (image->bytes_per_line * 8);

    }
#endif
}


