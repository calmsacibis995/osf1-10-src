
/* DLFD.c - prototype font name utility routines */

/*#define DEBUG 			/* should come from command line */

/*
    Source: HUMAN::FONT_ARCH$:DLFD.c
    Author: Jim FLOWERS, Font Architect, Corporate Font Program
    Create date: 11 Jan 1988


________________________________________________________________________
			-- REVISION HISTORY --
 Date		Author	Description of modifications
________________________________________________________________________
18 Aug 93	Eric Osman	- fix malloc for vxt environment
30 Jul 93	Eric Osman	- merge vms and vxt sources
24 Jul 92	Eric Osman	- don't write beyond buffer size
04 Feb 91       Alfred von Campe- change malloc to XtMalloc and free to XtFree

25 May 88	Tom Porcher	- make it compile on Ultrix pcc

18 Feb 88	JRF	- fixed bug in DLFDSetFontNameField that didn't
			replace the old contents of the field being updated
25 Feb 88	JRF	- took out documentation references to REPRESENTATION
			- renamed MAX_FN_FIELDS to DLFD_MAX_FN_FIELDS
26 Apr 88	JRF	- got rid of "$" in routine/variable names
________________________________________________________________________


This module implements font name access routines as defined in DLFD. It
includes (1) a string-based set to build up a font name pattern or to parse
the font name string(s) returned by various X font access routines, and (2)
a C data structure-based set to manipulate the font name as a standard data
structure (unimplemented as of 26 April 88).


The font name string is:

    "<FontNameRegistry>-<Foundry>-<FamilyName>-<WeightName>-<Slant>
     -<SetwidthName>-<AddStyleName>-<PixelSize>-<PointSize>-<ResolutionX>
     -<ResolutionY>-<Spacing>-<AverageWidth>-<CharSetRegistry>
     -<CharSetEncoding>"


Examples:

 -ADOBE-ITC Avant Garde Gothic-Book-R-Normal--11-80-100-100-P-59-ISO8859-1 
 -ADOBE-ITC Avant Garde Gothic-Book-R-Normal--14-100-100-100-P-80-ISO8859-1
 -Adobe-Courier-Bold-R-Normal--14-140-75-75-M-90-ISO8859-1
 -Adobe-Courier-Bold-R-Normal--18-180-75-75-M-110-ISO8859-1
 -ADOBE-Courier-Bold-R-Normal--24-240-75-75-M-150-ISO8859-1
 -Adobe-Helvetica-Medium-R-Normal--25-180-100-100-P-130-ISO8859-1
 -Adobe-Helvetica-Medium-R-Normal--34-240-100-100-P-176-ISO8859-1
 -ADOBE-Stone-Demi-R-Condensed-Informal--11-80-100-100-P-61-DEC-ADOBESTANDARD
 -ADOBE-Stone-Demi-R-Condensed-Serif--11-80-100-100-P-61-DEC-ADOBESTANDARD

*/


/* Include files */

#include "dlfd.h"		/* DLFD definitions */

#ifdef VXT_DECTERM
#include <stdlib.h>		/* required for proper malloc symbol mapping */
#endif

/* DLFD.c */
/* DLFDGetFontNameField - routine to get font name field value */


PSTR	DLFDGetFontNameField(eFieldID, pstrFontName, pFieldSize)

DLFD_EFNFIELD eFieldID;		/* enumerated type field id to read */
PSTR 	pstrFontName; 		/* ptr to the font name string */
int 	*pFieldSize; 		/* ptr to int where field size is returned */


/* "DLFDGetFontNameField" is used to locate the font name field specifed by
the "eFieldID" argument. Returns a ptr to the beginning of the field in the
specified font name, or NULL if the field was not found. The field size is
returned in the int specified by the ptr "pFieldSize".

See "DLFDCopyFontNameField" routine if a direct copy to a local buffer is
to be done. */

{
register int i = (int) eFieldID;		/* get field ID as counter */
register PSTR pstrField = pstrFontName; 	/* field pointer */
register PSTR pstrNextField;			/* local ptr to next field */


    while (i-- > NULL)				/* find field */
	while ((*pstrField != (char) NULL)
		&& (*pstrField++ != (char) DLFD_FN_DELIMITER)); 

    if (*pstrField == (char) NULL){		/* failed ? */
	*pFieldSize = NULL;
	return(NULL);
    }

    for (pstrNextField = pstrField;		/* find next field */
	    ((*pstrNextField != (char) NULL)
	     && (*pstrNextField != (char) DLFD_FN_DELIMITER)); 
		pstrNextField++);

    *pFieldSize =  pstrNextField - pstrField;	 /* update field size */

    if (*pstrNextField == (char) NULL)		/* correct if at end? */
	*pFieldSize--;

#ifdef DEBUG
printf("DLFDGetFontNameField:\n... %d chars of %s\n", 
	*pFieldSize, pstrField);
#endif

    return(pstrField);				/* return ptr to field */

}
/* end of "DLFDGetFontNameField" function */


/* DLFD.c */
/* DLFDCopyFontNameField - copy specified font name field to buffer */


PSTR	DLFDCopyFontNameField (pDest, eFieldID, pstrFontName)

