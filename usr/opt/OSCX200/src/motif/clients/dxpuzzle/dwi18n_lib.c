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
/*  DEC/CMS REPLACEMENT HISTORY, Element DWI18N_LIB.C */
/*  *2    12-JAN-1992 23:41:20 INADA "Bug fix - sizeof -->strlen" */
/*  *1    12-DEC-1991 17:23:46 DECWBUILD "Internationalization Library - initial version" */
/*  DEC/CMS REPLACEMENT HISTORY, Element DWI18N_LIB.C */
/*
*****************************************************************************
**                                                                          *
**  COPYRIGHT (c) 1991 BY                                                   *
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.                  *
**  ALL RIGHTS RESERVED.                                                    *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED   *
**  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE   *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER   *
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY   *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY   *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE   *
**  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT   *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS   *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
**                                                                          *
*****************************************************************************
**
**	DWI18N_LIB.C
**
**	- Common Library for DECwindows/Motif Applications
**	  These routines enable an application programmer to facilitate I18n
**	  on both VMS and ULTRIX platform.
**
**
**        DWI18n_ByteLengthOfChar
**        DWI18n_CharCount
**        DWI18n_CharIncrement
**        DWI18n_ClipboardCopy
**        DWI18n_ClipboardPaste
**        DWI18n_CreatePath
**        DWI18n_GetXLFDCharSet
**        DWI18n_IsXnlLangISOLatin1
**        DWI18n_SetIconName
**        DWI18n_SetTitle
**        DWI18n_ToLower
**        DWI18n_ToUpper
**        DWI18n_RebindIrreqularKeysym
**
*/

/*======================================*/
/*	   << Include Files >>		*/
/*======================================*/

#include <Xm/Xm.h>
#include <Xm/CutPaste.h>
#include <DXm/DECspecific.h>
#include <string.h>
#include <ctype.h>

#define XK_KATAKANA
#define XK_HEBREW
#include <X11/keysym.h>

#ifdef VMS
#include "descrip.h"
#include "lnmdef.h"
#include "ssdef.h"
#include "psldef.h"
#endif /* VMS */



/*=========================================================================*/
/*			<< Language Specification >>			   */
/*									   */
/* The language specification is represented in a format which consists of */
/* four parts as below.							   */
/*									   */
/*	'language'[_territory][.codeset][@modifier]			   */
/*									   */
/* Note that the 'language' is defined in standard ISO 639:1988 - ISO 	   */
/* Language Code and the 'territory' is defined in standard ISO 3166:1988 -*/
/* ISO Country Code.							   */
/*									   */
/*=========================================================================*/

/* This library supports the following 'language_territory' and 'codeset'. */

/*==============================================*/
/*      << Supported 'language_territory' >>	*/
/*==============================================*/

#define LANG_SPEC_CHINESE_CHINA		"zh_CN"
#define LANG_SPEC_CHINESE_TAIWAN	"zh_TW"
#define LANG_SPEC_HEBREW		"iw_IL"
#define LANG_SPEC_JAPANESE		"ja_JP"
#define	LANG_SPEC_KOREAN		"ko_KR"


/*==============================================*/
/*          << Supported 'codeset'	>>	*/
/*==============================================*/

#define LANG_SPEC_DECKOREAN		"deckorean"
#define	LANG_SPEC_DECHANYU     		"dechanyu"
#define LANG_SPEC_DECHANZI     		"dechanzi"
#define LANG_SPEC_ISOHEBREW    		"ISO8859-8"
#define LANG_SPEC_88598    		"88598"
#define LANG_SPEC_DECKANJI     		"deckanji"


/*=========================================================================*/
/* 		<< How to map the language specification >>		   */
/*									   */
/* The language specification is mapped to facilitate text manipulation.   */
/* The language is mapped to a constant defined as below.		   */
/*									   */
/*		      	      --- Bit Assignment ---			   */
/*		    24           16            8            0 		   */
/*	 +---------------------------------------------------+  	   */
/*	 |  reserved  | language_  |   codeset  |  modifier  |	  	   */
/*	 |	      |	territory  |            |            |		   */
/*	 +---------------------------------------------------+	   	   */
/*	 |<- 1 byte ->|<- 1 byte ->|<- 1 byte ->|<- 1 byte ->|	   	   */
/*									   */
/*									   */
/*=========================================================================*/

/* The following specifies the number to be mapped to.	*/

/*==============================================*/
/*   << No. indicates 'language_territory' >>   */
/*==============================================*/

#define	INVALID_LANG   	0  		/* Not mapped yet	*/
#define NON_ASIAN      	0x00010000
#define CHINESE_CHINA  	0x00020000
#define CHINESE_TAIWAN 	0x00030000
#define HEBREW	       	0x00040000
#define JAPANESE       	0x00050000
#define KOREAN         	0x00060000

/*==============================================*/
/*        << No. indicates 'codeset' >>   	*/
/*==============================================*/

#define	ISOLATIN1	0x00000100
#define DECKOREAN	0x00000200
#define	DECHANYU	0x00000300
#define DECHANZI	0x00000400
#define DECHEBREW	0x00000500
#define DECKANJI	0x00000600


/*==============================================*/
/*  << Default Combinations of Lang Spec. >>	*/
/*==============================================*/

#define NON_ASIAN_ISOLATIN1		(NON_ASIAN | ISOLATIN1)
#define CHINESE_CHINA_DECHANZI		(CHINESE_CHINA | DECHANZI)
#define CHINESE_TAIWAN_DECHANYU		(CHINESE_TAIWAN | DECHANYU)
#define HEBREW_DECHEBREW		(HEBREW | DECHEBREW)
#define JAPANESE_DECKANJI		(JAPANESE | DECKANJI)
#define KOREAN_DECKOREAN		(KOREAN | DECKOREAN)

/*==============================================*/
/*  Global variable to store the mapped lang.   */
/*==============================================*/

static	int language_id = INVALID_LANG;


/*==============================================*/
/*  << Leading char. code for HANYU >>		*/
/*==============================================*/

#define HANYU_LEADING_CODE_FIRST_BYTE  0xC2
#define HANYU_LEADING_CODE_SECOND_BYTE 0xCB


/*==============================================*/
/*	<< Miscellaneous Definitions >>		*/
/*==============================================*/

#define LANGUAGE_AREA		  2 		/* used to compare 'language' */
#define LANGUAGE_TERRITORY_AREA   5    		/* used to compare 	      */
						/* 'language_territory'	      */
#define MASK_MSB_OF_CHAR_CODE	  0x80 		/* used to check MSB of a     */
						/* char. code		      */
#define MASK_LANGUAGE_TERRITORY_AREA 0x00FF0000	/* used to check 	      */
						/* 'language_territory' area  */
#define MASK_CODESET_AREA	  0x0000FF00	/* used to see 'codeset' area */
#define CLEAR_CODESET_AREA	  0xFFFF00FF	/* used to clear 'codeset'    */
#define SEPARATOR_FOR_CODESET	  '.'		/* [.codeset]		      */
#define SEPARATOR_FOR_MODIFIER	  '@'		/* [@modifiler]		      */


/*==============================================*/
/*	<< Macros for Text Manipulation >>	*/
/*==============================================*/

#define	MapLangIfNeeded()\
	  {\
	    if (language_id == INVALID_LANG)\
	    _I18n_MapLanguage();\
	  }	    

#define ByteLengthOfCharForDECKorean(_ptr, _len)\
	  {\
		if (*_ptr & MASK_MSB_OF_CHAR_CODE)\
			_len = 2;\
		else\
			_len = 1;\
	  }

#define ByteLengthOfCharForDECHanzi(_ptr, _len)\
	  {\
		if (*_ptr & MASK_MSB_OF_CHAR_CODE)\
			_len = 2;\
		else\
			_len = 1;\
	  }

#define ByteLengthOfCharForDECHanyu(_ptr, _len)\
	  {\
		if (*_ptr & MASK_MSB_OF_CHAR_CODE)\
			if ((_ptr[0] == HANYU_LEADING_CODE_FIRST_BYTE) &&\
            		    (_ptr[1] == HANYU_LEADING_CODE_SECOND_BYTE) &&\
			    (_ptr[2] & MASK_MSB_OF_CHAR_CODE))\
				_len = 4;\
			else\
				_len = 2;\
		else\
			_len = 1;\
	   }

#define ByteLengthOfCharForDECKanji(_ptr, _len)\
	  {\
		if (*_ptr & MASK_MSB_OF_CHAR_CODE)\
			_len = 2;\
		else\
			_len = 1;\
	  }


/*
**++
**  ROUTINE NAME: _I18n_MapLanguage
**
**  FUNCTIONAL DESCRIPTION:
**	This routine maps the global language string to a constant.
**	At this stage, only the 'language_territory' in the locale 
**	representation is referred. The 'codeset' will be set based on the
**	'language_territory'.
**
**  FORMAL PARAMETERS:
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

static void
 _I18n_MapLanguage()
{
char	*language;
int	i, j;
int	len_codeset;

language = (char *)xnl_getlanguage();         /* Retrieve language	     */

    if (!language)		              /* See if language is invalid. */
      {
	language_id = NON_ASIAN_ISOLATIN1;    /* Set default language        */
	return;
      }

/*------------------------------------------------------*/
/* Set default value based on 'language_territory'.	*/
/*------------------------------------------------------*/

    if (!strncmp(language, LANG_SPEC_CHINESE_CHINA, LANGUAGE_TERRITORY_AREA))
	language_id = CHINESE_CHINA_DECHANZI;
    else if (!strncmp(language, LANG_SPEC_CHINESE_TAIWAN, LANGUAGE_TERRITORY_AREA))
	language_id = CHINESE_TAIWAN_DECHANYU;
    else if (!strncmp(language, LANG_SPEC_HEBREW, LANGUAGE_AREA))
	language_id = HEBREW_DECHEBREW;
    else if (!strncmp(language, LANG_SPEC_JAPANESE, LANGUAGE_AREA))
	language_id = JAPANESE_DECKANJI;
    else if (!strncmp(language, LANG_SPEC_KOREAN, LANGUAGE_AREA))
	language_id = KOREAN_DECKOREAN;
    else
        language_id = NON_ASIAN_ISOLATIN1;    /* Set default language        */


