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
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/cardfiler/cardio.c,v 1.1.4.2 1993/09/09 17:05:15 Susan_Ng Exp $";
#endif
#endif
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
**	cardio.c
**
**  FACILITY:
**      OOTB Cardfiler
**
**  ABSTRACT:
**	Currently contains most of the IO routines for the Cardfiler.
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

#ifdef VMS
void InitImaging();
globalref Boolean imagingInited;
globalref int (*ImagingCloseDDIFFile)();
globalref void (*ImagingDeleteFrame)();
globalref void (*ImagingDeleteDDIFStream)();
globalref unsigned long int (*ImagingCopy)();
globalref unsigned long int (*ImagingCreateDDIFStream)();
globalref unsigned long int (*ImagingCreateFrame)();
globalref unsigned long int (*ImagingImportBitmap)();
globalref unsigned long int (*ImagingExportDDIFFrame)();
globalref unsigned long int (*ImagingImportDDIFFrame)();
globalref unsigned long int (*ImagingGetFrameAttributes)();
globalref unsigned long int (*ImagingDecompress)();
globalref unsigned long int (*ImagingExportBitmap)();
globalref unsigned long int (*ImagingOpenDDIFFile)();
#else
extern Boolean imagingInited;
#endif

#ifndef NO_ISL
#include <img/ChfDef.h>
long ootb$isl_condition_handler();
#endif

#define INTSIZE 2
#define UNSIGNEDSIZE 2
#define UNSIGNEDCHARSIZE 1
#define UNSIGNEDLONGSIZE 4

int readcards();
static int failure_occured;
static char reserved_rec[32];

/*
**++
**  ROUTINE NAME: readcards
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure gets the data for a card from the file in which it
**	is stored.
**
**  FORMAL PARAMETERS:
**	fp -	file pointer to read the cards from.
**	mergefile - Boolean stating if we are merging a file into the index window.
**		    TRUE = merge,  FALSE = open.
**		    New ID will be assigned to the merged cards.
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
int readcards(fp, mergefile)
    FILE *fp;
    Boolean mergefile;
{
    char mgc[4], line[INDEX_LENGTH + 1];
    int i, j, lfData, error = FALSE;
    short int number;
    size_t status;
    unsigned int card_id;
    unsigned long temp;
    unsigned short reserved, flag;
    card_pointer temp_card;
    card_pointer current;
    /* reserved field is used for a unique card id. */
    /* char reserved[6]; */
    /* the old flags field is being used as a version number of the application.
     * Motif V1.0 or V3.0 will write out a %x3000 or "0", while prior versions have
     * been writing a %x2000 or " ". */

    /* Read in the first three bytes and check to see if they are 'mgc'. If
     * not this file isn't a cardfiler compatable file. */

    WorkCursorDisplay();
    if (fread(mgc, sizeof(mgc) - 1, 1, fp) !=1)
	error = TRUE;
    mgc[3] = '\0';

    if (strcmp(mgc, "mgc") != 0) {
	WorkCursorRemove();
	display_message("NotCardfileError");
	XBell(dpy, 0);
        error = TRUE;
    } else {
	/* Read in the number of cards and then loop reading in the data for
	 * each card. */
	if (fread(&number, INTSIZE, 1, fp) != 1) 
	    error = TRUE;
#ifdef IO_DEBUG
	printf("\nNumber of cards: %d\n",number);
#endif
	for (j = 0; j < number; j++) {

	    /* Read in reserved byte. */
	    /* In V3.0 or Motif V1.0 the reserved field has been used for
	     * storing an unsigned int = the unique identifier to the card an
	     * unsigned short = reserved */
	    if (fread(&card_id, UNSIGNEDLONGSIZE, 1, fp) != 1)
		error = TRUE;
	    if (fread(&reserved, UNSIGNEDSIZE, 1, fp) != 1)
		error = TRUE;

	    /* Read in the offset to the data in the file. */
	    if (fread(&lfData, UNSIGNEDLONGSIZE, 1, fp) != 1)
		error = TRUE;

	    /* Read in the flag bytes. */
	    if (fread(&flag, UNSIGNEDSIZE, 1, fp) != 1)
		error = TRUE;

	    /* Read in the index field of the card. */
	    if (fread(line, sizeof(line), 1, fp) != 1)
		error = TRUE;

	    /* Create a card and update it's pointers to point to the data in
	     * this file. */
	    temp_card = addcard(line);

	    /* if it is a file version of 3.0 or higher, then copy the id.
	     * with "0" being hex value of %x30. */
	    if ((flag >= 48) && (mergefile == FALSE)) {
		temp_card->card_id = card_id;
	    }
	      /* otherwise assign it one. */
	      else {
		highest_card_id_used++;
		temp_card->card_id = highest_card_id_used;
	    }
#ifdef IO_DEBUG
	    printf("Card_id: %d\t",card_id);
	    printf("New Card_id: %d\t",temp_card->card_id);
            printf("Reserved: %d\t",reserved);
            printf("Data Offset: %d\t",lfData);
	    if ((flag >= 48) && (mergefile == FALSE)) 
		printf("*");
            printf("Flag: %d\t",flag);
	    printf("Index Title: %s\n",line);
#endif
	    temp_card->reserved = reserved;
	    temp_card->orig_fp = fp;
	    temp_card->orig_offset = lfData;
	    temp_card->fp = fp;
	    temp_card->offset = lfData;
	    temp_card->index_number = index_count;
	    index_count++;
	    numcards++;
	}

	/* if it is a file version of 3.0 or higher,  then read in the trailer
	 * record info. with "0" being hex value of %x30. */
	if ((flag >= 48) && (mergefile == FALSE)) {

	    /* Read in the highest unique card id used. */
	    if (fread(&highest_card_id_used, UNSIGNEDLONGSIZE, 1, fp) != 1)
		error = TRUE;
	    if (fread(reserved_rec, sizeof(reserved_rec), 1, fp) != 1)
		error = TRUE;
	}

#ifndef VMS
	current = first;
	while (current->next != NULL) {
	    readonecard(current, &temp_card1);
	    saveonecard(current, &temp_card1, TRUE);
	    current = current->next;
	}
	fclose(fp);
#endif
	if (numcards > 0)
	    gray_no_cards();

	if (error) {
	    WorkCursorRemove();
	    display_message("ReadFileError");
	    XBell(dpy, 0);
	}
    }
    if (!error)
	WorkCursorRemove();

    return (error);
}

