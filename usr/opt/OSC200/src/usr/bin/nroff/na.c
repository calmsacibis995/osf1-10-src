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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: na.c,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/10/14 04:17:51 $";
#endif
/*
 * HISTORY
 */
/*
 *  NROFF
 *
 * +++
 *
 *	Modification History
 *	--------------------
 *
 *	Code	      By	  Date	     Country	    Version
 *	----	      --	  ----	      ------	    -------
 *	WL001      W.M. Long    10/24/91      Asian	     V4.2A
 *
 *	This module contains routines that are specific to the Asian nroff.
 *
 *	 - wchar_t Decode		(wchar_t ch)
 *	 - wchar_t Encode		(wchar_t ch)
 *	 - int     Display_Width	(wchar_t ch)
 *	 - char   *Gen_Codeptr		(wchar_t ch)
 *	 - void    Get_Display_Status	()
 *	 - wchar_t Translate		(wchar_t ch)
 *	 - void	   Xlate_Map		(wchar_t source, wchar_t destination)
 *	 - int	   Pair			(wchar_t ch1, int ch2)
 *	 - int     Get_First_Of_Pair	(int ch)
 *	 - int     Get_Second_Of_Pair	(int ch)
 *	 - bool    Is_MByte_Space  	(wchar_t ch)
 *	 - void    Init_Stack		()
 *	 - void    Inc_Stack_Level	()
 *	 - void    Dec_Stack_Level	()
 *	 - void    Flush_Stack		()
 *	 - void    Push_To_Stack     	(wchar_t ch)
 *	 - wchar_t Pop_From_Stack	()
 *	 - int	   wc_comp		(wchar_t *a,*b)
 *	 - int	   CanSpaceBefore	(wchar_t wc)
 *	 - int	   CanSpaceAfter	(wchar_t wc)
 *	 - int	   IsNoFirstChar	(wchar_t wc)
 *	 - int	   IsNoLastChar		(wchar_t wc)
 *	 - void	   Check_NoFirst_NoLast_Char	()
 *	 - void    InitOneTable		(char *tmpstr, wchar_t table, 
 *						int mb_size, int *tab_sizep)
 *	 - void    InitAsianTables	(nl_catd catd)
 * --- 
 */


/*=========================[ INCLUDE FILES ]============================*/


#include "tdef.h"
#include "d.h"

/*=======================[ MACROS DEFINITION ]==========================*/

#define XTAB_SIZE	256	/* Size of translation table */
#define HYPHEN		0200	/* Hyphen character	     */
#define DASH		0203	/* 3/4 M dash		     */
#define	INFINITY	0x10000000
#define	INDEX_MASK	0x7f	/* Mask for leading code index	*/
#define	BIT15_MASK	0x8000	/* Mask for bit 15		*/
#define	BIT7_MASK	0x80	/* Mask for bit 7		*/
#define	BYTE_SHIFT	8
#define	MAX_INDEX	INDEX_MASK	/* Maximum leading code index	*/
#define CH_CL_SIZE	128	/* Size of no first and last char tables */

/*========================[ TYPE DEFINITIONS ]==========================*/

typedef short		bool   ;
typedef struct
{
    wchar_t	src  ;	/* Translation source 	   */
    wchar_t	dest ;  /* Translation destination */
} xtab ;		/* Translation table	   */

/*========================[ LOCAL VARIABLES ]===========================*/

