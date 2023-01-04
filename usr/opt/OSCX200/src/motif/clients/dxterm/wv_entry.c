/* #module WV_ENTRY "X03-316" */
/*
 *  Title:	WV_ENTRY
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1985, 1993                                                 |
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
 *	Main entry to the VT220 parser.
 *
 *	prtcon		When print controller mode, send char to printer
 *	ansi_display	Logical display context update routine
 *	fill_buffer	Rebuild an escape sequence for the printer
 *	xinsertchar	Insert a character into the display (the hard way)
 *	xtranslatechar	Find a character's character set and display code
 *
 *  Author:	Frederick G. Kleinsorge
 *		Low-End Workstation Graphics Engineering
 *
 * Revision history:
 *
 *  Eric Osman		 4-Oct-1993	BL-E
 *	- Add DECTERM_SHOW_PARSING to optionally show characters as they
 *	  are parsed.
 *
 *  Grace Chung         15-Sep-1993     BL-E
 *      - Add 7-bit/8-bit printer support
 *	  by fixing send_to_pr for 7 bit printer controller mode
 *
 *  Eric Osman		30_aug-1993	BL-D
 *	- If output stopped, exit ansi_chars in middle of parse.
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Aston Chan		12-Mar-1993	V1.2/BL2
 *	- Add Turkish/Greek support.
 *
 *  Aston Chan		29-Dec-1992	Post V1.1
 *	- Fix VT52 line/column problem.  031 (which is 25 octal) is subtracted
 *	  from the input row/column instead of 31 decimal
 *
 *  Eric Osman		25-Aug-1992	VXT V1.2
 *	- Use separate loop for prtcon mode, so that <esc>T isn't translated
 *	  to a C1 control character, and other nasty things that the the
 *	  parse-then-unparse solution was doing.
 *
 *  Eric Osman	    	12-Jun-1992	VXT V1.2
 *	- Check for correct WVT$PRINT_LINE return value so that printing
 *	  to non-printer doesn't hang the beast.
 *
 *  Alfred von Campe    02-Apr-1992     Ag/BL6.2.1
 *	- Remove unnecessary "&" that the OSF/1 compiler complained about.
 *
 *  Aston Chan		14-Feb-1992	V3.1/BL5
 *	- More of the double width/height RtoL support fix below.  Make 
 *	  the fix work even in auto_wrap mode.
 *
 *  Aston Chan		06-Feb-1992	V3.1
 *	- Bug fix by Shai.  Double size/width RtoL character display problem.
 *	  A wrong #ifdef (I18N_MULTIBYTE).
 *
 *  Aston Chan		22-Dec-1991	V3.1
 *	- Add Globaldef in front of HEB_DEC_SUPPLEMENTAL and ISO_LATIN_8.
 *
 *  Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 *  Dave Doucette	14-oct-1991	Motif 1.1
 *	- Included Michele Lien fix which removed a semicolon from an if Stmt.
 *
 *  Alfred von Campe    06-Oct-1991     Hercules/1 T0.7
 *      - Removed superfluous "&" characters and added unsigned qualifier.
 *
 * Aston Chan		1-Sep-1991	Alpha
 *	- Type casting needed inside fill_buffer() routine.  Complained by
 *	  DECC.
 *
 *  Michele Lien    	24-Aug-1990	VXT X0.0
 *	- Modify this module to work on VXT platform. Change #ifdef VMS to
 *	  #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *	  VMS or ULTRIX development environment.
 *
 *  Bob Messenger       17-Jul-1990     X3.0-5
 *      Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- two byte code parsing
 *	- handling of Asian character sets
 *
 * Bob Messenger	23-Jun-1990	X3.0-5
 *	- Support the printer port.
 *		- Call WVT$PRINT_LINE instead of print_line, because
 *		  print_line prints a specific line from the display list
 *		  rather than just flushing the print buffer.
 *	- Fix up printer controller mode.
 *
 * Bob Messenger	16-Jul-1989	X2.0-16
 *	- Test for error character in xtranslatechar.
 *
 * Bob Messenger	23-May-1989	X2.0-12
 *	- Use either the V1 or V2 font encodings, depending on whether
 *	  we're dealing with V1 or V2 fonts.
 *
 * Bob Messenger	14-May-1989	X2.0-10
 *	- Treat space and delete correctly in 94 and 96 character sets.
 *
 * Bob Messenger	11-Apr-1989	X2.0-6
 *	- Support DECRSTS for color table reports.
 *
 * Bob Messenger	 8-Apr-1989	X2.0-6
 *	- Make dec_supplemental[] and dec_technical[] readonly on VMS so this
 *	  module can be part of the shareable library.
 *
 * Bob Messenger	 1-Apr-1989	X2.0-5
 *	- Always enable OSC strings.
 *
 * Bob Messenger	24-Mar-1989	X2.0-3
 *	- Change character set conversions to use new fonts: ISO Latin 1
 *	  in one font (with three characters from MCS in C1 space) and
 *	  line drawing and DEC Technical in the DECTECH font.
 *
 * Bob Messenger	04-Mar-1989	X2.0-2
 *	- Removed support for Dutch NRC
 *
 * Bob Messenger	24-Jan-1989	X1.1-1
 *	- Support DECLKD (locator key definition)
 *
 * Bob Messenger	16-Jan-1989	X1.1-1
 *	- Moved many ld fields into common area
 *
 * Eric Osman		19-Oct-1988	BL11.2
 *	- Fix iso-latin table usage (use c-32-128 instead of c-32)
 *
 * Eric Osman		26-Sep-1988	BL10.2
 *	- Durga Rao's change:  Make char_code int instead of unsigned
 *	  char in xinsertchar.
 *
 * Tom Porcher		22-Jul-1988	X0.4-39
 *	- Remove unnecessary change for auto-wrap/batch scroll problem.
 *
 * Mike Leibow		21-Jul-1988	X0.4-38
 *	- Fix auto-wrap/batch scroll problem.
 *
 * Bob Messenger	12-Jul-1988
 *	- ansi_display now returns the number of characters that were parsed,
 *	  to support the S(T) timer function in ReGIS.
 *
 * Mike Leibow          07-Jun-1988
 *      - Prepared code for status line.  Some ld fields are now accessed
 *        by _cld instead of _ld.
 *
 *	X03-316 FGK0016	Frederick G. Kleinsorge	12-Aug-1987
 *  
 * 	o Put Bit-7 strip in VT100 and VT52 modes into inner loop.
 *
 *	X03-315 FGK0015	Frederick G. Kleinsorge	29-Jul-1987
 *  
 * 	o Clear FF/LF counter at exit from SIXEL or ReGIS
 *	  processing.  This is used to detect runaway LF
 *	  output for J. Johnson.
 *
 *	X03-314 FGK0014	Frederick G. Kleinsorge	22-Jul-1987
 *  
 * 	o Add deferred up scrolling
 *
 *	X03-301 FGK0013	Frederick G. Kleinsorge	08-May-1987
 *  
 * 	o Repair margin setting for VSSM mode.
 *
 *	V03-200 FGK0012	Frederick G. Kleinsorge	20-Apr-1987
 *  
 * 	o Mass edit symbols
 *
 *	V03-200 FGK0011	Frederick G. Kleinsorge	16-Apr-1987
 *  
 * 	o Change ld data type
 *
 *	V03-200 FGK0010	Frederick G. Kleinsorge	08-Apr-1987
 *  
 * 	o Put in feature check for ISO latin to allow disable.
 *
 *	V03-200 FGK0009	Frederick G. Kleinsorge	06-Mar-1987
 *  
 * 	o Add copy text to paste buffer case for DCS mode
 *
 *	V03-200 FGK0008	Frederick G. Kleinsorge	05-Mar-1987
 *  
 * 	o V3.2
 *
 *	X04-020 FGK0007	Frederick G. Kleinsorge	27-Feb-1987
 *  
 * 	o Clean up the control code optimization checks and force a
 *	  display update flush before executing a full ANSI parse.
 *
 *	X04-019 FGK0006	Frederick G. Kleinsorge	04-Feb-1987
 *  
 * 	o Select the correct keyboard compose tables when DECAUPSS
 *	  is executed.  Adds ISO KB support.
 *
 *	X04-018 FGK0005	Frederick G. Kleinsorge	14-Aug-1986
 *  
 * 	o Add DECAUPSS DCS sequence.  Re-write xinsertchar to make it
 *	  smaller, more efficient and to discard DEL and 255 characters
 *	  except when ISO Latin 1 is the current character set.
 *
 *	X04-017 FGK0004	Frederick G. Kleinsorge	22-Jul-1986
 *  
 * 	o Update version to X04-017
 *
 *	X04-003	FGK0003	Frederick G. Kleinsorge	17-Jul-1986
 *
 *	o Pass the ReGIS processing routine the address of the pointer
 *	  and the count -- just like the sixel routine.
 *
 *	X04-002	FGK0002	Frederick G. Kleinsorge	1-Jul-1986
 *
 *	o Pass the sixel processing routine the address of the pointer
 *	  and the count -- instead of one character at a time.
 *
 *	X04-001	FGK0001	Frederick G. Kleinsorge	1-Jun-1986
 *
 *	o Create X04-001 version
 */

#include "wv_hdr.h"

/* error character */

/* note: the character code should be character 56+128 in TECHNICAL, i.e.
   the reverse question mark symbol.  Since that character is blank in
   the fonts we were using, we use the checkerboard character in the line
   drawing set instead */

#define ERROR_CHARACTER_CODE (154)
#define ERROR_CHARACTER_SET (CRM_FONT_L)
#define ERROR_CHARACTER_EXT_SET (ONE_BYTE_SET)

/* character set encodings */

#if defined (VMS_DECTERM) || defined (VXT_DECTERM)
globaldef readonly
#endif

char dec_supplemental[] = {	/* map DEC Supplemental into ISO Latin-1.
				   0 means error character */
      0, 161, 162, 163,   0, 165,   0, 167,
    164, 169, 170, 171,   0,   0,   0,   0,
    176, 177, 178, 179,   0, 181, 182, 183,
      0, 185, 186, 187, 188, 189,   0, 191,
    192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 205, 206, 207,
      0, 209, 210, 211, 212, 213, 214, 128,
    216, 217, 218, 219, 220, 130,   0, 223,
    224, 225, 226, 227, 228, 229, 230, 231,
    232, 233, 234, 235, 236, 237, 238, 239,
      0, 241, 242, 243, 244, 245, 246, 129,
    248, 249, 250, 251, 252, 255,   0,   0
    };

#if defined (VMS_DECTERM) || defined (VXT_DECTERM)
globaldef readonly
#endif

char iso_latin_1[] = {		/* map ISO Latin 1 to DEC Supplemental in
				   the V1 encodings */
    129, 161, 162, 163, 168, 165, 130, 167,
    131, 169, 170, 171, 132, 133, 134, 135,
    176, 177, 178, 179, 136, 181, 182, 183,
    137, 185, 186, 187, 188, 189, 138, 191,
    192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 205, 206, 207,
    139, 209, 210, 211, 212, 213, 214, 140,
    216, 217, 218, 219, 220, 141, 142, 223,
    224, 225, 226, 227, 228, 229, 230, 231,
    232, 233, 234, 235, 236, 237, 238, 239,
    143, 241, 242, 243, 244, 245, 246, 144,
    248, 249, 250, 251, 252, 145, 146, 147
    };

#if defined (VMS_DECTERM) || defined (VXT_DECTERM)
globaldef readonly
#endif

char dec_technical[] = {	/* map DEC technical from GL to GR.  Undefined
				   characters map to 0 */
      0, 161, 162, 163, 164, 165, 166, 167,
    168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183,
      0,   0,   0,   0, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 205, 206, 207,
    208, 209,   0, 211,   0,   0, 214, 215,
    216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231,
    232, 233, 234, 235, 236,   0, 238, 239,
    240, 241, 242, 243, 244,   0, 246, 247,
    248, 249, 250, 251, 252, 253, 254,   0
    };

#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
globaldef readonly
#endif

char iso_latin_8[] = {
    129, 129, 162, 163, 168, 165, 130, 167,
    131, 169, 140, 171, 132, 133, 134, 135,
    176, 177, 178, 179, 136, 181, 182, 183,
    137, 185, 144, 187, 188, 189, 138, 129
    };

#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
globaldef readonly
#endif

char heb_dec_supplemental[] = {	/* map Hebrew DEC Supplemental into ISO Latin-8.
				   0 means error character */
      0, 161, 162, 163,   0, 165,   0, 167,
    164, 169, 170, 171,   0,   0,   0,   0,
    176, 177, 178, 179,   0, 181, 182, 183,
      0, 185, 186, 187, 188, 189,   0, 191,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
    224, 225, 226, 227, 228, 229, 230, 231,
    232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247,
    248, 249, 250,   0,   0,   0,   0,   0
    };

#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
globaldef readonly
#endif
char turkish_dec_supplemental[] = { /* map DEC Supplemental into ISO Latin-1.
				     0 means error character */
      0, 161, 162, 163,   0, 165,   0, 167,
    164, 169, 170, 171,   0,   0, 221,   0, /* I dot above 221 added */
    176, 177, 178, 179,   0, 181, 182, 183,
      0, 185, 186, 187, 188, 189, 253, 191, /* i w/o dot 253 added */
    192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 128, /* 208 added */
    216, 217, 218, 219, 220, 130, 222, 223, /* 222 added */
    224, 225, 226, 227, 228, 229, 230, 231,
    232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 129, /* 240 added */
    248, 249, 250, 251, 252, 255, 254,   0  /* 254 added */
    };

#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
globaldef readonly
#endif

char iso_latin_5[] = {
      0, 161, 162, 163, 164, 165, 166, 167,
    168, 169, 170, 171, 172, 173, 174, 175, /* 174 added */
    176, 177, 178, 179, 180, 181, 182, 183,
    184, 185, 186, 187, 188, 189, 190, 191, /* 190 added */
    192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215, /* 208 added */
    216, 217, 218, 219, 220, 221, 222, 223, /* 222 added */
    224, 225, 226, 227, 228, 229, 230, 231,
    232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, /* 240 added */
    248, 249, 250, 251, 252, 190, 254, 255  /* 254 added */
    };

/* character set encodings */

