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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: gettytab.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/08/02 17:57:59 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint

#endif not lint

#include <ctype.h>
#include <stdio.h>
#include <syslog.h>

#define ACTIVE 		1
#define FINISHED	0
#define MAXLINE		255
#define MAXIDLENGTH	15
#define MAXMESSAGE	79

char  *skip();

/*
 * Get the default message if any from the gettydefs file.
 */
getent(bp, name)
        char *bp, *name;
{
	register char *ptr, c;
        int input,rawc,size;
	char oldc, *optr, quoted();
	char line[MAXLINE+1];
	static char d_id[MAXIDLENGTH+1];
	int linesize = 0;
	extern char *getword();
	FILE *fp;

	/* Open the gettydefs file		*/

	   fp = fopen("/etc/gettydefs","r");
           if (fp < 0)
               return(-1);
	   
	/* Read the file */
	   input = ACTIVE;   
	   do {
		for(ptr=line,linesize=0,oldc='\0'; ptr < &line[sizeof(line)] &&
                    (rawc = getc(fp)) != EOF; ptr++,linesize++,oldc = c) {
                        c = *ptr = rawc;

	/* Search for two \n's in a row. */
                        if (c == '\n' && oldc == '\n') break;
                }

	/* If we didn't end with a '\n' or EOF, then the line is too long. */
	/* Skip over the remainder of the stuff in the line so that we */
	/* start correctly on next line. */
              
		    if (rawc != EOF && c != '\n') {
                        for (oldc='\0'; (rawc = getc(fp)) != EOF;oldc=c) {
                                c = rawc;
                                if (c == '\n' && oldc != '\n') break;
                        }
	            }

	/* If we ended at the end of the file, then if there is no */
	/* input, break out immediately otherwise set the "input" */
	/* flag to FINISHED so that the "do" loop will terminate. */
                if (rawc == EOF) {
                        if (ptr == line) break;
                        else input = FINISHED;
                }

	/* If the last character stored was an EOF or '\n', replace it */
	/* with a '\0'. */
                if (line[linesize] == EOF || line[linesize] == '\n')
                        line[linesize] = '\0';

	/* If the next-to-last character stored was an '\n', replace it */
	/* with a '\0'. */
                if (line[linesize-1] == '\n')
                        line[linesize-1] = '\0';

	/* If the buffer is full, then make sure there is a null after the */
	/* last character stored. */
                else *++ptr == '\0';

	/* If line starts with #, treat as comment */
                if(line[0] == '#') continue;

	/* We have the complete first line scan it */

	   ptr = line;

	/* get the identifier			   */
	   
   	   strncpy(d_id,getword(ptr,&size),MAXIDLENGTH);

        /* If there is an "id", compare them. */

           if (strcmp(name,d_id) == 0) {

	/* Move to the next field.  If there is anything but space */
        /* following the id up until the '#', then it is failure   */

              ptr += size;
              while (isspace(*ptr)) ptr++;
                                if (*ptr != '#'){ 
				   return(-1);
				}
                                else 
                                   ptr++;  /* Skip the '#' */
	      /* Next field is iflags skip it */
		
	      ptr = skip(ptr);
	
	     /* Next field is fflags skip it */

	      ptr = skip(ptr); 

	      /* Next field is message. Copy until the next field*/
	      
		for (optr = bp; (c = *ptr) != '\0'
                                    && c != '#';ptr++) {

             /* If the next character is a backslash, then */
	     /* get the quoted character as one item       */
                    if (c == '\\') {
                       if (ptr[1] == '\n') {
                          ++ptr;
                          continue;
                    }
                    c = quoted(ptr,&size);
             /* -1 accounts for ++ that takes place later. */
                    ptr += size - 1;
                    }

             /* If there is room, store the next character in bp. */
                    if (optr < &bp[MAXMESSAGE])
                        *optr++ = c;
               }/* for */
	       if (c == '#') {
		  /* close the file */
	         fclose(fp);
		 *optr++ = '\0';
                 return(1);
	       }
	       else{
		 fclose(fp);
		 return(-1);
	       }
	   }/* if no match of id */
	   else{
	     fclose(fp);
	     return(-1);
	   }

	} while (input == ACTIVE);
	fclose(fp);
	return(-1);

}

static char *
skip(ptr)
register char *ptr;

{
	char *word,*getword();
	int size;

	while (*ptr != '#' && *ptr != '\0') {

	/* Pick up the next word in the sequence */
	     word = getword(ptr,&size);

	/* Advance pointer to after the word. */

	     ptr += size;
	}
	ptr++;
	return(ptr);

}


char *getword(ptr,size)
register char *ptr;
int *size;
{

      register char *optr,c;
        char quoted();
        static char word[MAXIDLENGTH+1];
        int qsize;

        /* Skip over all white spaces including quoted spaces and tabs. */
        for (*size=0; isspace(*ptr) || *ptr == '\\';) {
                if (*ptr == '\\') {
                        c = quoted(ptr,&qsize);
                        (*size) += qsize;
                        ptr += qsize+1;
                        /*
                         * If this quoted character is not a space or a
                         * tab or a newline then break.
                         */
                        if (isspace(c) == 0) break;
                } else {
                        (*size)++;
                        ptr++;
                }
        }

        /*
       * Put all characters from here to next white space or '#' or '\0'
         * into the word, up to the size of the word.
         */
        for (optr= word,*optr='\0'; isspace(*ptr) == 0 &&
            *ptr != '\0' && *ptr != '#'; ptr++,(*size)++) {

                /* If the character is quoted, analyze it. */
                if (*ptr == '\\') {
                        c = quoted(ptr,&qsize);
                        (*size) += qsize;
                        ptr += qsize;
                } else c = *ptr;

                /* If there is room, add this character to the word. */
                if (optr < &word[MAXIDLENGTH+1] ) *optr++ = c;
        }

        *optr++ = '\0';
        return(word);
}
/*      "quoted" takes a quoted character, starting at the quote        */
/*      character, and returns a single character plus the size of      */
/*      the quote string.  "quoted" recognizes the following as         */
/*      special, \n,\r,\v,\t,\b,\f as well as the \nnn notation.        */

char quoted(ptr,qsize)
char *ptr;
int *qsize;
{
        register char c,*rptr;
        register int i;

        rptr = ptr;
        switch(*++rptr) {
        case 'n':
                c = '\n';
                break;
        case 'r':
                c = '\r';
                break;
        case 'v':
                c = '\013';
                break;
     case 'b':
                c = '\b';
                break;
        case 't':
                c = '\t';
                break;
        case 'f':
                c = '\f';
                break;
        default:
                /*
                 * If this is a numeric string, take up to three characters of
                 * it as the value of the quoted character.
                 */
                if (*rptr >= '0' && *rptr <= '7') {
                        for (i=0,c=0; i < 3;i++) {
                                c = c*8 + (*rptr - '0');
                                if (*++rptr < '0' || *rptr > '7') break;
                        }
                        rptr--;
                /*
                 * If the character following the '\\' is a NULL, back up the
                 * ptr so that the NULL won't be missed.  The sequence
                 * backslash null is essentially illegal.
                 */
                } else if (*rptr == '\0') {
                        c = '\0';
                        rptr--;

                /* In all other cases the quoting does nothing. */
                } else c = *rptr;
                break;
        }

        /* Compute the size of the quoted character. */
        (*qsize) = rptr - ptr + 1;
        return(c);
}