static xtab	translate_table1[XTAB_SIZE] ; /* Asian to Asian or non-Asian  */
static wchar_t	translate_table2[XTAB_SIZE] ; /* Non-Asian to Asian	      */
static int	translate_count1 = 0        ; /* # of valid entries in table1 */
static short	leadcode_count   = 1	    ; /* Number of leading code entry */
static ushort	leadcode_table[128]	    ; /* Leading code table	      */
static wchar_t	nofirst_tab1[CH_CL_SIZE]    ; /* No first characters, ASIAN   */
static wchar_t  nolast_tab1[CH_CL_SIZE]     ; /* No last characters, ASIAN    */
static int	nofirst_size1, nolast_size1 ; /* char table sizes , ASIAN     */
static wchar_t	nofirst_tab2[CH_CL_SIZE]    ; /* No first characters, ASCII   */
static wchar_t  nolast_tab2[CH_CL_SIZE]     ; /* No last characters, ASCII    */
static int	nofirst_size2, nolast_size2 ; /* char table sizes, ASCII      */
static wchar_t	unofirst_tab[CH_CL_SIZE]={0}; /* user no first char class     */
static wchar_t  unolast_tab[CH_CL_SIZE]={0} ; /* user no last char class      */
static int      unofirst_size=0             ; /* user char class sizes        */
static int	unolast_size=0              ; /* user char class sizes        */
static wchar_t	sbefore_tab[CH_CL_SIZE]     ; /* canbe space before tab       */
static wchar_t  safter_tab[CH_CL_SIZE]      ; /* canbe space after tab        */
static int	sbefore_size,safter_size    ; /* canbe space tables sizes     */
static wchar_t	left_tab[CH_CL_SIZE]        ; /* left letter tab              */
static wchar_t  right_tab[CH_CL_SIZE]       ; /* right letter tab             */
static int	right_size,left_size	    ; /* right/left letter tab sizes  */
static int	word_buffer[CH_CL_SIZE]={0} ; /* word buffer                  */
static int	wcword_buffer[CH_CL_SIZE]={0};/* ????? */
static int 	*wordp2, *wcwordp2	    ; /* word buffer pointers 	      */

/*
 * The followings are applicable to Kanji only.  Other countries will
 * also check them but they will not be changed at all.
 */
static int	restrict_rules=1	    ; /* turn first/last rules on/off */
static int	user_restrict_rule=0	    ; /* use user/system restrict char*/

/*========================[ GLOBAL VARIABLES ]==========================*/

/* Stack for saving chars that violate no-first or no-last restrictions */
PeekStack 	peek 			  ;
int		is_flushing_stack = FALSE ;
int		check_trap	  = FALSE ;

/*======================[ EXTERNAL VARIABLES ]==========================*/

extern uchar	trtab[] ; /* Non-Asian to non-Asian translation table 	*/
extern wchar_t	word [] ; /* Word buffer				*/
extern wchar_t	line [] ; /* Line buffer				*/
extern wchar_t *wordp   ; /* Word buffer pointer			*/
extern wchar_t *linep   ; /* Line buffer pointer			*/
extern wchar_t *pendw   ; /* Pointer to pending word			*/
extern int	wch     ; /* Number of characters in word buffer	*/
extern int	nc	; /* Number of characters in line buffer	*/
extern int	wne	; /* Width of all characters in word buffer	*/
extern int	ne	; /* Width of all characters in line buffer	*/
extern int	nel	; /* Amount of empty space available in line[]	*/
extern int	nwd	; /* Number of words in line buffer		*/
extern int	ch	; /* Peek character				*/
extern int	spflg   ; /* Space flag					*/
extern nl_catd  catd    ; /* catalog file descriptor                    */
extern int	chbits  ; /* char bit for encoding			*/
extern int	mb_lang ;

/*======================[ EXTERNAL FUNCTIONS ]==========================*/

extern char    *getenv() ;
extern wchar_t  getch0() ;

/*==============================[ MACROS]===============================*/
/* Checks whether a wchar can be found in a table. */
#define SEARCH_TAB(wc,tab,tab_size)			\
{							\
    register int i;					\
    if (wc>=tab[0] && wc<=tab[tab_size-1]) {		\
	for(i=0;tab[i]<wc;i++);				\
	return (tab[i]==wc) ? 1 : 0;			\
    }							\
    else return 0;					\
}
/*=====================[ FUNCTIONS START HERE ]=========================*/

