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
static char	*sccsid = "@(#)$RCSfile: y4.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/08/02 19:43:58 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: aoutput, callopt, gin, gtnm, nxti, osummary, stin
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

# include "dextern"
#ifndef _BLD /*osf compatibility */
#include "yacc_msg.h"
extern nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd,MS_YACC,Num,Str)
#else
#define MSGSTR(Num,Str) Str
#endif

# define a amem
# define mem mem0
# define pa indgo
# define yypact temp1
# define greed tystate

int * ggreed;
int * pgo = wsets[0].ws.lset;
int *yypgo = &nontrst[0].tvalue;

int maxspr = 0;  /* maximum spread of any entry */
int maxoff = 0;  /* maximum offset into a array */
int *pmem;
int *maxa;
# define NOMORE -1000

int nxdb = 0;
int adb = 0;

callopt(){

        register i, *p, j, k, *q;

        pmem = mem;
        ggreed = lkst[0].lset;
        /* read the arrays from tempfile and set parameters */

        if( (finput=fopen(TEMPNAME,"r")) == NULL ) error( MSGSTR(NOOPT,
                "optimizer cannot open tempfile") ); /*MSG*/

        pgo[0] = 0;
        yypact[0] = 0;
        nstate = 0;
        nnonter = 0;
        for(;;){
                switch( gtnm() ){

                case '\n':
                        yypact[++nstate] = (--pmem) - mem;
                case ',':
                        continue;

                case '$':
                        break;

                default:
                        error( MSGSTR(BADTEMP, "bad tempfile") ); /*MSG*/
                        }
                break;
                }

        yypact[nstate] = yypgo[0] = (--pmem) - mem;

        for(;;){
                switch( gtnm() ){

                case '\n':
                        yypgo[++nnonter]= pmem-mem;
                case ',':
                        continue;

                case EOF:
                        break;

                default:
                        error( MSGSTR(BADTEMP, "bad tempfile") ); /*MSG*/
                        }
                break;
                }

        yypgo[nnonter--] = (--pmem) - mem;



        for( i=0; i<nstate; ++i ){

                k = 32000;
                j = 0;
                q = mem + yypact[i+1];
                for( p = mem + yypact[i]; p<q ; p += 2 ){
                        if( *p > j ) j = *p;
                        if( *p < k ) k = *p;
                        }
                if( k <= j ){ /* nontrivial situation */
#if 0
                        /* temporarily, kill this for compatibility */
                        j -= k;  /* j is now the range */
#endif
                        if( k > maxoff ) maxoff = k;
                        }
                greed[i] = (yypact[i+1]-yypact[i]) + 2*j;
                if( j > maxspr ) maxspr = j;
                }

        /* initialize ggreed table */

        for( i=1; i<=nnonter; ++i ){
                ggreed[i] = 1;
                j = 0;
                /* minimum entry index is always 0 */
                q = mem + yypgo[i+1] -1;
                for( p = mem+yypgo[i]; p<q ; p += 2 ) {
                        ggreed[i] += 2;
                        if( *p > j ) j = *p;
                        }
                ggreed[i] = ggreed[i] + 2*j;
                if( j > maxoff ) maxoff = j;
                }


        /* now, prepare to put the shift actions into the a array */

        for( i=0; i<ACTSIZE; ++i ) a[i] = 0;
        maxa = a;

        for( i=0; i<nstate; ++i ) {
                if( greed[i]==0 && adb>1 ) fprintf( ftable,
                        "State %d: null\n", i );
                pa[i] = YYFLAG1;
                }

        while( (i = nxti()) != NOMORE ) {
                if( i >= 0 ) stin(i);
                else gin(-i);

                }

        if( adb>2 ){ /* print a array */
                for( p=a; p <= maxa; p += 10){
                        fprintf( ftable, "%4d  ", p-a );
                        for( i=0; i<10; ++i ) fprintf( ftable, "%4d  ", p[i] );
                        fprintf( ftable, "\n" );
                        }
                }
        /* write out the output appropriate to the language */

        aoutput();

        osummary();
        ZAPFILE(TEMPNAME);
        }

gin(i){

        register *p, *r, *s, *q1, *q2;

        /* enter gotos on nonterminal i into array a */

        ggreed[i] = 0;

        q2 = mem+ yypgo[i+1] - 1;
        q1 = mem + yypgo[i];

        /* now, find a place for it */

        for( p=a; p < &a[ACTSIZE]; ++p ){
                if( *p ) continue;
                for( r=q1; r<q2; r+=2 ){
                        s = p + *r +1;
                        if( *s ) goto nextgp;
                        if( s > maxa ){
                                if( (maxa=s) > &a[ACTSIZE] )
                                        error( MSGSTR(ARYOVR,
                                                "a array overflow") ); /*MSG*/
                                }
                        }
                /* we have found a spot */

                *p = *q2;
                if( p > maxa ){
                                if( (maxa=p) > &a[ACTSIZE] )
                                        error( MSGSTR(ARYOVR,
                                                "a array overflow") ); /*MSG*/
                        }
                for( r=q1; r<q2; r+=2 ){
                        s = p + *r + 1;
                        *s = r[1];
                        }

                pgo[i] = p-a;
                if( adb>1 ) fprintf( ftable, MSGSTR(NONTERM3,
                        "Nonterminal %d, entry at %d\n") , i, pgo[i] ); /*MSG*/
                goto nextgi;

                nextgp:  ;
                }

        error( MSGSTR(CANTGOTO, "cannot place goto %d\n"), i ); /*MSG*/

        nextgi:  ;
        }

