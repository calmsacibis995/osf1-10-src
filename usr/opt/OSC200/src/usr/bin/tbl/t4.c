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
static char rcsid[] = "@(#)$RCSfile: t4.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/10 17:58:10 $";
#endif
/*
 * HISTORY
 */
/*
static	char	*sccsid = "@(#)t4.c	8.1	(Japanese ULTRIX)  2/19/91";
static char sccsid[] = "@(#)t4.c	4.2 8/11/83";
*/

 /* t4.c: read table specification */
# include "t..c"
int oncol;
getspec()
{
	int icol, i;
	for(icol=0; icol<MAXCOL; icol++)
	{
		sep[icol]= -1;
		evenup[icol]=0;
		cll[icol][0]=0;
		for(i=0; i<MAXHEAD; i++)
		{
			csize[i][icol][0]=0;
			vsize[i][icol][0]=0;
			font[i][icol][0] = lefline[i][icol] = 0;
			ctop[i][icol]=0;
			style[i][icol]= 'l';
		}
	}
	nclin=ncol=0;
	oncol =0;
	left1flg=rightl=0;
	readspec();
	fprintf(tabout, ".rm");
	for(i=0; i<ncol; i++)
		fprintf(tabout, " %02d", 80+i);
	fprintf(tabout, "\n");
}
readspec()
{
	int icol, c, sawchar, stopc, i;
	char sn[10], *snp, *temp;
	sawchar=icol=0;
	while (c=get1char())
	{
		switch(c)
		{
		default:
			if (c != tab)
				error(catgets(catd, 1, 6, "bad table specification character"));
		case ' ': /* note this is also case tab */
			continue;
		case '\n':
			if(sawchar==0) continue;
		case ',':
		case '.': /* end of table specification */
			ncol = max(ncol, icol);
			if (lefline[nclin][ncol]>0) {
				ncol++; 
				rightl++;
			};
			if(sawchar)
				nclin++;
			if (nclin>=MAXHEAD)
				error(catgets(catd, 1, 7, "too many lines in specification"));
			icol=0;
			if (ncol==0 || nclin==0)
				error(catgets(catd, 1, 8, "no specification"));
			if (c== '.')
			{
				while ((c=get1char()) && c != '\n')
					if (c != ' ' && c != '\t')
						error(catgets(catd, 1, 9, "dot not last character on format line"));
				/* fix up sep - default is 3 except at edge */
				for(icol=0; icol<ncol; icol++)
					if (sep[icol]<0)
						sep[icol] =  icol+1<ncol ? 3 : 1;
				if (oncol == 0)
					oncol = ncol;
				else if (oncol +2 <ncol)
					error(catgets(catd, 1, 10, "tried to widen table in T&, not allowed"));
				return;
			}
			sawchar=0;
			continue;
		case 'C': 
		case 'S': 
		case 'R': 
		case 'N': 
		case 'L':  
		case 'A':
			c += ('a'-'A');
		case '_': 
			if (c=='_') c= '-';
		case '=': 
		case '-':
		case '^':
		case 'c': 
		case 's': 
		case 'n': 
		case 'r': 
		case 'l':  
		case 'a':
			style[nclin][icol]=c;
			if (c== 's' && icol<=0)
				error(catgets(catd, 1, 11, "first column can not be S-type"));
			if (c=='s' && style[nclin][icol-1] == 'a')
			{
				fprintf(tabout, ".tm warning: can't span a-type cols, changed to l\n");
				style[nclin][icol-1] = 'l';
			}
			if (c=='s' && style[nclin][icol-1] == 'n')
			{
				fprintf(tabout, ".tm warning: can't span n-type cols, changed to c\n");
				style[nclin][icol-1] = 'c';
			}
			icol++;
			if (c=='^' && nclin<=0)
				error(catgets(catd, 1, 12, "first row can not contain vertical span"));
			if (icol>=MAXCOL)
				error(catgets(catd, 1, 13, "too many columns in table"));
			sawchar=1;
			continue;
		case 'b': 
		case 'i': 
			c += 'A'-'a';
		case 'B': 
		case 'I':
			if (icol==0) continue;
			snp=font[nclin][icol-1];
			snp[0]= (c=='I' ? '2' : '3');
			snp[1]=0;
			continue;
		case 't': 
		case 'T':
			if (icol>0)
				ctop[nclin][icol-1] = 1;
			continue;
		case 'd': 
		case 'D':
			if (icol>0)
				ctop[nclin][icol-1] = -1;
			continue;
		case 'f': 
		case 'F':
			if (icol==0) continue;
			snp=font[nclin][icol-1];
			snp[0]=snp[1]=stopc=0;
			for(i=0; i<2; i++)
			{
				c = get1char();
				if (i==0 && c=='(')
				{
					stopc=')';
					c = get1char();
				}
				if (c==0) break;
				if (c==stopc) {
					stopc=0; 
					break;
				}
				if (stopc==0)  if (c==' ' || c== tab ) break;
				if (c=='\n'){
					un1getc(c); 
					break;
				}
				snp[i] = c;
				if (c>= '0' && c<= '9') break;
			}
			if (stopc) if (get1char()!=stopc)
				error(catgets(catd, 1, 14, "Nonterminated font name"));
			continue;
		case 'P': 
		case 'p':
			if (icol<=0) continue;
			temp = snp = csize[nclin][icol-1];
			while (c = get1char())
			{
				if (c== ' ' || c== tab || c=='\n') break;
				if (c=='-' || c == '+')
					if (snp>temp)
						break;
					else
						*snp++=c;
				else
					if (digit(c))
						*snp++ = c;
					else break;
				if (snp-temp>4)
					error(catgets(catd, 1, 15, "point size too large"));
			}
			*snp = 0;
			if (atoi(temp)>36)
				error(catgets(catd, 1, 16, "point size unreasonable"));
			un1getc (c);
			continue;
		case 'V': 
		case 'v':
			if (icol<=0) continue;
			temp = snp = vsize[nclin][icol-1];
			while (c = get1char())
			{
				if (c== ' ' || c== tab || c=='\n') break;
				if (c=='-' || c == '+')
					if (snp>temp)
						break;
					else
						*snp++=c;
				else
					if (digit(c))
						*snp++ = c;
					else break;
				if (snp-temp>4)
					error(catgets(catd, 1, 17, "vertical spacing value too large"));
			}
			*snp=0;
			un1getc(c);
			continue;
		case 'w': 
		case 'W':
			snp = cll [icol-1];
			/* Dale Smith didn't like this check - possible to have two text blocks
					   of different widths now ....
						if (*snp)
							{
							fprintf(tabout, "Ignored second width specification");
							continue;
							}
					/* end commented out code ... */
			stopc=0;
			while (c = get1char())
			{
				if (snp==cll[icol-1] && c=='(')
				{
					stopc = ')';
					continue;
				}
				if ( !stopc && (c>'9' || c< '0'))
					break;
				if (stopc && c== stopc)
					break;
				*snp++ =c;
			}
			*snp=0;
			if (snp-cll[icol-1]>CLLEN)
				error (catgets(catd, 1, 18, "column width too long"));
			if (!stopc)
				un1getc(c);
			continue;
		case 'e': 
		case 'E':
			if (icol<1) continue;
			evenup[icol-1]=1;
			evenflg=1;
			continue;
		case '0': 
		case '1': 
		case '2': 
		case '3': 
		case '4':
		case '5': 
		case '6': 
		case '7': 
		case '8': 
		case '9': 
			sn[0] = c;
			snp=sn+1;
			while (digit(*snp++ = c = get1char()))
				;
			un1getc(c);
			sep[icol-1] = max(sep[icol-1], numb(sn));
			continue;
		case '|':
			lefline[nclin][icol]++;
			if (icol==0) left1flg=1;
			continue;
		}
	}
	error(catgets(catd, 1, 19, "EOF reading table specification"));
}
