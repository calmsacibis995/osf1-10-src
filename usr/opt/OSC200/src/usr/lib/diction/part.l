%{
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
%}
%{
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
static char sccsid[] = "@(#)part.l	1.5  com/bsd.d/style.d,3.1,9021 4/2/90 15:30:22";
/*
 * COMPONENT_NAME: (CMDTEXT) Text Formatting Services
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26,27,28
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
%}
%{

/*              include file for message texts          */
#include "nwords_msg.h" 
#define MSGSTR(N,S)  NLcatgets(catd,MS_nwords,N,S)
nl_catd  catd;   /* Cat descriptor for scmc conversion */
#include "nl_types.h"
#include <locale.h>
/* part.l	4.2	(Berkeley)	82/11/06"; */

#include "style.h"
#include "names.h"
#include "conp.h"
FILE *deb;
int nosave = 1;
int part = 0;
int barebones = 0;
int topic = 0;
int style = 1;
int pastyle = 0;
int pstyle = 0;
int lstyle = 0;
int rstyle = 0;
int estyle = 0;
int nstyle = 0;
int Nstyle = 0;
int lthresh;
int rthresh;
int nomin;
char s[SCHAR];
char *sptr = s;
struct ss sent[SLENG];
struct ss *sentp = sent;
float wperc();
float sperc();
float typersent();
float vperc();
int numsent = 0;
int qcount = 0;
int icount = 0;
long vowel = 0;
long numwds = 0;
long twds = 0;
long numnonf = 0;
long letnonf = 0;
int maxsent = 0;
int maxindex = 0;
int minsent = 30;
int minindex = 0;
int simple = 0;
int compound = 0;
int compdx = 0;
int prepc = 0;
int conjc = 0;
int complex = 0;
int tobe = 0;
int adj = 0;
int infin = 0;
int pron = 0;
int passive = 0;
int aux = 0;
int adv = 0;
int verbc = 0;
int tverbc = 0;
int noun = 0;
long numlet = 0;
int beg[15]  = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
int sleng[50];
int nsleng = 0;
int j,jj,i;
int comma = 0;
int cflg;
int question;
int quote = 0;
char *st;
int initf = 0;
int over = 1;
int nroff = 0;
int nrofflg = 0;
int leng[MAXPAR];
int sentno= 0;
%}
C	[A-DF-Za-z]
%%
^{C}:.+	{
collect:
	sentp->cc = sentp->ic = yytext[0];
	if(sentp->cc == NOM)
		sentp->cc = NOUN;
collect1:
	nsleng++;
	sentp->leng = yyleng-2;
	sentp++->sp = sptr;
	if(sentp >= &sent[SLENG-1]){
		if(over)fprintf(stderr, MSGSTR(M_MSG_1, "sentence %d too many words\n") ,numsent+2);
		over=0;
		sentp--;
	}
	if(sptr+yyleng >= &s[SCHAR-1]){
		if(over)fprintf(stderr, MSGSTR(M_MSG_2, "sentence %d too many characters\n") ,numsent+2);
		over=0;
	}
	else {
		for(i=2;i<yyleng;i++)*sptr++=yytext[i];
		*sptr++ = '\0';
	}
	}
^";:".+	{
	sentp->cc=END;
	sentp->ic = ';';
	goto collect1;
	}
^",:".+	{
	comma++;
	goto collect;
	}
^",:"[:,-]+	{
	comma++;
	goto collect;
	}
[\n]	;
^"\":".+	{
	goto collect;
	}
^E:"/."	{
	cflg = 1;
	goto sdone;
	}
^E:.+	{
	cflg = 0;
sdone:
	over=1;
	sentp->cc=sentp->ic=END;
	sentp++->sp = sptr;
	for(i=2;i<yyleng;i++)*sptr++=yytext[i];
	*sptr++='\0';
	if(yytext[2]=='?')question=1;
	else question=0;

fragment:
	jj=0;
	if(quote == 1 && sent[jj].cc == ED){
		sent[jj].cc = VERB;
		quote = 0;
	}
	if(sent[jj].cc=='"')jj++;
	if(sent[jj].cc==SUBCONJ){
		if(sent[jj+1].cc == ','){
			sent[jj].cc=ADV;
			jj += 2;
			comma--;
		}
		else {
			jj=scan(1,',',0);
			if(jj != -1)jj++;
			comma--;
		}
	}
	if(jj != -1){
		if(sent[jj].cc==CONJ || sent[jj].cc=='"')jj++;
		while((jj=scan(jj,END,cflg)) != -1){
			jj++;
			if(sent[jj].cc == SUBCONJ && sent[jj+1].cc == ','){
				sent[jj].cc=ADV;
				jj += 2;
				comma--;
			}
		}
	}
	st = sent[i].sp;
	if(*(st+1) == '"')
		if(*st == '?' || *st == '!')quote = 1;
	outp();
	nsleng = 0;
	if(nroff){
		if(sentno > 0){
			printf(".SL \"");
			for(i=0;i<sentno;i++)
				printf(" %d",leng[i]);
			printf("\"\n");
			sentno = 0;
		}
		printf("%s",&yytext[1]);
		nroff = 0;
	}
	sptr=s;
	sentp=sent;
	comma=0;
	}