/*=========================================================================
 *
 *		Asian Character Encoding Scheme
 *		-------------------------------
 *
 *	The CE version of nroff uses only the lower 16 bits of an 
 *	integer to store a character. The lower byte stores the ASCII
 *	code, whereas the upper byte stores some control bits.
 *
 *	The current scheme is to utilize the upper 16 bits as well to store the
 *	Asian characters. This scheme provides only 3 bytes for storage
 *	wheras a wide character can be 4-byte long. So I have to encode
 *	the 4-byte Asian wide character to 3-byte codes.
 *
 *	The upper 16 bits of an integer code is used to store the lower two 
 *	bytes of the Asian wide character read. The upper two bytes of
 *	the Asian wide character is stored indirectly in a 128-entry
 *	lookup table with the index to this table stored in the lower 8
 *	bit of the integer code.
 *
 *	bit 00-06 : index to the leading code table.
 *	bit 07    : bit 15 of the original word
 *	bit 08    : ZBIT - zero width bit
 *	bit 09-10 : character font 	 if bit 15 = 0
 *	bit 11-14 : character point size if bit 15 = 0
 *	bit 13    : negative motion indicator if bit 15 = 1
 *	bit 14    : vertical motion indicator if bit 15 = 1
 *	bit 15    : motion character indicator bit
 *	bit 16-30 : lower 15 bits of the Asian character
 *
 *	N.B. It is implicitly assumed that the number of different possible
 *	     leading code is within 127 limit or the program cannot handle it.
 *
 *=========================================================================
 */

/*
 * Name : wchar_t Encode(wchar_t wc)
 *
 * This routine encodes the given wide character into the format listed 
 * above and returns the encoded character.
 */
wchar_t
Encode(wc)
register wchar_t wc ;
{
    wchar_t	lower_word ;
    wchar_t	upper_word ;
    int		bit15_set  ;
    register	int index;

    if (!mb_lang) return (wc);

    if (wc <= 0x7f)
	return(wc) ;	/* 7 bit code, so no change */

    bit15_set  = (wc & BIT15_MASK  ) ;
    lower_word = (wc & LO_CHAR_MASK) ;
    upper_word = (wc & HI_WORD_MASK) >> WORD_SHIFT ;

    /* leadcode_table[0] always = 0 */
    index=0;
    if (upper_word != 0)
    {
	for (index = 1 ; index < leadcode_count ; index++)
	    if (leadcode_table[index] == upper_word)
		break ;
    }
    if (index > MAX_INDEX)
    {
	prstr("Too many different leading code\n") ;
	exit (1) ;
    }
    else if (index == leadcode_count)
	leadcode_table[leadcode_count++] = upper_word ;
    return((lower_word << WORD_SHIFT) | index | (bit15_set ? BIT7_MASK : 0)) ;
}

/*
 * Name : wchar_t Decode(wchar_t wc)
 *
 * This routine decodes the given internal wide character of the format listed 
 * above to a format recognized by other wchar_t functions and returns 
 * that decoded character.
 */
wchar_t
Decode(wc)
register wchar_t wc ;
{
    wchar_t	lower_word ;
    wchar_t	index      ;
    int		bit7_set   ;

    lower_word = (wc & HI_WORD_MASK) >> WORD_SHIFT ;
    index      = (wc & INDEX_MASK  ) ;
    bit7_set   = (wc & BIT7_MASK   ) ;

    if (!mb_lang) return(wc);	/* efficiency */

    if (lower_word == 0)
	return(index) ;	/* One byte code only */
    if (index >= leadcode_count)
	return(-1) ;	/* Invalid input code */
    return(lower_word | (leadcode_table[index] << WORD_SHIFT) | 
	  (bit7_set ? BIT15_MASK : 0)) ;
}

/*
 * This routine returns the display width of the given encoded character in 
 * number of columns.
 */
int
Display_Width(ch)
wchar_t	ch ;
{
    if (!IS_ENCODED_ACHAR(ch))
	return(1) ;
    else
	return(wcwidth(Decode(ch))) ;
}

/*
 * This routine accepts an Asian character as parameter and constructs
 * a string having the same format as the entries in codetab[]. It 
 * returns a pointer to that string as a result.
 *
 * RETURN : 0 if ch is not an Asian character
 *	    pointer to a codetab[] entry simulation string
 *
 */
char *
Gen_Codeptr(ch)
wchar_t	ch ;
{
    static char	codetab_entry[MB_MAX + 2] ;
    char       *char_ptr = codetab_entry	  ;
    short	char_size			  ;

    if (IS_ENCODED_ACHAR(ch))
    {
	char_size   = Display_Width(ch)    ;
	ch 	    = Decode(ch)	   ;
	*char_ptr++ = char_size | 0x80     ;
	char_size   = wctomb(char_ptr, ch) ;
	if (char_size >= 0)
	    char_ptr[char_size] = '\0'	   ;
	return (codetab_entry)		   ;
    }
    return (NULL) ;
}

