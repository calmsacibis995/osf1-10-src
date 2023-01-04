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
 * DviChar.h
 *
 * descriptions for mapping dvi names to
 * font indexes and back.  Dvi fonts are all
 * 256 elements (actually only 256-32 are usable).
 *
 * The encoding names are taken from X -
 * case insensitive, a dash seperating the
 * CharSetRegistry from the CharSetEncoding
 */

# define DVI_MAX_SYNONYMS	10
# define DVI_MAP_SIZE		256
# define DVI_HASH_SIZE		256
# define DVI_MAX_LIGATURES	16

typedef struct _dviCharNameHash {
	struct _dviCharNameHash	*next;
	char			*name;
	int			position;
} DviCharNameHash;

typedef struct _dviCharNameMap {
    char		*encoding;
    int			special;
    char		*dvi_names[DVI_MAP_SIZE][DVI_MAX_SYNONYMS];
    char		*ligatures[DVI_MAX_LIGATURES][2];
    DviCharNameHash	*buckets[DVI_HASH_SIZE];
} DviCharNameMap;

extern DviCharNameMap	*DviFindMap ( /* char *encoding */ );
extern void		DviRegisterMap ( /* DviCharNameMap *map */ );
#ifdef NOTDEF
extern char		*DviCharName ( /* DviCharNameMap *map, int index, int synonym */ );
#else
#define DviCharName(map,index,synonym)	((map)->dvi_names[index][synonym])
#endif
extern int		DviCharIndex ( /* DviCharNameMap *map, char *name */ );
extern unsigned char	*DviCharIsLigature ( /* DviCharNameMap *map, char *name */ );
extern void		ResetFonts ( /* DviWidget dw */ );
