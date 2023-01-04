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
static char rcsid[] = "@(#)$RCSfile: conv.c,v $ $Revision: 1.1.4.6 $ (DEC) $Date: 1994/01/24 22:57:04 $";
#endif
/*
 * HISTORY
 */

/*  conv.c - Rudimentary multi-language support for mailx.
**
**  Mail is unique amongst I18N applications.  There is the current
**  user's codeset (and local settings like currency, date format, etc),
**  and the codeset that we wish for the message body (or the codeset
**  that already exists on incoming mail).  Adding to this complexity
**  is the concept of codeset conversion, i.e. the same language,
**  represented in different character sets.  Happy, happy, joy, joy!
**
**  The added I18n support is not quite full MIME.  The intent was
**  to be able to send normal text messages in things other than
**  US-ASCII.  So, this version:
**	- supports only simple text/plain messages ; no other content-types
**	(e.g multi-part, image, etc) are supported.
**	- doesn't support RFC-1522 header encodings; it directly writes
**	non-ascii text into the header (bad).
**	- doesn't support transfer-encodings.  So, if the charset you
**	are using has the 8th bit on, your MTA (e.g. sendmail) better
**	be 8-bit capable.
**	- assumes that US-ASCII and NULL are a subset of the charset,
**	i.e. wide chars can be scanned as single-bytes.  This seems
**	to be a reasonable (and needed) assumption from X/Open.
**  TBD: better parsing of the content-type field.
**	Replacement of this stuff with meta-mail?
**	Encoding of the headers fields as per RFC 1522.
**	Case insensitive scan of the excode language sets.
**  RWu 1/4/94
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <nl_types.h>
#include <langinfo.h>
#include <iconv.h>
#include <locale.h>

#include "rcv.h"
#include "Mail_msg.h"
extern nl_catd catd;
#define calloc __calloc
#include "stdlib.h"
#undef calloc

#define	EXCODE		"EXCODE"	/* name of the environment variable */
#define MP_EXCODE	"excode"	/* name used in .mh_profile */
#define	LANG		"LANG"		/* name of the environment variable */
#define	LCALL		"LC_ALL"	/* name of the environment variable */
#define MP_LANG		"lang"		/* name used in .mh_profile */

#define	MBUFSIZE	512
#define MAXNAMLEN	64		/* maximum length of esc seq name */


/*  Codes used for ICONV_SERRACTION; i.e. set error action if a problem
**  occurs during code-set conversion.
**  This functionality is undocumnted for the user.
*/
#define	NO_HEADER	1
#define	SENDLANG 	2		/* Print warning and continue */
#define	ABORT		3		/* exit on error */


int	error_action = 0;		/* set by ICONV_SERRACTION */
int	out_no_convert=0;
int	mb_lang=0;			/* Multi-byte language for locale? */
char	*codeset=NULL;			/* User's code-set */
char	*def_codeset = "US-ASCII";

char	Content[]="Content-Type";
char	Charset[]="charset=";
char	Mime[]="Mime-Version: 1.0";

char	*excode=NULL;			/* Current Exchange Code */
char    *sys_excode=NULL;		/* System-wide dflt excode; unused, undoc'd */

struct	{
	iconv_t cvt;
	long size;
	char *filename;
	FILE *fileptr;
	int  shift_state_code;
	char *excode;
	}	tempfiles[MAX_NOFILE];

struct	{
	char *excode;
	iconv_t cvt;
	}	cvt_tab[64];	/* we can open ~ 70 iconv's only */
int	cvt_count=0;	/* counter for cvt_tab */
char	shift_out[]={27, '(', 'B'};	/* for resetting shifted state */


#define OUT_MESS 1
#define IN_MESS 0

#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s)
#define CONTENT_SIZE sizeof(Content)-1

iconv_t	cvt_out, cvt_in;	/* libiconv descriptors for in and out mails */


/*
 ***************************************************************************
 * Utility routines
 ***************************************************************************
 */

/*
 * Local get copy routines.  The savestr routine is not used because it calls
 * salloc() to allocate space.  The space alloacated by salloc() will
 * be cleared by sreset() which is not under our control.  We may lose
 * information or screw up easily if so.
 */
char *_savestr(char *str)
{
        register char *cp, *top;
	register int size;

        for (cp = str; *cp; cp++);
        top = malloc(size=cp-str+1);
        if (top == NULLSTR) return(NULLSTR);
	bcopy(str, top, size);
        return(top);
}