DLFD_EFNFIELD eFieldID;
PSTR	pDest, pstrFontName;

{
PSTR	pstrField;
int	nFieldChars;

    pstrField = DLFDGetFontNameField(eFieldID, pstrFontName, &nFieldChars);
    strncpy(pDest, pstrField, nFieldChars);
    *(pDest + nFieldChars)= (char) NULL;		/* NULL terminate */

    return(pDest);
}
/* end "DLFDCopyFontNameField" function */


/* DLFD.c */
/* DLFDSetFontNameField - update font name field with string value */


PSTR 	DLFDSetFontNameField(eFieldID, pstrFontName, nFieldSize, 
			      pstrFieldValue)

DLFD_EFNFIELD eFieldID;		/* enumerated type field id to update */
PSTR	pstrFontName; 		/* ptr to the font name string to update */
int	nFieldSize;		/* the number of chars in given field */
PSTR	pstrFieldValue;		/* ptr to field string */


/* "DLFDSetFontNameField" is used to update the font name field specified
by the "eFieldID" argument. A ptr to the field string value and length is
given by "pstrFieldValue" and "nFieldSize", respectively. Returns ptr to the
updated font name string, "pstrFontName", or NULL, if the field ID was
incorrect, or scratch space couldn't be found. Note, that any previous 
contents of the field in the font name string will be replaced. 

Guaranteed to build a syntactically correct font name string, i.e., can be
called with an empty font name, and will check that the field value is
permitted for the current version-revision level (eventually, JRF 26 Apr 88) */

{
register PSTR pstrSource = pstrFontName; 	/* ptr to source byte  */
PSTR pLocalBuffer;				/* ptr to local scratch buffer*/
int nBufferSize;
register PSTR pstrDest = pLocalBuffer;		/* ptr to destination byte */
register int i = (int) eFieldID;		/* get field ID as counter */

#ifdef DEBUG
printf("DLFDSetFontNameField:\n... %3d, %s of %s\n", (int)eFieldID, 
	pstrFieldValue, pstrFontName);
#endif

   nBufferSize = strlen(pstrFontName) + nFieldSize + DLFD_MAX_FN_FIELDS +1;

   if((pLocalBuffer = pstrDest = (PSTR) malloc(nBufferSize))  /* get local storage */
	== NULL)
	return ((PSTR) NULL);

    while (i-- > NULL){				/* copy upto eFieldID */
	while ((*pstrSource != (char) NULL)
		&& (*pstrSource != (char) DLFD_FN_DELIMITER))
	    *pstrDest++ = *pstrSource++;
	if (*pstrSource == (char) DLFD_FN_DELIMITER)
	    *pstrDest++ = *pstrSource++;
	else *pstrDest++ = (char) DLFD_FN_DELIMITER;	/* append delimiter */
    }
    /* end of "while" */

    strncpy(pstrDest, pstrFieldValue, nFieldSize); /* append field value */
    *(pstrDest+nFieldSize) = (char) NULL;	/* null terminate */

    for (; ((*pstrSource != (char) NULL)	/* find next field */
	     && (*pstrSource != (char) DLFD_FN_DELIMITER)); 
		pstrSource++);

    strcat(pLocalBuffer, pstrSource);		/* append rest */
    strncpy(pstrFontName, pLocalBuffer, MAX_FONT_NAME_SIZE);

    free(pLocalBuffer);				/* return space */

#ifdef DEBUG
printf("... Fontname %s\n", pstrFontName);
#endif
    return(pstrFontName);

}
/* end of "DLFDSetFontNameField" function */


/* DLFD.c */
/* DLFDSetFontNameStruct - convert font name string to data struct rep */


DLFD_FONTNAMESTRUCT *DLFDSetFontNameStruct(pstrFontName, pFontNameStruct)

PSTR 	pstrFontName; 		/* ptr to the font name string to convert */
DLFD_FONTNAMESTRUCT *pFontNameStruct; /* ptr to buffer return font name DS */


/* "DLFDSetFontNameStruct" is used to convert a font name string, as used by
X/DECwindows calls directly, into the equivalent DLFD font name data
structure. Should be used where the font name will be extensively modified,
or for DDIF storage/interchange. Returns a ptr to the updated font name
structure, "pFontNameStruct", or NULL if an error was encountered.*/


{

}
/* end of "DLFDSetFontNameStruct" function */


/* DLFD.c */
/* DLFDSetFontNameString - convert fontname data structure rep to string */


PSTR 	DLFDSetFontNameString(pstrFontName, pFontNameStruct)


PSTR 	pstrFontName; 		/* ptr to a buffer to return font name */
DLFD_FONTNAMESTRUCT *pFontNameStruct; /* ptr to the font name data structure */

/* "DLFDGetFontNameString" is used to convert a font name data structure into
a valid font name string, according to the syntax specified in DLFD. This
font name can then be used directly by X/DECwindows font access routines, or
can be modified via "DLFDSetFontNameField" to set up a suitable pattern
match. Returns a ptr to the created font name string "pstrFontName", or NULL
if an error was encountered.*/


{

}
/* end of "DLFDSetFontNameString" function */


/* end of module "DLFD" */