/*
**++
**  ROUTINE NAME: savecards
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure saves the data for the cards in the file.
**
**  FORMAL PARAMETERS:
**	fp - file pointer to file in which to save the cards.
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
static char mgc[] = "mgc";
static char flag[] = "0";

int savecards(fp)
    FILE *fp;
{
    char buff1[INDEX_LENGTH + 1], buff2[TEXT_LENGTH + 1], *buff3;
    int rem, save_width, textsize, sizetest, i = 0;
    short temp;
    short int number;
    unsigned int tempoffset;
    unsigned short bmsize;
    card_pointer current;

    number = numcards;

    /* Write out 'mgc' to identify this as a cardfile. Write out the number of
     * cards. */

    if (fwrite(mgc, strlen(mgc), 1, fp) != 1)
	return (FALSE);
    if (fwrite(&number, INTSIZE, 1, fp) != 1)
	return (FALSE);

    /* Cycle through all the cards writing out the info according to the MS
     * CARDFILER's format. */

    current = first;
    while (current->next != NULL) {

	/* Write out the reserved field. */
	/* In V3.0 or Motif V1.0 the reserved field has been used for storing
	 * an unsigned int = the unique identifier to the card an unsigned
	 * short = reserved  */
#ifdef VMS
	/* don't do fwrite checking on VMS (it's too Slooow)
	   until the end, this should still pick up most disk errors */
	fwrite(&current->card_id, UNSIGNEDLONGSIZE, 1, fp);
	fwrite(&current->reserved, UNSIGNEDSIZE, 1, fp);
#else
	if (fwrite(&current->card_id, UNSIGNEDLONGSIZE, 1, fp) != 1)
	    return (FALSE);
	if (fwrite(&current->reserved, UNSIGNEDSIZE, 1, fp) != 1)
	    return (FALSE);
#endif
	/* Get the current offset into the file, so that we can return and
	 * write real offset into file for data. */
	current->temp_offset = tempoffset = ftell(fp);
	if (tempoffset == EOF) {
	    printf("Cardfiler-S-Filerr, ftell error.\n");
	    return (FALSE);
	}

	/* Write out the temporary offset. */
#ifdef VMS
	fwrite(&tempoffset, UNSIGNEDLONGSIZE, 1, fp);

	/* Write out the flag. */
	fwrite(flag, UNSIGNEDSIZE, 1, fp);

	/* Write out the index field of the card. */
	fwrite(current->index, INDEX_LENGTH + 1, 1, fp);
#else
	if (fwrite(&tempoffset, UNSIGNEDLONGSIZE, 1, fp) != 1)
	     return (FALSE);

	/* Write out the flag. */
	if (fwrite(flag, UNSIGNEDSIZE, 1, fp) != 1)
	     return (FALSE);

	/* Write out the index field of the card. */
	if (fwrite(current->index, INDEX_LENGTH + 1, 1, fp) != 1)
	     return (FALSE);
#endif

	current = current->next;
    }


    /* Write out the highest_card_id_used and some reserved bytes. */
    if (fwrite(&highest_card_id_used, UNSIGNEDLONGSIZE, 1, fp) != 1)
	return (FALSE);
    if (fwrite(reserved_rec, sizeof(reserved_rec), 1, fp) != 1)
	return (FALSE);

    /* Cycle through cards. Now we write out the text and bitmap information.
     * and return to update the offset pointer into the data. */

    current = first;
    while (current->next != NULL) {

	/* Get the current offset so we can return here after updating the
	 *  previously written temporary offset above. Then go back and write
	 * the real offset. */
	tempoffset = ftell(fp);
	if (tempoffset == EOF)
	    return (FALSE);
	if (fseek(fp, current->temp_offset, 0))
	    return (FALSE);

#ifdef VMS
	fwrite(&tempoffset, UNSIGNEDLONGSIZE, 1, fp);

	/* Read in the data for the current card that we're writing out. */
	bmsize = 0;
	readonecard(current, &temp_card1);
	if ((temp_card1.bitmap_height > 0) && (temp_card1.bitmap_width > 0)) {
	    rem = temp_card1. bitmap_width % 32;
	    if (rem == 0)
		save_width = temp_card1. bitmap_width;
	    else
		save_width = temp_card1. bitmap_width - rem + 32;
	    bmsize = (temp_card1. bitmap_height * save_width) / 8 + 1;
	}

	/* Write out the bitmap info. */
	fseek(fp, tempoffset, 0);

	fwrite(&bmsize, INTSIZE, 1, fp);

	if (bmsize > 0) {
	    temp = 0;
	    fwrite(&temp, INTSIZE, 1, fp);
	    fwrite(&temp, INTSIZE, 1, fp);
	    temp = temp_card1.bitmap_width;
	    fwrite(&temp, INTSIZE, 1, fp);
	    temp = temp_card1.bitmap_height;
	    fwrite(&temp, INTSIZE, 1, fp);
	    fwrite(temp_card1.bitmap, bmsize - 1, 1, fp);
	}

	/* Write out the text size and the text data.*/
	textsize = strlen(temp_card1.text) + 1;
	fwrite(&textsize, INTSIZE, 1, fp);
	fwrite(temp_card1.text, textsize, 1, fp);
#else
	if (fwrite(&tempoffset, UNSIGNEDLONGSIZE, 1, fp) != 1)
	     return (FALSE);

	/* Read in the data for the current card that we're writing out. */
	bmsize = 0;
	readonecard(current, &temp_card1);
	if ((temp_card1.bitmap_height > 0) && (temp_card1.bitmap_width > 0)) {
	    rem = temp_card1. bitmap_width % 32;
	    if (rem == 0)
		save_width = temp_card1. bitmap_width;
	    else
		save_width = temp_card1. bitmap_width - rem + 32;
	    bmsize = (temp_card1. bitmap_height * save_width) / 8 + 1;
	}

	/* Write out the bitmap info. */
	if (fseek(fp, tempoffset, 0))
	    return (FALSE);

	if (fwrite(&bmsize, INTSIZE, 1, fp) != 1)
	     return (FALSE);

	if (bmsize > 0) {
	    temp = 0;
	    if (fwrite(&temp, INTSIZE, 1, fp) != 1)
		return (FALSE);
	    if (fwrite(&temp, INTSIZE, 1, fp) != 1)
		return (FALSE);
	    temp = temp_card1.bitmap_width;
	    if (fwrite(&temp, INTSIZE, 1, fp) != 1)
		return (FALSE);
	    temp = temp_card1.bitmap_height;
	    if (fwrite(&temp, INTSIZE, 1, fp) != 1)
		return (FALSE);
	    if (fwrite(temp_card1.bitmap, bmsize - 1, 1, fp) != 1)
		return (FALSE);
	}

	/* Write out the text size and the text data.*/
	textsize = strlen(temp_card1.text) + 1;
	if (fwrite(&textsize, INTSIZE, 1, fp) != 1)
	    return (FALSE);
	if (fwrite(temp_card1.text, textsize, 1, fp) != 1)
	    return (FALSE);
#endif
	current = current->next;
    }
    return (TRUE);
}

