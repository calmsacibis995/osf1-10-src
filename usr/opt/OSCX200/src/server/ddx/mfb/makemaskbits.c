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
 * this generates the maskbits for maskbits.c for LONG_BIT == 64
 */

MSB()
{
	unsigned long i,j, mask, tmp;
	for(i=0x8000000000000000;i>0;i>>=1) {
		printf("\t{\n");
		if ( i == 0x8000000000000000 )
		printf("\t\tLONG2CHARS( 0xFFFFFFFFFFFFFFFF ),\n");
		else
		printf("\t\tLONG2CHARS( 0x0000000000000000 ),\n");
		mask = i;
		for(j=0;j<(sizeof(long)*8-1);j++) {
			printf("\t\tLONG2CHARS( 0x%16.16lX ),\n",mask);
			tmp = mask | (mask >> 1);
			if ( mask != 0 ) {
			    if ( mask == tmp )
				    mask = 0;
			    else 
				    mask = tmp;
			}
		}
		printf("\t},\n");
	}
}

LSB()
{
	unsigned long i,j, mask, tmp;
	for(i=0x0000000000000001;i!=0;i<<=1) {
		printf("\t{\n");
		if ( i == 0x0000000000000001 )
		printf("\t\tLONG2CHARS( 0xFFFFFFFFFFFFFFFF ),\n");
		else
		printf("\t\tLONG2CHARS( 0x0000000000000000 ),\n");
		mask = i;
		for(j=0;j<(sizeof(long)*8-1);j++) {
			printf("\t\tLONG2CHARS( 0x%16.16lX ),\n",mask);
			tmp = mask | (mask << 1);
			if ( mask != 0 ) {
			    if ( mask == tmp )
				    mask = 0;
			    else 
				    mask = tmp;
			}
		}
		printf("\t},\n");
	}
}

main()
{
	MSB();
	printf("********************\n");
	LSB();
}