/*
 * Lookup hash table storing values of entries in ~/.mailrc
 */
char	*m_find(name)
char	*name;
{
	register struct var *vp;

	vp = lookup(name);
	if (vp == NOVAR) return(NULL);
	else return(_savestr(vp->v_value));
}


/*
 * Get right part of str starting from and excluding chr
 * Returns str if chr is not found.
 */
char *r1bindex(str, chr)
register char *str;
register int chr;
{
    register char  *cp;

    for (cp = str; *cp; cp++);
    --cp;
    while (cp >= str && *cp != chr)
	--cp;
    return (++cp);
}


/*
 * Create tempfiles for a message.
 * Returns: 0 if a new tmpfile was opened;
 *	!0 if file already existed (ass: message already converted)
 * Sets: the filename, and fileptr in tempfiles[].
 * Position 0 is reserved for outmail use, i.e. the file is closed
 * and re-opened.
 */
int create_tempfile(int msg, FILE **otfp)
{
	char temp[PATH_MAX];
	int size;


	if (msg) {
		if (tempfiles[msg].filename) {
			*otfp=tempfiles[msg].fileptr;
			fseek(*otfp, 0, 0);
			return (1);
		}
		sprintf(temp,"%s/mail%d%d", gettmpdir(), msg, getpid());
		size=strlen(temp)+1;
		tempfiles[msg].filename=malloc(size);
		bcopy(temp, tempfiles[msg].filename, size);
	}
	else if (tempfiles[0].filename) {	/* close old one */
		fclose(tempfiles[0].fileptr);
		/* TBD - should the old file's contents be destroyed first? */
	}
	else {	/* prepare file name */
		sprintf(temp,"%s/mail0%d", gettmpdir(), getpid());
		size=strlen(temp)+1;
		tempfiles[0].filename=malloc(size);
		bcopy(temp, tempfiles[msg].filename, size);
	}
	if ((*otfp = fopen(tempfiles[msg].filename, "w+")) == NULL) {
		perror(tempfiles[msg].filename);
		exit(1);
	}
	tempfiles[msg].fileptr=*otfp;
	return(0);
}


show_file(FILE *file)
{
	char buf[LINESIZE];
	long pos;

	pos=ftell(file);
	while (fgets(buf, LINESIZE, file) != NULL)
		printf("%s", buf);
	fseek(file, pos, 0);
}


iconv_t _iconv_open(char *to, char *from)
{
	register int i;

	cvt_tab[0].excode=from;
	for (i=cvt_count; strcmp(from, cvt_tab[i].excode); i--);
	if (i)		/* found it */
		return(cvt_tab[i].cvt);
	else {
		cvt_tab[++cvt_count].excode=_savestr(from);
		cvt_tab[cvt_count].cvt=iconv_open(to, from);
		return(cvt_tab[cvt_count].cvt);
	}
}


/*
 *********************************************************************
 * Routines relating to error actions when conversion is unsuccessful
 *********************************************************************
 */
get_error_action()
{
	char	*action;

	action = (char *)getenv("ICONV_SERRACTION");
	if (action) return(check_action(action));

	action = (char *)m_find("iconv_serraction");
	if (action) return(check_action(action));
	
	return(NO_HEADER);
}

check_action(char *action)
{
	char	*p;

	p = action;
	while(*p) 
	{
		*p |= 0x20;
		p++;
	}

	if(!strcmp(action,"noheader")) return(NO_HEADER);
	if(!strcmp(action,"sendlang")) return(SENDLANG);
	if(!strcmp(action,"abort")) return(ABORT);
	return(NO_HEADER);
}

void take_error_action(int errno)
{
	if (errno==EILSEQ) {	/* other cases are handled by caller */
		printf(MSGSTR(INVALIDCHAR,
			"Invalid character in subject/message line.\n"));
		switch (error_action) {
			case ABORT:
				printf(MSGSTR(ABORTNOW,
					"Operation aborted.\n"));
				exit(1);
			case SENDLANG:
			case NO_HEADER:
				printf(MSGSTR(NOCONVERT,
					"Subsequent message will not be converted.\n"));
				break;
			default:
				printf(MSGSTR(INTERNAL, "Internal error"));
				break;
		}
	}
}


/*
 *********************************************************************
 * Routines to get locale and excode info.
 *********************************************************************
 */

/*
 * Get value of entry pcEntry in file pcCode
 */