/*
**++
**  ROUTINE NAME: readonecard
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure gets the data for a card from the file in which it
**	is stored.
**
**  FORMAL PARAMETERS:
**	thiscard -	card_pointer of card to read data for
**	card - 		buffer to return card info in.
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
int readonecard(thiscard, card)
    card_pointer thiscard;
    card_contents card;
{
    int flag, num, status;
    short cxBitmap, cyBitmap, cxEigths, cyEigths;
    unsigned short bmSize, tSize;

    if (thiscard->fp == NULL)
	return (NULL);
    else {
	strcpy(card->index, thiscard->index);
	status = fseek(thiscard->fp, thiscard->offset, 0);

	if (fread(&bmSize, INTSIZE, 1, thiscard->fp) != 1)
		return (FALSE);

	/* Get the bitmap information if the bitmap size is > 0 */
	if ((bmSize > 0) && ( ((int)bmSize) <= BITMAP_SIZE)) {
#ifdef VMS
	    fread(&cxBitmap, INTSIZE, 1, thiscard->fp);
	    fread(&cyBitmap, INTSIZE, 1, thiscard->fp);
	    fread(&cxEigths, INTSIZE, 1, thiscard->fp);
	    fread(&cyEigths, INTSIZE, 1, thiscard->fp);
#else
	    if (fread(&cxBitmap, INTSIZE, 1, thiscard->fp) != 1)
		return (FALSE);
	    if (fread(&cyBitmap, INTSIZE, 1, thiscard->fp) != 1)
		return (FALSE);
	    if (fread(&cxEigths, INTSIZE, 1, thiscard->fp) != 1)
		return (FALSE);
	    if (fread(&cyEigths, INTSIZE, 1, thiscard->fp) != 1)
		return (FALSE);
#endif
	    card->bitmap_height = cyEigths;
	    card->bitmap_width = cxEigths;
	    if ((card->bitmap_height * card->bitmap_width) <=
	      (BITMAP_SIZE * 8)) {
		if (fread(card->bitmap, bmSize - 1, 1, thiscard->fp) != 1)
		    return (FALSE);
	    } else {
		card->bitmap_height = 0;
		card->bitmap_width = 0;
	    }
	} else {
	    card->bitmap_height = 0;
	    card->bitmap_width = 0;
	}

	if (fread(&tSize, INTSIZE, 1, thiscard->fp) != 1)
		return (FALSE);

	/* Get the text info. */
	if (tSize < TEXT_LENGTH + 2) {
	    if (fread(card->text, tSize, 1, thiscard->fp) != 1)
		return (FALSE);
	    card->text[tSize] = '\0';
	    flag = 1;
	} else {
	    card->text[TEXT_LENGTH] = '\0';
	    flag = 1;
	}
	return (flag);
    }
}