/*------------------------------------------------------*/
/* If 'codeset' is specified, replace the value.	*/
/*------------------------------------------------------*/

/*
 *	The following code will be useful in the near future.
 *
 *
 *   i = 0;				        
 *   while ((language[i] != SEPARATOR_FOR_CODESET) && language[i])  i++;
 *
 *   if (language[i] == SEPARATOR_FOR_CODESET)
 *     {
 *	len_codeset = 0;
 *	j = i + 1;
 *	while ((language[j] != SEPARATOR_FOR_MODIFIER) && language[j])
 *	     {
 *		len_codeset++;
 *		j++;
 *	     }
 *
 *	if (len_codeset > 0)
 *	  {
 *		language_id &= CLEAR_CODESET_AREA;
 *
 *		if ((len_codeset == strlen(LANG_SPEC_DECKOREAN)) &&
 *		    !strncmp(&language[i+1], LANG_SPEC_DECKOREAN, len_codeset))
 *			language_id |= DECKOREAN;
 *	        else if ((len_codeset == strlen(LANG_SPEC_DECHANYU)) &&
 *		    !strncmp(&language[i+1], LANG_SPEC_DECHANYU, len_codeset))
 *		   	language_id |= DECHANYU;
 *	        else if ((len_codeset == strlen(LANG_SPEC_DECHANZI)) &&
 *		    !strncmp(&language[i+1], LANG_SPEC_DECHANZI, len_codeset))
 *		   	language_id |= DECHANZI;
 *	        else if ((len_codeset == strlen(LANG_SPEC_ISOHEBREW)) &&
 *		    !strncmp(&language[i+1], LANG_SPEC_ISOHEBREW, len_codeset))
 *		   	language_id |= DECHEBREW;
 *	        else if ((len_codeset == strlen(LANG_SPEC_88598)) &&
 *		    !strncmp(&language[i+1], LANG_SPEC_88598, len_codeset))
 *		   	language_id |= DECHEBREW;
 *	        else if ((len_codeset == strlen(LANG_SPEC_DECKANJI)) &&
 *		    !strncmp(&language[i+1], LANG_SPEC_DECKANJI, len_codeset))
 *		   	language_id |= DECKANJI;
 *		else
 *		   language_id |= ISOLATIN1;
 *         }
 *     }
 */


/*------------------------------------------------------*/
/* If 'modifier' is specified, replace the value.	*/
/*------------------------------------------------------*/

		/* Not implemented */

}


/*
**++
**  ROUTINE NAME: DWI18n_ByteLengthOfChar
**
**  FUNCTIONAL DESCRIPTION:
**	This routine returns the number of bytes in the length of a character.
**
**  FORMAL PARAMETERS:
**      chr - (I) A pointer to the input character code to be count.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**	The number of bytes of a character code.
**
**  SIDE EFFECTS:
**--
**/

int DWI18n_ByteLengthOfChar(chr)
char *chr;
{
    int		len;

    MapLangIfNeeded();

    switch (language_id & MASK_CODESET_AREA) {
       case DECHANZI:
		ByteLengthOfCharForDECHanzi(chr, len);
		break;
       case DECKOREAN:
		ByteLengthOfCharForDECKorean(chr, len);
		break;
       case DECHANYU:
		ByteLengthOfCharForDECHanyu(chr, len);
		break;
       case DECKANJI:
		ByteLengthOfCharForDECKanji(chr, len);
		break;
       default:
           	len = 1;
                break;
    }

    return (len);
}


/*
**++
**  ROUTINE NAME: DWI18n_CharCount
**
**  FUNCTIONAL DESCRIPTION:
**	This routine counts characters contained in the string.
**
**  FORMAL PARAMETERS:
**	str   -	A pointer to a string to be counted.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**	The number of characters contained in the string.
**
**  SIDE EFFECTS:
**--
**/
int DWI18n_CharCount(str)
  char	*str;
{
  int 	str_len;
  int	char_len;
  int	i = 0;
  int	char_count = 0;
  char	*ptr;

  MapLangIfNeeded();

  str_len = strlen(str);
  switch(language_id & MASK_CODESET_AREA) {
	case DECHANZI:
		while (i < str_len) {
		    ByteLengthOfCharForDECHanzi(&str[i], char_len);
		    i += char_len;
		    char_count++;
		}
		break;
	case DECKOREAN:
		while (i < str_len) {
		    ByteLengthOfCharForDECKorean(&str[i], char_len);
		    i += char_len;
		    char_count++;
		}
		break;
	case DECHANYU:
		while (i < str_len) {
		    ptr = &str[i];
		    ByteLengthOfCharForDECHanyu(ptr, char_len);
		    i += char_len;
		    char_count++;
		}
		break;
	case DECKANJI:
		while (i < str_len) {
		    ByteLengthOfCharForDECKanji(&str[i], char_len);
		    i += char_len;
		    char_count++;
		}
		break;
	default:
		char_count = str_len;
		break;
   }

   return (char_count);
}


/*
**++
**  ROUTINE NAME: DWI18n_CharIncrement
**
**  FUNCTIONAL DESCRIPTION:
**	This routine increments a character pointer.
**
**  FORMAL PARAMETERS:
**	ptr - (I/O) A pointer to character pointer to be incremented.
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

void DWI18n_CharIncrement(ptr)
char **ptr;
{
  int	len;

  MapLangIfNeeded();

  switch(language_id & MASK_CODESET_AREA) {
        case DECHANZI:
		ByteLengthOfCharForDECHanzi(*ptr, len);
		break;
        case DECKOREAN:
		ByteLengthOfCharForDECKorean(*ptr, len);
		break;
        case DECHANYU:
		ByteLengthOfCharForDECHanyu(*ptr, len);
		break;
	case DECKANJI:
		ByteLengthOfCharForDECKanji(*ptr, len);
		break;
	default:
		len = 1;
		break;
  }

 (*ptr) += len;
}


/*
**++
**  ROUTINE NAME: DWI18n_ClipboardCopy
**
**  FUNCTIONAL DESCRIPTION:
**	This routine transfers data to the clipboard in three types of
**	format - DDIF, COMPOUND_TEXT, and STRING.
**
**  FORMAL PARAMETERS:
**	dpy	    -   a pointer to the Display structure that was returned
** 		        in a previous call to XOpenDisplay or XtInitialize.
**	win	    -   the window ID that relates the application
**			window to the clipboard. The same application
**			instance should pass the same window ID to each of
**			the clipboard functions that it calls. Note that
**			this window must be associated with a widget.
**	item_id     -  	the number assigned to this data item. This number
**			was returned by a previous call to XmClipboardStartCopy.
**	private_id  -  	the private data that the application wants to
**			store with the data item.
**	cs_data     -  	a compound string to be converted into three types
**			of format and to be copied to the clipboard.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**	This routine can return one of the following status return constants:
**		ClipboardSuccess	The routine is successful.
**		ClipboardLocked		The routine failed because the
**					clipboard was locked by another
**					application. The application can
**					continue to call the function again
**					with the same parameters until the lock
**					goes away.
**		ClipboardFail		The routine could not transfer
**					data to the clipboard at all.
**  SIDE EFFECTS:
**--
**/

int DWI18n_ClipboardCopy(dpy, win, item_id, private_id, cs_data)
  Display	*dpy;
  Window	win;
  long		item_id;
  int		private_id;
  XmString	cs_data;
{
  long		byte_count, cvt_status;
  int 		status;
  int		data_id;
  char		*str;
  char		*ct_str;
  Opaque	ddif_str;
  Boolean	NoCopy	= TRUE;

  if (!cs_data)
     return (ClipboardFail);			/* no data */


  /* Try to copy DDIF data first */

  ddif_str = DXmCvtCStoDDIF (cs_data, &byte_count, &cvt_status);
  if (cvt_status != DXmCvtStatusFail)
    {
	  status = XmClipboardCopy (dpy, win, item_id, "DDIF", ddif_str,
			    	byte_count, private_id, &data_id);
	  XtFree(ddif_str);

	  if (status == ClipboardSuccess)
	    {
		NoCopy = FALSE;
	    }
    }

  if ((cvt_status == DXmCvtStatusFail) || (status != ClipboardSuccess))
    {	/* clean the clipboard with '\0' */
	  status = XmClipboardCopy (dpy, win, item_id, "DDIF", '\0',
			    	    1, private_id, &data_id);
    }


  /* Try to copy COMPOUND_TEXT data secondly */

  ct_str = XmCvtXmStringToCT (cs_data);
  status = XmClipboardCopy (dpy, win, item_id, "COMPOUND_TEXT", ct_str,
			    strlen(ct_str), private_id, &data_id);
  XtFree(ct_str);
  if (status == ClipboardSuccess)
    {
	NoCopy = FALSE;
    }
  else
    {	/* clean the clipboard with '\0' */
	  status = XmClipboardCopy (dpy, win, item_id, "COMPOUND_TEXT", '\0',
			    	    1, private_id, &data_id);
    }


  /* Try to copy STRING data lastly */

  str = DXmCvtCStoFC (cs_data, &byte_count, &cvt_status);
  if (cvt_status != DXmCvtStatusFail)
    {
  	status = XmClipboardCopy (dpy, win, item_id, "STRING", str,
			    	  byte_count, private_id, &data_id);
	XtFree(str);
	if (status == ClipboardSuccess)
    	  {
		NoCopy = FALSE;
    	  }
    }

  if ((cvt_status == DXmCvtStatusFail) || (status != ClipboardSuccess))
    {	/* clean the clipboard with '\0' */
	  status = XmClipboardCopy (dpy, win, item_id, "STRING", '\0',
			    	    1, private_id, &data_id);
    }

  if (NoCopy)
    {
	if (status == ClipboardLocked)
	  {
		return(ClipboardLocked);
	  }
	else
	  {
		return(ClipboardFail);
          }
    }
  else
    {
	return(ClipboardSuccess);
    }
}