char	*m_getcontent (char *pcCode)
{
	FILE	*fp;
	char	buf[MBUFSIZE];
	char	*cp, *cp1;
	wchar_t wc;
	int mb;

	if ((fp = fopen (pcCode, "r")) == NULL)
		return(NULL);

	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		cp=buf;
		if (*cp=='#') continue;	/* skip comment */
		while (*cp && ISSPACE(wc, cp, mb)) 
			cp+=mb;	/* skip leading space */
		if (!(*cp)) continue;	/* just an empty line */
		fclose(fp);	/* found content if come to here */
		cp1=cp;
		while (*cp1 && !ISSPACE(wc, cp1, mb))
			cp1+=mb;	/* skip till trailing space */
		*cp1='\0';
		return(_savestr(cp));
	}
	fclose(fp);	/* cannot find if come to here */
	return(NULL);
}



void get_excode(char **excode_ptr, char **sys_excode_ptr)
{
	char *excode;

	*sys_excode_ptr = m_getcontent(MCODESET);	/* system excode */
	*excode_ptr=(char *)NULL;

	excode = (char *)getenv(EXCODE);	/* environment var */
	if (excode) {
		*excode_ptr=_savestr(excode);
		return;
	}

	*excode_ptr = m_find(MP_EXCODE);	/* ~/.mailrc */
	if (*excode_ptr) return;

	if (*sys_excode_ptr) {			/* /usr/lib/mail-codesets */
		*excode_ptr=_savestr(*sys_excode_ptr);	
		return;
	}
}

char	*get_lang()
{
	char	*lang;

	lang = (char *)getenv(LCALL) ;
	if (lang) return(_savestr(lang));

	lang = (char *)getenv(LANG);
	if (lang) return(_savestr(lang));

	lang = (char *)m_find(MP_LANG);
	if (lang) return(lang);

	return(_savestr("C"));		/* default to C */
}


/*
 * Extract the charset part of lang, excode, sys_excode.
 */
void get_charsets(char **codeset_ptr, char **excode_ptr, char **sys_excode_ptr)
{
	char *cp;
	char *lang;

	/* Get codeset for current locale */
	lang=*codeset_ptr;
        if (!setlocale(LC_ALL, lang)) {
                printf(MSGSTR(INVALIDLOC, "Cannot set the %s locale.\n"),
                        lang);
                printf(MSGSTR(DEFAULTLOC, "Will use default C locale.\n"));
		printf(MSGSTR(BOTHNOCONVERT,
		 	"Both incoming and outgoing messages will not be converted.\n"));
		*codeset_ptr=NULL;
                setlocale(LC_ALL, "C");
	}
	else {
		if (!strcmp(lang, "C") || !strcmp(lang, "POSIX"))
                        *codeset_ptr=_savestr(def_codeset);
                else if (index(lang, '.') == NULL) {
			*codeset_ptr = nl_langinfo(CODESET);
			if (*codeset_ptr)
				*codeset_ptr=_savestr(*codeset_ptr);
		}
		else
			*codeset_ptr = _savestr(r1bindex(lang, '.'));
	}
        mb_lang=(MB_CUR_MAX>1) ? 1 : 0;
	if (lang) free(lang);

	/* Get codeset for excode */
	if (*excode_ptr) {
		cp = _savestr(r1bindex(*excode_ptr, '.'));
		free(*excode_ptr);
		if (!strcmp(cp, "C") || !strcmp(cp, "POSIX"))
			*excode_ptr=_savestr(def_codeset);
		else *excode_ptr=cp;
	}

	if (*sys_excode_ptr) {
		cp = _savestr(r1bindex(*sys_excode_ptr, '.'));
		free(*sys_excode_ptr);
		if (!strcmp(cp, "C") || !strcmp(cp, "POSIX"))
			*sys_excode_ptr=_savestr(def_codeset);
		else *sys_excode_ptr=cp;
	}
}


/*
 * Scan message for it's charset.
 * Returns: NULL if no charset is found, or a _savestr of the type.
 * Modifies: *shift_state_code_ptr if the code type is ISO-2022 (a shift
 *	state character set).
 */
