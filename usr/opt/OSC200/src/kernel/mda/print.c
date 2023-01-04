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
static char	*sccsid = "@(#)$RCSfile: print.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:37:31 $";
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
#include "mda.h"
#include "machine/vm_types.h"


#define	max(a, b)	(((a) > (b)) ? (a) : (b))

printthree(fmt, p1, p2, p3, v1, v2, v3)
char	*fmt;
char	*p1, *p2, *p3;
unsigned int v1, v2, v3;
{
	int	l1, l2, l3, tot, boundary;
	char	*Name = "%15s:";

	l1 = strlen(p1);
	l2 = strlen(p2);
	l3 = strlen(p3);

	if(*fmt != ' ') {
		printf(Name, p1);
		tot = max(l1, 16) + printval(*fmt, v1);
		boundary = 26;
	} else {
		tot = 0;
		boundary = 0;
	}

	
	if(*++fmt != ' ') {
		if(tot > boundary) {
			printf("\n%*s:", boundary + 15, p2);
			tot = boundary + 16;
		} else {
			printf(Name, p2);
			tot += max(l2, 16);
		}
		tot += printval(*fmt, v2);
		boundary += 26;
	}

	if(*++fmt != ' ') {
		if(tot > boundary)
			printf("\n%*s:", boundary*2 + 16, p3);
		else
			printf(Name, p3);
		printval(*fmt, v3);
	}
	putchar('\n');
}

printval(how, val)
char	how;
int	val;
{
	char	*fmt;
	char	buf[80];

	switch(how) {
		case 'd':	fmt = "%10d";
				break;

		case 'u':	fmt = "%10u";
				break;

			
		case 'x':	fmt = "%#10x";
				break;

		case 'o':	fmt = "%#10o";
				break;

		case 's':	fmt = "%10s";
				break;

		default:	fmt = "%10d";
				break;

	}

	/*
	 * printf is supposed to return the number of bytes output,
	 *  but it doesn't.
	 */
	sprintf(buf, fmt, val);
	printf(buf);
	return(strlen(buf));;
}


vprt1l(p1, v1)
     char	  *p1;
     unsigned int *v1;	 	/* VIRTUAL addresses ! */
{
  unsigned int val1;
  unsigned int *pv1;		/* PHYSICAL addresses ! */
  
  int result;

  result = phys(v1, &pv1, ptb0);
  if (result != SUCCESS)
    return(FAILED);

  val1 = MAP(pv1);
  printf("%15s: %#10x\n", p1, val1);
  return(SUCCESS);
}

vprt1s(p1, v1)
     char	  *p1;
     unsigned short *v1; 	/* VIRTUAL addresses ! */
{
  unsigned short val1;
  unsigned short *pv1;		/* PHYSICAL addresses ! */
  
  int result;

  result = phys(v1, &pv1, ptb0);
  if (result != SUCCESS)
    return(FAILED);

  val1 = MAP(pv1);
  printf("%15s: %#10x\n", p1, val1);
  return(SUCCESS);
}

vprt3s(p1, p2, p3, v1, v2, v3)
     char	    *p1, *p2, *p3;
     unsigned short *v1, *v2, *v3; 	/* VIRTUAL addresses ! */
{
  unsigned short val1, val2, val3;
  unsigned short *pv1, *pv2, *pv3;	/* PHYSICAL addresses ! */
  
  int result;

  result = phys(v1, &pv1, ptb0);
  if (result != SUCCESS)
    return(FAILED);
  result = phys(v2, &pv2, ptb0);
  if (result != SUCCESS)
    return(FAILED);
  result = phys(v3, &pv3, ptb0);
  if (result != SUCCESS)
    return(FAILED);
  val1 = MAP(pv1);
  val2 = MAP(pv2);
  val3 = MAP(pv3);

  printthree("xxx", p1, p2, p3, val1, val2, val3);
}


vprt3l(p1, p2, p3, v1, v2, v3)
     char	  *p1, *p2, *p3;
     unsigned int *v1, *v2, *v3; 	/* VIRTUAL addresses ! */
{
  unsigned int val1, val2, val3;
  unsigned int *pv1, *pv2, *pv3;	/* PHYSICAL addresses ! */
  
  int result;

  result = phys(v1, &pv1, ptb0);
  if (result != SUCCESS)
    return(FAILED);
  result = phys(v2, &pv2, ptb0);
  if (result != SUCCESS)
    return(FAILED);
  result = phys(v3, &pv3, ptb0);
  if (result != SUCCESS)
    return(FAILED);
  val1 = MAP(pv1);
  val2 = MAP(pv2);
  val3 = MAP(pv3);

  printthree("xxx", p1, p2, p3, val1, val2, val3);
}


vprintthree(fmt, p1, p2, p3, v1, v2, v3)
     char	  *p1, *p2, *p3;
     unsigned int *v1, *v2, *v3; 	/* VIRTUAL addresses ! */
{
  unsigned int val1, val2, val3;
  unsigned int *pv1, *pv2, *pv3;	/* PHYSICAL addresses ! */
  
  int result;

  result = phys(v1, &pv1, ptb0);
  if (result != SUCCESS)
    return(FAILED);
  result = phys(v2, &pv2, ptb0);
  if (result != SUCCESS)
    return(FAILED);
  result = phys(v3, &pv3, ptb0);
  if (result != SUCCESS)
    return(FAILED);
  val1 = MAP(pv1);
  val2 = MAP(pv2);
  val3 = MAP(pv3);

  printthree(fmt, p1, p2, p3, val1, val2, val3);
}


sdiagram(buf, reg, names, size)
char	*buf;
int     reg;
char   *names[];
int     size;
{
	int     prev, i, k;
	char	*p;

	p = buf;
	*p++ = '<';
	prev = 0;
	for(i=1, k = 0; k < size; i <<= 1, k++) {
		if(reg & i) {
			if(prev)
				strcat(p, ",");
			strcat(p, names[k]);
			prev++;
		}
	}
	strcat(p, ">");
}