/*
**++
**  ROUTINE NAME: DWI18n_ClipboardPaste
**
**  FUNCTIONAL DESCRIPTION:
**	This routine retrieves data from the clipboard and converts the
**	data into a compound string.
**	This routine can not handle bitmap data. For example,
**	if DDIF data contain bitmap data, the DDIF data are skipped and the
**	routine tries to retrieve COMPOUND_TEXT data.
**
**  FORMAL PARAMETERS:
**	dpy	    -   a pointer to the Display structure that was returned
** 			in a previous call to XOpenDisplay or XtInitialize.
**	win	    -   the window ID that relates the application
**			window to the clipboard. The same application
**			instance should pass the same window ID to each of
**			the clipboard functions that it calls. Note that
**			this window must be associated with a widget.
**	cs_data	    -   a pointer to a buffer in which a compound string is
**			stored (The data retrieved from the clipboard are
**			converted to the compound_string).
**			The user must free this buffer by using the Toolkit
**			routine XmStringFree.
**	private_id  -   returns any private data that may have been stored
**			with data item
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**	This routine can return one of the following status return constants:
**		ClipboardSuccess	The routine is successful.
**		ClipboardLocked		The routine failed because the
**					clipboard was locked by another
**					application. The application can
**					continue to call the function again
**					with the same parameters until the lock
**					goes away.
**		ClipboardNoData		The routine could not find data on
**					the clipboard.
**		ClipboardFail		The routine could not allocate
**					enough memory to store data.
**
**  SIDE EFFECTS:
**--
**/

int DWI18n_ClipboardPaste(dpy, win, cs_data, private_id)
  Display       *dpy;
  Window        win;
  XmString	*cs_data;
  int		*private_id;
{
  int           status;
  unsigned long length;
  unsigned long num_bytes;
  char          *buf;
  long		byte_count, cvt_status;

  /* DDIF */
  status = XmClipboardInquireLength (dpy, win,"DDIF", &length);
  if (status == ClipboardLocked)
    {
     	return(status);
    }

  if (length > 0)
    {
	buf = NULL;
        buf = XtMalloc((Cardinal)(length+1));
	if (!buf)
	  {
		return (ClipboardFail);
	  }

        status = XmClipboardRetrieve (dpy, win, "DDIF", buf, length,
				      &num_bytes, private_id);

        switch (status)
           {
                case ClipboardLocked :
			XtFree(buf);
                        return (status);
                case ClipboardNoData :
			XtFree(buf);
                        break;
                case ClipboardSuccess :
                        buf[num_bytes] = '\0';
			*cs_data = DXmCvtDDIFtoCS(buf, &byte_count, &cvt_status);
			XtFree(buf);
			if (cvt_status != DXmCvtStatusFail)
			  {
	                        return (status);
			  }
           }
    }


  /* COMPOUND_TEXT */
  status = XmClipboardInquireLength (dpy, win,"COMPOUND_TEXT", &length);
  if (status == ClipboardLocked)
    {
     	return(status);
    }

  if (length > 0)
    {
	buf = NULL;
        buf = XtMalloc((Cardinal)(length+1));
	if (!buf)
	  {
		return (ClipboardFail);
	  }

        status = XmClipboardRetrieve (dpy, win, "COMPOUND_TEXT", buf, length,
				      &num_bytes, private_id);

        switch (status)
           {
                case ClipboardLocked :
			XtFree(buf);
                        return (status);
                case ClipboardNoData :
			XtFree(buf);
                        break;
                case ClipboardSuccess :
                        buf[num_bytes] = '\0';
			*cs_data = XmCvtCTToXmString(buf);
			XtFree(buf);
	                return (status);
           }
    }

  /* STRING */
  status = XmClipboardInquireLength (dpy, win,"STRING", &length);
  if (status == ClipboardLocked)
    {
     	return(status);
    }

  if (length > 0)
    {
	buf = NULL;
        buf = XtMalloc((Cardinal)(length+1));
	if (!buf)
	  {
		return (ClipboardFail);
	  }

        status = XmClipboardRetrieve (dpy, win, "STRING", buf, length,
				      &num_bytes, private_id);

        switch (status)
           {
                case ClipboardLocked :
			XtFree(buf);
                        return (status);
                case ClipboardNoData :
			XtFree(buf);
                        break;
                case ClipboardSuccess :
                        buf[num_bytes] = '\0';
			*cs_data = DXmCvtFCtoCS(buf, &byte_count, &cvt_status);
			XtFree(buf);
			if (cvt_status != DXmCvtStatusFail)
			  {
	                        return (status);
			  }
           }
    }

  return (ClipboardNoData);
}


#ifdef VMS		/* This routine is available on VMS platform only. */
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      DWI18n_CreatePath - create a language specific variant of a
**          		    logical name defined search path.
**
**	This is an exact copy of xnl_createpath.
**
**  FORMAL PARAMETERS:
**
**      path 	 - (I) The logical name to create a language specific
**          	       variant of.
**      language - (I) The variant to create.
**
**  IMPLICIT INPUTS:
**
**      Existing search list elements from the logical name are
**      propagated to the new logical name.
**
**  FUNCTION VALUE:
**
**      1 is returned if success, 0 otherwise.
**
**  SIDE EFFECTS:
**
**      The new logical name is created.
**
**--
**/