/*
 * This function translates the input character according to the setting of
 * the translation tables trtab[], translate_table1[] & translate_table2[].
 *
 *   trtab[] maps a non-Asian character to another non-Asian character.
 *   translate_table1[] maps an Asian character to another Asian character 
 *   or a non-Asian character.
 *   translate_table2[] maps a non-Asian character to an Asian character.
 * 
 * RETURN : The original character if translation is not possible, or
 *	    the translated character.
 */
wchar_t
Translate(ch)
wchar_t	ch ;	/* Character to be translated, has been masked by CMASK */
{
    wchar_t pure_ch = ch & CMASK ; /* Char without attribute bits */

    if (IS_ENCODED_ACHAR(pure_ch))
    {
    	register	int	    index;
	for (index = 0 ; index < translate_count1 ; index++)
	    if (translate_table1[index].src == pure_ch)
		break ;
	if (index < translate_count1)
	    return (translate_table1[index].dest) ;
	else
	    return (ch) ;
    }
    else if (translate_table2[pure_ch] != 0)
    {
	return ((wchar_t) translate_table2[pure_ch]);
    }
    else
    {
	return (trtab[pure_ch] & BMASK);
    }
}

/*
 * This functions maps the given source character to the destination character.
 * Depending on the types of source and destination, different translation
 * tables are used.
 * translate_table1[] - map an Asian character to another Asian or a non-Asian
 *			character
 * translate_table2[] - map a non-Asian character to an Asian character
 * trtab[]	      - map a non-Asian character to another non-Asian character
 */
void
Xlate_Map(source, destination)
register wchar_t	source      ;	/* Character to be mapped	 */
register wchar_t	destination ;	/* Character to be translated to */
{
    register	index ;

    if (IS_ENCODED_ACHAR(source) || IS_ENCODED_ACHAR(destination))
    {
	if (IS_ENCODED_ACHAR(source))
	{
	    /* Check to see if source has been mapped before */
	    for (index = 0 ; index < translate_count1 ; index++)
		if (source == translate_table1[index].src) break ;

	    if (index >= XTAB_SIZE)
	    {	/* Translation table full */
		prstr("Translation table overflow.\n")  ;
	    }
	    else if (index < translate_count1)
	    {
		if (source != destination) /* Add mapping to table */
		    translate_table1[index].dest = destination ;
		else if (index == translate_count1 - 1)
		    translate_count1-- ;   /* Remove mapping from table */
		else
		{   /* Remove current structure pointed by index & dec count */
		    translate_table1[index] = translate_table1[--translate_count1] ;
		}
	    }
	    else if (source != destination) /* Source not mapped before */
	    {
		translate_table1[index].src  = source	   ;
		translate_table1[index].dest = destination ;
		translate_count1++			   ;
	    }
	}
	else /* Source is not an Asian character, but destination is */
	    translate_table2[source] = destination ;
    }
    else /* Both source and destination are not Asian character */
    {
	trtab		[source] = destination	;
	translate_table2[source] = 0		; /* Remove mapping to Asian */
    }

    return ;
}

/*
 * This function constructs a character pair from the two given input
 * characters.
 *
 * RETURN : The character pair formed.
 */
wchar_t
Pair(ch1, ch2)
wchar_t ch1 ;	/* The first  character of the pair */
wchar_t ch2 ;	/* The second character of the pair */
{
    /* Decode Asian characters first */
    if (IS_ENCODED_ACHAR(ch1))
	ch1 = Decode(ch1) ;
    if (IS_ENCODED_ACHAR(ch2))
	ch2 = Decode(ch2) ;
    return (FORM_PAIR(ch1, ch2)) ;
}

/*
 * This function returns the first character in the given character pair.
 */
wchar_t
Get_First_Of_Pair(pair_ch)
wchar_t	pair_ch ;
{
    register wchar_t	ch ;

    ch = FIRST_OF_PAIR(pair_ch) ;
    if (IS_ACHAR(ch))
	return (Encode(ch)) ;
    else
	return (ch)	    ;
}