#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
globaldef readonly
#endif
char greek_dec_supplemental[] = { /* map DEC Supplemental into ISO Latin-1.
				     0 means error character */
      0, 161, 162, 163,   0,   0, 166, 167,
    168, 169,   0, 171, 172, 173,   0, 175,
    176, 177, 178, 179, 180, 181, 182, 183,
    184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 205, 206, 207,
    208, 209,   0, 211, 212, 213, 214, 215, 
    216, 217, 218, 219, 220, 221, 222, 223,                          
    224, 225, 226, 227, 228, 229, 230, 231,
    232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247,                          
    248, 249, 250, 251, 252, 253, 254,   0  
    };

typedef struct {
        unsigned short c83;
        unsigned short c78;
        }  CTABLE;

#ifdef  VMS
globaldef readonly
#endif

CTABLE code_table[] =           /* conversion table from JIS78 to JIS83       */
    {
    { 0xb0b3, 0xf2cd },     { 0xb2a9, 0xf2f4 },     { 0xb3c2, 0xe9da },
    { 0xb3c9, 0xd9f8 },     { 0xb3f6, 0xe3de },     { 0xb4c3, 0xdef5 },
    { 0xb4d2, 0xebdd },     { 0xb6c6, 0xf4a1 },     { 0xb7db, 0xf0f4 },
    { 0xb9dc, 0xe2e8 },     { 0xbcc9, 0xe9a2 },     { 0xbfd9, 0xf0d7 },
    { 0xc1a8, 0xeccd },     { 0xc4db, 0xd4e4 },     { 0xc5d7, 0xe2ea },
    { 0xc5ee, 0xdbed },     { 0xc5f3, 0xdeb9 },     { 0xc6f6, 0xedee },
    { 0xc7e8, 0xeaa4 },     { 0xc9b0, 0xdbd8 },     { 0xcbea, 0xf4a2 },
    { 0xcbf9, 0xd0d6 },     { 0xccf9, 0xe9ae },     { 0xcdda, 0xf4a3 },
    { 0xcfb6, 0xe4c6 },     { 0xd0d6, 0xcbf9 },     { 0xd4e4, 0xc4db },
    { 0xd9f8, 0xb3c9 },     { 0xdbd8, 0xc9b0 },     { 0xdbed, 0xc5ee },
    { 0xdeb9, 0xc5f3 },     { 0xdef5, 0xb4c3 },     { 0xe0f6, 0xf4a4 },
    { 0xe2e8, 0xb9dc },     { 0xe2ea, 0xc5d7 },     { 0xe3de, 0xb3f6 },
    { 0xe4c6, 0xcfb6 },     { 0xe9a2, 0xbcc9 },     { 0xe9ae, 0xccf9 },
    { 0xe9da, 0xb3c2 },     { 0xeaa4, 0xc7e8 },     { 0xebdd, 0xb4d2 },
    { 0xeccd, 0xc1a8 },     { 0xedee, 0xc6f6 },     { 0xf0d7, 0xbfd9 },
    { 0xf0f4, 0xb7db },     { 0xf2cd, 0xb0b3 },     { 0xf2f4, 0xb2a9 },
    { 0xf4a1, 0xa2a2 },     { 0xf4a2, 0xa2a2 },     { 0xf4a3, 0xa2a2 },
    { 0xf4a4, 0xa2a2 },     { 0xf4a4, 0xa2a2 },     { 0xfe21, 0xa8b0 },
    { 0xfe22, 0xa8af },     { 0xfe23, 0xa8ae },     { 0xfe24, 0xa8b1 },
    { 0xfe25, 0xa8b6 },     { 0xfe26, 0xa8ac },     { 0xfe27, 0xa8b2 },
    { 0xfe28, 0xa8b4 },     { 0xfe29, 0xa8b5 },     { 0xfe2a, 0xa8b3 },
    { 0xfe2b, 0xa8ad }
    };


/*
 * is_fancy_char returns true if character should be displayed in fancy way
 */
static is_fancy_char(code)
{
return (0x7f&(code))<32 || (code)==127;
}

/*
 * Here's a routine that returns a pointer to a printable version of a
 * character.  But use the pointer quickly, since a subsequent call to this
 * routine uses the same data space !
 */
static char *dressed_char (code) unsigned char code;
{
static char printable[2];

char *c0_names[32]={
	"<NUL>","<SOH>","<STX>","<ETX>","<EOT>","<ENQ>","<ACK>","<BEL>",
	"<BS>","<HT>","<LF>","<VT>","<FF>","<CR>","<SO>","<SI>",
	"<DLE>","<DC1>","<DC2>","<DC3>","<DC4>","<NAK>","<SYN>","<ETB>",
	"<CAN>","<EM>","<SUB>","<ESC>","<FS>","<GS>","<RS>","<US>"};
char *c1_names[32]={
	"<0x80>","<0x81>","<0x82>","<0x83>","<IND>","<NEL>","<SSA>","<ESA>",
	"<HTS>","<HTJ>","<VTS>","<PLD>","<PLU>","<RI>","<SS2>","<SS3>",
	"<DCS>","<PU1>","<PU2>","<STS>","<CCH>","<MW>","<SPA>","<EPA>",
	"<0x98>","<0x99>","<0x9a>","<CSI>","<ST>","<OSC>","<PM>","<APC>"};

if (is_fancy_char (code))
    if (code==127) return ("<DEL>");
    else if (code<32)
	return c0_names[code];
    else
	return c1_names[0x7f&code];
else
    {
    printable[0] = code;
    printable[1] = 0;
    return printable;
    }
}

/*
 * Here's audit_chars, a routine that outputs characters in a printable form,
 * suitable for reading in a log file.
 *
 * NOTE:  One place this is called is to implement the DECTERM_SHOW_PARSING
 * feature, which shows characters as they are parsed.  Currently, sometimes
 * decterm calls this routine twice for the same character.  This ought to
 * be cleaned up some day.
 *
 */
#define audit printf
audit_chars (message, len, data, suffix) char *message, *data, *suffix;
{
int i, s=0;
audit ("%s", message);
for (i=0; i<len; i++)
    if (is_fancy_char (data[i]))
	{
	if (i-s) audit ("%.*s", i-s, &data[s]);
	audit ("%s", dressed_char (data[i]));
	s = i + 1;
	}
if (i-s) audit ("%.*s", i-s, &data[s]);
audit ("%s", suffix);
}

#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
noshare
#endif
char show_parsing_flag;   /* if set, we show characters as we parse */

#if defined (VXT_DECTERM) || defined(VMS_DECTERM)
globaldef readonly
#endif

CTABLE code_table_reverse[] =   /* conversion table from JIS83 to JIS78       */
    {
    { 0xfe26, 0xa8ac },     { 0xfe2b, 0xa8ad },     { 0xfe23, 0xa8ae },
    { 0xfe22, 0xa8af },     { 0xfe21, 0xa8b0 },     { 0xfe24, 0xa8b1 },
    { 0xfe27, 0xa8b2 },     { 0xfe2a, 0xa8b3 },     { 0xfe28, 0xa8b4 },
    { 0xfe29, 0xa8b5 },     { 0xfe25, 0xa8b6 },     { 0xf2cd, 0xb0b3 },
    { 0xf2f4, 0xb2a9 },     { 0xe9da, 0xb3c2 },     { 0xd9f8, 0xb3c9 },
    { 0xe3de, 0xb3f6 },     { 0xdef5, 0xb4c3 },     { 0xebdd, 0xb4d2 },
    { 0xf0f4, 0xb7db },     { 0xe2e8, 0xb9dc },     { 0xe9a2, 0xbcc9 },
    { 0xf0d7, 0xbfd9 },     { 0xeccd, 0xc1a8 },     { 0xd4e4, 0xc4db },
    { 0xe2ea, 0xc5d7 },     { 0xdbed, 0xc5ee },     { 0xdeb9, 0xc5f3 },
    { 0xedee, 0xc6f6 },     { 0xeaa4, 0xc7e8 },     { 0xdbd8, 0xc9b0 },
    { 0xd0d6, 0xcbf9 },     { 0xe9ae, 0xccf9 },     { 0xe4c6, 0xcfb6 },
    { 0xcbf9, 0xd0d6 },     { 0xc4db, 0xd4e4 },     { 0xb3c9, 0xd9f8 },
    { 0xc9b0, 0xdbd8 },     { 0xc5ee, 0xdbed },     { 0xc5f3, 0xdeb9 },
    { 0xb4c3, 0xdef5 },     { 0xb9dc, 0xe2e8 },     { 0xc5d7, 0xe2ea },
    { 0xb3f6, 0xe3de },     { 0xcfb6, 0xe4c6 },     { 0xbcc9, 0xe9a2 },
    { 0xccf9, 0xe9ae },     { 0xb3c2, 0xe9da },     { 0xc7e8, 0xeaa4 },
    { 0xb4d2, 0xebdd },     { 0xc1a8, 0xeccd },     { 0xc6f6, 0xedee },
    { 0xbfd9, 0xf0d7 },     { 0xb7db, 0xf0f4 },     { 0xb0b3, 0xf2cd },
    { 0xb2a9, 0xf2f4 },     { 0xb6c6, 0xf4a1 },     { 0xcbea, 0xf4a2 },
    { 0xcdda, 0xf4a3 },     { 0xe0f6, 0xf4a4 }
    };

/*
 * Routine to send chars to printer during prtcon mode
 */
static void send_to_pr (ld, len, str)
    wvtp ld;
    char *str;
{
int iptr;
unsigned char code;
for (iptr = 0; iptr < len; iptr++)
    {
    code = str[iptr];
/* In 7 bit printer mode, C1 code is translated to ESC Fe sequence */
/* GR codes will have the eighth bit stripped                      */

    if (!(_cld wvt$w_print_flags & pf1_m_prt_transmission_mode)) 
       if ((code >= C1_IND) && (code <= C1_APC)) /* C1 code */
          {
          _cld wvt$b_work_buffer[_cld wvt$l_work_string.offset++] = C0_ESC;
          code = code - 64;
          }
       else        
          code &= 127;                           /* GR code */
    _cld wvt$b_work_buffer[_cld wvt$l_work_string.offset++] = code;
    }

if ( WVT$PRINT_LINE(ld) != DECwPrinterReady )
    {
    DECtermWidget w = ld_to_w(ld);
    _cld wvt$w_print_flags &= ~pf1_m_prt_controller_mode;
    w->common.printMode = DECwNormalPrintMode;
    }
}

/*
 * prtcon handles characters from host, when we're in printer controller
 * mode.  The rule is:
 *
 *	o	In ansi mode, only <CSI>[4i or <ESC>[4i gets us out,
 *		and in vt52 mode, only <ESC>X gets us out.  These sequences are
 *		not sent.  Hence we don't send anything until we know we're not
 *		seeing the exit sequence.
 *
 */
prtcon (ld, code)
	wvtp ld;
	unsigned char code;
#define ansi_pr_suffix '4i'
#define vt52_pr_suffix 'X'
{
char esc_seq[5] = {C0_ESC,'[',ansi_pr_suffix,'\0'};
char csi_seq[4] = {C1_CSI,ansi_pr_suffix,'\0'};
char vt52_seq[3] = {C0_ESC,vt52_pr_suffix,'\0'};

if (_cld wvt$l_vt200_flags&vt1_m_ansi_mode)
    if (_cld n_pr_exit_chars==0)
    parse:
	if (code == C0_ESC)
	    {
	    _cld n_pr_exit_chars = 1;		/* we've seen first exit char */
	    _cld wvt$w_print_flags |= pf1_m_prt_esc;	/* first is esc */
	    }
	else if (code == C1_CSI)
	    {
	    _cld n_pr_exit_chars = 1;		/* we've seen first exit char */
	    _cld wvt$w_print_flags &= ~pf1_m_prt_esc;	/* first is csi */
	    }
	else send_to_pr (ld, 1, &code);		/* not a match just send it */
    else
    if (_cld wvt$w_print_flags & pf1_m_prt_esc) switch (_cld n_pr_exit_chars)
	{
				/* somewhere in esc-style exit sequence */
	case 1:			/* we've seen esc or csi so far */
	    if (code == '[')
		{
		_cld n_pr_exit_chars = 2;	/* we have esc [ */
		break;
		}
	    else
		{
		send_to_pr (ld, 1, esc_seq);	/* esc other, send esc */
		_cld n_pr_exit_chars = 0;	/* treat other as first char */
		goto parse;			/* see if esc esc or esc csi */
		}
	case 2:			/* we know it's esc [ something */
	    if (code == '4')
		{
		_cld n_pr_exit_chars = 3;	/* we have esc [ 4 */
		break;
		}
	    else
		{
		send_to_pr (ld, 2, esc_seq);	/* send esc [ */
		_cld n_pr_exit_chars = 0;	/* see if non-4 is esc or csi */
		goto parse;
		}
	case 3:			/* it's esc [ 4 something */
	    if (code == 'i')
		{
		WVT$EXIT_PRINT_CONTROLLER_MODE (ld);
		break;
		}
	    else
		{
		send_to_pr (ld, 3, esc_seq);	/* send esc [ 4 */
		_cld n_pr_exit_chars = 0;	/* see if non-i is esc or csi */
		goto parse;
		}
	default:				/* shouldn't get here ! */
		printf (
		    "unexpected esc_flag = 1, n_pr_exit_chars = %d\n",
		    _cld n_pr_exit_chars);
		break;
	}
    else switch (_cld n_pr_exit_chars)		/* we've got csi something */
	{
	case 1:
	    if (code == '4')
		{
		_cld n_pr_exit_chars = 2;	/* we have csi 4 */
		break;
		}
	     else
		{
		send_to_pr (ld, 1, csi_seq);	/* csi other, send csi */
		_cld n_pr_exit_chars = 0;	/* treat other as first char */
		goto parse;			/* see if csi csi or csi esc */
		}
	case 2:
	    if (code == 'i')
		{
		WVT$EXIT_PRINT_CONTROLLER_MODE (ld);
		break;
		}
	    else
		{
		send_to_pr (ld, 2, csi_seq);	/* send csi 4 */
		_cld n_pr_exit_chars = 0;	/* check for esc or csi next */
		goto parse;
		}
	default:		/* shouldn't get here ! */
		printf (
		    "unexpected esc_flag = 0, n_pr_exit_chars = %d\n",
		    _cld n_pr_exit_chars);
		break;
	}
else						/* we're in vt52 mode */
    switch (_cld n_pr_exit_chars)
    {
    case 0:					/* no seq seen yet */
	if (code == C0_ESC)
	    _cld n_pr_exit_chars = 1;		/* we see esc starting seq */
	else
	    send_to_pr (ld, 1, &code);		/* non-esc */
	break;
    case 1:
	if (code == 'X')			/* is it esc X ? */
	    WVT$EXIT_PRINT_CONTROLLER_MODE (ld);/* yes */
	else					/* no, so send the esc */
	    {
	    send_to_pr (ld, 1, vt52_seq);
	    if (code != C0_ESC)			/* does another esc follow ? */
		{
		send_to_pr (ld, 1, &code);	/* no, so send it now */
		_cld n_pr_exit_chars = 0;	/* no seq in progress anymore */
		}
	    }					/* for esc, leave n=1 */
	break;
    default:				/* shouldn't get here ! */
	printf ("unexpected vt52 = 1, n_pr_exit_chars = %d\n",
	    _cld n_pr_exit_chars);
	break;
    }
}