stin(i){
        register *r, *s, n, flag, j, *q1, *q2;

        greed[i] = 0;

        /* enter state i into the a array */

        q2 = mem+yypact[i+1];
        q1 = mem+yypact[i];
        /* find an acceptable place */

        for( n= -maxoff; n<ACTSIZE; ++n ){

                flag = 0;
                for( r = q1; r < q2; r += 2 ){
                        if( (s = *r + n + a ) < a ) goto nextn;
                        if( *s == 0 ) ++flag;
                        else if( *s != r[1] ) goto nextn;
                        }

                /* check that the position equals another only
                 * if the states are identical */

                for( j=0; j<nstate; ++j ){
                        if( pa[j] == n ) {
                                if( flag ) goto nextn; /* we have some
                                                        * disagreement */
                                if( yypact[j+1] + yypact[i] ==
                                    yypact[j] + yypact[i+1] ){
                                        /* states are equal */
                                        pa[i] = n;
                                        if( adb>1 ) fprintf( ftable, MSGSTR(STENTEQ, "State %d: entry at %d equals state %d\n"), /*MSG*/
                                                i, n, j );
                                        return;
                                        }
                                goto nextn;  /* we have some disagreement */
                                }
                        }

                for( r = q1; r < q2; r += 2 ){
                        if( (s = *r + n + a ) >= &a[ACTSIZE] )
                                error( MSGSTR(OPTSPC,
                                "out of space in optimizer a array")); /*MSG*/
                        if( s > maxa ) maxa = s;
                        if( *s != 0 && *s != r[1] )    {
                                char emsg[512];
                                sprintf( emsg, MSGSTR(ARYCLBR,
                                        "clobber of a array, pos'n %d, by %d"),
                                        s-a, r[1] );
                                error( "%s", emsg );
                                }
                        *s = r[1];
                        }
                pa[i] = n;
                if( adb>1 ) fprintf( ftable, MSGSTR(STATENT,
                        "State %d: entry at %d\n"), i, pa[i] ); /*MSG*/
                return;

                nextn:  ;
                }

        error( MSGSTR(STATERR, "Error; failure to place state %d\n"),
                i ); /*MSG*/

        }

nxti(){ /* finds the next i */
        register i, max, maxi;

        max = 0;

        for( i=1; i<= nnonter; ++i ) if( ggreed[i] >= max ){
                max = ggreed[i];
                maxi = -i;
                }

        for( i=0; i<nstate; ++i ) if( greed[i] >= max ){
                max = greed[i];
                maxi = i;
                }

        if( nxdb ) fprintf( ftable, "nxti = %d, max = %d\n", maxi, max );
        if( max==0 ) return( NOMORE );
        else return( maxi );
        }

osummary(){
        /* write summary */

        register i, *p;

        if( foutput == NULL ) return;
        i=0;
        for( p=maxa; p>=a; --p ) {
                if( *p == 0 ) ++i;
                }

        fprintf( foutput, MSGSTR(OPTUSED,
                "Optimizer space used: input %d/%d, output %d/%d\n"), /*MSG*/
                pmem-mem+1, memsize, maxa-a+1, ACTSIZE );
        fprintf( foutput, MSGSTR(OPTTAB, "%d table entries, %d zero\n"),
                (maxa-a)+1, i ); /*MSG*/
        fprintf( foutput, MSGSTR(OPTSPRED,
                "maximum spread: %d, maximum offset: %d\n"),
                maxspr, maxoff ); /*MSG*/

        }

aoutput(){ /* this version is for C */


        /* write out the optimized parser */

        fprintf( ftable, "# define YYLAST %d\n", maxa-a+1 );

        warray( "yyact", a, (maxa-a)+1 );
        warray( "yypact", pa, nstate );
        warray( "yypgo", pgo, nnonter+1 );

        }


gtnm(){

        register s, val, c;

        /* read and convert an integer from the standard input */
        /* return the terminating character */
        /* blanks, tabs, and newlines are ignored */

        s = 1;
        val = 0;

        while( (c=getc(finput)) != EOF ){
                if( isdigit(c) ){
                        val = val * 10 + c - '0';
                        }
                else if ( c == '-' ) s = -1;
                else break;
                }

        *pmem++ = s*val;
        if( pmem > &mem[memsize] ) error( MSGSTR(OUTOSPC,
                "out of space") ); /*MSG*/
        return( c );

        }