/*
 * This function returns the second character in the given character pair.
 */
wchar_t
Get_Second_Of_Pair(pair_ch)
wchar_t	pair_ch ;
{
    register wchar_t	ch ;

    ch = SECOND_OF_PAIR(pair_ch) ;
    if (IS_ACHAR(ch))
	return (Encode(ch)) ;
    else
	return (ch)	    ;
}


/*
 * This function returns TRUE if the given character is an encoded multi-byte
 * space character. Otherwise it returns 0.
 */
bool
Is_MByte_Space(ch)
register wchar_t	ch ; 	/* Encoded Asian or non-Asian character */
{
    if (!IS_ENCODED_ACHAR(ch))
	return(FALSE) ;
    else
    	return(iswspace(Decode(ch))) ;
}

/*
 * Initialize peek stack
 */
void
Init_Stack()
{
    register int level ;

    peek.cur_slevel = 0	         ;
    peek.cur_sbase  = peek.stack ;
    peek.cur_sptr   = peek.stack ;
}

/*
 * Increment peek stack level
 */
void
Inc_Stack_Level(frame_ptr)
struct s *frame_ptr ;
{
    if (check_trap == FALSE) return ;
    if (peek.cur_slevel >= MAX_SLEVEL)
    {
	prstr("Peek stack level overflow.\n") ;
	return				 ;
    }

    peek.sbase[peek.cur_slevel] = peek.cur_sbase ; /* Save pointers */
    peek.sptr [peek.cur_slevel] = peek.cur_sptr  ;
    peek.frame[peek.cur_slevel] = frame_ptr	 ;
    peek.cur_sbase 		= peek.cur_sptr  ; /* Create new empty stack */
    peek.cur_slevel++				 ;
    if (*(peek.cur_sptr - 1) == ASIAN_NL_PAD)
    {	/* Remember the newline */
	*peek.cur_sptr++ = ASIAN_NL_PAD ;
    }
}

/*
 * Decrement peek stack level
 */
void
Dec_Stack_Level(frame_ptr)
struct s *frame_ptr ;
{
    if (peek.cur_slevel == 0)
	return ;
    else if (peek.frame[peek.cur_slevel - 1] != frame_ptr)
	return ;

    peek.cur_slevel-- ;
    peek.cur_sbase = peek.sbase[peek.cur_slevel] ; /* Restore pointers */
    peek.cur_sptr  = peek.sptr [peek.cur_slevel] ;
}

/*
 * Flush the content of the peek stack to line buffer.
 * Note that all the characters in the peek stack is flushed to one line 
 * even if the resulting line is too long. There is no need to check
 * no-first and no-last character as a line break will be forced later on.
 */
void
Flush_Stack()
{
    setnel() 			 ;
    is_flushing_stack = TRUE     ;
    nel 	     += INFINITY ; /* Set to practically infinity */
    while (getword(0) == 0)
    {
	movword() ;
    }
    is_flushing_stack = FALSE ;
    peek.cur_sptr = peek.cur_sbase ; /* Empty stack */
    nel = (nel > INFINITY) ? nel - INFINITY : 0 ;
}

#if 0
/* changed to macro and moved to tdef.h */

/*
 * This routine pushes the given character onto the peek stack.
 */
void
Push_To_Stack(ch)
wchar_t	ch ;
{
    if (peek.cur_sptr >= &peek.stack[LNSIZE])
    {
	prstr("Peek stack overflow.\n") ;
	return				;
    }

    *peek.cur_sptr++ = ch ;
}
#endif

/* 
 * This routine pops a character off the peek stack. It returns 0
 * if the stack is empty.
 */
wchar_t
Pop_From_Stack()
{
    if (peek.cur_sptr == peek.cur_sbase)
	return (0) ;
    else 
	return (*--peek.cur_sptr) ;
}


/*
 * This routines compares betwwen two wchars in the tables.
 * I don't concern the language specific collating order,
 * so just the value is important.
 */
static int wc_comp(a,b)
wchar_t *a, *b;
{
	return	(*a<*b ? -1 : (*a>*b ? 1  : 0));
}