/*****************************************************************************/
int ansi_display(ld, in_buf, in_len)    /* MAIN ENTRY POINT FOR ANSI DISPLAY */
/*****************************************************************************/


/*
 *
 *	Main entry point to parse and display output data.
 *
 */


wvtp ld;
unsigned char *in_buf;
int in_len;

{

int event, start, final, drop, temp;
unsigned char *in_ptr;
int char_code, char_code_2, char_set, ignore;
int ext_char_set;
register int x;

/* clear the detected flag before parsing, CAN (0x18), SUB (0x1a)	*/
_cld wvt$b_can_sub_detected = 0;

/* Main Loop */

for (in_ptr=in_buf; (in_ptr-in_buf)<in_len; in_ptr++)

  {
  if ((_cld stop_output) & ~STOP_OUTPUT_TEMP) /* break if anything other than
					       * STOP_OUTPUT_TEMP is set */
	{
	break;   /* common reason is a pending resize event in dt_output.  We
		  * need to stop parsing in this case.  Otherwise the resize
		  * event causes regis mode to be cleared in middle of user's
		  * regis string. */
	}
  /* show characters as parsed if requested */
  if (show_parsing_flag) audit_chars ("", 1, in_ptr, "");  


/*
 *  Control Representation Mode:
 *
 *  This is a special mode, in which we don't really execute most ESC or
 *  controls, instead we output most characters using a special character
 *  set.
 *
 *  We do execute:
 *
 *  LF	- Code representation displayed, then execute a NEW_LINE
 *  FF	- Same
 *  VT	- Same
 *  RM  - Reset Mode sequence, we will display it and then exit back to
 *	  normal mode
 *
 */

  if (( _cld wvt$l_ext_flags & vte1_m_asian_common ) &&
	( _cld wvt$l_vt200_flags & vt1_m_display_controls_mode ))
   {

    xinsertchar(ld, *in_ptr);

    if ((_ld wvt$l_actv_column != _ld wvt$l_disp_pos) ||
        (_ld wvt$l_actv_column == line_width(_ld wvt$l_actv_line)))
    display_segment(	ld,
			_ld wvt$l_actv_line,
			_ld wvt$l_disp_pos,
			_ld wvt$l_actv_column - _ld wvt$l_disp_pos);

    _ld wvt$l_disp_pos = _ld wvt$l_actv_column;
    
    if ((*in_ptr == C0_LF) || (*in_ptr == C0_FF) || (*in_ptr == C0_VT))
     {

      xnel(ld);
      _ld wvt$l_disp_pos = _ld wvt$l_actv_column;

     }

    parse_ansi(ld, *in_ptr, &event, &start, &final);

    if (!( (_cld wvt$b_last_event == R_CONTINUE) &&
       (event == R_CONTROL) )) _cld wvt$b_last_event = event;

    if (event == R_CSI_SEQ)
     {

      seqparm(ld, &_cld wvt$r_com_state, start, final);

      if (_cld wvt$b_finalchar == 108)
        if (_cld wvt$b_privparm == 0)
          if (_cld wvt$l_parms[0] == 3)
	     {

	      _cld wvt$l_vt200_flags &= ~vt1_m_display_controls_mode; /* Exit DCM */

	      WVT$RECALL_FONT(ld); /* Restore font that was current */

	     }
     }

    continue;

   }



/*
 *
 *	Normal processing (not in display controls mode).
 *
 */



    /* Strip nulls */

    if (!*in_ptr) continue;
       
    /* Now begin evaluation */

    if ( _cld wvt$l_ext_flags & vte1_m_tomcat ) {
/*--I.Seto, 01-NOV-1990                                                     --*/
/*--    To enable the 8bit codes even in VT100 mode (level 1) for Kanji and --*/
/*--    VT52 mode for Katakana display capability.                          --*/
    /* If control char in Level 1 or VT52 mode, strip high bit	*/
    if ( !( *in_ptr & 0x60 ) &&
	(( _cld wvt$b_conformance_level == LEVEL1 ) ||
	( !(_cld wvt$l_vt200_flags & vt1_m_ansi_mode ))))
        *in_ptr &= 127;
    } else {

    /* If in level 1 mode, or NOT in ANSI mode, strip high bit */

    if (_cld wvt$b_conformance_level == LEVEL1 ||
     (!(_cld wvt$l_vt200_flags & vt1_m_ansi_mode)) ||
       (_cld wvt$l_vt200_flags & vt1_m_nrc_mode) )
	*in_ptr &= 127;
    }

#if PRINTER_EXTENSION

    /* Process Print Controller Data */

    if (_cld wvt$w_print_flags & pf1_m_prt_controller_mode)
	{
	prtcon (ld, *in_ptr);
	continue;
	}
#endif



/*
 *	Special processing mode "wvt$b_in_dcs".
 *
 *	This is a catchall for processing:  DCS, PM, OSC and APC data.
 *
 *	If we recognize it, the processing routine is called.  Otherwise
 *	the IGNORE_DCS routine is used -- which simply throws away data
 *	until a <ST> is processed.
 *
 *	The "drop" flag may be set to allow a character that was just
 *	processed to be kicked out for processing by the normal parser.
 *
 */

    if (_cld wvt$b_in_dcs)

    {

      drop = FALSE;

      switch (_cld wvt$b_in_dcs)

      {

	/* Process User Preference Set select */

	case DECAUPSS:

	  if ( !( _cld wvt$l_vt200_flags_2 & vt2_m_enable_ISO_latin ) ) break;

	  switch (*in_ptr)
	  	{
		 case '"':
			if (!_cld wvt$l_parms[0])
				_cld wvt$w_udk_state = *in_ptr;
			else _cld wvt$b_in_dcs = IGNORE_DCS;
			break;

		 case '4':
			if (_cld wvt$w_udk_state == '"')
				_cld wvt$w_udk_state = *in_ptr;
			else _cld wvt$b_in_dcs = IGNORE_DCS;
			break;

		 case 'H':
			if (_cld wvt$l_parms[0])
				_cld wvt$w_udk_state = *in_ptr;
			else _cld wvt$b_in_dcs = IGNORE_DCS;
			break;
		 case '%':

			if (!_cld wvt$l_parms[0])
				_cld wvt$w_udk_state = *in_ptr;
			else _cld wvt$b_in_dcs = IGNORE_DCS;
			break;

		 case '5':

			if (_cld wvt$w_udk_state == '%')
				_cld wvt$w_udk_state = *in_ptr;
			else _cld wvt$b_in_dcs = IGNORE_DCS;
			break;

		 case 'A':	/* A for ISO LATIN 1 */
		 case 'F':	/* F for ISO Greek (Latin 7) */
		 case 'M':	/* M for ISO Turkish (Latin 5) */

			if (_cld wvt$l_parms[0])
				_cld wvt$w_udk_state = *in_ptr;
			else _cld wvt$b_in_dcs = IGNORE_DCS;
			break;

		 case '?':	/* " ? for DEC Greek */

			if (_cld wvt$w_udk_state == '"')
				_cld wvt$w_udk_state = *in_ptr;
			else _cld wvt$b_in_dcs = IGNORE_DCS;
			break;

		 case '0':	/* % 0 for DEC Turkish */

			if (_cld wvt$w_udk_state == '%')
				_cld wvt$w_udk_state = *in_ptr;
			else _cld wvt$b_in_dcs = IGNORE_DCS;
			break;

		 case  C0_ESC:
		 case  C1_ST:
			switch (_cld wvt$w_udk_state)
			    {
			    case '5' :
				_cld wvt$b_user_preference_set = SUPPLEMENTAL;
				WVT$RESET_KB_COMPOSE(ld);
				break;

			    case '4' :
				_cld wvt$b_user_preference_set =
							       HEB_SUPPLEMENTAL;
				WVT$RESET_KB_COMPOSE(ld);
				break;

			    case '?' :
				_cld wvt$b_user_preference_set = 
							GREEK_SUPPLEMENTAL;
				WVT$SET_ISO_KB_COMPOSE(ld);
				break;

			    case '0' :
				_cld wvt$b_user_preference_set = 
							TURKISH_SUPPLEMENTAL;
				WVT$SET_ISO_KB_COMPOSE(ld);
				break;

			    case 'A' :
				_cld wvt$b_user_preference_set = ISO_LATIN_1;
				WVT$SET_ISO_KB_COMPOSE(ld);
				break;

			    case 'M' :
				_cld wvt$b_user_preference_set = ISO_LATIN_5;
				WVT$SET_ISO_KB_COMPOSE(ld);
				break;

			    case 'F' :
				_cld wvt$b_user_preference_set = ISO_LATIN_7;
				WVT$SET_ISO_KB_COMPOSE(ld);
				break;

			    case 'H' :
				_cld wvt$b_user_preference_set = ISO_LATIN_8;
				WVT$SET_ISO_KB_COMPOSE(ld);
				break;

			    default :
				break;
                            }

		 case  C0_SUB:
		 case  C0_CAN:

			_cld wvt$b_in_dcs = FALSE;
			drop = TRUE;
			break;

		 default:

			parse_ansi(ld, *in_ptr, &event, &start, &final);
			_cld wvt$b_last_event = event;
			if ((event == R_CONTROL) &&
			     _cld wvt$r_com_state.data[final] == C1_ST)
					_cld wvt$b_in_dcs = FALSE;
			else _cld wvt$b_in_dcs = IGNORE_DCS;
			break;
		}
          break;

	/* Process Locator Button Definitions */

	case 119:

	  xlkd(ld,*in_ptr);
          break;


	case IN_REGIS:

          xregis(ld, &in_ptr, in_len-(in_ptr-in_buf));

	  if (!_cld wvt$b_in_dcs)
		{
		 drop = TRUE;
		}
	  else if ( in_ptr - in_buf < in_len )
		{  /* entire buffer not consumed - probably S(T) */
		return ( in_ptr - in_buf );
		}
          break;

	/* Process SIXEL graphic data */

        case SIXEL:

          xsixel(ld, &in_ptr, in_len-(in_ptr-in_buf));

	  if (!_cld wvt$b_in_dcs)
		{
		 drop = TRUE;
		}
          break;

#if DRCS_EXTENSION

	/* Process Dynamicly Redefinable Character Set data */

        case DRCS:
          xdrcs(ld,*in_ptr);
          break;
#endif

	/* Process User Defined Key data */

        case UDK:

          xudk(ld, *in_ptr);
          break;


	/* Process OSC strings (VWS window control) */

	case C1_OSC:
	
#if DEVICE_TYPE != DECTERM_DEVICE
	  if (!(_cld wvt$l_vt200_flags & vt1_m_enable_osc_strings))
	  {
	   _cld wvt$b_in_dcs = IGNORE_DCS;
	   break;
	  }
#endif

	  xosc_seq(ld, *in_ptr);
	  break;

	/* Process Request for parameter setting */

	case DECRQSS:

	  xdecrqss(ld, *in_ptr);
	  break;

	/* Copy Text to Paste Buffer */

	case DECCTPB:

	  xdecctpb(ld, *in_ptr);
	  break;

	/* Process Assign Type Font Family request (change fonts) */

	case DECATFF:

	  xdecatff(ld, *in_ptr);
	  break;

	case DECRSPS_CIR :
	  xdecrsps_cir(ld, *in_ptr);
	  break;

	case DECRSPS_TABSR :
	  xdecrsps_tabsr(ld, *in_ptr);
	  break;

	case DECRSTS :
	  xdecrsts(ld, *in_ptr);
	  break;

	case DECRSTS_CTR:	/* color table report */
	  xdecrsts_ctr(ld, *in_ptr);
	  break;

	/* Ignore the data until a String Terminator */

        case IGNORE_DCS:
        default:

          /* Parse the character stream and ignore until ST */

	  switch (*in_ptr)
	  	{
		 case  C0_ESC:
		 case  C1_ST:
		 case  C0_SUB:
		 case  C0_CAN:
			_cld wvt$b_in_dcs = FALSE;
			drop = TRUE;
			break;
                }
	  if (!drop) {
		parse_ansi(ld, *in_ptr, &event, &start, &final);
		_cld wvt$b_last_event = event;
          	if ((event == R_CONTROL) && _cld wvt$r_com_state.data[final] == C1_ST)
          	_cld wvt$b_in_dcs = FALSE;
	  }
          break;

        };

      /* End of DCS data processing, next character (unless DROP is set) */

      if (!drop) continue;

    /* End of in DCS data processing */
    };




/*
 *   Normal Data Input (output to terminal):
 *
 *   Normal ANSI processing follows.  Before we do a full ANSI parse
 *   of the incoming data, we will first see if we're not in the middle
 *   of a escape or control sequence, and the character isn't in the C0 or
 *   C1 control range.  If this is true, then the character is a wvt$b_graphic
 *   and can be inserted into the display without further parsing.  In
 *   addition, if the previous character was a wvt$b_graphic, and if we're not
 *   at the end of the line and we're not in INSERT mode, we can skip the
 *   normal insertion call and just stuff the character into the display
 *   and copy the rendition (using the current GL or GR since no single
 *   shift should be in effect).
 *
 *   Once we get into this routine, we'll stay in it as a tight loop until
 *   we run out of data or we get a non-graphic character.  This is a large
 *   performance optimization.
 *
 */

    /* Was last event anything BUT continue?  And character NOT a control? */

    /* space should be treated as a control char in two byte parsing */

    if ((_cld wvt$b_last_event != R_CONTINUE) && (0140 & *in_ptr) && 
	!(_cld wvt$b_char_stack_top && (*in_ptr == 040 || *in_ptr == 0240))
	&& !_ld wvt$b_single_shift )	/* 910913, TN400, EIC_JPN	*/
      /* Yes, character is a GRAPHIC and we're not processing a sequence */
      {

       /* Was the PREVIOUS character a GRAPHIC (insertion pointer valid)? */
       if (_ld wvt$a_cur_cod_ptr)
         {

	  /* Start a new inner loop here for speed optimization */
	  for (;(in_ptr-in_buf)<in_len; in_ptr++)
	  {
	/* show characters as parsed if requested */
	if (show_parsing_flag) audit_chars ("", 1, in_ptr, "");  

	if ( !( _cld wvt$l_ext_flags & vte1_m_tomcat )) {
/*--I.Seto, 01-NOV-1990                                                     --*/
/*--    To enable the 8bit codes even in VT100 mode (level 1) for Kanji and --*/
/*--    VT52 mode for Katakana display capability.                          --*/
	    /* Strip MSB if in VT100 or VT52 mode */
	    if (_cld wvt$b_conformance_level == LEVEL1 ||
		(!(_cld wvt$l_vt200_flags & vt1_m_ansi_mode)) ||
                (_cld wvt$l_vt200_flags & vt1_m_nrc_mode) )
		*in_ptr &= 127;

	    /* If it's a control, back up one and exit this loop */
	}

	    if ( _cld wvt$l_ext_flags & vte1_m_asian_common ) {
	    if (!( 0140 & *in_ptr ) ||
		 ( _cld wvt$b_char_stack_top &&
			( *in_ptr == 040 || *in_ptr == 0240 )))
	      {
	       *in_ptr--;
	       break;
	      }
	} else {

	    /* If it's a control, back up one and exit this loop */
	    if (!(0140 & *in_ptr))
	      {
	       *in_ptr--;
	       break;
	      }
	}

            /* Are we at the end of the line? */
	    if (( _cld wvt$l_ext_flags & vte1_m_hebrew ) &&
		( _cld wvt$l_ext_specific_flags & vte2_m_rtl ) &&
		( _ld wvt$l_actv_column <= _ld wvt$l_left_margin ))
		xinsertchar(ld, *in_ptr);
	    else if (( _cld wvt$l_ext_flags & vte1_m_hebrew &&
		       _cld wvt$l_ext_specific_flags & vte2_m_rtl ) ||
               ( _ld wvt$l_actv_column <
	       ( _cld wvt$l_ext_flags & vte1_m_asian_common ?
		 _cld wvt$l_actv_width - _cld wvt$b_char_stack_top :
		 _cld wvt$l_actv_width )))

	      /* Character insertion.  We can directly insert the character
		 into the display.  We must ignore 127 & 255 unless it's a
		 96-character set (hard coded ISO Latin-1 for now).
	      */

              {
		char_code = *in_ptr;
		if ( _cld wvt$l_vt200_flags & vt1_m_nrc_mode 
		  || _cld wvt$b_conformance_level == LEVEL1 )
		    {
		    xtranslatechar( ld, char_code, &char_code_2, &char_set,
		      &ext_char_set,
		      &ignore );
		    char_code = char_code_2;
			/* this works around a VAXC V2.3 compiler bug that
			   caused the later "char_code &= 127" not to work */
		    }
		else if ( char_code == 32 || char_code == 127 ||
		    char_code == 128 || char_code == 160 || char_code == 255 ) {
			xtranslatechar( ld, char_code, &char_code_2, &char_set,
			    &ext_char_set, &ignore );
			char_code = char_code_2;
		} else {
		    if ( _cld wvt$b_char_stack_top ) {
			char_set = _cld wvt$b_char_set;
			ext_char_set = _cld wvt$b_ext_char_set;
		    } else if ( char_code < 128 ) {
			char_set = _ld wvt$b_g_sets[_ld wvt$b_gl];
			ext_char_set = _ld wvt$b_ext_g_sets[_ld wvt$b_gl];
		    } else {
			char_set = _ld wvt$b_g_sets[_ld wvt$b_gr];
			ext_char_set = _ld wvt$b_ext_g_sets[_ld wvt$b_gr];
		    }
		    switch ( ext_char_set ) {
		    case STANDARD_SET:
			char_code &= 127;
			switch ( char_set ) {
			    case LINE_DRAWING:
				if ( ld->common.v1_encodings ) {
				    if ( char_code >= 95 && char_code <= 126 )
					char_code -= 95;
				} else
				    char_set = TECHNICAL;
		    		break;
  			    case TECHNICAL:
				char_code = dec_technical[ char_code - 32 ];
				if ( !char_code ) {
				    char_code = ERROR_CHARACTER_CODE;
				    char_set = ERROR_CHARACTER_SET;
				    ext_char_set = ERROR_CHARACTER_EXT_SET;
				}
				else if ( ld->common.v1_encodings )
				    char_code &= 127;
		    		break;
			    case ISO_LATIN_7: /* v1_encodings is not supported
					       * in Greek.
					       */
				char_code |= 128;
				break;
			    case ISO_LATIN_1:
				if ( ld->common.v1_encodings )
				    char_code = iso_latin_1[ char_code - 32 ];
				else
				    char_code |= 128;
				break;
			    case SUPPLEMENTAL:          
				char_code_2 = dec_supplemental[ char_code - 32 ];
				if ( !char_code_2 ) {
				    char_code = ERROR_CHARACTER_CODE;
				    char_set = ERROR_CHARACTER_SET;
				    ext_char_set = ERROR_CHARACTER_EXT_SET;
				} else if ( !ld->common.v1_encodings )
				    char_code = char_code_2;
				else
				    char_code |= 128;
				break;

			    case TURKISH_SUPPLEMENTAL:
				char_code_2 =
				    turkish_dec_supplemental[ char_code - 32 ];
				if ( !char_code_2 ) {
				    char_code = ERROR_CHARACTER_CODE;
				    char_set = ERROR_CHARACTER_SET;
				    ext_char_set = ERROR_CHARACTER_EXT_SET;
				} else if ( !ld->common.v1_encodings )
				    char_code = char_code_2;
				else
				    char_code |= 128;
				break;

			    case GREEK_SUPPLEMENTAL:          
				char_code_2 =
					greek_dec_supplemental[ char_code - 32 ];
				if ( !char_code_2 ) {
				    char_code = ERROR_CHARACTER_CODE;
				    char_set = ERROR_CHARACTER_SET;
				    ext_char_set = ERROR_CHARACTER_EXT_SET;
				} else if ( !ld->common.v1_encodings )
				    char_code = char_code_2;
				else
				    char_code |= 128;
				break;

			    case ISO_LATIN_5:
				char_code |= 128;
				if ( ld->common.v1_encodings ) {
				    if ( char_code < 0x40 )
					char_code = iso_latin_5[char_code - 32];
				    else if ( char_code < 0x60 ) {
					char_code = 2;
					char_set = LINE_DRAWING;
					ext_char_set = STANDARD_SET;
				    } else
					char_code |= 128;
				} else {
				    if (( char_code == 0x21 ) ||
				( char_code >= 0x3f && char_code <= 0x5e ) ||
				( char_code >= 0x7b && char_code <= 0x7f )) {
					char_code = ERROR_CHARACTER_CODE;
					char_set = ERROR_CHARACTER_SET;
					ext_char_set = ERROR_CHARACTER_EXT_SET;
				    } else
					char_code |= 128;
				}
				break;

			    case ASCII:
			    default:
				break;
			}
			break;

		    case ONE_BYTE_SET:
			char_code &= 127;
			switch ( char_set ) {
			    case JIS_KATAKANA:
				char_code |= 128;
				if ( char_code >= 224 && char_code <= 254 ) {
				    char_code = ERROR_CHARACTER_CODE;
				    char_set = ERROR_CHARACTER_SET;
				    ext_char_set = ERROR_CHARACTER_EXT_SET;
				}
				break;
			    case CRM_FONT_L:
			    case CRM_FONT_R:
				char_code |= 128;
				break;
			    case ISO_LATIN_8:
				if ( ld->common.v1_encodings ) {
				    if ( char_code < 0x40 )
					char_code = iso_latin_8[char_code - 32];
				    else if ( char_code < 0x60 ) {
					char_code = 2;
					char_set = LINE_DRAWING;
					ext_char_set = STANDARD_SET;
				    } else
					char_code |= 128;
				} else {
				    if (( char_code == 0x21 ) ||
				( char_code >= 0x3f && char_code <= 0x5e ) ||
				( char_code >= 0x7b && char_code <= 0x7f )) {
					char_code = ERROR_CHARACTER_CODE;
					char_set = ERROR_CHARACTER_SET;
					ext_char_set = ERROR_CHARACTER_EXT_SET;
				    } else
					char_code |= 128;
				}
				break;
			    case HEB_SUPPLEMENTAL:
				if ( ld->common.v1_encodings ) {
				    if ( char_code >= 0x40 &&
					 char_code < 0x60 ) {
					char_code = 2;
					char_set = LINE_DRAWING;
					ext_char_set = STANDARD_SET;
				    } else
					char_code |= 128;
				} else {
				    char_code =
					heb_dec_supplemental[ char_code - 32 ];
				    if ( !char_code ) {  /* undefined */
					char_code = ERROR_CHARACTER_CODE;
					char_set = ERROR_CHARACTER_SET;
					ext_char_set = ERROR_CHARACTER_EXT_SET;
				    } else if (( char_code == 0xaa ) ||
					       ( char_code == 0xba )) {
					char_set = SUPPLEMENTAL;
					ext_char_set = STANDARD_SET;
				    }
				}
				break;
  			    case JIS_ROMAN:
			    case KS_ROMAN:
			    default:
				break;
			}
			break;
		    case TWO_BYTE_SET:
		    case FOUR_BYTE_SET:
		    default:
			break;
		    }	/* switch ( ext_char_set ) */
		    ignore = FALSE;
		}
		if ( ! ignore )
		   {
		    if (( _cld wvt$l_ext_flags & vte1_m_hebrew ) &&
			( _cld wvt$l_ext_specific_flags & vte2_m_rtl )) {
			*_ld wvt$a_cur_cod_ptr-- = char_code;
			*_ld wvt$a_cur_rnd_ptr-- =
			    _ld wvt$w_actv_rendition | char_set;
                        *_ld wvt$a_cur_ext_rnd_ptr-- =
			    _ld wvt$w_actv_ext_rendition | ext_char_set;
			_ld wvt$l_actv_column--;
			_ld wvt$b_single_shift = FALSE;
		    } else
		    switch ( _cld wvt$b_char_stack_top ) {
		    case 0:	/* for 1,2 & 4 byte	*/
			if (( ext_char_set == TWO_BYTE_SET ) ||
			    ( ext_char_set == FOUR_BYTE_SET )) {
			    _cld wvt$b_char_stack[_cld wvt$b_char_stack_top++]
							= char_code;
			    _cld wvt$b_char_set		= char_set;
			    _cld wvt$b_ext_char_set	= ext_char_set;
			} else {
			    *_ld wvt$a_cur_cod_ptr++ = char_code;
        	            *_ld wvt$a_cur_rnd_ptr++ =
				_ld wvt$w_actv_rendition | char_set;
                            *_ld wvt$a_cur_ext_rnd_ptr++ =
				_ld wvt$w_actv_ext_rendition | ext_char_set;
/* EIC_JPN */		    _ld wvt$l_actv_column++;
/* EIC_JPN */		    _ld wvt$b_single_shift = FALSE;
			}
			break;
		    case 1:	/* for 2,4 byte	*/
			if (( ext_char_set == FOUR_BYTE_SET ) &&
			    ( _cld wvt$b_char_stack[0] == LC1 ) &&
			    ( char_code == LC2 )) {
			    if ( _cld wvt$l_ext_specific_flags &
				vte2_m_leading_code_mode ) {
			    _cld wvt$b_char_stack[_cld wvt$b_char_stack_top-1]
				= 0;
			    _cld wvt$b_char_stack[_cld wvt$b_char_stack_top++]
				= 0;
			    } else
			    _cld wvt$b_char_stack[_cld wvt$b_char_stack_top++]
				= char_code;
			} else {
			    if (( char_set == DEC_KANJI ) &&
				( _cld wvt$l_ext_specific_flags & vte2_m_kanji_78 )) {
				unsigned char xbuf[2];
				trans7883( _cld wvt$b_char_stack
				    [_cld wvt$b_char_stack_top-1], char_code, xbuf );
				_cld wvt$b_char_stack
				    [_cld wvt$b_char_stack_top-1] = xbuf[0];
				char_code = xbuf[1];
			    }
			    *_ld wvt$a_cur_cod_ptr++ =
				_cld wvt$b_char_stack[--_cld wvt$b_char_stack_top];
                            *_ld wvt$a_cur_rnd_ptr++ =
			   	_ld wvt$w_actv_rendition | char_set;
                            *_ld wvt$a_cur_ext_rnd_ptr++ =
		           	_ld wvt$w_actv_ext_rendition | ext_char_set | FIRST_BYTE;
			    *_ld wvt$a_cur_cod_ptr++ = char_code;
                            *_ld wvt$a_cur_rnd_ptr++ = 
			   	_ld wvt$w_actv_rendition | char_set;
                            *_ld wvt$a_cur_ext_rnd_ptr++ =
		           	_ld wvt$w_actv_ext_rendition | ext_char_set | SECOND_BYTE;
/* EIC_JPN */		    _ld wvt$l_actv_column += 2;
/* EIC_JPN */		    _ld wvt$b_single_shift = FALSE;
			}
			break;
		    case 2:	/* for 4 byte only	*/
			_cld wvt$b_char_stack[_cld wvt$b_char_stack_top++] =
			    char_code;
			break;
		    case 3:	/* for 4 byte only	*/
			char_set = DEC_HANYU_4;
			if ( _cld wvt$l_ext_specific_flags &
			    vte2_m_leading_code_mode ) {
			    *_ld wvt$a_cur_cod_ptr++ = 
				_cld wvt$b_char_stack[--_cld wvt$b_char_stack_top];
                            *_ld wvt$a_cur_rnd_ptr++ = 
			   	_ld wvt$w_actv_rendition | char_set;
                            *_ld wvt$a_cur_ext_rnd_ptr++ =
		           	_ld wvt$w_actv_ext_rendition | THIRD_OF_FOUR;
			    *_ld wvt$a_cur_cod_ptr++ = char_code;
                            *_ld wvt$a_cur_rnd_ptr++ = 
			   	_ld wvt$w_actv_rendition | char_set;
                            *_ld wvt$a_cur_ext_rnd_ptr++ =
		           	_ld wvt$w_actv_ext_rendition | FOURTH_OF_FOUR;
			    _cld wvt$b_char_stack_top = 0;
/* EIC_JPN */		    _ld wvt$l_actv_column += 2;
			} else {
			    *_ld wvt$a_cur_cod_ptr++ = LC1;
                            *_ld wvt$a_cur_rnd_ptr++ =
				_ld wvt$w_actv_rendition | char_set;
                            *_ld wvt$a_cur_ext_rnd_ptr++ =
				_ld wvt$w_actv_ext_rendition | FIRST_OF_FOUR_LC;
			    *_ld wvt$a_cur_cod_ptr++ = LC2;
                            *_ld wvt$a_cur_rnd_ptr++ =
				_ld wvt$w_actv_rendition | char_set;
                            *_ld wvt$a_cur_ext_rnd_ptr++ =
				_ld wvt$w_actv_ext_rendition | SECOND_OF_FOUR_LC;
			    *_ld wvt$a_cur_cod_ptr++ =
				_cld wvt$b_char_stack[--_cld wvt$b_char_stack_top];
	                    *_ld wvt$a_cur_rnd_ptr++ =
				_ld wvt$w_actv_rendition | char_set;
                            *_ld wvt$a_cur_ext_rnd_ptr++ =
				_ld wvt$w_actv_ext_rendition | THIRD_OF_FOUR_LC;
			    *_ld wvt$a_cur_cod_ptr++ = char_code;
                            *_ld wvt$a_cur_rnd_ptr++ = 
				_ld wvt$w_actv_rendition | char_set;
                            *_ld wvt$a_cur_ext_rnd_ptr++ =
				_ld wvt$w_actv_ext_rendition | FOURTH_OF_FOUR_LC;
			    _cld wvt$b_char_stack_top = 0;
/* EIC_JPN */		    _ld wvt$l_actv_column += 4;
			}
/* EIC_JPN */		_ld wvt$b_single_shift = FALSE;
			break;
		    default:
			break;
		    }	/* switch ( _cld wvt$b_char_stack_top )	*/
		    if ( (_ld wvt$l_actv_column == _ld wvt$l_right_margin - 8)
			&& _cld wvt$l_flags & vt4_m_margin_bell &&
			WVT$KEY_PRESSED(ld) ) WVT$BELL(ld);
		   }
               }

             /* We're at the end of the line, must insert the hard way */
             else xinsertchar(ld, *in_ptr);

	   } /* end of the local for loop optimization */

	  /* Hit a control while in the local loop, go back to the top */
          continue;

       }

        /* Not set up for direct insertion, do it the hard way */
        else xinsertchar(ld, *in_ptr);

        /* Process next character (all the way to the top) */
        continue;

      };



/*
 *  Special control character checks:
 *
 *  Before we go off and parse the character, check for the most common
 *  control codes - CR, LF, FF and TAB.  If it's one of these, do it special
 *  without all of the parsing and dancing.
 *
 */

    if (_cld wvt$b_last_event != R_CONTINUE)
       {
	if (((*in_ptr >= C0_HT) && (*in_ptr <= C0_CR)) ||
	    (_cld wvt$b_char_stack_top && (*in_ptr == 040 || *in_ptr == 0240)))
	   {
	    if ((*in_ptr >= C0_LF) && (*in_ptr <= C0_FF))
		{
		/*
		 * Auto print the line we are leaving if auto print mode is
		 * enabled and we are not in the status line.
		 *
		 * Work around a compiler bug (VAX C V3.1-051 on VMS): it
		 * executed the print_lines statement when auto print mode was
		 * not enabled, rather than when it was enabled.
		 */

		{
		int auto_print_enabled, status_display_active;

		auto_print_enabled = ( _cld wvt$w_print_flags &
		  pf1_m_auto_print_mode ) != 0;
		status_display_active = ( _cld wvt$l_vt200_common_flags
		  & vtc1_m_actv_status_display ) != 0;

		if ( auto_print_enabled && ! status_display_active )
		    {
		    print_lines( ld, 1, _mld wvt$l_actv_line,
		      line_width(_mld wvt$l_actv_line),
		          _mld wvt$l_actv_line, *in_ptr );
		    }
		}

		 if (!_ld wvt$l_defer_count)
			{
			 if ((	_ld wvt$l_actv_column -
				_ld wvt$l_disp_pos) ||
				_ld wvt$b_disp_eol)
					display_segment(ld,
							_ld wvt$l_actv_line,
							_ld wvt$l_disp_pos,
							_ld wvt$l_actv_column
							- _ld wvt$l_disp_pos);
			}
		 xlf(ld);	/* line feed, form feed, vertical tab */
		}
	    else
		{
		if ( !_ld wvt$l_defer_count )
			{
			if ((	_ld wvt$l_actv_column -
				_ld wvt$l_disp_pos) ||
				_ld wvt$b_disp_eol)
					display_segment(ld,
						_ld wvt$l_actv_line,
						_ld wvt$l_disp_pos,
						_ld wvt$l_actv_column
						- _ld wvt$l_disp_pos);
			}
		 if (*in_ptr == C0_CR)		/* return */
			{
			if (( _cld wvt$l_ext_flags & vte1_m_hebrew ) &&
			    ( _cld wvt$l_ext_specific_flags & vte2_m_rtl ))
			 _ld wvt$l_actv_column = line_width(_ld wvt$l_actv_line);
			else
			 _ld wvt$l_actv_column = _ld wvt$l_left_margin;
			 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;
			 WVT$KEY_PRESSED(ld); /* clear key pressed flag */
			}
		 else if ( !( _cld wvt$l_ext_flags & vte1_m_asian_common ) ||
		    *in_ptr == C0_HT)
		    xht(ld);			/* tab */
		 else if ( *in_ptr == 160 ) {
		    /* 0xA0 should be displayed as error character	*/
		    unsigned char temp2;
		    temp2 = _cld wvt$b_char_stack_top;
		    _cld wvt$b_char_stack_top = 0;
		    xinsertchar( ld, *in_ptr );
		    _cld wvt$b_char_stack_top = temp2;
		    display_segment(ld, _ld wvt$l_actv_line,
		    _ld wvt$l_actv_column - 1, 1);
		 } else
		    xinsertspace(ld);		/* space */
		}

	    _ld wvt$l_disp_pos = _ld wvt$l_actv_column;

            if (_cld wvt$l_vt200_common_flags & vtc1_m_insert_mode)
		_ld wvt$a_cur_cod_ptr = 0;
            else
               {
		_ld wvt$a_cur_cod_ptr = _ld wvt$a_code_base[_ld wvt$l_actv_line]+_ld wvt$l_actv_column;
		_ld wvt$a_cur_rnd_ptr = _ld wvt$a_rend_base[_ld wvt$l_actv_line]+_ld wvt$l_actv_column;
		_ld wvt$a_cur_ext_rnd_ptr = _ld wvt$a_ext_rend_base[_ld wvt$l_actv_line]+_ld wvt$l_actv_column;
               }
            _ld wvt$b_single_shift = FALSE;
            continue;
           }
        }



/*
 *  Full ANSI parsing:
 *
 *  Process control codes (other than the above), escape and control
 *  sequences.  Also get here with wvt$b_graphic data under unusual
 *  conditions.
 *
 */

    /* Flush any undisplayed updates */
    if (_ld wvt$l_defer_count)	/* if we have been defering up scrolls */
	{
	 if (_ld wvt$l_defer_count >=
		(_ld wvt$l_bottom_margin - _ld wvt$l_top_margin)+1)
		{
		 WVT$ERASE_SCREEN(ld,	_ld wvt$l_top_margin,
					_ld wvt$l_left_margin,
					_ld wvt$l_bottom_margin,
					_ld wvt$l_right_margin);
		}
	 else
		{
		 WVT$UP_SCROLL(ld,	_ld wvt$l_top_margin,
					_ld wvt$l_defer_count, 2);
		}

	 temp = (_ld wvt$l_bottom_margin - _ld wvt$l_defer_count) + 1;
	 if (temp < _ld wvt$l_top_margin) temp = _ld wvt$l_top_margin;

	 _ld wvt$l_defer_count = 0;

	 WVT$REFRESH_DISPLAY(ld, temp, _ld wvt$l_bottom_margin);
	}
    else
	{
	 if ((_ld wvt$l_actv_column - _ld wvt$l_disp_pos) || _ld wvt$b_disp_eol)
		display_segment(ld, _ld wvt$l_actv_line, _ld wvt$l_disp_pos,
				    _ld wvt$l_actv_column - _ld wvt$l_disp_pos);
	}

    _ld wvt$l_disp_pos = _ld wvt$l_actv_column;
    _ld wvt$a_cur_cod_ptr = 0;

    /* parse the current character */

    parse_ansi(ld, *in_ptr, &event, &start, &final);	/* ANSI parser */


    /* treat DEL as a graphic character except in escape sequences */

    if (event == R_CONTROL && *in_ptr == 0x7F)
	event = R_GRAPHIC;

    /* Process recognized events,
       save event (except when a control character during a sequence) */

    if (!( (_cld wvt$b_last_event == R_CONTINUE) &&
       (event == R_CONTROL) )) _cld wvt$b_last_event = event;


    /* ANSI mode processing */

    if (_cld wvt$l_vt200_flags & vt1_m_ansi_mode)

      {

        switch (event)

        {

          case R_CONTINUE:
            break;

          case R_GRAPHIC:
            xinsertchar(ld, _cld wvt$r_com_state.data[final]);
            break;

          case R_CONTROL: 
            xcontrol(ld, _cld wvt$r_com_state.data[final]);
	    _ld wvt$l_disp_pos = _ld wvt$l_actv_column;
            break;

          case R_ESC_SEQ:
            xescseq(ld, &_cld wvt$r_com_state, start, final);
	    _ld wvt$l_disp_pos = _ld wvt$l_actv_column;
            break;

          case R_CSI_SEQ:
            seqparm(ld, &_cld wvt$r_com_state, start, final);
            xcntseq(ld);
	    _ld wvt$l_disp_pos = _ld wvt$l_actv_column;
            break;

          case R_DCS_SEQ:
            seqparm(ld, &_cld wvt$r_com_state, start, final);
            xdcsseq(ld);
	    _ld wvt$l_disp_pos = _ld wvt$l_actv_column;
            break;

          }   /* end switch event */

      } /* end ansi mode dispatch */

    else

      /* Non-ANSI dispatch (i.e. VT52) */

      {

      switch (event)

        {

          case R_GRAPHIC:
            if (_cld wvt$l_vt200_flags & vt1_m_vt52_cursor_seq)
              {
	       /* subtracting 31 decimal or 037 which is 37 octal */
               _cld wvt$l_parms[_cld wvt$b_parmcnt] =
				_cld wvt$r_com_state.data[final] - 31;
               _cld wvt$b_parmcnt++;
               if (_cld wvt$b_parmcnt == 2)
		  {
  /**************************************************/
  /*  Direct Cursor Move in VT52 Mode should be	    */	
  /*  canceled by illegal parameter		    */
  /*              EIC-J Korosue			    */
  /**************************************************/
			if ( _cld wvt$l_parms[0]>0 && _cld wvt$l_parms[0]<25 &&
			     _cld wvt$l_parms[1]>0 && _cld wvt$l_parms[1]<81 ) {
			    x52cup(ld);
			    _ld wvt$l_disp_pos = _ld wvt$l_actv_column;
			} else {
			    _cld wvt$l_vt200_flags &= ~vt1_m_vt52_cursor_seq;
			}
		  }
               else _cld wvt$b_last_event = R_CONTINUE;
              }

            else
              {
               xinsertchar(ld, _cld wvt$r_com_state.data[final]);
              }
            break;

          case R_CONTROL:
            if (_cld wvt$l_vt200_flags & vt1_m_vt52_cursor_seq)
	      {
		x52cup(ld);
		_ld wvt$l_disp_pos = _ld wvt$l_actv_column;
		break;
	      }
            if (_cld wvt$r_com_state.data[final] < 32)
            xcontrol(ld, _cld wvt$r_com_state.data[final]);
            else
              {
               x52escseq(ld, _cld wvt$r_com_state.data[final]);
              }
	    _ld wvt$l_disp_pos = _ld wvt$l_actv_column;
            break;

          case R_ESC_SEQ:
            if (_cld wvt$l_vt200_flags & vt1_m_vt52_cursor_seq)
	      {
		x52cup(ld);
		_ld wvt$l_disp_pos = _ld wvt$l_actv_column;
		break;
	      }
            if (_cld wvt$b_intercnt == 0) x52escseq(ld, _cld wvt$r_com_state.data[final]);
	    _ld wvt$l_disp_pos = _ld wvt$l_actv_column;
            break;

          default: break;

        };		/* End VT52 mode despatch */
      }

    /* Line may have changed, recompute the line width */

    /************ REMOVE THIS, MAKE ANY CODE THAT CHANGES THE
     CURRENT LINE ALSO UPDATE THE LINE WIDTH ****************/

    _cld wvt$l_actv_width  = line_width(_ld wvt$l_actv_line);
    if (_ld wvt$l_right_margin < _cld wvt$l_actv_width)
	_cld wvt$l_actv_width  = _ld wvt$l_right_margin;


    /* restore single shifts */

    if (!((event == R_CONTROL) && 
	((_cld wvt$r_com_state.data[final] == C0_SO) ||
	 (_cld wvt$r_com_state.data[final] == C0_SI) ||
	 (_cld wvt$r_com_state.data[final] == C1_SS2) ||
	 (_cld wvt$r_com_state.data[final] == C1_SS3)))
	&& (_ld wvt$b_single_shift < 10)
	&& !_cld wvt$b_char_stack_top )
	_ld wvt$b_single_shift = FALSE;

    }



/*
 *
 *    Finished parsing entire input string, unless output was stopped in the
 *    middle.  Return from ANSI_DISPLAY.
 *
 *    Note that all of the data may not have been displayed yet... the
 *    actual display of the data is defered until: [A] an event occurs
 *    that requires the display flushed (like a non-graphic character
 *    being seen) or [B] END_BLOCK (macro-32 routine) is called by the port
 *    driver.
 *
 *    Also, note that if the inner loop finished, and then the outer loop
 *    also tested in_ptr++ and found it to be depleted, now in_ptr has been
 *    ++'ed one too many times.  Hence the following "?" operation to
 *    accurately return the number of parsed characters.
 */

return (in_ptr-in_buf >= in_len ? in_len : in_ptr-in_buf);

}