int DWI18n_CreatePath(path, language_str)
    char *path;
    char *language_str;
{
        int status, i, trnsl_num;
        char *new_path;
        char *trnsl_table[129];


        /* Allocate enough memory to build a new logical name specification
        ** and then build the logical name string.
        */
        new_path = XtMalloc((Cardinal)(strlen(path) + strlen(language_str) + 3));

        if (new_path == 0) {
            return(0);
        }

        strcpy( new_path, path);
        strcat(new_path, "_");
        strcat( new_path, language_str);
        strcat(new_path, ":");
        trnsl_table[0] = new_path;


        /* Get the old values of the general purpose search list.  If this
        ** fails, then something is really wrong since these logical names
        ** should always be defined.
        */
        status = _I18n_trnlnm(path,
                          "LNM$FILE_DEV",
                          &trnsl_num,
                          &trnsl_table[1],
                          PSL$C_USER);

        if (status != 1) {
            XtFree(new_path);
            return(0);
        }


        /* Everything looks good.  Define the logical.
        */
        trnsl_num = trnsl_num + 1;

        status = _I18n_crelnm(path,
                             "LNM$PROCESS",
                             trnsl_table,
                             trnsl_num,
                             PSL$C_USER);


        /* We're done.  Free up our memory and get out.
        */
        for (i=0; i<trnsl_num; i++) XtFree(trnsl_table[i]);
        return(status);
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      _I18n_trnlnm - translates a logical name and returns its values.
**      This is an exact copy of xnl_trnlnm.
**
**  FORMAL PARAMETERS:
**
**      lognam      - (I) A name to translate
**      tabnam      - (I) A logical name table to look in
**      trnsl_num   - (O) num. of translations found
**      trnsl_table - (O) The table of pointers to translations found.
**              	  These pointers need to be deallocated by the caller.
**      access_mode - (I) access mode to look in
**
**  IMPLICIT INPUTS:
**
**      The existing logical name.
**
**  FUNCTION VALUE:
**
**      If successful, 1.  Otherwise, 0.
**
**  SIDE EFFECTS:
**
**      None.
**
**--
**/

static int
_I18n_trnlnm(lognam, tabnam, trnsl_num, trnsl_table, access_mode)
    char *lognam;
    char *tabnam;
    int *trnsl_num;
    char *trnsl_table[];
    unsigned char access_mode;
{
        typedef struct {
            unsigned short buf_len;
            unsigned short item_code;
            unsigned char *buf_add;
            unsigned long ret_add;
        } item_desc;

        item_desc item_list[3];
        unsigned long attr, index;
        unsigned short ret_len;
        struct dsc$descriptor_s lognam_dsc, tabnam_dsc;
        char temp_buf[255];
        int status;

        /* Set up the inputs for the translation.  We can do this once
        ** because everything that changes is passed by ref.
        */
        lognam_dsc.dsc$a_pointer = lognam;
        lognam_dsc.dsc$b_dtype   = DSC$K_DTYPE_T;
        lognam_dsc.dsc$w_length  = strlen(lognam);
        lognam_dsc.dsc$b_class   = DSC$K_CLASS_S;

        tabnam_dsc.dsc$a_pointer = tabnam;
        tabnam_dsc.dsc$b_dtype   = DSC$K_DTYPE_T;
        tabnam_dsc.dsc$w_length  = strlen(tabnam);
        tabnam_dsc.dsc$b_class   = DSC$K_CLASS_S;

        attr = LNM$M_CASE_BLIND;
        index = 0;

        item_list[0].buf_len   = 4;
        item_list[0].buf_add   = &index;
        item_list[0].item_code = LNM$_INDEX;
        item_list[0].ret_add   = &ret_len;

        item_list[1].buf_len   = 255;
        item_list[1].buf_add   = temp_buf;
        item_list[1].item_code = LNM$_STRING;
        item_list[1].ret_add   = &ret_len;

        item_list[2].buf_len   = 0;
        item_list[2].buf_add   = 0;
        item_list[2].item_code = 0;
        item_list[2].ret_add   = 0;


        /* Do as many translations as are required to get all the existing
        ** values for the logical name.  For each value, allocate enough
        ** memory to save a copy of it and then put a pointer to it into
        ** our return table.
        */
        do {
            status = SYS$TRNLNM(&attr,
                            &tabnam_dsc,
                            &lognam_dsc,
                            &access_mode,
                            &item_list);

            if ( ((status & SS$_NORMAL) == SS$_NORMAL) && (ret_len != 0) ) {
                trnsl_table[index] = XtMalloc((Cardinal)(ret_len + 1));
                strncpy(trnsl_table[index], item_list[1].buf_add, ret_len);
                strcpy(trnsl_table[index] + ret_len, "");
                index++;
            }
        }
        while ( ((status & SS$_NORMAL) == SS$_NORMAL) && (ret_len != 0) );


        /* Return the number of translations found and a normalized status.
        */
        *trnsl_num = index;
        if ((status & SS$_NORMAL) == SS$_NORMAL) {
            return (1);
        }
        else return (0);
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      _I18n_crelnm - creates a logical name
**      This is an exact copy of xnl_crelnm.
**
**  FORMAL PARAMETERS:
**
**      lognam - name to create
**      tabnam - logical name table to create it in
**      trnsl_num - num of translations to be provided
**      trnsl_table - table of translations to provide
**      access_mode - access mode to look in
**
**  IMPLICIT INPUTS:
**
**      The existing logical name.
**
**  FUNCTION VALUE:
**
**      If successful, 1.  Otherwise, 0.
**
**  SIDE EFFECTS:
**
**      The logical name is created.
**
**--
**/

static int
_I18n_crelnm(lognam, tabnam, trnsl_table, trnsl_num, access_mode)
    char *lognam;
    char *tabnam;
    char *trnsl_table[];
    int trnsl_num;
    unsigned char access_mode;
{
        typedef struct {
            unsigned short buf_len;
            unsigned short item_code;
            unsigned long buf_add;
            unsigned long ret_add;
        } item_desc;

        struct dsc$descriptor_s lognam_dsc, tabnam_dsc;
        item_desc item_list[129];
        unsigned long attr, index;
        int status;


        /* Set up the inputs for the creation.
        */
        attr = 0;

        lognam_dsc.dsc$a_pointer = lognam;
        lognam_dsc.dsc$b_dtype   = DSC$K_DTYPE_T;
        lognam_dsc.dsc$w_length  = strlen(lognam);
        lognam_dsc.dsc$b_class   = DSC$K_CLASS_S;

        tabnam_dsc.dsc$a_pointer = tabnam;
        tabnam_dsc.dsc$b_dtype   = DSC$K_DTYPE_T;
        tabnam_dsc.dsc$w_length  = strlen(tabnam);
        tabnam_dsc.dsc$b_class   = DSC$K_CLASS_S;

        for (index=0; index < trnsl_num; index++) {
            item_list[index].buf_add   = trnsl_table[index];
            item_list[index].buf_len   = strlen(trnsl_table[index]);
            item_list[index].item_code = LNM$_STRING;
        }

        item_list[trnsl_num].buf_add   = 0;
        item_list[trnsl_num].buf_len   = 0;
        item_list[trnsl_num].item_code = 0;

        /* Create the logical name
        */
        if (access_mode == PSL$C_SUPER) {
            status = LIB$SET_LOGICAL(&lognam_dsc,
                           0,
                           &tabnam_dsc,
                           0,
                           &item_list);
        }
        else {
            status = SYS$CRELNM(&attr,
                           &tabnam_dsc,
                           &lognam_dsc,
                           &access_mode,
                           &item_list);
        }


        /* Return a normalized status.
        */
        if ((status != SS$_NORMAL) && (status != SS$_SUPERSEDE)) {
            return (0);
        }
        else return (1);
}
#endif /* VMS */



/*
**++
**  ROUTINE NAME: DWI18n_GetXLFDCharSet
**
**  FUNCTIONAL DESCRIPTION:
**	This routine returns a character set part from the font name
**	specified with XLFD.
**
**  FORMAL PARAMETERS:
**    fontname  -      The font name specified with XLFD.
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

char *DWI18n_GetXLFDCharSet(fontname)
char *fontname;
{
    int 	i = 0;
    char	*buf;

    while (*fontname){
	if (*fontname == '-'){
	    fontname++;
	    i++;
	    if (i == 13)
	      {
		buf = NULL;
		buf = XtMalloc((Cardinal)(strlen(fontname)+1));
		if (!buf)
		  {
			return 0;
		  }
		strcpy (buf, fontname);
		return (buf);
	      }
	} else
	    fontname++;
    }
    return 0;
}


/*
**++
**  ROUTINE NAME: DWI18n_IsXnlLangISOLatin1
**
**  FUNCTIONAL DESCRIPTION:
**	This routine indicates whether a code set is ISO_Latin1.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**	True,  if the code set is ISO_Latin1
**	False, if the code set is not ISO_Latin1
**
**  SIDE EFFECTS:
**--
**/

Boolean	DWI18n_IsXnlLangISOLatin1()
{
  MapLangIfNeeded();

  if ((language_id & MASK_CODESET_AREA) == ISOLATIN1)
	return(TRUE);
  else
	return(FALSE);
}


/*
**++
**  ROUTINE NAME: _I18n_InvertString
**
**  FUNCTIONAL DESCRIPTION:
**	This routine inverts the string.
**
**  FORMAL PARAMETERS:
**      str 	- (I/O) A pointer to the string to be inverted
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

static void
_I18n_InvertString (str)
  char		*str;
{
  int	head, tail;
  char	buf;

  head = 0;
  tail = strlen(str) - 1;

  while (tail > head)
     {
      	buf = str[head];
	str[head] = str[tail];
	str[tail] = buf;
	head++;
	tail--;
     }
}



/*
**++
**  ROUTINE NAME: DWI18n_SetIconName
**
**  FUNCTIONAL DESCRIPTION:
**	This routine sets the icon name. If it contains iso-latin1 only,
**      the encoding will be STRING.  Otherwise, it will be COMPOUND_TEXT
**      encoding.
**
**  FORMAL PARAMETERS:
**      widget  - (I) The widget in which the current icon name is to be
**		      changed.
**      cs_icon - (I) A compound string to be converted and be set as the icon
**		      name
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

void DWI18n_SetIconName (widget, cs_icon)
  Widget	widget;
  XmString	cs_icon;
{
  char			*iconname,*icon_os=NULL;
  char			*encoding;
  long			byte_count;
  long			cvt_status;
  Arg			arglist[2];

  if (!cs_icon)
      return;

  MapLangIfNeeded();

  if (DXmCSContainsStringCharSet(cs_icon))	/* See if ISO_Latin1 Only */
    {
	encoding = "STRING";
	iconname = (char *) DXmCvtCStoFC(cs_icon, &byte_count, &cvt_status);
	if (cvt_status == DXmCvtStatusFail)
	  {
		iconname = NULL;
	  }
    }						/* See if Hebrew	  */
  else if ((language_id & MASK_LANGUAGE_TERRITORY_AREA) == HEBREW)
    {						/* For Hebrew		  */
  	if (DXIsXUIWMRunning(widget,FALSE))
	    {
		encoding = "STRING";
		iconname = DXmCvtCStoFC(cs_icon, &byte_count, &cvt_status);
		if (cvt_status == DXmCvtStatusFail)
		  {
		      iconname = NULL;
		  }
		else 
		  {
		      icon_os = DXmCvtCStoOS(cs_icon, &byte_count, &cvt_status);
		      if (icon_os != NULL) XmStringFree(icon_os);
		      if (cvt_status != DXmCvtStatusOK)
		  	{
			     _I18n_InvertString(iconname);                   
		  	}
		  }
	    }
	else
	    {
		encoding = "COMPOUND_TEXT";
		iconname = (char *) XmCvtXmStringToCT(cs_icon);
	    }
    }
  else						/* For other locales	  */
    {
	encoding = "COMPOUND_TEXT";
	iconname = (char *) XmCvtXmStringToCT(cs_icon);
    }	  

  if (iconname)
    {
	  XtSetArg(arglist[0], XmNiconName, iconname);
	  XtSetArg(arglist[1], XmNiconNameEncoding,
		   XmInternAtom(XtDisplay(widget), encoding, FALSE));
	  XtSetValues (widget, arglist, 2);
	  XtFree((char *) iconname);
    }
}



/*
**++
**  ROUTINE NAME: DWI18n_SetTitle
**
**  FUNCTIONAL DESCRIPTION:
**	This routine sets the window title.  If it contains iso-latin1 only,
**      the encoding will be STRING.  Otherwise, it will be COMPOUND_TEXT
**      encoding.
**
**  FORMAL PARAMETERS:
**      widget   - (I) The widget in which the current window title is to be
**		       changed.
**      cs_title - (I) A compound string to be converted and be set as the
**		       window title
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

void DWI18n_SetTitle (widget, cs_title)
  Widget	widget;
  XmString	cs_title;
{
  char			*title,*title_os=NULL;
  char			*encoding;
  long			byte_count;
  long			cvt_status;
  Arg			arglist[2];

  if (!cs_title)
      return;

  MapLangIfNeeded();

  if (DXmCSContainsStringCharSet(cs_title))	/* See if ISO_Latin1 only */
    {
	encoding = "STRING";
	title = (char *) DXmCvtCStoFC(cs_title, &byte_count, &cvt_status);
	if (cvt_status == DXmCvtStatusFail)
	  {
		title = NULL;
	  }
    }						/* See if Hebrew	  */
  else if ((language_id & MASK_LANGUAGE_TERRITORY_AREA) == HEBREW)
    {						/* For Hebrew		  */
  	if (DXIsXUIWMRunning(widget,FALSE))
	    {
		encoding = "STRING";
		title = DXmCvtCStoFC(cs_title, &byte_count, &cvt_status);
		if (cvt_status == DXmCvtStatusFail)
		  {
		     title = NULL;
		  }
		else 
		  {
		     title_os = DXmCvtCStoOS(cs_title, &byte_count, &cvt_status);
		     if (title_os != NULL) XmStringFree(title_os);
		     if (cvt_status != DXmCvtStatusOK)
		  	{
			    _I18n_InvertString(title);
		  	}
		   }
	    }
	else
	    {
		encoding = "COMPOUND_TEXT";
		title = (char *) XmCvtXmStringToCT(cs_title);
	    }
    }
  else					       /* For other locales	  */
    {
	encoding = "COMPOUND_TEXT";
	title = (char *) XmCvtXmStringToCT(cs_title);
    }	  

  if (title)
    {
	  XtSetArg(arglist[0], XmNtitle, title);
	  XtSetArg(arglist[1], XmNtitleEncoding,
		   XmInternAtom(XtDisplay(widget), encoding, FALSE));
	  XtSetValues (widget, arglist, 2);
	  XtFree((char *) title);
    }
}



/*
**++
**  ROUTINE NAME: DWI18n_ToLower
**
**  FUNCTIONAL DESCRIPTION:
**	This routine converts a string of upper-case characters to lower-case
**	according to the code set.
**	The user must free the string by using the Intrinsic routine XtFree.
**
**  FORMAL PARAMETERS:
**	str -	(I) A pointer to the input string.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**	  buffer to return lower-case string in.
**
**  SIDE EFFECTS:
**--
**/