/*
**++
**  ROUTINE NAME: saveonecard
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure stores the data for one card and modifies it's data
**	structure to reflect the new situation.
**
**  FORMAL PARAMETERS:
**	savecard - pointer to data structure of card to be saved
**	cardstuff - contents of card to be saved.
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
int saveonecard(savecard, cardstuff, from_file)
    card_pointer savecard;
    card_contents cardstuff;
    Boolean from_file;
{
    int offset, rem, save_width, sizetest;
    unsigned short bmsize, textsize, temp;

    if ((cardstuff->bitmap_width > 0) && (cardstuff->bitmap_height > 0)) {
	rem = cardstuff->bitmap_width % 32;
	if (rem == 0)
	    save_width = cardstuff->bitmap_width;
	else
	    save_width = cardstuff->bitmap_width - rem + 32;

	bmsize = (cardstuff->bitmap_height * save_width) / 8 + 1;
    } else
	bmsize = 0;

    textsize = strlen(cardstuff->text) + 1;
    temp = 0;

    /* Go to the end of the temporary file and get the offset. */
    if (fseek(tempfile, 0, 2)) {
	printf("Cardfiler-S-Filerr, fseek error.\n");
	return (FALSE);
    }
    offset = ftell(tempfile);
    if (offset == EOF) {
	printf("Cardfiler-S-Filerr, ftell error.\n");
	return (FALSE);
    }

    /* Write out the bitmap info. */
    if (fwrite(&bmsize, INTSIZE, 1, tempfile) != 1)
	return (FALSE);
    if (bmsize > 0) {
	temp = 0;
	if (fwrite(&temp, INTSIZE, 1, tempfile) != 1)
	    return (FALSE);
	if (fwrite(&temp, INTSIZE, 1, tempfile) != 1)
	    return (FALSE);
	temp = cardstuff->bitmap_width;
	if (fwrite(&temp, INTSIZE, 1, tempfile) != 1)
	    return (FALSE);
	temp = cardstuff->bitmap_height;
	if (fwrite(&temp, INTSIZE, 1, tempfile) != 1)
	    return (FALSE);
	if (fwrite(cardstuff->bitmap, bmsize - 1, 1, tempfile) != 1)
	    return (FALSE);
    }

    /* Write out the text info. */
    if (fwrite(&textsize, sizeof(textsize), 1, tempfile) != 1)
	return (FALSE);
    if (fwrite(cardstuff->text, textsize, 1, tempfile) != 1)
	return (FALSE);

    /* Update the cards file and offset pointers to point to newly written
     * data.*/
    savecard->fp = tempfile;
    savecard->offset = offset;

    /* If the card came from readcards then set orig_fp and orig_offset for
     * Restore option */
    if (from_file) {
	savecard->orig_fp = tempfile;
	savecard->orig_offset = offset;
    }

    /* Mark the file changed. */
    if (!from_file) {
	file_changed = TRUE;
	bitmap_changed = FALSE;
    }
}