/***********************************************************************/
fill_buffer(ld, state ,evnt, strt, finl)  /* Unpack an escape sequence */
/***********************************************************************/


/*
 *	Routine to unpack a parsed escape sequence on the parser stack
 *	into an ansi string stored in the caller's printer output buffer.
 *
 */

wvtp ld;
struct parse_stack *state;
int evnt, strt, finl;

{

#if PRINTER_EXTENSION

int i;
short parm,power;
char parm_flag,out_flag;
unsigned char *buffer_pointer;

/*
 *  out_buffer points to a single byte length,
 *  followed by a printer line buffer.
 *  We will point to the end of the current buffer.
 *
 */

buffer_pointer = _cld wvt$b_work_buffer + _cld wvt$l_work_string.offset;

parm_flag = FALSE;
for (i=strt; i<=finl; i++) {
  switch (evnt) {
    case R_ESC_SEQ:
      *buffer_pointer++ = state->data[i];
      break;

    case R_CSI_SEQ:
    case R_DCS_SEQ:
      if (i == strt)
         {
          if (_cld wvt$w_print_flags & pf1_m_prt_transmission_mode)
            {
             if (state->class[i] == CSI) *buffer_pointer++ = 155;   /* CSI */
             else *buffer_pointer++ = 144;                          /* DCS */
            }
          else
            {
             *buffer_pointer++ = C0_ESC;
             if (state->class[i] == CSI) *buffer_pointer++ = '[';   /* CSI */
             else *buffer_pointer++ = 'P';                          /* DCS */
            }
         }
      switch (state->value[i]) {
        case CS_PRIVATE:
        case CS_INTER:
        case CS_IGNORE:
          *buffer_pointer++ = state->data[i];
          break;
	case CS_OMIT:
	  if (parm_flag) *buffer_pointer++ = ';';
	  parm_flag = TRUE;
	  break;
        case CS_PARAM:
          if (parm_flag) *buffer_pointer++ = ';';
          parm_flag = TRUE;
          parm = state->data[i];
          power = 10000;
          out_flag = FALSE;
          while (power > 0) {
            if ((parm/power) || (out_flag)) {
              out_flag = TRUE;
              *buffer_pointer++ = (parm/power) + '0';
              parm = parm % power;
              }
            power /= 10;
            }
          break;
        }
      break;
    case R_GRAPHIC:
    case R_CONTROL:
      if ((evnt == R_CONTROL) && (state->data[i] > 127))
        { if (_cld wvt$w_print_flags & pf1_m_prt_transmission_mode)
		*buffer_pointer++ = state->data[i];
	  else {
		*buffer_pointer++ = C0_ESC;
		*buffer_pointer++ = state->data[i] - 64;
	  }
	}
      else *buffer_pointer++ = state->data[i];
      break;
    }
  }

_cld wvt$l_work_string.offset = buffer_pointer - _cld wvt$b_work_buffer;

#endif

}