/*
 * This routines checks whether a wchar can be inserted space before.  Return
 * 0 if not, else pointer to the wchar.
 */
int CanSpaceBefore(wc)
wchar_t wc;
{
	SEARCH_TAB(wc, sbefore_tab, sbefore_size);
}


/*
 * This routines checks whether a wchar can be inserted space after.  Return
 * 0 if not, else pointer to the wchar.
 */
int CanSpaceAfter(wc)
wchar_t wc;
{
	SEARCH_TAB(wc, safter_tab, safter_size);
}


/*
 * This routines checks whether a wchar is a no first char. Returnes 0 if not,
 * else 1
 */
int IsNoFirstChar(wc)
wchar_t wc;
{
    if (!restrict_rules) return 0;
    if (user_restrict_rule)
	SEARCH_TAB(wc, unofirst_tab, unofirst_size)
    else {
	register int i;

    	if (wc>=nofirst_tab1[0] && wc<=nofirst_tab1[nofirst_size1-1]) {
		for (i=0; nofirst_tab1[i]<wc; i++);
		return (nofirst_tab1[i]==wc) ? 1 : 0;
	}
        else if (wc>=nofirst_tab2[0] && wc<=nofirst_tab2[nofirst_size2-1]) {
		for (i=0; nofirst_tab2[0]<wc; i++);
		return (nofirst_tab2[i]==wc) ? 1 : 0;
	}
        else return 0;
    }
}


/*
 * This routines checks whether a wchar is a no last char. Returnes 0 if not,
 * else 1
 */
int IsNoLastChar(wc)
wchar_t wc;
{
    if (!restrict_rules) return 0;
    if (user_restrict_rule)
	SEARCH_TAB(wc, unolast_tab, unolast_size)
    else {
	register int i;

    	if (wc>=nolast_tab1[0] && wc<=nolast_tab1[nolast_size1-1]) {
		for (i=0; nolast_tab1[i]<wc; i++);
		return (nolast_tab1[i]==wc) ? 1 : 0;
	}
        else if (wc>=nolast_tab2[0] && wc<=nolast_tab2[nolast_size2-1]) {
		for (i=0; nolast_tab2[0]<wc; i++);
		return (nolast_tab2[i]==wc) ? 1 : 0;
	}
        else return 0;
    }
}



/*
 * This routine checks the if the line break boundary contains a no-first
 * or no-last character. If it is the case, this function will push back
 * the illegal characters onto the peek stack until a legal line break 
 * boundary is found.
 */