char *DWI18n_ToLower(str)
char *str;
{
char	*src_str, *dst_str, *buf;
int	len;

    MapLangIfNeeded();

    buf = NULL;				/* Allocate mem. to store converted */
    buf = XtMalloc((Cardinal)(strlen(str)+1));	/* string.		    */
    if (!buf)
      {
	return (NULL);
      }

    src_str = str;
    dst_str = buf;

    switch (language_id & MASK_CODESET_AREA)
    {
	case DECHANZI:
	    while (*src_str) {
		ByteLengthOfCharForDECHanzi(src_str, len);
		if (len == 1){
			*dst_str = tolower(*src_str);
			src_str++;
			dst_str++;
	          }
		else {
		    while (len > 0) {
				*dst_str = *src_str;
				len--;
				src_str++;
				dst_str++;
			}
		  }
    	    }
	    break;
	case DECKOREAN:
	    while (*src_str) {
		ByteLengthOfCharForDECHanzi(src_str, len);
		if (len == 1){
			*dst_str = tolower(*src_str);
			src_str++;
			dst_str++;
	          }
		else {
		    while (len > 0) {
				*dst_str = *src_str;
				len--;
				src_str++;
				dst_str++;
			}
		  }
    	    }
	    break;
	case DECHANYU:
	    while (*src_str) {
		ByteLengthOfCharForDECHanzi(src_str, len);
		if (len == 1){
			*dst_str = tolower(*src_str);
			src_str++;
			dst_str++;
	          }
		else {
		    while (len > 0) {
				*dst_str = *src_str;
				len--;
				src_str++;
				dst_str++;
			}
		  }
    	    }
	    break;
        case DECHEBREW:
	    while (*src_str) {
 		if (*src_str & MASK_MSB_OF_CHAR_CODE)
 		    *dst_str = *src_str;
		else
	            *dst_str = tolower(*src_str);
      		src_str++;
      		dst_str++;
    	    }
	    break;
	case DECKANJI:
	    while (*src_str) {
		ByteLengthOfCharForDECHanzi(src_str, len);
		if (len == 1){
			*dst_str = tolower(*src_str);
			src_str++;
			dst_str++;
	          }
		else {
		    while (len > 0) {
				*dst_str = *src_str;
				len--;
				src_str++;
				dst_str++;
			}
		  }
    	    }
	    break;
	default:
	    while (*src_str) {
	        *dst_str = tolower(*src_str);
      		src_str++;
      		dst_str++;
    	    }
	    break;
    }
    *dst_str = '\0';

    return (buf);
}


/*
**++
**  ROUTINE NAME: DWI18n_ToUpper
**
**  FUNCTIONAL DESCRIPTION:
**	This routine converts a string of lower-case characters to upper-case
**	according to the code set.
**	The user must free the string by using the Intrinsic routine XtFree.
**
**  FORMAL PARAMETERS:
**	str -	(I) A pointer to the input string.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**	  buffer to return upper-case string in.
**
**  SIDE EFFECTS:
**--
**/

char *DWI18n_ToUpper(str)
char *str;
{
char	*src_str, *dst_str, *buf;
int	len;

    MapLangIfNeeded();

    buf = NULL;				/* Allocate mem. to store converted */
    buf = XtMalloc((Cardinal)(strlen(str)+1));	/* string.		    */
    if (!buf)
      {
	return (NULL);
      }

    src_str = str;
    dst_str = buf;

    switch (language_id & MASK_CODESET_AREA)
    {
	case DECHANZI:
	    while (*src_str) {
		ByteLengthOfCharForDECHanzi(src_str, len);
		if (len == 1){
			*dst_str = toupper(*src_str);
			src_str++;
			dst_str++;
	          }
		else {
		    while (len > 0) {
				*dst_str = *src_str;
				len--;
				src_str++;
				dst_str++;
			}
		  }
    	    }
	    break;
	case DECKOREAN:
	    while (*src_str) {
		ByteLengthOfCharForDECHanzi(src_str, len);
		if (len == 1){
			*dst_str = toupper(*src_str);
			src_str++;
			dst_str++;
	          }
		else {
		    while (len > 0) {
				*dst_str = *src_str;
				len--;
				src_str++;
				dst_str++;
			}
		  }
    	    }
	    break;
	case DECHANYU:
	    while (*src_str) {
		ByteLengthOfCharForDECHanzi(src_str, len);
		if (len == 1){
			*dst_str = toupper(*src_str);
			src_str++;
			dst_str++;
	          }
		else {
		    while (len > 0) {
				*dst_str = *src_str;
				len--;
				src_str++;
				dst_str++;
			}
		  }
    	    }
	    break;
        case DECHEBREW:
	    while (*src_str) {
 		if (*src_str & MASK_MSB_OF_CHAR_CODE)
 		    *dst_str = *src_str;
		else
	            *dst_str = toupper(*src_str);
      		src_str++;
      		dst_str++;
    	    }
	    break;
	case DECKANJI:
	    while (*src_str) {
		ByteLengthOfCharForDECHanzi(src_str, len);
		if (len == 1){
			*dst_str = toupper(*src_str);
			src_str++;
			dst_str++;
	          }
		else {
		    while (len > 0) {
				*dst_str = *src_str;
				len--;
				src_str++;
				dst_str++;
			}
		  }
    	    }
	    break;
	default:
	    while (*src_str) {
	        *dst_str = toupper(*src_str);
      		src_str++;
      		dst_str++;
    	    }
	    break;
    }
    *dst_str = '\0';

    return (buf);
}


/*
**++
**  ROUTINE NAME: DWI18n_RebindIrreqularKeysym
**
**  FUNCTIONAL DESCRIPTION:
**	This routine switches the language specific KeySyms to the System
**	Default KeySym(ASCII).
**
**  FORMAL PARAMETERS:
**	dpy	    - (I) A pointer to Display structure.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**
** FACILITY: Common -- can be utilized by any applications which do not
**                     expect language specfic keysyms.
**
**           (Currently, Login Extension & Pause Session make use of this
**            this routine.)
**
** ABSTRACT:
**
**      Rebind irregular KeySyms to ASCII KeySym.
**
**      Today, some of keyboards, for instance Kana Keyboard, have a
**      locking-shift-key used for typing a native language.
**      From X11R4, Mode-switch mechanism was introduced as a part of I18n
**      to support country specific keyboards. Basically, Mode-switch is
**      assigned to the locking-shift-key. As an operator presses the
**      locking-shift-key once, the status of Mode-switch changes to ON,
**      and XLookupString returns country specific KeySyms. In general, this
**      works out fine, but in case of LOGIN windows/Pause session Window,
**      User name & Password must be ASCII, and no country specific keysyms
**      are allowed. If an operator presses locking-shift-key accidentally,
**      he/she cannot enter password, at all. Since LK401 has no LED of
**      Compose/Kana, there is no way for an operator to know the current
**      status of keyboard, especially in the Pause session window.
**      Therefore, this routine rebinds such kind of irregular (country
**      specific) keysyms to correspoding ASCII characters so that
**      an operator can type keys regardless of locking-shift.
**
**
** ENVIRONMENT:
**
**      VAX/VMS operating system.
**
** AUTHOR:  E.A., July 1991
**
** MODIFICATION HISTORY:
**
**
**
*/


static unsigned long kana_to_ascii_tbl1[] = {   /* Non-Shift state */

        XK_prolongedsound,           0x60,  /* `          0xb0, */
        XK_kana_NU,                  0x31,  /* 1          0xc7, */
        XK_kana_FU,                  0x32,  /* 2          0xcc, */
     /* XK_kana_HU,                  0x32,     2          0xcc, ***** deprecated***** */
        XK_kana_A,                   0x33,  /* 3          0xb1, */
        XK_kana_U,                   0x34,  /* 4          0xb3, */
        XK_kana_E,                   0x35,  /* 5          0xb4, */
        XK_kana_O,                   0x36,  /* 6          0xb5, */
        XK_kana_YA,                  0x37,  /* 7          0xd4, */
        XK_kana_YU,                  0x38,  /* 8          0xd5, */
        XK_kana_YO,                  0x39,  /* 9          0xd6, */
        XK_kana_WA,                  0x30,  /* 0          0xdc, */
        XK_kana_HO,                  0x2D,  /* -          0xce, */
        XK_kana_HE,                  0x3D,  /* =          0xcd, */
        XK_kana_TA,                  0x71,  /* q          0xc0, */
        XK_kana_TE,                  0x77,  /* w          0xc3, */
        XK_kana_I,                   0x65,  /* e          0xb2, */
        XK_kana_SU,                  0x72,  /* r          0xbd, */
        XK_kana_KA,                  0x74,  /* t          0xb6, */
        XK_kana_N,                   0x79,  /* y          0xdd, */
        XK_kana_NA,                  0x75,  /* u          0xc5, */
        XK_kana_NI,                  0x69,  /* i          0xc6, */
        XK_kana_RA,                  0x6F,  /* o          0xd7, */
        XK_kana_SE,                  0x70,  /* p          0xbe, */
        XK_kana_RO,                  0x5B,  /* [          0xdb, */
        XK_kana_MU,                  0x5D,  /* ]          0xd1, */
        XK_kana_CHI,                 0x61,  /* a          0xc1, */
     /* XK_kana_TI,                  0x61,     a          0xc1, ***** deprecated***** */
        XK_kana_TO,                  0x73,  /* s          0xc4, */
        XK_kana_SHI,                 0x64,  /* d          0xbc, */
        XK_kana_HA,                  0x66,  /* f          0xca, */
        XK_kana_KI,                  0x67,  /* g          0xb7, */
        XK_kana_KU,                  0x68,  /* h          0xb8, */
        XK_kana_MA,                  0x6A,  /* j          0xcf, */
        XK_kana_NO,                  0x6B,  /* k          0xc9, */
        XK_kana_RI,                  0x6C,  /* l          0xd8, */
        XK_kana_RE,                  0x3B,  /* ;          0xda, */
        XK_kana_KE,                  0x27,  /* '          0xb9, */
        XK_kana_openingbracket,      0x5C,  /* \          0xa2, */
        XK_kana_TSU,                 0x7A,  /* z          0xc2, */
     /* XK_kana_TU,                  0x7A,     z          0xc2, ***** deprecated***** */
        XK_kana_SA,                  0x78,  /* x          0xbb, */
        XK_kana_SO,                  0x63,  /* c          0xbf, */
        XK_kana_HI,                  0x76,  /* v          0xcb, */
        XK_kana_KO,                  0x62,  /* b          0xba, */
        XK_kana_MI,                  0x6E,  /* n          0xd0, */
        XK_kana_MO,                  0x6D,  /* m          0xd3, */
        XK_kana_NE,                  0x2C,  /* ,          0xc8, */
        XK_kana_RU,                  0x2E,  /* .          0xd9, */
        XK_kana_ME,                  0x2F,  /* /          0xd2, */
        XK_overline,                 0x7E,  /* ~          0x7e, */

        0,                              0   /*** End of Table ***/
};