/*
**++
**  ROUTINE NAME: check_and_save
**
**  FUNCTIONAL DESCRIPTION:
**	Checks to see if the card displayed is the same as it was last
**	saved. If not it saves it.
**
**  FORMAL PARAMETERS:
**	thisone - card to be compared and saved if different.
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
check_and_save(thisone)
    card_pointer thisone;
{
    char *temp;
    int rem, save_width;
    int flag;
    long byte_count, cvt_status;
    struct card_contents_rec oldcard;
    XmString cs_temp;

    cs_temp = DXmCSTextGetString(valuewindow);
    if (cs_temp == NULL) {
	temp = XtMalloc(1);
	temp[0] = '\0';
    } else
	temp = (char *) DXmCvtCStoFC(cs_temp, &byte_count, &cvt_status);

    XmStringFree(cs_temp);
    if (thisone != NULL) {
	flag = readonecard(thisone, &oldcard);
	if ((strcmp(oldcard .index, card_displayed->index) != 0) ||
	  (strcmp(oldcard .text, temp) != 0) || (flag == NULL) ||
	  (bitmap_changed == TRUE)) {
	    rem = bmp_width % 32;
	    if (rem == 0)
		save_width = bmp_width;
	    else
		save_width = bmp_width - rem + 32;

	    strcpy(temp_card1.text, temp);
	    strcpy(temp_card1.index, card_displayed->index);
	    mybcopy(bitmap, temp_card1.bitmap, (bmp_height * save_width) / 8);
	    temp_card1.bitmap_height = bmp_height;
	    temp_card1.bitmap_width = bmp_width;

	    saveonecard(thisone, &temp_card1, FALSE);
	}
    }
    XtFree(temp);
}

/*
**++
**  ROUTINE NAME: gray_no_cards
**
**  FUNCTIONAL DESCRIPTION:
**
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
gray_no_cards()
{
    int CardsPresent;

    if (numcards == 0)
	CardsPresent = FALSE;
    else
	CardsPresent = TRUE;

    XtSetSensitive((Widget)print_button, CardsPresent);
    XtSetSensitive((Widget)print_as_button, CardsPresent);
    XtSetSensitive((Widget)goto_button, CardsPresent);
    XtSetSensitive((Widget)find_button, CardsPresent);
    if (card_goto_button != NULL)
	XtSetSensitive((Widget)card_goto_button, CardsPresent);
    if (card_find_button != NULL)
	XtSetSensitive((Widget)card_find_button, CardsPresent);
    if (CardsPresent && (find_target[0] != '\0')) {
	XtSetSensitive((Widget)find_next_button, CardsPresent);
	if (card_find_next_button != NULL)
	    XtSetSensitive((Widget)card_find_next_button, CardsPresent);
    }
}

/*
**++
**  ROUTINE NAME: card_Read_DDIF
**
**  FUNCTIONAL DESCRIPTION:
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
int card_Read_DDIF(fname)
    char *fname;
{
#ifndef NO_ISL
    int i, status, byte_count;
    unsigned long int context; 
    CDAstatus CDA_Status;
    CDAsize fname_len;
    CDAitemlist item_list;
    DDISstreamhandle stream_handle;
    CDAfilehandle file_handle;
    CDArootagghandle root_handle;
    CDAaggtype aggregate_type;
    ITEM_LIST_TYPE process_option_list[3];
#ifdef VMS
    vms_descriptor_type descriptor;
#endif

    INIT_IMAGING;

    status = OK_STATUS;

    fname_len = strlen(fname);
#ifdef VMS
    descriptor.string_length = fname_len;
    descriptor.string_type = 14;
    descriptor.string_class = 1;
    descriptor.string_address = (unsigned char *) fname;
#endif
    MAKE_ITEM(process_option_list, 0, DDIF_INHERIT_ATTRIBUTES, 0, 0);
    MAKE_ITEM(process_option_list, 1, DDIF_EVALUATE_CONTENT, 0, 0);
    MAKE_ITEM(process_option_list, 2, 0, 0, 0);

    aggregate_type = DDIF_DDF;	/* Root aggregate type */
    CDA_Status = cda_open_file(&fname_len, fname,
	0, 		/* Default file specification length optional */
	0, 		/* Default file specification optional */
	0, 		/* Default memory_alloc_proc        */
	0, 		/* Default memory_dealloc_proc      */
	0, 		/* Default alloc_dealloc_context    */
	&aggregate_type,
	process_option_list,
	0,		/* Default result_filename_len      */
	0, 		/* Default result_filename          */
	0, 		/* Default result_filename_ret_len  */
	&stream_handle,
	&file_handle,
	&root_handle);

    ChfEstablish(ootb$isl_condition_handler);
    failure_occured = FALSE;

    if (CDA_Status == CDA_NORMAL) {
	CDA_Status = cda_close_file(&stream_handle, &file_handle);
	context = ImgOpenDDIFFile(ImgK_ModeImport, fname_len, fname, 0, 0, 0);
    } else
	status = NOT_DDIF_FILE;

    if (failure_occured) {
	ChfRevert();
	status = NOT_DDIF_FILE;
	failure_occured = FALSE;
    }

    if (!failure_occured && (status != NOT_DDIF_FILE)) {
	status = Load_DDIF_Frame(context);
    }

    if (status != NOT_DDIF_FILE)
	byte_count = ImgCloseDDIFFile(context, 0);
    return (status);