char *get_in_excode(FILE *ibuf,int *shift_state_code_ptr)
{
	long	pos;
	int	len;
	char	buf[LINESIZE], *cp, *cp1;

	pos=ftell(ibuf);
	cp=buf;
	while(fgets(buf, sizeof(buf), ibuf) != NULL)
	{
		if (*cp=='\n') {
			fseek(ibuf, pos, 0);
			return(NULL);	/* header expired */
		}
		/*  A simplistic scan for charset.  Assume the form is
		**  "content-type: text/plain; charset="
		*/
		if (!icnequal(cp, Content, CONTENT_SIZE))
		    continue;
		cp=buf+CONTENT_SIZE;
		while (*cp && *cp != ';') cp++;	/* content type */

		while (*cp && *cp != 'c'){
		    /*  Ass: must be "charset=" parameter".
		    **  TBD: ignore white-space
		    */
		    cp++;
		}
		cp += sizeof(Charset)-1;
		if (cp > buf + strlen(buf))
		    continue;

		if (icnequal(cp, "X-", sizeof("X-")-1))
		    cp+=sizeof("X-")-1;
		else if (icnequal(cp, "ISO-2022", sizeof("ISO-2022")-1)) 
		    (*shift_state_code_ptr)++;

		/*  TBD: is this scan for the charset robust enough? */
		for (cp1=cp; *cp1 && !isspace(*cp1); cp1++);
		*cp1='\0';

		fseek(ibuf, pos, 0);
		return(_savestr(cp));
	}
	fseek(ibuf, pos, 0);
	return(NULL);	/* cannot find if come to here */
}


/*
 *********************************************************************
 * Routine determining conversion methods
 *********************************************************************
 */
void get_conversions(char *codeset, char *excode, char *sys_excode)
{
	cvt_in=cvt_out=(iconv_t)0;
	if (codeset && !strcmp(codeset, def_codeset)) return;

	if (codeset && sys_excode && strcmp(codeset, sys_excode)) {
		cvt_in = iconv_open(codeset, sys_excode);
		cvt_tab[1].excode=_savestr(sys_excode);
		cvt_tab[1].cvt=cvt_in;
		cvt_count=1;
	}

	if (codeset && excode && strcmp(excode, codeset)) {
		cvt_out = iconv_open(excode, codeset);
	}
}


/*
 ***************************************************************************
 * Actual conversion routines
 ***************************************************************************
 */
char *convert_line(char *linebuf, iconv_t cvt)
{
	size_t	nb0, nb1, nb_max; 
	char	*outbuf, *lp0, *lp1;

        if (!cvt || cvt==(iconv_t)-1) return (NULL);

	if (linebuf) {
		nb0 = strlen(linebuf);
		nb_max=nb0*MB_MAX+1;
		lp1=outbuf=salloc(nb_max);
 		lp0 = linebuf;	
		errno=0;
		if (nb1 = iconv(cvt, &lp0, &nb0, &lp1, &nb_max)) {
			take_error_action(errno);	/* must be cvt error */
			return(NULL);
		}
		else { 
			*lp1=0;
			linebuf=outbuf;
		}
	}
	return(linebuf);
}



/* 
 * Change the Content-Type header in the file (not the message structure)
 * to reflect the charset of the application.
 * Assume line is long enough and nothing else following charset entry
 */
change_charset(char *line, int msg, int charset_opt)
{
	int len;
	register char *cp=line+CONTENT_SIZE;
	iconv_t cvt=tempfiles[msg].cvt;

	if (!icnequal(line, Content, CONTENT_SIZE)) return(0); 
	if (charset_opt==SET_CODESET && (!cvt || cvt==(iconv_t)-1))
		return(0);
	if (charset_opt==SET_EXCODE && (!cvt_out || cvt_out==(iconv_t)-1))
		return(0);

	while (*cp && *cp++ != '=');	/* skip till charset entry */

	if (!icnequal(codeset, "ISO", sizeof("ISO")-1)) {
		bcopy("X-", cp, 2);
		cp+=2;
	}

	if (charset_opt==SET_CODESET) {
		len=strlen(codeset);
		bcopy(codeset, cp, len);
	}
	else {
		len=strlen(excode);
		bcopy(excode, cp, len);
	}
	cp+=len;
	*cp++='\n';
	*cp='\0';
}



/*
 * Perform char. convertion on entire file.
 * Returns pointer to the converted file (or the original file).
 * The file is not converted if an error occurs.
 * If this is an outbound file (msg==0), then the `Content-Type' header
 * is modified to reflect the new charset.
 */