static unsigned long kana_to_ascii_tbl2[] = {   /* Caps Lock state */

        XK_prolongedsound,           0x60,  /* `          0xb0, */
        XK_kana_NU,                  0x31,  /* 1          0xc7, */
        XK_kana_FU,                  0x32,  /* 2          0xcc, */
     /* XK_kana_HU,                  0x32,     2          0xcc, ***** deprecated***** */
        XK_kana_A,                   0x33,  /* 3          0xb1, */
        XK_kana_U,                   0x34,  /* 4          0xb3, */
        XK_kana_E,                   0x35,  /* 5          0xb4, */
        XK_kana_O,                   0x36,  /* 6          0xb5, */
        XK_kana_YA,                  0x37,  /* 7          0xd4, */
        XK_kana_YU,                  0x38,  /* 8          0xd5, */
        XK_kana_YO,                  0x39,  /* 9          0xd6, */
        XK_kana_WA,                  0x30,  /* 0          0xdc, */
        XK_kana_HO,                  0x2D,  /* -          0xce, */
        XK_kana_HE,                  0x3D,  /* =          0xcd, */
        XK_kana_TA,                  0x51,  /* Q          0xc0, */
        XK_kana_TE,                  0x57,  /* W          0xc3, */
        XK_kana_i,                   0x45,  /* E          0xa8, */
        XK_kana_I,                   0x45,  /* E          0xb2, */
        XK_kana_SU,                  0x52,  /* R          0xbd, */
        XK_kana_KA,                  0x54,  /* T          0xb6, */
        XK_kana_N,                   0x59,  /* Y          0xdd, */
        XK_kana_NA,                  0x55,  /* U          0xc5, */
        XK_kana_NI,                  0x49,  /* I          0xc6, */
        XK_kana_RA,                  0x4F,  /* O          0xd7, */
        XK_kana_SE,                  0x50,  /* P          0xbe, */
        XK_kana_RO,                  0x5B,  /* [          0xdb, */
        XK_kana_MU,                  0x5D,  /* ]          0xd1, */
        XK_kana_CHI,                 0x41,  /* A          0xc1, */
     /* XK_kana_TI,                  0x41,     A          0xc1, ***** deprecated***** */
        XK_kana_TO,                  0x53,  /* S          0xc4, */
        XK_kana_SHI,                 0x44,  /* D          0xbc, */
        XK_kana_HA,                  0x46,  /* F          0xca, */
        XK_kana_KI,                  0x47,  /* G          0xb7, */
        XK_kana_KU,                  0x48,  /* H          0xb8, */
        XK_kana_MA,                  0x4A,  /* J          0xcf, */
        XK_kana_NO,                  0x4B,  /* K          0xc9, */
        XK_kana_RI,                  0x4C,  /* L          0xd8, */
        XK_kana_RE,                  0x3B,  /* ;          0xda, */
        XK_kana_KE,                  0x27,  /* '          0xb9, */
        XK_kana_openingbracket,      0x5C,  /* \          0xa2, */
        XK_kana_tsu,                 0x5A,  /* Z          0xaf, */
     /* XK_kana_tu,                  0x5A,     Z          0xaf, ***** deprecated***** */
        XK_kana_TSU,                 0x5A,  /* Z          0xc2, */
     /* XK_kana_TU,                  0x5A,     Z          0xc2, ***** deprecated***** */
        XK_kana_SA,                  0x58,  /* X          0xbb, */
        XK_kana_SO,                  0x43,  /* C          0xbf, */
        XK_kana_HI,                  0x56,  /* V          0xcb, */
        XK_kana_KO,                  0x42,  /* B          0xba, */
        XK_kana_MI,                  0x4E,  /* N          0xd0, */
        XK_kana_MO,                  0x4D,  /* M          0xd3, */
        XK_kana_NE,                  0x2C,  /* ,          0xc8, */
        XK_kana_RU,                  0x2E,  /* .          0xd9, */
        XK_kana_ME,                  0x2F,  /* /          0xd2, */
        XK_overline,                 0x7E,  /* ~          0x7e, */

        0,                              0   /*** End of Table ***/
};

static unsigned long kana_to_ascii_tbl3[] = {  /* Shift state */

        XK_prolongedsound,           0x7E,  /* ~          0xb0, */
        XK_kana_NU,                  0x21,  /* !          0xc7, */
        XK_kana_FU,                  0x40,  /* @          0xcc, */
     /* XK_kana_HU,                  0x40,     @          0xcc, ***** deprecated***** */
        XK_kana_a,                   0x23,  /* #          0xa7, */
        XK_kana_u,                   0x24,  /* $          0xa9, */
        XK_kana_e,                   0x25,  /* %          0xaa, */
        XK_kana_o,                   0x5E,  /* ^          0xab, */
        XK_kana_ya,                  0x26,  /* &          0xac, */
        XK_kana_yu,                  0x2A,  /* *          0xad, */
        XK_kana_yo,                  0x28,  /* (          0xae, */
        XK_kana_WO,                  0x29,  /* )          0xa6, */
        XK_kana_HO,                  0x5F,  /* _          0xce, */
        XK_kana_HE,                  0x2B,  /* +          0xcd, */
        XK_kana_TA,                  0x51,  /* Q          0xc0, */
        XK_kana_TE,                  0x57,  /* W          0xc3, */
        XK_kana_i,                   0x45,  /* E          0xa8, */
        XK_kana_I,                   0x45,  /* E          0xb2, */
        XK_kana_SU,                  0x52,  /* R          0xbd, */
        XK_kana_KA,                  0x54,  /* T          0xb6, */
        XK_kana_N,                   0x59,  /* Y          0xdd, */
        XK_kana_NA,                  0x55,  /* U          0xc5, */
        XK_kana_NI,                  0x49,  /* I          0xc6, */
        XK_kana_RA,                  0x4F,  /* O          0xd7, */
        XK_kana_SE,                  0x50,  /* P          0xbe, */
        XK_voicedsound,              0x7B,  /* {          0xde, */
        XK_semivoicedsound,          0x7D,  /* }          0xdf, */
        XK_kana_CHI,                 0x41,  /* A          0xc1, */
     /* XK_kana_TI,                  0x41,     A          0xc1, ***** deprecated***** */
        XK_kana_TO,                  0x53,  /* S          0xc4, */
        XK_kana_SHI,                 0x44,  /* D          0xbc, */
        XK_kana_HA,                  0x46,  /* F          0xca, */
        XK_kana_KI,                  0x47,  /* G          0xb7, */
        XK_kana_KU,                  0x48,  /* H          0xb8, */
        XK_kana_MA,                  0x4A,  /* J          0xcf, */
        XK_kana_NO,                  0x4B,  /* K          0xc9, */
        XK_kana_RI,                  0x4C,  /* L          0xd8, */
        XK_kana_RE,                  0x3A,  /* :          0xda, */
        XK_kana_KE,                  0x22,  /* "          0xb9, */
        XK_kana_closingbracket,      0x7C,  /* |          0xa3, */
        XK_kana_tsu,                 0x5A,  /* Z          0xaf, */
     /* XK_kana_tu,                  0x5A,     Z          0xaf, ***** deprecated***** */
        XK_kana_TSU,                 0x5A,  /* Z          0xc2, */
     /* XK_kana_TU,                  0x5A,     Z          0xc2, ***** deprecated***** */
        XK_kana_SA,                  0x58,  /* X          0xbb, */
        XK_kana_SO,                  0x43,  /* C          0xbf, */
        XK_kana_HI,                  0x56,  /* V          0xcb, */
        XK_kana_KO,                  0x42,  /* B          0xba, */
        XK_kana_MI,                  0x4E,  /* N          0xd0, */
        XK_kana_MO,                  0x4D,  /* M          0xd3, */
        XK_kana_comma,               0x2C,  /* , or <     0xa4, */
        XK_kana_fullstop,            0x2E,  /* . or >     0xa1, */
        XK_kana_conjunctive,         0x3F,  /* ?          0xa5, */
     /* XK_kana_middledot,           0x3F,     ?          0xa5, ***** deprecated***** */
        XK_overline,                 0x7E,  /* ~          0x7e, */

        0,                              0   /*** End of Table ***/
};