#endif
}

/*
**++
**  ROUTINE NAME: Load_DDIF_Frame
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure gets the data for a card from the file in which it
**	is stored.
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
int Load_DDIF_Frame(context)
    unsigned long int context;
{
#ifndef NO_ISL
    int status;
    unsigned long int width, height, size, stride, spec_type, new_stride, pad, 
	get_index, set_index, image_comp, image_id = 0, new_image_id = 0;
    struct ITMLST get_attributes[6];
    struct PUT_ITMLST set_attributes[4];
    CDAstatus CDA_Status;

    INIT_IMAGING;

    status = OK_STATUS;

    width = (unsigned long) bmp_width;
    height = (unsigned long) bmp_height;

    start_get_itemlist(get_attributes, get_index);
    put_get_item(get_attributes, get_index, Img_CompressionType, image_comp);
    put_get_item(get_attributes, get_index, Img_PixelsPerLine, width);
    put_get_item(get_attributes, get_index, Img_NumberOfLines, height);
    put_get_item(get_attributes, get_index, Img_SpectType, spec_type);
    put_get_item(get_attributes, get_index, Img_ScanlineStride, stride);
    end_get_itemlist(get_attributes, get_index);
    failure_occured = FALSE;
    image_id = ImgImportDDIFFrame(context, 0, 0, 0, 0, 0);
    if (image_id == 0)
	status = DDIF_NOGRAPHIC;

    if (failure_occured) {
	ChfRevert();
	status = DDIF_NOGRAPHIC;
    }

    if (status == OK_STATUS) {
	ImgGetFrameAttributes(image_id, get_attributes);
	if (image_comp != ImgK_PcmCompression) {
	    ImgDecompress(image_id);
	    image_comp = ImgK_PcmCompression;

	    /* After decompression, reget the frame attribute in case they have
	     * changed.  Added to fixed compatibility problem between
	     * DECpaint V2 and Cardfiler V3. */
	    ImgGetFrameAttributes(image_id, get_attributes);
	}

	/* Check to make sure image is 32 bit aligned */
	pad = stride % 32;

	if (pad != 0)
	    new_stride = stride + 32 - pad;
	else
	    new_stride = stride;
	size = new_stride * height / 8;
	if (size > BITMAP_SIZE)
	    status = BMP_TOOLARGE;

	if (spec_type != ImgK_StypeBitonal)
	    status = NOT_VALID_TYPE;
    }

    if (status == OK_STATUS) {
	if (pad != 0) {
	    start_set_itemlist(set_attributes, set_index);
	    put_set_item(set_attributes, set_index, Img_ScanlineStride,
	      new_stride);
	    end_set_itemlist(set_attributes, set_index);
	    new_image_id = ImgCopy(image_id, 0, set_attributes);
	    stride = new_stride;
	    ImgExportBitmap(new_image_id, 0, bitmap, BITMAP_SIZE, 0, 0, 0, 0);
	    ImgDeleteFrame(new_image_id);
	} else
	    ImgExportBitmap(image_id, 0, bitmap, BITMAP_SIZE, 0, 0, 0, 0);

	bmp_width = (Dimension) width;
	bmp_height = (Dimension) height;

	bmp_bytes = stride / 8;
    }

    if (status == OK_STATUS)
	ImgDeleteFrame(image_id);
    return (status);