FILE *convert_content(FILE *mtf, iconv_t cvt, int msg, long limit)
{
	static char linebuf[LINESIZE], *lp0, *lp1;
	static char outbuf[LINESIZE*MB_MAX+1];
	size_t nb_max=LINESIZE*MB_MAX+1;
	FILE *otf;
	int error_found=0;
	long pos, chcnt=0;
	size_t nb0, nb1, nb_tmp;
	register struct message *mp;

        if (!cvt || cvt==(iconv_t)-1) return (mtf);

	/* already converted */
	if (create_tempfile(msg, &otf)) return (otf);

       	lp0 = linebuf;	
	nb0=0;
	pos=ftell(mtf);
	while (fgets(lp0, sizeof(linebuf)-nb0, mtf) != NULL  && chcnt < limit) {
		if (!msg)	/* out mail, cvt all charset lines to excode */
			change_charset(linebuf, msg, SET_EXCODE);
		nb0 = strlen(linebuf);
		chcnt+=nb0;
		errno=0;
		lp1=outbuf;
		nb_tmp=nb_max;
		nb1 = iconv(cvt, &lp0, &nb0, &lp1, &nb_tmp);
		*lp1 = '\0';
		if (nb1) {
			if (errno==EINVAL) {  /* partial char */
				if (fputs(outbuf, otf)==EOF) {
					perror("mailx");
					exit(1);
				}
				bcopy(lp0, outbuf, nb0);
				bcopy(outbuf, linebuf, nb0);
				linebuf[nb0]='\0';
				lp0=linebuf+nb0;
			}
			else {	/* must be cvt error */
				take_error_action(errno);
				fclose(otf);
				error_found=1;
				break;
			}
		}
		else {
			if (fputs(outbuf, otf)==EOF) {
				perror("mailx");
				exit(1);
			}
			lp0=linebuf;
			nb0=0;
		}
	}

	if (error_found || nb0)	{ /*nb0>0 -> partial char after final read*/
		fclose(otf);
		tempfiles[msg].filename=NULL;
		fseek(mtf, pos, 0);
	}
	else {
		fflush(otf);
		fseek(otf, 0, 0);
		tempfiles[msg].size=fsize(otf);
		mtf=otf;
	}
	return(mtf);
}


/*
 ************************************************************************
 * Internal interface routines
 ************************************************************************
 */

/*
 * convert sent subject line and message content.
 */
convert_out_mail(struct header *hp, FILE **mtfp)
{
	char	*linebuf;
	FILE	*otf;

	out_no_convert=1;

        if (!cvt_out || cvt_out==(iconv_t)-1) return;

	linebuf=convert_line(hp->h_subject, cvt_out);

	if (hp->h_subject && !linebuf) return;

	rewind(*mtfp);
	otf=convert_content(*mtfp, cvt_out, 0, (long)fsize(*mtfp));
	if (otf != *mtfp) {	/* has cvt'ed */
		out_no_convert=0;	/* to be used by put_mime_header */
		if (linebuf) hp->h_subject=(char *)linebuf;
		*mtfp=otf;
	}
}


/*
 * convert incoming message content if possible.
 */
convert_in_mail(FILE **ibufp, int msg, int charset_opt)
{
	FILE *atmf;	/* asian tmp mail file */
	long limit;
	struct message *mp;
	char *in_excode=NULL;
	iconv_t cvt;
        int just_read=0;

	if (!tempfiles[msg].excode) {
		in_excode=get_in_excode(*ibufp, 
			&tempfiles[msg].shift_state_code);
                tempfiles[msg].excode=in_excode ? _savestr(in_excode)
                                                : (char *)-1;
                just_read=1;
        } else if (tempfiles[msg].excode != (char *)-1)
                in_excode=tempfiles[msg].excode;

        if (!codeset || !mb_lang) return;       /* do only if mb locale */

        if (!in_excode) {
                printf(MSGSTR(INNOEXCODE,
                        "This mail has no exchange code header.\n"));
                if (!sys_excode) {                      /* can do nothing */
                        printf(MSGSTR(INNOCONVERT,
                            "Message content will not be converted to your application codeset.\n"));
                        return;
                }

                /* use default codeset in mail-codesets */
                printf(MSGSTR(USEDEFEXCODE,
                        "Will use interchange codeset %s in %s.\n"),
                        sys_excode, MCODESET);
                if (!cvt_in) return;            /* codeset==sys_excode */
                tempfiles[msg].cvt=cvt=cvt_in;
                in_excode=sys_excode;
        } else if (!just_read) {
                if (!tempfiles[msg].cvt) return;        /* codeset=excode */
                cvt=tempfiles[msg].cvt;
        } else {
                if (!strcmp(codeset, in_excode)) return;
                tempfiles[msg].cvt=cvt=_iconv_open(codeset, in_excode);
	}

        if (cvt==(iconv_t)-1) {
                printf(MSGSTR(CANTCONVERT,
                        "Cannot convert from codeset %s to %s\n"),
                        in_excode, codeset);
                printf(MSGSTR(INNOCONVERT,
                        "Message content will not be converted to your application codeset.\n"));
                return;
        }

	mp=&message[0]+msg;
	limit=mp->m_block;
	limit <<= 9;
	limit = limit + mp->m_offset - ftell(*ibufp);
	atmf=convert_content(atmf=*ibufp, cvt, msg, limit);
	if (atmf != *ibufp)	/* has cvt'ed */
		*ibufp=atmf;
}