void
Check_NoFirst_NoLast_Char()
{
    register wchar_t *last_char_ptr ; /* Pointer to last char of current line */
    register wchar_t *first_char_ptr; /* Pointer to first char of next line   */
    wchar_t          *last_ptr_save ;
    wchar_t          *first_ptr_save;
    wchar_t          *iptr	    ;
    int	              char_width    ; /* Width of a character		      */
    int		      index	    ;
    bool	      space_state   ; /* State variable for dec nwd correctly */
    static wchar_t    zero[1] = {0} ;
    int		      prev_achar    ; /* prev char is an Asian one 	      */
    int		      curr_achar    ; /* curr char is an Asian one	      */

    if (nc <= 0) return ;	/* Line empty */

    /* Give initial value to first_char_ptr & last_char_ptr */
    if (wch == 0)
	first_char_ptr = zero ;
    else
    {
	for (index = 0 ; index < wch ; index++)
	    if (((wordp[index] & CMASK) != ' ') && !Is_MByte_Space(word[index]))
		break ;
	first_char_ptr = &wordp[index] ;
    }
    last_char_ptr = linep - 1 ;

    /* Find a line break point where the previous char is not a no-last */
    /* char and the next char is not a no-first char			*/
    while ((last_char_ptr > line)	       &&
	  ((IS_ENCODED_ACHAR(*first_char_ptr)  && 
	    IsNoFirstChar(*first_char_ptr)) ||
	   (IS_ENCODED_ACHAR(*last_char_ptr )  &&
	    IsNoLastChar(*last_char_ptr))))
    {
/* Shift both ptrs backwards to point to the previous non-space chars */
	last_ptr_save  = last_char_ptr  ;
	first_ptr_save = first_char_ptr ;
	first_char_ptr = last_char_ptr  ;
skip_space:
	while ((last_char_ptr > line) 		   && 
	      (((*--last_char_ptr & CMASK) == ' ') ||
		Is_MByte_Space(*last_char_ptr)))
	    continue ;	/* Skip spaces & 2-byte spaces */

	if ((first_char_ptr == last_char_ptr + 1) &&
	    !IS_ENCODED_ACHAR(*first_char_ptr)    &&
	    !IS_ENCODED_ACHAR(*last_char_ptr))
	{   /*
	     * If no space between last char of current line & first char 
	     * of next line, we have to check if both are non-space ASCII
	     * character. If so, we have to move back further until we find
	     * a true word boundary or hypenation point for line breaking.
	     */
	    while ((last_char_ptr > line) 	        && 
		   !IS_ENCODED_ACHAR(*first_char_ptr  ) && 
		   !IS_ENCODED_ACHAR(*last_char_ptr   ) &&
		   ((*last_char_ptr & CMASK) != '-'   ) &&
		   ((*last_char_ptr & CMASK) != HYPHEN) &&
		   ((*last_char_ptr & CMASK) != DASH  ) &&
		   ((*last_char_ptr & CMASK) != IMP   ))
	    {
		first_char_ptr--, last_char_ptr-- ;	/* Move back 1 char */
		if (((*last_char_ptr & CMASK) == ' ') ||
		       Is_MByte_Space(*last_char_ptr))
		    goto skip_space ;
	    }
	    if (last_char_ptr == line)
	    {   /*
		 * The ASCII word is too long, we can't do anything.
		 * So just leave it as it is.
		 */
		last_char_ptr  = last_ptr_save  ; /* Restore pointers */
		first_char_ptr = first_ptr_save ;
		break				; /* Quit checking    */
	    }
	    if (((*last_char_ptr       & CMASK) == IMP) && 
		((*(last_char_ptr - 1) & CMASK) != '-'))
	    {
		*last_char_ptr = (*(last_char_ptr - 1) & ~CMASK) | HYPHEN ;
		char_width = width(*last_char_ptr) ; /* Adjust nel & ne */
		nel	  -= char_width		   ;
		ne	  += char_width		   ;
	    }
	}
    }

    if (pendw == 0)
    {
	/* Push the character in word buffer onto the peek stack */
	while (wch > 0)
	    Push_To_Stack(wordp[--wch]) ;
	wordp = word ;
	wne   = 0    ;	/* Word buffer empty now */
	spflg = 0    ;  /* Reset space flag	 */
    }

    if (last_char_ptr != linep - 1)
    {	/* Line break point has been moved -  */
	/* save unused chars to be read again */
	space_state = TRUE ;	/* Start as if a space is read  */
	curr_achar = IS_ENCODED_ACHAR(*linep-1);
	for (iptr = linep - 1 ; iptr > last_char_ptr ; iptr--) 
	{
/* Deduct "virtual spaces" introduced by CanSpaceBefore/After chars */
	    prev_achar = curr_achar;
	    curr_achar = IS_ENCODED_ACHAR(*iptr);
	    if ((curr_achar && CanSpaceAfter(*iptr)) ||
	        (prev_achar && !curr_achar) ||
		(!prev_achar && curr_achar)) nwd--;
	    if (curr_achar && CanSpaceBefore(*iptr)) nwd--;

	    char_width = width(*iptr) ;
	    nel       += char_width   ;
	    ne        -= char_width   ;
	    nc	      -= 1	      ;
	    Push_To_Stack(*iptr)      ;
	    if (((*iptr & CMASK) == ' ') || Is_MByte_Space(*iptr))
	    {
		if (space_state == FALSE)
		    nwd-- ; /* Push a whole word already, dec word count */
		space_state = TRUE ;
	    }
	    else
	    {
		space_state = FALSE ;
	    }
	}
	linep  = last_char_ptr + 1 ;
	*linep = 0		   ;
/* check the last char and remove trailing spaces */
        prev_achar = curr_achar;
        curr_achar = IS_ENCODED_ACHAR(*last_char_ptr);
        if ((curr_achar && CanSpaceAfter(*last_char_ptr)) ||
            (prev_achar && !curr_achar) ||
       	    (!prev_achar && curr_achar)) nwd--; 
    }
    else if (CanSpaceAfter(*last_char_ptr)) nwd--;
/* check the last char and remove trailing spaces */
}