static unsigned long hebrew_to_ascii_tbl1[] = {   /* Non-Shift state */

        XK_hebrew_aleph,             0x74,  /* t          0xE0  */
        XK_hebrew_bet,               0x63,  /* c          0xE1  */
     /* XK_hebrew_beth,              0x63,  /* c          0xE1  ****** deprecated***** */
        XK_hebrew_gimel,             0x64,  /* d          0xE2  */
     /* XK_hebrew_gimmel,            0x64,  /* d          0xE2  ****** deprecated***** */
        XK_hebrew_dalet,             0x73,  /* s          0xE2  */
     /* XK_hebrew_daleth,            0x73,  /* s          0xE2  ****** deprecated***** */
        XK_hebrew_he,                0x76,  /* v          0xE4  */
        XK_hebrew_waw,               0x75,  /* u          0xE5  */
        XK_hebrew_zain,              0x7A,  /* z          0xE6  */
     /* XK_hebrew_zayin,             0x7A,  /* z          0xE6  ****** deprecated***** */
        XK_hebrew_chet,              0x6A,  /* j          0xE7  */
     /* XK_hebrew_het,               0x6A,  /* j          0xE7  ****** deprecated***** */
        XK_hebrew_tet,               0x79,  /* y          0xE8  */
     /* XK_hebrew_teth,              0x79,  /* y          0xE8  ****** deprecated***** */
        XK_hebrew_yod,               0x68,  /* h          0xE9  */
        XK_hebrew_finalkaph,         0x6C,  /* l          0xEA  */
        XK_hebrew_kaph,              0x66,  /* f          0xEB  */
        XK_hebrew_lamed,             0x6B,  /* k          0xEC  */
        XK_hebrew_finalmem,          0x6F,  /* o          0xED  */
        XK_hebrew_mem,               0x6E,  /* n          0xEE  */
        XK_hebrew_finalnun,          0x69,  /* i          0xEF  */
        XK_hebrew_nun,               0x62,  /* b          0xF0  */
        XK_hebrew_samech,            0x78,  /* x          0xF1  */
     /* XK_hebrew_samekh,            0x78,  /* x          0xF1  ****** deprecated***** */
        XK_hebrew_ayin,              0x67,  /* g          0xF2  */
        XK_hebrew_finalpe,           0x3B,  /* ;          0xF3  */
        XK_hebrew_pe,                0x70,  /* p          0xF4  */
        XK_hebrew_finalzade,         0x2E,  /* .          0xF5  */
     /* XK_hebrew_finalzadi,         0x2E,  /* .          0xF5  ****** deprecated***** */
        XK_hebrew_zade,              0x6D,  /* m          0xF6  */
     /* XK_hebrew_zadi,              0x6D,  /* m          0xF6  ****** deprecated***** */
        XK_hebrew_qoph,              0x65,  /* e          0xF7  */
     /* XK_hebrew_kuf,               0x65,  /* e          0xF7  ****** deprecated***** */
        XK_hebrew_resh,              0x72,  /* r          0xF8  */
        XK_hebrew_shin,              0x61,  /* a          0xF9  */
        XK_hebrew_taw,               0x2C,  /* ,          0xFA  */
     /* XK_hebrew_taf,               0x2C,  /* ,          0xFA  ****** deprecated***** */
        XK_apostrophe,		     0x77,  /* w	  0x27  */
        XK_period,		     0x2F,  /* /          0x2e  */
	XK_semicolon,		     0x5C,  /* \	  0x3b  */
	XK_comma,		     0x27,  /* '	  0x2C  */
	XK_slash,		     0x60,  /* `	  0x2F  */
     /* XK_VoidSymbol,               0x71,  /* q      0xFFFFFF  ** Not rebindable?*/
	0x10FFFF00,                  0x71,  /* q    0x10FFFF00	HACK for above */  
        0,                              0   /*** End of Table ***/
};

static unsigned long hebrew_to_ascii_tbl2[] = {   /* Caps Lock state */

        XK_hebrew_aleph,             0x54,  /* T          0xE0  */
        XK_hebrew_bet,               0x43,  /* C          0xE1  */
     /* XK_hebrew_beth,              0x43,  /* C          0xE1  ****** deprecated***** */
        XK_hebrew_gimel,             0x44,  /* D          0xE2  */
     /* XK_hebrew_gimmel,            0x44,  /* D          0xE2  ****** deprecated***** */
        XK_hebrew_dalet,             0x53,  /* S          0xE2  */
     /* XK_hebrew_daleth,            0x53,  /* S          0xE2  ****** deprecated***** */
        XK_hebrew_he,                0x56,  /* V          0xE4  */
        XK_hebrew_waw,               0x55,  /* U          0xE5  */
        XK_hebrew_zain,              0x5A,  /* Z          0xE6  */
     /* XK_hebrew_zayin,             0x5A,  /* Z          0xE6  ****** deprecated***** */
        XK_hebrew_chet,              0x4A,  /* J          0xE7  */
     /* XK_hebrew_het,               0x4A,  /* J          0xE7  ****** deprecated***** */
        XK_hebrew_tet,               0x59,  /* Y          0xE8  */
     /* XK_hebrew_teth,              0x59,  /* Y          0xE8  ****** deprecated***** */
        XK_hebrew_yod,               0x48,  /* H          0xE9  */
        XK_hebrew_finalkaph,         0x4C,  /* L          0xEA  */
        XK_hebrew_kaph,              0x46,  /* F          0xEB  */
        XK_hebrew_lamed,             0x4B,  /* K          0xEC  */
        XK_hebrew_finalmem,          0x4F,  /* O          0xED  */
        XK_hebrew_mem,               0x4E,  /* N          0xEE  */
        XK_hebrew_finalnun,          0x49,  /* I          0xEF  */
        XK_hebrew_nun,               0x42,  /* B          0xF0  */
        XK_hebrew_samech,            0x58,  /* X          0xF1  */
     /* XK_hebrew_samekh,            0x58,  /* X          0xF1  ****** deprecated***** */
        XK_hebrew_ayin,              0x47,  /* G          0xF2  */
        XK_hebrew_finalpe,           0x3B,  /* ;          0xF3  */
        XK_hebrew_pe,                0x50,  /* P          0xF4  */
        XK_hebrew_finalzade,         0x2E,  /* .          0xF5  */
     /* XK_hebrew_finalzadi,         0x2E,  /* .          0xF5  ****** deprecated***** */
        XK_hebrew_zade,              0x4D,  /* M          0xF6  */
     /* XK_hebrew_zadi,              0x4D,  /* M          0xF6  ****** deprecated***** */
        XK_hebrew_qoph,              0x45,  /* E          0xF7  */
     /* XK_hebrew_kuf,               0x45,  /* E          0xF7  ****** deprecated***** */
        XK_hebrew_resh,              0x52,  /* R          0xF8  */
        XK_hebrew_shin,              0x41,  /* A          0xF9  */
        XK_hebrew_taw,               0x2C,  /* ,          0xFA  */
     /* XK_hebrew_taf,               0x2C,  /* ,          0xFA  ****** deprecated***** */
        XK_period,		     0x2F,  /* /          0x2e  */
	XK_semicolon,		     0x5C,  /* \	  0x3b  */
	XK_comma,		     0x27,  /* '	  0x2C  */
	XK_slash,		     0x60,  /* `	  0x2F  */

        0,                              0   /*** End of Table ***/
};

static unsigned long hebrew_to_ascii_tbl3[] = {  /* Shift state */

        XK_hebrew_aleph,             0x54,  /* T          0xE0  */
        XK_hebrew_bet,               0x43,  /* C          0xE1  */
     /* XK_hebrew_beth,              0x43,  /* C          0xE1  ****** deprecated***** */
        XK_hebrew_gimel,             0x44,  /* D          0xE2  */
     /* XK_hebrew_gimmel,            0x44,  /* D          0xE2  ****** deprecated***** */
        XK_hebrew_dalet,             0x53,  /* S          0xE2  */
     /* XK_hebrew_daleth,            0x53,  /* S          0xE2  ****** deprecated***** */
        XK_hebrew_he,                0x56,  /* V          0xE4  */
        XK_hebrew_waw,               0x55,  /* U          0xE5  */
        XK_hebrew_zain,              0x5A,  /* Z          0xE6  */
     /* XK_hebrew_zayin,             0x5A,  /* Z          0xE6  ****** deprecated***** */
        XK_hebrew_chet,              0x4A,  /* J          0xE7  */
     /* XK_hebrew_het,               0x4A,  /* J          0xE7  ****** deprecated***** */
        XK_hebrew_tet,               0x59,  /* Y          0xE8  */
     /* XK_hebrew_teth,              0x59,  /* Y          0xE8  ****** deprecated***** */
        XK_hebrew_yod,               0x48,  /* H          0xE9  */
        XK_hebrew_finalkaph,         0x4C,  /* L          0xEA  */
        XK_hebrew_kaph,              0x46,  /* F          0xEB  */
        XK_hebrew_lamed,             0x4B,  /* K          0xEC  */
        XK_hebrew_finalmem,          0x4F,  /* O          0xED  */
        XK_hebrew_mem,               0x4E,  /* N          0xEE  */
        XK_hebrew_finalnun,          0x49,  /* I          0xEF  */
        XK_hebrew_nun,               0x42,  /* B          0xF0  */
        XK_hebrew_samech,            0x58,  /* X          0xF1  */
     /* XK_hebrew_samekh,            0x58,  /* X          0xF1  ****** deprecated***** */
        XK_hebrew_ayin,              0x47,  /* G          0xF2  */
        XK_hebrew_finalpe,           0x3A,  /* :          0xF3  */
        XK_hebrew_pe,                0x50,  /* P          0xF4  */
        XK_hebrew_finalzade,         0x2E,  /* .          0xF5  */
     /* XK_hebrew_finalzadi,         0x2E,  /* .          0xF5  ****** deprecated***** */
        XK_hebrew_zade,              0x4D,  /* M          0xF6  */
     /* XK_hebrew_zadi,              0x4D,  /* M          0xF6  ****** deprecated***** */
        XK_hebrew_qoph,              0x45,  /* E          0xF7  */
     /* XK_hebrew_kuf,               0x45,  /* E          0xF7  ****** deprecated***** */
        XK_hebrew_resh,              0x52,  /* R          0xF8  */
        XK_hebrew_shin,              0x41,  /* A          0xF9  */
        XK_hebrew_taw,               0x2C,  /* ,          0xFA  */
     /* XK_hebrew_taf,               0x2C,  /* ,          0xFA  ****** deprecated***** */

        0,                              0   /*** End of Table ***/
};



static KeySym modlist_tbl1_m1_a[] = {  /* Non-Shift */
        XK_Mode_switch
};

static KeySym modlist_tbl2_m2_a[] = {  /* Caps-Lock */
        XK_Mode_switch,
        XK_Caps_Lock
};

static KeySym modlist_tbl3_m3_a[] = {  /* Shift */
        XK_Mode_switch,
        XK_Caps_Lock,
        XK_Shift_L
};

static KeySym modlist_tbl3_m3_b[] = {  /* Shift */
        XK_Mode_switch,
        XK_Caps_Lock,
        XK_Shift_R
};

static KeySym modlist_tbl3_m2_c[] = {  /* Shift */
        XK_Mode_switch,
        XK_Shift_L
};