/*
 * convert incoming message header if possible.
 * This is less expensive than convert_in_mail
 */
char *convert_in_header(FILE *ibuf, int initpos, char *inline, int msg)
{
	long pos;
	char *in_excode=NULL;
	iconv_t cvt;
	char *cp;
        int just_read=0;

	if (!tempfiles[msg].excode) {
		pos=ftell(ibuf);
		fseek(ibuf, initpos, 0);	/* search from file start */
		in_excode=get_in_excode(ibuf, 
			&tempfiles[msg].shift_state_code);
		fseek(ibuf, pos, 0);
                tempfiles[msg].excode=in_excode ? _savestr(in_excode)
                                                : (char *)-1;
                just_read=1;
        } else if (tempfiles[msg].excode != (char *)-1)
                in_excode=tempfiles[msg].excode;

        if (!codeset || !mb_lang) return (inline);  /* do only if mb locale */

        if (!in_excode) {
                if (!sys_excode) return(inline);        /* can do nothing */
                if (!cvt_in) return(inline);    /* codeset==sys_excode */
                tempfiles[msg].cvt=cvt=cvt_in;
        } else if (!just_read) {
                if (!tempfiles[msg].cvt) return(inline);/* codeset=excode */
                cvt=tempfiles[msg].cvt;
        } else {
                if (!strcmp(codeset, in_excode)) return(inline);
                tempfiles[msg].cvt=cvt=_iconv_open(codeset, in_excode);
        }
	if (cvt==(iconv_t)-1) return (inline);
	if (cp=convert_line(inline, cvt)) return(cp);
	else return(inline);
}



/*
 * Initialization routine for setting up conversions
 */
void init_conversion()
{
	error_action = get_error_action();

	get_excode(&excode, &sys_excode); 
	codeset=get_lang();
	get_charsets(&codeset, &excode, &sys_excode);
	get_conversions(codeset, excode, sys_excode);

	if (debug) {
		printf("in init_conversion, excode is %s, codeset is %s, sys_excode is %s, error_action is %d\n", excode, codeset, sys_excode, error_action);

	}
	memset(tempfiles, 0, sizeof(tempfiles));

        if (!mb_lang) return;   /* dont show message if not mb locale */

        if (!excode) {
                printf(MSGSTR(NOEXCODE,
                        "Mail interchange code not defined.\n"));
                printf(MSGSTR(OUTNOCONVERT,
                        "Outgoing messages will not be converted to the mail interchange code.\n"));
        }
        else if (cvt_out==(iconv_t)-1) {
                printf(MSGSTR(CANTCONVERT,
                        "Cannot convert from codeset %s to %s\n"),
                        codeset, excode);
                printf(MSGSTR(OUTNOCONVERT,
                        "Outgoing messages will not be converted to the mail interchange code.\n"));
        }
}


/*
 * Put MIME-Version & Content-Type headers in outgoing mail
 */
put_mime_header(FILE *fo)
{
	int	ret,len;
	char	*cp;

        if ( (cvt_out && cvt_out!=(iconv_t)-1 && !out_no_convert) ||
	     (codeset && excode && !strcmp(codeset, excode)) ) {
		fputs(Mime, fo); fputc('\n', fo);
		fputs(Content, fo); fputs(": TEXT/PLAIN; ", fo);
		fputs(Charset, fo); 
		cp=excode;
	}
	else if (error_action==SENDLANG) {
		fputs(Mime, fo); fputc('\n', fo);
		fputs(Content, fo); fputs(": TEXT/PLAIN; ", fo);
		fputs(Charset, fo); 
		if (codeset) cp=codeset;
		else cp=def_codeset;
	}
	else return;

	if (strcmp(cp, def_codeset) && strncmp(cp,"ISO",3))
		fputs("X-", fo);
	fputs(cp, fo);
	putc('\n', fo);
}