#endif
}

/*
**++
**  ROUTINE NAME: ootb$isl_condition_handler()
**
**  FUNCTIONAL DESCRIPTION:
**	This routine is called when an exception or condition occurs which
**	would ordinarily cause the Viewer, and anyone who invoked the Viewer,
**	to exit.  This routine is specifically to take care of conditions
**	raised by the GKS, ISL, and IDS support routines.  Some of these
**	are "bad" in that they call LIB$STOP instead of just returning a
**	bad status.  This routine catches the bad status and returns it as
**	the value of the shell routine surrounding each support routine call.
**
**	To use this routine, write a shell routine which calls the desired
**	support routine.  It should look like:
**
**	    int Dvr$$foo_shell( foo_args )
**		{
**		int status;
**
**		status = DVR$_NORMAL;
**		ChfEstablish (dvru$graphic_condition_handler );
**		foo( foo_args );
**	        return( status );
**		}
**
**  FORMAL PARAMETERS:
**	signal	- signal vector
**	mechanism - error mechanism arguments
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**	Error status if any were raised.
**
**  FUNCTION VALUE:
**	Whatever error status caused the condition
**
**  SIDE EFFECTS:
**--
**/
#ifndef NO_ISL
#define K_ISL_FAILURE 4
/*#define ss$_continue	1*/
long ootb$isl_condition_handler(sigarg, mecharg)
    ChfSigVecPtr sigarg;
    ChfMchArgsPtr mecharg;
{
    int unwind_depth;

    /* Return from the procedure which established this exception handler */
#ifdef DEBUG
    printf("Condition handler called.\n");
#endif
    failure_occured = TRUE;
    unwind_depth = mecharg->depth;
    ChfUnwind(&unwind_depth, 0);
    return (Chf_Continue);
/*
    if (sigarg->name == Chf_Unwind)
        return Chf_Resignal;

    if (sigarg->name == Chf_Debug)
        return Chf_Resignal;

    if (!BAD(sigarg->name)) 
    return Chf_Continue;
*/

}
#endif