/*
 * This routine initialize one nofirst/nolast table and set the parameters.
 * return value : size of the table initialized
 */
int InitOneTable(catd, table, mb_size, cat_set, cat_id)
nl_catd catd;	/* catalog file to use */
wchar_t *table;	/* table to initialize */
int mb_size;	/* byte size of the mb strings */
int cat_set, cat_id;	/* cat set and id in cat file */
{
	char *cp;
	int i=0;
	char s[5];
	char *tmpstr;
	wchar_t wc;
	int j;

	table[0]=0;
 	tmpstr = catgets(catd, cat_set, cat_id, "");
#ifdef	DEBUG
	if (!(*tmpstr)) {
	    prstr(catgets(catd, 1, 5, "Cannot open "));
	    prstr("nroff.cat"); prstr("\n");
	}
	else {		/* convert byte strings to wc table */
#endif	/* DEBUG */
	    s[mb_size]='\0';
	    j = mb_size;
	    for (cp=tmpstr; *cp; cp+=j) {
	        bcopy(cp,s,mb_size);
	        if ((j = mbtowc(&wc, s, MB_CUR_MAX)) <= 0) break;
		table[i++] = Encode(wc) | chbits;
	    }
	    table[i]=0;
	    qsort((void *)table, i, sizeof(wchar_t), wc_comp);
#ifdef	DEBUG
	}
#endif	/* DEBUG */
	return i;			/* table size */
}


/*
 * Initialize all tables and parameters.
 *
 * ASIAN no first:	nofirst_tab1
 * ASIAN no last:	nolast_tab1
 * ASCII no first:	nofirst_tab2
 * ASCII no last:	nolast_tab2
 * Space before:	sbefore_tab
 * Space after:		safter_tab
 * Left letter:		left_tab
 * Right letter:	right_tab
 */
void InitAsianTables(catd)
nl_catd catd;
{
/*
 * The following table info should be present in all locales,
 * so report errors (report_flag=1) if error in catgets
 */

/* Asian nofirst and nolast chars, byte size 2 */
	nofirst_size1	= InitOneTable(catd, nofirst_tab1, 2, 2, 1);
	nolast_size1	= InitOneTable(catd, nolast_tab1, 2, 2, 2);

/* ASCII nofirst and nolast chars, byte size 1 */
	nofirst_size2	= InitOneTable(catd, nofirst_tab2, 1, 2, 3);
	nolast_size2	= InitOneTable(catd, nolast_tab2, 1, 2, 4);

/* Can insert space before/after tables */
	sbefore_size	= InitOneTable(catd, sbefore_tab, 2, 2, 5);
	safter_size	= InitOneTable(catd, safter_tab, 2, 2, 6);
/*
	left_size	= InitOneTable(catd, left_tab, 2, 2, 7);
	right_size	= InitOneTable(catd, right_tab, 2, 2, 8);
*/
}


/*
 * The following routines are specific to Kanji cases
 */
caseki() { restrict_rules = TRUE; }
caseko() { restrict_rules = FALSE; }
casekl() {
	wchar_t i, delim, wdelim, *tblp;

	while ((((i=gettch())&CMASK) == ' ') || Is_MByte_Space(i));
	if ((delim=i) & MOT) return;
	if ((delim&CMASK)=='\n') {
		user_restrict_rule=FALSE;
		return;
	}else {
		user_restrict_rule=TRUE;
		tblp = unofirst_tab;
		while (((i=gettch()) != delim) && ((i&CMASK)!='\n')) 
			*tblp++ = i;
		*tblp=0;
		unofirst_size=tblp-unofirst_tab;
		if ((i&CMASK)=='\n') return;
		tblp = unolast_tab;
		while (((i=gettch()) != delim) && ((i&CMASK)!='\n'))
			*tblp++ = i;
		*tblp=0;
		unolast_size=tblp-unolast_tab;
		if ((i&CMASK)=='\n') return;
		if ((gettch()&CMASK) != '\n') return;
	}
}