static KeySym modlist_tbl3_m2_d[] = {  /* Shift */
        XK_Mode_switch,
        XK_Shift_R
};

static KeySym modlist_tbl3_m2_e[] = {  /* Shift */
        XK_Mode_switch,
        XK_Shift_Lock
};

static KeySym modlist_tbl3_m3_f[] = {  /* Shift */
        XK_Mode_switch,
        XK_Shift_Lock,
        XK_Shift_L
};

static KeySym modlist_tbl3_m3_g[] = {  /* Shift */
        XK_Mode_switch,
        XK_Shift_Lock,
        XK_Shift_R
};

/* Forward declarations */
static void _I18n_RebindKanaKeysym ();
static void _I18n_RebindHebrewKeysym ();


/****************************************************************************
 *                                                                          *
 *  Rebind Non-Latin1 KeySyms to ASCII characters                           *
 *                                                                          *
 ****************************************************************************/

void DWI18n_RebindIrregularKeysym (dpy)

    Display   *dpy;

{
    KeyCode  lang_kc;

    lang_kc = XKeysymToKeycode(dpy, XK_kana_A);  /* Check to see if language */
    if (lang_kc) {                               /* specific Keysym exsits.  */
        _I18n_RebindKanaKeysym (dpy);            /* Japanese Kana Keyboard   */
    }

    lang_kc = XKeysymToKeycode(dpy, XK_hebrew_aleph); /* Check to see if language */
    if (lang_kc) {                               /* specific Keysym exsits.  */
        _I18n_RebindHebrewKeysym (dpy);          /* Hebrew Keyboard          */
    }

}


/****************************************************************************
 *                                                                          *
 *  Rebind Kana KeySyms to ASCII characters                                 *
 *                                                                          *
 ****************************************************************************/

 static void
 _I18n_RebindKanaKeysym (dpy)

    Display   *dpy;

{
    unsigned char str;
    int i;
    KeyCode  tmp_kcode;
    KeySym   ret_ksym;

                       /* Adjust the differences between LK201-AJ and others.*/
    tmp_kcode = XKeysymToKeycode(dpy, XK_kana_comma);
    if (tmp_kcode) {
        ret_ksym = XKeycodeToKeysym(dpy, tmp_kcode, 1);    /* Get "," or "<" */
        if (ret_ksym == NoSymbol)
            ret_ksym = XKeycodeToKeysym(dpy, tmp_kcode, 0);
        for (i = 0; kana_to_ascii_tbl3[i] | kana_to_ascii_tbl3[i + 1]; i += 2) {
            if (kana_to_ascii_tbl3[i] == XK_kana_comma) {
                kana_to_ascii_tbl3[i + 1] = (unsigned long)ret_ksym &
                                            0x000000FF;
            }
        }
    }
                       /* Adjust the differences between LK201-AJ and others.*/
    tmp_kcode = XKeysymToKeycode(dpy, XK_kana_fullstop);
    if (tmp_kcode) {
        ret_ksym = XKeycodeToKeysym(dpy, tmp_kcode, 1);    /* Get "." or ">" */
        if (ret_ksym == NoSymbol)
            ret_ksym = XKeycodeToKeysym(dpy, tmp_kcode, 0);
        for (i = 0; kana_to_ascii_tbl3[i] | kana_to_ascii_tbl3[i + 1]; i += 2) {
            if (kana_to_ascii_tbl3[i] == XK_kana_fullstop) {
                kana_to_ascii_tbl3[i + 1] = (unsigned long)ret_ksym &
                                            0x000000FF;
            }
        }
    }

    for (i = 0; kana_to_ascii_tbl1[i] | kana_to_ascii_tbl1[i + 1]; i += 2) {
        str = kana_to_ascii_tbl1[i + 1];
        XRebindKeysym(dpy, kana_to_ascii_tbl1[i], modlist_tbl1_m1_a, 1,
                      &str, sizeof(str));
    }

    for (i = 0; kana_to_ascii_tbl2[i] | kana_to_ascii_tbl2[i + 1]; i += 2) {
        str = kana_to_ascii_tbl2[i + 1];
        XRebindKeysym(dpy, kana_to_ascii_tbl2[i], modlist_tbl2_m2_a, 2,
                      &str, sizeof(str));
    }

    for (i = 0; kana_to_ascii_tbl3[i] | kana_to_ascii_tbl3[i + 1]; i += 2) {
        str = kana_to_ascii_tbl3[i + 1];
        XRebindKeysym(dpy, kana_to_ascii_tbl3[i], modlist_tbl3_m3_a, 3,
                      &str, sizeof(str));
    }

    for (i = 0; kana_to_ascii_tbl3[i] | kana_to_ascii_tbl3[i + 1]; i += 2) {
        str = kana_to_ascii_tbl3[i + 1];
        XRebindKeysym(dpy, kana_to_ascii_tbl3[i], modlist_tbl3_m3_b, 3,
                      &str, sizeof(str));
    }

    for (i = 0; kana_to_ascii_tbl3[i] | kana_to_ascii_tbl3[i + 1]; i += 2) {
        str = kana_to_ascii_tbl3[i + 1];
        XRebindKeysym(dpy, kana_to_ascii_tbl3[i], modlist_tbl3_m2_c, 2,
                      &str, sizeof(str));
    }

    for (i = 0; kana_to_ascii_tbl3[i] | kana_to_ascii_tbl3[i + 1]; i += 2) {
        str = kana_to_ascii_tbl3[i + 1];
        XRebindKeysym(dpy, kana_to_ascii_tbl3[i], modlist_tbl3_m2_d, 2,
                      &str, sizeof(str));
    }

    for (i = 0; kana_to_ascii_tbl3[i] | kana_to_ascii_tbl3[i + 1]; i += 2) {
        str = kana_to_ascii_tbl3[i + 1];
        XRebindKeysym(dpy, kana_to_ascii_tbl3[i], modlist_tbl3_m2_e, 2,
                      &str, sizeof(str));
    }

    for (i = 0; kana_to_ascii_tbl3[i] | kana_to_ascii_tbl3[i + 1]; i += 2) {
        str = kana_to_ascii_tbl3[i + 1];
        XRebindKeysym(dpy, kana_to_ascii_tbl3[i], modlist_tbl3_m3_f, 3,
                      &str, sizeof(str));
    }

    for (i = 0; kana_to_ascii_tbl3[i] | kana_to_ascii_tbl3[i + 1]; i += 2) {
        str = kana_to_ascii_tbl3[i + 1];
        XRebindKeysym(dpy, kana_to_ascii_tbl3[i], modlist_tbl3_m3_g, 3,
                      &str, sizeof(str));
    }

}


/****************************************************************************
 *                                                                          *
 *  Rebind Hebrew KeySyms to ASCII characters                               *
 *                                                                          *
 ****************************************************************************/

 static void
 _I18n_RebindHebrewKeysym (dpy)

    Display   *dpy;

{
    unsigned char str;
    int i;

    for (i = 0; hebrew_to_ascii_tbl1[i] | hebrew_to_ascii_tbl1[i + 1]; i += 2) {
        str = hebrew_to_ascii_tbl1[i + 1];
        XRebindKeysym(dpy, hebrew_to_ascii_tbl1[i], modlist_tbl1_m1_a, 1,
                      &str, sizeof(str));
    }

    for (i = 0; hebrew_to_ascii_tbl2[i] | hebrew_to_ascii_tbl2[i + 1]; i += 2) {
        str = hebrew_to_ascii_tbl2[i + 1];
        XRebindKeysym(dpy, hebrew_to_ascii_tbl2[i], modlist_tbl2_m2_a, 2,
                      &str, sizeof(str));
    }

    for (i = 0; hebrew_to_ascii_tbl3[i] | hebrew_to_ascii_tbl3[i + 1]; i += 2) {
        str = hebrew_to_ascii_tbl3[i + 1];
        XRebindKeysym(dpy, hebrew_to_ascii_tbl3[i], modlist_tbl3_m3_a, 3,
                      &str, sizeof(str));
    }

    for (i = 0; hebrew_to_ascii_tbl3[i] | hebrew_to_ascii_tbl3[i + 1]; i += 2) {
        str = hebrew_to_ascii_tbl3[i + 1];
        XRebindKeysym(dpy, hebrew_to_ascii_tbl3[i], modlist_tbl3_m3_b, 3,
                      &str, sizeof(str));
    }

    for (i = 0; hebrew_to_ascii_tbl3[i] | hebrew_to_ascii_tbl3[i + 1]; i += 2) {
        str = hebrew_to_ascii_tbl3[i + 1];
        XRebindKeysym(dpy, hebrew_to_ascii_tbl3[i], modlist_tbl3_m2_c, 2,
                      &str, sizeof(str));
    }

    for (i = 0; hebrew_to_ascii_tbl3[i] | hebrew_to_ascii_tbl3[i + 1]; i += 2) {
        str = hebrew_to_ascii_tbl3[i + 1];
        XRebindKeysym(dpy, hebrew_to_ascii_tbl3[i], modlist_tbl3_m2_d, 2,
                      &str, sizeof(str));
    }

    for (i = 0; hebrew_to_ascii_tbl3[i] | hebrew_to_ascii_tbl3[i + 1]; i += 2) {
        str = hebrew_to_ascii_tbl3[i + 1];
        XRebindKeysym(dpy, hebrew_to_ascii_tbl3[i], modlist_tbl3_m2_e, 2,
                      &str, sizeof(str));
    }

    for (i = 0; hebrew_to_ascii_tbl3[i] | hebrew_to_ascii_tbl3[i + 1]; i += 2) {
        str = hebrew_to_ascii_tbl3[i + 1];
        XRebindKeysym(dpy, hebrew_to_ascii_tbl3[i], modlist_tbl3_m3_f, 3,
                      &str, sizeof(str));
    }

    for (i = 0; hebrew_to_ascii_tbl3[i] | hebrew_to_ascii_tbl3[i + 1]; i += 2) {
        str = hebrew_to_ascii_tbl3[i + 1];
        XRebindKeysym(dpy, hebrew_to_ascii_tbl3[i], modlist_tbl3_m3_g, 3,
                      &str, sizeof(str));
    }

}