;.+	{
	if(style){
		nomin = atoi(&yytext[1]);
	}
	}
^:.+[\n]	{
	nrofflg=1;
	if(sentp != sent){
		sentp->cc = sentp->ic = END;
		sentp++->sp = sptr;
		*sptr++ = '.';
		*sptr++ = '\0';
		over = 1;
		nroff = 1;
		goto fragment;
	}
	if(sentno > 0){
		printf(".SL \"");
		for(i=0;i<sentno;i++)
			printf(" %d",leng[i]);
		printf("\"\n");
		sentno = 0;
	}
	printf("%s",&yytext[1]);
	}
%%
yywrap(){
	int ii;
	int ml,mg,lsum,gsum;
	float aindex, avl, avw;
	float cindex,kindex,findex,fgrad;
	float syl, avsy, adjs,snonf;
	FILE *io;

	if(style){
	if(numwds == 0 || numsent == 0)exit(0);
	avw = (float)(numwds)/(float)(numsent);
	avl = (float)(numlet)/(float)(numwds);
	aindex = 4.71*avl + .5*avw -21.43;
	twds = (twds == 0) ? 1 : twds;		/* 001 gray */
	syl = .9981*vowel-.3432*twds;
	avsy = syl/twds;
	kindex = 11.8*avsy+.39*avw-15.59;
	findex = 206.835-84.6*avsy-1.015*avw;
	if(findex < 30.)fgrad = 17.;
	else if(findex > 100.) fgrad = 4.;
	else if(findex > 70.)fgrad=(100.-findex)/10 +5.;
	else if(findex > 60.)fgrad =(70.-findex)/10+8.;
	else if(findex >50.)fgrad=(60.-findex)/5+10;
	else fgrad=(50.-findex)/6.66 +13.;
	adjs = 100 * (float)numsent/numwds;
	cindex = 5.89*avl-.3*adjs-15.8;
	printf( MSGSTR(M_MSG_11, "readability grades:\n	(Kincaid) %4.1f  (auto) %4.1f  (Coleman-Liau) %4.1f  (Flesch) %4.1f (%4.1f)\n") ,kindex,aindex,cindex,fgrad,findex);
	printf( MSGSTR(M_MSG_12, "sentence info:\n") );
	printf( MSGSTR(M_MSG_13, "	no. sent %d no. wds %ld\n") ,numsent,numwds);
	printf( MSGSTR(M_MSG_14, "	av sent leng %4.1f av word leng %4.2f\n") ,avw,avl);
	printf( MSGSTR(M_MSG_15, "	no. questions %d no. imperatives %d\n") ,qcount,icount);
	if(numnonf != 0){
		snonf = (float)(letnonf)/(float)(numnonf);
	}
	printf( MSGSTR(M_MSG_16, "	no. nonfunc wds %ld  %4.1f%%   av leng %4.2f\n") ,numnonf,(float)(numnonf)*100/numwds,snonf);
	mg = avw + 10.5;
	if(mg > 49)mg = 49;
	ml = avw - 4.5;
	if(ml <= 0)ml = 1;
	else if(ml > 49)ml=48;
	gsum = lsum = 0;
	for(ii=0;ii<50;ii++){
		if(ii < ml)lsum += sleng[ii];
		else if(ii > mg)gsum+= sleng[ii];
	}
	printf( MSGSTR(M_MSG_17, "	short sent (<%d)%3.0f%% (%d) long sent (>%d) %3.0f%% (%d)\n") ,ml,sperc(lsum),lsum,mg,sperc(gsum),gsum);
	printf( MSGSTR(M_MSG_18, "	longest sent %d wds at sent %d; shortest sent %d wds at sent %d\n") ,maxsent,maxindex,minsent,minindex);
	printf( MSGSTR(M_MSG_19, "sentence types:\n") );
	printf( MSGSTR(M_MSG_20, "	simple %3.0f%% (%d) complex %3.0f%% (%d)\n") ,sperc(simple),simple,sperc(complex),complex);
	printf( MSGSTR(M_MSG_21, "	compound %3.0f%% (%d) compound-complex %3.0f%% (%d)\n") ,sperc(compound),compound,sperc(compdx),compdx);
	printf( MSGSTR(M_MSG_22, "word usage:\n") );
	printf( MSGSTR(M_MSG_23, "	verb types as %% of total verbs\n") );
	printf( MSGSTR(M_MSG_24, "	tobe %3.0f%% (%d) aux %3.0f%% (%d) inf %3.0f%% (%d)\n") ,vperc(tobe),tobe,vperc(aux),aux,vperc(infin),infin);
	if(verbc != 0)adjs = (float)(passive)*100/(float)(verbc);
	else adjs=0;
	printf( MSGSTR(M_MSG_25, "	passives as %% of non-inf verbs %3.0f%% (%d)\n") ,adjs,passive);
	printf( MSGSTR(M_MSG_26, "	types as %% of total\n") );
	printf( MSGSTR(M_MSG_27, "	prep %3.1f%% (%d) conj %3.1f%% (%d) adv %3.1f%% (%d)\n") ,wperc(prepc),prepc,wperc(conjc),conjc,wperc(adv),adv);
	printf( MSGSTR(M_MSG_28, "	noun %3.1f%% (%d) adj %3.1f%% (%d) pron %3.1f%% (%d)\n") ,wperc(noun),noun,
		wperc(adj),adj,wperc(pron),pron);
	printf( MSGSTR(M_MSG_29, "	nominalizations %3.0f %% (%d)\n") ,wperc(nomin),nomin);
	printf( MSGSTR(M_MSG_30, "sentence beginnings:\n") );
	ii=beg[0]+beg[7]+beg[6]+beg[3]+beg[8];
	printf( MSGSTR(M_MSG_31, "	subject opener: noun (%d) pron (%d) pos (%d) adj (%d) art (%d) tot %3.0f%%\n") ,
beg[0],beg[7],beg[6],beg[3],beg[8],sperc(ii));
	printf( MSGSTR(M_MSG_32, "	prep %3.0f%% (%d) adv %3.0f%% (%d) \n") ,sperc(beg[9]),beg[9],sperc(beg[4]),beg[4]);
	printf( MSGSTR(M_MSG_33, "	verb %3.0f%% (%d) ") ,sperc(beg[1]+beg[10]+beg[11]),beg[1]+beg[10]+beg[11]);
	printf( MSGSTR(M_MSG_34, " sub_conj %3.0f%% (%d) conj %3.0f%% (%d)\n") ,sperc(beg[13]),beg[13],sperc(beg[5]),beg[5]);
	printf( MSGSTR(M_MSG_35, "	expletives %3.0f%% (%d)\n") ,sperc(beg[14]),beg[14]);
#ifdef SCATCH
	if(nosave && (fopen(SCATCH,"r")) != NULL){
	if(((io=fopen(SCATCH,"a")) != NULL)){
		fprintf(io," read %4.1f %4.1f %4.1f %4.1f %4.1f\n",kindex, aindex, cindex, findex, fgrad);
		fprintf(io," sentl %d %ld %4.2f %4.2f %d %d %ld %4.2f\n",numsent,numwds,avw,avl,qcount,icount,numnonf,snonf);
		fprintf(io," l var %d %d %d %d %d\n",ml,lsum,mg,gsum,maxsent);
		fprintf(io," t var %d %d %d %d\n",simple,complex,compound,compdx);
		fprintf(io," verbs %d %d %d %d %d %d\n",tverbc,verbc,tobe,aux,infin,passive);
		fprintf(io," ty %d %d %d %d %d %d %d\n",prepc,conjc,adv,noun,adj,pron,nomin);
		fprintf(io," beg %d %d %d %d %d %d\n",beg[0],beg[7],beg[6],beg[3],beg[8],ii);
		fprintf(io," sbeg %d %d %d %d %d %d\n",beg[9],beg[4],beg[1]+beg[10]+beg[11],beg[13],beg[5],beg[14]);
		}
	}
#endif
	}
	return(1);
}
float
wperc(a)
{
	return((float)(a)*100/numwds);
}
float
sperc(a)
{
	return((float)(a)*100/numsent);
}
float
typersent(a)
{
return((float)(a)/numsent);
}
float
vperc(a)
{
	if(tverbc == 0)return(0);
	return((float)(a)*100/tverbc);
}
main(argc,argv)
char **argv;
{
	setlocale(LC_ALL, "");
	catd=NLcatopen("nwords.cat",0);
	while(--argc > 0 && (++argv)[0][0] == '-' ){
		switch(argv[0][1]){
		case 'd': nosave = 0;
			continue;
		case 's': style=1;
			continue;
		case 'p': pastyle=style=1;
			continue;
		case 'a': pstyle=style=1;
			continue;
		case 'e': estyle = style = 1;
			continue;
		case 'n': nstyle = style = 1;
			continue;
		case 'N': Nstyle = style = 1;
			continue;
		case 'l': style=lstyle=1;
			lthresh = atoi(*(++argv));
			argc--;
			continue;
		case 'r':
			style=rstyle=1;
			rthresh = atoi(*(++argv));
			argc--;
			continue;
		case 'P':
			part = 1;
			style = 0;
			continue;
		case 'b':		/* print bare bones info rje */
			barebones = 1;
			style = 0;
			continue;
		case 'T':		/*topic*/
			style = 0;
			topic = 1;
			continue;
		default:
			fprintf(stderr, MSGSTR(M_MSG_44, "unknown flag to part %s\n") ,*argv);
			exit(1);
		}
		argv++;
	}
#ifdef SNOM
	if(fopen(SNOM,"r") != NULL){
		deb = fopen(SNOM,"a");	/* SAVE NOM*/
	}
#else
	deb = NULL;
#endif
	yylex();
	if(nrofflg && sentno > 0){
		printf(".SL \"");
		for(i=0;i<sentno;i++)
			printf(" %d",leng[i]);
		printf("\"\n");
	}
}