/****************************************************/
xinsertspace(ld) /* Insert a space into the display */
/****************************************************/

wvtp ld;
{
register int x;

if (_cld wvt$l_vt200_common_flags & vtc1_m_insert_mode)
	{
	 _ld wvt$b_disp_eol = TRUE;  /* Force next display_segment to end_of_line */
	 if (( _cld wvt$l_ext_flags & vte1_m_hebrew ) &&
	     ( _cld wvt$l_ext_specific_flags & vte2_m_rtl ))
	 for (x = _ld wvt$l_left_margin; x < _ld wvt$l_actv_column; x++)
		 {
		    character(_ld wvt$l_actv_line,x) =
			character(_ld wvt$l_actv_line,x+1);
		    rendition(_ld wvt$l_actv_line,x) =
			rendition(_ld wvt$l_actv_line,x+1);
		    ext_rendition(_ld wvt$l_actv_line,x) =
			ext_rendition(_ld wvt$l_actv_line,x+1);
		 }
	 else

	 for (x = _cld wvt$l_actv_width; x >= _ld wvt$l_actv_column+1; x--)
		{
		 character(_ld wvt$l_actv_line,x) =
			character(_ld wvt$l_actv_line,x-1);
		 rendition(_ld wvt$l_actv_line,x) =
			rendition(_ld wvt$l_actv_line,x-1);
		 ext_rendition(_ld wvt$l_actv_line,x) =
			ext_rendition(_ld wvt$l_actv_line,x-1);
		}
	}

if ((_ld wvt$l_vt200_specific_flags & vts1_m_auto_wrap_mode) &&
    (_ld wvt$l_vt200_specific_flags & vts1_m_last_column))
	{
	    if (!_ld wvt$l_defer_count)
		{
	 	    _ld wvt$b_disp_eol = TRUE; /* Flush the display */
         	    display_segment(ld, _ld wvt$l_actv_line, 
			_ld wvt$l_disp_pos, 0);
		}
	 _ld wvt$l_disp_pos =
	     _ld wvt$l_actv_column =
		(( _cld wvt$l_ext_flags & vte1_m_hebrew  ) &&
		 ( _cld wvt$l_ext_specific_flags & vte2_m_rtl )) ?
		 line_width(_ld wvt$l_actv_line) :

	         _ld wvt$l_left_margin; /* Set position to the left margin */

	 xlf(ld); /* new line line */
	}

character(_ld wvt$l_actv_line,_ld wvt$l_actv_column) = 32;
rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
	_ld wvt$w_actv_rendition | ASCII;
ext_rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
	_ld wvt$w_actv_ext_rendition | STANDARD_SET;

/* force to display a space */
display_segment(ld, _ld wvt$l_actv_line, _ld wvt$l_actv_column, 1);

_ld wvt$a_cur_cod_ptr = 0;  /* Default to no optimization */

if (_ld wvt$l_actv_column == _cld wvt$l_actv_width)
	{
	 _ld wvt$l_vt200_specific_flags |= vts1_m_last_column;
	 _ld wvt$b_disp_eol = TRUE;
	}
else
	{
	 if (_ld wvt$l_actv_column < line_width(_ld wvt$l_actv_line))
		{
		 _ld wvt$l_actv_column++;
		 if (!(_cld wvt$l_vt200_common_flags & vtc1_m_insert_mode))
		   {
		    _ld wvt$a_cur_cod_ptr =
		    _ld wvt$a_code_base[_ld wvt$l_actv_line]+
					_ld wvt$l_actv_column;
		    _ld wvt$a_cur_rnd_ptr =
		    _ld wvt$a_rend_base[_ld wvt$l_actv_line]+
					_ld wvt$l_actv_column;
		    _ld wvt$a_cur_ext_rnd_ptr =
		    _ld wvt$a_ext_rend_base[_ld wvt$l_actv_line]+
					_ld wvt$l_actv_column;
		   }
		}
	 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;
	}
    if ( (_ld wvt$l_actv_column == _ld wvt$l_right_margin - 8)
	&& _cld wvt$l_flags & vt4_m_margin_bell &&
	WVT$KEY_PRESSED(ld) ) WVT$BELL(ld);
}