/*
 * Remove the tempfile created by convert_content
 */
remove_tempfiles()
{
	register int j;

	for (j=0; j<=msgCount; j++) {
		if (tempfiles[j].filename) {
			fclose(tempfiles[j].fileptr);
			remove(tempfiles[j].filename);
			free(tempfiles[j].filename);
		}
	}
	for (j=1; j<=cvt_count; j++) {
		if (cvt_tab[j].excode) free (cvt_tab[j].excode);
                if (cvt_tab[j].cvt && cvt_tab[j].cvt != (iconv_t)-1)
                        iconv_close(cvt_tab[j].cvt);
	}
	if (cvt_in && cvt_in != (iconv_t)-1) iconv_close(cvt_in);
	if (cvt_out && cvt_out != (iconv_t)-1) iconv_close(cvt_out);
}	



/*
 * Set the input string to occupy at least minlen but no longer than maxlen
 * spaces on screen.
 * minlen=0: no space padding
 * maxlen=0: no length limit
 */
fix_len_header(char **header, int minlen, int maxlen, int msg)
{
	register int mb, worklen;
	wchar_t wc;
	register char *cp, *cp1, *buf;
	int rightadj=1, headsize, padsize;

	if (!header || !*header) return(0);
	if (maxlen && minlen>maxlen) return(0);
	cp=buf=*header;

	if (minlen<0) {
		rightadj=0;
		minlen=-minlen;
	}
	for (worklen=0, cp=*header;
	     worklen<minlen && *cp && 
             (mb= (codeset&&mb_lang) ? mbtowc(&wc, cp, MB_MAX) : 1)!=-1;
             worklen+= (codeset&&mb_lang) ? wcwidth(wc) : 1, cp+=mb);
	if (worklen<minlen) {		/* pad spaces after last (valid) char */
		headsize=cp-(*header);
		padsize=minlen-worklen;
		cp1=buf=salloc(headsize+padsize+4);
		if (rightadj) {
			memset(cp1, ' ', padsize); 	cp1+=padsize;
			bcopy(*header, cp1, headsize); 	cp1+=headsize;
                        if (tempfiles[msg].shift_state_code &&
                            (!tempfiles[msg].cvt ||
                             tempfiles[msg].cvt == (iconv_t)-1)) {
				bcopy(shift_out, cp1, 3);	cp1+=3;
			}
		}
		else {
			bcopy(*header, buf, headsize);	cp1+=headsize;
			memset(cp1, ' ', padsize);	cp1+=padsize;
                        if (tempfiles[msg].shift_state_code &&
                            (!tempfiles[msg].cvt ||
                             tempfiles[msg].cvt == (iconv_t)-1)) {
				bcopy(shift_out, cp1, 3);	cp1+=3;
			}
		}
		*cp1='\0';
		*header=buf;
		return;		/* no need to consider maxlen */
	}
		
	if (maxlen) {
		while(*cp && worklen<maxlen && 
                      (mb=(codeset&&mb_lang) ? mbtowc(&wc,cp,MB_MAX) : 1)!=-1) {
                        worklen+= (codeset&&mb_lang) ? wcwidth(wc) : 1;
			cp+=mb;
		}
		if (worklen>maxlen) cp-=mb;
		*cp='\0';	/* show up to (valid) char */
                if (tempfiles[msg].shift_state_code &&
                    (!tempfiles[msg].cvt ||
                     tempfiles[msg].cvt == (iconv_t)-1)) {
		/* we dont know if we have space to append shift_out, so
		   we allocate new buffer for it */
			headsize=cp-(*header);
			cp1=buf=salloc(headsize+4);
			bcopy(*header, cp1, headsize);	cp1+=headsize;
			bcopy(shift_out, cp1, 3);	cp1+=3;
			*cp1='\0';
			*header=buf;
		}
	}
} 


/*
 * Return the file size of the converted file size
 */
long cvtfile_size(struct message *mp)
{
	register int msg;

	msg=mp-&message[0]+1;
	if (!tempfiles[msg].filename) return(mp->m_size);
	else return(tempfiles[msg].size);
}