/******************************************************************/
xinsertchar(ld, char_code) /* Insert a character into the display */
/******************************************************************/

wvtp ld;
int char_code;

/*
 *
 *
 *	This is the normal display text insertion routine (the hard way).
 *
 *	First get the character set based on the single shift.
 *	Then see if it's a DEL or 255 and the character set is NOT
 *	ISO Latin 1 (exit if this is the case).
 *	Reset the sigle shift.
 *	Perform any shuffling if in insert mode.
 *	Perform wrap if needed.
 *	Insert the character.
 *	Set the end of line flag if needed.
 *	Set up insertion pointer if possible.
 */

{

register int x;
/* How many characters must be insert in insert mode EIC-J Korosue	*/
register int num_ins_chars;
int ext_char_set;
int char_set, ignore;

xtranslatechar( ld, char_code, &char_code, &char_set, &ext_char_set, &ignore );

if ( ignore )
	return;

if (_cld wvt$l_vt200_common_flags & vtc1_m_insert_mode)
	{
	 _ld wvt$b_disp_eol = TRUE;  /* Force next display_segment to end_of_line */
/********************************************************/
/* How many characters must be inserted?		*/
/*                 EIC-J Korosue     - 008 -		*/
/********************************************************/
	num_ins_chars = _cld wvt$l_ext_flags & vte1_m_asian_common ?
			_cld wvt$b_char_stack_top + 1 : 1;
	for ( x = _cld wvt$l_actv_width;
	    x >= _ld wvt$l_actv_column + num_ins_chars; x-- ) {
		 character(_ld wvt$l_actv_line,x) =
			character(_ld wvt$l_actv_line,x-num_ins_chars);
		 rendition(_ld wvt$l_actv_line,x) =
			rendition(_ld wvt$l_actv_line,x-num_ins_chars);
		 ext_rendition(_ld wvt$l_actv_line,x) =
			ext_rendition(_ld wvt$l_actv_line,x-num_ins_chars);
		}
	}

if ((_ld wvt$l_vt200_specific_flags & vts1_m_auto_wrap_mode) &&
    (_ld wvt$l_vt200_specific_flags & vts1_m_last_column))
	{

	/*
	 * Auto print the line we are leaving if auto print mode is enabled and
	 * we are not in the status line.
	 */

	/*
	 * Work around a compiler bug (VAX C V3.1-051 on VMS): it
	 * executed the print_lines statement when auto print mode was
	 * not enabled, rather than when it was enabled.
	 */
	int auto_print_enabled = ( _cld wvt$w_print_flags &
					pf1_m_auto_print_mode ) != 0,
	    status_display_active = ( _cld wvt$l_vt200_common_flags &
					vtc1_m_actv_status_display ) != 0;
	if ( auto_print_enabled && ! status_display_active )
	    {
	    print_lines( ld, 1, _mld wvt$l_actv_line,
	      line_width(_mld wvt$l_actv_line), _mld wvt$l_actv_line, '\n' );
	    }

	if (!_ld wvt$l_defer_count)
		{
	 	    _ld wvt$b_disp_eol = TRUE; /* Flush the display */
         	    display_segment(ld, _ld wvt$l_actv_line, 
			_ld wvt$l_disp_pos, 0);
		}
	 _ld wvt$l_disp_pos =
	     _ld wvt$l_actv_column =
	         _ld wvt$l_left_margin; /* Set position to the left margin */

	 xlf(ld); /* new line line */
	}
/* handle 1/2/4 byte parsing...	*/

    if (!(_ld wvt$l_vt200_specific_flags & vts1_m_auto_wrap_mode) &&
	(_cld wvt$l_ext_flags & vte1_m_asian_common) &&
	(_ld wvt$l_vt200_specific_flags & vts1_m_last_column)) {
	if ( _cld wvt$b_char_stack_top )
	    --_cld wvt$b_char_stack_top;
	else if (( ext_char_set == TWO_BYTE_SET ) ||
		 ( ext_char_set == FOUR_BYTE_SET ))	
	    _cld wvt$b_char_stack [_cld wvt$b_char_stack_top++] = char_code;
 	return;
    }
    switch ( _cld wvt$b_char_stack_top ) {
    case 0:	/* for 1,2 & 4 byte	*/
	if (( ext_char_set == TWO_BYTE_SET ) ||
	    ( ext_char_set == FOUR_BYTE_SET )) {
	    _cld wvt$b_char_stack [_cld wvt$b_char_stack_top++] = char_code;
	    _cld wvt$b_char_set = char_set;
	    _cld wvt$b_ext_char_set = ext_char_set;
	} else {
	    character(_ld wvt$l_actv_line,_ld wvt$l_actv_column) = char_code;
	    rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
		( _ld wvt$w_actv_rendition | char_set );
	    ext_rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
        	( _ld wvt$w_actv_ext_rendition | ext_char_set);
	    _ld wvt$b_single_shift = FALSE;	/* EIC_JPN */
	}
	break;
    case 1:	/* for 2,4 byte	*/
	if (( ext_char_set == FOUR_BYTE_SET ) &&
	    ( _cld wvt$b_char_stack [0] == LC1 ) && ( char_code == LC2 )) {
	    if ( _cld wvt$l_ext_specific_flags & vte2_m_leading_code_mode ) {
		_cld wvt$b_char_stack [_cld wvt$b_char_stack_top-1] = 0;
		_cld wvt$b_char_stack [_cld wvt$b_char_stack_top++] = 0;
	    } else {
		_cld wvt$b_char_stack [_cld wvt$b_char_stack_top++] = char_code;
	    }
	} else {
	    character(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
		_cld wvt$b_char_stack [--_cld wvt$b_char_stack_top];
	    rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
        	( _ld wvt$w_actv_rendition | char_set );
	    ext_rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
        	( _ld wvt$w_actv_ext_rendition | ext_char_set | FIRST_BYTE );
	    _ld wvt$l_actv_column++;
	    character(_ld wvt$l_actv_line,_ld wvt$l_actv_column) = char_code;
	    ext_rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
        	( _ld wvt$w_actv_ext_rendition | ext_char_set | SECOND_BYTE );
	    rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
        	( _ld wvt$w_actv_rendition | char_set );
	    _ld wvt$b_single_shift = FALSE;	/* EIC_JPN */
	}
	break;
    case 2:	/* for 4 byte only	*/
	_cld wvt$b_char_stack [_cld wvt$b_char_stack_top++] = char_code;
	break;
    case 3:	/* for 4 byte only	*/
	char_set = DEC_HANYU_4;
	if ( _cld wvt$l_ext_specific_flags & vte2_m_leading_code_mode ) {
	    character(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
		_cld wvt$b_char_stack [--_cld wvt$b_char_stack_top];
	    rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
        	( _ld wvt$w_actv_rendition | char_set );
	    ext_rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
        	( _ld wvt$w_actv_ext_rendition | THIRD_OF_FOUR );
	    _ld wvt$l_actv_column++;
	    character(_ld wvt$l_actv_line,_ld wvt$l_actv_column) = char_code;
	    rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
        	( _ld wvt$w_actv_rendition | char_set );
	    ext_rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
        	( _ld wvt$w_actv_ext_rendition | FOURTH_OF_FOUR );
	    _cld wvt$b_char_stack_top = 0;
	} else {
	    character(_ld wvt$l_actv_line,_ld wvt$l_actv_column) = LC1;
	    rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
        	( _ld wvt$w_actv_rendition | char_set );
	    ext_rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
        	( _ld wvt$w_actv_ext_rendition | FIRST_OF_FOUR_LC );
	    _ld wvt$l_actv_column++;
	    character(_ld wvt$l_actv_line,_ld wvt$l_actv_column) = LC2;
	    rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
        	( _ld wvt$w_actv_rendition | char_set );
	    ext_rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
        	( _ld wvt$w_actv_ext_rendition | SECOND_OF_FOUR_LC );
	    _ld wvt$l_actv_column++;
	    character(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
		_cld wvt$b_char_stack [--_cld wvt$b_char_stack_top];
	    rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
        	( _ld wvt$w_actv_rendition | char_set );
	    ext_rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
        	( _ld wvt$w_actv_ext_rendition | THIRD_OF_FOUR_LC );
	    _ld wvt$l_actv_column++;
	    character(_ld wvt$l_actv_line,_ld wvt$l_actv_column) = char_code;
	    rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
        	( _ld wvt$w_actv_rendition | char_set );
	    ext_rendition(_ld wvt$l_actv_line,_ld wvt$l_actv_column) =
        	( _ld wvt$w_actv_ext_rendition | FOURTH_OF_FOUR_LC );
	    _cld wvt$b_char_stack_top = 0;
	}
/* EIC_JPN */
	_ld wvt$b_single_shift = FALSE;
	break;
    default:
	break;
    }	/* switch ( _cld wvt$b_char_stack_top )	*/

_ld wvt$a_cur_cod_ptr = 0;  /* Default to no optimization */

    if (( _cld wvt$l_ext_flags & vte1_m_hebrew ) &&
	( _cld wvt$l_ext_specific_flags & vte2_m_rtl )) {
	if (_ld wvt$l_actv_column == _ld wvt$l_left_margin) {
	    _ld wvt$l_vt200_specific_flags |= vts1_m_last_column;
	    _ld wvt$b_disp_eol = TRUE;
	}
	else {
	    if (_ld wvt$l_actv_column > _ld wvt$l_left_margin) {
		_ld wvt$l_actv_column--;
		if (!(_cld wvt$l_vt200_common_flags & vtc1_m_insert_mode)) {
		    _ld wvt$a_cur_cod_ptr =
		    _ld wvt$a_code_base[_ld wvt$l_actv_line]+
					_ld wvt$l_actv_column;
		    _ld wvt$a_cur_rnd_ptr =
		    _ld wvt$a_rend_base[_ld wvt$l_actv_line]+
					_ld wvt$l_actv_column;
		    _ld wvt$a_cur_ext_rnd_ptr =
		    _ld wvt$a_ext_rend_base[_ld wvt$l_actv_line]+
					_ld wvt$l_actv_column;
		}
	    }
	    _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;
	}
    }
    else

if (_ld wvt$l_actv_column == _cld wvt$l_actv_width)
	{
	 _ld wvt$l_vt200_specific_flags |= vts1_m_last_column;
	 _ld wvt$b_disp_eol = TRUE;
	}
else
	{
	 if (_ld wvt$l_actv_column < line_width(_ld wvt$l_actv_line))
		{
		if ( !_cld wvt$b_char_stack_top )
		 _ld wvt$l_actv_column++;
		 if (!(_cld wvt$l_vt200_common_flags & vtc1_m_insert_mode))
		   {
		    _ld wvt$a_cur_cod_ptr =
		    _ld wvt$a_code_base[_ld wvt$l_actv_line]+
					_ld wvt$l_actv_column;
		    _ld wvt$a_cur_rnd_ptr =
		    _ld wvt$a_rend_base[_ld wvt$l_actv_line]+
					_ld wvt$l_actv_column;
		    _ld wvt$a_cur_ext_rnd_ptr =
		    _ld wvt$a_ext_rend_base[_ld wvt$l_actv_line]+
					_ld wvt$l_actv_column;
		   }
		}
	 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;
	}
    if ( (_ld wvt$l_actv_column == _ld wvt$l_right_margin - 8)
	&& _cld wvt$l_flags & vt4_m_margin_bell &&
	WVT$KEY_PRESSED(ld) ) WVT$BELL(ld);

}

/* xtranslatechar - find a character's character set and display code */

xtranslatechar( ld, char_code, display_code, display_char_set, display_ext_char_set, ignore )
wvtp ld;
int char_code, *display_code, *display_char_set, *display_ext_char_set, *ignore;
{
int char_set;
int ext_char_set;

/* character 128 means display the error character */

if ( char_code == 128 )
   {
	char_set = _ld wvt$b_g_sets[_ld wvt$b_gr];
	ext_char_set = _ld wvt$b_ext_g_sets[_ld wvt$b_gr];
	if (ext_char_set == ONE_BYTE_SET && char_set == CRM_FONT_R) {
	    *display_code = char_code;
	    *display_ext_char_set = ext_char_set;
	    *display_char_set = char_set;
	    *ignore = FALSE;
	    return;
	} else if (( _cld wvt$l_ext_flags & vte1_m_asian_common )
	    && _cld wvt$b_char_stack_top ) {
	    /* First byte + <SUB> should be full size reversed question mark */
	    *display_code = 128;
	    *display_ext_char_set = TWO_BYTE_SET;
	    *display_char_set = DEC_KANJI;
	    *ignore = FALSE;
	    return;
	}
   *display_ext_char_set = ERROR_CHARACTER_EXT_SET;
   *display_code = ERROR_CHARACTER_CODE;
   *display_char_set = ERROR_CHARACTER_SET;
   *ignore = FALSE;
   return;
   }

switch (_ld wvt$b_single_shift)

   {

    case SS_2:

	char_set = _ld wvt$b_g_sets[2];
	ext_char_set = _ld wvt$b_ext_g_sets[2];
	break;

    case SS_3:

	char_set = _ld wvt$b_g_sets[3];
	ext_char_set = _ld wvt$b_ext_g_sets[3];
	if ( ext_char_set == TWO_BYTE_SET )
	    char_code |= 128;
	break;

    case SSS_3_TCS:

	char_set = TECHNICAL;
	ext_char_set = STANDARD_SET;
	break;

    case ERROR_SHIFT:

        char_code = 160;		/* use and invalid character */
	char_set = SUPPLEMENTAL;
	ext_char_set = STANDARD_SET;
	break;

    case FALSE:
    default:

	if (char_code < 128)
	    {
	    char_set = _ld wvt$b_g_sets[_ld wvt$b_gl];
	    ext_char_set = _ld wvt$b_ext_g_sets[_ld wvt$b_gl];
	    }
	else
	    {
	    char_set = _ld wvt$b_g_sets[_ld wvt$b_gr];
	    ext_char_set = _ld wvt$b_ext_g_sets[_ld wvt$b_gr];
	    }
	break;

    }

if ( _cld wvt$l_ext_flags & vte1_m_hebrew )
if (( _cld wvt$b_conformance_level == LEVEL1 ||
	_cld wvt$l_vt200_flags & vt1_m_nrc_mode ) &&
    ( char_code >= 0x60 && char_code <= 0x7a ) &&
    ( _cld wvt$l_ext_specific_flags & vte2_m_kb_map &&
	ext_char_set == STANDARD_SET &&
	char_set == ASCII )) {
/* remove this because of bug in original code - designation of ASCI to any G
/* causes wvt$b_nrc_set =0	&&  _ld wvt$b_nrc_set == 17) */
	    *display_code = char_code | 128;
	    *display_char_set = ISO_LATIN_8;
	    *display_ext_char_set = ONE_BYTE_SET;
	    *ignore = FALSE;
	    return;
	}

if ( char_code == 32 || char_code == 160 && 
   ( ext_char_set == STANDARD_SET && char_set == ISO_LATIN_1 )) {
	char_code = 32;
	char_set = ASCII;		/* real space */
	ext_char_set = STANDARD_SET;
} else if ( char_code == 127 || char_code == 255 ) {
	if ( ext_char_set == ONE_BYTE_SET &&
	   ( char_set == CRM_FONT_L || char_set == CRM_FONT_R )) {
	    char_code |= 128;
	    *display_code = char_code;
	    *display_char_set = char_set;
	    *display_ext_char_set = ext_char_set;
	    *ignore = FALSE;
	    return;
	} else if ( ext_char_set == STANDARD_SET && char_set != ISO_LATIN_1 ) {
	    _ld wvt$b_single_shift = FALSE;
	    *ignore = TRUE;
	    return;
	}
} else if (( char_code == 160 ) &&
         !(( char_set == CRM_FONT_R ) && ( ext_char_set == ONE_BYTE_SET ))) {
        char_set = ERROR_CHARACTER_SET;
        char_code = ERROR_CHARACTER_CODE;
        ext_char_set = ERROR_CHARACTER_EXT_SET;
}

if ( ext_char_set == STANDARD_SET )
    if ( char_set >= ISO_LATIN_1 )
	char_code |= 128;		/* Set high bit if ISO/MCS */
    else
	char_code &= 127;		/* Strip high bit if not ISO/MCS */

if (_cld wvt$l_vt200_flags & vt1_m_nrc_mode
    && ext_char_set == STANDARD_SET
    && char_set == ASCII && _ld wvt$b_nrc_set != 0 )
	switch (char_code)
	    {
	    case '#':
		switch (_ld wvt$b_nrc_set)
		    {
		    case 2:	/* United Kingdom */
		    case 9:	/* Italian */
		    case 14:	/* French */
		    case 15:	/* Spanish */
			char_code = 163;	/* pound sterling */
			char_set = ISO_LATIN_1;
			break;
		    case 10:	/* Swiss */
			char_code = 249;	/* u-grave */
			char_set = ISO_LATIN_1;
			break;
		    }
		break;
	    case '@':
		switch (_ld wvt$b_nrc_set)
		    {
		    case 4:	/* French Canadian */
		    case 10:	/* Swiss */
		    case 14:	/* French */
			char_code = 224;	/* a-grave */
			char_set = ISO_LATIN_1;
			break;
		    case 7:	/* German */
		    case 9:	/* Italian */
		    case 15:	/* Spanish */
			char_code = 167;	/* section */
			char_set = ISO_LATIN_1;
			break;
		    case 12:	/* Swedish */
			char_code = 201;	/* E-acute */
			char_set = ISO_LATIN_1;
			break;
		    }
		break;
	    case '[':
		switch (_ld wvt$b_nrc_set)
		    {
		    case 6:	/* Finnish */
		    case 7:	/* German */
		    case 12:	/* Swedish */
			char_code = 196;	/* A-dieresis */
			char_set = ISO_LATIN_1;
			break;
		    case 9:	/* Italian */
		    case 14:	/* French */
			char_code = 176;	/* degrees */
			char_set = ISO_LATIN_1;
			break;
		    case 4:	/* French Canadian */
			char_code = 226;	/* a-circumflex */
			char_set = ISO_LATIN_1;
			break;
		    case 5:	/* Norwegian/Danish */
			char_code = 198;	/* AE */
			char_set = ISO_LATIN_1;
			break;
		    case 16:	/* Portuguese */
			char_code = 195;	/* A-tilde */
			char_set = ISO_LATIN_1;;
			break;
		    case 15:	/* Spanish */
			char_code = 161;	/* inverted ! */
			char_set = ISO_LATIN_1;
			break;
		    case 10:	/* Swiss */
			char_code = 233;	/* e-acute */
			char_set = ISO_LATIN_1;
			break;
		    }
		break;
	    case '\\':
		switch (_ld wvt$b_nrc_set)
		    {
		    case 6:	/* Finnish */
		    case 7:	/* German */
		    case 12:	/* Swedish */
			char_code = 214;	/* O-umlaut */
			char_set = ISO_LATIN_1;
			break;
		    case 4:	/* French Canadian */
		    case 9:	/* Italian */
		    case 10:	/* Swiss */
		    case 14:	/* French */
			char_code = 231;	/* c-cedilla */
			char_set = ISO_LATIN_1;
			break;
		    case 5:	/* Norwegian/Danish */
			char_code = 216;	/* O-slash */
			char_set = ISO_LATIN_1;
			break;
		    case 16:	/* Portuguese */
			char_code = 199;	/* C-cedilla */
			char_set = ISO_LATIN_1;
			break;
		    case 15:	/* Spanish */
			char_code = 209;	/* N-tilde */
			char_set = ISO_LATIN_1;
			break;
		    }
		break;
	    case ']':
		switch (_ld wvt$b_nrc_set)
		    {
		    case 6:	/* Finnish */
		    case 5:	/* Norwegian/Danish */
		    case 12:	/* Swedish */
			char_code = 197;	/* A-circle */
			char_set = ISO_LATIN_1;
			break;
		    case 14:	/* French */
			char_code = 167;	/* section */
			char_set = ISO_LATIN_1;
			break;
		    case 4:	/* French Canadian */
		    case 10:	/* Swiss */
			char_code = 234;	/* e-circumflex */
			char_set = ISO_LATIN_1;
			break;
		    case 7:	/* German */
			char_code = 220;	/* U-umlaut */
			char_set = ISO_LATIN_1;
			break;
		    case 9:	/* Italian */
			char_code = 233;	/* e-acute */
			char_set = ISO_LATIN_1;
			break;
		    case 16:	/* Portuguese */
			char_code = 213;	/* O-tilde */
			char_set = ISO_LATIN_1;
			break;
		    case 15:	/* Spanish */
			char_code = 191;	/* inverted ? */
			char_set = ISO_LATIN_1;
			break;
		    }
		break;
	    case '^':
		switch (_ld wvt$b_nrc_set)
		    {
		    case 6:	/* Finnish */
		    case 12:	/* Swedish */
			char_code = 220;	/* U-umlaut */
			char_set = ISO_LATIN_1;
			break;
		    case 4:	/* French Canadian */
		    case 10:	/* Swiss */
			char_code = 238;	/* i-circumflex */
			char_set = ISO_LATIN_1;
			break;
		    }
		break;
	    case '_':
		switch (_ld wvt$b_nrc_set)
		    {
		    case 10:	/* Swiss */
			char_code = 232;	/* e-grave */
			char_set = ISO_LATIN_1;
			break;
		    }
		break;
	    case '`':
		switch (_ld wvt$b_nrc_set)
		    {
		    case 6:	/* Finnish */
		    case 12:	/* Swedish */
			char_code = 233;	/* e-acute */
			char_set = ISO_LATIN_1;
			break;
		    case 4:	/* French Canadian */
		    case 10:	/* Swiss */
			char_code = 244;	/* o-circumflex */
			char_set = ISO_LATIN_1;
			break;
		    case 9:	/* Italian */
			char_code = 249;	/* u-grave */
			char_set = ISO_LATIN_1;
			break;
		    }
		break;
	    case '{':
		switch (_ld wvt$b_nrc_set)
		    {
		    case 6:	/* Finnish */
		    case 7:	/* German */
		    case 12:	/* Swedish */
		    case 10:	/* Swiss */
			char_code = 228;	/* a-umlaut */
			char_set = ISO_LATIN_1;
			break;
		    case 14:	/* French */
		    case 4:	/* French Canadian */
			char_code = 233;	/* e-acute */
			char_set = ISO_LATIN_1;
			break;
		    case 9:	/* Italian */
			char_code = 224;	/* a-grave */
			char_set = ISO_LATIN_1;
			break;
		    case 5:	/* Norwegian/Danish */
			char_code = 230;	/* ae */
			char_set = ISO_LATIN_1;
			break;
		    case 16:	/* Portuguese */
			char_code = 227;	/* a-tilde */
			char_set = ISO_LATIN_1;
			break;
		    case 15:	/* Spanish */
			char_code = 96;		/* grave accent */
			break;
		    }
		break;
	    case '|':
		switch (_ld wvt$b_nrc_set)
		    {
		    case 6:	/* Finnish */
		    case 7:	/* German */
		    case 12:	/* Swedish */
		    case 10:	/* Swiss */
			char_code = 246;	/* o-umlaut */
			char_set = ISO_LATIN_1;
			break;
		    case 14:	/* French */
		    case 4:	/* French Canadian */
			char_code = 249;	/* u-grave */
			char_set = ISO_LATIN_1;
			break;
		    case 9:	/* Italian */
			char_code = 242;	/* o-grave */
			char_set = ISO_LATIN_1;
			break;
		    case 5:	/* Norwegian/Danish */
			char_code = 248;	/* o-slash */
			char_set = ISO_LATIN_1;
			break;
		    case 16:	/* Portuguese */
			char_code = 231;	/* c-cedilla */
			char_set = ISO_LATIN_1;
			break;
		    case 15:	/* Spanish */
			char_code = 176;	/* degrees */
			char_set = ISO_LATIN_1;
			break;
		    }
		break;
	    case '}':
		switch (_ld wvt$b_nrc_set)
		    {
		    case 6:	/* Finnish */
		    case 5:	/* Norwegian/Danish */
		    case 12:	/* Swedish */
			char_code = 229;	/* a-circle */
			char_set = ISO_LATIN_1;
			break;
		    case 14:	/* French */
		    case 4:	/* French Canadian */
		    case 9:	/* Italian */
			char_code = 232;	/* e-grave */
			char_set = ISO_LATIN_1;
			break;
		    case 7:	/* German */
		    case 10:	/* Swiss */
			char_code = 252;	/* u-umlaut */
			char_set = ISO_LATIN_1;
			break;
		    case 16:	/* Portuguese */
			char_code = 245;	/* o-tilde */
			char_set = ISO_LATIN_1;
			break;
		    case 15:	/* Spanish */
			char_code = 241;	/* n-tilde */
			char_set = ISO_LATIN_1;
			break;
		    }
		break;
	    case '~':
		switch (_ld wvt$b_nrc_set)
		    {
		    case 6:	/* Finnish */
		    case 12:	/* Swedish */
			char_code = 252;	/* u-umlaut */
			char_set = ISO_LATIN_1;
			break;
		    case 14:	/* French */
			char_code = 168;	/* dieresis */
			char_set = ISO_LATIN_1;
			break;
		    case 4:	/* French Canadian */
		    case 10:	/* Swiss */
			char_code = 251;	/* u-circumflex */
			char_set = ISO_LATIN_1;
			break;
		    case 7:	/* German */
			char_code = 223;	/* sharp s */
			char_set = ISO_LATIN_1;
			break;
		    case 9:	/* Italian */
			char_code = 236;	/* i-grave */
			char_set = ISO_LATIN_1;
			break;
		    case 15:	/* Spanish */
			char_code = 231;	/* c-cedilla */
			char_set = ISO_LATIN_1;
			break;
		    }
		break;
	    }

if (( char_code == 160 ) &&
  !(( char_set == CRM_FONT_R ) && ( ext_char_set == ONE_BYTE_SET ))) {
       char_code = ERROR_CHARACTER_CODE;
       ext_char_set = ERROR_CHARACTER_EXT_SET;
       char_set  = ERROR_CHARACTER_SET;
} else if ( char_code == 32 || char_code == 160 ) {
    char_code = 32;
    char_set = ASCII;
    ext_char_set = STANDARD_SET;
}

/* following codes are identical to those in ansi_display() */
 
switch ( ext_char_set ) {
    case STANDARD_SET:
	char_code &= 127;
	switch ( char_set ) {
	    case LINE_DRAWING:
		if ( !ld->common.v1_encodings )
		    char_set = TECHNICAL;
		else if ( char_code >= 95 && char_code <= 126 )
		    char_code -= 95;
		break;
  	    case TECHNICAL:
		char_code = dec_technical[ char_code - 32 ];
		if ( !char_code ) {
		    char_code = ERROR_CHARACTER_CODE;
		    char_set = ERROR_CHARACTER_SET;
		    ext_char_set = ERROR_CHARACTER_EXT_SET;
		}
		else if ( ld->common.v1_encodings )
		    char_code &= 127;
		break;
	    case ISO_LATIN_1:
		if ( ld->common.v1_encodings )
		    char_code = iso_latin_1[ char_code - 32 ];
		else
		    char_code |= 128;
		break;
	    case SUPPLEMENTAL:
		if ( !dec_supplemental[ char_code - 32 ] ) {
		    char_code = ERROR_CHARACTER_CODE;
		    char_set = ERROR_CHARACTER_SET;
		    ext_char_set = ERROR_CHARACTER_EXT_SET;
		} else if ( !ld->common.v1_encodings )
		    char_code = dec_supplemental[ char_code - 32 ];
		else
		    char_code |= 128;
		break;

	    case TURKISH_SUPPLEMENTAL:
		if ( !turkish_dec_supplemental[ char_code - 32 ] ) {
		    char_code = ERROR_CHARACTER_CODE;
		    char_set = ERROR_CHARACTER_SET;
		    ext_char_set = ERROR_CHARACTER_EXT_SET;
		} else if ( !ld->common.v1_encodings )
		    char_code = turkish_dec_supplemental[ char_code - 32 ];
		else
		    char_code |= 128;
		break;

	    case GREEK_SUPPLEMENTAL:
		if ( !greek_dec_supplemental[ char_code - 32 ] ) {
		    char_code = ERROR_CHARACTER_CODE;
		    char_set = ERROR_CHARACTER_SET;
		    ext_char_set = ERROR_CHARACTER_EXT_SET;
		} else if ( !ld->common.v1_encodings )
		    char_code = greek_dec_supplemental[ char_code - 32 ];
		else
		    char_code |= 128;
		break;

	    case ISO_LATIN_7: /* v1_encodings is not supported
			       * in Greek.
			       */
		char_code |= 128;
		break;

	    case ISO_LATIN_5:
		char_code |= 128;
		if ( ld->common.v1_encodings ) {
		    if ( char_code < 0x40 )
			char_code = iso_latin_5[char_code - 32];
		    else if ( char_code < 0x60 ) {
			char_code = 2;
			char_set = LINE_DRAWING;
			ext_char_set = STANDARD_SET;
		    } else
			char_code |= 128;
		} else {
		    if (( char_code == 0x21 ) ||
			( char_code >= 0x3f && char_code <= 0x5e ) ||
			( char_code >= 0x7b && char_code <= 0x7f )) {
			char_code = ERROR_CHARACTER_CODE;
			char_set = ERROR_CHARACTER_SET;
			ext_char_set = ERROR_CHARACTER_EXT_SET;
		    } else
			char_code |= 128;
		}
		break;

	    case ASCII:
	    default:
		break;
	}
	break;

    case ONE_BYTE_SET:
	char_code &= 127;
	switch ( char_set ) {
	    case JIS_KATAKANA:
		char_code |= 128;
		if( char_code >= 224 && char_code <= 254 ) {
		    char_code = ERROR_CHARACTER_CODE;
		    ext_char_set = ERROR_CHARACTER_EXT_SET;
		    char_set = ERROR_CHARACTER_SET;
		}
		break;
	    case CRM_FONT_L:
	    case CRM_FONT_R:
		char_code |= 128;
		break;
	    case ISO_LATIN_8:
		if ( ld->common.v1_encodings ) {
		    if ( char_code < 0x40 )
			char_code = iso_latin_8[char_code - 32];
		    else if ( char_code < 0x60 ) {
			char_code = 2;
			char_set = LINE_DRAWING;
			ext_char_set = STANDARD_SET;
		    } else
			char_code |= 128;
		} else {
		    if (( char_code == 0x21 ) ||
			( char_code >= 0x3f && char_code <= 0x5e ) ||
			( char_code >= 0x7b && char_code <= 0x7f )) {
			char_code = ERROR_CHARACTER_CODE;
			char_set = ERROR_CHARACTER_SET;
			ext_char_set = ERROR_CHARACTER_EXT_SET;
		    } else
			char_code |= 128;
		}
		break;
	    case HEB_SUPPLEMENTAL:
		if ( ld->common.v1_encodings ) {
		    if ( char_code >= 0x40 && char_code < 0x60 ) {
			char_code = 2;
			char_set = LINE_DRAWING;
			ext_char_set = STANDARD_SET;
		    } else
			char_code |= 128;
		} else {
		    char_code = heb_dec_supplemental[ char_code - 32 ];
		    if ( !char_code ) {  /* undefined */
			char_code = ERROR_CHARACTER_CODE;
			char_set = ERROR_CHARACTER_SET;
			ext_char_set = ERROR_CHARACTER_EXT_SET;
		    } else if (( char_code == 0xaa ) || ( char_code == 0xba )) {
			char_set = SUPPLEMENTAL;
			ext_char_set = STANDARD_SET;
		    }
		}
		break;
	    case JIS_ROMAN:
	    case KS_ROMAN:
	    default:
		break;
	}
	break;
    case TWO_BYTE_SET:
    case FOUR_BYTE_SET:
    default:
	break;
}	/* switch ( ext_char_set ) */	

*display_code = char_code;
*display_char_set = char_set;
*display_ext_char_set = ext_char_set;
*ignore = FALSE;
}

/*****************************************************************************
 *    Kanji : 1978 <---> 1983 translation module.  ( modified from VT1200 )
 *                		EIC-J Yamao       - 014 -
 */
 
/* trans7883.c
 *
 * This module includes trans7883() only.
 *
 * trans7883( short )
 *    This function tranlates DEC Kanji 78 into DEC Kanji 83.
 *
 *    Argument     : Kanji character code
 *    Return Value : Kanji code for DEC Kanji 83.
 *
 *    Because this function doesn't check Kanji flag, this function 
 *    must be call after checking Kanji flag.
 *
 */

/* #include <stdio.h> */
/* Yamao deletes the line at 8-Jan-1991 */

/*----------------------------------------------------------------------*/
/* I.Seto deletes the code_table[] and code_table_reverse[]             */
/* (places them at the beggining of this files)                         */
/*----------------------------------------------------------------------*/

/* Yamao deletes some lines from here at 8-Jan-1991 */
/* unsigned short trans7883( char_code)
  unsigned short char_code;
{
  CTABLE *pointer = code_table;

  if( 0xa2af <= char_code && char_code <= 0xa2fe )
    return(0xa1a1);
  if( 0xa8a1 <= char_code && char_code <= 0xa8fe )
    return(0xa2a2);
  if( 0xf4a5 <= char_code && char_code <= 0xf4fe )
    return(0xa2a2);
  while(char_code > pointer->c83 && pointer->c83 != 0xfe2b ) pointer++ ;
  if( pointer->c83 == char_code )
    return (pointer->c78);
  else
    return (char_code);
} */

/* Yamao adds some lines from here at 8-Jan-1991 */
trans7883( first, second, buf )
  unsigned char first;
  unsigned char second;
  unsigned char buf[] ;
{
  CTABLE *pointer = code_table;
  unsigned short code16 ;

  code16 = first ;
  code16 = (( code16 << 8 ) & 0xff00 ) + second ;

  if( 0xa2af <= code16 && code16 <= 0xa2fe ) {
    buf[0] = buf[1] = 0xa1;
    return;
  }
  if( ( 0xa8a1 <= code16 && code16 <= 0xa8fe )  ||
		( 0xf4a5 <= code16 && code16 <= 0xf4fe ) ) {
    buf[0] = buf[1] = 0xa2;
    return;
  }
  while( code16 > pointer->c83 && pointer->c83 != 0xfe2b ) pointer++ ;
  if( pointer->c83 == code16 ) {
    buf[0] = (unsigned char)(( pointer->c78 >> 8 ) & 0x00ff ) ;
    buf[1] = (unsigned char)( pointer->c78 & 0x00ff ) ;
    return;
  }
  else{
    buf[0] = first ;
    buf[1] = second ;
    return;
  }
}
/* End of Yamao modification at 8-Jan-1991 */

/* unsigned short trans8378( first, second, buf ) */
/* Yamao deletes the line at 8-Jan-1991 */
trans8378( first, second, buf )
/* Yamao adds the line at 8-Jan-1991 */

  unsigned char first;
  unsigned char second;
  unsigned char buf[] ;
{
  CTABLE *pointer = code_table_reverse;
  unsigned short code16 ;

  code16 = first ;
  code16 = (( code16 << 8 ) & 0xff00 ) ;
  code16 += second ;
  while(code16 > pointer->c78 && pointer->c78 != 0xf4a4 ) pointer++ ;
  if( pointer->c78 == code16 ){
    buf[0] = (unsigned char)(( pointer->c83 >> 8 ) & 0x00ff ) ;
    buf[1] = (unsigned char)( pointer->c83 & 0x00ff ) ;
    return (pointer->c83);
  }
  else{
    buf[0] = first ;
    buf[1] = second ;
    return (code16);
  }
}
/* ===== End of Kanji code translation module.   EIC-J Yamao  - 014 - ===== */
